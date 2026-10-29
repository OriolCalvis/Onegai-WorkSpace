// CAER-01 — Huir y caer, segun la mision.
//
// Oriol (08/10), a la pregunta de si caer en una mision deberia seguir la
// historia, como pasa al huir, en vez de acabar la partida: «Depende, si la
// mision es una mision donde huir o fallar es una opcion bien, pero de
// entrada, no tienes ninguna obligacion de terminarla [...] a no ser que
// sean momentos de historia ineludibles».
//
// Lo que se prueba:
//   - caer en un combate de la historia no acaba la partida: el heroe se
//     levanta con un cuarto de la vida, los que ganaron se van y se
//     enciende flagDefeat (y las oleadas que faltaban se dan por perdidas);
//   - huir enciende flagHuida si la aventura la distingue;
//   - de un combate ineludible no se huye (ni se gasta el turno, y la carta
//     sale bloqueada) y caer en el acaba la partida;
//   - los combates del mapa, sin aventura, siguen acabandola;
//   - el formato: flagHuida e ineludible se leen, se escriben solo si estan
//     y se validan; y el comando de evento lleva lo mismo.
#include "TestHarness.h"

#include "Game/BarraCombate.h"
#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "RPG/AdventureSerializer.h"
#include "RPG/EventCommand.h"
#include "RPG/NarrativeEngine.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

class TestCombatant : public ICombatant {
public:
    explicit TestCombatant(int maxHealth) : m_health(maxHealth), m_maxHealth(maxHealth) {}

    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::min(m_maxHealth, m_health + amount); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }

    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }

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

// Un heroe de 40 PV que entra al combate con 1: el primer golpe lo tumba.
struct Partida {
    ObjectCatalog objetos = catalogo();
    SkillCatalog habilidades;
    TestCombatant heroe{40};
    SkillSet aprendidas{10};
    std::vector<std::string> mochila;
    RPG::NarrativeEngine motor;
    RPG::NarrativeState trama;
    GameSession sesion;

    explicit Partida(LevelDefinition level = nivel())
        : sesion(nullptr, &objetos, &habilidades, level, &heroe, &aprendidas, &mochila) {
        heroe.takeDamage(39);
        sesion.setNarrative(&motor, &trama);
    }

    // Los enemigos pegan hasta que el heroe cae.
    void caer() {
        for (int i = 0; i < 400 && sesion.battle() != nullptr &&
                        sesion.battle()->outcome() == BattleOutcome::InProgress;
             ++i)
            sesion.battle()->resolveEnemyTurn();
    }

    bool enRegistro(const std::string& linea) const {
        const auto& log = sesion.log();
        return std::any_of(log.begin(), log.end(), [&](const std::string& l) { return l == linea; });
    }
};

}  // namespace

MOTOR_TEST_CASE(caer_en_un_combate_de_la_historia_no_acaba_la_partida) {
    Partida p;
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d"});
    MOTOR_CHECK(p.sesion.mode() == GameMode::Battle);
    MOTOR_CHECK(p.sesion.battle() != nullptr && p.sesion.battle()->sePuedeHuir());
    p.caer();
    MOTOR_CHECK(p.sesion.battle() != nullptr && p.sesion.battle()->outcome() == BattleOutcome::Defeat);
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    // Sigue la historia: explorando, con la flag de derrota y no la de victoria.
    MOTOR_CHECK(p.sesion.mode() == GameMode::Exploration);
    MOTOR_CHECK(p.trama.hasFlag("d") && !p.trama.hasFlag("v"));
    // Se levanta con un cuarto de la vida, y los que ganaron se han ido.
    MOTOR_CHECK(p.heroe.health() == 10);
    MOTOR_CHECK(p.sesion.enemies().empty());
    MOTOR_CHECK(p.enRegistro("Has caído derrotado."));
    MOTOR_CHECK(p.enRegistro("Te levantas con 10 PV."));
}

