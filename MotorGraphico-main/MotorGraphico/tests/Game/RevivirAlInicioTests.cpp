// E-REVIVIR-01 / D0027 — Morir en un combate: el heroe revive al inicio de ese mismo combate.
//
// Lo que se prueba, todo sin ventana:
//   - el combate vuelve a empezar con todo como al empezar (heroe, mochila, oro, mana, enemigos,
//     acompanantes y sus curas), desde su primera oleada (D0030);
//   - morir no enciende flagDefeat; huir sigue encendiendo su flag y tira un dado por el oro (D0030);
//   - cada muerte suma una cicatriz (fuera de la partida guardada) y baja la moral, y repetir la
//     muerte vuelve a costar;
//   - las peleas sinMatar siguen como siempre;
//   - la regla de moral es un dato (moral.json -> actos.json).
#include "TestHarness.h"

#include "Game/Acompanante.h"
#include "Game/Cicatrices.h"
#include "Game/GameSession.h"
#include "Game/ReglasMoral.h"
#include "Game/SaveGame.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"
#include "RPG/NarrativeEngine.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

namespace {

class Cuerpo : public ICombatant {
public:
    explicit Cuerpo(int maxHealth) : m_health(maxHealth), m_maxHealth(maxHealth) {}
    RPG::CharacterSheet* character_sheet() override { return &ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return &ficha; }
    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::min(m_maxHealth, m_health + amount); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }
    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }
    RPG::CharacterSheet ficha;

private:
    int m_health;
    int m_maxHealth;
};

ObjectCatalog catalogo() {
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({
        "objects": [
            {"id": "llop", "name": "Llop", "category": "enemy",
             "combat": {"maxHealth": 8, "maxMana": 0, "skills": []}}
        ]
    })")
                    .isOk());
    return objects;
}

LevelDefinition nivel() {
    LevelDefinition level;
    level.name = "Plaza";
    level.playerStart = GridCoord{0, 0};
    return level;
}

const char* kRutaCicatrices = "test_revivir_tmp/cicatrices.txt";

struct Partida {
    ObjectCatalog objetos = catalogo();
    SkillCatalog habilidades;
    Cuerpo heroe{40};
    SkillSet aprendidas{10};
    std::vector<std::string> mochila{"pocion", "pocion"};
    RPG::NarrativeEngine motor;
    RPG::NarrativeState trama;
    GameSession sesion;

    explicit Partida(LevelDefinition level = nivel(), const RPG::AdventureScript* aventura = nullptr)
        : sesion(nullptr, &objetos, &habilidades, level, &heroe, &aprendidas, &mochila) {
        heroe.ficha.id = "sulivan";
        if (aventura != nullptr) motor.setAdventure(aventura);
        sesion.setNarrative(&motor, &trama);
        std::remove(kRutaCicatrices);
        sesion.ponRutaCicatrices(kRutaCicatrices);
    }

    BattleState& combate() { return *sesion.battle(); }
    // El heroe cae: le quitan toda la vida y la ronda se cierra (ahi se mira el desenlace).
    void caer() {
        heroe.takeDamage(1000);
        combate().tickRound();
    }
    // Cae y la sesion cierra el combate (y, si toca, lo vuelve a empezar).
    void caeYSeResuelve() {
        MOTOR_CHECK(sesion.battle() != nullptr);
        if (sesion.battle() == nullptr) return;
        caer();
        MOTOR_CHECK(combate().outcome() == BattleOutcome::Defeat);
        MOTOR_CHECK(sesion.syncBattleOutcome());
    }
    // Gana el combate en curso (los enemigos caen) y la sesion lo cierra.
    void ganaYSeResuelve() {
        for (const BattleParticipant& e : combate().enemies()) e.combatant->takeDamage(1000);
        combate().tickRound();
        MOTOR_CHECK(combate().outcome() == BattleOutcome::Victory);
        MOTOR_CHECK(sesion.syncBattleOutcome());
    }
    int enRegistro(const std::string& parte) const {
        int n = 0;
        for (const std::string& l : sesion.log()) n += l.find(parte) != std::string::npos ? 1 : 0;
        return n;
    }
    ~Partida() { std::remove(kRutaCicatrices); }
};

}  // namespace

