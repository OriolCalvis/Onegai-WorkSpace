#include "RPG/InventoryEngine.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace RPG {

// ---------------------------------------------------------------------------
// Utilidades internas
// ---------------------------------------------------------------------------
static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

int InventoryEngine::weight_rank(const std::string& w) {
    auto s = to_lower(w);
    if (s.find("light") != std::string::npos || s.find("liger") != std::string::npos) return 1;
    if (s.find("medium") != std::string::npos || s.find("medi") != std::string::npos) return 2;
    if (s.find("heavy") != std::string::npos || s.find("pesad") != std::string::npos) return 3;
    return 0; // indefinido → sin restriccion
}

bool InventoryEngine::is_weapon_slot(const std::string& slot_name) {
    auto s = to_lower(slot_name);
    return (s.find("mainhand") != std::string::npos ||
            s.find("offhand")  != std::string::npos ||
            s.find("twohand")  != std::string::npos ||
            s.find("mano")     != std::string::npos);
}

EquipSlot InventoryEngine::resolve_slot(const std::string& slot_name, bool& out_two_handed) {
    auto s = to_lower(slot_name);
    out_two_handed = false;
    if (s.find("head")    != std::string::npos || s.find("cabez") != std::string::npos) return EquipSlot::Head;
    if (s.find("torso")   != std::string::npos || s.find("torso") != std::string::npos) return EquipSlot::Torso;
    if (s.find("legs")    != std::string::npos || s.find("piern") != std::string::npos) return EquipSlot::Legs;
    if (s.find("feet")    != std::string::npos || s.find("pie")   != std::string::npos) return EquipSlot::Feet;
    if (s.find("twohand") != std::string::npos || s.find("dosmanos") != std::string::npos) {
        out_two_handed = true;
        return EquipSlot::MainHand;
    }
    if (s.find("offhand") != std::string::npos || s.find("mano_izq") != std::string::npos || s.find("secund") != std::string::npos) {
        return EquipSlot::OffHand;
    }
    if (s.find("mainhand") != std::string::npos || s.find("mano_pri") != std::string::npos || s.find("arma_principal") != std::string::npos) {
        return EquipSlot::MainHand;
    }
    if (s.find("accessor") != std::string::npos || s.find("anillo") != std::string::npos || s.find("amuleto") != std::string::npos) {
        return EquipSlot::Accessory;
    }
    // Por defecto MainHand (consecuencia de ser arma sin categoría)
    out_two_handed = false;
    return EquipSlot::MainHand;
}

std::string InventoryEngine::slot_name(EquipSlot s) {
    switch (s) {
        case EquipSlot::Head:      return "Head";
        case EquipSlot::Torso:     return "Torso";
        case EquipSlot::Legs:      return "Legs";
        case EquipSlot::Feet:      return "Feet";
        case EquipSlot::MainHand:  return "MainHand";
        case EquipSlot::OffHand:   return "OffHand";
        case EquipSlot::Accessory: return "Accessory";
        default:                   return "Unknown";
    }
}

// ---------------------------------------------------------------------------
// Validadores
// ---------------------------------------------------------------------------
bool InventoryEngine::check_rarity(const TierRules& tierRules, int tier, ItemRarity rarity) {
    auto t = tierRules.at(tier);
    const int rank_cur = static_cast<int>(rarity);
    const int rank_max = rarityRank(t.maxEquipRarity);
    if (rank_max < 0) return true; // si el tier no define → sin limite
    return rank_cur <= rank_max;
}

bool InventoryEngine::check_weight(const ClassDefinition& clazz, const EquipmentDefinition& eq) {
    const std::string& class_max = is_weapon_slot(eq.slot) ? clazz.maxWeaponWeight : clazz.maxArmorWeight;
    const int eq_rk = weight_rank(eq.weightCategory);
    const int cl_rk = weight_rank(class_max);
    if (eq_rk == 0 || cl_rk == 0) return true; // si alguno es indeterminado → ok
    return eq_rk <= cl_rk;
}

bool InventoryEngine::check_symbols(const ClassDefinition& clazz, const EquipmentDefinition& eq) {
    if (eq.compatibleSymbols.empty()) return true;
    const auto& class_symbols = is_weapon_slot(eq.slot) ? clazz.compatibilityWeapons : clazz.compatibilityArmor;
    if (class_symbols.empty()) return true;
    for (const auto& s : eq.compatibleSymbols) {
        for (const auto& cs : class_symbols) {
            if (s == cs) return true;
        }
    }
    return false;
}

