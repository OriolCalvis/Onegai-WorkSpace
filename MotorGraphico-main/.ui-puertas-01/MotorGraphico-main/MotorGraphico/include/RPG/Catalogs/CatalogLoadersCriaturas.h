#pragma once

// Lo que respira enfrente: monstruos, invocaciones y monturas.
//
// Los tres comparten la forma de una ficha de combate. Monsters es el
// mas grande (432 fichas) y el que mas revisiones de datos recibe.
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
    // D-03 — La vida y las defensas viven en `defenses{}` (ingles) con
    // respaldo en `derived{}` (castellano: vida, defensaMental,
    // resistenciaFisica, movimiento); la raiz solo trae legados.
    // DEC-02: manda el asset, el C++ se adapta. Antes todo cargaba
    // maxHealth = 0 y ca = 10 porque se buscaba en la raiz.
    const JsonValue& defensas = e["defenses"];
    const JsonValue& derived = e["derived"];
    m.maxHealth = defensas["maxHealth"].asInt(derived["vida"].asInt(
        e["maxHealth"].asInt(e["max_health"].asInt(e["health"].asInt(0)))));
    m.ca = defensas["ca"].asInt(derived["ca"].asInt(
        e["ca"].asInt(e["ac"].asInt(10))));
    m.defMental = defensas["defMental"].asInt(derived["defensaMental"].asInt(
        e["defMental"].asInt(e["def_mental"].asInt(10))));
    m.resFis = defensas["resFis"].asInt(derived["resistenciaFisica"].asInt(
        e["resFis"].asInt(e["res_fis"].asInt(10))));
    m.precMag = defensas["precMag"].asInt(
        e["precMag"].asInt(e["prec_mag"].asInt(10)));
    m.movement = defensas["movement"].asInt(derived["movimiento"].asInt(
        e["movement"].asInt(e["movementSpeed"].asInt(5))));
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
    // D-03 — El botin va en `loot`, no en `lootIds` (siempre vacio antes).
    // `passive` es objeto {name, description}, no id: queda para D-11.
    const JsonValue& loot = e["lootIds"];
    m.lootIds  = copy_json_string_vec(loot.isArray() ? loot : e["loot"]);
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

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<MonsterDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"flavorText", "flavor_text"}, false},
        {{"description"}, false},
        {{"rank"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"role"}, false},
        {{"faction", "factionId"}, false},
        {{"factionName"}, false},
        {{"stats", "statBonuses"}, false},
        {{"defenses"}, false},
        {{"derived"}, false},
        {{"maxHealth", "max_health", "health"}, false},
        {{"ca", "ac"}, false},
        {{"defMental", "def_mental"}, false},
        {{"resFis", "res_fis"}, false},
        {{"precMag", "prec_mag"}, false},
        {{"movement", "movementSpeed"}, false},
        {{"attacks"}, false},
        {{"passiveId", "passive_id"}, false},
        {{"conditionsInflicted"}, false},
        {{"skillIds"}, false},
        {{"lootIds", "loot"}, false},
        {{"phases"}, false},
    };
    return campos;
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
    // D-07 — `duration` es string ("concentration" 117 / "1_min" 84): se
    // guarda en crudo y "concentration" activa el flag que el struct ya
    // preveía. "1_min" a rondas lo decide D-11 O2, no este cargador.
    // `health` (201/201) es la vida: `maxHealth` no existe en la carta.
    s.durationRaw = e["duration"].asString("");
    s.concentrationRequired = (s.durationRaw == "concentration") ||
        e["concentrationRequired"].asBool(e["concentration_required"].asBool(false));
    s.durationRounds = e["durationRounds"].asInt(e["duration_rounds"].asInt(3));
    s.maxHealth = e["maxHealth"].asInt(e["max_health"].asInt(e["health"].asInt(0)));
    // D-07 — Quién lo manda y qué carta lo invoca (en crudo).
    s.control = e["control"].asString("");
    s.summonedBySkillId = e["summonedBy"].asString(e["summoned_by"].asString(""));
    s.ca = e["ca"].asInt(10);
    s.movement = e["movement"].asInt(5);
    s.stats = statarr_json(e, "stats", "statBonuses");
    s.attackSkillIds = copy_json_string_vec(e["attackSkillIds"]);
    s.passiveIds     = copy_json_string_vec(e["passiveIds"]);
    s.summonRole     = e["summonRole"].asString(e["summon_role"].asString("dps"));
    s.disappearAtZeroHp = e["disappearAtZeroHp"].asBool(e["disappear_at_zero_hp"].asBool(true));
    // D-07 — `concentrationRequired` ya se resolvió arriba (implica el
    // `duration: "concentration"` de la carta); no releerlo en plano.
    return s;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<SummonDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"durationRounds", "duration_rounds"}, false},
        {{"duration"}, false},
        {{"maxHealth", "max_health", "health"}, false},
        {{"ca"}, false},
        {{"movement"}, false},
        {{"control"}, false},
        {{"summonedBy", "summoned_by"}, false},
        {{"stats", "statBonuses"}, false},
        {{"attackSkillIds"}, false},
        {{"passiveIds"}, false},
        {{"summonRole", "summon_role"}, false},
        {{"disappearAtZeroHp", "disappear_at_zero_hp"}, false},
        {{"concentrationRequired", "concentration_required"}, false},
    };
    return campos;
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
    m.stats = statarr_json(e, "stats", "statBonuses");
    // D-07 — La carta trae `health` (no `maxHealth`), `armor` (no `ca`) y
    // `capacity` en plazas (no kg). `ridingRequirements{minimo, stat}`,
    // `terrain[]`, `mountedTags[]` y `dismountTrigger` en crudo.
    m.maxHealth = e["maxHealth"].asInt(e["max_health"].asInt(e["health"].asInt(20)));
    m.ca = e["ca"].asInt(e["armor"].asInt(12));
    m.capacity = e["capacity"].asInt(1);
    m.terrain = copy_json_string_vec(e["terrain"]);
    m.mountedTags = copy_json_string_vec(e["mountedTags"]);
    m.dismountTrigger = e["dismountTrigger"].asString(e["dismount_trigger"].asString(""));
    const JsonValue& req = e["ridingRequirements"];
    m.requiredRideStat = req["stat"].asString("");
    m.requiredRideMin = req["minimo"].asInt(req["minimum"].asInt(0));
    m.upkeep = e["upkeep"].asString("");
    m.carryWeightKg = e["carryWeightKg"].asInt(e["carry_weight_kg"].asInt(100));
    m.mountedStatBonus = statarr_json(e, "mountedStatBonus", "mounted_stat_bonus");
    m.mountedCaBonus = e["mountedCaBonus"].asInt(e["mounted_ca_bonus"].asInt(0));
    m.disengageBonus = e["disengageBonus"].asInt(e["disengage_bonus"].asInt(0));
    m.canFly  = e["canFly"].asBool(e["can_fly"].asBool(false));
    m.canSwim = e["canSwim"].asBool(e["can_swim"].asBool(false));
    m.grantedPassiveIdsWhileMounted = copy_json_string_vec(e["grantedPassiveIdsWhileMounted"]);
    return m;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<MountDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"price"}, false},
        {{"movement", "movementSpeed"}, false},
        {{"stats", "statBonuses"}, false},
        {{"maxHealth", "max_health", "health"}, false},
        {{"ca", "armor"}, false},
        {{"capacity"}, false},
        {{"terrain"}, false},
        {{"mountedTags"}, false},
        {{"dismountTrigger", "dismount_trigger"}, false},
        {{"ridingRequirements"}, false},
        {{"upkeep"}, false},
        {{"carryWeightKg", "carry_weight_kg"}, false},
        {{"mountedStatBonus", "mounted_stat_bonus"}, false},
        {{"mountedCaBonus", "mounted_ca_bonus"}, false},
        {{"disengageBonus", "disengage_bonus"}, false},
        {{"canFly", "can_fly"}, false},
        {{"canSwim", "can_swim"}, false},
        {{"grantedPassiveIdsWhileMounted"}, false},
    };
    return campos;
}

// -----------------------------------------------------------------
// TrapDefinition
// -----------------------------------------------------------------

}  // namespace Catalogs
}  // namespace RPG
