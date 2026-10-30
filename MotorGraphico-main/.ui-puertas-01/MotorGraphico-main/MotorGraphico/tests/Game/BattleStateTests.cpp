// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-01  (fase C, 2 j) - La curació cura qui toca
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

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

// AV13-35 (14/10) — Se borraron los casos del combate antiguo (el ataque Nd6 y las habilidades
// con mana sin mesa de cartas): planificacion_alcance_y_vision/MEDIDA_COMBATE_VIEJO.md. Quedan
// huir, los objetos, el tick de los estados y lo que no es del combate antiguo.
#include "TestHarness.h"

#include "Game/BattleState.h"
#include "Game/Skill.h"
#include "Render/ICombatant.h"
#include "ScriptedRng.h"
#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "RPG/ConditionEngine.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/EnhanceHooks.h"

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

// D-09 — Les cartes apliquen les seves condicions.
//
// El contracte te dues meitats: l'executor REPORTA la condicio pendent
// (id + rondes de la carta) quan el grau hi arriba, i el combat L'APLICA
// via ConditionEngine. Amb PARCIAL no s'aplica; amb EXIT si. Sense
// cataleg, sense fitxa o amb id desconegut, el combat no rebenta: el
// carregador ja falla en dur (D-01/D-10), el combat no.

namespace {

SkillCatalog makeVenomCatalog() {
    SkillCatalog catalog;
    Skill ponzona;
    ponzona.id = "ponzona";
    ponzona.name = "Ponzoña";
    ponzona.mpCost = 4;
    ponzona.power = 10;
    ponzona.effect = SkillEffect::Damage;
    ponzona.target = SkillTarget::SingleEnemy;
    ponzona.applyConditionId = "poison";
    ponzona.applyConditionMinDegree = 2;  // SUCCESS
    // Mes que els 3 per defecte del cataleg de condicions: mana la carta.
    ponzona.conditionDurationRounds = 5;
    catalog.add(ponzona);
    return catalog;
}

}  // namespace

// L'executor informa la condicio pendent: amb EXIT hi es, amb PARCIAL no.
MOTOR_TEST_CASE(executor_reporta_condicio_pendent_nomes_si_el_grau_hi_arriba) {
    RPG::SkillDefinition def;
    def.id = "ponzona_exec";
    def.name = "Ponzoña";
    def.casting_stat = RPG::Stat::DES;
    def.magnitude_by_degree[0] = 0.0f;
    def.magnitude_by_degree[1] = 5.0f;
    def.magnitude_by_degree[2] = 10.0f;
    def.magnitude_by_degree[3] = 15.0f;
    def.apply_condition_id = "poison";
    def.apply_condition_min_degree = RPG::Degree::SUCCESS;
    def.condition_duration_rounds = 5;

    // EXIT: un 5 amb un sol dau = mig exit, sense rival -> SUCCESS.
    {
        RPG::ExecutionContext ctx;
        ctx.skill = &def;
        ctx.override_base_dice = 1;
        ScriptedRng dado({5});
        ctx.rng = &dado;
        RPG::SkillExecutor::Out out = RPG::SkillExecutor::execute(ctx);
        MOTOR_CHECK(out.final_degree == RPG::Degree::SUCCESS);
        MOTOR_CHECK(out.pending_condition_id == "poison");
        MOTOR_CHECK(out.pending_condition_rounds == 5);
    }
    // PARCIAL: un 4 = 0 exits sense pifia (un sol dau no pot pifiar) -> PARTIAL.
    {
        RPG::ExecutionContext ctx;
        ctx.skill = &def;
        ctx.override_base_dice = 1;
        ScriptedRng dado({4});
        ctx.rng = &dado;
        RPG::SkillExecutor::Out out = RPG::SkillExecutor::execute(ctx);
        MOTOR_CHECK(out.final_degree == RPG::Degree::PARTIAL);
        MOTOR_CHECK(out.pending_condition_id.empty());
        MOTOR_CHECK(out.pending_condition_rounds == 0);
    }
}