MOTOR_TEST_CASE(morir_reinicia_el_combate_con_todo_como_al_empezar) {
    Partida p;
    p.heroe.takeDamage(10);  // llega con 30 de 40
    p.sesion.addGold(25);
    p.aprendidas.ponMana(7);
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d"});
    MOTOR_CHECK(p.sesion.mode() == GameMode::Battle && p.sesion.battle() != nullptr);
    if (p.sesion.battle() == nullptr) return;
    const BattleState* primero = p.sesion.battle();
    // A mitad del combate: el heroe gasta una pocion, el oro cambia, se queda sin mana y le pega al lobo.
    p.mochila.pop_back();
    p.sesion.addGold(100);
    p.aprendidas.ponMana(0);
    p.combate().enemies()[0].combatant->takeDamage(5);
    MOTOR_CHECK(p.combate().enemies()[0].combatant->health() == 3);
    p.caeYSeResuelve();
    // Sigue el combate (uno nuevo), el mismo, y todo como al empezar.
    MOTOR_CHECK(p.sesion.mode() == GameMode::Battle);
    MOTOR_CHECK(p.sesion.battle() != nullptr && p.sesion.battle() != primero);
    if (p.sesion.battle() == nullptr) return;
    MOTOR_CHECK(p.sesion.battle()->outcome() == BattleOutcome::InProgress && p.sesion.battle()->sePuedeHuir());
    MOTOR_CHECK(p.heroe.health() == 30 && p.heroe.isAlive());
    MOTOR_CHECK(p.mochila == (std::vector<std::string>{"pocion", "pocion"}));
    MOTOR_CHECK(p.sesion.gold() == 25);
    MOTOR_CHECK(p.aprendidas.mana() == 7);
    MOTOR_CHECK(p.sesion.battle()->enemies().size() == 1 && p.sesion.enemies().size() == 1);
    MOTOR_CHECK(p.sesion.battle()->enemies()[0].combatant->health() == 8);
    MOTOR_CHECK(p.sesion.battle()->enemies()[0].name == "Llop");
}

MOTOR_TEST_CASE(reiniciar_vuelve_a_la_primera_oleada_y_no_a_donde_se_cayo) {
    Partida p;
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v1", .flagDefeat = "d1"});
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v2", .flagDefeat = "d2"});  // encolada
    p.sesion.addGold(50);  // lo de despues de la foto se pierde
    p.heroe.takeDamage(10);  // la primera oleada le cuesta 10 PV
    p.ganaYSeResuelve();
    MOTOR_CHECK(p.trama.hasFlag("v1") && !p.trama.hasFlag("v2"));
    MOTOR_CHECK(p.sesion.mode() == GameMode::Battle && p.sesion.battle() != nullptr);
    if (p.sesion.battle() == nullptr) return;
    MOTOR_CHECK(p.heroe.health() == 30);  // entra en la segunda con 30
    p.caeYSeResuelve();
    // Vuelve a la PRIMERA: con los 40 PV del principio, sin el oro ni la flag de haberla ganado.
    MOTOR_CHECK(p.sesion.mode() == GameMode::Battle && p.sesion.battle() != nullptr);
    MOTOR_CHECK(p.heroe.health() == 40);
    MOTOR_CHECK(p.sesion.gold() == 0);
    MOTOR_CHECK(p.sesion.enemies().size() == 1);
    MOTOR_CHECK(!p.trama.hasFlag("v1") && !p.trama.hasFlag("v2") && !p.trama.hasFlag("d1") && !p.trama.hasFlag("d2"));
    // Y las dos oleadas siguen por delante, una vez cada una; caer otra vez en la segunda vuelve a la primera.
    p.ganaYSeResuelve();
    MOTOR_CHECK(p.trama.hasFlag("v1") && !p.trama.hasFlag("v2") && p.sesion.battle() != nullptr);
    if (p.sesion.battle() == nullptr) return;
    p.caeYSeResuelve();
    MOTOR_CHECK(!p.trama.hasFlag("v1"));
    p.ganaYSeResuelve();
    p.ganaYSeResuelve();
    MOTOR_CHECK(p.trama.hasFlag("v1") && p.trama.hasFlag("v2"));
    MOTOR_CHECK(p.sesion.battle() == nullptr && p.sesion.mode() == GameMode::Exploration);
    MOTOR_CHECK(p.sesion.cicatrices() == 2);
    // Un combate posterior hace su propia foto.
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v3", .flagDefeat = "d3"});
    p.heroe.takeDamage(5);
    const int vida = p.heroe.health();
    p.caeYSeResuelve();
    MOTOR_CHECK(p.heroe.health() == vida + 5 && p.trama.hasFlag("v1") && p.trama.hasFlag("v2"));
}

