#pragma once

#include <string>

#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/TierRules.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "Core/Errors/Result.h"

namespace RPG {

// ============================================================
// InventoryEngine (GAMEMACHINE_P0 §4)
//
// Responsabilidades:
//   - equip / unequip en CharacterSheet.equippedIds[]
//   - límites: weightCategory ≤ class.maxArmorWeight, rarityRank ≤ tier.maxEquipRarity
//   - symbols compatibles (class.compatibilityWeapons ∩ equipment.compatibleSymbols)
//   - recalcula equipBonuses[], equipArmorBonus, equipShieldBonus, maxHealthBonus equipo
//
// Diseño stateless: TODAS las operaciones toman refs. No hay estado interno.
// ============================================================
class InventoryEngine {
public:
    // Equipa un EquipmentDefinition* (de EquipmentCatalog) en el slot que indique
    // equipment.slot, con estas reglas:
    //   - Si slot == TwoHanded → ocupa MainHand + OffHand. Si OffHand estaba ocupado
    //     se desequipa automáticamente.
    //   - Si slot == OffHand pero el MainHand es twoHanded → Error (tienes que desequipar primero).
    //   - Si hay algo ya en el slot objetivo → se desequipa primero (y se añade a inventario).
    //   - Si no pasa check_rarity / check_weight / check_symbols → Error, no se toca nada.
    //
    // Retorna:
    //   Ok(true) si el equipamiento fue exitoso.
    //   Error("mensaje") si no pasó alguna regla.
    static Result<bool> equip(CharacterSheet& sheet,
                              const EquipmentDefinition& eq,
                              const ClassDefinition& clazz,
                              const TierRules& tierRules,
                              const Catalogs::EquipmentCatalog& cat);

    // Desequipa lo que haya en `slot`. Si slot == MainHand y MainHand es two-handed,
    // también desequipa OffHand (y viceversa por coherencia). Añade el id desequipado
    // a sheet.inventoryEquipment con stack +1. out_unequipped_id se rellena si no es nullptr.
    // Retorna Ok(true) si se desequipó algo; Ok(false) si el slot ya estaba vacío.
    static Result<bool> unequip(CharacterSheet& sheet,
                                EquipSlot slot,
                                const Catalogs::EquipmentCatalog& cat,
                                std::string* out_unequipped_id = nullptr);

    // Añade a inventario sin equipar: stack count += cantidad (si el id ya estaba)
    // o bien push_back con el count dado.
    static void add_to_inventory(CharacterSheet& sheet,
                                 const std::string& equipment_or_consumable_id,
                                 int count = 1,
                                 bool is_consumable = false);
    // Quita X stacks; retorna Ok(true) si habia suficientes. Si count pasa a 0
    // se elimina la entrada del vector.
    static Result<bool> remove_from_inventory(CharacterSheet& sheet,
                                              const std::string& equipment_or_consumable_id,
                                              int count = 1,
                                              bool is_consumable = false);

    // ====== Validadores (usados internamente; públicos para UI/pre-check) ======

    // rarity ≤ TierRules.at(tier).maxEquipRarity (por rank)
    static bool check_rarity(const TierRules& tierRules, int tier, ItemRarity rarity);

    // equipment.weightCategory (light/medium/heavy) ≤ class.maxArmorWeight (o
    // .maxWeaponWeight según el slot: si es arma usar maxWeaponWeight else maxArmorWeight)
    // Cadenas comparadas con prefijos insensibles: light=1, medium=2, heavy=3.
    static bool check_weight(const ClassDefinition& clazz,
                             const EquipmentDefinition& eq);

    // Si el eq no define símbolos → siempre compatible. Si clazz no define → compatible.
    // Si ambos tienen al menos 1 → debe haber intersección.
    static bool check_symbols(const ClassDefinition& clazz,
                              const EquipmentDefinition& eq);

    // Ejecuta TODOS los checks en orden: symbols → weight → rarity → twoHanded collisions.
    static Result<bool> can_equip(const CharacterSheet& sheet,
                                  const EquipmentDefinition& eq,
                                  const ClassDefinition& clazz,
                                  const TierRules& tierRules);

    // Después de equipar/desequipal: suma/resta statBonuses, armor/shield/maxHealth bonuses,
    // save bonuses de equipment a sheet. Se llama desde equip() y unequip() automáticamente.
    // También es público si el usuario quiere regenerar de cero (con mode=REBUILD).
    enum class RecalcMode { APPLY_EQUIP, UNAPPLY_EQUIP, REBUILD };
    static void recalc_equipment_bonuses(CharacterSheet& sheet,
                                         const Catalogs::EquipmentCatalog& cat,
                                         RecalcMode mode = RecalcMode::REBUILD);

    // Resuelve un string "Head" / "Torso" / "Legs" / "Feet" / "MainHand" /
    // "OffHand" / "Accessory" / "TwoHanded" a (EquipSlot::MainHand, two_handed=true) etc.
    static EquipSlot resolve_slot(const std::string& slot_name, bool& out_two_handed);
    static std::string slot_name(EquipSlot s);

private:
    static int weight_rank(const std::string& w);
    static bool is_weapon_slot(const std::string& slot_name);
    static void apply_bonus(CharacterSheet& sheet,
                            const EquipmentDefinition& eq,
                            bool add); // true = suma, false = resta
};

} // namespace RPG