// Fase 10 — la accion Item en combate. Cada fallo deja su motivo en el
// log y no consume: sin inventario, item que no tienes o item de
// efecto None (una llave) no se gastan. Una pocion con efecto Heal cura y
// CONSUME una instancia del inventario real del jugador.
MOTOR_TEST_CASE(la_accion_item_cura_consume_y_no_mal_gasta) {
    SkillCatalog skills;
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({
        "objects": [
            {"id": "pocion", "name": "Pocion", "category": "pickup",
             "pickup": {"effect": "heal", "power": 15}},
            {"id": "llave", "name": "Llave", "category": "pickup",
             "pickup": {"effect": "none"}}
        ]
    })")
                   .isOk());

    // Sin inventario: la accion falla controladamente ("no lleva").
    {
        TestCombatant guardia{60, 60};
        TestCombatant slime{30, 30};
        BattleState battle({{"Guardia", &guardia, nullptr, nullptr}},
                           {{"Slime", &slime, nullptr, nullptr}}, &skills, &objects);
        BattleAction action;
        action.type = BattleActionType::Item;
        action.itemId = "pocion";
        battle.resolveAllyAction(0, action);
        MOTOR_CHECK(guardia.health() == 60);
        MOTOR_CHECK(battle.outcome() == BattleOutcome::InProgress);
    }

    TestCombatant player{60, 10};
    TestCombatant slime{30, 30};
    SkillSet abilities{10};
    std::vector<std::string> inventory{"pocion", "llave"};
    BattleState battle({{"Player", &player, &abilities, &inventory}},
                       {{"Slime", &slime, nullptr, nullptr}}, &skills, &objects);

    BattleAction usar;
    usar.type = BattleActionType::Item;
    usar.itemId = "pocion";

    // Pocion: cura 15 (clamp hasta el tope 25) y consume UNA instancia.
    battle.resolveAllyAction(0, usar);
    MOTOR_CHECK(player.health() == 25);
    MOTOR_CHECK(inventory.size() == 1);
    MOTOR_CHECK(inventory[0] == "llave");

    // La llave (effect None) no hace nada en combate y no se gasta.
    usar.itemId = "llave";
    battle.resolveAllyAction(0, usar);
    MOTOR_CHECK(inventory.size() == 1);

    // Repetir la pocion cuando ya no la tienes: otro no-op con aviso.
    usar.itemId = "pocion";
    battle.resolveAllyAction(0, usar);
    MOTOR_CHECK(player.health() == 25);
    MOTOR_CHECK(inventory.size() == 1);

    // Un item del inventario que no esta en el catalogo (o que no es
    // Pickup) tampoco consume ni cura.
    inventory.push_back("pocion");
    inventory.push_back("misterio");
    usar.itemId = "misterio";
    battle.resolveAllyAction(0, usar);
    MOTOR_CHECK(player.health() == 25);
    MOTOR_CHECK(inventory.size() == 3);
    MOTOR_CHECK(battle.outcome() == BattleOutcome::InProgress);
}

// H0-06 — Los estados alterados caducan por ronda (criterio del Gantt:
// "Envenenar a un enemigo y esperar N rondas: el estado desaparece solo y
// deja de hacer daño"). BattleState::tickRound() es el consumidor de
// ConditionEngine::tick_conditions: una llamada por ronda de combate,
// aplica el dano por ronda de cada condicion (quien posee la vida es
// BattleState, no el ConditionEngine — ver su cabecera) y caduca las que
// agotan las rondas.

namespace {

// Catalogo con un veneno que HACE DANO por ronda (el de conditionsCatalog()
// no declara damagePerRound). La duracion ROUNDS la retira sola la ronda.
RPG::Catalogs::ConditionCatalog dotConditionsCatalog() {
    RPG::Catalogs::ConditionCatalog catalog;
    RPG::ConditionDefinition poison;
    poison.id = "poison";
    poison.defaultRounds = 3;
    poison.damagePerRound = 4;
    catalog.insert(poison);
    return catalog;
}

}  // namespace

