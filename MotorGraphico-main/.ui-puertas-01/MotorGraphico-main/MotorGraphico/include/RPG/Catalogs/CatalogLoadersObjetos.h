#pragma once

// Condiciones, equipo y consumibles: lo que entra y sale del inventario.
//
// Las condiciones estan aqui y no con los rasgos porque quien las aplica
// es un objeto o un golpe, no la ficha.
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
inline ConditionDefinition CatalogLoader<ConditionDefinition>::from_json(const JsonValue& e) {
    ConditionDefinition c;
    c.id = e["id"].asString("");
    c.name = e["name"].asString(c.id);
    c.description = e["description"].asString("");

    // stackable (bool, 16/16 cartas): apilable = suma intensidad; no
    // apilable = reemplazo. DURATION (refrescar sin apilar) no lo pide
    // ninguna carta publicada y se conserva solo como API del motor.
    c.stacking = e["stackable"].asBool(false)
                     ? ConditionDefinition::Stacking::STACK_INTENSITY
                     : ConditionDefinition::Stacking::NONE;

    // duration ("hasta_curarse" | "1_turn" | "N_turns" | número):
    // 15 de las 16 cartas son "hasta_curarse" — no expiran por rondas,
    // las retira quien las cure (tick no descuenta: ConditionEngine).
    // defaultRounds queda con su valor por defecto en UNTIL_CURED: solo
    // mantiene la condición viva.
    const JsonValue& dur = e["duration"];
    std::string durStr = dur.asString("");
    if (durStr.empty() && dur.isNumber()) {
        durStr = std::to_string(dur.asInt(0)) + "_turns";
    }
    if (durStr == "hasta_curarse" || durStr == "hasta_curar") {
        c.durationMode = ConditionDefinition::Duration::UNTIL_CURED;
    } else {
        c.durationMode = ConditionDefinition::Duration::ROUNDS;
        // "1_turn"/"1_turno"/"3_turns"/"2 turnos" -> las rondas que dice.
        int rounds = 0;
        for (char ch : durStr) {
            if (ch >= '0' && ch <= '9') rounds = rounds * 10 + (ch - '0');
            else if (rounds > 0) break;
        }
        if (rounds > 0) c.defaultRounds = rounds;
    }

    // category ("negativa"/"neutro"/"positiva", 16/16 cartas).
    const std::string category = tolower_inline(e["category"].asString(""));
    c.isNegative = (category != "neutro" && category != "positiva" && category != "positive");

    // effects[].mechanicHook — el DSL de la carta. Se conservan TODOS
    // crudos; se traducen solo los unívocos:
    //   dano_por_turno:N        -> damagePerRound (GDD: sangrado)
    //   velocidad_mitad         -> halfSpeed (GDD: ralentizado)
    //   desventaja:ataque[_y_habilidad] -> grantDisadvantageToCaster
    //   bloquea:vision          -> grantAdvantageToEnemies (GDD: cegado)
    //   incapacitado            -> preventAction (GDD: inconsciente)
    //   velocidad_0             -> speedZero (D-11; agarrado/anclado/
    //                              paralizado; más fuerte que halfSpeed)
    const JsonValue& effects = e["effects"];
    if (effects.isArray()) {
        for (std::size_t i = 0; i < effects.size(); ++i) {
            const std::string hook = effects[i]["mechanicHook"].asString("");
            if (hook.empty()) continue;
            c.mechanicHooks.push_back(hook);
            if (hook.compare(0, 15, "dano_por_turno:") == 0) {
                c.damagePerRound += std::atoi(hook.c_str() + 15);
            } else if (hook == "velocidad_mitad") {
                c.halfSpeed = true;
            } else if (hook == "desventaja:ataque" ||
                       hook == "desventaja:ataque_y_habilidad") {
                c.grantDisadvantageToCaster = true;
            } else if (hook == "bloquea:vision") {
                c.grantAdvantageToEnemies = true;
            } else if (hook == "incapacitado") {
                c.preventAction = true;
            } else if (hook == "velocidad_0") {
                c.speedZero = true;
            }
        }
    }
    return c;
}

