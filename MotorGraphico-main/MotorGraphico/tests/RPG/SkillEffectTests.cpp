// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   D-11  (fase D, 12 j) - Vocabulari d'efectes d'habilitat
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "TestHarness.h"

#include "RPG/SkillEffect.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/CharacterSheet.h"
#include "RPG/EnhanceHooks.h"
#include "Render/ICombatant.h"
#include "ScriptedRng.h"

#include <string>
#include <vector>

namespace {

void require_eq(const std::string& what, int got, int expected) {
    if (got != expected) throw std::runtime_error(what + ": " + std::to_string(got) + " != " + std::to_string(expected));
}

void require_eq_str(const std::string& what, const std::string& got, const std::string& expected) {
    if (got != expected) throw std::runtime_error(what + ": '" + got + "' != '" + expected + "'");
}

void require_true(const std::string& what, bool ok) {
    if (!ok) throw std::runtime_error(what + ": expected true");
}

class DummyTarget : public RPG::ICombatant {
public:
    void takeDamage(int amount) override { m_hp -= amount; }
    void heal(int amount) override { m_hp += amount; }
    int health() const override { return m_hp; }
    int maxHealth() const override { return 100; }
    bool isAlive() const override { return m_hp > 0; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "dummy"; }
private:
    int m_hp = 50;
};

}  // namespace

// === parse_effect_scaling ===

