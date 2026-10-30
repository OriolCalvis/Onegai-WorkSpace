#include "RPG/SkillEffect.h"
#include "RPG/CharacterSheet.h"
#include "Render/ICombatant.h"
#include "RPG/RandomEngine.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace RPG {

static const struct { const char* name; EffectFamily family; } family_table[] = {
    {"damage", EffectFamily::DAMAGE},
    {"heal", EffectFamily::HEAL},
    {"condition", EffectFamily::CONDITION},
    {"summon", EffectFamily::SUMMON},
    {"buff", EffectFamily::BUFF},
    {"debuff", EffectFamily::DEBUFF},
    {"extra_dice", EffectFamily::EXTRA_DICE},
    {"reroll", EffectFamily::REROLL},
    {"steal", EffectFamily::STEAL},
    {"custom", EffectFamily::CUSTOM},
    {"none", EffectFamily::NONE},
};

static const struct { const char* name; ScalingStat stat; } scaling_table[] = {
    {"none", ScalingStat::NONE},
    {"con", ScalingStat::CON},
    {"int", ScalingStat::INT},
    {"des", ScalingStat::DES},
    {"car", ScalingStat::CAR},
    {"tier", ScalingStat::TIER},
};

EffectFamily effect_family_from_string(const std::string& s) {
    std::string lower;
    lower.reserve(s.size());
    for (char c : s) lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    for (const auto& entry : family_table) {
        if (entry.name == lower) return entry.family;
    }
    return EffectFamily::NONE;
}

std::string effect_family_to_string(EffectFamily f) {
    switch (f) {
        case EffectFamily::NONE: return "none";
        case EffectFamily::DAMAGE: return "damage";
        case EffectFamily::HEAL: return "heal";
        case EffectFamily::CONDITION: return "condition";
        case EffectFamily::SUMMON: return "summon";
        case EffectFamily::BUFF: return "buff";
        case EffectFamily::DEBUFF: return "debuff";
        case EffectFamily::EXTRA_DICE: return "extra_dice";
        case EffectFamily::REROLL: return "reroll";
        case EffectFamily::STEAL: return "steal";
        case EffectFamily::CUSTOM: return "custom";
    }
    return "unknown";
}

ScalingStat scaling_stat_from_string(const std::string& s) {
    std::string lower;
    lower.reserve(s.size());
    for (char c : s) lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    for (const auto& entry : scaling_table) {
        if (entry.name == lower) return entry.stat;
    }
    return ScalingStat::NONE;
}

std::string scaling_stat_to_string(ScalingStat s) {
    switch (s) {
        case ScalingStat::NONE: return "none";
        case ScalingStat::CON: return "CON";
        case ScalingStat::INT: return "INT";
        case ScalingStat::DES: return "DES";
        case ScalingStat::CAR: return "CAR";
        case ScalingStat::TIER: return "tier";
    }
    return "unknown";
}

