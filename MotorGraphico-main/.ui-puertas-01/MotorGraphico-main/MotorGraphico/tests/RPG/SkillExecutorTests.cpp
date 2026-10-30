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

#include "RPG/DicePoolEngine.h"
#include "RPG/EnhanceHooks.h"
#include "RPG/SkillEffect.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/CharacterSheet.h"
#include "ScriptedRng.h"

#include <string>
#include <vector>

namespace {

void require_eq(const std::string& what, int got, int expected) {
    if (got != expected) throw std::runtime_error(
        what + ": " + std::to_string(got) + " != " + std::to_string(expected));
}

void require_eq_str(const std::string& what, const std::string& got, const std::string& expected) {
    if (got != expected) throw std::runtime_error(
        what + ": '" + got + "' != '" + expected + "'");
}

void require_true(const std::string& what, bool ok) {
    if (!ok) throw std::runtime_error(what + ": expected true");
}

void require_eq_float(const std::string& what, float got, float expected) {
    if (got != expected) throw std::runtime_error(
        what + ": " + std::to_string(got) + " != " + std::to_string(expected));
}

}  // namespace

// === Context bonus flags ===

MOTOR_TEST_CASE(context_bonus_flanked_adds_one_die) {
    RPG::SkillDefinition def;
    def.id = "ataque_flanquejat";
    def.effect_scaling = "none";
    def.magnitude_by_degree[0] = 0.0f;
    def.magnitude_by_degree[1] = 2.0f;
    def.magnitude_by_degree[2] = 4.0f;
    def.magnitude_by_degree[3] = 8.0f;

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;

    // 1 base die + 1 flanked = 2 dice total. Rolls: {6, 3} = 1 exit -> SUCCESS
    ScriptedRng rng({6, 3});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 1;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.flags = RPG::ContextBonusFlag::FLANKED_ENEMY;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_eq("extra_dice from FLANKED", 1, ctx.extra_dice);
    require_true("2 dice consumed (1 base + 1 flanked)", rng.consumed() == 2);
    require_true("final_degree == SUCCESS", out.final_degree == RPG::Degree::SUCCESS);
}

MOTOR_TEST_CASE(context_bonus_inspired_upgrades_partial) {
    RPG::SkillDefinition def;
    def.id = "habilitat_inspirada";
    def.effect_scaling = "none";

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;

    // 1 base die + 1 inspired = 2 dice. Rolls: {3, 2} = 0 exits, no pifia -> PARTIAL
    // But INSPIRED sets can_upgrade_partial_to_success, so final = SUCCESS
    ScriptedRng rng({3, 2});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 1;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.flags = RPG::ContextBonusFlag::INSPIRED;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_true("INSPIRED sets upgrade flag", ctx.can_upgrade_partial_to_success);
    require_eq("extra_dice from INSPIRED", 1, ctx.extra_dice);
    require_true("PARTIAL upgraded to SUCCESS", out.final_degree == RPG::Degree::SUCCESS);
    require_true("upgrade recorded in applied_enhancements",
                 out.applied_enhancements.size() >= 1);
}

// === Opposed rolls ===

MOTOR_TEST_CASE(opposed_roll_attacker_wins) {
    RPG::SkillDefinition def;
    def.id = "ataque_enfrentat";
    def.effect_scaling = "none";

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;

    // Attacker: 2 dice {6, 3} = 1 exit
    // Defender: 1 die {4} = 0 exits
    // Attacker wins by 1 -> SUCCESS
    ScriptedRng rng({6, 3, 4});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = 1;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_true("opposed roll", out.opposed);
    require_true("attacker wins -> SUCCESS", out.final_degree == RPG::Degree::SUCCESS);
    require_eq_float("attacker successes", out.effective_successes, 1.0f);
}

MOTOR_TEST_CASE(opposed_roll_defender_wins) {
    RPG::SkillDefinition def;
    def.id = "ataque_rebutjat";
    def.effect_scaling = "none";

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;

    // Attacker: 1 die {3} = 0 exits
    // Defender: 2 dice {6, 4} = 1 exit
    // Defender wins -> BOTCH
    ScriptedRng rng({3, 6, 4});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 1;
    ctx.override_target_dice = 2;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_true("opposed roll", out.opposed);
    require_true("defender wins -> BOTCH", out.final_degree == RPG::Degree::BOTCH);
}

// === Combined pipelines ===