// D0030 — Al huir se tira un dado por lo que se cae en la carrera.
MOTOR_TEST_CASE(huir_tira_un_dado_y_puede_costar_oro) {
    struct DadoFijo : RPG::RandomEngine {
        int cara = 1;
        int veces = 0;
        int roll_d6() override { ++veces; return cara; }
        int roll_in_range(int a, int /*b*/) override { return a; }
        uint64_t seed() const override { return 0; }
    };
    const struct { int cara, oro, queda; } casos[] = {{1, 100, 70}, {2, 100, 80}, {3, 100, 90}, {4, 100, 100}, {6, 100, 100}, {1, 1, 0}};
    for (const auto& c : casos) {
        Partida p;
        DadoFijo dado;
        dado.cara = c.cara;
        p.sesion.setRandomEngine(dado);
        p.sesion.addGold(c.oro);
        p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d", .flagHuida = "h"});
        BattleAction correr;
        correr.type = BattleActionType::Flee;
        p.combate().resolveAllyAction(0, correr);
        MOTOR_CHECK(p.sesion.syncBattleOutcome());
        MOTOR_CHECK(p.sesion.gold() == c.queda);
        MOTOR_CHECK(p.enRegistro("Tiras un dado al huir") == 1);
    }
    // Sin oro no hay tirada.
    Partida p;
    DadoFijo dado;
    p.sesion.setRandomEngine(dado);
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d", .flagHuida = "h"});
    BattleAction correr;
    correr.type = BattleActionType::Flee;
    p.combate().resolveAllyAction(0, correr);
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    MOTOR_CHECK(dado.veces == 0 && p.enRegistro("Tiras un dado al huir") == 0);
}

// D0030 — Huir no da nivel ni hito: ni el final de la huida ni la del tutorial.
MOTOR_TEST_CASE(las_huidas_no_dan_recompensa) {
    auto ocaso = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_ocaso.json");
    auto dia1 = RPG::AdventureScript::loadFromFile("assets/adventures/boundington_primer_dia.json");
    MOTOR_CHECK(ocaso.isOk() && dia1.isOk());
    if (!ocaso.isOk() || !dia1.isOk()) return;
    const struct { const RPG::AdventureScript* guion; const char* beat; } huidas[] = {
        {&ocaso.value(), "b3_huida_perdida"}, {&ocaso.value(), "b3_rodeo_huida"}, {&dia1.value(), "tut_huida"}};
    for (const auto& h : huidas) {
        bool visto = false;
        for (const RPG::NarrativeBeat& beat : h.guion->beats) {
            if (beat.id != h.beat) continue;
            visto = true;
            for (const RPG::NarrativeEffect& e : beat.effects)
                MOTOR_CHECK(e.type != RPG::EffectType::GrantLevel && e.type != RPG::EffectType::GrantMilestone && e.type != RPG::EffectType::GrantGold);
        }
        MOTOR_CHECK(visto);
    }
}

MOTOR_TEST_CASE(los_acompanantes_y_sus_curas_vuelven_como_al_empezar) {
    const auto aventura = RPG::AdventureScript::loadFromString(R"({
      "id": "caer",
      "acompanantes": [{"id": "xila", "nombre": "Xila", "vida": 6, "stats": {"DES": 2}, "atacable": false,
                        "cura": {"dado": 4, "mas": 1, "veces": 2, "umbral": 0.34}}],
      "beats": [{"id": "pelea", "trigger": {"type": "auto"}, "lines": ["."],
        "effects": [{"type": "startBattle", "startBattle": {"monsterId": "llop", "flagVictory": "v", "flagDefeat": "d",
          "acompanantes": [{"id": "xila"}]}}]}]})");
    MOTOR_CHECK(aventura.isOk());
    if (!aventura.isOk()) return;
    Partida p(nivel(), &aventura.value());
    p.sesion.startNarrativeBattle(aventura.value().beats[0].effects[0].battle);
    MOTOR_CHECK(p.sesion.battle() != nullptr && p.sesion.battle()->allies().size() == 2);
    if (p.sesion.battle() == nullptr || p.sesion.battle()->allies().size() != 2) return;
    auto* xila = dynamic_cast<Game::Acompanante*>(p.combate().allies()[1].combatant);
    MOTOR_CHECK(xila != nullptr && xila->curasQueQuedan() == 2);
    if (xila == nullptr) return;
    MOTOR_CHECK(xila->gastaCura() && xila->gastaCura() && xila->curasQueQuedan() == 0);
    xila->takeDamage(4);
    p.caeYSeResuelve();
    MOTOR_CHECK(p.sesion.battle() != nullptr && p.sesion.battle()->allies().size() == 2);
    if (p.sesion.battle() == nullptr || p.sesion.battle()->allies().size() != 2) return;
    const auto* otra = dynamic_cast<const Game::Acompanante*>(p.sesion.battle()->allies()[1].combatant);
    MOTOR_CHECK(otra != nullptr && otra != xila && otra->curasQueQuedan() == 2 && otra->health() == 6);
    // Las curas gastadas en un intento que se pierde no cuentan para la aventura.
    MOTOR_CHECK(p.trama.variable(Game::Acompanante::claveCuras("caer", "xila")) == 0);
}

