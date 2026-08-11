#pragma once

#include <cstddef>
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <array>
#include <algorithm>
#include <cctype>

#include "Core/Errors/Result.h"
#include "Core/Json/JsonValue.h"
#include "Core/Resources/ICatalog.h"

#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/TierRules.h"

namespace RPG {
namespace Catalogs {

// ============================================================
// Helpers locales (no salen de este TU).
// ============================================================
static inline std::string tolower_inline(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

template <std::size_t N>
static inline void copy_json_int_array(const JsonValue& jv, std::array<int, N>& arr) {
    if (!jv.isArray()) return;
    const std::size_t sz = std::min<std::size_t>(jv.size(), N);
    for (std::size_t i = 0; i < sz; ++i) arr[i] = jv[i].asInt(0);
}

static inline std::vector<std::string> copy_json_string_vec(const JsonValue& jv) {
    std::vector<std::string> v;
    if (!jv.isArray()) return v;
    v.reserve(jv.size());
    for (std::size_t i = 0; i < jv.size(); ++i) v.push_back(jv[i].asString(""));
    return v;
}

static inline ItemRarity rarity_from_json(const JsonValue& jv, ItemRarity def = ItemRarity::Common) {
    if (jv.isNumber()) {
        int r = jv.asInt(0);
        if (r < 0) r = 0;
        if (r > 5) r = 5;
        return static_cast<ItemRarity>(r);
    }
    std::string s = tolower_inline(jv.asString(""));
    if (s == "common"     || s == "comun"      || s == "0") return ItemRarity::Common;
    if (s == "uncommon"   || s == "poco_comun" || s == "rara" || s == "1") return ItemRarity::Uncommon;
    if (s == "rare"       || s == "raro"       || s == "2") return ItemRarity::Rare;
    if (s == "epic"       || s == "epico"      || s == "3") return ItemRarity::Epic;
    if (s == "legendary"  || s == "legendario" || s == "4") return ItemRarity::Legendary;
    if (s == "mythic"     || s == "mitico"     || s == "5") return ItemRarity::Mythic;
    return def;
}

static inline Degree degree_from_string(const std::string& s, Degree def = Degree::SUCCESS) {
    std::string low = tolower_inline(s);
    if (low == "botch"    || low == "critico_fallo" || low == "0") return Degree::BOTCH;
    if (low == "partial"  || low == "parcial"       || low == "1") return Degree::PARTIAL;
    if (low == "success"  || low == "exito"         || low == "2") return Degree::SUCCESS;
    if (low == "critical" || low == "critico"       || low == "3") return Degree::CRITICAL;
    return def;
}

// ============================================================
// CatalogLoader<T> — especializamos UNA VEZ por cada T definido.
// Implementa `static T from_json(const JsonValue& entry)`.
//
// La base default da linker error (imposible especializar sin código) para
// obligarnos a cubrir todos los tipos; no hay fallback silencioso.
// ============================================================
template <typename T>
struct CatalogLoader {
    static T from_json(const JsonValue& entry);
};

// -----------------------------------------------------------------
// Especializacion: SkillDefinition (tambien sirve para spell, que es
// un subconjunto: id, name, description, actionType, recovery, target,
// magnitudeByDegree... todo es comun)
// -----------------------------------------------------------------
template<>
inline SkillDefinition CatalogLoader<SkillDefinition>::from_json(const JsonValue& e) {
    SkillDefinition s;
    s.id = e["id"].asString("");
    s.name = e["name"].asString(s.id);
    s.description = e["description"].asString("");
    s.flavor_text = e["flavorText"].asString("");
    if (s.flavor_text.empty()) s.flavor_text = e["flavor_text"].asString("");

    s.tier_min = e["tier"].asInt(e["tierMin"].asInt(e["tier_min"].asInt(0)));
    s.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);

    s.action_type = action_type_from_string(e["actionType"].asString(e["action_type"].asString("")));
    s.channel_rounds = e["channelRounds"].asInt(e["channel_rounds"].asInt(0));
    s.recovery = recovery_from_string(e["recovery"].asString(e["recoveryPile"].asString(e["recovery_pile"].asString(""))));
    s.target = skill_target_from_string(e["target"].asString(""));

    std::string casting = tolower_inline(e["castingStat"].asString(e["casting_stat"].asString("int")));
    if (casting == "con") s.casting_stat = Stat::CON;
    else if (casting == "des") s.casting_stat = Stat::DES;
    else if (casting == "car") s.casting_stat = Stat::CAR;
    else                       s.casting_stat = Stat::INT;

    s.save_attribute = save_attribute_from_string(e["saveAttribute"].asString(e["save_attribute"].asString("")));
    s.override_cd_if_save = static_cast<float>(e["overrideCdIfSave"].asNumber(e["override_cd_if_save"].asNumber(0.0)));

    // magnitudeByDegree: array de 4 floats, o si falta: heurística de magnitude por degree si
    // venía como damage/heal/magnitudeBase (lo normalizamos en Python pero por seguridad)
    const JsonValue& mb = e["magnitudeByDegree"];
    if (mb.isArray() && mb.size() >= 4) {
        for (int i = 0; i < 4; ++i) s.magnitude_by_degree[i] = static_cast<float>(mb[i].asNumber(0.0));
    } else {
        float base = static_cast<float>(e["magnitudeBase"].asNumber(e["damage"].asNumber(e["heal"].asNumber(4))));
        s.magnitude_by_degree[static_cast<int>(Degree::BOTCH)] = 0.f;
        s.magnitude_by_degree[static_cast<int>(Degree::PARTIAL)] = base * 0.5f;
        s.magnitude_by_degree[static_cast<int>(Degree::SUCCESS)] = base * 1.0f;
        s.magnitude_by_degree[static_cast<int>(Degree::CRITICAL)] = base * 2.0f;
    }
    std::string mtype = tolower_inline(e["magnitudeType"].asString(e["magnitude_type"].asString("")));
    if (mtype == "heal")         s.magnitude_type = MagnitudeType::HEAL;
    else if (mtype == "move")    s.magnitude_type = MagnitudeType::MOVE_DISTANCE;
    else if (mtype == "condition") s.magnitude_type = MagnitudeType::CONDITION_DURATION;
    else if (mtype == "damage")  s.magnitude_type = MagnitudeType::DAMAGE;
    else                         s.magnitude_type = MagnitudeType::OTHER;

    s.apply_condition_id = e["applyConditionId"].asString(e["apply_condition_id"].asString(""));
    s.apply_condition_min_degree = degree_from_string(e["applyConditionMinDegree"].asString(e["apply_condition_min_degree"].asString("partial")), Degree::PARTIAL);
    s.condition_duration_rounds = e["conditionDurationRounds"].asInt(e["condition_duration_rounds"].asInt(3));

    s.tags               = copy_json_string_vec(e["tags"]);
    s.required_tags      = copy_json_string_vec(e["requiredTags"]);
    s.incompatible_tags  = copy_json_string_vec(e["incompatibleTags"]);
    s.granted_summon_id  = e["grantedSummonId"].asString(e["granted_summon_id"].asString(""));
    return s;
}

// Helper: stat array 4-elem
static inline std::array<int, 4> statarr_json(const JsonValue& e, const std::string& k1, const std::string& k2="") {
    std::array<int, 4> arr = {0,0,0,0};
    const JsonValue& j = e.has(k1) ? e[k1] : (k2.empty() ? e["__null__"] : e[k2]);
    if (j.isArray()) {
        copy_json_int_array<4>(j, arr);
    } else if (j.isObject()) {
        arr[0] = j["con"].asInt(j["CON"].asInt(0));
        arr[1] = j["des"].asInt(j["DES"].asInt(0));
        arr[2] = j["int"].asInt(j["INT"].asInt(0));
        arr[3] = j["car"].asInt(j["CAR"].asInt(0));
    }
    return arr;
}

// -----------------------------------------------------------------
// ClassDefinition
// -----------------------------------------------------------------
template<>
inline ClassDefinition CatalogLoader<ClassDefinition>::from_json(const JsonValue& e) {
    ClassDefinition c;
    c.id = e["id"].asString("");
    c.name = e["name"].asString(c.id);
    c.description = e["description"].asString("");
    c.lore = e["lore"].asString(e["flavorText"].asString(e["flavor_text"].asString("")));
    c.tier = e["tier"].asInt(1);
    c.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    c.role = e["role"].asString("");
    c.baseHealth = e["baseHealth"].asInt(e["base_health"].asInt(8));
    c.primaryStat   = e["primaryStat"].asString(e["primary_stat"].asString("INT"));
    c.secondaryStat = e["secondaryStat"].asString(e["secondary_stat"].asString("DES"));
    c.maxSkillCards  = e["maxSkillCards"].asInt(e["max_skill_cards"].asInt(10));
    c.compatibilityWeapons = copy_json_string_vec(e["compatibilityWeapons"]);
    c.compatibilityArmor   = copy_json_string_vec(e["compatibilityArmor"]);
    c.allowedEquipmentTags = copy_json_string_vec(e["allowedEquipmentTags"]);
    c.maxArmorWeight  = e["maxArmorWeight"].asString(e["max_armor_weight"].asString(""));
    c.maxWeaponWeight = e["maxWeaponWeight"].asString(e["max_weapon_weight"].asString(""));
    c.passiveId       = e["passiveId"].asString(e["passive_id"].asString(""));
    c.startingSkillIds     = copy_json_string_vec(e["startingCards"]);
    if (c.startingSkillIds.empty()) c.startingSkillIds = copy_json_string_vec(e["startingSkillIds"]);
    c.startingEquipmentIds = copy_json_string_vec(e["startingEquipmentIds"]);
    c.specializationIds    = copy_json_string_vec(e["specializationIds"]);
    c.canMulticlassInto = e["canMulticlassInto"].asBool(e["can_multiclass_into"].asBool(false));
    c.restrictedTags = copy_json_string_vec(e["restrictedTags"]);
    return c;
}

// -----------------------------------------------------------------
// RaceDefinition
// -----------------------------------------------------------------
template<>
inline RaceDefinition CatalogLoader<RaceDefinition>::from_json(const JsonValue& e) {
    RaceDefinition r;
    r.id = e["id"].asString("");
    r.name = e["name"].asString(r.id);
    r.description = e["description"].asString("");
    r.flavorText = e["flavorText"].asString(e["flavor_text"].asString(""));
    r.tier = e["tier"].asInt(1);
    r.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    r.baseSpeed = e["baseSpeed"].asInt(e["movementSpeed"].asInt(e["movement_speed"].asInt(30)));
    r.languages = copy_json_string_vec(e["languages"]);
    r.compatibleDeityIds = copy_json_string_vec(e["compatibleDeityIds"]);
    r.narrativeTags = copy_json_string_vec(e["narrativeTags"]);
    r.statBonuses = statarr_json(e, "statBonuses", "stat_bonuses");
    r.passiveTraitId = e["passiveTraitId"].asString(e["passive_trait_id"].asString(""));
    r.activeTraitId  = e["activeTraitId"].asString(e["active_trait_id"].asString(""));
    r.racialTraitId  = e["racialTraitId"].asString(e["racial_trait_id"].asString(""));
    r.affinities  = copy_json_string_vec(e["affinities"]);
    r.limitations = copy_json_string_vec(e["limitations"]);
    r.unlocks     = copy_json_string_vec(e["unlocks"]);
    return r;
}

// -----------------------------------------------------------------
// BackgroundDefinition
// -----------------------------------------------------------------
template<>
inline BackgroundDefinition CatalogLoader<BackgroundDefinition>::from_json(const JsonValue& e) {
    BackgroundDefinition b;
    b.id = e["id"].asString("");
    b.name = e["name"].asString(b.id);
    b.description = e["description"].asString("");
    b.tier = e["tier"].asInt(0);
    b.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    b.startingWealthGold = e["startingWealthGold"].asInt(e["starting_wealth_gold"].asInt(0));
    b.virtue = e["virtue"].asString("");
    b.defect = e["defect"].asString("");
    b.goal   = e["goal"].asString("");
    b.startingEquipmentIds = copy_json_string_vec(e["startingEquipmentIds"]);
    b.skillProficiencyIds  = copy_json_string_vec(e["skillProficiencyIds"]);
    b.featureIds = copy_json_string_vec(e["featureIds"]);
    b.factionId = e["factionId"].asString(e["faction_id"].asString(""));
    b.startingFactionRep = e["startingFactionRep"].asInt(e["starting_faction_rep"].asInt(0));
    return b;
}

// -----------------------------------------------------------------
// PassiveDefinition
// -----------------------------------------------------------------
template<>
inline PassiveDefinition CatalogLoader<PassiveDefinition>::from_json(const JsonValue& e) {
    PassiveDefinition p;
    p.id = e["id"].asString("");
    p.name = e["name"].asString(p.id);
    p.description = e["description"].asString("");
    p.flavorText = e["flavorText"].asString("");
    p.tier = e["tier"].asInt(0);
    p.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    std::string trg = tolower_inline(e["trigger"].asString("always_active"));
    if (trg == "on_play_longrest" || trg == "longrest")              p.trigger = PassiveDefinition::Trigger::ON_PLAY_LONGREST;
    else if (trg == "on_not_moved")                                 p.trigger = PassiveDefinition::Trigger::ON_NOT_MOVED;
    else if (trg == "on_ally_healed")                               p.trigger = PassiveDefinition::Trigger::ON_ALLY_HEALED;
    else if (trg == "on_turn_start" || trg == "turn_start")         p.trigger = PassiveDefinition::Trigger::ON_TURN_START;
    else if (trg == "on_crit_taken")                                p.trigger = PassiveDefinition::Trigger::ON_CRIT_TAKEN;
    else if (trg == "on_attack_rolled")                             p.trigger = PassiveDefinition::Trigger::ON_ATTACK_ROLLED;
    else if (trg == "on_damage_taken")                              p.trigger = PassiveDefinition::Trigger::ON_DAMAGE_TAKEN;
    else if (trg == "on_crit_dealt")                                p.trigger = PassiveDefinition::Trigger::ON_CRIT_DEALT;
    else                                                            p.trigger = PassiveDefinition::Trigger::ALWAYS_ACTIVE;
    p.extra_dice_on_trigger   = e["extraDiceOnTrigger"].asInt(e["extra_dice_on_trigger"].asInt(0));
    p.success_bonus_on_trigger = static_cast<float>(e["successBonusOnTrigger"].asNumber(e["success_bonus_on_trigger"].asNumber(0.0)));
    p.required_tag = e["requiredTag"].asString(e["required_tag"].asString(""));
    p.statBonuses      = statarr_json(e, "statBonuses", "stat_bonuses");
    p.armorBonus       = e["armorBonus"].asInt(e["armor_bonus"].asInt(0));
    p.physicalSaveBonus= e["physicalSaveBonus"].asInt(e["physical_save_bonus"].asInt(0));
    p.mentalSaveBonus  = e["mentalSaveBonus"].asInt(e["mental_save_bonus"].asInt(0));
    p.spellSaveBonus   = e["spellSaveBonus"].asInt(e["spell_save_bonus"].asInt(0));
    p.maxHealthBonus   = e["maxHealthBonus"].asInt(e["max_health_bonus"].asInt(0));
    p.handSizeBonus    = e["handSizeBonus"].asInt(e["hand_size_bonus"].asInt(0));
    return p;
}

// -----------------------------------------------------------------
// FeatDefinition
// -----------------------------------------------------------------
template<>
inline FeatDefinition CatalogLoader<FeatDefinition>::from_json(const JsonValue& e) {
    FeatDefinition f;
    f.id = e["id"].asString("");
    f.name = e["name"].asString(f.id);
    f.description = e["description"].asString("");
    f.tier_min = e["tierMin"].asInt(e["tier_min"].asInt(1));
    f.rarity = rarity_from_json(e["rarity"], ItemRarity::Rare);
    f.requiredClassId = e["requiredClassId"].asString(e["required_class_id"].asString(""));
    f.requiredBackgroundId = e["requiredBackgroundId"].asString(e["required_background_id"].asString(""));
    f.requiredTier = e["requiredTier"].asInt(e["required_tier"].asInt(1));
    f.statBonuses = statarr_json(e, "statBonuses", "stat_bonuses");
    f.grantedPassiveIds = copy_json_string_vec(e["grantedPassiveIds"]);
    f.grantedSkillIds   = copy_json_string_vec(e["grantedSkillIds"]);
    return f;
}

// -----------------------------------------------------------------
// TraitDefinition
// -----------------------------------------------------------------
template<>
inline TraitDefinition CatalogLoader<TraitDefinition>::from_json(const JsonValue& e) {
    TraitDefinition t;
    t.id = e["id"].asString("");
    t.name = e["name"].asString(t.id);
    t.description = e["description"].asString("");
    t.tier_min = e["tierMin"].asInt(e["tier_min"].asInt(0));
    t.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    t.isActive = e["isActive"].asBool(e["is_active"].asBool(false));
    t.timesPerRest = e["timesPerRest"].asInt(e["times_per_rest"].asInt(1));
    t.recovery = e["recovery"].asString("descanso_largo");
    t.linkedSkillId = e["linkedSkillId"].asString(e["linked_skill_id"].asString(""));
    return t;
}

// -----------------------------------------------------------------
// DeityDefinition
// -----------------------------------------------------------------
template<>
inline DeityDefinition CatalogLoader<DeityDefinition>::from_json(const JsonValue& e) {
    DeityDefinition d;
    d.id = e["id"].asString("");
    d.name = e["name"].asString(d.id);
    d.title = e["title"].asString("");
    d.alignment = e["alignment"].asString("");
    d.description = e["description"].asString("");
    d.domains = copy_json_string_vec(e["domains"]);
    d.symbolTags = copy_json_string_vec(e["symbolTags"]);
    d.tier_min = e["tierMin"].asInt(e["tier_min"].asInt(0));
    d.prayerSkillIds    = copy_json_string_vec(e["prayerSkillIds"]);
    d.grantedPassiveIds = copy_json_string_vec(e["grantedPassiveIds"]);
    d.dogmaStatBonus = statarr_json(e, "dogmaStatBonus", "dogma_stat_bonus");
    return d;
}

// -----------------------------------------------------------------
// ConditionDefinition
// -----------------------------------------------------------------
template<>
inline ConditionDefinition CatalogLoader<ConditionDefinition>::from_json(const JsonValue& e) {
    ConditionDefinition c;
    c.id = e["id"].asString("");
    c.name = e["name"].asString(c.id);
    c.description = e["description"].asString("");
    std::string stk = tolower_inline(e["stacking"].asString("duration"));
    if (stk == "none") c.stacking = ConditionDefinition::Stacking::NONE;
    else if (stk == "stacks" || stk == "stack" || stk == "stack_intensity") c.stacking = ConditionDefinition::Stacking::STACK_INTENSITY;
    else c.stacking = ConditionDefinition::Stacking::DURATION;
    c.defaultRounds = e["defaultRounds"].asInt(e["default_rounds"].asInt(3));
    c.maxStacks = e["maxStacks"].asInt(e["max_stacks"].asInt(1));
    c.damagePerRound = e["damagePerRound"].asInt(e["damage_per_round"].asInt(0));
    c.isNegative = e["isNegative"].asBool(e["is_negative"].asBool(true));
    c.statMod = statarr_json(e, "statMod", "stat_mod");
    c.caMod = e["caMod"].asInt(e["ca_mod"].asInt(0));
    c.physicalSaveMod = e["physicalSaveMod"].asInt(e["physical_save_mod"].asInt(0));
    c.willSaveMod     = e["willSaveMod"].asInt(e["will_save_mod"].asInt(0));
    c.preventAction = e["preventAction"].asBool(e["prevent_action"].asBool(false));
    c.halfSpeed = e["halfSpeed"].asBool(e["half_speed"].asBool(false));
    c.grantAdvantageToEnemies = e["grantAdvantageToEnemies"].asBool(e["grant_advantage_to_enemies"].asBool(false));
    c.grantDisadvantageToCaster = e["grantDisadvantageToCaster"].asBool(e["grant_disadvantage_to_caster"].asBool(false));
    c.breaksOnDamage = e["breaksOnDamage"].asBool(e["breaks_on_damage"].asBool(false));
    return c;
}

// -----------------------------------------------------------------
// EquipmentDefinition
// -----------------------------------------------------------------
template<>
inline EquipmentDefinition CatalogLoader<EquipmentDefinition>::from_json(const JsonValue& e) {
    EquipmentDefinition eq;
    eq.id = e["id"].asString("");
    eq.name = e["name"].asString(eq.id);
    eq.description = e["description"].asString("");
    eq.flavorText = e["flavorText"].asString(e["flavor_text"].asString(""));
    eq.tier = e["tier"].asInt(0);
    eq.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    eq.price = e["price"].asInt(0);
    eq.slot = e["slot"].asString("");
    eq.weightCategory = e["weightCategory"].asString(e["weight_category"].asString(e["weight"].asString("")));
    eq.compatibleSymbols = copy_json_string_vec(e["compatibleSymbols"]);
    eq.grantedTags = copy_json_string_vec(e["grantedTags"]);
    eq.statBonuses = statarr_json(e, "statBonuses", "stat_bonuses");
    eq.caBonus = e["caBonus"].asInt(e["ca_bonus"].asInt(0));
    eq.maxHealthBonus = e["maxHealthBonus"].asInt(e["max_health_bonus"].asInt(0));
    eq.physicalSaveBonus = e["physicalSaveBonus"].asInt(e["physical_save_bonus"].asInt(0));
    eq.willSaveBonus     = e["willSaveBonus"].asInt(e["will_save_bonus"].asInt(0));
    eq.spellSaveBonus    = e["spellSaveBonus"].asInt(e["spell_save_bonus"].asInt(0));
    eq.damageBonus = e["damageBonus"].asInt(e["damage_bonus"].asInt(0));
    eq.linkedSkillId = e["linkedSkillId"].asString(e["linked_skill_id"].asString(""));
    eq.twoHanded = e["twoHanded"].asBool(e["two_handed"].asBool(false));
    return eq;
}

// -----------------------------------------------------------------
// ConsumableDefinition
// -----------------------------------------------------------------
template<>
inline ConsumableDefinition CatalogLoader<ConsumableDefinition>::from_json(const JsonValue& e) {
    ConsumableDefinition c;
    c.id = e["id"].asString("");
    c.name = e["name"].asString(c.id);
    c.description = e["description"].asString("");
    c.tier = e["tier"].asInt(0);
    c.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    c.price = e["price"].asInt(0);
    c.healHp = e["healHp"].asInt(e["heal_hp"].asInt(0));
    c.restorePileCards = e["restorePileCards"].asInt(e["restore_pile_cards"].asInt(0));
    c.clearOneCondition = e["clearOneCondition"].asBool(e["clear_one_condition"].asBool(false));
    c.grantConditionId = e["grantConditionId"].asString(e["grant_condition_id"].asString(""));
    c.grantConditionRounds = e["grantConditionRounds"].asInt(e["grant_condition_rounds"].asInt(3));
    c.bonusGold = e["bonusGold"].asInt(e["bonus_gold"].asInt(0));
    c.grantPassiveId = e["grantPassiveId"].asString(e["grant_passive_id"].asString(""));
    c.grantPassiveRounds = e["grantPassiveRounds"].asInt(e["grant_passive_rounds"].asInt(0));
    return c;
}

// -----------------------------------------------------------------
// MonsterDefinition
// -----------------------------------------------------------------
template<>
inline MonsterDefinition CatalogLoader<MonsterDefinition>::from_json(const JsonValue& e) {
    MonsterDefinition m;
    m.id = e["id"].asString("");
    m.name = e["name"].asString(m.id);
    m.flavorText = e["flavorText"].asString(e["flavor_text"].asString(""));
    m.description = e["description"].asString("");
    m.rank = e["rank"].asString("criatura");
    m.tier = e["tier"].asInt(0);
    m.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    m.role = e["role"].asString("striker");
    m.faction = e["faction"].asString(e["factionId"].asString(""));
    m.factionName = e["factionName"].asString("");
    m.stats = statarr_json(e, "stats", "statBonuses");
    m.maxHealth = e["maxHealth"].asInt(e["max_health"].asInt(e["health"].asInt(0)));
    m.ca = e["ca"].asInt(e["ac"].asInt(10));
    m.defMental  = e["defMental"].asInt(e["def_mental"].asInt(10));
    m.resFis     = e["resFis"].asInt(e["res_fis"].asInt(10));
    m.precMag    = e["precMag"].asInt(e["prec_mag"].asInt(10));
    m.movement   = e["movement"].asInt(e["movementSpeed"].asInt(5));
    const JsonValue& atks = e["attacks"];
    if (atks.isArray()) {
        m.attacks.reserve(atks.size());
        for (std::size_t i = 0; i < atks.size(); ++i) {
            const JsonValue& a = atks[i];
            MonsterDefinition::MonsterAttack ma;
            ma.name = a["name"].asString("");
            ma.defense = a["defense"].asString("CA");
            ma.effect  = a["effect"].asString("");
            ma.magnitudeHint = a["magnitude"].asInt(a["damage"].asInt(0));
            m.attacks.push_back(std::move(ma));
        }
    }
    m.passiveId = e["passiveId"].asString(e["passive_id"].asString(""));
    m.conditionsInflictedIds = copy_json_string_vec(e["conditionsInflicted"]);
    m.skillIds = copy_json_string_vec(e["skillIds"]);
    m.lootIds  = copy_json_string_vec(e["lootIds"]);
    const JsonValue& ph = e["phases"];
    if (ph.isArray()) {
        m.phases.reserve(ph.size());
        for (std::size_t i = 0; i < ph.size(); ++i) {
            const JsonValue& p = ph[i];
            MonsterDefinition::MonsterPhase mp;
            mp.name = p["name"].asString("");
            mp.hpThresholdPercent = p["hpThresholdPercent"].asInt(p["hp_threshold_percent"].asInt(0));
            mp.gainedSkillIds = copy_json_string_vec(p["gainedSkillIds"]);
            m.phases.push_back(std::move(mp));
        }
    }
    return m;
}

// -----------------------------------------------------------------
// SummonDefinition
// -----------------------------------------------------------------
template<>
inline SummonDefinition CatalogLoader<SummonDefinition>::from_json(const JsonValue& e) {
    SummonDefinition s;
    s.id = e["id"].asString("");
    s.name = e["name"].asString(s.id);
    s.description = e["description"].asString("");
    s.tier = e["tier"].asInt(0);
    s.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    s.durationRounds = e["durationRounds"].asInt(e["duration_rounds"].asInt(3));
    s.maxHealth = e["maxHealth"].asInt(e["max_health"].asInt(0));
    s.ca = e["ca"].asInt(10);
    s.movement = e["movement"].asInt(5);
    s.stats = statarr_json(e, "stats", "statBonuses");
    s.attackSkillIds = copy_json_string_vec(e["attackSkillIds"]);
    s.passiveIds     = copy_json_string_vec(e["passiveIds"]);
    s.summonRole     = e["summonRole"].asString(e["summon_role"].asString("dps"));
    s.disappearAtZeroHp = e["disappearAtZeroHp"].asBool(e["disappear_at_zero_hp"].asBool(true));
    s.concentrationRequired = e["concentrationRequired"].asBool(e["concentration_required"].asBool(false));
    return s;
}

// -----------------------------------------------------------------
// MountDefinition
// -----------------------------------------------------------------
template<>
inline MountDefinition CatalogLoader<MountDefinition>::from_json(const JsonValue& e) {
    MountDefinition m;
    m.id = e["id"].asString("");
    m.name = e["name"].asString(m.id);
    m.description = e["description"].asString("");
    m.tier = e["tier"].asInt(0);
    m.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    m.price = e["price"].asInt(0);
    m.movement = e["movement"].asInt(e["movementSpeed"].asInt(8));
    m.maxHealth = e["maxHealth"].asInt(e["max_health"].asInt(20));
    m.ca = e["ca"].asInt(12);
    m.carryWeightKg = e["carryWeightKg"].asInt(e["carry_weight_kg"].asInt(100));
    m.mountedStatBonus = statarr_json(e, "mountedStatBonus", "mounted_stat_bonus");
    m.mountedCaBonus = e["mountedCaBonus"].asInt(e["mounted_ca_bonus"].asInt(0));
    m.disengageBonus = e["disengageBonus"].asInt(e["disengage_bonus"].asInt(0));
    m.canFly  = e["canFly"].asBool(e["can_fly"].asBool(false));
    m.canSwim = e["canSwim"].asBool(e["can_swim"].asBool(false));
    m.grantedPassiveIdsWhileMounted = copy_json_string_vec(e["grantedPassiveIdsWhileMounted"]);
    return m;
}

// -----------------------------------------------------------------
// TrapDefinition
// -----------------------------------------------------------------
template<>
inline TrapDefinition CatalogLoader<TrapDefinition>::from_json(const JsonValue& e) {
    TrapDefinition t;
    t.id = e["id"].asString("");
    t.name = e["name"].asString(t.id);
    t.description = e["description"].asString("");
    t.tier = e["tier"].asInt(0);
    t.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    t.damage = e["damage"].asInt(6);
    t.saveAttribute = e["saveAttribute"].asString(e["save_attribute"].asString("DES"));
    t.cd = static_cast<float>(e["cd"].asNumber(1.0));
    t.detectBySearch = e["detectBySearch"].asBool(e["detect_by_search"].asBool(true));
    t.detectDifficulty = e["detectDifficulty"].asInt(e["detect_difficulty"].asInt(0));
    t.applyConditionId = e["applyConditionId"].asString(e["apply_condition_id"].asString(""));
    t.applyConditionRounds = e["applyConditionRounds"].asInt(e["apply_condition_rounds"].asInt(2));
    t.oneShot = e["oneShot"].asBool(e["one_shot"].asBool(true));
    t.cooldownRounds = e["cooldownRounds"].asInt(e["cooldown_rounds"].asInt(0));
    return t;
}

// -----------------------------------------------------------------
// LootTableDefinition
// -----------------------------------------------------------------
template<>
inline LootTableDefinition CatalogLoader<LootTableDefinition>::from_json(const JsonValue& e) {
    LootTableDefinition l;
    l.id = e["id"].asString("");
    l.name = e["name"].asString(l.id);
    l.description = e["description"].asString("");
    l.tier = e["tier"].asInt(0);
    l.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    const JsonValue& entries = e["entries"];
    if (entries.isArray()) {
        l.entries.reserve(entries.size());
        for (std::size_t i = 0; i < entries.size(); ++i) {
            const JsonValue& it = entries[i];
            LootTableDefinition::Entry et;
            et.refId = it["refId"].asString(it["ref_id"].asString(it["id"].asString("")));
            et.weight = it["weight"].asInt(it["w"].asInt(1));
            et.min = it["min"].asInt(1);
            et.max = it["max"].asInt(et.min);
            l.entries.push_back(std::move(et));
        }
    }
    l.minGold = e["minGold"].asInt(e["min_gold"].asInt(0));
    l.maxGold = e["maxGold"].asInt(e["max_gold"].asInt(l.minGold));
    return l;
}

// -----------------------------------------------------------------
// NpcDefinition
// -----------------------------------------------------------------
template<>
inline NpcDefinition CatalogLoader<NpcDefinition>::from_json(const JsonValue& e) {
    NpcDefinition n;
    n.id = e["id"].asString("");
    n.name = e["name"].asString(n.id);
    n.description = e["description"].asString("");
    n.tier = e["tier"].asInt(1);
    n.raceId = e["raceId"].asString(e["race_id"].asString(""));
    n.classId = e["classId"].asString(e["class_id"].asString(""));
    n.factionId = e["factionId"].asString(e["faction_id"].asString(""));
    n.factionRepDefault = e["factionRepDefault"].asInt(e["faction_rep_default"].asInt(0));
    n.occupation = e["occupation"].asString("");
    n.isMerchant = e["isMerchant"].asBool(e["is_merchant"].asBool(false));
    n.shopLootId = e["shopLootId"].asString(e["shop_loot_id"].asString(""));
    n.hasDialogue = e["hasDialogue"].asBool(e["has_dialogue"].asBool(false));
    n.dialogueId = e["dialogueId"].asString(e["dialogue_id"].asString(""));
    n.isHostile = e["isHostile"].asBool(e["is_hostile"].asBool(false));
    n.monsterId = e["monsterId"].asString(e["monster_id"].asString(""));
    n.gold = e["gold"].asInt(0);
    return n;
}

// -----------------------------------------------------------------
// AdventureDefinition
// -----------------------------------------------------------------
template<>
inline AdventureDefinition CatalogLoader<AdventureDefinition>::from_json(const JsonValue& e) {
    AdventureDefinition a;
    a.id = e["id"].asString("");
    a.name = e["name"].asString(a.id);
    a.summary = e["summary"].asString(e["description"].asString(""));
    a.tier = e["tier"].asInt(1);
    a.recommendedTierMin = e["recommendedTierMin"].asInt(e["recommended_tier_min"].asInt(1));
    a.recommendedTierMax = e["recommendedTierMax"].asInt(e["recommended_tier_max"].asInt(5));
    a.questIds = copy_json_string_vec(e["questIds"]);
    a.requiredNpcIds = copy_json_string_vec(e["requiredNpcIds"]);
    a.mainFactionId = e["mainFactionId"].asString(e["main_faction_id"].asString(""));
    a.grantOnStartStoryCardIds = copy_json_string_vec(e["grantOnStartStoryCardIds"]);
    a.grantOnEndStoryCardIds   = copy_json_string_vec(e["grantOnEndStoryCardIds"]);
    a.grantMilestoneOnEnd = e["grantMilestoneOnEnd"].asString(e["grant_milestone_on_end"].asString(""));
    return a;
}

// -----------------------------------------------------------------
// EventDefinition
// -----------------------------------------------------------------
template<>
inline EventDefinition CatalogLoader<EventDefinition>::from_json(const JsonValue& e) {
    EventDefinition ev;
    ev.id = e["id"].asString("");
    ev.name = e["name"].asString(ev.id);
    ev.description = e["description"].asString("");
    ev.tier = e["tier"].asInt(0);
    ev.triggerKind = e["triggerKind"].asString(e["trigger_kind"].asString(e["trigger"].asString("")));
    ev.validZoneIds = copy_json_string_vec(e["validZoneIds"]);
    ev.weight = static_cast<float>(e["weight"].asNumber(1.0));
    const JsonValue& opts = e["options"];
    if (opts.isArray()) {
        ev.options.reserve(opts.size());
        for (std::size_t i = 0; i < opts.size(); ++i) {
            const JsonValue& o = opts[i];
            EventDefinition::Option op;
            op.label = o["label"].asString("");
            op.gold = o["gold"].asInt(0);
            op.applyConditionId = o["applyConditionId"].asString(o["apply_condition_id"].asString(""));
            op.applyConditionRounds = o["applyConditionRounds"].asInt(o["apply_condition_rounds"].asInt(0));
            op.grantQuestId = o["grantQuestId"].asString(o["grant_quest_id"].asString(""));
            op.startCombatWithMonsterId = o["startCombatWithMonsterId"].asString(o["start_combat_with_monster_id"].asString(""));
            op.grantStoryCardId = o["grantStoryCardId"].asString(o["grant_story_card_id"].asString(""));
            op.levelTransitionId = o["levelTransitionId"].asString(o["level_transition_id"].asString(""));
            ev.options.push_back(std::move(op));
        }
    }
    return ev;
}

// -----------------------------------------------------------------
// LocationDefinition
// -----------------------------------------------------------------
template<>
inline LocationDefinition CatalogLoader<LocationDefinition>::from_json(const JsonValue& e) {
    LocationDefinition l;
    l.id = e["id"].asString("");
    l.name = e["name"].asString(l.id);
    l.tier = e["tier"].asInt(0);

    const std::string kind = e["locationType"].asString(e["kind"].asString("city"));
    if (kind == "nation") {
        l.kind = LocationDefinition::Kind::Nation;
    } else if (kind == "zone") {
        l.kind = LocationDefinition::Kind::Zone;
    } else {
        l.kind = LocationDefinition::Kind::City;
    }

    l.controlledBy = e["controlledBy"].asString(e["controlled_by"].asString(""));
    l.description = e["description"].asString("");
    l.polygon = e["polygon"].asString("");

    // position es un objeto {x,y}; las naciones no lo traen.
    const JsonValue& pos = e["position"];
    if (pos.isObject()) {
        l.x = pos["x"].asInt(0);
        l.y = pos["y"].asInt(0);
        l.hasPosition = true;
    }

    l.settlementSize = e["settlementSize"].asString(e["settlement_size"].asString(""));
    l.population = e["population"].asString("");
    l.ruler = e["ruler"].asString("");
    l.trait = e["trait"].asString("");
    l.pendingCanonName = e["pendingCanonName"].asBool(false);

    l.terrain = e["terrain"].asString("");
    l.danger = e["danger"].asString("");

    l.storyIds = copy_json_string_vec(e["storyIds"]);
    l.npcIds   = copy_json_string_vec(e["npcIds"]);
    l.deityIds = copy_json_string_vec(e["deityIds"]);
    l.raceIds  = copy_json_string_vec(e["raceIds"]);
    return l;
}

// ============================================================
// Catalog<T> — implementación de ICatalog<T> con carga JSON.
//
// Todo es header-only (el coste de linkeo de 20 clases similares sería
// ridículo si no). Usa un std::unordered_map<string, unique_ptr<T>> para
// poseer las entradas y devolver const T* no propietarios.
// ============================================================
template <typename T>
class Catalog : public ICatalog<T> {
public:
    Catalog() = default;
    ~Catalog() override = default;

