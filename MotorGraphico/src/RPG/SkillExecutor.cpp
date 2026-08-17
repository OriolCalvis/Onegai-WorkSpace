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

    // —— PASO 5 (Ed. 3, bitácora 2.17): enfrentada o resolución no resistida ——
    // La CA y la CD se eliminan: si hay defensor, tira su pool (regido por
    // su stat) y se compara (opposed_degree); si no lo hay (curaciones,
    // buffs propios), el grado lo decide la tirada propia sin rival.
    PoolResult eff_pool;
    eff_pool.successes = eff;
    eff_pool.botch = out.pool.botch;  // pifia real: ningún 5/6 y mínimo dos 1es
    eff_pool.critical = out.pool.critical;

    Degree d;
    const bool hay_defensor = (ctx.override_target_dice >= 0) || (ctx.target != nullptr);
    if (!hay_defensor) {
        // No resistida: pifia -> BOTCH; 0 éxitos sin pifia -> PARTIAL
        // (efecto débil, no un fallo contra nadie); todos 6 -> CRITICAL;
        // cualquier otra -> SUCCESS.
        if (eff_pool.botch)                  d = Degree::BOTCH;
        else if (eff <= 0.0f)                d = Degree::PARTIAL;
        else if (out.pool.critical)          d = Degree::CRITICAL;
        else                                 d = Degree::SUCCESS;
    } else {
        int def_dice = ctx.override_target_dice;
        if (def_dice < 0 && ctx.target != nullptr) {
            // Stat gobernante de la defensa (bitácora 2.17, regla 4):
            // hechizo/maldición -> el stat prefijado en la carta
            // (save_attribute); físico -> DES o CON, y para defensas no
            // jugadoras se toma el MAYOR de los dos (cuando defienda el
            // jugador, elegirá él).
            Stat stat_defensa;
            if (ctx.skill->save_attribute != SaveAttribute::NONE) {
                switch (ctx.skill->save_attribute) {
                    case SaveAttribute::CON: stat_defensa = Stat::CON; break;
                    case SaveAttribute::INT: stat_defensa = Stat::INT; break;
                    case SaveAttribute::CAR: stat_defensa = Stat::CAR; break;
                    default:                 stat_defensa = Stat::DES; break;
                }
            } else {
                const int des = ctx.target->stat(Stat::DES);
                const int con = ctx.target->stat(Stat::CON);
                stat_defensa = (con > des) ? Stat::CON : Stat::DES;
            }
            def_dice = ctx.target->stat(stat_defensa);
        }
        if (def_dice < 0) def_dice = 0;
        // Bufos/condiciones del defensor: +1 dados y ventaja/desventaja
        // se cuentan igual que para el atacante (Oriol, 17/08).
        out.defender_pool = DicePoolEngine::roll_pool(
            def_dice + ctx.defender_extra_dice, ctx.defender_dice_mod, *ctx.rng);
        out.opposed = true;
        d = DicePoolEngine::opposed_degree(eff_pool, out.defender_pool);
    }

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