MOTOR_TEST_CASE(combined_condition_summon_effect) {
    RPG::SkillDefinition def;
    def.id = "golpe_mortal_invocador";
    def.effect_scaling = "damage";
    def.apply_condition_id = "bleeding";
    def.apply_condition_min_degree = RPG::Degree::SUCCESS;
    def.condition_duration_rounds = 4;
    def.granted_summon_id = "shadow_wolf";
    def.magnitude_by_degree[0] = 0.0f;
    def.magnitude_by_degree[1] = 2.0f;
    def.magnitude_by_degree[2] = 6.0f;
    def.magnitude_by_degree[3] = 10.0f;

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;

    // 2 dados: {6, 6} = 2 éxitos y dos 6 -> CRITICAL (D0013)
    // (un dado nunca es crítico)
    // All 3 pipelines should fire: condition + summon + effect
    ScriptedRng rng({6, 6});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_true("CRITICAL con dos 6", out.final_degree == RPG::Degree::CRITICAL);
    require_true("all_sixes_critical flag", out.was_all_sixes_critical);

    // Condition pipeline
    require_eq_str("pending_condition_id", out.pending_condition_id, "bleeding");
    require_eq("pending_condition_rounds", out.pending_condition_rounds, 4);

    // Summon pipeline
    require_eq_str("pending_summon_id", out.pending_summon_id, "shadow_wolf");

    // Effect pipeline
    require_eq_str("effect_family", out.effect_family, "damage");
    require_true("effect_supported", out.effect_supported);
}

// === Magnitude calculation ===

MOTOR_TEST_CASE(magnitude_calculation_success_degree) {
    RPG::SkillDefinition def;
    def.id = "golpe_basic";
    def.effect_scaling = "none";
    def.magnitude_by_degree[0] = 0.0f;
    def.magnitude_by_degree[1] = 2.0f;
    def.magnitude_by_degree[2] = 6.0f;
    def.magnitude_by_degree[3] = 10.0f;

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;

    // 2 dados: {6, 3} = 1 éxito y un solo 6 -> SUCCESS, sin crítico
    // (D0013: el crítico pide dos 6).
    ScriptedRng rng({6, 3});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_true("SUCCESS degree", out.final_degree == RPG::Degree::SUCCESS);
    require_eq("magnitude_by_degree[SUCCESS] = 6", out.final_magnitude, 6);
    require_true("not all-sixes critical", !out.was_all_sixes_critical);
}

MOTOR_TEST_CASE(all_sixes_gives_critical_magnitude_1_5x) {
    RPG::SkillDefinition def;
    def.id = "golpe_critico";
    def.effect_scaling = "none";
    def.magnitude_by_degree[0] = 0.0f;
    def.magnitude_by_degree[1] = 2.0f;
    def.magnitude_by_degree[2] = 6.0f;
    def.magnitude_by_degree[3] = 10.0f;

    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;

    // 2 dados: {6, 6} = 2 éxitos y dos 6 -> CRITICAL (D0013)
    // magnitude_by_degree[CRITICAL] = 10, y después × 1.5 = 15
    ScriptedRng rng({6, 6});
    RPG::ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = -1;
    ctx.skill = &def;
    ctx.rng = &rng;

    auto out = RPG::SkillExecutor::execute(ctx);
    require_true("CRITICAL", out.final_degree == RPG::Degree::CRITICAL);
    require_true("all_sixes_critical", out.was_all_sixes_critical);
    require_eq("magnitude 10 × 1.5 = 15", out.final_magnitude, 15);
}

// D0013 (Oriol, 05/10 y 08/10/2026): el crítico pide dos 6, así que un
// dado solo que saca un 6 es un éxito, no un crítico, con rival y sin él;
// con dos seises sí lo es.
MOTOR_TEST_CASE(un_seis_con_un_solo_dado_no_es_critico) {
    ScriptedRng uno({6});
    const RPG::PoolResult solo = RPG::DicePoolEngine::roll_pool(1, RPG::DiceMod::NORMAL, uno);
    require_true("un 6 suma un exito", solo.successes == 1.0f);
    require_true("un dado no es critico", !solo.critical);

    ScriptedRng dos({6, 6});
    const RPG::PoolResult pareja = RPG::DicePoolEngine::roll_pool(2, RPG::DiceMod::NORMAL, dos);
    require_true("dos seises si", pareja.critical);

    ScriptedRng nada(std::vector<int>{});
    require_true("sin dados tampoco", !RPG::DicePoolEngine::roll_pool(0, RPG::DiceMod::NORMAL, nada).critical);

    RPG::SkillDefinition def;
    def.id = "golpe";
    def.effect_scaling = "none";
    def.magnitude_by_degree[0] = 0.0f;
    def.magnitude_by_degree[1] = 5.0f;
    def.magnitude_by_degree[2] = 10.0f;
    def.magnitude_by_degree[3] = 15.0f;
    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    for (const int rival : {-1, 0}) {  // sin rival y contra un rival sin dados
        ScriptedRng rng({6});
        RPG::ExecutionContext ctx;
        ctx.caster = &caster;
        ctx.override_base_dice = 1;
        ctx.override_target_dice = rival;
        ctx.skill = &def;
        ctx.rng = &rng;
        const auto out = RPG::SkillExecutor::execute(ctx);
        require_true("un dado con un 6 es SUCCESS", out.final_degree == RPG::Degree::SUCCESS);
        require_true("sin el x1,5", !out.was_all_sixes_critical);
        require_eq("magnitud de exito", out.final_magnitude, 10);
    }
}

