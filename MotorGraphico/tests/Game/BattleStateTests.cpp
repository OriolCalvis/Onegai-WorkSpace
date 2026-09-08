// C-01 — El objetivo de una habilidad lo decide Skill::target, no el bando
// que resolveAllyAction miraba por costumbre.
//
// El bug: resolveAllyAction validaba y elegia el objetivo SIEMPRE dentro
// de m_enemies, tirase lo que tirase Skill::target. La orden "Cura" del
// menu gastaba los 4 PM del jugador y le curaba ~20 PV al monstruo.
//
// Unit y no demo: sin assets, sin cwd y sin GL -- BattleState opera sobre
// ICombatant, y el dado entra por setRandomEngine() (ver ScriptedRng.h),
// asi que el resultado es exacto y no "lo que saliera de la semilla".

#include "TestHarness.h"

#include "Game/BattleState.h"
#include "Game/Skill.h"
#include "Render/ICombatant.h"
#include "ScriptedRng.h"
#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "RPG/ConditionEngine.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace {

// Combatiente minimo: todos los stats a 0, para que el pool sea 1 dado
// (1 base + stat) y una sola cara del guion decida el grado.
class TestCombatant : public ICombatant {
public:
    explicit TestCombatant(int maxHealth, int health)
        : m_health(health), m_maxHealth(maxHealth) {}

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

// Catalogo con las dos habilidades que enfrenta la prueba: una que cura
// al bando propio y otra que dana al contrario.
SkillCatalog makeCatalog() {
    SkillCatalog catalog;

    Skill cura;
    cura.id = "cura";
    cura.name = "Cura";
    cura.mpCost = 4;
    cura.power = 20;
    cura.effect = SkillEffect::Heal;
    cura.target = SkillTarget::SingleAlly;
    catalog.add(cura);

    Skill flama;
    flama.id = "flama";
    flama.name = "Flama";
    flama.mpCost = 4;
    flama.power = 20;
    flama.effect = SkillEffect::Damage;
    flama.target = SkillTarget::SingleEnemy;
    catalog.add(flama);

    Skill vigor;
    vigor.id = "vigor";
    vigor.name = "Vigor";
    vigor.mpCost = 3;
    vigor.power = 10;
    vigor.effect = SkillEffect::Heal;
    vigor.target = SkillTarget::Self;
    catalog.add(vigor);

    return catalog;
}

}  // namespace

// Curar con 30/60 deja al jugador en 50 y al enemigo intacto. Con el
// codigo anterior a C-01 el mismo caso dejaba al jugador en 30 (perdiendo
// los PM) y al enemigo curado.
MOTOR_TEST_CASE(curar_actua_sobre_el_aliado_y_no_sobre_el_enemigo) {
    SkillCatalog catalog = makeCatalog();

    TestCombatant player(60, 30);
    TestCombatant slime(40, 40);
    SkillSet playerSkills(10);
    playerSkills.learn("cura");

    std::vector<BattleParticipant> allies{BattleParticipant{"Jugador", &player, &playerSkills}};
    std::vector<BattleParticipant> enemies{BattleParticipant{"Slime", &slime, nullptr}};
    BattleState battle(std::move(allies), std::move(enemies), &catalog);

    // 5 con un solo dado: medio exito, sin pifia y sin todo-seises ->
    // SUCCESS -> magnitud = power. La curacion no se resiste (no hay
    // defensor), asi que basta una cara.
    ScriptedRng dado({5});
    battle.setRandomEngine(dado);

    BattleAction action;
    action.type = BattleActionType::UseSkill;
    action.skillId = "cura";
    action.targetIndex = 0;  // indexa el bando propio, porque la habilidad es SingleAlly
    battle.resolveAllyAction(0, action);

    MOTOR_CHECK(player.health() == 50);
    MOTOR_CHECK(slime.health() == 40);
    MOTOR_CHECK(playerSkills.mana() == 6);  // 10 - 4, gastado UNA vez
}

// La otra mitad del contrato: una habilidad de dano sigue yendo al bando
// contrario, y el aliado no se toca.
MOTOR_TEST_CASE(una_habilidad_de_dano_sigue_yendo_al_enemigo) {
    SkillCatalog catalog = makeCatalog();

    TestCombatant player(60, 30);
    TestCombatant slime(40, 40);
    SkillSet playerSkills(10);
    playerSkills.learn("flama");

    std::vector<BattleParticipant> allies{BattleParticipant{"Jugador", &player, &playerSkills}};
    std::vector<BattleParticipant> enemies{BattleParticipant{"Slime", &slime, nullptr}};
    BattleState battle(std::move(allies), std::move(enemies), &catalog);

    // Atacante 1 dado (5 -> medio exito), defensor 0 dados (stat 0) ->
    // el atacante gana -> SUCCESS -> 20 de dano.
    ScriptedRng dado({5});
    battle.setRandomEngine(dado);

    BattleAction action;
    action.type = BattleActionType::UseSkill;
    action.skillId = "flama";
    action.targetIndex = 0;
    battle.resolveAllyAction(0, action);

    MOTOR_CHECK(slime.health() == 20);
    MOTOR_CHECK(player.health() == 30);
    MOTOR_CHECK(playerSkills.mana() == 6);
}

