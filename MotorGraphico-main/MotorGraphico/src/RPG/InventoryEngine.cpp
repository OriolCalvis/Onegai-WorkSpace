// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   D-05  (fase D, 3 j) - Objectes, equipament i consumibles
//   G-12  (fase G, 5 j) - Inventari, equip i fitxa utilitzables
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "RPG/InventoryEngine.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

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
    // PART-02: los brazos, el collar y el cinturon; los anillos van al de la mano principal (equip() pasa al
    // de la otra si ese esta ocupado).
    if (s.find("arms") != std::string::npos || s.find("brazo") != std::string::npos) return EquipSlot::Arms;
    if (s.find("neck") != std::string::npos || s.find("cuello") != std::string::npos ||
        s.find("collar") != std::string::npos || s.find("amuleto") != std::string::npos) {
        return EquipSlot::Neck;
    }
    if (s.find("belt") != std::string::npos || s.find("cintur") != std::string::npos) return EquipSlot::Belt;
    if (s.find("ring2") != std::string::npos) return EquipSlot::Ring2;
    if (s.find("accessor") != std::string::npos || s.find("anillo") != std::string::npos ||
        s.find("ring") != std::string::npos) {
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
        case EquipSlot::Arms:      return "Arms";
        case EquipSlot::Neck:      return "Neck";
        case EquipSlot::Belt:      return "Belt";
        case EquipSlot::Ring2:     return "Ring2";
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
    // PART-02 — Las ranuras se abren por tier: el collar, el cinturon y los anillos, desde el 2 (una
    // ficha sin tier, a 0, cuenta como de Tier 1).
    {
        bool dosManos = false;
        const EquipSlot ranura = resolve_slot(eq.slot, dosManos);
        if (!ranuraAbierta(ranura, sheet.tier)) {
            std::ostringstream os;
            os << "los anillos, el collar y el cinturon se abren en el Tier " << tierDeRanura(ranura)
               << " (estas en el " << sheet.tier << ")";
            return Result<bool>::Error(os.str());
        }
    }
    if (!check_rarity(tierRules, sheet.tier, eq.rarity)) {
        auto t = tierRules.at(sheet.tier);
        std::ostringstream os;
        os << "Raridad excedida: " << static_cast<int>(eq.rarity)
           << " > tier " << sheet.tier << " max=" << rarityRank(t.maxEquipRarity);
        return Result<bool>::Error(os.str());
    }
    // NIVELES-01 — El nivel del heroe para llevarlo puesto (en la mochila se
    // puede tener antes: un botin raro espera a que llegues).
    if (eq.requiredLevel > 1 && sheet.nivel < eq.requiredLevel) {
        std::ostringstream os;
        os << "necesitas nivel " << eq.requiredLevel << " (tienes el " << sheet.nivel << ")";
        return Result<bool>::Error(os.str());
    }
    // D-05 — Requisitos de la carta (`requiredStats`, orden CON/DES/INT/CAR):
    // el total actual del PJ debe llegar a cada minimo. Sin requisito pasa.
    {
        static const char* nombres[4] = {"CON", "DES", "INT", "CAR"};
        static const Stat stats[4] = {Stat::CON, Stat::DES, Stat::INT, Stat::CAR};
        for (int i = 0; i < 4; ++i) {
            if (eq.requiredStats[i] > 0 && sheet.stat(stats[i]) < eq.requiredStats[i]) {
                std::ostringstream os;
                os << "Requisito sin cumplir: " << nombres[i] << " " << sheet.stat(stats[i])
                   << " < " << eq.requiredStats[i] << " para " << eq.id;
                return Result<bool>::Error(os.str());
            }
        }
    }
    return Result<bool>::Ok(true);
}