// D0013 — El crítico son dos 6 (ley de Oriol del 08/10/2026):
//   1 dado -> nunca · 2 dados -> los dos en 6 · 3 o más -> basta con dos 6.
// Los bordes: tres dados con un solo 6 no es crítico; con dos, sí, saque lo
// que saque el tercero. La ventaja y la desventaja no cambian el crítico:
// cuentan los 6, no los éxitos.
MOTOR_TEST_CASE(critico_de_dos_seises_D0013) {
    struct Caso { std::vector<int> caras; RPG::DiceMod mod; bool critico; const char* que; };
    const std::vector<Caso> casos = {
        {{6}, RPG::DiceMod::NORMAL, false, "1 dado: un 6 no es critico"},
        {{6, 5}, RPG::DiceMod::NORMAL, false, "2 dados: 6 y 5 no"},
        {{6, 6}, RPG::DiceMod::NORMAL, true, "2 dados: los dos en 6"},
        {{6, 1, 1}, RPG::DiceMod::NORMAL, false, "3 dados con un solo 6 no"},
        {{6, 5, 5}, RPG::DiceMod::ADVANTAGE, false, "3 dados, un 6, con ventaja tampoco"},
        {{6, 6, 1}, RPG::DiceMod::NORMAL, true, "3 dados: dos 6 y un 1"},
        {{1, 6, 6}, RPG::DiceMod::DISADVANTAGE, true, "3 dados: dos 6 con desventaja"},
        {{6, 6, 6}, RPG::DiceMod::NORMAL, true, "3 dados: tres 6"},
        {{6, 2, 3, 6}, RPG::DiceMod::NORMAL, true, "4 dados: dos 6"},
        {{6, 5, 5, 5}, RPG::DiceMod::NORMAL, false, "4 dados: un 6 y tres 5"},
        {{2, 6, 3, 4, 1, 1, 6, 2}, RPG::DiceMod::NORMAL, true, "8 dados: dos 6"},
    };
    for (const Caso& c : casos) {
        ScriptedRng rng(c.caras);
        const RPG::PoolResult r = RPG::DicePoolEngine::roll_pool(
            static_cast<int>(c.caras.size()), c.mod, rng);
        require_true(c.que, r.critical == c.critico);
        require_true(std::string(c.que) + " (es_critico)",
                     RPG::DicePoolEngine::es_critico(c.caras) == c.critico);
        require_true(std::string(c.que) + ": el critico no es pifia", !(r.critical && r.botch));
    }

    // Por el ejecutor: tres dados con dos 6 sin rival -> CRITICAL y el x1,5;
    // con un solo 6 -> SUCCESS.
    RPG::SkillDefinition def;
    def.id = "golpe";
    def.effect_scaling = "none";
    def.magnitude_by_degree[0] = 0.0f;
    def.magnitude_by_degree[1] = 5.0f;
    def.magnitude_by_degree[2] = 10.0f;
    def.magnitude_by_degree[3] = 10.0f;
    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    struct Ejec { std::vector<int> caras; RPG::Degree grado; int magnitud; };
    for (const Ejec& e : {Ejec{{6, 6, 2}, RPG::Degree::CRITICAL, 15},
                          Ejec{{6, 4, 2}, RPG::Degree::SUCCESS, 10}}) {
        ScriptedRng rng(e.caras);
        RPG::ExecutionContext ctx;
        ctx.caster = &caster;
        ctx.override_base_dice = 3;
        ctx.override_target_dice = -1;
        ctx.skill = &def;
        ctx.rng = &rng;
        const auto out = RPG::SkillExecutor::execute(ctx);
        require_true("grado por el ejecutor", out.final_degree == e.grado);
        require_eq("magnitud por el ejecutor", out.final_magnitude, e.magnitud);
    }

    // Dos 6 que pierden la enfrentada no son crítico: manda la diferencia.
    ScriptedRng pierde({6, 6, 1, 6, 6, 6});
    const RPG::PoolResult atk = RPG::DicePoolEngine::roll_pool(3, RPG::DiceMod::NORMAL, pierde);
    const RPG::PoolResult def3 = RPG::DicePoolEngine::roll_pool(3, RPG::DiceMod::NORMAL, pierde);
    require_true("dos 6 que pierden: BOTCH",
                 RPG::DicePoolEngine::opposed_degree(atk, def3) == RPG::Degree::BOTCH);
}

int main() { return motor_test::runAllTests(); }