// Self ignora targetIndex: el objetivo es quien actua, aunque el HUD
// mande un indice cualquiera.
MOTOR_TEST_CASE(una_habilidad_self_se_aplica_sobre_quien_actua) {
    SkillCatalog catalog = makeCatalog();

    TestCombatant player(60, 30);
    TestCombatant slime(40, 40);
    SkillSet playerSkills(10);
    playerSkills.learn("vigor");

    std::vector<BattleParticipant> allies{BattleParticipant{"Jugador", &player, &playerSkills}};
    std::vector<BattleParticipant> enemies{BattleParticipant{"Slime", &slime, nullptr}};
    BattleState battle(std::move(allies), std::move(enemies), &catalog);

    ScriptedRng dado({5});
    battle.setRandomEngine(dado);

    BattleAction action;
    action.type = BattleActionType::UseSkill;
    action.skillId = "vigor";
    action.targetIndex = 7;  // fuera de rango a proposito: Self no lo mira
    battle.resolveAllyAction(0, action);

    MOTOR_CHECK(player.health() == 40);
    MOTOR_CHECK(slime.health() == 40);
    MOTOR_CHECK(playerSkills.mana() == 7);
}

// El PM no se gasta si la accion no llega a aplicarse: objetivo fuera de
// rango del bando que toca.
MOTOR_TEST_CASE(un_objetivo_invalido_no_gasta_mana) {
    SkillCatalog catalog = makeCatalog();

    TestCombatant player(60, 30);
    TestCombatant slime(40, 40);
    SkillSet playerSkills(10);
    playerSkills.learn("cura");

    std::vector<BattleParticipant> allies{BattleParticipant{"Jugador", &player, &playerSkills}};
    std::vector<BattleParticipant> enemies{BattleParticipant{"Slime", &slime, nullptr}};
    BattleState battle(std::move(allies), std::move(enemies), &catalog);

    ScriptedRng dado({5});
    battle.setRandomEngine(dado);

    BattleAction action;
    action.type = BattleActionType::UseSkill;
    action.skillId = "cura";
    action.targetIndex = 3;  // el bando propio tiene un solo participante
    battle.resolveAllyAction(0, action);

    MOTOR_CHECK(player.health() == 30);
    MOTOR_CHECK(slime.health() == 40);
    MOTOR_CHECK(playerSkills.mana() == 10);
}


namespace {
class ConditionCombatant : public TestCombatant {
public:
    ConditionCombatant() : TestCombatant(100, 50) {}
    RPG::CharacterSheet sheet;
    bool blocksDamage = false;
    int defenseStat = 0;
    int stat(RPG::Stat) const override { return defenseStat; }
    RPG::CharacterSheet* character_sheet() override { return &sheet; }
    const RPG::CharacterSheet* character_sheet() const override { return &sheet; }
    void takeDamage(int amount) override {
        if (!blocksDamage) TestCombatant::takeDamage(amount);
    }
};

RPG::Catalogs::ConditionCatalog conditionsCatalog() {
    RPG::Catalogs::ConditionCatalog catalog;
    RPG::ConditionDefinition sleep;
    sleep.id = "sleep";
    sleep.defaultRounds = 3;
    sleep.breaksOnDamage = true;
    sleep.statMod = {0, -1, 0, 0};
    catalog.insert(sleep);
    RPG::ConditionDefinition poison;
    poison.id = "poison";
    poison.defaultRounds = 3;
    catalog.insert(poison);
    return catalog;
}

void applySleep(RPG::CharacterSheet& sheet, const RPG::Catalogs::ConditionCatalog& catalog) {
    MOTOR_CHECK(RPG::ConditionEngine::apply(sheet, *catalog.find("sleep"), catalog).isOk());
}
} // namespace