MOTOR_TEST_CASE(caer_da_por_perdidas_las_oleadas_que_faltaban) {
    Partida p;
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v1", .flagDefeat = "d1"});
    p.sesion.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v2", .flagDefeat = "d2"});  // encolada
    p.caer();
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    // Quien cae no sigue peleando: la segunda no empieza y cuenta como perdida.
    MOTOR_CHECK(p.sesion.battle() == nullptr);
    MOTOR_CHECK(p.sesion.mode() == GameMode::Exploration);
    MOTOR_CHECK(p.trama.hasFlag("d1") && p.trama.hasFlag("d2"));
    MOTOR_CHECK(!p.trama.hasFlag("v2"));
}

MOTOR_TEST_CASE(huir_enciende_la_flag_de_huida_si_la_aventura_la_distingue) {
    Partida p;
    p.sesion.startNarrativeBattle(
        {.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d", .flagHuida = "h"});
    MOTOR_CHECK(p.sesion.battle() != nullptr);
    if (p.sesion.battle() == nullptr) return;
    BattleAction correr;
    correr.type = BattleActionType::Flee;
    p.sesion.battle()->resolveAllyAction(0, correr);
    MOTOR_CHECK(p.sesion.battle()->outcome() == BattleOutcome::Fled);
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    MOTOR_CHECK(p.trama.hasFlag("h") && !p.trama.hasFlag("d"));
    // Huir no mata a nadie: el que te corto el paso sigue ahi.
    MOTOR_CHECK(p.sesion.enemies().size() == 1);
    MOTOR_CHECK(p.heroe.health() == 1);
}

MOTOR_TEST_CASE(de_un_combate_ineludible_no_se_huye_y_caer_acaba_la_partida) {
    Partida p;
    p.sesion.startNarrativeBattle(
        {.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d", .ineludible = true});
    BattleState* combate = p.sesion.battle();
    MOTOR_CHECK(combate != nullptr);
    if (combate == nullptr) return;
    MOTOR_CHECK(!combate->sePuedeHuir());
    // La carta Huir sale bloqueada y dice por que.
    const Game::VistaBarra vista = Game::vistaDeBatalla(*combate);
    MOTOR_CHECK(vista.basicas.size() == 3 && vista.basicas[2].id == "#huir");
    MOTOR_CHECK(vista.basicas[2].estado == Game::CartaBarra::Estado::Bloqueada);
    MOTOR_CHECK(vista.basicas[2].motivo == "de este combate no se huye");
    // Y si alguien lo pide igual, no pasa nada: ni se huye ni se acaba el combate.
    BattleAction correr;
    correr.type = BattleActionType::Flee;
    combate->resolveAllyAction(0, correr);
    MOTOR_CHECK(combate->outcome() == BattleOutcome::InProgress);
    MOTOR_CHECK(!combate->log().empty() && combate->log().back() == "De este combate no se huye.");
    // Caer en un momento ineludible si acaba la partida.
    p.caer();
    MOTOR_CHECK(combate->outcome() == BattleOutcome::Defeat);
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    MOTOR_CHECK(p.sesion.mode() == GameMode::GameOver);
    MOTOR_CHECK(p.heroe.health() == 0);
}

MOTOR_TEST_CASE(un_combate_del_mapa_sigue_acabando_la_partida) {
    LevelDefinition level = nivel();
    ObjectSpawn lobo;
    lobo.objectId = "llop";
    lobo.position = GridCoord{1, 0};
    lobo.patrolMin = GridCoord{1, 0};
    lobo.patrolMax = GridCoord{1, 0};
    level.objects.push_back(lobo);
    Partida p(level);
    MOTOR_CHECK(!p.sesion.tryMovePlayer(1, 0));
    MOTOR_CHECK(p.sesion.mode() == GameMode::Battle);
    MOTOR_CHECK(p.sesion.battle() != nullptr && p.sesion.battle()->sePuedeHuir());
    p.caer();
    MOTOR_CHECK(p.sesion.syncBattleOutcome());
    MOTOR_CHECK(p.sesion.mode() == GameMode::GameOver);
}

MOTOR_TEST_CASE(huida_e_ineludible_se_leen_se_escriben_y_se_validan) {
    const auto leida = RPG::AdventureScript::loadFromString(R"({"id": "caer", "beats": [
        {"id": "atraco", "trigger": {"type": "auto"}, "lines": ["La bolsa."],
         "effects": [{"type": "startBattle", "startBattle": {"monsterId": "llop",
            "flagVictory": "v1", "flagDefeat": "d1", "flagHuida": "h1"}}]},
        {"id": "puente", "trigger": {"type": "auto"}, "lines": ["No hay otro camino."],
         "effects": [{"type": "startBattle", "startBattle": {"monsterId": "llop",
            "flagVictory": "v2", "flagDefeat": "d2", "ineludible": true}}]},
        {"id": "de_siempre", "trigger": {"type": "auto"}, "lines": ["Uno."],
         "effects": [{"type": "startBattle", "startBattle": {"monsterId": "llop",
            "flagVictory": "v3", "flagDefeat": "d3"}}]}]})");
    MOTOR_CHECK(leida.isOk());
    if (!leida.isOk()) return;
    const RPG::BattleRequest& atraco = leida.value().beats[0].effects[0].battle;
    MOTOR_CHECK(atraco.flagHuida == "h1" && !atraco.ineludible);
    const RPG::BattleRequest& puente = leida.value().beats[1].effects[0].battle;
    MOTOR_CHECK(puente.flagHuida.empty() && puente.ineludible);
    const RPG::BattleRequest& siempre = leida.value().beats[2].effects[0].battle;
    MOTOR_CHECK(siempre.flagHuida.empty() && !siempre.ineludible);
    // Se escriben solo si estan (lo de antes se escribe igual que antes), y de
    // ida y vuelta da lo mismo.
    const std::string texto = RPG::serializeAdventure(leida.value());
    MOTOR_CHECK(texto.find("\"flagHuida\": \"h1\"") != std::string::npos);
    MOTOR_CHECK(texto.find("\"ineludible\": true") != std::string::npos);
    MOTOR_CHECK(texto.find("\"ineludible\": false") == std::string::npos);
    const auto otra = RPG::AdventureScript::loadFromString(texto);
    MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == texto);
    // De un combate ineludible no se huye: sobra flagHuida. E ineludible es un booleano.
    MOTOR_CHECK(!RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b", "trigger": {"type": "auto"},
        "effects": [{"type": "startBattle", "startBattle": {"monsterId": "llop",
        "flagVictory": "v", "flagDefeat": "d", "flagHuida": "h", "ineludible": true}}]}]})").isOk());
    MOTOR_CHECK(!RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b", "trigger": {"type": "auto"},
        "effects": [{"type": "startBattle", "startBattle": {"monsterId": "llop",
        "flagVictory": "v", "flagDefeat": "d", "ineludible": "si"}}]}]})").isOk());
}

MOTOR_TEST_CASE(el_comando_de_evento_lleva_la_huida_y_lo_ineludible) {
    const RPG::CommandSpec* iniciar = RPG::FindCommand("combate.iniciar");
    MOTOR_CHECK(iniciar != nullptr);
    if (iniciar == nullptr) return;
    const auto parametro = [&](const std::string& clave) -> const RPG::CommandParamSpec* {
        for (const RPG::CommandParamSpec& p : iniciar->params)
            if (p.key == clave) return &p;
        return nullptr;
    };
    MOTOR_CHECK(parametro("flag_huida") != nullptr && parametro("flag_huida")->type == RPG::CommandParamType::FlagRef);
    MOTOR_CHECK(parametro("ineludible") != nullptr && parametro("ineludible")->type == RPG::CommandParamType::Bool);
    MOTOR_CHECK(!parametro("ineludible")->required);
}

int main() { return motor_test::runAllTests(); }