std::optional<SkillEffectDef> parse_effect_scaling(const std::string& raw_scaling) {
    SkillEffectDef def;
    def.scaling_raw = raw_scaling;

    if (raw_scaling.empty() || raw_scaling == "none") {
        def.family = EffectFamily::NONE;
        def.scaling = ScalingStat::NONE;
        return def;
    }

    std::string trimmed = raw_scaling;

    // Compound: "family:stat" (colon-separated)
    std::size_t colon_pos = trimmed.find(':');
    if (colon_pos != std::string::npos) {
        std::string family_str = trimmed.substr(0, colon_pos);
        std::string stat_str = trimmed.substr(colon_pos + 1);
        def.family = effect_family_from_string(family_str);
        def.scaling = scaling_stat_from_string(stat_str);
        return def;
    }

    // Compound: "stat_stat" (underscore-separated, e.g. "con_tier", "con_con")
    // Also handles family names with underscores like "extra_dice".
    std::size_t under_pos = trimmed.find('_');
    if (under_pos != std::string::npos) {
        // First try the full string as a family name (e.g. "extra_dice")
        def.family = effect_family_from_string(trimmed);
        if (def.family != EffectFamily::NONE) {
            def.scaling = ScalingStat::NONE;
            return def;
        }

        std::string first = trimmed.substr(0, under_pos);
        std::string second = trimmed.substr(under_pos + 1);
        ScalingStat st1 = scaling_stat_from_string(first);
        ScalingStat st2 = scaling_stat_from_string(second);
        if (st1 != ScalingStat::NONE) {
            // First part is the scaling stat (e.g. "con" in "con_tier")
            def.family = EffectFamily::DAMAGE;
            def.scaling = st1;
        } else {
            // Try first part as family name
            def.family = effect_family_from_string(first);
            def.scaling = st2;
        }
        if (def.family == EffectFamily::NONE && def.scaling == ScalingStat::NONE) {
            return std::nullopt;
        }
        return def;
    }

    // Bare stat name (e.g. "INT", "CON") → DAMAGE with that stat scaling
    ScalingStat st = scaling_stat_from_string(trimmed);
    if (st != ScalingStat::NONE) {
        def.family = EffectFamily::DAMAGE;
        def.scaling = st;
        return def;
    }

    // Bare family name (e.g. "damage", "heal")
    def.family = effect_family_from_string(trimmed);
    if (def.family == EffectFamily::NONE) {
        return std::nullopt;
    }
    return def;
}

int scale_magnitude(const SkillEffectDef& effect, const CharacterSheet* caster, int base_magnitude) {
    if (effect.scaling == ScalingStat::NONE || caster == nullptr) {
        return base_magnitude;
    }

    int stat_value = 0;
    switch (effect.scaling) {
        case ScalingStat::CON: stat_value = caster->stat(Stat::CON); break;
        case ScalingStat::INT: stat_value = caster->stat(Stat::INT); break;
        case ScalingStat::DES: stat_value = caster->stat(Stat::DES); break;
        case ScalingStat::CAR: stat_value = caster->stat(Stat::CAR); break;
        case ScalingStat::TIER: stat_value = caster->tier; break;
        default: return base_magnitude;
    }

    int scaled = static_cast<int>(base_magnitude * (1.0f + static_cast<float>(stat_value) / 100.0f));
    return std::max(1, scaled);
}

bool execute_effect(const SkillEffectDef& effect, ICombatant& target,
                    RandomEngine& rng, int base_magnitude,
                    const CharacterSheet* caster) {
    (void)rng;
    CharacterSheet* target_sheet = target.character_sheet();
    
    switch (effect.family) {
        case EffectFamily::DAMAGE: {
            int magnitude = scale_magnitude(effect, caster, base_magnitude);
            target.takeDamage(magnitude);
            return true;
        }
        case EffectFamily::HEAL: {
            int magnitude = scale_magnitude(effect, caster, base_magnitude);
            target.heal(magnitude);
            return true;
        }
        case EffectFamily::CONDITION: {
            // Apply a condition to the target
            if (!target_sheet || effect.condition_id.empty()) {
                return false;
            }
            // Condition application would use ConditionEngine in a full implementation
            // For now, acknowledge the effect
            return true;
        }
        case EffectFamily::SUMMON: {
            // Summon a creature - requires summon_id from effect
            if (effect.summon_id.empty()) {
                return false;
            }
            return true;
        }
        case EffectFamily::BUFF: {
            // Apply a buff (positive effect) - could be a condition with positive effects
            // Placeholder implementation - acknowledged but not fully implemented
            return true;
        }
        case EffectFamily::DEBUFF: {
            // Apply a debuff (negative effect) - similar to CONDITION but negative
            // Placeholder implementation - acknowledged but not fully implemented
            return true;
        }
        case EffectFamily::EXTRA_DICE: {
            // Grant extra dice for next roll
            return true;
        }
        case EffectFamily::REROLL: {
            // Allow rerolling dice
            return true;
        }
        case EffectFamily::STEAL: {
            // Steal something from target
            return true;
        }
        case EffectFamily::CUSTOM: {
            // Custom effect - placeholder for modders/extensions
            return true;
        }
        case EffectFamily::NONE:
            return true;
        default:
            return false;
    }
}

} // namespace RPG