MOTOR_TEST_CASE(parse_effect_scaling_reconeix_damage) {
    auto def = RPG::parse_effect_scaling("damage");
    require_true("parse_effect_scaling(damage)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "damage");
}

MOTOR_TEST_CASE(parse_effect_scaling_reconeix_heal) {
    auto def = RPG::parse_effect_scaling("heal");
    require_true("parse_effect_scaling(heal)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "heal");
}

MOTOR_TEST_CASE(parse_effect_scaling_reconeix_stat) {
    auto def = RPG::parse_effect_scaling("CON");
    require_true("parse_effect_scaling(CON)", def.has_value());
    require_eq_str("scaling", RPG::scaling_stat_to_string(def->scaling), "CON");
}

MOTOR_TEST_CASE(parse_effect_scaling_reconeix_stat_family) {
    auto def = RPG::parse_effect_scaling("heal:INT");
    require_true("parse_effect_scaling(heal:INT)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "heal");
    require_eq_str("scaling", RPG::scaling_stat_to_string(def->scaling), "INT");
}

MOTOR_TEST_CASE(parse_effect_scaling_none_da_none) {
    auto def = RPG::parse_effect_scaling("none");
    require_true("parse_effect_scaling(none)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "none");
}

MOTOR_TEST_CASE(parse_effect_scaling_buit_da_none) {
    auto def = RPG::parse_effect_scaling("");
    require_true("parse_effect_scaling(empty)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "none");
}

MOTOR_TEST_CASE(parse_effect_scaling_desconegut_da_nullopt) {
    auto def = RPG::parse_effect_scaling("efecte_fictici");
    require_true("parse_effect_scaling desconegut retorna nullopt", !def.has_value());
}

// === D-11: compound underscore strings (con_tier, con_con, etc.) ===

MOTOR_TEST_CASE(parse_effect_scaling_compound_con_tier) {
    auto def = RPG::parse_effect_scaling("con_tier");
    require_true("parse_effect_scaling(con_tier)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "damage");
    require_eq_str("scaling", RPG::scaling_stat_to_string(def->scaling), "CON");
}

MOTOR_TEST_CASE(parse_effect_scaling_compound_con_con) {
    auto def = RPG::parse_effect_scaling("con_con");
    require_true("parse_effect_scaling(con_con)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "damage");
    require_eq_str("scaling", RPG::scaling_stat_to_string(def->scaling), "CON");
}

MOTOR_TEST_CASE(parse_effect_scaling_compound_con_int) {
    auto def = RPG::parse_effect_scaling("con_int");
    require_true("parse_effect_scaling(con_int)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "damage");
    require_eq_str("scaling", RPG::scaling_stat_to_string(def->scaling), "CON");
}

// === D-11: execute_effect with caster scaling ===

MOTOR_TEST_CASE(execute_effect_damage_scales_with_caster_int) {
    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 8, 0};  // INT=8 (max stat rank)
    caster.tier = 1;
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::DAMAGE;
    def.scaling = RPG::ScalingStat::INT;
    ScriptedRng rng({});
    // base=10, INT=8 → scaled = 10 * (1 + 8/100) = 10.8 → 10
    bool ok = RPG::execute_effect(def, target, rng, 10, &caster);
    require_true("execute_effect retorna true", ok);
    require_eq("dany escalat amb INT=8", target.health(), 40);
}

MOTOR_TEST_CASE(execute_effect_heal_scales_with_caster_con) {
    RPG::CharacterSheet caster;
    caster.baseStats = {8, 0, 0, 0};  // CON=8 (max stat rank)
    caster.tier = 1;
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::HEAL;
    def.scaling = RPG::ScalingStat::CON;
    ScriptedRng rng({});
    // base=10, CON=8 → scaled = 10 * (1 + 8/100) = 10.8 → 10
    bool ok = RPG::execute_effect(def, target, rng, 10, &caster);
    require_true("execute_effect retorna true", ok);
    require_eq("cura escalada amb CON=8", target.health(), 60);
}

MOTOR_TEST_CASE(execute_effect_damage_no_caster_returns_base) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::DAMAGE;
    def.scaling = RPG::ScalingStat::INT;
    ScriptedRng rng({});
    // No caster → base magnitude returned (no scaling)
    bool ok = RPG::execute_effect(def, target, rng, 20);
    require_true("execute_effect retorna true", ok);
    require_eq("dany sense caster", target.health(), 30);
}

// === effect_family string conversion ===

MOTOR_TEST_CASE(effect_family_from_string_caseless) {
    require_eq_str("DAMAGE", RPG::effect_family_to_string(RPG::effect_family_from_string("DAMAGE")), "damage");
    require_eq_str("Damage", RPG::effect_family_to_string(RPG::effect_family_from_string("Damage")), "damage");
    require_eq_str("damage", RPG::effect_family_to_string(RPG::effect_family_from_string("damage")), "damage");
}

MOTOR_TEST_CASE(to_string_effect_family_values) {
    require_eq_str("none", RPG::effect_family_to_string(RPG::EffectFamily::NONE), "none");
    require_eq_str("damage", RPG::effect_family_to_string(RPG::EffectFamily::DAMAGE), "damage");
    require_eq_str("heal", RPG::effect_family_to_string(RPG::EffectFamily::HEAL), "heal");
    require_eq_str("condition", RPG::effect_family_to_string(RPG::EffectFamily::CONDITION), "condition");
    require_eq_str("summon", RPG::effect_family_to_string(RPG::EffectFamily::SUMMON), "summon");
}

// === scale_magnitude ===

MOTOR_TEST_CASE(scale_magnitude_none_scaling_retorna_base) {
    RPG::SkillEffectDef def;
    def.scaling = RPG::ScalingStat::NONE;
    int result = RPG::scale_magnitude(def, nullptr, 10);
    require_eq("scale_magnitude with NONE scaling", result, 10);
}

// === execute_effect ===

MOTOR_TEST_CASE(execute_effect_damage_aplica_dany) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::DAMAGE;
    def.scaling = RPG::ScalingStat::NONE;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("execute_effect retorna true", ok);
    require_eq("dany aplicat", target.health(), 40);
}

MOTOR_TEST_CASE(execute_effect_heal_cura) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::HEAL;
    def.scaling = RPG::ScalingStat::NONE;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("execute_effect retorna true", ok);
    require_eq("cura aplicada", target.health(), 60);
}

MOTOR_TEST_CASE(execute_effect_none_no_fa_res) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::NONE;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("execute_effect NONE retorna true", ok);
    require_eq("salut sense canvi", target.health(), 50);
}

// === SkillExecutor effect reporting ===

MOTOR_TEST_CASE(skill_executor_reporta_efecte_supportat) {
    RPG::SkillDefinition def;
    def.id = "test_damage";
    def.effect_scaling = "damage";

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    RPG::CharacterSheet targetSheet;
    targetSheet.baseStats = {0, 0, 0, 0};
    targetSheet.tier = 1;
    ScriptedRng rng({});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.target = &targetSheet;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_true("efecte suportat", out.effect_supported);
    require_eq_str("effect_family", out.effect_family, "damage");
}

MOTOR_TEST_CASE(skill_executor_reporta_efecte_desconegut) {
    RPG::SkillDefinition def;
    def.id = "test_fictici";
    def.effect_scaling = "efecte_fictici";

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    RPG::CharacterSheet targetSheet;
    targetSheet.baseStats = {0, 0, 0, 0};
    targetSheet.tier = 1;
    ScriptedRng rng({});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.target = &targetSheet;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_true("efecte no suportat", !out.effect_supported);
    require_eq_str("effect_family", out.effect_family, "efecte_fictici");
    require_true("efecte te diagnostic", !out.effect_diagnostic.empty());
}

MOTOR_TEST_CASE(skill_executor_none_effect_scaling_passa) {
    RPG::SkillDefinition def;
    def.id = "test_none";
    def.effect_scaling = "none";

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    RPG::CharacterSheet targetSheet;
    targetSheet.baseStats = {0, 0, 0, 0};
    targetSheet.tier = 1;
    ScriptedRng rng({});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.target = &targetSheet;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_true("efecte none suportat", out.effect_supported);
}

MOTOR_TEST_CASE(skill_executor_summon_exito_genera_pending_summon) {
    RPG::SkillDefinition def;
    def.id = "invocar_slime";
    def.granted_summon_id = "slime_verde";
    def.effect_scaling = "none";

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    // 1 dado forçat: un 6 = 1 exito -> SUCCESS (un dado no fa critic) -> summon
    ScriptedRng rng({6});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 1;
    ctx.override_target_dice = 0;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_eq_str("pending_summon_id", out.pending_summon_id, "slime_verde");
    require_true("final_degree >= SUCCESS",
                 static_cast<int>(out.final_degree) >= static_cast<int>(RPG::Degree::SUCCESS));
}

MOTOR_TEST_CASE(skill_executor_summon_pifia_no_genera_summon) {
    RPG::SkillDefinition def;
    def.id = "invocar_slime";
    def.granted_summon_id = "slime_verde";
    def.effect_scaling = "none";

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    // 2 daus forçats: dos 1s = pifia -> BOTCH -> sense summon
    ScriptedRng rng({1, 1});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = 0;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_eq_str("pending_summon_id buit", out.pending_summon_id, "");
    require_true("final_degree == BOTCH",
                 out.final_degree == RPG::Degree::BOTCH);
}

// === D-09: condition pipeline (apply_condition_id) ===

// D-11 O3: l'executor escala final_magnitude per l'estat que declara la
// carta quan hi ha un CharacterSheet* com a caster. Magnitud 0 (BOTCH) es
// manté en 0; sense caster o amb stat 0 no hi ha scaling. L'ordre és
// escala (per stat) i després x1.5 si CRITICAL per sisos.
MOTOR_TEST_CASE(skill_executor_escala_final_magnitude_amb_caster) {
    RPG::SkillDefinition def;
    def.id = "foc_escalat";
    def.effect_scaling = "INT";
    def.magnitude_by_degree[0] = 0;
    def.magnitude_by_degree[1] = 50;
    def.magnitude_by_degree[2] = 100;
    def.magnitude_by_degree[3] = 200;

    RPG::CharacterSheet caster;
    caster.tier = 1;

    RPG::ExecutionContext ctx;
    ctx.skill = &def;
    ctx.is_player_attacking = true;

    // SUCCESS (un 5 = mig exit sense rival) amb INT=8: 100*1.08 -> 108.
    {
        caster.baseStats = {0, 0, 8, 0};  // INT=8 (max stat rank)
        ctx.caster = &caster;
        ctx.override_base_dice = 1;
        ctx.override_target_dice = 0;
        ScriptedRng dado({5});
        ctx.rng = &dado;
        auto out = RPG::SkillExecutor::execute(ctx);
        require_eq("SUCCESS escalat INT=8", out.final_magnitude, 108);
    }
    // PARTIAL (un 4 = 0 exits) amb INT=8: 50*1.08 -> 54.
    {
        ctx.override_base_dice = 1;
        ScriptedRng dado({4});
        ctx.rng = &dado;
        auto out = RPG::SkillExecutor::execute(ctx);
        require_eq("PARTIAL escalat INT=8", out.final_magnitude, 54);
    }
    // INT=0: sense scaling -> magnitud base 100.
    {
        caster.baseStats = {0, 0, 0, 0};
        ctx.override_base_dice = 1;
        ScriptedRng dado({5});
        ctx.rng = &dado;
        auto out = RPG::SkillExecutor::execute(ctx);
        require_eq("SUCCESS sense stat", out.final_magnitude, 100);
    }
    // Sense caster (cami legat, ICombatant!): no scaling -> 100.
    {
        ctx.caster = nullptr;
        ctx.override_base_dice = 1;
        ScriptedRng dado({5});
        ctx.rng = &dado;
        auto out = RPG::SkillExecutor::execute(ctx);
        require_eq("SUCCESS sense caster CharacterSheet", out.final_magnitude, 100);
    }
    // BOTCH (dos 1) amb INT=8: magnitud 0 es manté en 0.
    {
        caster.baseStats = {0, 0, 8, 0};
        ctx.caster = &caster;
        ctx.override_base_dice = 2;
        ScriptedRng dado({1, 1});
        ctx.rng = &dado;
        auto out = RPG::SkillExecutor::execute(ctx);
        require_eq("BOTCH mag 0 queda 0", out.final_magnitude, 0);
    }
    // CRITICAL per sisos amb INT=8: escala abans del x1.5.
    // 200*1.08=216; 216*1.5=324. (Dos 6: un de sol no fa critic, D0013.)
    {
        ctx.override_base_dice = 2;
        ScriptedRng dado({6, 6});
        ctx.rng = &dado;
        auto out = RPG::SkillExecutor::execute(ctx);
        require_true("CRITICAL marca sisos",
                     out.final_degree == RPG::Degree::CRITICAL &&
                     out.was_all_sixes_critical);
        require_eq("CRITICAL escala i x1.5", out.final_magnitude, 324);
    }
}

MOTOR_TEST_CASE(skill_executor_condition_applied_on_success) {
    RPG::SkillDefinition def;
    def.id = "llencament_de_foc";
    def.effect_scaling = "none";
    def.apply_condition_id = "burning";
    def.apply_condition_min_degree = RPG::Degree::SUCCESS;
    def.condition_duration_rounds = 3;

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    ScriptedRng rng({6, 3});  // 2 daus: 1 exit + 0 = 1 exit -> SUCCESS (no critical)
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_eq_str("pending_condition_id", out.pending_condition_id, "burning");
    require_eq("pending_condition_rounds", out.pending_condition_rounds, 3);
}

MOTOR_TEST_CASE(skill_executor_condition_not_applied_below_min_degree) {
    RPG::SkillDefinition def;
    def.id = "golpe_con_hielo";
    def.effect_scaling = "none";
    def.apply_condition_id = "frozen";
    def.apply_condition_min_degree = RPG::Degree::SUCCESS;
    def.condition_duration_rounds = 2;

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    // 1 dado: {4} -> 0 exits, no pifia -> PARTIAL (per sota de SUCCESS)
    ScriptedRng rng({4});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 1;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_eq_str("pending_condition_id buit", out.pending_condition_id, "");
    require_eq("pending_condition_rounds zero", out.pending_condition_rounds, 0);
}

MOTOR_TEST_CASE(skill_executor_condition_empty_id_no_condition) {
    RPG::SkillDefinition def;
    def.id = "ataque_normal";
    def.effect_scaling = "none";
    def.apply_condition_id = "";
    def.condition_duration_rounds = 3;

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    ScriptedRng rng({6, 3});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_eq_str("pending_condition_id buit", out.pending_condition_id, "");
}

MOTOR_TEST_CASE(skill_executor_condition_applied_on_critical) {
    RPG::SkillDefinition def;
    def.id = "golpe_devastador";
    def.effect_scaling = "none";
    def.apply_condition_id = "stunned";
    def.apply_condition_min_degree = RPG::Degree::SUCCESS;
    def.condition_duration_rounds = 1;

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    // 2 daus: {6, 6} -> dos 6 (D0013) -> CRITICAL
    ScriptedRng rng({6, 6});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_eq_str("pending_condition_id", out.pending_condition_id, "stunned");
    require_eq("pending_condition_rounds", out.pending_condition_rounds, 1);
    require_true("final_degree == CRITICAL", out.final_degree == RPG::Degree::CRITICAL);
}

MOTOR_TEST_CASE(skill_executor_condition_duration_zero_defaults_to_one) {
    RPG::SkillDefinition def;
    def.id = "toxicidad";
    def.effect_scaling = "none";
    def.apply_condition_id = "poisoned";
    def.apply_condition_min_degree = RPG::Degree::SUCCESS;
    def.condition_duration_rounds = 0;

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    ScriptedRng rng({6, 3});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_eq_str("pending_condition_id", out.pending_condition_id, "poisoned");
    require_eq("pending_condition_rounds defaults to 1", out.pending_condition_rounds, 1);
}

// === Effect family: condition via parse_effect_scaling ===

MOTOR_TEST_CASE(parse_effect_scaling_reconeix_condition) {
    auto def = RPG::parse_effect_scaling("condition");
    require_true("parse_effect_scaling(condition)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "condition");
}

MOTOR_TEST_CASE(parse_effect_scaling_condition_with_scaling) {
    auto def = RPG::parse_effect_scaling("condition:CON");
    require_true("parse_effect_scaling(condition:CON)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "condition");
    require_eq_str("scaling", RPG::scaling_stat_to_string(def->scaling), "CON");
}

MOTOR_TEST_CASE(execute_effect_condition_returns_false) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::CONDITION;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("execute_effect CONDITION returns false", !ok);
    require_eq("health unchanged", target.health(), 50);
}

// === D-11 Wave 4: untested effect families ===
// Each family is parseable and execute_effect returns true (implemented).
// Actual effect implementation is a separate task; here we verify
// the effect is recognized and returns true (acknowledged).

MOTOR_TEST_CASE(execute_effect_buff_returns_true) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::BUFF;
    def.scaling = RPG::ScalingStat::NONE;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("BUFF returns true (implemented)", ok);
    require_eq("health unchanged after BUFF (placeholder)", target.health(), 50);
}

MOTOR_TEST_CASE(execute_effect_debuff_returns_true) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::DEBUFF;
    def.scaling = RPG::ScalingStat::NONE;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("DEBUFF returns true (implemented)", ok);
    require_eq("health unchanged after DEBUFF (placeholder)", target.health(), 50);
}

MOTOR_TEST_CASE(execute_effect_extra_dice_returns_true) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::EXTRA_DICE;
    def.scaling = RPG::ScalingStat::NONE;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("EXTRA_DICE returns true (implemented)", ok);
    require_eq("health unchanged after EXTRA_DICE (placeholder)", target.health(), 50);
}

MOTOR_TEST_CASE(execute_effect_reroll_returns_true) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::REROLL;
    def.scaling = RPG::ScalingStat::NONE;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("REROLL returns true (implemented)", ok);
    require_eq("health unchanged after REROLL (placeholder)", target.health(), 50);
}

