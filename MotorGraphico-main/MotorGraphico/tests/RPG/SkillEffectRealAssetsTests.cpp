// D-11 (complement sobre assets publicats reals) — Vocabulari d'efectes
// d'habilitat. Els 19 tests originals a tests/RPG/SkillEffectTests.cpp
// utilitzen SkillEffectDef inline sintètic i SkillExecutor amb SkillDefinition
// inline. Aquests tests complementen utilitzant SKILL CATALOG PUBLICAT
// (assets/catalogs/skills.json, 492 entrades) i comproven:
//
// Pla D-11:
//   O1. Totes les 492 habilitats publicades tenen effect_scaling reconegut
//        per parse_effect_scaling() (o be CON/INT/DES/CAR/none: família
//        DAMAGE per defecte; be family:STAT; be scaling desconegut retorna
//        nullopt però no crash).
//   O2. magnitude_by_degree segueix la derivació per tier (2+2·tier) per
//        les 483/492 que no declaren magnitudeByDegree.
//   O3. Cartas por ID: magnitud narrativa cero y daño de exito/critico
//        con tiradas controladas, sin depender del orden del catalogo.
//   O4. Summon publicat i condition aplicada (si alguna skill ho porta).
//
// GL-free. NO toca SkillExecutor.cpp (shared D-09) ni SkillEffectTests.cpp
// (propietat agent anterior).

#include "TestHarness.h"

#include "RPG/SkillEffect.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/EnhanceHooks.h"
#include "RPG/CharacterSheet.h"
#include "ScriptedRng.h"

#include <cmath>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace {

using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::SkillCatalog;
using RPG::Degree;
using RPG::EffectFamily;
using RPG::ExecutionContext;
using RPG::ScalingStat;
using RPG::SkillEffectDef;
using RPG::SkillDefinition;
using RPG::SkillExecutor;
using RPG::Stat;