// ---------------------------------------------------------------------------
// Aplicación/resta de bonuses
// ---------------------------------------------------------------------------
// D-05 — Escut real: el slot ho diu (shield/escudo, insensible a
// majuscules). Abans qualsevol objecte amb slot OffHand anava a
// equipShieldBonus: una daga a la ma esquerra comptava com a escut. Ara
// nomes l'escut hi va; la resta (inclosa la ma esquerra) va a armor.
static bool is_shield_item(const EquipmentDefinition& eq) {
    const std::string s = to_lower(eq.slot);
    return s.find("hield") != std::string::npos || s.find("escudo") != std::string::npos;
}

void InventoryEngine::apply_bonus(CharacterSheet& sheet, const EquipmentDefinition& eq, bool add) {
    const int sign = add ? 1 : -1;
    for (int i = 0; i < 4; ++i) {
        sheet.equipBonuses[i] += sign * eq.statBonuses[i];
        // D-05 — Penalizadores de la carta (`penalties`): magnitudes a restar.
        sheet.equipBonuses[i] -= sign * eq.statPenalties[i];
    }
    if (is_shield_item(eq)) {
        sheet.equipShieldBonus += sign * eq.caBonus;
    } else {
        sheet.equipArmorBonus  += sign * eq.caBonus;
    }
    sheet.physicalSaveBonus += sign * eq.physicalSaveBonus;
    sheet.mentalSaveBonus   += sign * eq.willSaveBonus;
    sheet.spellSaveExtra    += sign * eq.spellSaveBonus;
    // D-05 — maxHealthBonus ja no es descarta: s'acumula com els altres.
    // Consumir-lo al healthCap() es G-12 (canvia numeros de combat).
    sheet.equipMaxHealthBonus += sign * eq.maxHealthBonus;
}

void InventoryEngine::recalc_equipment_bonuses(CharacterSheet& sheet, const Catalogs::EquipmentCatalog& cat, RecalcMode mode) {
    if (mode == RecalcMode::REBUILD) {
        // Poner a cero
        sheet.equipBonuses = {0,0,0,0};
        sheet.equipArmorBonus = 0;
        sheet.equipShieldBonus = 0;
        sheet.equipMaxHealthBonus = 0;
        sheet.physicalSaveBonus = 0;
        sheet.mentalSaveBonus = 0;
        sheet.spellSaveExtra = 0;
        // D-05: una arma a dues mans ocupa MainHand+OffHand amb el MATEIX id:
        // s'aplica un sol cop (abans es comptava dues vegades: el REBUILD
        // donava el doble que l'equip incremental). En general, cap id es
        // compta dues vegades.
        std::vector<std::string> seen;
        for (const auto& id : sheet.equippedIds) {
            if (id.empty()) continue;
            bool dup = false;
            for (const auto& s : seen) {
                if (s == id) { dup = true; break; }
            }
            if (dup) continue;
            seen.push_back(id);
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
    // PART-02 — Un anillo va a la mano principal; si ya lleva uno y la otra esta libre, a la otra.
    if (primary_slot == EquipSlot::Accessory && !sheet.equippedIds[static_cast<size_t>(EquipSlot::Accessory)].empty() &&
        sheet.equippedIds[static_cast<size_t>(EquipSlot::Ring2)].empty()) {
        primary_slot = EquipSlot::Ring2;
    }

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

    // G-12 — La ficha se refresca al instante. equip() acaba de cambiar
    // equipArmorBonus/equipMaxHealthBonus, y cachedDefenses/
    // cachedHealthCap (lo que leen Player/Enemy y healthCap()) se quedarian
    // obsoletos si el refresco quedara en manos de un caller que hoy no
    // existe (la cabecera dice "GameSession llama a recompute_derived",
    // pero GameSession todavia no lo hace y el equip vive a este nivel).
    // Aqui tierRules/clazz estan a mano, asi que recomputar es gratis y el
    // bonus del equipo se ve (hasta el tope del tier) nada mas equipar la
    // pieza.
    sheet.recompute_derived(tierRules, &clazz);

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
