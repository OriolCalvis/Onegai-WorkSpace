// FASE1-BND-01 — Lo que faltaba de la fase 1 de «Los Perdidos de Boundington
// — revisión para jugarla entera» («El motor»): las franjas del día, la tabla
// aleatoria (el 1d4 del traje roto), los descansos (la comida con la familia de
// Pico Dragón), huir a 0 PV (Sandes) y pelear sin matar (la pelea de taberna).
//
// Se prueba: las cuentas del dia y la franja; la lectura (lo que vale y lo que
// no, con su error); el motor (pasarFranja pone las variables y deja la linea
// del registro, y los beats las piden como cualquier variable); la escritura de
// vuelta (idempotente); la validacion (la franja la escribe pasarFranja y las
// flags de la tabla cuentan como escritas); la sesion (la tabla enciende la flag
// de la cara, el descanso cura con la regla del reglamento, huir y sin matar
// cambian lo que se cuenta al ganar); y el rotulo del HUD. GL-free.

#include "TestHarness.h"

#include "Game/GameSession.h"
#include "HUD/Pergamino/HudRetales.h"
#include "Level/ObjectCatalog.h"
#include "RPG/AdventureSerializer.h"
#include "RPG/AdventureValidation.h"
#include "RPG/Narrative/FranjasDelDia.h"
#include "RPG/NarrativeEngine.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace {

namespace fr = RPG::franjas;

// Un dia en Boundington, en pequeño: Xila solo habla sobrio por la mañana; la
// pelea es de noche y sin matar; el traje se rompe con 1d4; la familia da un
// descanso corto y la cama, uno largo.
const char* kAventura = R"({
  "id": "prueba_fase_uno",
  "beats": [
    {"id": "llegada", "trigger": {"type": "auto"}, "requires": {"notFlags": ["llegado"]},
     "lines": ["Boundington."],
     "effects": [{"type": "setFlag", "arg": "llegado"}, {"type": "setVariable", "arg": "dia", "value": 1}]},
    {"id": "xila_sobrio", "trigger": {"type": "talk", "target": "xila"}, "requires": {"variables": {"franja": 0}},
     "lines": ["Por la mañana hablo."], "effects": [{"type": "pasarFranja"}]},
    {"id": "xila_borracho", "trigger": {"type": "talk", "target": "xila"}, "lines": ["..."]},
    {"id": "a_la_noche", "trigger": {"type": "talk", "target": "skilla"}, "requires": {"variables": {"franja": {"max": 1}}},
     "lines": ["Vuelve de noche."], "effects": [{"type": "pasarFranja", "arg": "noche"}]},
    {"id": "dormir", "trigger": {"type": "talk", "target": "skilla"}, "requires": {"variables": {"franja": 2}},
     "lines": ["Sube."], "effects": [{"type": "pasarFranja", "arg": "manana"}, {"type": "descanso", "arg": "largo"}]},
    {"id": "traje", "trigger": {"type": "talk", "target": "muro"}, "lines": ["Trepas."],
     "effects": [{"type": "tablaAleatoria",
                  "tabla": {"dado": 4, "que": "El traje", "flags": ["roto_culo", "roto_axilas", "roto_espalda", "roto_poco"]}}]},
    {"id": "comida", "trigger": {"type": "talk", "target": "padre"}, "lines": ["Siéntate."],
     "effects": [{"type": "descanso", "arg": "corto"}]},
    {"id": "pelea", "trigger": {"type": "talk", "target": "camorrista"}, "lines": ["¡A palos!"],
     "effects": [{"type": "startBattle", "startBattle": {"monsterId": "camorrista",
                  "enemigos": [{"id": "camorrista", "cantidad": 1}, {"id": "borracho", "cantidad": 2}],
                  "sinMatar": true, "flagVictory": "pelea_ganada", "flagDefeat": "pelea_perdida"}}]},
    {"id": "sandes", "trigger": {"type": "talk", "target": "sandes"}, "lines": ["No me vais a coger."],
     "effects": [{"type": "startBattle", "startBattle": {"monsterId": "sandes",
                  "enemigos": [{"id": "sandes", "cantidad": 1}, {"id": "borracho", "cantidad": 1}],
                  "huyen": ["sandes"], "flagVictory": "sandes_huido", "flagDefeat": "sandes_gana"}}]}
  ]
})";

