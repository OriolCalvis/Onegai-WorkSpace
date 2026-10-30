#pragma once

// Lo que llevas puesto sin elegirlo: pasivas, dotes, rasgos y deidades.
//
// Cuatro catalogos pequenos que se consultan al resolver una tirada o
// un favor. Cambian poco y casi siempre a la vez.
//
// Salio de include/RPG/Catalogs/RpgCatalogs.h el 23/09/2026 (EST-22):
// aquella cabecera tenia 1922 lineas, la incluian 34 ficheros y la cruzan
// 8 paquetes del plan. Un campo nuevo en una carta obligaba a recompilar
// medio motor. Se movio, no se reescribio.
//
// No se incluye directamente: entra por RpgCatalogs.h, que sigue siendo
// el paraguas de siempre.

#include "RPG/Catalogs/CatalogLoader.h"

namespace RPG {
namespace Catalogs {

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
    // H2-06 — effects[].mechanicHook, crudos (mismo DSL que conditions.json).
    const JsonValue& efectos = e["effects"];
    for (std::size_t i = 0; efectos.isArray() && i < efectos.size(); ++i) {
        const std::string hook = efectos[i]["mechanicHook"].asString("");
        if (!hook.empty()) p.mechanicHooks.push_back(hook);
    }
    return p;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<PassiveDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"flavorText"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"trigger"}, false},
        {{"extraDiceOnTrigger", "extra_dice_on_trigger"}, false},
        {{"successBonusOnTrigger", "success_bonus_on_trigger"}, false},
        {{"requiredTag", "required_tag"}, false},
        {{"statBonuses", "stat_bonuses"}, false},
        {{"armorBonus", "armor_bonus"}, false},
        {{"physicalSaveBonus", "physical_save_bonus"}, false},
        {{"mentalSaveBonus", "mental_save_bonus"}, false},
        {{"spellSaveBonus", "spell_save_bonus"}, false},
        {{"maxHealthBonus", "max_health_bonus"}, false},
        {{"handSizeBonus", "hand_size_bonus"}, false},
        {{"effects"}, false},
    };
    return campos;
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

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<FeatDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"tierMin", "tier_min"}, false},
        {{"rarity"}, false},
        {{"requiredClassId", "required_class_id"}, false},
        {{"requiredBackgroundId", "required_background_id"}, false},
        {{"requiredTier", "required_tier"}, false},
        {{"statBonuses", "stat_bonuses"}, false},
        {{"grantedPassiveIds"}, false},
        {{"grantedSkillIds"}, false},
    };
    return campos;
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

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<TraitDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"tierMin", "tier_min"}, false},
        {{"rarity"}, false},
        {{"isActive", "is_active"}, false},
        {{"timesPerRest", "times_per_rest"}, false},
        {{"recovery"}, false},
        {{"linkedSkillId", "linked_skill_id"}, false},
    };
    return campos;
}

// -----------------------------------------------------------------
// DeityDefinition
// -----------------------------------------------------------------
// D-07 — Parte `domain` ("forja, artesanía y juramentos") por comas y
// recorta. SOLO por comas: el " y " interior ("noche urbana y protección
// de los vulnerables", sin coma) se conserva tal cual — partirlo sería
// inventar fronteras que la carta no marca.
static inline std::vector<std::string> split_comma_trim(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char ch : s) {
        if (ch == ',') {
            std::size_t a = cur.find_first_not_of(" \t");
            std::size_t b = cur.find_last_not_of(" \t");
            if (a != std::string::npos) out.push_back(cur.substr(a, b - a + 1));
            cur.clear();
        } else {
            cur.push_back(ch);
        }
    }
    std::size_t a = cur.find_first_not_of(" \t");
    std::size_t b = cur.find_last_not_of(" \t");
    if (a != std::string::npos) out.push_back(cur.substr(a, b - a + 1));
    return out;
}

template<>
inline DeityDefinition CatalogLoader<DeityDefinition>::from_json(const JsonValue& e) {
    DeityDefinition d;
    d.id = e["id"].asString("");
    d.name = e["name"].asString(d.id);
    d.title = e["title"].asString("");
    d.alignment = e["alignment"].asString("");
    d.description = e["description"].asString("");
    d.flavorText = e["flavorText"].asString(e["flavor_text"].asString(""));
    d.rank = e["rank"].asString("");
    d.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    d.tier_min = e["tierMin"].asInt(e["tier_min"].asInt(e["tier"].asInt(0)));
    // D-07 — `domain` (string, 10/10) es la fuente; `domains` (array) queda
    // como alias por si alguna carta futura lo trae ya partido.
    d.domains = copy_json_string_vec(e["domains"]);
    if (d.domains.empty()) {
        d.domains = split_comma_trim(e["domain"].asString(""));
    }
    d.compatibleWithIds = copy_json_string_vec(e["compatibleWith"]);
    d.obligations = copy_json_string_vec(e["obligations"]);
    const JsonValue& favor = e["favor"];
    d.favorDescription = favor["description"].asString("");
    d.favorScaling = favor["scaling"].asString("");
    d.symbolTags = copy_json_string_vec(e["symbolTags"]);
    d.prayerSkillIds    = copy_json_string_vec(e["prayerSkillIds"]);
    // D-07 — La carta los trae en `grantedSpells` (100/100 resuelven en
    // spells.json): alias que se suma, no campo nuevo.
    for (const auto& s : copy_json_string_vec(e["grantedSpells"])) {
        d.prayerSkillIds.push_back(s);
    }
    d.grantedPassiveIds = copy_json_string_vec(e["grantedPassiveIds"]);
    d.dogmaStatBonus = statarr_json(e, "dogmaStatBonus", "dogma_stat_bonus");
    return d;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<DeityDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"title"}, false},
        {{"alignment"}, false},
        {{"description"}, false},
        {{"flavorText", "flavor_text"}, false},
        {{"rank"}, false},
        {{"rarity"}, false},
        {{"domains", "domain"}, false},
        {{"compatibleWith"}, false},
        {{"obligations"}, false},
        {{"favor"}, false},
        {{"symbolTags"}, false},
        {{"tierMin", "tier_min", "tier"}, false},
        {{"prayerSkillIds", "grantedSpells"}, false},
        {{"grantedPassiveIds"}, false},
        {{"dogmaStatBonus", "dogma_stat_bonus"}, false},
    };
    return campos;
}

// -----------------------------------------------------------------
// ConditionDefinition — D-04: lee el asset real (DEC-02).
//
// El cargador antiguo esperaba stacking/defaultRounds/damagePerRound/
// isNegative/caMod... y el asset publicado (las 16 cartas) habla
// stackable/duration/effects[]. Resultado: las 16 condiciones cargaban
// IDÉNTICAS (DURATION, 3 rondas, 1 stack, 0 daño, todo apagado) y nadie
// lo notaba porque el modo estructural solo exige id/name.
//
// El mapeo es conservador: cada campo del asset se traduce a lo que ya
// existe en el motor SOLO si la traducción es unívoca y respaldada por
// el GDD; el resto del DSL de la carta queda crudo en mechanicHooks
// para D-11/E-14/G-13. Nada se inventa: lo que la carta no dice, no se
// rellena con números propios (maxStacks sigue a 1 aunque fatiga sea
// stackable, porque ninguna carta declara el máximo).
// -----------------------------------------------------------------

}  // namespace Catalogs
}  // namespace RPG
