#pragma once

// Lo que pone el mapa: trampas, botin, NPCs y tablas de encuentro.
//
// Es la familia que consume el editor al colocar cosas en un nivel, y
// la que crece cuando aparece una mecanica de mundo nueva.
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
inline TrapDefinition CatalogLoader<TrapDefinition>::from_json(const JsonValue& e) {
    TrapDefinition t;
    t.id = e["id"].asString("");
    t.name = e["name"].asString(t.id);
    t.description = e["description"].asString("");
    t.flavorText = e["flavorText"].asString(e["flavor_text"].asString(""));
    t.tier = e["tier"].asInt(0);
    t.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    t.damage = e["damage"].asInt(6);
    t.saveAttribute = e["saveAttribute"].asString(e["save_attribute"].asString("DES"));
    t.cd = static_cast<float>(e["cd"].asNumber(1.0));
    t.detectBySearch = e["detectBySearch"].asBool(e["detect_by_search"].asBool(true));
    t.detectDifficulty = e["detectDifficulty"].asInt(e["detect_difficulty"].asInt(0));
    // D-07 — La carta trae `detection{stat,cd}`, `disarm{stat,cd,herramienta}`
    // y `effect{condicionInfligida,defensa}` (cd en éxitos Nd6: 0.5/1/…).
    // `reset` ("única"/"recargable"/"persistente", 20/16/15) decide oneShot.
    // `trigger`/`onFail` son prosa para el futuro ejecutor.
    const JsonValue& det = e["detection"];
    t.detectStat = det["stat"].asString("");
    t.detectCd = static_cast<float>(det["cd"].asNumber(0.0));
    const JsonValue& dis = e["disarm"];
    t.disarmStat = dis["stat"].asString("");
    t.disarmCd = static_cast<float>(dis["cd"].asNumber(0.0));
    t.disarmTool = dis["herramienta"].asString(dis["tool"].asString(""));
    const JsonValue& ef = e["effect"];
    if (!ef["condicionInfligida"].asString("").empty()) {
        t.applyConditionId = ef["condicionInfligida"].asString("");
    }
    if (!ef["defensa"].asString("").empty()) {
        t.saveAttribute = ef["defensa"].asString("");
    }
    t.category = e["category"].asString("");
    const std::string reset = e["reset"].asString("");
    if (reset == "recargable" || reset == "persistente") {
        t.oneShot = false;
    }
    // D-07 — Las claves planas solo pisan si vienen con valor: si no,
    // borrarían lo leído de `effect`/`reset` de arriba con "".
    {
        const std::string cond = e["applyConditionId"].asString(e["apply_condition_id"].asString(""));
        if (!cond.empty()) t.applyConditionId = cond;
    }
    t.applyConditionRounds = e["applyConditionRounds"].asInt(e["apply_condition_rounds"].asInt(2));
    if (e["oneShot"].isBool() || e["one_shot"].isBool()) {
        t.oneShot = e["oneShot"].asBool(e["one_shot"].asBool(true));
    }
    t.cooldownRounds = e["cooldownRounds"].asInt(e["cooldown_rounds"].asInt(0));
    return t;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<TrapDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"flavorText", "flavor_text"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"category"}, false},
        {{"detection"}, false},
        {{"disarm"}, false},
        {{"effect"}, false},
        {{"reset"}, false},
        {{"damage"}, false},
        {{"saveAttribute", "save_attribute"}, false},
        {{"cd"}, false},
        {{"detectBySearch", "detect_by_search"}, false},
        {{"detectDifficulty", "detect_difficulty"}, false},
        {{"applyConditionId", "apply_condition_id"}, false},
        {{"applyConditionRounds", "apply_condition_rounds"}, false},
        {{"oneShot", "one_shot"}, false},
        {{"cooldownRounds", "cooldown_rounds"}, false},
    };
    return campos;
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
    // D-07 — La carta trae `gold{min,max}` (no minGold plano),
    // `tierRange[2]` (`tier` plano es 0 en las 201), `clue` y el sistema
    // alternativo `drops[]` (chance+condición+item, en paralelo a entries).
    const JsonValue& gold = e["gold"];
    if (gold.isObject()) {
        l.minGold = gold["min"].asInt(l.minGold);
        l.maxGold = gold["max"].asInt(l.maxGold);
    }
    const JsonValue& tr = e["tierRange"];
    if (tr.isArray() && tr.size() >= 2) {
        l.tierMin = tr[static_cast<std::size_t>(0)].asInt(0);
        l.tierMax = tr[static_cast<std::size_t>(1)].asInt(l.tierMin);
    }
    l.clue = e["clue"].asString("");
    const JsonValue& dr = e["drops"];
    if (dr.isArray()) {
        l.drops.reserve(dr.size());
        for (std::size_t i = 0; i < dr.size(); ++i) {
            const JsonValue& x = dr[i];
            LootTableDefinition::Drop dp;
            dp.itemId = x["item"].asString(x["itemId"].asString(x["refId"].asString("")));
            dp.chance = x["chance"].asInt(0);
            dp.condition = x["condition"].asString("");
            l.drops.push_back(std::move(dp));
        }
    }
    return l;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<LootTableDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"entries"}, false},
        {{"minGold", "min_gold"}, false},
        {{"maxGold", "max_gold"}, false},
        {{"tierRange"}, false},
        {{"gold"}, false},
        {{"clue"}, false},
        {{"drops"}, false},
    };
    return campos;
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

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<NpcDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"tier"}, false},
        {{"raceId", "race_id"}, false},
        {{"classId", "class_id"}, false},
        {{"factionId", "faction_id"}, false},
        {{"factionRepDefault", "faction_rep_default"}, false},
        {{"occupation"}, false},
        {{"isMerchant", "is_merchant"}, false},
        {{"shopLootId", "shop_loot_id"}, false},
        {{"hasDialogue", "has_dialogue"}, false},
        {{"dialogueId", "dialogue_id"}, false},
        {{"isHostile", "is_hostile"}, false},
        {{"monsterId", "monster_id"}, false},
        {{"gold"}, false},
    };
    return campos;
}