MOTOR_TEST_CASE(h0_06_el_veneno_hace_dano_por_ronda_y_caduca_solo) {
    auto conditions = dotConditionsCatalog();
    ConditionCombatant player, enemy;
    MOTOR_CHECK(RPG::ConditionEngine::apply(enemy.sheet, *conditions.find("poison"), conditions)
                    .isOk());

    BattleState battle({{"Player", &player, nullptr}}, {{"Enemy", &enemy, nullptr}}, nullptr);
    battle.setConditionCatalog(&conditions);
    MOTOR_CHECK(enemy.health() == 50);
    MOTOR_CHECK(enemy.sheet.conditions.front().roundsRemaining == 3);
    MOTOR_CHECK(enemy.sheet.conditions.front().stacks == 1);

    // Roda 1: 4 de dano por ronda, quedan 2 rondas.
    battle.tickRound();
    MOTOR_CHECK(enemy.health() == 46);
    MOTOR_CHECK(enemy.sheet.conditions.size() == 1);
    MOTOR_CHECK(enemy.sheet.conditions.front().roundsRemaining == 2);

    // Ronda 2: otros 4, queda 1.
    battle.tickRound();
    MOTOR_CHECK(enemy.health() == 42);
    MOTOR_CHECK(enemy.sheet.conditions.size() == 1);
    MOTOR_CHECK(enemy.sheet.conditions.front().roundsRemaining == 1);

    // Ronda 3: el ultimo tick hace su dano (ConditionEngine::tick_conditions
    // aplica el efecto en la ultima ronda tambien) y la condicion desaparece.
    battle.tickRound();
    MOTOR_CHECK(enemy.health() == 38);
    MOTOR_CHECK(enemy.sheet.conditions.empty());

    // Ya caducada: los siguientes ticks no hacen nada (sin condiciones).
    battle.tickRound();
    MOTOR_CHECK(enemy.health() == 38);
    MOTOR_CHECK(battle.outcome() == BattleOutcome::InProgress);
}

// El DOT se paga sobre el portador del veneno (aliado o enemigo) y el tick
// tambien expone el log de caducidad para el HUD (Fase 9).
MOTOR_TEST_CASE(h0_06_tick_hace_dano_en_ambos_bandos_y_loguea_la_caducidad) {
    auto conditions = dotConditionsCatalog();
    ConditionCombatant player, enemy;
    MOTOR_CHECK(RPG::ConditionEngine::apply(player.sheet, *conditions.find("poison"), conditions)
                    .isOk());
    MOTOR_CHECK(RPG::ConditionEngine::apply(enemy.sheet, *conditions.find("poison"), conditions)
                    .isOk());

    BattleState battle({{"Player", &player, nullptr}}, {{"Enemy", &enemy, nullptr}}, nullptr);
    battle.setConditionCatalog(&conditions);

    for (int i = 0; i < 3; ++i) {
        battle.tickRound();
    }
    MOTOR_CHECK(player.health() == 38);  // 50 - 3x4
    MOTOR_CHECK(enemy.health() == 38);   // 50 - 3x4
    MOTOR_CHECK(player.sheet.conditions.empty());
    MOTOR_CHECK(enemy.sheet.conditions.empty());

    bool foundExpiry = false;
    for (const std::string& line : battle.log()) {
        if (line.find("poison se ha agotado") != std::string::npos) {
            foundExpiry = true;
            break;
        }
    }
    MOTOR_CHECK(foundExpiry);
}

// Sin catalogo de condiciones el tick es un no-op (misma robustez que
// resolveAllyAction sin m_conditions): el combate no rebenta.
MOTOR_TEST_CASE(h0_06_sin_catalogo_de_condiciones_el_tick_no_hace_nada) {
    auto conditions = dotConditionsCatalog();
    ConditionCombatant player, enemy;
    MOTOR_CHECK(RPG::ConditionEngine::apply(enemy.sheet, *conditions.find("poison"), conditions)
                    .isOk());

    BattleState battle({{"Player", &player, nullptr}}, {{"Enemy", &enemy, nullptr}}, nullptr);
    // A proposito SIN setConditionCatalog.

    battle.tickRound();
    MOTOR_CHECK(enemy.health() == 50);
    MOTOR_CHECK(enemy.sheet.conditions.size() == 1);
}

int main() { return motor_test::runAllTests(); }