MOTOR_TEST_CASE(morir_no_enciende_la_flag_de_derrota_pero_huir_si_enciende_la_suya) {
    Partida p;
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d", .flagHuida = "h"});
    p.caeYSeResuelve();
    p.caeYSeResuelve();
    MOTOR_CHECK(!p.trama.hasFlag("d") && !p.trama.hasFlag("h") && !p.trama.hasFlag("v"));
    MOTOR_CHECK(p.sesion.mode() == GameMode::Battle && p.sesion.enemies().size() == 1);
    // Huir, en cambio, sigue encendiendo la suya (CAER-01), aunque se haya muerto antes.
    if (p.sesion.battle() == nullptr) return;
    BattleAction correr;
    correr.type = BattleActionType::Flee;
    p.combate().resolveAllyAction(0, correr);
    MOTOR_CHECK(p.combate().outcome() == BattleOutcome::Fled);
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    MOTOR_CHECK(p.trama.hasFlag("h") && !p.trama.hasFlag("d"));
    MOTOR_CHECK(p.sesion.mode() == GameMode::Exploration && p.sesion.enemies().size() == 1);
}

MOTOR_TEST_CASE(sin_flag_de_huida_huir_enciende_la_de_derrota_como_siempre) {
    Partida p;
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d"});
    p.caeYSeResuelve();
    MOTOR_CHECK(!p.trama.hasFlag("d"));
    if (p.sesion.battle() == nullptr) return;
    BattleAction correr;
    correr.type = BattleActionType::Flee;
    p.combate().resolveAllyAction(0, correr);
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    MOTOR_CHECK(p.trama.hasFlag("d") && !p.trama.hasFlag("v"));
}

MOTOR_TEST_CASE(cada_muerte_suma_una_cicatriz_que_no_va_en_la_partida_guardada) {
    Partida p;
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d"});
    MOTOR_CHECK(p.sesion.cicatrices() == 0 && p.sesion.muertes() == 0);
    p.caeYSeResuelve();
    MOTOR_CHECK(p.sesion.cicatrices() == 1 && p.sesion.muertes() == 1);
    MOTOR_CHECK(Game::Cicatrices::carga(kRutaCicatrices).de("sulivan") == 1);
    MOTOR_CHECK(p.enRegistro("Has caído derrotado.") == 1 && p.enRegistro(Game::Cicatrices::nota(1)) == 1);
    // Y el jugador, que en combate lee el registro de la batalla, tambien lo ve ahi.
    MOTOR_CHECK(p.sesion.battle() != nullptr);
    if (p.sesion.battle() != nullptr) {
        const auto& registro = p.sesion.battle()->log();
        const auto esta = [&](const std::string& l) { return std::find(registro.begin(), registro.end(), l) != registro.end(); };
        MOTOR_CHECK(esta("Has caído derrotado.") && esta(Game::Cicatrices::nota(1)) && esta("Vuelves al inicio del combate."));
    }
    p.caeYSeResuelve();
    p.caeYSeResuelve();
    MOTOR_CHECK(p.sesion.cicatrices() == 3 && p.sesion.muertes() == 3);
    MOTOR_CHECK(Game::Cicatrices::carga(kRutaCicatrices).de("sulivan") == 3);
    MOTOR_CHECK(p.enRegistro(Game::Cicatrices::nota(3)) == 1);
    // Fuera de la partida guardada: el guardado no sabe de ellas (viven en su fichero).
    const SaveGameData foto = p.sesion.captureSaveGame("niveles/plaza.json");
    MOTOR_CHECK(serializeSaveGame(foto).find("cicatri") == std::string::npos);
}

