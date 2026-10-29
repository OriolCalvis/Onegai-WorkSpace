#pragma once

// Lo que define quien eres: clase, raza y trasfondo.
//
// Las tres van juntas porque la creacion de personaje las lee de golpe
// y porque comparten vocabulario (competencias, bonos, equipo inicial).
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
inline ClassDefinition CatalogLoader<ClassDefinition>::from_json(const JsonValue& e) {
    ClassDefinition c;
    c.id = e["id"].asString("");
    c.name = e["name"].asString(c.id);
    c.description = e["description"].asString("");
    c.lore = e["lore"].asString(e["flavorText"].asString(e["flavor_text"].asString("")));
    c.tier = e["tier"].asInt(1);
    c.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    c.role = e["role"].asString("");
    c.playable = e["playable"].asBool(true);
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
    // "startingCards" es un OBJETO {skills, spells, passives, learnableSkills},
    // no un array. Leerlo con copy_json_string_vec devolvia vacio siempre, y
    // "startingEquipmentIds" no es la clave del JSON (es "startingEquipment"),
    // asi que las 61 clases se cargaban SIN habilidades y SIN equipo. No daba
    // ningun error: daba un personaje con la hoja en blanco, que es justo lo
    // que el GDD dice que no puede pasar ("nadie empieza con una hoja en
    // blanco"). Salio al montar la creacion de personaje.
    const JsonValue& cards = e["startingCards"];
    if (cards.isObject()) {
        // skills y spells juntos: startingSkillIds documenta "skill_* y spell_*".
        c.startingSkillIds = copy_json_string_vec(cards["skills"]);
        for (const std::string& s : copy_json_string_vec(cards["spells"])) {
            c.startingSkillIds.push_back(s);
        }
        c.learnableSkillIds = copy_json_string_vec(cards["learnableSkills"]);
        const std::vector<std::string> pas = copy_json_string_vec(cards["passives"]);
        if (c.passiveId.empty() && !pas.empty()) {
            c.passiveId = pas.front();
        }
    } else {
        c.startingSkillIds = copy_json_string_vec(cards);
    }
    if (c.startingSkillIds.empty()) c.startingSkillIds = copy_json_string_vec(e["startingSkillIds"]);
    c.startingEquipmentIds = copy_json_string_vec(e["startingEquipment"]);
    if (c.startingEquipmentIds.empty())
        c.startingEquipmentIds = copy_json_string_vec(e["startingEquipmentIds"]);
    c.specializationIds    = copy_json_string_vec(e["specializationIds"]);
    c.canMulticlassInto = e["canMulticlassInto"].asBool(e["can_multiclass_into"].asBool(false));
    c.restrictedTags = copy_json_string_vec(e["restrictedTags"]);
    return c;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<ClassDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"lore", "flavorText", "flavor_text"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"role"}, false},
        {{"baseHealth", "base_health"}, false},
        {{"primaryStat", "primary_stat"}, false},
        {{"secondaryStat", "secondary_stat"}, false},
        {{"maxSkillCards", "max_skill_cards"}, false},
        {{"compatibilityWeapons"}, false},
        {{"compatibilityArmor"}, false},
        {{"allowedEquipmentTags"}, false},
        {{"maxArmorWeight", "max_armor_weight"}, false},
        {{"maxWeaponWeight", "max_weapon_weight"}, false},
        {{"passiveId", "passive_id"}, false},
        {{"startingCards"}, false},
        {{"startingSkillIds"}, false},
        {{"startingEquipment"}, false},
        {{"startingEquipmentIds"}, false},
        {{"specializationIds"}, false},
        {{"canMulticlassInto", "can_multiclass_into"}, false},
        {{"restrictedTags"}, false},
        {{"playable"}, false},
    };
    return campos;
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
    // D-01 (17/09) — Estas dos lecturas se perdieron en f23dba9 mientras
    // su declaracion ganaba los alias del asset («speed», 5 de 43 razas;
    // «compatibleDeities», 4). Desde entonces toda raza cargaba velocidad
    // 30 y ninguna deidad compatible, sin error: lo detecto
    // tools/comprobar_contrato_cargadores.py. El nombre del asset va al
    // final de la cadena para que los canonicos antiguos sigan mandando.
    r.baseSpeed = e["baseSpeed"].asInt(
        e["movementSpeed"].asInt(e["movement_speed"].asInt(e["speed"].asInt(30))));
    r.compatibleDeityIds = copy_json_string_vec(
        e.has("compatibleDeityIds") ? e["compatibleDeityIds"] : e["compatibleDeities"]);
    r.languages = copy_json_string_vec(e["languages"]);
    r.narrativeTags = copy_json_string_vec(e["narrativeTags"]);
    r.statBonuses = statarr_json(e, "statBonuses", "stat_bonuses");
    r.passiveTraitId = e["passiveTraitId"].asString(e["passive_trait_id"].asString(""));
    r.activeTraitId  = e["activeTraitId"].asString(e["active_trait_id"].asString(""));
    r.racialTraitId  = e["racialTraitId"].asString(e["racial_trait_id"].asString(""));
    r.affinities  = copy_json_string_vec(e["affinities"]);
    r.limitations = copy_json_string_vec(e["limitations"]);
    r.unlocks     = copy_json_string_vec(e["unlocks"]);
    r.playable    = e["playable"].asBool(true);
    return r;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<RaceDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"flavorText", "flavor_text"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"baseSpeed", "movementSpeed", "movement_speed", "speed"}, false},
        {{"languages"}, false},
        {{"compatibleDeityIds", "compatibleDeities"}, false},
        {{"narrativeTags"}, false},
        {{"statBonuses", "stat_bonuses"}, false},
        {{"passiveTraitId", "passive_trait_id"}, false},
        {{"activeTraitId", "active_trait_id"}, false},
        {{"racialTraitId", "racial_trait_id"}, false},
        {{"affinities"}, false},
        {{"limitations"}, false},
        {{"unlocks"}, false},
        {{"playable"}, false},
    };
    return campos;
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

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<BackgroundDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"startingWealthGold", "starting_wealth_gold"}, false},
        {{"virtue"}, false},
        {{"defect"}, false},
        {{"goal"}, false},
        {{"startingEquipmentIds"}, false},
        {{"skillProficiencyIds"}, false},
        {{"featureIds"}, false},
        {{"factionId", "faction_id"}, false},
        {{"startingFactionRep", "starting_faction_rep"}, false},
    };
    return campos;
}

// -----------------------------------------------------------------
// PassiveDefinition
// -----------------------------------------------------------------

}  // namespace Catalogs
}  // namespace RPG