RPG::AdventureScript carga(const char* texto = kAventura) {
    auto r = RPG::AdventureScript::loadFromString(texto);
    MOTOR_CHECK(r.isOk());
    return r.isOk() ? r.value() : RPG::AdventureScript{};
}

std::string conEfecto(const std::string& efecto) {
    return R"({"id": "x", "beats": [{"id": "a", "trigger": {"type": "auto"}, "lines": ["?"], "effects": [)" + efecto +
           "]}]}";
}

bool carga_mal(const std::string& efecto) { return !RPG::AdventureScript::loadFromString(conEfecto(efecto)).isOk(); }

// El dado que sale, a mano (las tablas y los descansos tiran con roll_in_range).
class DadoFijo : public RPG::RandomEngine {
public:
    explicit DadoFijo(int cara) : m_cara(cara) {}
    int roll_d6() override { return std::clamp(m_cara, 1, 6); }
    int roll_in_range(int a, int b) override { return std::clamp(m_cara, a, b); }
    uint64_t seed() const override { return 0; }
    int m_cara;
};

class Cuerpo : public ICombatant {
public:
    explicit Cuerpo(int vida) : m_vida(vida), m_max(vida) {}
    void takeDamage(int n) override { m_vida = std::max(0, m_vida - n); }
    void heal(int n) override { m_vida = std::min(m_max, m_vida + n); }
    int health() const override { return m_vida; }
    int maxHealth() const override { return m_max; }
    bool isAlive() const override { return m_vida > 0; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return m_tier; }
    std::string combatant_id() const override { return "heroe"; }
    int m_tier = 1;

private:
    int m_vida, m_max;
};

ObjectCatalog catalogo() {
    ObjectCatalog objetos;
    MOTOR_CHECK(objetos.loadFromString(R"({"objects": [
        {"id": "camorrista", "name": "Camorrista", "category": "enemy", "combat": {"maxHealth": 6, "maxMana": 0, "skills": []}},
        {"id": "borracho", "name": "Borracho", "category": "enemy", "combat": {"maxHealth": 4, "maxMana": 0, "skills": []}},
        {"id": "sandes", "name": "Sandes", "category": "enemy", "combat": {"maxHealth": 30, "maxMana": 0, "skills": []}},
        {"id": "pocion", "name": "Poción", "category": "pickup", "pickup": {"effect": "heal", "power": 15}}
    ]})").isOk());
    return objetos;
}

struct Partida {
    RPG::AdventureScript aventura = carga();
    RPG::NarrativeEngine motor;
    RPG::NarrativeState trama;
    ObjectCatalog objetos = catalogo();
    SkillCatalog habilidades;
    SkillSet aprendidas{10};
    std::vector<std::string> mochila;
    Cuerpo heroe{40};
    DadoFijo dado{3};
    GameSession sesion;

    static LevelDefinition nivel() {
        LevelDefinition l;
        l.name = "Taberna";
        l.playerStart = GridCoord{0, 0};
        return l;
    }
    Partida() : sesion(nullptr, &objetos, &habilidades, nivel(), &heroe, &aprendidas, &mochila) {
        motor.setAdventure(&aventura);
        trama.setFlag("llegado");  // sin la llegada: cada cierre no abre otro dialogo
        sesion.setNarrative(&motor, &trama);
        sesion.setRandomEngine(dado);
    }
    bool enRegistro(const std::string& linea) const {
        const auto& log = sesion.log();
        return std::find(log.begin(), log.end(), linea) != log.end();
    }
    // Todos los enemigos a 0 PV menos el primero, que se queda con 1 y lo tumba
    // el heroe (el desenlace solo se mira tras una accion).
    void tumbaATodos() {
        BattleState* b = sesion.battle();
        const std::vector<BattleParticipant>& enemigos = b->enemies();
        for (std::size_t i = 1; i < enemigos.size(); ++i) enemigos[i].combatant->takeDamage(1000);
        enemigos[0].combatant->takeDamage(enemigos[0].combatant->health() - 1);
        for (int i = 0; i < 500 && b->outcome() == BattleOutcome::InProgress; ++i)
            b->resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0});
    }
};

}  // namespace

