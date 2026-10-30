#include "Game/InventoryMenuState.h"

#include <algorithm>
#include <map>
#include <string>
#include <utility>

#include "Level/ObjectCatalog.h"

namespace Game {

namespace {

const std::pair<RPG::EquipSlot, const char*> kRanuras[] = {
    {RPG::EquipSlot::Head, "Cabeza"},
    {RPG::EquipSlot::Torso, "Torso"},
    {RPG::EquipSlot::Legs, "Piernas"},
    {RPG::EquipSlot::Feet, "Pies"},
    {RPG::EquipSlot::MainHand, "Mano principal"},
    {RPG::EquipSlot::OffHand, "Otra mano"},
    {RPG::EquipSlot::Accessory, "Anillo"},
    // PART-02: el Tier 1 abre los brazos; el Tier 2, el collar, el cinturon y el otro anillo
    {RPG::EquipSlot::Arms, "Brazos"},
    {RPG::EquipSlot::Neck, "Collar"},
    {RPG::EquipSlot::Belt, "Cinturón"},
    {RPG::EquipSlot::Ring2, "Otro anillo"},
};

ItemAction Accion(const std::string& motivo) {
    ItemAction a;
    a.allowed = motivo.empty();
    a.reason = motivo;
    return a;
}

}  // namespace

const char* ItemCategoryName(ItemCategory category) {
    switch (category) {
        case ItemCategory::Consumable: return "Consumibles";
        case ItemCategory::Equipment:  return "Equipo";
        case ItemCategory::Quest:      return "Objetos clave";
        case ItemCategory::Other:      return "Otros";
    }
    return "Otros";
}

InventoryMenuState::InventoryMenuState(GameSession& session) : m_session(session) {
    refresh();
}

std::string nombreObjeto(const GameSession& session, const std::string& objectId) {
    const ObjectCatalog* catalogo = session.objectCatalog();
    const ObjectDefinition* def = catalogo != nullptr ? catalogo->find(objectId) : nullptr;
    const RPG::EquipmentDefinition* eq = session.equipmentFor(objectId);
    return def != nullptr ? def->name : (eq != nullptr ? eq->name : objectId);
}

ItemCategory clasificaObjeto(const GameSession& session, const std::string& objectId) {
    if (session.equipmentFor(objectId) != nullptr) return ItemCategory::Equipment;
    const ObjectCatalog* catalogo = session.objectCatalog();
    const ObjectDefinition* def = catalogo != nullptr ? catalogo->find(objectId) : nullptr;
    if (def == nullptr) return ItemCategory::Other;
    if (def->category == ObjectCategory::Pickup && def->pickup.effect != PickupEffect::None) {
        return ItemCategory::Consumable;
    }
    // Sin efecto y sin precio: no se gasta ni se vende. Es una llave, una
    // carta, un sello: lo que una mision pide tener.
    if (def->pickup.price <= 0) return ItemCategory::Quest;
    return ItemCategory::Other;
}

std::string describeObjeto(const GameSession& session, const std::string& objectId, ItemCategory category) {
    if (const RPG::EquipmentDefinition* eq = session.equipmentFor(objectId)) {
        return eq->description;
    }
    const ObjectCatalog* catalogo = session.objectCatalog();
    const ObjectDefinition* def = catalogo != nullptr ? catalogo->find(objectId) : nullptr;
    if (def == nullptr) return "";
    if (category == ItemCategory::Consumable) {
        return def->pickup.effect == PickupEffect::Heal
                   ? "Cura " + std::to_string(def->pickup.power) + " PV."
                   : "Restaura " + std::to_string(def->pickup.power) + " PM.";
    }
    if (category == ItemCategory::Quest) return "Objeto clave: no se gasta ni se vende.";
    return "";
}

void InventoryMenuState::refresh() {
    // Una fila por id, con cantidad; orden por categoria y nombre para que
    // la lista no baile al usar o recoger algo.
    std::map<std::string, int> cuantos;
    if (const std::vector<std::string>* mochila = m_session.inventory()) {
        for (const std::string& id : *mochila) ++cuantos[id];
    }
    m_all.clear();
    for (const auto& [id, n] : cuantos) {
        InventoryRow fila;
        fila.objectId = id;
        fila.name = nombreObjeto(m_session, id);
        fila.category = clasificaObjeto(m_session, id);
        fila.description = describeObjeto(m_session, id, fila.category);
        fila.quantity = n;
        if (fila.category == ItemCategory::Equipment) {
            fila.use = Accion(m_session.whyCannotUse(id));
            fila.equip = Accion(m_session.whyCannotEquip(id));
        } else {
            fila.use = Accion(m_session.whyCannotUse(id));
            fila.equip = Accion(fila.name + " no es una pieza de equipo");
        }
        m_all.push_back(std::move(fila));
    }
    std::stable_sort(m_all.begin(), m_all.end(), [](const InventoryRow& a, const InventoryRow& b) {
        if (a.category != b.category) return a.category < b.category;
        if (a.name != b.name) return a.name < b.name;
        return a.objectId < b.objectId;
    });

    m_slots.clear();
    const RPG::CharacterSheet* ficha = m_session.characterSheet();
    for (const auto& [ranura, etiqueta] : kRanuras) {
        EquipmentSlotRow fila;
        fila.slot = ranura;
        fila.label = etiqueta;
        const std::size_t i = static_cast<std::size_t>(ranura);
        if (ficha != nullptr && i < ficha->equippedIds.size()) fila.equipmentId = ficha->equippedIds[i];
        const RPG::Catalogs::EquipmentCatalog* cat = m_session.inventoryRules().equipment;
        const RPG::EquipmentDefinition* eq =
            (cat != nullptr && !fila.equipmentId.empty()) ? cat->find(fila.equipmentId) : nullptr;
        fila.name = fila.equipmentId.empty() ? "(vacío)" : (eq != nullptr ? eq->name : fila.equipmentId);
        // PART-02: la ranura que el tier aun no abre lo dice
        if (fila.equipmentId.empty() && ficha != nullptr && !RPG::ranuraAbierta(ranura, ficha->tier))
            fila.name = "(desde el Tier " + std::to_string(RPG::tierDeRanura(ranura)) + ")";
        fila.unequip = Accion(m_session.whyCannotUnequip(ranura));
        m_slots.push_back(std::move(fila));
    }
    rebuildVisible();
}

void InventoryMenuState::rebuildVisible() {
    // Donde estaba la seleccion en la lista ANTERIOR, para caer en la
    // siguiente si su fila desaparece.
    std::size_t posAnterior = 0;
    for (std::size_t i = 0; i < m_visible.size(); ++i) {
        if (m_visible[i].objectId == m_selected) posAnterior = i;
    }
    m_visible.clear();
    for (const InventoryRow& fila : m_all) {
        if (fila.category == m_category) m_visible.push_back(fila);
    }
    if (m_visible.empty()) {
        m_selected.clear();
        return;
    }
    if (selectedRow().has_value()) return;
    m_selected = m_visible[std::min(posAnterior, m_visible.size() - 1)].objectId;
}

void InventoryMenuState::setCategory(ItemCategory category) {
    if (category == m_category) return;
    m_category = category;
    m_selected.clear();
    cancelTarget();
    rebuildVisible();
}

std::size_t InventoryMenuState::countIn(ItemCategory category) const {
    return static_cast<std::size_t>(std::count_if(
        m_all.begin(), m_all.end(), [category](const InventoryRow& f) { return f.category == category; }));
}

bool InventoryMenuState::select(const std::string& objectId) {
    for (const InventoryRow& fila : m_visible) {
        if (fila.objectId == objectId) {
            m_selected = objectId;
            return true;
        }
    }
    return false;
}

void InventoryMenuState::moveSelection(int delta) {
    if (m_visible.empty() || delta == 0 || hasPendingTarget()) return;
    const long actual = static_cast<long>(selectedRow().value_or(0));
    const long destino = std::clamp(actual + delta, 0L, static_cast<long>(m_visible.size()) - 1);
    m_selected = m_visible[static_cast<std::size_t>(destino)].objectId;
}

std::optional<std::size_t> InventoryMenuState::selectedRow() const {
    for (std::size_t i = 0; i < m_visible.size(); ++i) {
        if (m_visible[i].objectId == m_selected) return i;
    }
    return std::nullopt;
}

const InventoryRow* InventoryMenuState::selectedEntry() const {
    const auto fila = selectedRow();
    return fila ? &m_visible[*fila] : nullptr;
}

Result<bool> InventoryMenuState::remember(Result<bool> result, const std::string& okText) {
    m_message = result.isOk() ? okText : result.errorMessage();
    refresh();
    return result;
}

Result<bool> InventoryMenuState::requestUse() {
    const InventoryRow* fila = selectedEntry();
    if (fila == nullptr) {
        m_message = "no hay nada elegido";
        return Result<bool>::Error(m_message);
    }
    if (!fila->use.allowed) {
        m_message = fila->use.reason;
        return Result<bool>::Error(m_message);
    }
    // Todo lo que se puede usar hoy (pociones, eteres) va a un personaje:
    // se elige a quien. Con un solo heroe la lista tiene una entrada, pero
    // el paso existe: es donde el jugador se arrepiente sin gastar nada, y
    // el que usara el grupo cuando lo haya.
    m_pendingItem = fila->objectId;
    m_targets.clear();
    const RPG::CharacterSheet* ficha = m_session.characterSheet();
    m_targets.push_back(ficha != nullptr && !ficha->displayName.empty() ? ficha->displayName : "Tú");
    m_message = "¿Sobre quién usar " + fila->name + "?";
    return Result<bool>::Ok(true);
}

Result<bool> InventoryMenuState::confirmTarget(std::size_t target) {
    if (!hasPendingTarget()) {
        m_message = "no hay nada pendiente de usar";
        return Result<bool>::Error(m_message);
    }
    if (target >= m_targets.size()) {
        m_message = "ese destinatario no existe";
        return Result<bool>::Error(m_message);
    }
    const std::string objeto = m_pendingItem;
    m_pendingItem.clear();
    m_targets.clear();
    return remember(m_session.useItem(objeto), "Hecho.");
}

void InventoryMenuState::cancelTarget() {
    if (!hasPendingTarget()) return;
    m_pendingItem.clear();
    m_targets.clear();
    m_message = "Cancelado.";
}

Result<bool> InventoryMenuState::equipSelected() {
    const InventoryRow* fila = selectedEntry();
    if (fila == nullptr) {
        m_message = "no hay nada elegido";
        return Result<bool>::Error(m_message);
    }
    const std::string nombre = fila->name;
    return remember(m_session.equipItem(fila->objectId), "Te equipas " + nombre + ".");
}

Result<GameSession::EquipPreview> InventoryMenuState::previewSelected() const {
    const InventoryRow* fila = selectedEntry();
    if (fila == nullptr) return Result<GameSession::EquipPreview>::Error("no hay nada elegido");
    return m_session.previewEquip(fila->objectId);
}

Result<bool> InventoryMenuState::unequip(RPG::EquipSlot slot) {
    return remember(m_session.unequipItem(slot), "Guardado en la mochila.");
}

}  // namespace Game
