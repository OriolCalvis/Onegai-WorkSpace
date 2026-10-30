#pragma once

// Habilidades y conjuros (492 cartas publicadas).
//
// La familia mas grande y la que mas cambia: cada carta nueva del canon
// entra por aqui. Separada para que tocarla no recompile los 34
// consumidores de RpgCatalogs.h.
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

    std::string casting = tolower_inline(e["castingStat"].asString(e["casting_stat"].asString("")));
    // D-02 — La carta manda cuando declara (68 skills + los 210 hechizos
    // lo hacen). Vacío = carta física sin stat firmante: DES, el stat del
    // ataque básico (GDD §7.1: "la característica implicada"; puente
    // Skill.cpp ApplyBasicAttackNd6). Antes el vacío caía a INT sin que
    // ninguna carta lo pidiera. Las 9 utilitarias narrativas (las únicas
    // que usan los skillCheck de las aventuras publicadas) declaran su
    // stat, así que ningún contenido cambia.
    if (casting == "con") s.casting_stat = Stat::CON;
    else if (casting == "des") s.casting_stat = Stat::DES;
    else if (casting == "car") s.casting_stat = Stat::CAR;
    else if (casting == "int") s.casting_stat = Stat::INT;
    else s.casting_stat = Stat::DES;

    // D-02 — La defensa que la carta fija al objetivo. El canónico del
    // asset es defenseStat (465/492 skills); saveAttribute es un alias
    // legado que hoy solo aparece VACÍO en las 9 utilitarias, y se acepta
    // por si vuelve con contenido ("con"/"des"/...). Mapeo a Ed. 3 (GDD
    // §5 + bitácora 2.17 regla 4): defensa_mental->CAR,
    // resistencia_fisica->CON, precision_magica->INT; "CA" y vacío->NONE
    // porque la física sin prefijo se defiende en enfrentada con el MAYOR
    // de DES/CON. El crudo se guarda aparte para HUD/selectores (E-14).
    s.defense_stat = e["defenseStat"].asString("");
    const std::string save_alias =
        e["saveAttribute"].asString(e["save_attribute"].asString(""));
    if (!save_alias.empty()) {
        s.save_attribute = save_attribute_from_string(save_alias);
    } else {
        const std::string defense = tolower_inline(s.defense_stat);
        if (defense == "defensa_mental")         s.save_attribute = SaveAttribute::CAR;
        else if (defense == "resistencia_fisica") s.save_attribute = SaveAttribute::CON;
        else if (defense == "precision_magica")   s.save_attribute = SaveAttribute::INT;
        else                                      s.save_attribute = SaveAttribute::NONE;
    }
    s.override_cd_if_save = static_cast<float>(e["overrideCdIfSave"].asNumber(e["override_cd_if_save"].asNumber(0.0)));

    // magnitudeByDegree: lo que la carta declara, en cualquiera de sus
    // dos formas. D-02:
    //   - array [BOTCH, PARTIAL, SUCCESS, CRITICAL] (forma clásica), o
    //   - objeto {botch, partial, success, critical} — la forma que traen
    //     las 9 utilitarias narrativas (percepcion, sigilo...): un check
    //     no hace daño y lo declara a cero. Antes caían silenciosamente
    //     al fallback {0, 2, 4, 8}.
    // Si la carta no declara magnitudes (null en las 483 restantes), la
    // base se DERIVA del tier de la propia carta: base = 2 + 2·tier
    // (t1->4 ... t5->12), que reproduce la escalera de presets
    // MAGNITUDE_PRESETS ("Petit (1d6)" base 4, "Mitjà (1d8)" 6, "Gran
    // (1d10)" 8, "Llegendari (1d12)" 12) — el mapeo legacy-dados del GDD
    // §7.6 regla 4. El tier 1 queda EXACTAMENTE como antes ({0,2,4,8}),
    // así que los números plantados de la campaña no cambian. Si algún
    // día las cartas declaran magnitudes reales aguas arriba, su valor
    // SIEMPRE gana a esta derivación.
    const JsonValue& mb = e["magnitudeByDegree"];
    if (mb.isArray() && mb.size() >= 4) {
        for (int i = 0; i < 4; ++i) s.magnitude_by_degree[i] = static_cast<float>(mb[i].asNumber(0.0));
    } else if (mb.isObject()) {
        s.magnitude_by_degree[static_cast<int>(Degree::BOTCH)] =
            static_cast<float>(mb["botch"].asNumber(0.0));
        s.magnitude_by_degree[static_cast<int>(Degree::PARTIAL)] =
            static_cast<float>(mb["partial"].asNumber(0.0));
        s.magnitude_by_degree[static_cast<int>(Degree::SUCCESS)] =
            static_cast<float>(mb["success"].asNumber(0.0));
        s.magnitude_by_degree[static_cast<int>(Degree::CRITICAL)] =
            static_cast<float>(mb["critical"].asNumber(0.0));
    } else {
        float base = -1.0f;
        if (e.has("magnitudeBase")) base = static_cast<float>(e["magnitudeBase"].asNumber(0.0));
        else if (e.has("damage"))   base = static_cast<float>(e["damage"].asNumber(0.0));
        else if (e.has("heal"))     base = static_cast<float>(e["heal"].asNumber(0.0));
        if (base < 0.0f) {
            int tier = s.tier_min;
            if (tier < TIER_MIN) tier = TIER_MIN;
            if (tier > TIER_MAX) tier = TIER_MAX;
            base = static_cast<float>(2 + 2 * tier);
        }
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

    // D-02 — cost{resource, amount}, range y effect.scaling: presentes en
    // el 94-99% de las skills y hasta hoy ignorados por el cargador.
    const JsonValue& cost = e["cost"];
    if (cost.isObject()) {
        s.cost_resource = cost["resource"].asString("");
        s.cost_amount = cost["amount"].asInt(0);
    }
    s.range = skill_range_from_string(e["range"].asString(""));
    const JsonValue& effect = e["effect"];
    if (effect.isObject()) {
        s.effect_scaling = effect["scaling"].asString("");
    }

    s.tags               = copy_json_string_vec(e["tags"]);
    s.required_tags      = copy_json_string_vec(e["requiredTags"]);
    s.incompatible_tags  = copy_json_string_vec(e["incompatibleTags"]);
    s.granted_summon_id  = e["grantedSummonId"].asString(e["granted_summon_id"].asString(""));
    // H2-06 — effects[].mechanicHook, crudos (mismo DSL que conditions.json).
    const JsonValue& efectos = e["effects"];
    for (std::size_t i = 0; efectos.isArray() && i < efectos.size(); ++i) {
        const std::string hook = efectos[i]["mechanicHook"].asString("");
        if (!hook.empty()) s.mechanicHooks.push_back(hook);
    }
    return s;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<SkillDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"flavorText", "flavor_text"}, false},
        {{"tier", "tierMin", "tier_min"}, false},
        {{"rarity"}, false},
        {{"actionType", "action_type"}, false},
        {{"channelRounds", "channel_rounds"}, false},
        {{"recovery", "recoveryPile", "recovery_pile"}, false},
        {{"target"}, false},
        {{"castingStat", "casting_stat"}, false},
        {{"saveAttribute", "save_attribute"}, false},
        {{"defenseStat"}, false},
        {{"cost"}, false},
        {{"range"}, false},
        {{"effect"}, false},
        {{"overrideCdIfSave", "override_cd_if_save"}, false},
        {{"magnitudeByDegree"}, false},
        {{"magnitudeBase", "damage", "heal"}, false},
        {{"magnitudeType", "magnitude_type"}, false},
        {{"applyConditionId", "apply_condition_id"}, false},
        {{"applyConditionMinDegree", "apply_condition_min_degree"}, false},
        {{"conditionDurationRounds", "condition_duration_rounds"}, false},
        {{"tags"}, false},
        {{"requiredTags"}, false},
        {{"incompatibleTags"}, false},
        {{"grantedSummonId", "granted_summon_id"}, false},
        {{"effects"}, false},
    };
    return campos;
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

}  // namespace Catalogs
}  // namespace RPG