MOTOR_TEST_CASE(las_cuentas_del_dia_y_la_franja) {
    MOTOR_CHECK(fr::indice("manana") == 0 && fr::indice("tarde") == 1 && fr::indice("noche") == 2);
    MOTOR_CHECK(fr::indice("mañana") == -1 && fr::indice("") == -1);  // en el JSON, sin tilde, como los ids
    MOTOR_CHECK(std::string(fr::clave(2)) == "noche" && std::string(fr::clave(3)).empty());
    MOTOR_CHECK(std::string(fr::nombre(0)) == "Mañana");
    // Despues de la noche, la mañana del dia siguiente.
    const fr::Momento m = fr::pasa({1, 2}, 1);
    MOTOR_CHECK(m.dia == 2 && m.franja == 0);
    MOTOR_CHECK(fr::pasa({1, 0}, 5).dia == 2 && fr::pasa({1, 0}, 5).franja == 2);
    // Si la cuenta no habia empezado, empieza el dia 1.
    MOTOR_CHECK(fr::pasa({0, 0}, 1).dia == 1 && fr::pasa({0, 0}, 1).franja == 1);
    // Hasta: la siguiente vez que sea esa franja; si ya lo es, nada.
    MOTOR_CHECK(fr::hasta({1, 2}, 0).dia == 2 && fr::hasta({1, 2}, 0).franja == 0);  // dormir
    MOTOR_CHECK(fr::hasta({1, 0}, 2).dia == 1 && fr::hasta({1, 0}, 2).franja == 2);
    MOTOR_CHECK(fr::hasta({1, 1}, 1).dia == 1 && fr::hasta({1, 1}, 1).franja == 1);
    // Lo que se lee.
    MOTOR_CHECK(fr::rotulo({0, 0}).empty());
    MOTOR_CHECK(fr::rotulo({2, 1}) == "Día 2 · Tarde");
    MOTOR_CHECK(fr::lineaRegistro({1, 0}, {1, 1}) == "Llega la tarde.");
    MOTOR_CHECK(fr::lineaRegistro({1, 1}, {1, 2}) == "Anochece.");
    MOTOR_CHECK(fr::lineaRegistro({1, 2}, {2, 0}) == "Amanece el día 2.");
    MOTOR_CHECK(fr::lineaRegistro({1, 2}, {2, 1}) == "Amanece el día 2. Llega la tarde.");
    MOTOR_CHECK(fr::lineaRegistro({0, 0}, {1, 1}) == "Llega la tarde.");
    MOTOR_CHECK(fr::lineaRegistro({1, 1}, {1, 1}).empty());
}

MOTOR_TEST_CASE(se_leen_los_efectos_nuevos_y_lo_que_no_vale_da_error) {
    const RPG::AdventureScript a = carga();
    MOTOR_CHECK(a.beats.size() == 9);
    MOTOR_CHECK(a.beats[1].effects[0].type == RPG::EffectType::PasarFranja && a.beats[1].effects[0].value == 1);
    MOTOR_CHECK(a.beats[3].effects[0].arg == "noche");
    MOTOR_CHECK(a.beats[4].effects[1].type == RPG::EffectType::Descanso && a.beats[4].effects[1].arg == "largo");
    const RPG::TablaAleatoriaRequest& t = a.beats[5].effects[0].tabla;
    MOTOR_CHECK(t.dado == 4 && t.flags.size() == 4 && t.flags[3] == "roto_poco" && t.que == "El traje");
    MOTOR_CHECK(a.beats[7].effects[0].battle.sinMatar && a.beats[7].effects[0].battle.huyen.empty());
    MOTOR_CHECK(a.beats[8].effects[0].battle.huyen == std::vector<std::string>{"sandes"});

    MOTOR_CHECK(!carga_mal(R"({"type": "pasarFranja", "value": 3})"));
    MOTOR_CHECK(carga_mal(R"({"type": "pasarFranja", "arg": "mediodia"})"));
    MOTOR_CHECK(carga_mal(R"({"type": "pasarFranja", "value": 0})"));
    MOTOR_CHECK(carga_mal(R"({"type": "pasarFranja", "value": 10})"));
    MOTOR_CHECK(carga_mal(R"({"type": "descanso"})"));
    MOTOR_CHECK(carga_mal(R"({"type": "descanso", "arg": "siesta"})"));
    MOTOR_CHECK(carga_mal(R"({"type": "tablaAleatoria", "tabla": {"dado": 4, "flags": ["a", "b"]}})"));
    MOTOR_CHECK(carga_mal(R"({"type": "tablaAleatoria", "tabla": {"dado": 1, "flags": ["a"]}})"));
    MOTOR_CHECK(carga_mal(R"({"type": "tablaAleatoria"})"));
    MOTOR_CHECK(carga_mal(R"({"type": "startBattle", "startBattle": {"monsterId": "a", "flagVictory": "v",
                              "flagDefeat": "d", "huyen": ["b"]}})"));
    MOTOR_CHECK(carga_mal(R"({"type": "startBattle", "startBattle": {"monsterId": "a", "flagVictory": "v",
                              "flagDefeat": "d", "sinMatar": "si"}})"));
    // El error dice el tipo que falta entre los validos.
    const auto r = RPG::AdventureScript::loadFromString(conEfecto(R"({"type": "volar"})"));
    MOTOR_CHECK(!r.isOk() && r.errorMessage().find("pasarFranja/tablaAleatoria/descanso") != std::string::npos);
}