    // No se puede copiar (posee memoria); mover OK.
    Catalog(const Catalog&) = delete;
    Catalog& operator=(const Catalog&) = delete;
    Catalog(Catalog&&) noexcept = default;
    Catalog& operator=(Catalog&&) noexcept = default;

    // Carga desde texto JSON raw (wrapper {"entries": [...]}).
    // Result<int> = OK con número de entradas cargadas, o error.
    Result<int> loadFromString(const std::string& jsonText) {
        auto parsed = JsonValue::parse(jsonText);
        if (!parsed.isOk()) return Result<int>::Error(parsed.errorMessage());
        const JsonValue& root = parsed.value();
        if (!root.isObject()) return Result<int>::Error("Catalog JSON root no es objeto");
        const JsonValue& arr = root["entries"];
        if (!arr.isArray()) return Result<int>::Error("Catalog JSON no tiene clave 'entries' (array)");
        std::size_t ok = 0;
        m_store.clear();
        for (std::size_t i = 0; i < arr.size(); ++i) {
            const JsonValue& entry = arr[i];
            try {
                T t = CatalogLoader<T>::from_json(entry);
                if (t.id.empty()) continue;
                auto owned = std::make_unique<T>(std::move(t));
                const std::string key = owned->id;
                m_store[key] = std::move(owned);
                ++ok;
            } catch (...) {
                // Loader no debería lanzar; si lo hace, continuamos con el resto
            }
        }
        return Result<int>::Ok(static_cast<int>(ok));
    }