MOTOR_TEST_CASE(las_cicatrices_se_leen_del_fichero_al_empezar_la_sesion) {
    Partida p;
    Game::Cicatrices antes;
    antes.suma("sulivan");
    antes.suma("sulivan");
    MOTOR_CHECK(antes.guarda(kRutaCicatrices));
    p.sesion.ponRutaCicatrices(kRutaCicatrices);
    MOTOR_CHECK(p.sesion.cicatrices() == 2);
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d"});
    p.caeYSeResuelve();
    MOTOR_CHECK(p.sesion.cicatrices() == 3 && Game::Cicatrices::carga(kRutaCicatrices).de("sulivan") == 3);
}

MOTOR_TEST_CASE(cada_muerte_baja_la_moral_y_repetirla_vuelve_a_costar) {
    Partida p;
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d"});
    MOTOR_CHECK(p.sesion.morality().pendientes().empty() && p.sesion.morality().value() == 0);
    p.caeYSeResuelve();
    MOTOR_CHECK(p.sesion.morality().pendientes().size() == 1);
    MOTOR_CHECK(p.sesion.morality().pendientes()[0].id == "heroe_cae_en_combate");
    MOTOR_CHECK(p.sesion.morality().pendientes()[0].gravedad == moral::Gravedad::Menor);
    MOTOR_CHECK(p.enRegistro("Caíste en combate") >= 1);
    p.sesion.morality().asienta(moral::reglas().parametros, 1);
    const int trasUna = p.sesion.morality().value();
    MOTOR_CHECK(trasUna < 0);
    // Otra muerte la misma noche: otro acto, y baja otra vez (el segundo del dia pesa la mitad).
    p.caeYSeResuelve();
    MOTOR_CHECK(p.sesion.morality().pendientes().size() == 1);
    p.sesion.morality().asienta(moral::reglas().parametros, 1);
    const int trasDos = p.sesion.morality().value();
    MOTOR_CHECK(trasDos < trasUna);
    // Y la gente lo ve: la reputacion con la ciudad baja con cada muerte.
    MOTOR_CHECK(p.heroe.ficha.factionRep["aegroum"] == -4);
}

MOTOR_TEST_CASE(una_pelea_sin_matar_sigue_como_siempre) {
    Partida p;
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d", .sinMatar = true});
    MOTOR_CHECK(p.sesion.battle() != nullptr);
    if (p.sesion.battle() == nullptr) return;
    p.heroe.takeDamage(30);
    p.caer();
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    // Te levantas con un cuarto de la vida, sigue la historia por su flag de derrota, sin cicatriz ni moral.
    MOTOR_CHECK(p.sesion.mode() == GameMode::Exploration && p.sesion.battle() == nullptr);
    MOTOR_CHECK(p.trama.hasFlag("d") && !p.trama.hasFlag("v"));
    MOTOR_CHECK(p.heroe.health() == 10);
    MOTOR_CHECK(p.sesion.enemies().empty());
    MOTOR_CHECK(p.sesion.cicatrices() == 0 && p.sesion.muertes() == 0);
    MOTOR_CHECK(p.sesion.morality().pendientes().empty());
    MOTOR_CHECK(Game::Cicatrices::carga(kRutaCicatrices).de("sulivan") == 0);
}

MOTOR_TEST_CASE(un_combate_ineludible_y_uno_del_mapa_tambien_reinician) {
    Partida p;
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d", .ineludible = true});
    p.caeYSeResuelve();
    MOTOR_CHECK(p.sesion.mode() == GameMode::Battle && p.sesion.battle() != nullptr);
    if (p.sesion.battle() == nullptr) return;
    MOTOR_CHECK(!p.sesion.battle()->sePuedeHuir());  // sigue siendo ineludible
    MOTOR_CHECK(!p.trama.hasFlag("d"));

    LevelDefinition level = nivel();
    ObjectSpawn lobo;
    lobo.objectId = "llop";
    lobo.position = GridCoord{1, 0};
    lobo.patrolMin = GridCoord{1, 0};
    lobo.patrolMax = GridCoord{1, 0};
    level.objects.push_back(lobo);
    Partida q(level);
    MOTOR_CHECK(!q.sesion.tryMovePlayer(1, 0));
    MOTOR_CHECK(q.sesion.mode() == GameMode::Battle);
    q.caeYSeResuelve();
    MOTOR_CHECK(q.sesion.mode() == GameMode::Battle && q.sesion.battle() != nullptr && q.sesion.battle()->sePuedeHuir());
    MOTOR_CHECK(q.sesion.enemies().size() == 1 && q.sesion.cicatrices() == 1);
}