Result<bool> InventoryEngine::can_equip(const CharacterSheet& sheet,
                                        const EquipmentDefinition& eq,
                                        const ClassDefinition& clazz,
                                        const TierRules& tierRules) {
    (void)sheet; // por ahora no hay restricciones dependientes de estado (símbolos son clase)
    if (!check_symbols(clazz, eq)) {
        std::ostringstream os;
        os << "Simbolos incompatibles: la clase " << clazz.id
           << " no puede portar " << eq.id;
        return Result<bool>::Error(os.str());
    }
    if (!check_weight(clazz, eq)) {
        std::ostringstream os;
        os << "Peso incompatible: " << eq.weightCategory << " > "
           << (is_weapon_slot(eq.slot) ? clazz.maxWeaponWeight : clazz.maxArmorWeight);
        return Result<bool>::Error(os.str());
    }
    if (!check_rarity(tierRules, sheet.tier, eq.rarity)) {
        auto t = tierRules.at(sheet.tier);
        std::ostringstream os;
        os << "Raridad excedida: " << static_cast<int>(eq.rarity)
           << " > tier " << sheet.tier << " max=" << rarityRank(t.maxEquipRarity);
        return Result<bool>::Error(os.str());
    }
    return Result<bool>::Ok(true);
}

// ---------------------------------------------------------------------------
// Aplicación/resta de bonuses
// ---------------------------------------------------------------------------
void InventoryEngine::apply_bonus(CharacterSheet& sheet, const EquipmentDefinition& eq, bool add) {
    const int sign = add ? 1 : -1;
    for (int i = 0; i < 4; ++i) {
        sheet.equipBonuses[i] += sign * eq.statBonuses[i];
    }
    if (eq.slot.find("hield") != std::string::npos || eq.slot.find("escudo") != std::string::npos || eq.slot.find("OffHand") != std::string::npos) {
        sheet.equipShieldBonus += sign * eq.caBonus;
    } else {
        sheet.equipArmorBonus  += sign * eq.caBonus;
    }
    sheet.physicalSaveBonus += sign * eq.physicalSaveBonus;
    sheet.mentalSaveBonus   += sign * eq.willSaveBonus;
    sheet.spellSaveExtra    += sign * eq.spellSaveBonus;
    // maxHealthBonus lo aplicamos en recalc_equipment_bonuses globalmente, porque no hay campo
    // directo; para P0 no llevamos tracking aparte de suma/resta total.
    (void)eq.maxHealthBonus;
}

void InventoryEngine::recalc_equipment_bonuses(CharacterSheet& sheet, const Catalogs::EquipmentCatalog& cat, RecalcMode mode) {
    if (mode == RecalcMode::REBUILD) {
        // Poner a cero
        sheet.equipBonuses = {0,0,0,0};
        sheet.equipArmorBonus = 0;
        sheet.equipShieldBonus = 0;
        sheet.physicalSaveBonus = 0;
        sheet.mentalSaveBonus = 0;
        sheet.spellSaveExtra = 0;
        for (const auto& id : sheet.equippedIds) {
            if (id.empty()) continue;
            const EquipmentDefinition* r = cat.find(id);
            if (!r) continue;
            apply_bonus(sheet, *r, true);
        }
    }
    // APPLY_EQUIP y UNAPPLY_EQUIP se hacen in-place por caller (no necesitamos iterar otra vez)
}