    // Carga desde fichero; wrapper que llama a loadFromString.
    Result<int> loadFromFile(const std::string& path) {
        FILE* f = std::fopen(path.c_str(), "rb");
        if (!f) {
            return Result<int>::Error("Catalog: no se puede abrir fichero: " + path);
        }
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

    // Programa (no JSON) — añade una entrada copiada o movida.
    const T* insert(T t) {
        if (t.id.empty()) return nullptr;
        auto owned = std::make_unique<T>(std::move(t));
        const std::string key = owned->id;
        const T* ptr = owned.get();
        m_store[key] = std::move(owned);
        return ptr;
    }

    // ICatalog<T>
    bool has(const std::string& id) const override {
        return m_store.find(id) != m_store.end();
    }
    const T* find(const std::string& id) const override {
        auto it = m_store.find(id);
        if (it == m_store.end()) return nullptr;
        return it->second.get();
    }
    std::size_t size() const override { return m_store.size(); }

    // Iteracion (const) — util para tests
    template <typename Fn>
    void forEach(Fn&& fn) const {
        for (const auto& [k, v] : m_store) fn(*v);
    }

    void clear() { m_store.clear(); }

private:
    std::unordered_map<std::string, std::unique_ptr<T>> m_store;
};

// ============================================================
// Tipos concretos de catálogos (P0 20+). La aplicación GameMachine
// hará:  ClassCatalog classes; classes.loadFromFile("assets/catalogs/classes.json");
// ============================================================
using SkillCatalog       = Catalog<SkillDefinition>;
using SpellCatalog       = Catalog<SkillDefinition>;   // hechizos comparten struct Skills
using ClassCatalog       = Catalog<ClassDefinition>;
using RaceCatalog        = Catalog<RaceDefinition>;
using BackgroundCatalog  = Catalog<BackgroundDefinition>;
using PassiveCatalog     = Catalog<PassiveDefinition>;
using FeatCatalog        = Catalog<FeatDefinition>;
using TraitCatalog       = Catalog<TraitDefinition>;
using DeityCatalog       = Catalog<DeityDefinition>;
using ConditionCatalog   = Catalog<ConditionDefinition>;
using EquipmentCatalog   = Catalog<EquipmentDefinition>;
using ConsumableCatalog  = Catalog<ConsumableDefinition>;
using MonsterCatalog     = Catalog<MonsterDefinition>;
using SummonCatalog      = Catalog<SummonDefinition>;
using MountCatalog       = Catalog<MountDefinition>;
using TrapCatalog        = Catalog<TrapDefinition>;
using LootTableCatalog   = Catalog<LootTableDefinition>;
using NpcCatalog         = Catalog<NpcDefinition>;
using AdventureCatalog   = Catalog<AdventureDefinition>;
using EventCatalog       = Catalog<EventDefinition>;
using LocationCatalog    = Catalog<LocationDefinition>;   // el mundo (B24)

}  // namespace Catalogs
}  // namespace RPG