MOTOR_TEST_CASE(execute_effect_steal_returns_true) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::STEAL;
    def.scaling = RPG::ScalingStat::NONE;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("STEAL returns true (implemented)", ok);
    require_eq("health unchanged after STEAL (placeholder)", target.health(), 50);
}

MOTOR_TEST_CASE(execute_effect_custom_returns_true) {
    DummyTarget target;
    RPG::SkillEffectDef def;
    def.family = RPG::EffectFamily::CUSTOM;
    def.scaling = RPG::ScalingStat::NONE;
    ScriptedRng rng({});
    bool ok = RPG::execute_effect(def, target, rng, 10);
    require_true("CUSTOM returns true (implemented)", ok);
    require_eq("health unchanged after CUSTOM (placeholder)", target.health(), 50);
}

MOTOR_TEST_CASE(parse_effect_family_buff_recognized) {
    auto def = RPG::parse_effect_scaling("buff");
    require_true("parse_effect_scaling(buff)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "buff");
}

MOTOR_TEST_CASE(parse_effect_family_debuff_recognized) {
    auto def = RPG::parse_effect_scaling("debuff");
    require_true("parse_effect_scaling(debuff)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "debuff");
}

MOTOR_TEST_CASE(parse_effect_family_extra_dice_recognized) {
    auto def = RPG::parse_effect_scaling("extra_dice");
    require_true("parse_effect_scaling(extra_dice)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "extra_dice");
}

MOTOR_TEST_CASE(parse_effect_family_reroll_recognized) {
    auto def = RPG::parse_effect_scaling("reroll");
    require_true("parse_effect_scaling(reroll)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "reroll");
}

MOTOR_TEST_CASE(parse_effect_family_steal_recognized) {
    auto def = RPG::parse_effect_scaling("steal");
    require_true("parse_effect_scaling(steal)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "steal");
}

MOTOR_TEST_CASE(parse_effect_family_custom_recognized) {
    auto def = RPG::parse_effect_scaling("custom");
    require_true("parse_effect_scaling(custom)", def.has_value());
    require_eq_str("family", RPG::effect_family_to_string(def->family), "custom");
}

MOTOR_TEST_CASE(skill_executor_summon_empty_id_no_genera_res) {
    RPG::SkillDefinition def;
    def.id = "ataque_normal";
    def.granted_summon_id = "";
    def.effect_scaling = "none";

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    RPG::CharacterSheet targetSheet;
    targetSheet.baseStats = {0, 0, 0, 0};
    targetSheet.tier = 1;
    ScriptedRng rng({6});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.target = &targetSheet;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_eq_str("pending_summon_id buit", out.pending_summon_id, "");
}

int main() { return motor_test::runAllTests(); }