MOTOR_TEST_CASE(el_motor_pasa_las_franjas_y_los_beats_las_piden) {
    const RPG::AdventureScript a = carga();
    RPG::NarrativeEngine motor;
    motor.setAdventure(&a);
    RPG::NarrativeState estado;
    MOTOR_CHECK(motor.tick(estado).beatId == "llegada");
    MOTOR_CHECK(fr::momentoDe(estado).dia == 1 && fr::momentoDe(estado).franja == 0);

    // Por la mañana, Xila sobrio; hablar con el gasta la mañana.
    const RPG::NarrativeResult r = motor.talkTo("xila", estado);
    MOTOR_CHECK(r.beatId == "xila_sobrio");
    MOTOR_CHECK(std::find(r.log.begin(), r.log.end(), "Llega la tarde.") != r.log.end());
    MOTOR_CHECK(estado.variable("franja") == 1 && estado.variable("dia") == 1);
    MOTOR_CHECK(motor.talkTo("xila", estado).beatId == "xila_borracho");

    // De tarde, Rosita te manda a esperar la noche; de noche, a dormir hasta mañana.
    MOTOR_CHECK(motor.talkTo("skilla", estado).beatId == "a_la_noche");
    MOTOR_CHECK(estado.variable("franja") == 2);
    const RPG::NarrativeResult cama = motor.talkTo("skilla", estado);
    MOTOR_CHECK(cama.beatId == "dormir");
    MOTOR_CHECK(estado.variable("dia") == 2 && estado.variable("franja") == 0);
    MOTOR_CHECK(std::find(cama.log.begin(), cama.log.end(), "Amanece el día 2.") != cama.log.end());
    MOTOR_CHECK(cama.descansos == std::vector<std::string>{"largo"});
    // Otra mañana: Xila vuelve a estar sobrio.
    MOTOR_CHECK(motor.talkTo("xila", estado).beatId == "xila_sobrio");

    // La tabla se pide, no se tira: el motor no tiene dados.
    const RPG::NarrativeResult traje = motor.talkTo("muro", estado);
    MOTOR_CHECK(traje.tablas.size() == 1 && traje.tablas[0].dado == 4);
    MOTOR_CHECK(!estado.hasFlag("roto_culo") && !estado.hasFlag("roto_poco"));
}

