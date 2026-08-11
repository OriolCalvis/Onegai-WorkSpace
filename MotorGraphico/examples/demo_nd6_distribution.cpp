#include <cstdio>
#include <string>

#include "RPG/CharacterSheet.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/EnhanceHooks.h"
#include "RPG/RandomEngine.h"
#include "RPG/Definitions/SkillDefinition.h"

using namespace RPG;

int main() {
    const int TRIALS = 20000;
    Xoroshiro128p rng;
    printf("DicePoolEngine Fase 0 —— %d tirades per pool, seed=%llu\n\n", TRIALS, (unsigned long long)rng.seed());

    struct Case {
        const char* name; int N; DiceMod mod; float cd;
    };
    const Case CASES[] = {
        {"N=1 NORMAL contra CD 0.0", 1, DiceMod::NORMAL, 0.0f},
        {"N=2 NORMAL contra CD 0.5 (CA 11)", 2, DiceMod::NORMAL, 0.5f},
        {"N=3 NORMAL contra CD 1.0 (CA 12)", 3, DiceMod::NORMAL, 1.0f},
        {"N=4 NORMAL contra CD 2.0 (CA 14, molt difícil)", 4, DiceMod::NORMAL, 2.0f},
        {"N=4 ADVANTAGE contra CD 2.0 (CA 14)", 4, DiceMod::ADVANTAGE, 2.0f},
        {"N=4 DISADV contra CD 2.0 (fatiga + equip incompatible)", 4, DiceMod::DISADVANTAGE, 2.0f},
        {"N=0 (clamp a 1d6 DISADV) contra CD 0.0", 0, DiceMod::NORMAL, 0.0f}
    };
    const int NCASES = sizeof(CASES) / sizeof(CASES[0]);

    for (int c = 0; c < NCASES; ++c) {
        int counts[4] = {0, 0, 0, 0};
        int crits_all_sixes = 0;
        double avg_successes = 0.0;

        for (int t = 0; t < TRIALS; ++t) {
            PoolResult r = DicePoolEngine::roll_pool(CASES[c].N, CASES[c].mod, rng);
            CheckOutcome out = DicePoolEngine::resolve_against_cd(r, CASES[c].cd);
            counts[static_cast<int>(out.degree)]++;
            if (r.critical) crits_all_sixes++;
            avg_successes += r.successes;
        }
        avg_successes /= TRIALS;

        printf("▶ %s\n", CASES[c].name);
        printf("   Botch    %5.2f%% | Partial  %5.2f%% | Success %5.2f%% | Critical %5.2f%%\n",
               100.0 * counts[0] / TRIALS,
               100.0 * counts[1] / TRIALS,
               100.0 * counts[2] / TRIALS,
               100.0 * counts[3] / TRIALS);
        printf("   èxits mitjos: %.3f | all-sixes: %5.3f%%\n\n", avg_successes, 100.0 * crits_all_sixes / TRIALS);
    }

    // — Test ExecutionContext PASSIVOS (Fase 1) —
    printf("=== ExecutionContext Enhancements test (Fase 1) ===\n");
    const int TR2 = 15000;
    const Stat stat_of_test = Stat::DES;
    ClassDefinition cls;
    cls.baseHealth = 14; cls.id = "guerrer";

    CharacterSheet caster{};
    caster.classId = "guerrer";
    caster.raceId = "huma";
    caster.tier = 1;
    caster.equipBonuses[static_cast<int>(Stat::DES)] = 3; // DES = 3 (0 base + 3 equip)
    caster.equipArmorBonus = 3; // CA física 10+3+3 = 16

    CharacterSheet target{};
    target.classId = "llop";
    target.equipBonuses[static_cast<int>(Stat::DES)] = 2; // DES=2
    target.equipArmorBonus = 0;

    SkillDefinition basic_attack;
    basic_attack.id = "atac_basic";
    basic_attack.name = "Atac Bàsic";
    basic_attack.casting_stat = Stat::DES;
    basic_attack.save_attribute = SaveAttribute::NONE;
    basic_attack.magnitude_by_degree[0] = 0;
    basic_attack.magnitude_by_degree[1] = 2;
    basic_attack.magnitude_by_degree[2] = 4;
    basic_attack.magnitude_by_degree[3] = 8;

    auto run_ctx = [&](const char* label, ContextBonusFlag f) {
        int counts[4] = {0,0,0,0};
        int mag_avg = 0;
        for (int i = 0; i < TR2; ++i) {
            ExecutionContext ctx{};
            ctx.caster = &caster; ctx.target = &target; ctx.skill = &basic_attack;
            ctx.rng = &rng;
            ctx.flags = f;
            ctx.is_player_attacking = true;
            SkillExecutor::Out o = SkillExecutor::execute(ctx);
            counts[static_cast<int>(o.final_degree)]++;
            mag_avg += o.final_magnitude;
        }
        printf("- %s:\n   B/P/S/C %5.2f/%5.2f/%5.2f/%5.2f%%, magnitud mitja %.2f\n", label,
               100.0*counts[0]/TR2, 100.0*counts[1]/TR2,
               100.0*counts[2]/TR2, 100.0*counts[3]/TR2, (double)mag_avg/TR2);
    };

    run_ctx("Atac bàsic DES 3 vs CA 12 (target) [estàndard]", ContextBonusFlag::NONE);
    run_ctx("...+ FLANKED (+1)", ContextBonusFlag::FLANKED_ENEMY);
    run_ctx("...+ FLANKED + HIGH_GROUND (+2)", ContextBonusFlag::FLANKED_ENEMY | ContextBonusFlag::HIGH_GROUND);
    run_ctx("...+ FATIGUED (DISADV)", ContextBonusFlag::FATIGUED);
    run_ctx("...+ INSPIRED (+1 + PARTIAL→SUCCESS)", ContextBonusFlag::INSPIRED);

    printf("\nNd6 + Enhance Layer: COHÉRENT ✔\n");
    return 0;
}
