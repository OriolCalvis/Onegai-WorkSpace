#include "RPG/CharacterSheet.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace RPG {

// ============================================================
// Helpers de RpgCoreDefinitions que no van en header (redundante ponerlos
// inline, mejor en este translation unit que linkea con el CharacterSheet).
// ============================================================
static Stat stat_from_string(const std::string& s_in) {
    std::string s;
    s.reserve(s_in.size());
    for (auto c : s_in) s.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    if (s == "CON" || s == "CONSTITUCION") return Stat::CON;
    if (s == "DES" || s == "DESTREZA")     return Stat::DES;
    if (s == "INT" || s == "INTELIGENCIA") return Stat::INT;
    if (s == "CAR" || s == "CARISMA")      return Stat::CAR;
    return Stat::INT;
}
Stat ClassDefinition::primary_stat_enum() const { return stat_from_string(primaryStat); }
Stat ClassDefinition::secondary_stat_enum() const { return stat_from_string(secondaryStat); }

// ============================================================
// Formula canonical (GDD Onegai Ed. 2)
//   CA física   = 10 + DES + peso_armadura_permitido + eqArmor + eqShield
//   Defensa mental = 10 + CAR + mentalSave
//   Resistencia física = 10 + CON + physicalSave
//   Prec. mágica = 10 + INT + spellSave
// ============================================================
DefenseBlock CharacterSheet::defenses(const ClassDefinition* /*cls*/) const {
    DefenseBlock d;
    int ca  = 10 + des() + equipArmorBonus + equipShieldBonus;
    int dfs = 10 + int_() + spellSaveExtra;
    int defM  = 10 + car() + mentalSaveBonus;
    int resF = 10 + con() + physicalSaveBonus;
    d.values[static_cast<int>(Defense::ARMOR_CLASS)] = ca;
    d.values[static_cast<int>(Defense::PHYSICAL_SAVE)] = resF;
    d.values[static_cast<int>(Defense::WILL_SAVE)] = defM;
    d.values[static_cast<int>(Defense::SPELL_SAVE_DC)] = dfs;
    return d;
}

// ============================================================
// Vida tope = min( baseHealth + CON*3 + CAR*1 + healthBonus_tier + eqBonus, healthCap_tier )
// ============================================================
int CharacterSheet::healthCap(const TierRules& rules, const ClassDefinition* cls) const {
    int baseHealth = (cls) ? cls->baseHealth : 8;
    int eqBonus = equipArmorBonus / 2; // aproximacion; equipos con vida explícita lo suman en P1
    return rules.compute_cap_health(tier, baseHealth, con(), car(), eqBonus);
}

// ============================================================
// Tier up: cada tier N→N+1 requiere (N+1) milestones. Por ejemplo T1→T2 2 hitos.
// Se le pasa el total de milestones obtenidos; devuelve true si al menos 1 salto.
// ============================================================
bool CharacterSheet::tierUp(const TierRules& rules, int milestoneCount) {
    bool advanced = false;
    while (tier < TIER_MAX) {
        // Milestones necesarios para pasar a (tier+1):
        int needed = 1 + tier + 1;   // T1→T2 = 3? GDD: T0→T1 =1, T1→T2=2, T2→T3=3, T3→T4=4, T4→T5=5
        needed = tier + 1;
        if (milestoneCount < needed) break;
        // La regla del tier permite multiclass?
        const TierRule* tr = rules.find(tier + 1);
        if (!tr) break;
        tier += 1;
        milestoneCount -= needed;
        advanced = true;
    }
    return advanced;
}

// ============================================================
// Manejo de condiciones
// ============================================================
bool CharacterSheet::hasCondition(const std::string& conditionId) const {
    return conditionStacks(conditionId) > 0;
}
int  CharacterSheet::conditionStacks(const std::string& conditionId) const {
    for (const auto& c : conditions) {
        if (c.conditionId == conditionId && c.roundsRemaining > 0 && c.stacks > 0)
            return c.stacks;
    }
    return 0;
}

// ============================================================
// Facciones
// ============================================================
int CharacterSheet::getRep(const std::string& factionId) const {
    auto it = factionRep.find(factionId);
    return (it == factionRep.end()) ? 0 : it->second;
}
void CharacterSheet::adjustRep(const std::string& factionId, int delta) {
    auto it = factionRep.find(factionId);
    if (it == factionRep.end()) {
        factionRep[factionId] = delta;
    } else {
        it->second += delta;
    }
    int& v = factionRep[factionId];
    if (v < -100) v = -100;
    if (v > 100)  v = 100;
}
// Rep normalizado: -100 (aliado enemigo, precios ×2.0) .. +100 (aliado, 0.5x)
int CharacterSheet::repPriceModPct(const std::string& factionId) const {
    int rep = getRep(factionId);
    // lineal de -100→+100% a +100→-50% (P0 simplificado)
    // Rep -100: 100%, 0:0%, +100:-50%
    return static_cast<int>(-0.75f * rep);
}

}  // namespace RPG