MOTOR_TEST_CASE(se_escriben_de_vuelta_igual) {
    const RPG::AdventureScript a = carga();
    const std::string una = RPG::serializeAdventure(a);
    auto otra = RPG::AdventureScript::loadFromString(una);
    MOTOR_CHECK(otra.isOk());
    if (!otra.isOk()) return;
    MOTOR_CHECK(RPG::serializeAdventure(otra.value()) == una);
    MOTOR_CHECK(otra.value().beats[3].effects[0].arg == "noche");
    MOTOR_CHECK(otra.value().beats[5].effects[0].tabla.flags.size() == 4);
    MOTOR_CHECK(otra.value().beats[7].effects[0].battle.sinMatar);
    MOTOR_CHECK(otra.value().beats[8].effects[0].battle.huyen.size() == 1);
    // Un combate de antes se sigue escribiendo como antes: sin huyen ni sinMatar.
    MOTOR_CHECK(una.find("\"huyen\"") != std::string::npos);
    auto viejo = RPG::AdventureScript::loadFromString(conEfecto(
        R"({"type": "startBattle", "startBattle": {"monsterId": "a", "flagVictory": "v", "flagDefeat": "d"}})"));
    MOTOR_CHECK(viejo.isOk());
    if (viejo.isOk()) {
        const std::string texto = RPG::serializeAdventure(viejo.value());
        MOTOR_CHECK(texto.find("huyen") == std::string::npos && texto.find("sinMatar") == std::string::npos);
    }
}

MOTOR_TEST_CASE(la_validacion_sabe_quien_escribe_la_franja_y_las_flags_de_la_tabla) {
    const RPG::ValidationReport informe = RPG::ValidateAdventure(carga(R"({"id": "y", "beats": [
        {"id": "a", "trigger": {"type": "talk", "target": "x"}, "lines": ["?"],
         "effects": [{"type": "pasarFranja"},
                     {"type": "tablaAleatoria", "tabla": {"dado": 2, "flags": ["cara", "cruz"]}}]},
        {"id": "b", "trigger": {"type": "auto"}, "requires": {"allFlags": ["cara"], "variables": {"franja": 1}},
         "lines": ["De tarde, cara."]}]})"));
    for (const RPG::Diagnostic& d : informe.diagnostics) {
        MOTOR_CHECK(d.message.find("'franja'") == std::string::npos);
        MOTOR_CHECK(d.message.find("cara") == std::string::npos);
    }
}

MOTOR_TEST_CASE(en_la_sesion_la_tabla_tira_y_el_descanso_cura) {
    Partida p;
    // La tabla: con el dado en 3, se rompe por la espalda (y solo eso).
    MOTOR_CHECK(p.sesion.talkNarrative("muro"));
    MOTOR_CHECK(p.trama.hasFlag("roto_espalda"));
    MOTOR_CHECK(!p.trama.hasFlag("roto_culo") && !p.trama.hasFlag("roto_axilas") && !p.trama.hasFlag("roto_poco"));
    MOTOR_CHECK(p.enRegistro("El traje: 1d4, sale un 3."));
    p.sesion.closeInteraction();

    // Descanso corto: 1d8 por tier (dado en 3, tier 2: 6 PV).
    p.heroe.takeDamage(30);
    p.heroe.m_tier = 2;
    MOTOR_CHECK(p.sesion.talkNarrative("padre"));
    MOTOR_CHECK(p.heroe.health() == 16);
    MOTOR_CHECK(p.enRegistro("Descanso corto: recuperas 6 PV."));
    p.sesion.closeInteraction();
    // Sin pasarse del tope.
    p.dado.m_cara = 8;
    p.heroe.heal(20);  // 36
    MOTOR_CHECK(p.sesion.talkNarrative("padre"));
    MOTOR_CHECK(p.heroe.health() == 40);
    p.sesion.closeInteraction();
}

MOTOR_TEST_CASE(sin_matar_y_huir_cambian_lo_que_se_cuenta_al_ganar) {
    Partida p;
    // La pelea de taberna: sin matar.
    MOTOR_CHECK(p.sesion.talkNarrative("camorrista"));
    p.sesion.closeInteraction();  // lo que dice el beat se lee antes de pelear
    MOTOR_CHECK(p.sesion.mode() == GameMode::Battle && p.sesion.battle() != nullptr);
    MOTOR_CHECK(p.sesion.battle()->enemies().size() == 3);
    p.tumbaATodos();
    MOTOR_CHECK(p.sesion.battle()->outcome() == BattleOutcome::Victory);
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    MOTOR_CHECK(p.trama.hasFlag("pelea_ganada"));
    MOTOR_CHECK(p.enRegistro("Quedan fuera de combate, pero nadie ha muerto."));
    MOTOR_CHECK(!p.enRegistro("Has derrotado a todos."));
    p.sesion.closeInteraction();

    // Sandes huye a 0 PV; el otro cae.
    MOTOR_CHECK(p.sesion.talkNarrative("sandes"));
    p.sesion.closeInteraction();
    MOTOR_CHECK(p.sesion.battle() != nullptr);
    p.tumbaATodos();
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    MOTOR_CHECK(p.trama.hasFlag("sandes_huido"));
    MOTOR_CHECK(p.enRegistro("Sandes huye malherido."));
    MOTOR_CHECK(p.enRegistro("Has derrotado a todos."));
}

