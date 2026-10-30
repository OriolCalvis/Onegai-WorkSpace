#pragma once

// Lo que cuenta la historia: aventuras, eventos y lugares.
//
// Aguas arriba de todo esto esta el canon (Onegai 2). Aqui solo se lee.
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

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<AdventureDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"summary", "description"}, false},
        {{"tier"}, false},
        {{"recommendedTierMin", "recommended_tier_min"}, false},
        {{"recommendedTierMax", "recommended_tier_max"}, false},
        {{"questIds"}, false},
        {{"requiredNpcIds"}, false},
        {{"mainFactionId", "main_faction_id"}, false},
        {{"grantOnStartStoryCardIds"}, false},
        {{"grantOnEndStoryCardIds"}, false},
        {{"grantMilestoneOnEnd", "grant_milestone_on_end"}, false},
    };
    return campos;
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

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<EventDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"tier"}, false},
        {{"triggerKind", "trigger_kind", "trigger"}, false},
        {{"validZoneIds"}, false},
        {{"weight"}, false},
        {{"options"}, false},
    };
    return campos;
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

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<LocationDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"tier"}, false},
        {{"locationType", "kind"}, false},
        {{"controlledBy", "controlled_by"}, false},
        {{"description"}, false},
        {{"polygon"}, false},
        {{"position"}, false},
        {{"settlementSize", "settlement_size"}, false},
        {{"population"}, false},
        {{"ruler"}, false},
        {{"trait"}, false},
        {{"pendingCanonName"}, false},
        {{"terrain"}, false},
        {{"danger"}, false},
        {{"storyIds"}, false},
        {{"npcIds"}, false},
        {{"deityIds"}, false},
        {{"raceIds"}, false},
    };
    return campos;
}

}  // namespace Catalogs
}  // namespace RPG