// D0031 — Un combate ineludible no se puede huir, y al morir se reinicia salvo que salte su rescate.
MOTOR_TEST_CASE(el_rescate_al_morir_cierra_el_combate_solo_si_su_beat_puede_saltar) {
    const auto aventura = RPG::AdventureScript::loadFromString(R"({
        "id": "prueba", "name": "Prueba",
        "beats": [
            {"id": "rescate", "trigger": {"type": "auto"},
             "requires": {"allFlags": ["d", "con_amigos"], "notFlags": ["rescate_visto"]},
             "speaker": "", "lines": ["Llegan los tuyos."],
             "effects": [{"type": "setFlag", "arg": "rescate_visto"}]}
        ]
    })");
    MOTOR_CHECK(aventura.isOk());
    if (!aventura.isOk()) return;
    const RPG::BattleRequest pelea{.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d", .ineludible = true, .rescateAlMorir = "rescate"};
    {   // Sin los requisitos: se reinicia, con cicatriz, y la flag de derrota sigue apagada.
        Partida p(nivel(), &aventura.value());
        p.sesion.startNarrativeBattle(pelea);
        p.caeYSeResuelve();
        MOTOR_CHECK(p.sesion.mode() == GameMode::Battle && p.sesion.cicatrices() == 1);
        MOTOR_CHECK(!p.trama.hasFlag("d") && !p.trama.hasFlag("rescate_visto"));
    }
    {   // Con los requisitos: el combate se cierra, sin cicatriz, y salta el rescate.
        Partida p(nivel(), &aventura.value());
        p.trama.setFlag("con_amigos");
        p.sesion.startNarrativeBattle(pelea);
        p.caeYSeResuelve();
        MOTOR_CHECK(p.sesion.battle() == nullptr && p.sesion.mode() == GameMode::Dialogue);
        MOTOR_CHECK(p.sesion.cicatrices() == 0 && p.heroe.health() == 10);
        MOTOR_CHECK(p.trama.hasFlag("d") && p.trama.hasFlag("rescate_visto"));
    }
}

// D0031 — Los combates importantes de la historia: de los caps, los rescates y la Barriada no se huye.
MOTOR_TEST_CASE(de_los_combates_importantes_no_se_huye) {
    int ineludibles = 0, conRescate = 0;
    for (const char* ruta : {"assets/adventures/boundington_ocaso.json", "assets/adventures/boundington_dia_gorran.json",
                             "assets/adventures/boundington_segundo_dia.json"}) {
        const auto guion = RPG::AdventureScript::loadFromFile(ruta);
        MOTOR_CHECK(guion.isOk());
        if (!guion.isOk()) continue;
        for (const RPG::NarrativeBeat& beat : guion.value().beats)
            for (const RPG::NarrativeEffect& e : beat.effects) {
                if (e.type != RPG::EffectType::StartBattle) continue;
                const std::string& d = e.battle.flagDefeat;
                const bool importante = e.battle.monsterId == "sandes" || e.battle.monsterId == "diacono_orfeu" || e.battle.monsterId == "edul_furioso" ||
                                        d == "bnd3_verina_muerta" || d == "bnd3_edul_perdido" || d == "bnd3_venides_perdido" || d == "bnd2_saqueadores_ganan";
                MOTOR_CHECK(e.battle.ineludible == importante);
                ineludibles += e.battle.ineludible ? 1 : 0;
                conRescate += e.battle.rescateAlMorir == "b3_compania" ? 1 : 0;
            }
    }
    MOTOR_CHECK(ineludibles >= 1 && conRescate >= 1);
}

MOTOR_TEST_CASE(la_regla_de_moral_de_la_muerte_es_un_dato) {
    const moral::Reglas& r = moral::reglas();
    MOTOR_CHECK(r.parametros.actoAlMorir == "heroe_cae_en_combate");
    const moral::DefinicionActo* acto = r.acto(r.parametros.actoAlMorir);
    MOTOR_CHECK(acto != nullptr);
    if (acto == nullptr) return;
    MOTOR_CHECK(acto->gravedad == moral::Gravedad::Menor && acto->signo == -1 && !acto->motivo.empty());
    // Un moral.json que apunta a un acto que no existe es un error, no un acto que no hace nada.
    const auto mala = moral::Reglas::lee(R"({"alMorir": "no_existe"})", R"({"actos": []})");
    MOTOR_CHECK(!mala.isOk());
}

int main() { return motor_test::runAllTests(); }