// «Da:» de las fichas de escena: las 2 pociones de Luisarda a cambio de silencio.
MOTOR_TEST_CASE(dar_objetos_los_mete_en_la_mochila_con_su_nombre) {
    // La lectura: 1 si no se dice; sin id, o mas de 20, es una errata.
    const RPG::AdventureScript a = carga(conEfecto(R"({"type": "darObjeto", "arg": "pocion", "value": 2},
                                                     {"type": "darObjeto", "arg": "pan"})").c_str());
    MOTOR_CHECK(a.beats.size() == 1 && a.beats[0].effects.size() == 2);
    MOTOR_CHECK(a.beats[0].effects[0].type == RPG::EffectType::DarObjeto && a.beats[0].effects[0].value == 2);
    MOTOR_CHECK(a.beats[0].effects[1].value == 1);
    MOTOR_CHECK(carga_mal(R"({"type": "darObjeto"})"));
    MOTOR_CHECK(carga_mal(R"({"type": "darObjeto", "arg": "pocion", "value": 0})"));
    MOTOR_CHECK(carga_mal(R"({"type": "darObjeto", "arg": "pocion", "value": 21})"));
    // Se escribe de vuelta igual.
    auto otra = RPG::AdventureScript::loadFromString(RPG::serializeAdventure(a));
    MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == RPG::serializeAdventure(a));
    // El motor lo pide, un id por unidad.
    RPG::NarrativeEngine motor;
    motor.setAdventure(&a);
    RPG::NarrativeState estado;
    const RPG::NarrativeResult r = motor.tick(estado);
    MOTOR_CHECK((r.objetos == std::vector<std::string>{"pocion", "pocion", "pan"}));

    // En la sesion: lo que esta en el catalogo entra; lo que no, se avisa y no entra.
    Partida p;
    p.aventura = carga(R"({"id": "z", "beats": [{"id": "silencio", "trigger": {"type": "talk", "target": "luisarda"},
        "lines": ["Por tu silencio."], "effects": [{"type": "darObjeto", "arg": "pocion", "value": 2},
                                                   {"type": "darObjeto", "arg": "pan"}]}]})");
    p.motor.setAdventure(&p.aventura);
    MOTOR_CHECK(p.sesion.talkNarrative("luisarda"));
    MOTOR_CHECK((p.mochila == std::vector<std::string>{"pocion", "pocion"}));
    MOTOR_CHECK(p.enRegistro("Recibes: Poción x2."));
    MOTOR_CHECK(p.enRegistro("(No se da «pan»: no está en el catálogo de objetos.)"));
}

MOTOR_TEST_CASE(el_hud_lleva_el_dia_y_la_franja) {
    // Lo que el retal del minimapa pone junto a la zona.
    RPG::NarrativeState estado;
    MOTOR_CHECK(fr::rotulo(fr::momentoDe(estado)).empty());  // sin cuenta, nada
    estado.setVariable("dia", 3);
    estado.setVariable("franja", 2);
    MOTOR_CHECK(fr::rotulo(fr::momentoDe(estado)) == "Día 3 · Noche");
    estado.setVariable("franja", 7);  // puesta a mano fuera de rango: la mas cercana
    MOTOR_CHECK(fr::rotulo(fr::momentoDe(estado)) == "Día 3 · Noche");
    Pergamino::DatosExploracion d;
    MOTOR_CHECK(d.momento.empty());
}

int main() { return motor_test::runAllTests(); }
