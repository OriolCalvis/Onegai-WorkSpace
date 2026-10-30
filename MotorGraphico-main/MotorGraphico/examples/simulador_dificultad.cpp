// ACOMPANANTES-01 — El simulador del estudio de dificultad (06/10/2026).
//
// Oriol: «un combate 1 contra 3, siendo tu el 1, mueres un 40 % de las veces
// [...] 2 contra 3 mueres pocas veces, mucha mala suerte; 3 contra 3 solo
// morirías si son todos mas fuertes que tu [...] prueba con el Apostata, el
// Señor de las Armas y la Furia de los Creadores ademas del Bardo». Y: «piensa
// en el equipo, que puedes subir los stats maximos, y haz un ranking por
// niveles de fuerza».
//
// Combates con el motor de verdad: BattleState, la mesa de cartas
// (Game::CartasEnCombate) y los acompañantes. El heroe se crea como en el
// juego: cada una de las razas jugables, la clase con todas sus cartas, el kit
// del cofre puesto pieza a pieza (InventoryEngine) y la vida de TierRules; o,
// para los niveles de fuerza, el mejor reparto que permite el tope de dados
// del equipo (RPG/TopesDados.h). Juega una IA de cartas por clase: prioridades
// escritas a mano, como jugaria alguien con cabeza, y remata al que esta a un
// golpe. Cuenta las veces que cae el heroe.
//
// No es una prueba: es la herramienta para rehacer las cuentas cuando cambien
// las cartas, el equipo o los enemigos. Las conclusiones del 06/10, en
// sesiones/2026-10-06-claude-acompanantes-dificultad.md; las del dano por
// diferencia (DANO-02) y las vidas a la mitad, en
// sesiones/2026-10-06-claude-dano-diferencia.md.
//
// Uso (desde la carpeta de build, que tiene assets/):
//   simulador_dificultad <modo> [combates por raza, 25 por defecto]
//   modos: fichas | escalera | equipo | niveles | razas | ocaso | jefes | log | kits | alternativas | esbirros
//   (esbirros, D0014 14/10: los esbirros con DES 0, 1 y 2 y lo que se mide de ellos)
//   (alternativas, BND-ALT 08/10: los combates de las alternativas de Boundington)
//   MG_VIDA_HEROE, MG_VIDA_ENEMIGO y MG_ESCALERA prueban otras vidas y otra
//   escalera sin tocar los datos (ver vidaDeHeroe y escalera()).
#include "Game/Acompanante.h"
#include "Game/BattleState.h"
#include "Game/CartasEnCombate.h"
#include "Game/CatalogoCartas.h"
#include "Game/Personajes/KitBasico.h"
#include "Game/Skill.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterCreation.h"
#include "RPG/CharacterSheet.h"
#include "RPG/InventoryEngine.h"
#include "RPG/Narrative/AdventureScript.h"
#include "RPG/RandomEngine.h"
#include "RPG/TierRules.h"
#include "RPG/TopesDados.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace {

// ---------------------------------------------------------------- datos

[[noreturn]] void muere(const std::string& que) {
    std::fprintf(stderr, "simulador_dificultad: %s\n", que.c_str());
    std::exit(1);
}

const Game::CatalogoCartas& cartas() {
    static const std::unique_ptr<Game::CatalogoCartas> c = [] {
        auto r = Game::CatalogoCartas::cargar("assets/catalogs");
        if (!r.isOk()) muere(r.errorMessage());
        return std::move(r.value());
    }();
    return *c;
}
const RPG::CharacterCreation& creacion() {
    static const RPG::CharacterCreation c = [] {
        auto r = RPG::CharacterCreation::load("assets/catalogs");
        if (!r.isOk()) muere(r.errorMessage());
        return std::move(r.value());
    }();
    return c;
}
const RPG::Catalogs::EquipmentCatalog& equipo() {
    static const RPG::Catalogs::EquipmentCatalog e = [] {
        RPG::Catalogs::EquipmentCatalog c;
        if (!c.loadFromFile("assets/catalogs/equipment.json").isOk()) muere("sin equipment.json");
        return c;
    }();
    return e;
}
const RPG::TierRules& tiers() {
    static const RPG::TierRules t = [] {
        RPG::TierRules r;
        if (!r.loadFromFile("assets/rules/tier_rules.json").isOk()) muere("sin tier_rules.json");
        return r;
    }();
    return t;
}
const Game::KitBasico::Reglas& reglasKit() {
    static const Game::KitBasico::Reglas r = [] {
        auto c = Game::KitBasico::cargarReglas("assets/rules/kit_basico.json");
        if (!c.isOk()) muere(c.errorMessage());
        return c.value();
    }();
    return r;
}