// D-01 — Campos que from_json lee. Tras D-04 son los del asset (DEC-02);
// los nombres del cargador antiguo (stacking/defaultRounds/damagePerRound/
// caMod...) ya no se aceptan. id/name obligatorios como en los 20 cargadores.
template<>
inline const std::vector<CatalogField>& CatalogLoader<ConditionDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"stackable"}, false},
        {{"duration"}, false},
        {{"category"}, false},
        {{"effects"}, false},
    };
    return campos;
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
    // D-05 — `compatibilitySymbols` (13 cartas) es el mismo campo con otra
    // mayúscula: se fusiona sin duplicar. `linkedSkill` (33 cartas) es el
    // id de la carta de ataque, no `linkedSkillId`.
    for (const auto& s : copy_json_string_vec(e["compatibilitySymbols"])) {
        bool dup = false;
        for (const auto& t : eq.compatibleSymbols) {
            if (t == s) { dup = true; break; }
        }
        if (!dup) eq.compatibleSymbols.push_back(s);
    }
    eq.grantedTags = copy_json_string_vec(e["grantedTags"]);
    eq.statBonuses = statarr_json(e, "statBonuses", "stat_bonuses");
    eq.requiredStats = statarr_json(e, "requiredStats", "required_stats");
    eq.requiredLevel = e["requiredLevel"].asInt(0);  // NIVELES-01
    eq.statPenalties = statarr_json(e, "penalties");
    eq.caBonus = e["caBonus"].asInt(e["ca_bonus"].asInt(0));
    eq.maxHealthBonus = e["maxHealthBonus"].asInt(e["max_health_bonus"].asInt(0));
    eq.physicalSaveBonus = e["physicalSaveBonus"].asInt(e["physical_save_bonus"].asInt(0));
    eq.willSaveBonus     = e["willSaveBonus"].asInt(e["will_save_bonus"].asInt(0));
    eq.spellSaveBonus    = e["spellSaveBonus"].asInt(e["spell_save_bonus"].asInt(0));
    eq.damageBonus = e["damageBonus"].asInt(e["damage_bonus"].asInt(0));
    // D-05 — El nombre canonico del asset es `linkedSkill` (31 cartas con
    // ataque asociado). linkedSkillId/linked_skill_id son aliases legacy.
    eq.linkedSkillId = e["linkedSkill"].asString(e["linkedSkillId"].asString(e["linked_skill_id"].asString("")));
    // twoHanded: boolean explicito (si se usa en el futuro en vez de slot)
    eq.twoHanded = e["twoHanded"].asBool(e["two_handed"].asBool(false));
    // D-05 — slot == TwoHanded / OffHand implicito (por si el JSON lo trae
    // directamente como string en vez de booleano separado).
    if (!eq.twoHanded) {
        const std::string s = eq.slot;
        if (s == "TwoHanded" || s == "two_handed" || s == "TWOHANDED") eq.twoHanded = true;
    }
    return eq;
}

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<EquipmentDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"flavorText", "flavor_text"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"price"}, false},
        {{"slot"}, false},
        {{"weightCategory", "weight_category", "weight"}, false},
        {{"compatibleSymbols", "compatibilitySymbols"}, false},
        {{"grantedTags"}, false},
        {{"statBonuses", "stat_bonuses"}, false},
        {{"requiredStats", "required_stats"}, false},
        {{"requiredLevel"}, false},
        {{"penalties"}, false},
        {{"caBonus", "ca_bonus"}, false},
        {{"maxHealthBonus", "max_health_bonus"}, false},
        {{"physicalSaveBonus", "physical_save_bonus"}, false},
        {{"willSaveBonus", "will_save_bonus"}, false},
        {{"spellSaveBonus", "spell_save_bonus"}, false},
        {{"damageBonus", "damage_bonus"}, false},
        {{"linkedSkill", "linkedSkillId", "linked_skill_id"}, false},
        {{"twoHanded", "two_handed"}, false},
        {{"defenseBonuses"}, false},
    };
    return campos;
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
    c.flavorText = e["flavorText"].asString(e["flavor_text"].asString(""));
    c.tier = e["tier"].asInt(0);
    c.rarity = rarity_from_json(e["rarity"], ItemRarity::Common);
    c.price = e["price"].asInt(0);
    // D-05 — Lo que la carta trae (31/31): usos y tipo de acción.
    // `effect` es solo {description}: prosa, no ejecutable.
    c.uses = e["uses"].asInt(1);
    c.actionType = e["actionType"].asString(e["action_type"].asString(""));
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

// D-01 — Campos que from_json lee, derivados del propio cuerpo (claves
// e["..."] y cadenas de defaults). id/name son los unicos obligatorios:
// estan en las 2650 entradas de los 21 assets publicados (07/09/2026). El resto
// pasa a obligatorio cuando D-02..D-09 migren su asset.
template<>
inline const std::vector<CatalogField>& CatalogLoader<ConsumableDefinition>::expected_fields() {
    static const std::vector<CatalogField> campos = {
        {{"id"}, true},
        {{"name"}, true},
        {{"description"}, false},
        {{"flavorText", "flavor_text"}, false},
        {{"tier"}, false},
        {{"rarity"}, false},
        {{"price"}, false},
        {{"uses"}, false},
        {{"actionType", "action_type"}, false},
        {{"healHp", "heal_hp"}, false},
        {{"restorePileCards", "restore_pile_cards"}, false},
        {{"clearOneCondition", "clear_one_condition"}, false},
        {{"grantConditionId", "grant_condition_id"}, false},
        {{"grantConditionRounds", "grant_condition_rounds"}, false},
        {{"bonusGold", "bonus_gold"}, false},
        {{"grantPassiveId", "grant_passive_id"}, false},
        {{"grantPassiveRounds", "grant_passive_rounds"}, false},
    };
    return campos;
}

// -----------------------------------------------------------------
// MonsterDefinition
// -----------------------------------------------------------------

}  // namespace Catalogs
}  // namespace RPG