// ---------------------------------------------------------------------------
// equip / unequip
// ---------------------------------------------------------------------------
Result<bool> InventoryEngine::equip(CharacterSheet& sheet,
                                    const EquipmentDefinition& eq,
                                    const ClassDefinition& clazz,
                                    const TierRules& tierRules,
                                    const Catalogs::EquipmentCatalog& cat) {
    auto pre = can_equip(sheet, eq, clazz, tierRules);
    if (!pre.isOk()) return pre;

    bool eq_two = eq.twoHanded;
    bool tmp = false;
    auto primary_slot = resolve_slot(eq.slot, tmp);
    if (tmp) eq_two = true;
    if (eq_two) primary_slot = EquipSlot::MainHand;

    // Verifica ocupación two-handed
    if (eq_two) {
        const auto& main_s = sheet.equippedIds[static_cast<size_t>(EquipSlot::MainHand)];
        const auto& off_s  = sheet.equippedIds[static_cast<size_t>(EquipSlot::OffHand)];
        // Si hay algo en MainHand y no es two-handed → desequipar
        if (!main_s.empty()) {
            auto r = unequip(sheet, EquipSlot::MainHand, cat);
            if (!r.isOk()) return r;
        }
        if (!off_s.empty()) {
            auto r = unequip(sheet, EquipSlot::OffHand, cat);
            if (!r.isOk()) return r;
        }
    } else {
        // One-handed: si MainHand es two-handed → impedir OffHand si el nuevo va a OffHand
        const auto& main_s = sheet.equippedIds[static_cast<size_t>(EquipSlot::MainHand)];
        if (!main_s.empty()) {
            const EquipmentDefinition* m = cat.find(main_s);
            if (m && m->twoHanded) {
                // MainHand ocupa las dos manos. Si intentamos poner algo en OffHand → error
                if (primary_slot == EquipSlot::OffHand) {
                    return Result<bool>::Error("MainHand lleva arma a dos manos; desequipa primero antes de portar en OffHand");
                }
                if (primary_slot == EquipSlot::MainHand) {
                    // Desequipa mainhand primero (desequipa también offhand si fuera two-handed en unequip)
                    auto r = unequip(sheet, EquipSlot::MainHand, cat);
                    if (!r.isOk()) return r;
                }
            }
        }
        // Si ya algo en el slot objetivo → desequipar
        const size_t idx = static_cast<size_t>(primary_slot);
        if (idx < sheet.equippedIds.size() && !sheet.equippedIds[idx].empty()) {
            auto r = unequip(sheet, primary_slot, cat);
            if (!r.isOk()) return r;
        }
    }

    // Poner el id
    const size_t mh = static_cast<size_t>(EquipSlot::MainHand);
    const size_t oh = static_cast<size_t>(EquipSlot::OffHand);
    const size_t ps = static_cast<size_t>(primary_slot);
    if (eq_two) {
        sheet.equippedIds[mh] = eq.id;
        sheet.equippedIds[oh] = eq.id;
    } else {
        sheet.equippedIds[ps] = eq.id;
    }

    // También quitamos del inventario (está en inventoryEquipment; si existe -1 stack y borrar si 0)
    if (eq_two) {
        (void)remove_from_inventory(sheet, eq.id, 1, false);
    } else {
        (void)remove_from_inventory(sheet, eq.id, 1, false);
    }

    // Suma bonuses incremental
    apply_bonus(sheet, eq, true);
    return Result<bool>::Ok(true);
}

Result<bool> InventoryEngine::unequip(CharacterSheet& sheet,
                                      EquipSlot slot,
                                      const Catalogs::EquipmentCatalog& cat,
                                      std::string* out_unequipped_id) {
    const size_t idx = static_cast<size_t>(slot);
    if (idx >= sheet.equippedIds.size()) {
        return Result<bool>::Error("Slot fuera de rango");
    }
    const std::string id = sheet.equippedIds[idx];
    if (id.empty()) return Result<bool>::Ok(false);

    // Miramos si es two-handed → si queremos desequipar MainHand o OffHand, vacíamos ambos
    const EquipmentDefinition* fd = cat.find(id);
    const bool is_two = fd && (fd->twoHanded ||
                               fd->slot.find("TwoHanded") != std::string::npos ||
                               fd->slot.find("twohand") != std::string::npos ||
                               fd->slot.find("dos_manos") != std::string::npos);

    const size_t mh = static_cast<size_t>(EquipSlot::MainHand);
    const size_t oh = static_cast<size_t>(EquipSlot::OffHand);
    if (is_two) {
        // Resta bonuses 1 sola vez (id duplicado en mh+oh)
        if (fd) apply_bonus(sheet, *fd, false);
        sheet.equippedIds[mh].clear();
        sheet.equippedIds[oh].clear();
    } else {
        if (fd) apply_bonus(sheet, *fd, false);
        sheet.equippedIds[idx].clear();
    }

    // Mueve al inventario
    add_to_inventory(sheet, id, 1, false);
    if (out_unequipped_id) *out_unequipped_id = id;
    return Result<bool>::Ok(true);
}

// ---------------------------------------------------------------------------
// Inventario (stacks)
// ---------------------------------------------------------------------------
void InventoryEngine::add_to_inventory(CharacterSheet& sheet,
                                       const std::string& equipment_or_consumable_id,
                                       int count,
                                       bool is_consumable) {
    auto& vec = is_consumable ? sheet.inventoryConsumables : sheet.inventoryEquipment;
    for (auto& [k, n] : vec) {
        if (k == equipment_or_consumable_id) {
            n += count;
            return;
        }
    }
    vec.emplace_back(equipment_or_consumable_id, count);
}

Result<bool> InventoryEngine::remove_from_inventory(CharacterSheet& sheet,
                                                    const std::string& equipment_or_consumable_id,
                                                    int count,
                                                    bool is_consumable) {
    auto& vec = is_consumable ? sheet.inventoryConsumables : sheet.inventoryEquipment;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if (it->first == equipment_or_consumable_id) {
            if (it->second < count) {
                std::ostringstream os;
                os << "No hay suficientes stacks de " << equipment_or_consumable_id
                   << ": " << it->second << " < " << count;
                return Result<bool>::Error(os.str());
            }
            it->second -= count;
            if (it->second <= 0) vec.erase(it);
            return Result<bool>::Ok(true);
        }
    }
    return Result<bool>::Error(std::string("Id no encontrado en inventario: ") + equipment_or_consumable_id);
}

} // namespace RPG
