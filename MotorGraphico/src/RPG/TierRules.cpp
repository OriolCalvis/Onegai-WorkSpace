#include "RPG/TierRules.h"

#include "Core/Json/JsonValue.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <sstream>
#include <utility>
#include <cstdio>

namespace {

constexpr std::array<const char*, 6> kRarities = {
    "common", "uncommon", "rare", "epic", "legendary", "mythic"
};

// TierRules por defecto (coincide con assets/rules/tier_rules.json).
// NO constexpr: TierRule tiene std::string (no es LiteralType).
// Orden de TierRule:  tier, name, description, handLimit, maxEquipRarity,
//                     healthCap, healthBonus, allowMulticlass, multiclassHandLimit,
//                     maxSummons, maxFeats, maxDivineCards.
static const std::array<RPG::TierRule, 6> kDefaultTiers = {{
    // 0: Civil
    { 0, "Civil / Aprendiz",          "",  6, "common",     27,  0, false,  0, 1, 0, 0 },
    // 1: Aventurero Inicial
    { 1, "Aventurero Inicial",        "", 10, "uncommon",   35,  0, false,  0, 2, 1, 2 },
    // 2: Competente
    { 2, "Aventurero Competente",     "", 15, "rare",       50,  4, true,  12, 3, 2, 3 },
    // 3: Heroe
    { 3, "Heroe Consolidado",         "", 20, "epic",       70,  9, true,  16, 4, 3, 5 },
    // 4: Campeon
    { 4, "Campeon / Maestro",         "", 24, "epic",       95, 16, true,  20, 5, 4, 6 },
    // 5: Mitico
    { 5, "Mitico",                    "", 28, "legendary", 125, 25, true,  24, 6, 5, 8 },
}};

}  // namespace (anon)

namespace RPG {

// =========================================================================
// Rarity helpers
// =========================================================================
int rarityRank(const std::string& rarity) {
    for (std::size_t i = 0; i < kRarities.size(); ++i) {
        if (rarity == kRarities[i]) return static_cast<int>(i);
    }
    return -1;
}

ItemRarity rarity_from_name(const std::string& rarity) {
    const int r = rarityRank(rarity);
    if (r < 0) return ItemRarity::Common;
    if (r >= static_cast<int>(ItemRarity::Mythic)) return ItemRarity::Mythic;
    return static_cast<ItemRarity>(r);
}

std::string rarity_name(ItemRarity r) {
    const auto idx = static_cast<std::size_t>(r);
    if (idx >= kRarities.size()) return "common";
    return kRarities[idx];
}

// =========================================================================
// TierRules
// =========================================================================
TierRules::TierRules() {
    m_tiers.assign(kDefaultTiers.begin(), kDefaultTiers.end());
}

const TierRule* TierRules::find(int tier) const {
    for (const auto& r : m_tiers) if (r.tier == tier) return &r;
    return nullptr;
}

const TierRule& TierRules::at(int tier) const {
    static const TierRule kFallback{0, "", "", 6, "common", 27, 0, false, 0, 1, 0, 0};
    const TierRule* r = find(tier);
    return r ? *r : kFallback;
}

int TierRules::handLimit(int tier, bool isMulticlass) const {
    const TierRule& r = at(tier);
    return (isMulticlass && r.allowMulticlass && r.multiclassHandLimit > 0)
               ? r.multiclassHandLimit
               : r.handLimit;
}

// Formula canonical: vida = min(base + CON*3 + CAR*1 + healthBonus + eqBonus, healthCap)
int TierRules::compute_cap_health(int tier, int baseHealth, int CON, int CAR, int eqBonus) const {
    const TierRule& r = at(tier);
    int total = baseHealth + CON * 3 + CAR * 1 + r.healthBonus + eqBonus;
    if (total < 1) total = 1;
    if (r.healthCap > 0 && total > r.healthCap) total = r.healthCap;
    return total;
}

// ============================================================
// Carga JSON
// ============================================================
static ItemRarity parse_rarity_field(const JsonValue& jv, ItemRarity def) {
    if (jv.isNumber()) {
        int i = jv.asInt(0);
        if (i < 0) i = 0;
        if (i > 5) i = 5;
        return static_cast<ItemRarity>(i);
    }
    return rarity_from_name(jv.asString(rarity_name(def)));
}

static TierRule parse_one_tier(const JsonValue& j) {
    TierRule t;
    t.tier = j["tier"].asInt(0);
    t.name = j["name"].asString("Tier " + std::to_string(t.tier));
    t.description = j["description"].asString("");
    t.maxEquipRarity = j["maxEquipRarity"].asString(j["max_equip_rarity"].asString("common"));
    t.handLimit = j["handLimit"].asInt(j["hand_limit"].asInt(6));
    t.healthCap = j["healthCap"].asInt(j["health_cap"].asInt(27));
    t.healthBonus = j["healthBonus"].asInt(j["health_bonus"].asInt(0));
    t.allowMulticlass = j["allowMulticlass"].asBool(j["allow_multiclass"].asBool(false));
    t.multiclassHandLimit = j["multiclassHandLimit"].asInt(j["multiclass_hand_limit"].asInt(0));
    t.maxSummons = j["maxSummons"].asInt(j["max_summons"].asInt(0));
    t.maxFeats = j["maxFeats"].asInt(j["max_feats"].asInt(1));
    t.maxDivineCards = j["maxDivineCards"].asInt(j["max_divine_cards"].asInt(0));
    return t;
}

Result<int> TierRules::loadFromString(const std::string& jsonText) {
    auto parsed = JsonValue::parse(jsonText);
    if (!parsed.isOk()) return Result<int>::Error(parsed.errorMessage());
    const JsonValue& root = parsed.value();
    if (!root.isObject()) return Result<int>::Error("TierRules root no es objeto");
    const JsonValue& tiers = root["tiers"];
    if (!tiers.isArray()) return Result<int>::Error("TierRules no tiene clave 'tiers' (array)");
    m_tiers.clear();
    for (std::size_t i = 0; i < tiers.size(); ++i) {
        const JsonValue& tj = tiers[i];
        TierRule t = parse_one_tier(tj);
        auto it = std::find_if(m_tiers.begin(), m_tiers.end(), [t](const TierRule& r){ return r.tier == t.tier; });
        if (it == m_tiers.end()) m_tiers.push_back(std::move(t));
        else                     *it = std::move(t);
    }
    std::sort(m_tiers.begin(), m_tiers.end(), [](const TierRule& a, const TierRule& b){ return a.tier < b.tier; });
    return Result<int>::Ok(static_cast<int>(m_tiers.size()));
}

Result<int> TierRules::loadFromFile(const std::string& path) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return Result<int>::Error("TierRules: no se puede abrir fichero: " + path);
    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    std::string buf;
    if (sz > 0) {
        buf.resize(static_cast<std::size_t>(sz));
        (void)std::fread(buf.data(), 1, static_cast<std::size_t>(sz), f);
    }
    std::fclose(f);
    return loadFromString(buf);
}

} // namespace RPG