MOTOR_TEST_CASE(dano_basico_y_habilidad_rompen_solo_condiciones_del_receptor) {
    for (bool enemyTurn : {false, true}) {
        for (bool skill : {false, true}) {
            auto conditions = conditionsCatalog();
            auto skills = makeCatalog();
            ConditionCombatant player, enemy;
            applySleep(player.sheet, conditions);
            applySleep(enemy.sheet, conditions);
            auto& receiver = enemyTurn ? player : enemy;
            auto& attacker = enemyTurn ? enemy : player;
            MOTOR_CHECK(RPG::ConditionEngine::apply(receiver.sheet, *conditions.find("poison"), conditions).isOk());
            SkillSet abilities(10);
            abilities.learn("flama");
            BattleState battle({{"Player", &player, skill && !enemyTurn ? &abilities : nullptr}},
                               {{"Enemy", &enemy, skill && enemyTurn ? &abilities : nullptr}}, &skills);
            battle.setConditionCatalog(&conditions);
            ScriptedRng rng({5});
            battle.setRandomEngine(rng);
            if (enemyTurn) battle.resolveEnemyTurn();
            else battle.resolveAllyAction(0, {skill ? BattleActionType::UseSkill : BattleActionType::Attack, "flama", 0});
            MOTOR_CHECK(receiver.health() < 50);
            MOTOR_CHECK(receiver.sheet.conditions.size() == 1);
            MOTOR_CHECK(receiver.sheet.conditions.front().conditionId == "poison");
            MOTOR_CHECK(receiver.sheet.conditionModifiers[1] == 0);
            MOTOR_CHECK(attacker.health() == 50);
            MOTOR_CHECK(attacker.sheet.conditions.size() == 1);
            MOTOR_CHECK(attacker.sheet.conditionModifiers[1] == -1);
        }
    }
}

MOTOR_TEST_CASE(curacion_dano_bloqueado_y_fallo_no_rompen_condiciones) {
    for (int scenario = 0; scenario < 3; ++scenario) {
        auto conditions = conditionsCatalog();
        auto skills = makeCatalog();
        ConditionCombatant player, enemy;
        applySleep(player.sheet, conditions);
        applySleep(enemy.sheet, conditions);
        enemy.blocksDamage = scenario == 1;
        enemy.defenseStat = scenario == 2 ? 1 : 0;
        SkillSet abilities(10);
        abilities.learn("cura");
        BattleState battle({{"Player", &player, &abilities}}, {{"Enemy", &enemy, nullptr}}, &skills);
        battle.setConditionCatalog(&conditions);
        ScriptedRng rng({scenario == 2 ? 1 : 5, 6});
        battle.setRandomEngine(rng);
        battle.resolveAllyAction(0, {scenario == 0 ? BattleActionType::UseSkill : BattleActionType::Attack, "cura", 0});
        MOTOR_CHECK(player.health() == (scenario == 0 ? 70 : 50));
        MOTOR_CHECK(enemy.health() == 50);
        MOTOR_CHECK(player.sheet.conditions.size() == 1);
        MOTOR_CHECK(enemy.sheet.conditions.size() == 1);
    }
}

MOTOR_TEST_CASE(combate_sin_catalogo_o_sin_ficha_conserva_compatibilidad) {
    auto conditions = conditionsCatalog();
    ConditionCombatant receiver;
    TestCombatant legacy(100, 100);
    applySleep(receiver.sheet, conditions);
    BattleState battle({{"Legacy", &legacy, nullptr}}, {{"Sheet", &receiver, nullptr}}, nullptr);
    ScriptedRng rng({5});
    battle.setRandomEngine(rng);
    battle.resolveAllyAction(0, {});
    MOTOR_CHECK(receiver.health() < 50);
    MOTOR_CHECK(receiver.sheet.conditions.size() == 1);
    battle.setConditionCatalog(&conditions);
    battle.resolveEnemyTurn();
    MOTOR_CHECK(legacy.health() < 100);
}

MOTOR_TEST_CASE(game_session_propaga_catalogo_antes_y_durante_el_combate) {
    for (bool setBeforeBattle : {true, false}) {
        auto conditions = conditionsCatalog();
        ObjectCatalog objects;
        ObjectDefinition enemy;
        enemy.id = "enemy";
        enemy.category = ObjectCategory::Enemy;
        enemy.combat.maxHealth = 100;
        objects.add(enemy);
        LevelDefinition level;
        level.playerStart = {0, 0};
        ObjectSpawn spawn;
        spawn.objectId = "enemy";
        spawn.position = {1, 0};
        level.objects.push_back(spawn);
        ConditionCombatant player;
        GameSession session(nullptr, &objects, nullptr, level, &player, nullptr, nullptr);
        RPG::CharacterSheet enemySheet;
        applySleep(enemySheet, conditions);
        session.enemies().front().brain->set_character_sheet(&enemySheet);
        if (setBeforeBattle) session.setConditionCatalog(&conditions);
        MOTOR_CHECK(!session.tryMovePlayer(1, 0));
        MOTOR_CHECK(session.battle() != nullptr);
        if (!setBeforeBattle) session.setConditionCatalog(&conditions);
        ScriptedRng rng({5});
        session.battle()->setRandomEngine(rng);
        session.battle()->resolveAllyAction(0, {});
        MOTOR_CHECK(session.enemies().front().brain->health() < 100);
        MOTOR_CHECK(enemySheet.conditions.empty());
    }
}

int main() { return motor_test::runAllTests(); }