SkillCatalog loadSkills() {
    SkillCatalog cat;
    auto r = cat.loadFromFile("assets/catalogs/skills.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 400);
    return cat;
}

// Contracte O1: effect_scaling de l'skill publicada → parse_effect_scaling
// retorna SkillEffectDef amb family != NONE per aStat (CON/INT/DES/CAR/none)
// i compound (con_tier/con_int/con_con). Les 9 entrades amb compound
// ara parsegen OK (D-11 Wave 3). 0 nullopt en total (tret que alguna
// future entry porti un scaling totalment desconegut).
MOTOR_TEST_CASE(d11_totes_les_skills_publicades_tienen_parse_effect_scaling_ok) {
    const SkillCatalog cat = loadSkills();

    std::size_t total = 0;
    std::size_t parse_ok = 0;
    std::size_t parse_nullopt = 0;
    std::size_t parse_empty = 0;

    std::set<std::string> nulls;

    cat.forEach([&](const SkillDefinition& s) {
        ++total;
        if (s.effect_scaling.empty()) {
            ++parse_empty;
            return;
        }
        auto def = RPG::parse_effect_scaling(s.effect_scaling);
        if (def.has_value()) {
            ++parse_ok;
        } else {
            ++parse_nullopt;
            nulls.insert(s.effect_scaling);
        }
    });

    MOTOR_CHECK(total >= 400);
    // Totes les entries amb effect_scaling no buit han de parsejar OK.
    // Les 9 entries amb compound "con_tier"/"con_int"/"con_con" ara tenen
    // suport (D-11 Wave 3). Si algun scaling totalment nou apareix i
    // retorna nullopt, cal afegir-lo al vocabulari.
    MOTOR_CHECK(parse_ok + parse_empty == total);
    MOTOR_CHECK(parse_nullopt == 0);
}

// Contracte O2: magnitude_by_degree[SUCCESS] = 2+2·tier (o 1 si tier<0)
// per a les que NO van informar magnitudeByDegree al asset (483/492).
MOTOR_TEST_CASE(d11_magnitude_by_degree_derivat_de_tier_sense_declaracio) {
    const SkillCatalog cat = loadSkills();

    std::size_t comprovats = 0;
    std::size_t derivats_ok = 0;

    cat.forEach([&](const SkillDefinition& s) {
        // Base esperada: 2 + 2*tier
        int tier = s.tier_min;
        if (tier < 1) return;  // només les tiers declarats
        if (tier > 5) return;
        const float expected = static_cast<float>(2 + 2 * tier);
        const float esperat_partial = expected * 0.5f;
        const float esperat_critical = expected * 2.0f;
        ++comprovats;
        if (std::fabs(s.magnitude_by_degree[static_cast<int>(Degree::SUCCESS)] - expected) < 0.001f) {
            if (std::fabs(s.magnitude_by_degree[static_cast<int>(Degree::PARTIAL)] - esperat_partial) < 0.001f &&
                std::fabs(s.magnitude_by_degree[static_cast<int>(Degree::CRITICAL)] - esperat_critical) < 0.001f &&
                std::fabs(s.magnitude_by_degree[static_cast<int>(Degree::BOTCH)] - 0.0f) < 0.001f) {
                ++derivats_ok;
            }
        }
    });

    MOTOR_CHECK(comprovats > 0);
    // La majoria haurien de seguir la derivació (483/492 ho fan).
    MOTOR_CHECK(derivats_ok >= comprovats * 9 / 10);
}

// Contracte O3: SkillExecutor amb una skill publicada + caster retorna
// final_magnitude >= 0 (no crash) i magnitude_by_degree coincideix amb
// skill->magnitude(finish_degree).
MOTOR_TEST_CASE(d11_skill_executor_sobre_skill_publicada_magnitud_coincideix) {
    const SkillCatalog cat = loadSkills();

    // Busquem una skill publicada tier>=1 (491/492 ho són).
    const SkillDefinition* s = nullptr;
    cat.forEach([&](const SkillDefinition& cand) {
        if (s) return;
        if (cand.tier_min >= 1 && cand.tier_min <= 5) s = &cand;
    });
    MOTOR_CHECK(s != nullptr);

    RPG::CharacterSheet caster;
    caster.baseStats = {10, 10, 10, 10};
    caster.tier = s->tier_min;

    RPG::CharacterSheet target;
    target.baseStats = {0, 0, 0, 0};
    target.tier = 1;

    ScriptedRng rng({6, 6});  // 2 exitos: SUCCESS o CRITICAL
    ExecutionContext ctx;
    ctx.caster = &caster;
    ctx.target = &target;
    ctx.skill = s;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;
    ctx.override_base_dice = 2;
    ctx.override_target_dice = 0;

    auto out = SkillExecutor::execute(ctx);

    // No crash; final_magnitude >= 0.
    MOTOR_CHECK(out.final_magnitude >= 0);

    // final_magnitude = magnitude_by_degree[final_degree], escalada per
    // l'estat de la carta quan n'hi ha (D-11 O3: scale_magnitude amb el
    // caster de baseStats {10,10,10,10} → ×1.1), pot x1.5 si CRITICAL.
    int expected = s->magnitude(out.final_degree);
    auto efd = RPG::parse_effect_scaling(s->effect_scaling);
    if (efd && expected > 0) {
        expected = RPG::scale_magnitude(*efd, &caster, expected);
    }
    if (out.was_all_sixes_critical) {
        expected = static_cast<int>(expected * 1.5f);
    }
    MOTOR_CHECK(out.final_magnitude == expected);
}

// Cartas elegidas por ID: una ofensiva y una narrativa de magnitud cero.
// El orden del catalogo no elige el significado del caso. Se fijan por
// separado exito ordinario y critico de dos 6 (D0013), con dados controlados.
MOTOR_TEST_CASE(d11_cartes_publicades_magnitud_zero_exit_i_critic) {
    const SkillCatalog cat = loadSkills();
    RPG::CharacterSheet caster;
    caster.baseStats = {0, 0, 0, 0};
    caster.tier = 1;
    for (const std::string id : {"abrazo_de_brasas", "atletismo"}) {
        const auto* skill = cat.find(id);
        MOTOR_CHECK(skill != nullptr);
        const bool ofensiva = id == "abrazo_de_brasas";
        MOTOR_CHECK(skill->magnitude(Degree::SUCCESS) == (ofensiva ? 4 : 0));
        MOTOR_CHECK(skill->magnitude(Degree::CRITICAL) == (ofensiva ? 8 : 0));
        for (const bool critico : {false, true}) {
            ScriptedRng rng({critico ? 6 : 5, critico ? 6 : 5});
            ExecutionContext ctx;
            ctx.caster = &caster;
            ctx.skill = skill;
            ctx.rng = &rng;
            ctx.override_base_dice = 2;
            ctx.override_target_dice = 0;
            const auto out = SkillExecutor::execute(ctx);
            MOTOR_CHECK(out.final_degree == (critico ? Degree::CRITICAL : Degree::SUCCESS));
            MOTOR_CHECK(out.was_all_sixes_critical == critico);
            MOTOR_CHECK(out.final_magnitude == (ofensiva ? (critico ? 12 : 4) : 0));
            MOTOR_CHECK(rng.consumed() == 2);
        }
    }
}

// Contracte O4: Si alguna skill publicada porta applyConditionId o
// grantedSummonId, SkillExecutor ho reporta pending si degree>=min.
MOTOR_TEST_CASE(d11_condition_o_summon_asset_reportat_com_pending) {
    const SkillCatalog cat = loadSkills();
    const SkillDefinition* with_ac = nullptr;
    const SkillDefinition* with_gs = nullptr;
    cat.forEach([&](const SkillDefinition& cand) {
        if (!with_ac && !cand.apply_condition_id.empty()) with_ac = &cand;
        if (!with_gs && !cand.granted_summon_id.empty()) with_gs = &cand;
    });

    // Asset publicat 2026-09-12: 1 applyConditionId, 0 grantedSummonId.
    // Per mantenir tests sense inventar dades, els dos casos venen
    // respectivament del published o (si falta) de SkillDefinition inline
    // que no toquen l'asset publicat.
    RPG::CharacterSheet caster;
    caster.baseStats = {5, 5, 5, 5};
    caster.tier = 1;
    RPG::CharacterSheet target;
    target.baseStats = {0, 0, 0, 0};
    target.tier = 1;

    if (with_ac) {
        ScriptedRng rng({6, 6});
        ExecutionContext ctx;
        ctx.caster = &caster;
        ctx.target = &target;
        ctx.skill = with_ac;
        ctx.rng = &rng;
        ctx.is_player_attacking = true;
        ctx.override_base_dice = 2;
        ctx.override_target_dice = 0;
        auto out = SkillExecutor::execute(ctx);
        // Si degree >= apply_condition_min_degree, pending ha de ser no buit.
        if (out.final_degree >= with_ac->apply_condition_min_degree) {
            MOTOR_CHECK(out.pending_condition_id == with_ac->apply_condition_id);
            MOTOR_CHECK(out.pending_condition_rounds >= 1);
        }
    }

    if (!with_gs) {
        // Versió inline sense tocar assets.
        SkillDefinition fallback;
        fallback.id = "d11_fallback_invoke_test";
        fallback.tier_min = 1;
        fallback.granted_summon_id = "summon_test_placeholder";
        fallback.effect_scaling = "none";
        fallback.magnitude_by_degree[static_cast<int>(Degree::BOTCH)] = 0.f;
        fallback.magnitude_by_degree[static_cast<int>(Degree::PARTIAL)] = 2.f;
        fallback.magnitude_by_degree[static_cast<int>(Degree::SUCCESS)] = 4.f;
        fallback.magnitude_by_degree[static_cast<int>(Degree::CRITICAL)] = 8.f;
        ScriptedRng rng({6});
        ExecutionContext ctx;
        ctx.caster = &caster;
        ctx.target = &target;
        ctx.skill = &fallback;
        ctx.rng = &rng;
        ctx.is_player_attacking = true;
        ctx.override_base_dice = 1;
        ctx.override_target_dice = 0;
        auto out = SkillExecutor::execute(ctx);
        if (out.final_degree >= Degree::SUCCESS) {
            MOTOR_CHECK(out.pending_summon_id == fallback.granted_summon_id);
        }
    }
}

// Contracte O5: scaling_stat_from_string i effect_family_from_string
// accepten majúscules/minúscules sobre els noms publicats (INT/int/Int).
MOTOR_TEST_CASE(d11_string_conversions_caseless_sobre_noms_publicats) {
    for (auto name : {"CON", "con", "Con"}) {
        auto s = RPG::scaling_stat_from_string(name);
        MOTOR_CHECK(s == ScalingStat::CON);
    }
    for (auto name : {"INT", "int", "Int"}) {
        auto s = RPG::scaling_stat_from_string(name);
        MOTOR_CHECK(s == ScalingStat::INT);
    }
    for (auto name : {"DES", "des", "Des"}) {
        auto s = RPG::scaling_stat_from_string(name);
        MOTOR_CHECK(s == ScalingStat::DES);
    }
    for (auto name : {"CAR", "car", "Car"}) {
        auto s = RPG::scaling_stat_from_string(name);
        MOTOR_CHECK(s == ScalingStat::CAR);
    }
    for (auto name : {"DAMAGE", "damage", "Damage"}) {
        auto f = RPG::effect_family_from_string(name);
        MOTOR_CHECK(f == EffectFamily::DAMAGE);
    }
    for (auto name : {"HEAL", "heal", "Heal"}) {
        auto f = RPG::effect_family_from_string(name);
        MOTOR_CHECK(f == EffectFamily::HEAL);
    }
}

}  // namespace

int main() { return motor_test::runAllTests(); }