// -----------------------------------------------------------------
// EncounterTableDefinition (GDB-10.3)
// -----------------------------------------------------------------
template<>
inline EncounterTableDefinition CatalogLoader<EncounterTableDefinition>::from_json(const JsonValue& json) {
    EncounterTableDefinition enc;
    enc.id = json["id"].asString("");
    enc.name = json["name"].asString(enc.id);
    enc.description = json["description"].asString("");
    enc.tier = json["tier"].asInt(0);
    enc.rarity = rarity_from_json(json["rarity"], ItemRarity::Common);
    const JsonValue& entries = json["entries"];
    if (entries.isArray()) {
        enc.entries.reserve(entries.size());
        for (std::size_t i = 0; i < entries.size(); ++i) {
            const JsonValue& it = entries[i];
            EncounterTableDefinition::Entry et;
            et.refId = it["refId"].asString(it["ref_id"].asString(it["id"].asString("")));
            et.weight = it["weight"].asInt(it["w"].asInt(1));
            et.min = it["min"].asInt(1);
            et.max = it["max"].asInt(et.min);
            enc.entries.push_back(std::move(et));
        }
    }
    const JsonValue& conditions = json["conditions"];
    if (conditions.isArray()) {
        enc.conditions.reserve(conditions.size());
        for (std::size_t i = 0; i < conditions.size(); ++i) {
            const JsonValue& c = conditions[i];
            EncounterTableDefinition::Condition cond;
            cond.type = c["type"].asString("");
            cond.value = c["value"].asString("");
            cond.comparison = c["comparison"].asString("");
            enc.conditions.push_back(std::move(cond));
        }
    }
    enc.lootTableId = json["lootTableId"].asString("");
    enc.minPartyLevel = json["minPartyLevel"].asInt(1);
    enc.maxPartyLevel = json["maxPartyLevel"].asInt(5);
    return enc;
}

template<>
inline const std::vector<Catalogs::CatalogField>& Catalogs::CatalogLoader<EncounterTableDefinition>::expected_fields() {
    static const std::vector<Catalogs::CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"entries"}, false},
        {{"conditions"}, false},
        {{"lootTableId"}, false},
        {{"minPartyLevel"}, false},
        {{"maxPartyLevel"}, false},
    };
    return campos;
}

// -----------------------------------------------------------------
// AdventureDefinition
// -----------------------------------------------------------------

}  // namespace Catalogs
}  // namespace RPG