uint64_t mezcla(uint64_t x) {  // splitmix64: semillas que no se parecen
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

// ---------------------------------------------------------------- cuerpos

// Para probar otras vidas sin tocar los datos: MG_VIDA_HEROE="base%,con,car,tope"
// cambia la formula del heroe (ese tanto por ciento de la vida base de la
// clase + CON x con + CAR x car, hasta el tope) y MG_VIDA_ENEMIGO="50" deja a
// los enemigos en ese tanto por ciento.
int vidaBaseDeClase(const std::string& id);
int vidaDeHeroe(const RPG::CharacterSheet& f) {
    const char* v = std::getenv("MG_VIDA_HEROE");
    if (v == nullptr) return std::max(1, f.healthCap());
    int base = 0, con = 0, car = 0, tope = 0;
    if (std::sscanf(v, "%d,%d,%d,%d", &base, &con, &car, &tope) != 4) return std::max(1, f.healthCap());
    const int b = (vidaBaseDeClase(f.classId) * base + 50) / 100;
    return std::max(1, std::min(tope, b + con * f.con() + car * f.car()));
}
int vidaDeEnemigo(int vida) {
    const char* v = std::getenv("MG_VIDA_ENEMIGO");
    return v == nullptr ? vida : std::max(1, (vida * std::atoi(v) + 50) / 100);
}

class Heroe : public ICombatant {
public:
    explicit Heroe(const RPG::CharacterSheet& f) : ficha(f), max(vidaDeHeroe(f)), vida(max) {}
    RPG::CharacterSheet ficha;
    int max, vida;
    RPG::CharacterSheet* character_sheet() override { return &ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return &ficha; }
    void takeDamage(int n) override { vida = n >= vida ? 0 : vida - std::max(0, n); }
    void heal(int n) override { vida = std::min(max, vida + std::max(0, n)); }
    int health() const override { return vida; }
    int maxHealth() const override { return max; }
    bool isAlive() const override { return vida > 0; }
    int stat(RPG::Stat s) const override { return ficha.stat(s); }
    RPG::DefenseBlock defenses() const override { return ficha.defenseValues(); }
    int tier() const override { return ficha.tier; }
    std::string combatant_id() const override { return "heroe"; }
};

// Un enemigo sin ficha de monstruo, como los de las aventuras: vida y dados
// (ataca con 1 + DES y se defiende con el mayor de CON y DES).
struct PerfilEnemigo {
    std::string nombre;
    int vida = 1;
    std::array<int, 4> stats{};  // CON DES INT CAR
};

class Enemigo : public ICombatant {
public:
    explicit Enemigo(const PerfilEnemigo& p) : m_p(p), m_vida(vidaDeEnemigo(p.vida)) { m_p.vida = m_vida; }
    void takeDamage(int n) override { m_vida = n >= m_vida ? 0 : m_vida - std::max(0, n); }
    void heal(int n) override { m_vida = std::min(m_p.vida, m_vida + std::max(0, n)); }
    int health() const override { return m_vida; }
    int maxHealth() const override { return m_p.vida; }
    bool isAlive() const override { return m_vida > 0; }
    int stat(RPG::Stat s) const override { return m_p.stats[static_cast<std::size_t>(s)]; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return m_p.nombre; }

private:
    PerfilEnemigo m_p;
    int m_vida;
};

// La escalera de fuerza de Tier 1 (06/10), con el dano de DANO-02 (d6 por la
// diferencia de exitos) y las vidas a la mitad: del esbirro al jefe. Con un
// solo dado de ataque (DES 0) un enemigo casi no hiere: el estandar es DES 1.
const std::vector<PerfilEnemigo>& escalera() {
    static const std::vector<PerfilEnemigo> e = {
        {"F1 esbirro", 4, {0, 0, 0, 0}},     {"F2 recluta", 6, {1, 0, 0, 0}},
        {"F3 estandar", 8, {1, 1, 0, 0}},    {"F4 veterano", 6, {1, 2, 0, 0}},
        {"F5 elite", 8, {2, 2, 0, 0}},       {"F6 elite dura", 6, {2, 3, 0, 0}},
        {"F7 campeon", 8, {2, 3, 0, 0}},     {"F8 campeon fiero", 10, {3, 3, 0, 0}},
        {"F9 jefe", 8, {3, 4, 0, 0}},
    };
    // MG_ESCALERA="nombre:vida:CON:DES;..." para probar otra.
    static const std::vector<PerfilEnemigo> otra = [] {
        std::vector<PerfilEnemigo> v;
        const char* t = std::getenv("MG_ESCALERA");
        std::string resto = t != nullptr ? t : "";
        while (!resto.empty()) {
            const std::string paso = resto.substr(0, resto.find(';'));
            resto = resto.find(';') == std::string::npos ? "" : resto.substr(resto.find(';') + 1);
            char nombre[64] = {};
            int vida = 0, con = 0, des = 0;
            if (std::sscanf(paso.c_str(), "%63[^:]:%d:%d:%d", nombre, &vida, &con, &des) == 4)
                v.push_back({nombre, vida, {con, des, 0, 0}});
        }
        return v;
    }();
    return otra.empty() ? e : otra;
}

// ---------------------------------------------------------------- el heroe

int indiceStat(const std::string& s) {
    const char* n[4] = {"CON", "DES", "INT", "CAR"};
    for (int i = 0; i < 4; ++i)
        if (s == n[i]) return i;
    return 1;
}

// Nivel de equipo: tope 0 = el kit del cofre tal cual (+2, +2, +1, +1 en la
// clase); tope 3 o 4 = el mejor reparto de ese tope (3 3 1 1, 4 4 2 2) en la
// principal y la secundaria de la clase, o en CON y DES («para aguantar»).
struct Equipo {
    const char* nombre;
    int tope = 0;
    bool conDes = false;
};

std::array<int, 4> reparto(const RPG::ClassDefinition& cls, int tope, bool conDes) {
    const RPG::TopesDados::Reparto r = RPG::TopesDados::maximo(tope);
    std::vector<int> orden;
    auto pon = [&](int i) {
        if (std::find(orden.begin(), orden.end(), i) == orden.end()) orden.push_back(i);
    };
    if (conDes) {
        pon(0);
        pon(1);
    }
    pon(indiceStat(cls.primaryStat));
    pon(indiceStat(cls.secondaryStat));
    for (int i = 0; i < 4; ++i) pon(i);
    std::array<int, 4> b{};
    b[static_cast<std::size_t>(orden[0])] = r.primero;
    b[static_cast<std::size_t>(orden[1])] = r.segundo;
    b[static_cast<std::size_t>(orden[2])] = std::max(1, r.resto);
    b[static_cast<std::size_t>(orden[3])] = std::max(1, r.resto);
    return b;
}

RPG::CharacterSheet creaFicha(const RPG::ClassDefinition& cls, const RPG::RaceDefinition& raza, const Equipo& eq) {
    RPG::CharacterSheet f;
    f.tier = 1;
    f.classId = cls.id;
    f.raceId = raza.id;
    f.racialBonuses = raza.statBonuses;
    f.knownSkillIds = cls.startingSkillIds;
    for (const std::string& id : cls.learnableSkillIds)
        if (std::find(f.knownSkillIds.begin(), f.knownSkillIds.end(), id) == f.knownSkillIds.end())
            f.knownSkillIds.push_back(id);
    if (!cls.passiveId.empty()) f.passiveIds.push_back(cls.passiveId);
    const Game::KitBasico::Kit kit = Game::KitBasico::deClase(cls, equipo(), tiers(), reglasKit());
    for (const std::string& id : kit.piezas) {
        if (const RPG::EquipmentDefinition* e = equipo().find(id)) {
            f.inventoryEquipment.push_back({id, 1});
            (void)RPG::InventoryEngine::equip(f, *e, cls, tiers(), equipo());
        }
    }
    // Con mas tope, las mismas piezas (su armadura da la misma vida) y los
    // dados del mejor reparto.
    if (eq.tope > 0 || eq.conDes) f.equipBonuses = reparto(cls, eq.tope > 0 ? eq.tope : 2, eq.conDes);
    f.recompute_derived(tiers(), &cls);
    return f;
}

// ---------------------------------------------------------------- la IA

struct Partida {
    BattleState* b = nullptr;
    Game::CartasEnCombate* m = nullptr;
    std::vector<Enemigo*> enemigos;
    Heroe* heroe = nullptr;
};

int vivos(const Partida& p) {
    int n = 0;
    for (const Enemigo* e : p.enemigos) n += e->isAlive() ? 1 : 0;
    return n;
}
int elegido(const Partida& p, bool fuerte) {
    int mejor = -1;
    for (std::size_t i = 0; i < p.enemigos.size(); ++i) {
        if (!p.enemigos[i]->isAlive()) continue;
        const int v = p.enemigos[i]->health();
        if (mejor < 0 || (fuerte ? v > p.enemigos[static_cast<std::size_t>(mejor)]->health()
                                 : v < p.enemigos[static_cast<std::size_t>(mejor)]->health()))
            mejor = static_cast<int>(i);
    }
    return mejor;
}
float vidaHeroe(const Partida& p) { return static_cast<float>(p.heroe->health()) / static_cast<float>(p.heroe->maxHealth()); }

bool juega(Partida& p, const std::string& accion, int objetivo) {
    for (const auto& f : p.m->construirFilas()) {
        if (f.accion != accion || !f.bloqueo.empty()) continue;
        p.b->resolveAllyAction(0, BattleAction{BattleActionType::UseSkill, accion, std::max(0, objetivo), ""});
        return true;
    }
    return false;
}

enum class Blanco { Debil, Fuerte, Yo };
using Cond = std::function<bool(const Partida&)>;
struct Jugada {
    std::string accion;
    Blanco blanco;
    Cond si;
    bool antesDeRematar = false;  // vale mas que rematar al que esta a un golpe
};
struct Politica {
    std::string estacion, reaccion, canaliza;
    std::vector<Jugada> libres, principal;
};

const Cond siempre = [](const Partida&) { return true; };
const Cond enGrupo = [](const Partida& p) { return vivos(p) >= 2; };

Politica politicaDe(const std::string& clase) {
    Politica pol;
    if (clase == "bardo_de_las_cuatro_estaciones") {
        pol.estacion = "invierno";  // la mejor de las cuatro para aguantar
        pol.reaccion = "bendicion_de_las_estrellas";
        pol.libres = {{"ola_de_elementos#cancion_del_invierno", Blanco::Yo, siempre}};
        pol.principal = {
            {"cancion_de_la_primavera", Blanco::Yo, [](const Partida& p) { return vidaHeroe(p) <= 0.45f; }, true},
            {"cancion_del_invierno", Blanco::Fuerte, enGrupo, true},
            {"cancion_del_verano", Blanco::Fuerte, siempre},
            {"cancion_del_otono", Blanco::Fuerte, enGrupo},
            {"cancion_del_invierno", Blanco::Fuerte, siempre},
            {"invocacion_elemental", Blanco::Yo, siempre},
        };
    } else if (clase == "senor_de_las_armas") {
        pol.reaccion = "escudo_divino";
        pol.principal = {
            {"rafagas_del_arsenal", Blanco::Debil, enGrupo, true},
            {"corte_real", Blanco::Fuerte, siempre},
            {"cadena_real", Blanco::Fuerte, enGrupo},
            {"rafagas_del_arsenal", Blanco::Debil, siempre},
            {"arsenal_del_rey", Blanco::Yo, siempre},
        };
    } else if (clase == "furia_de_los_creadores") {
        pol.reaccion = "proteccion_del_creador";
        pol.principal = {
            {"furia_fe", Blanco::Debil, [](const Partida& p) { return vidaHeroe(p) <= 0.5f; }, true},
            {"ataque_del_rayo", Blanco::Fuerte, enGrupo, true},
            {"ira_imparable", Blanco::Debil, siempre},
            {"castigo_divino", Blanco::Fuerte, siempre},
            {"ataque_del_rayo", Blanco::Fuerte, siempre},
            {"furia_fe", Blanco::Debil, siempre},
        };
    } else if (clase == "el_apostata") {
        pol.canaliza = "absorcion_de_alma";
        pol.principal = {
            {"invocacion_oscura#espectro_menor", Blanco::Yo, siempre, true},
            {"estruendo_desolador", Blanco::Fuerte, enGrupo, true},
            {"maldicion_de_la_debilidad", Blanco::Fuerte, enGrupo},
            {"escudo_de_sombras", Blanco::Yo, siempre},
            {"marca_profana", Blanco::Debil, siempre},
            {"destructor_del_escudo_de_sombras", Blanco::Debil, siempre},
            {"estruendo_desolador", Blanco::Fuerte, siempre},
        };
    }
    return pol;
}

int blanco(const Partida& p, Blanco b) { return b == Blanco::Yo ? 0 : elegido(p, b == Blanco::Fuerte); }

constexpr int kRemate = 15;  // remata si el mas debil tiene esto o menos (un exito y un parcial)

// El turno del heroe: elige estacion, prepara su reaccion, canaliza, juega lo
// libre y la accion principal, o ataca al mas debil.
void turnoHeroe(Partida& p, const Politica& pol) {
    for (int paso = 0; paso < 10 && p.b->outcome() == BattleOutcome::InProgress; ++paso) {
        const Game::PasivaEnCombate& pas = p.m->pasiva();
        if (!pas.opciones.empty() && pas.elegida.empty() && !pol.estacion.empty() && juega(p, "#elige#" + pol.estacion, 0))
            continue;
        const Game::TurnoHeroe& t = p.m->turno();
        if (!pol.reaccion.empty() && t.preparada < 0 && !t.reaccionUsada && juega(p, pol.reaccion, 0)) continue;
        if (!pol.canaliza.empty() && !t.canalizacion && juega(p, pol.canaliza, elegido(p, true))) continue;
        bool hecha = false;
        for (const Jugada& j : pol.libres)
            if (!hecha && j.si(p) && juega(p, j.accion, blanco(p, j.blanco))) hecha = true;
        if (hecha) continue;
        const int debil = elegido(p, false);
        const bool aUnGolpe = debil >= 0 && p.enemigos[static_cast<std::size_t>(debil)]->health() <= kRemate;
        for (const Jugada& j : pol.principal)
            if (!hecha && (!aUnGolpe || j.antesDeRematar) && j.si(p) && juega(p, j.accion, blanco(p, j.blanco)))
                hecha = true;
        if (hecha) {
            if (!p.m->turnoAbierto()) return;
            continue;
        }
        p.b->resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", std::max(0, debil), ""});
        return;
    }
}

// ---------------------------------------------------------------- combates

struct Escenario {
    std::vector<PerfilEnemigo> enemigos;
    std::vector<RPG::AcompananteDef> acompanantes;
};

struct Cuenta {
    long combates = 0, muertes = 0;
    double pct() const { return combates ? 100.0 * static_cast<double>(muertes) / static_cast<double>(combates) : 0.0; }
};

bool g_log = false;

// D0014 (Oriol, 14/10): lo que se mide de los esbirros. Los ataques de los
// enemigos al heroe, del registro: acierta (d6), roza (el empate) o nada.
struct Medida {
    long combates = 0, ganados = 0, rondas = 0, ataques = 0, aciertos = 0, roces = 0;
    double vidaQueda = 0.0;  // fraccion de la vida del heroe, sumada en los que gana
};
Medida* g_medida = nullptr;

// Un combate. vidaInicial < 0 = entera; curas: las del sanador, que pasan de
// un combate a otro. Devuelve la vida que le queda al heroe (0 si cae).
int combate(const RPG::CharacterSheet& ficha, const Escenario& esc, uint64_t semilla, const Politica& pol,
            Cuenta& cuenta, int vidaInicial = -1, int* curas = nullptr) {
    Heroe heroe(ficha);
    if (vidaInicial >= 0) heroe.vida = std::min(heroe.max, vidaInicial);
    std::vector<std::unique_ptr<Enemigo>> cuerpos;
    std::vector<BattleParticipant> lado;
    for (std::size_t i = 0; i < esc.enemigos.size(); ++i) {
        cuerpos.push_back(std::make_unique<Enemigo>(esc.enemigos[i]));
        lado.push_back({esc.enemigos[i].nombre + " " + std::to_string(i + 1), cuerpos.back().get(), nullptr, nullptr});
    }
    auto mesa = Game::CartasEnCombate::paraHeroe(cartas(), heroe);
    if (mesa == nullptr) muere("un heroe sin cartas");
    std::vector<BattleParticipant> aliados{{"Heroe", &heroe, nullptr, nullptr}};
    std::vector<Game::Acompanante*> sanadores;
    for (const RPG::AcompananteDef& a : esc.acompanantes) {
        const int quedan = a.cura.veces > 0 && curas != nullptr ? *curas : a.cura.veces;
        auto* cuerpo = static_cast<Game::Acompanante*>(mesa->sumaAcompanante(a, quedan));
        if (cuerpo->cura()) sanadores.push_back(cuerpo);
        aliados.push_back({a.nombre, cuerpo, nullptr, nullptr});
    }
    SkillCatalog legado;
    RPG::Xoroshiro128p dado(mezcla(semilla));
    BattleState b(std::move(aliados), std::move(lado), &legado);
    b.setRandomEngine(dado);
    Game::CartasEnCombate* m = mesa.get();
    b.setMesa(std::move(mesa));
    Partida p{&b, m, {}, &heroe};
    for (auto& c : cuerpos) p.enemigos.push_back(c.get());
    int ronda = 0;
    while (b.outcome() == BattleOutcome::InProgress && ronda < 60) {
        turnoHeroe(p, pol);
        if (b.outcome() != BattleOutcome::InProgress) break;
        b.resolveEnemyTurn();
        b.tickRound();
        ++ronda;
    }
    const bool gana = b.outcome() == BattleOutcome::Victory;
    if (g_log) {
        std::printf("---- %s (PV %d, CON %d, DES %d): %s en %d rondas, le quedan %d\n", ficha.classId.c_str(), heroe.max,
                    heroe.stat(RPG::Stat::CON), heroe.stat(RPG::Stat::DES), gana ? "gana" : "cae", ronda, heroe.vida);
        for (const std::string& l : b.log()) std::printf("     %s\n", l.c_str());
    }
    if (g_medida != nullptr) {
        Medida& m = *g_medida;
        ++m.combates;
        m.rondas += ronda;
        if (gana) {
            ++m.ganados;
            m.vidaQueda += static_cast<double>(heroe.vida) / static_cast<double>(std::max(1, heroe.max));
        }
        for (const std::string& l : b.log()) {
            if (l.find(" ataca a Heroe (") == std::string::npos) continue;
            ++m.ataques;
            if (l.find("d6, ") != std::string::npos) ++m.aciertos;
            else if (l.find("roce, ") != std::string::npos) ++m.roces;
        }
    }
    if (curas != nullptr && !sanadores.empty()) *curas = sanadores.front()->curasQueQuedan();
    ++cuenta.combates;
    if (!gana) ++cuenta.muertes;  // cae o 60 rondas sin acabar
    return gana ? heroe.vida : 0;
}

const char* kClases[4] = {"el_apostata", "senor_de_las_armas", "furia_de_los_creadores",
                          "bardo_de_las_cuatro_estaciones"};
const char* kCorto[4] = {"Apostata", "Senor Armas", "Furia", "Bardo"};

const std::vector<const RPG::RaceDefinition*>& razas() {
    static const std::vector<const RPG::RaceDefinition*> r = [] {
        std::vector<const RPG::RaceDefinition*> out;
        creacion().races().forEach([&](const RPG::RaceDefinition& x) {
            if (x.playable) out.push_back(&x);
        });
        return out;
    }();
    return r;
}

const RPG::ClassDefinition& clase(int c) {
    const RPG::ClassDefinition* cls = creacion().classes().find(kClases[c]);
    if (cls == nullptr) muere(std::string("sin la clase ") + kClases[c]);
    return *cls;
}

int vidaBaseDeClase(const std::string& id) {
    const RPG::ClassDefinition* cls = creacion().classes().find(id);
    return cls != nullptr ? cls->baseHealth : 10;
}

// Las veces que cae el heroe de cada clase, sobre todas las razas jugables.
// Una serie son varios combates seguidos sin descanso.
std::array<Cuenta, 4> mideSerie(const std::vector<Escenario>& serie, int porRaza, const Equipo& eq) {
    std::array<Cuenta, 4> out{};
    for (int c = 0; c < 4; ++c) {
        const Politica pol = politicaDe(kClases[c]);
        for (std::size_t r = 0; r < razas().size(); ++r) {
            const RPG::CharacterSheet ficha = creaFicha(clase(c), *razas()[r], eq);
            for (int k = 0; k < porRaza; ++k) {
                int vida = -1, curas = 2;
                Cuenta tmp;
                for (std::size_t f = 0; f < serie.size() && vida != 0; ++f)
                    vida = combate(ficha, serie[f],
                                   (static_cast<uint64_t>(c) << 48) ^ (r << 32) ^ (static_cast<uint64_t>(k) << 8) ^ f,
                                   pol, tmp, vida, &curas);
                Cuenta& o = out[static_cast<std::size_t>(c)];
                ++o.combates;
                if (vida == 0) ++o.muertes;
            }
        }
    }
    return out;
}
std::array<Cuenta, 4> mide(const Escenario& esc, int porRaza, const Equipo& eq) { return mideSerie({esc}, porRaza, eq); }

void cabecera() {
    std::printf("  %-36s", "");
    for (const char* n : kCorto) std::printf(" %11s", n);
    std::printf("   media   (cae el heroe)\n");
}
void fila(const std::string& titulo, const std::array<Cuenta, 4>& r) {
    double media = 0;
    std::printf("  %-36s", titulo.c_str());
    for (const Cuenta& c : r) {
        std::printf(" %10.1f%%", c.pct());
        media += c.pct() / 4.0;
    }
    std::printf("  %5.1f%%\n", media);
}

RPG::AcompananteDef par(const char* id, int vida, std::array<int, 4> stats) {
    RPG::AcompananteDef a;
    a.id = a.nombre = id;
    a.vida = vida;
    a.stats = stats;
    a.ataques = 1;
    return a;
}

const Equipo kKit{"H1 kit del cofre (+2 +2 +1 +1)", 0, false};

PerfilEnemigo deObjeto(const ObjectCatalog& objetos, const std::string& id) {
    const ObjectDefinition* d = objetos.find(id);
    if (d == nullptr) muere("sin el enemigo " + id);
    return PerfilEnemigo{d->name, d->combat.maxHealth, d->combat.stats};
}

}  // namespace

