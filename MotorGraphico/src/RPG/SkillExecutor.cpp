#include <array>
#include <cstdio>
#include <string>
#include <vector>

#include "RPG/CharacterSheet.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/EnhanceHooks.h"
#include "RPG/RandomEngine.h"
#include "RPG/Definitions/SkillDefinition.h"

namespace RPG {

static inline uint32_t flag_bits(ContextBonusFlag f) {
    return static_cast<uint32_t>(f);
}

static inline void calc_passive_bonuses_pre_roll(ExecutionContext& ctx,
                                              std::vector<std::string>& applied) {
    if (flag_bits(ctx.flags) == 0u) return;

    if (has_flag(ctx.flags, ContextBonusFlag::FLANKED_ENEMY))    { ctx.extra_dice += 1; applied.push_back("FLANKED +1"); }
    if (has_flag(ctx.flags, ContextBonusFlag::HIGH_GROUND))      { ctx.extra_dice += 1; applied.push_back("HIGH_GROUND +1"); }
    if (has_flag(ctx.flags, ContextBonusFlag::SURPRISED_ENEMY))  {
        ctx.force_dice_mod = DiceMod::ADVANTAGE;
        applied.push_back("SURPRISE ADV");
    }
    if (has_flag(ctx.flags, ContextBonusFlag::INSPIRED))         {
        ctx.can_upgrade_partial_to_success = true;
        ctx.extra_dice += 1;
        applied.push_back("INSPIRED +1 +PART→SUC");
    }
    if (has_flag(ctx.flags, ContextBonusFlag::FATIGUED))         {
        ctx.force_dice_mod = DiceMod::DISADVANTAGE;
        applied.push_back("FATIGUED DISADV");
    }
    if (has_flag(ctx.flags, ContextBonusFlag::EQUIP_INCOMPATIBLE)) {
        if (ctx.force_dice_mod == DiceMod::NORMAL) ctx.force_dice_mod = DiceMod::DISADVANTAGE;
        ctx.post_roll_success_bonus -= 0.5f;
        applied.push_back("EQUIP_INCOMPAT -0.5");
    }
    if (has_flag(ctx.flags, ContextBonusFlag::ENEMY_BLEEDING))   {
        ctx.post_roll_success_bonus += 0.5f;
        applied.push_back("BLEED +0.5");
    }
    if (has_flag(ctx.flags, ContextBonusFlag::CASTER_LOW_HP))    {
        ctx.post_roll_success_bonus += 0.5f;
        applied.push_back("LOW HP BERSERK +0.5");
    }
}

SkillExecutor::Out SkillExecutor::execute(ExecutionContext& ctx) {
    Out out{};
    if (!ctx.rng) return out;
    if (!ctx.skill) return out;

    std::vector<std::string> applied;

    // —— PASO 1: Contexto pre-tirada (bonificadores pasivos) ——
    calc_passive_bonuses_pre_roll(ctx, applied);

    // —— PASO 2: Timing QTE (si está disponible) ——
    if (ctx.qte) {
        if (!ctx.is_player_attacking) {
            QteResult r = ctx.qte->do_reaction_dodge_qte(1200, ctx.skill->id);
            if (r == QteResult::GOOD)    { ctx.post_roll_success_bonus += 0.5f; applied.push_back("DODGE GOOD +0.5"); }
            if (r == QteResult::PERFECT) { ctx.force_dice_mod = DiceMod::ADVANTAGE; applied.push_back("DODGE PERFECT ADV"); }
        } else {
            QteResult r = ctx.qte->do_attack_timing_qte(1000, ctx.skill->id);
            if (r == QteResult::GOOD)    { ctx.post_roll_success_bonus += 0.5f; applied.push_back("ATTACK GOOD +0.5"); }
            if (r == QteResult::PERFECT) { ctx.extra_dice += 1; ctx.perfect_critical_confirm_allowed = true; applied.push_back("ATTACK PERFECT +1 +CONF CRIT"); }
        }
    }

    // —— PASO 3: SIEMPRE: Tirar pool Nd6 (INVIOLABLE) ——
    int base_dice = 1;
    if (ctx.override_base_dice >= 0) {
        base_dice = ctx.override_base_dice;
    } else if (ctx.caster) {
        base_dice = ctx.caster->stat(ctx.skill->casting_stat);
    }
    int N = base_dice + ctx.extra_dice;
    out.pool = DicePoolEngine::roll_pool(N, ctx.force_dice_mod, *ctx.rng);

    // —— PASO 4: Post-process successes con bonus de timing/passives ——
    float eff = out.pool.successes + ctx.post_roll_success_bonus;
    if (eff < 0.0f) eff = 0.0f;
    out.effective_successes = eff;

    // —— PASO 5: Determinar CD contra qué resolver ——
    int def_int = 10;
    if (ctx.override_target_defense >= 0) {
        def_int = ctx.override_target_defense;
    } else if (ctx.target) {
        // CharacterSheet API: defenses() requiere ClassDefinition*.
        // Para cálculos sin clase concreta (sin multiclass, sin regla de
        // armadura pesada CON alternativa) usamos un cls vacío: la
        // fórmula que se aplica es la genérica 10+DES+eq que es justo lo
        // que esperamos cuando solo nos llega un CharacterSheet* sin
        // contexto de clase.
        ClassDefinition cls;
        cls.id = "";
        cls.name = "";
        DefenseBlock db = ctx.target->defenses(&cls);
        if (ctx.skill->save_attribute != SaveAttribute::NONE) {
            switch (ctx.skill->save_attribute) {
                case SaveAttribute::CON: def_int = db.get(Defense::PHYSICAL_SAVE); break;
                case SaveAttribute::DES: def_int = 10 + ctx.target->stat(Stat::DES); break;
                case SaveAttribute::INT: def_int = db.get(Defense::WILL_SAVE); break;
                case SaveAttribute::CAR: def_int = 10 + ctx.target->stat(Stat::CAR); break;
                default:                  def_int = db.get(Defense::ARMOR_CLASS); break;
            }
        } else {
            def_int = db.get(Defense::ARMOR_CLASS);
        }
    }
    float cd = ctx.skill->override_cd_if_save > 0.0f
                 ? ctx.skill->override_cd_if_save
                 : DicePoolEngine::defense_to_cd(def_int);

    // —— PASO 5.5: Resolver grado contra CD ——
    PoolResult eff_pool;
    eff_pool.successes = eff;
    eff_pool.botch = (eff == 0.0f);
    eff_pool.critical = out.pool.critical;
    CheckOutcome check = DicePoolEngine::resolve_against_cd(eff_pool, cd);
    Degree d = check.degree;

    // —— PASO 6: Post-process upgrades (BOTCH→PARTIAL, PARTIAL→SUCCESS) ——
    if (d == Degree::BOTCH && ctx.can_upgrade_botch_to_partial && eff > 0.0f) {
        d = Degree::PARTIAL;
        applied.push_back("BOTCH→PARTIAL (prot.)");
    }
    if (d == Degree::PARTIAL && ctx.can_upgrade_partial_to_success) {
        d = Degree::SUCCESS;
        applied.push_back("PARTIAL→SUCCESS (inspi.)");
    }

    // —— PASO 7: Critical confirm (timing PERFECT + just SUCCESS) ——
    if (ctx.perfect_critical_confirm_allowed && d == Degree::SUCCESS) {
        int confirm = ctx.rng->roll_d6();
        if (confirm == 6) {
            d = Degree::CRITICAL;
            applied.push_back("CRIT CONFIRMED! 6");
        }
    }

    // —— PASO 8: Extra bonus crítico all-sixes × 1.5 ——
    if (out.pool.critical && d == Degree::CRITICAL) {
        out.was_all_sixes_critical = true;
        applied.push_back("ALL-SIXES EXTRA ×1.5");
    }

    out.final_degree = d;

    // —— PASO 9: Calcular magnitud final (dany/cura) ——
    out.final_magnitude = ctx.skill->magnitude(d);
    if (out.was_all_sixes_critical) {
        out.final_magnitude = static_cast<int>(out.final_magnitude * 1.5f);
    }

    out.applied_enhancements = std::move(applied);

    return out;
}

} // namespace RPG