int main(int argc, char** argv) {
    const std::string modo = argc > 1 ? argv[1] : "fichas";
    const int porRaza = argc > 2 ? std::max(1, std::atoi(argv[2])) : 25;
    std::printf("simulador_dificultad %s: %d combates por raza, %zu razas jugables\n", modo.c_str(), porRaza,
                razas().size());

    if (modo == "kits") {  // las piezas del kit de cada clase (para los niveles del equipo)
        creacion().classes().forEach([&](const RPG::ClassDefinition& cls) {
            const Game::KitBasico::Kit kit = Game::KitBasico::deClase(cls, equipo(), tiers(), reglasKit());
            for (const std::string& id : kit.piezas) std::printf("%s %s\n", cls.id.c_str(), id.c_str());
        });
        return 0;
    }
    if (modo == "fichas") {  // los dados y la vida de cada clase con el kit, raza a raza
        for (int c = 0; c < 4; ++c) {
            std::map<std::string, int> visto;
            for (const RPG::RaceDefinition* r : razas()) {
                const RPG::CharacterSheet f = creaFicha(clase(c), *r, kKit);
                char t[96];
                std::snprintf(t, sizeof t, "CON %d DES %d INT %d CAR %d, %2d PV", f.con(), f.des(), f.int_(), f.car(),
                              f.healthCap());
                ++visto[t];
            }
            std::printf("%s (principal %s, secundaria %s)\n", kCorto[c], clase(c).primaryStat.c_str(),
                        clase(c).secondaryStat.c_str());
            for (const auto& [t, n] : visto) std::printf("   %2d razas: %s\n", n, t.c_str());
        }
        return 0;
    }
    if (modo == "escalera") {  // la fuerza de cada escalon contra el heroe con el kit
        cabecera();
        for (const PerfilEnemigo& e : escalera())
            for (int n = 1; n <= 3; ++n) {
                Escenario esc;
                esc.enemigos.assign(static_cast<std::size_t>(n), e);
                fila("1v" + std::to_string(n) + " " + e.nombre, mide(esc, porRaza, kKit));
            }
        return 0;
    }
    const Equipo niveles[] = {kKit, {"H2 tope 3 en la clase (3 3 1 1)", 3, false},
                              {"H3 tope 4 en la clase (4 4 2 2)", 4, false},
                              {"H4 tope 4 en CON y DES (4 4 2 2)", 4, true}};
    if (modo == "equipo") {  // 1 contra 3 en cada nivel de equipo: que escalon da el 40 %
        for (const Equipo& eq : niveles) {
            std::printf("\n%s\n", eq.nombre);
            cabecera();
            for (const PerfilEnemigo& e : escalera()) {
                Escenario esc;
                esc.enemigos.assign(3, e);
                fila("1v3 " + e.nombre, mide(esc, porRaza, eq));
            }
        }
        return 0;
    }
    if (modo == "niveles") {  // cada nivel contra su escalon y el de arriba, de 1v1 a 3v3
        const int igual[] = {2, 3, 5, 7}, fuerte[] = {3, 4, 6, 8};  // indices en la escalera
        const std::array<int, 4> dadosPar[] = {{2, 2, 1, 1}, {3, 3, 1, 1}, {4, 4, 2, 2}, {4, 4, 2, 2}};
        const int vidaPar[] = {10, 12, 13, 13};
        for (int h = 0; h < 4; ++h)
            for (int f = 0; f < 2; ++f) {
                const PerfilEnemigo& e = escalera()[static_cast<std::size_t>(f == 0 ? igual[h] : fuerte[h])];
                std::printf("\n%s contra %s%s\n", niveles[h].nombre, e.nombre.c_str(), f == 0 ? "" : "  (mas fuertes)");
                cabecera();
                Escenario esc;
                for (int n = 1; n <= 3; ++n) {
                    esc.enemigos.assign(static_cast<std::size_t>(n), e);
                    fila("1 contra " + std::to_string(n), mide(esc, porRaza, niveles[h]));
                }
                esc.acompanantes = {par("par", vidaPar[h], dadosPar[h])};
                fila("2 contra 3", mide(esc, porRaza, niveles[h]));
                esc.acompanantes.push_back(par("otro", vidaPar[h], dadosPar[h]));
                fila("3 contra 3", mide(esc, porRaza, niveles[h]));
            }
        return 0;
    }
    if (modo == "razas") {  // raza a raza: 1v3 y 2v3 contra el estandar, con el kit
        Escenario solo, dos;
        solo.enemigos.assign(3, escalera()[2]);
        dos = solo;
        dos.acompanantes = {par("par", 10, {2, 2, 1, 1})};
        for (int c = 0; c < 4; ++c) {
            std::printf("%s\n", kCorto[c]);
            const Politica pol = politicaDe(kClases[c]);
            for (std::size_t r = 0; r < razas().size(); ++r) {
                const RPG::CharacterSheet f = creaFicha(clase(c), *razas()[r], kKit);
                Cuenta a, b;
                for (int k = 0; k < porRaza; ++k) {
                    combate(f, solo, (r << 32) ^ static_cast<uint64_t>(k), pol, a);
                    combate(f, dos, (r << 32) ^ static_cast<uint64_t>(k), pol, b);
                }
                std::printf("   %-28s CON %d DES %d, %2d PV   1v3 %5.1f%%   2v3 %5.1f%%\n", razas()[r]->id.c_str(), f.con(),
                            f.des(), f.healthCap(), a.pct(), b.pct());
            }
        }
        return 0;
    }
    if (modo == "ocaso" || modo == "jefes") {  // con los datos de verdad de Boundington
        ObjectCatalog objetos;
        if (!objetos.loadFromFile("assets/objects/boundington_enemigos.json").isOk()) muere("sin boundington_enemigos.json");
        if (modo == "jefes") {
            cabecera();
            for (const char* id : {"perdido_saqueador", "gorran_bandido"}) {
                Escenario esc;
                esc.enemigos = {deObjeto(objetos, id)};
                fila(std::string("1v1 ") + id, mide(esc, porRaza, kKit));
            }
            return 0;
        }
        auto aventura = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_ocaso.json");
        if (!aventura.isOk()) muere(aventura.errorMessage());
        const RPG::AcompananteDef* venides = aventura.value().acompanante("venides");
        const RPG::AcompananteDef* xila = aventura.value().acompanante("xila");
        if (venides == nullptr || xila == nullptr) muere("el Ocaso sin Venides o sin Xila");
        const PerfilEnemigo devoto = deObjeto(objetos, "perdido_devoto"), fanatico = deObjeto(objetos, "perdido_fanatico"),
                            carcelero = deObjeto(objetos, "cultista_carcelero");
        std::printf("devoto %d PV, fanatico %d PV, carcelero %d PV (dados en assets/objects/boundington_enemigos.json)\n",
                    devoto.vida, fanatico.vida, carcelero.vida);
        cabecera();
        for (int quien = 0; quien < 3; ++quien) {
            std::vector<RPG::AcompananteDef> lado;
            if (quien >= 1) lado.push_back(*venides);
            if (quien == 2) lado.push_back(*xila);
            for (int hombres : {20, 10, 0}) {
                auto esc = [&](std::vector<PerfilEnemigo> en) { return Escenario{std::move(en), lado}; };
                std::vector<Escenario> serie;
                if (hombres == 20) serie = {esc({devoto}), esc({fanatico}), esc({carcelero})};
                if (hombres == 10) serie = {esc({devoto, devoto}), esc({fanatico}), esc({carcelero})};
                if (hombres == 0) serie = {esc({devoto, devoto, devoto}), esc({fanatico, fanatico}), esc({carcelero, devoto})};
                const char* con = quien == 0 ? "heroe solo" : quien == 1 ? "con Venides" : "con Venides y Xila";
                fila(std::string(con) + (hombres == 20 ? ", 20 hombres" : hombres == 10 ? ", 10 hombres" : ", sin hombres"),
                     mideSerie(serie, porRaza, kKit));
            }
        }
        return 0;
    }
    if (modo == "esbirros") {  // D0014 (14/10): los esbirros con DES 0, 1 y 2, contra el heroe de Tier 1 con el kit
        ObjectCatalog objetos;
        if (!objetos.loadFromFile("assets/objects/boundington_enemigos.json").isOk()) muere("sin boundington_enemigos.json");
        std::printf("D0014: aciertan 30-35 %%; contra dos, unas 4 rondas y el heroe casi siempre gana; contra tres, cae ~40 %% (Oriol, 06/10)\n");
        std::printf("  %-34s %8s %8s %7s %7s %10s\n", "", "aciertan", "rozan", "rondas", "gana", "le queda");
        for (const char* id : {"perdido_saqueador", "saqueador_local"}) {
            for (int cuantos : {1, 2, 3}) {
                for (int des = 0; des <= 2; ++des) {
                    PerfilEnemigo s = deObjeto(objetos, id);
                    s.stats[1] = des;
                    Escenario esc;
                    esc.enemigos.assign(static_cast<std::size_t>(cuantos), s);
                    Medida m;
                    g_medida = &m;
                    mide(esc, porRaza, kKit);
                    g_medida = nullptr;
                    const double at = static_cast<double>(std::max(1L, m.ataques));
                    char titulo[96];
                    std::snprintf(titulo, sizeof titulo, "%dx %s (%d PV) DES %d", cuantos, id, s.vida, des);
                    std::printf("  %-34s %7.1f%% %7.1f%% %7.1f %6.1f%% %9.1f%%\n", titulo, 100.0 * m.aciertos / at,
                                100.0 * m.roces / at, static_cast<double>(m.rondas) / std::max(1L, m.combates),
                                100.0 * m.ganados / std::max(1L, m.combates),
                                100.0 * m.vidaQueda / std::max(1L, m.ganados));
                }
            }
        }
        return 0;
    }
    if (modo == "alternativas") {  // BND-ALT: los combates de las alternativas de Boundington, con el kit
        ObjectCatalog objetos;
        if (!objetos.loadFromFile("assets/objects/boundington_enemigos.json").isOk()) muere("sin boundington_enemigos.json");
        const PerfilEnemigo devoto = deObjeto(objetos, "perdido_devoto"), fanatico = deObjeto(objetos, "perdido_fanatico");
        cabecera();
        Escenario verdaderos, cripta, edul;
        verdaderos.enemigos.assign(5, devoto);
        fila("Verdaderos: 5 devotos (de antes)", mide(verdaderos, porRaza, kKit));
        edul.enemigos.assign(3, devoto);
        fila("cripta, plantar cara: 3 devotos", mide(edul, porRaza, kKit));
        // Oriol: «lo último bastante difícil». Con 4 devotos y 2 fanáticos el héroe caía el 95 %.
        cripta.enemigos = {devoto, devoto, devoto, fanatico};
        fila("cripta, acabar con todos: 3 y 1", mide(cripta, porRaza, kKit));
        Escenario barriada;  // [C01]: «un grupo de 6 saqueadores»; esbirros, para que sea un 1 contra 3
        barriada.enemigos.assign(6, deObjeto(objetos, "saqueador_local"));
        fila("Barriada: 6 saqueadores", mide(barriada, porRaza, kKit));
        return 0;
    }
    if (modo == "log") {  // un combate de cada clase contra tres estandar, con su registro
        g_log = true;
        Escenario esc;
        esc.enemigos.assign(3, escalera()[2]);
        for (int c = 0; c < 4; ++c) {
            Cuenta cu;
            combate(creaFicha(clase(c), *razas()[static_cast<std::size_t>(porRaza) % razas().size()], kKit), esc,
                    77 + static_cast<uint64_t>(c), politicaDe(kClases[c]), cu);
        }
        return 0;
    }
    muere("modo desconocido: " + modo);
}
