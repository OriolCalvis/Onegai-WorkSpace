#include "Game/InventoryScreen.h"
#include "Game/Morality.h"  // MORAL-01
#include "RPG/CharacterSheet.h"
#include "Render/ICombatant.h"
#include <algorithm>

namespace Game {

namespace {

const InventoryEntry kNoEntry{};

}  // namespace

InventoryScreen::InventoryScreen(GameSession& session)
    : m_session(&session), m_menu(std::in_place, session) {
    // Sin catalogos la ficha sigue saliendo, con ids legibles.
    (void)m_fichaVista.cargarCatalogos("assets/catalogs");
    (void)m_codice.cargarCatalogos("assets/catalogs");
    rebuildEntries();
}

FichaDatos InventoryScreen::ficha() const {
    const RPG::CharacterSheet* sheet = nullptr;
    if (m_session) {
        if (m_session->hasCharacterSheet()) {
            sheet = m_session->characterSheet();
        } else if (m_session->playerBody()) {
            sheet = m_session->playerBody()->character_sheet();
        }
    }
    if (sheet == nullptr) return FichaDatos{};
    return m_fichaVista.construir(*sheet, health(), maxHealth(), mana(), maxMana());
}

void InventoryScreen::rebuildEntries() {
    m_inventoryEntries.clear();
    m_equipmentEntries.clear();
    if (m_session) {
        // El modelo decide nombres, cantidades, categorias y permisos. Esta
        // vista solo aplana sus categorias para las cuatro pestañas existentes.
        InventoryMenuState listing(*m_session);
        for (auto category : {ItemCategory::Consumable, ItemCategory::Equipment,
                              ItemCategory::Quest, ItemCategory::Other}) {
            listing.setCategory(category);
            for (const auto& row : listing.rows()) {
                const auto action = category == ItemCategory::Equipment
                    ? InventoryAction::EquipItem : InventoryAction::UseItem;
                const auto& permission = category == ItemCategory::Equipment ? row.equip : row.use;
                m_inventoryEntries.push_back({row.name + " x" + std::to_string(row.quantity),
                    action, row.objectId, -1, category, permission.reason});
            }
        }
        if (m_session->characterSheet()) {
            for (const auto& slot : listing.equipment()) {
                m_equipmentEntries.push_back({slot.label + ": " + slot.name,
                    slot.equipmentId.empty() ? InventoryAction::None : InventoryAction::UnequipItem,
                    slot.equipmentId, static_cast<int>(slot.slot), ItemCategory::Equipment,
                    slot.unequip.reason});
            }
        }
    }
    if (m_inventoryEntries.empty()) {
        m_inventoryEntries.push_back({"(vacío)", InventoryAction::None, "", -1,
                                      ItemCategory::Other, "No hay objetos en la mochila."});
    }
    if (m_equipmentEntries.empty()) {
        m_equipmentEntries.push_back({"(sin ficha de personaje)", InventoryAction::None,
            "", -1, ItemCategory::Equipment, "El personaje no tiene ficha donde equiparlo."});
    }
    if (m_menu && m_menu->hasPendingTarget()) {
        m_inventoryEntries.clear();
        const auto& targets = m_menu->targets();
        for (std::size_t i = 0; i < targets.size(); ++i) {
            m_inventoryEntries.push_back({targets[i], InventoryAction::ConfirmTarget, "",
                                          static_cast<int>(i)});
        }
        m_inventoryEntries.push_back({"Cancelar", InventoryAction::CancelTarget, "", -1});
    }
    m_statusEntries = {{"Ver stats", InventoryAction::ViewStats, "", -1},
                      {"Ver rasgos", InventoryAction::ViewTraits, "", -1},
                      {"Ver condiciones", InventoryAction::ViewConditions, "", -1}};
    m_actionEntries = {{"Cerrar", InventoryAction::Close, "", -1}};
}

void InventoryScreen::refresh() {
    const InventoryEntry previous = selectedEntry();
    if (m_menu) m_menu->refresh();
    rebuildEntries();
    const auto& entries = entriesInFocus();
    for (std::size_t i = 0; i < entries.size(); ++i) {
        if ((!previous.itemId.empty() && entries[i].itemId == previous.itemId &&
             entries[i].slotIndex == previous.slotIndex) ||
            (m_focus == InventoryFocus::Equipment && entries[i].slotIndex == previous.slotIndex)) {
            m_selectedInFocus = i;
            return;
        }
    }
    m_selectedInFocus = entries.empty() ? 0 : std::min(m_selectedInFocus, entries.size() - 1);
}

Result<bool> InventoryScreen::executeSelected() {
    const InventoryEntry entry = selectedEntry();
    Result<bool> result = Result<bool>::Ok(false);
    const bool hadTarget = m_menu && m_menu->hasPendingTarget();
    if (entry.action == InventoryAction::None || !m_menu) {
        result = Result<bool>::Error(entry.reason.empty() ? "No hay ninguna acción disponible." : entry.reason);
    } else if (entry.action == InventoryAction::UnequipItem) {
        result = m_menu->unequip(static_cast<RPG::EquipSlot>(entry.slotIndex));
    } else if (entry.action == InventoryAction::ConfirmTarget) {
        result = m_menu->confirmTarget(static_cast<std::size_t>(entry.slotIndex));
    } else if (entry.action == InventoryAction::CancelTarget) {
        m_menu->cancelTarget();
    } else if (entry.action == InventoryAction::UseItem || entry.action == InventoryAction::EquipItem) {
        m_menu->setCategory(entry.category);
        if (!m_menu->select(entry.itemId)) {
            result = Result<bool>::Error("El objeto ya no está en la mochila.");
        } else {
            result = entry.action == InventoryAction::EquipItem
                ? m_menu->equipSelected() : m_menu->requestUse();
        }
    }
    m_message = result.isOk() ? m_menu ? m_menu->message() : "" : result.errorMessage();
    refresh();
    if (hadTarget != (m_menu && m_menu->hasPendingTarget())) m_selectedInFocus = 0;
    return result;
}

std::string InventoryScreen::panelText(std::size_t maxRows) const {
    const char* titles[] = {"INVENTARIO", "EQUIPO", "ESTADO", "ACCIONES", "MISIONES", "CODICE"};
    std::string text = std::string(titles[static_cast<int>(m_focus)]) + " " +
        std::to_string(static_cast<int>(m_focus) + 1) + "/" + std::to_string(static_cast<int>(InventoryFocus::COUNT)) + "\n";
    text += "< > Pestaña | ESC Cerrar\nW/S Elegir | ENTER\n";
    const auto& entries = entriesInFocus();
    maxRows = std::max<std::size_t>(1, maxRows);
    const auto first = m_selectedInFocus >= maxRows ? m_selectedInFocus - maxRows + 1 : 0;
    const auto end = std::min(entries.size(), first + maxRows);
    for (std::size_t i = first; i < end; ++i) {
        text += (i == m_selectedInFocus ? "> " : "  ") + entries[i].label + "\n";
    }
    if (m_focus == InventoryFocus::Status) {
        for (const auto& label : statusLabels()) text += label + "\n";
    } else {
        switch (confirm()) {
            case InventoryAction::UseItem: text += "Usar\n"; break;
            case InventoryAction::EquipItem: text += "Equipar\n"; break;
            case InventoryAction::UnequipItem: text += "Desequipar\n"; break;
            default: break;
        }
    }
    text += m_message.empty() ? selectedEntry().reason : m_message;
    return text;
}

void InventoryScreen::moveTabLeft() {
    if (m_menu) m_menu->cancelTarget();
    rebuildEntries();
    using U = std::underlying_type_t<InventoryFocus>;
    U f = static_cast<U>(m_focus);
    f = (f == 0) ? static_cast<U>(InventoryFocus::COUNT) - 1 : f - 1;
    m_focus = static_cast<InventoryFocus>(f);
    m_selectedInFocus = 0;
}

void InventoryScreen::moveTabRight() {
    if (m_menu) m_menu->cancelTarget();
    rebuildEntries();
    using U = std::underlying_type_t<InventoryFocus>;
    U f = static_cast<U>(m_focus);
    f = (f + 1) % static_cast<U>(InventoryFocus::COUNT);
    m_focus = static_cast<InventoryFocus>(f);
    m_selectedInFocus = 0;
}

void InventoryScreen::moveUp() {
    const auto& entries = entriesInFocus();
    const std::size_t n = entries.empty() ? m_externalRows : entries.size();
    if (n == 0) return;
    m_selectedInFocus = (m_selectedInFocus == 0) ? n - 1 : m_selectedInFocus - 1;
}

void InventoryScreen::moveDown() {
    const auto& entries = entriesInFocus();
    const std::size_t n = entries.empty() ? m_externalRows : entries.size();
    if (n == 0) return;
    m_selectedInFocus = (m_selectedInFocus + 1) % n;
}

InventoryAction InventoryScreen::confirm() const {
    return selectedEntry().action;
}

const InventoryEntry& InventoryScreen::selectedEntry() const {
    const auto& entries = entriesInFocus();
    if (m_selectedInFocus >= entries.size()) {
        return kNoEntry;
    }
    return entries[m_selectedInFocus];
}

const std::vector<InventoryEntry>& InventoryScreen::entriesInFocus() const {
    static const std::vector<InventoryEntry> kEmpty;
    switch (m_focus) {
        case InventoryFocus::Inventory: return m_inventoryEntries;
        case InventoryFocus::Equipment: return m_equipmentEntries;
        case InventoryFocus::Status: return m_statusEntries;
        case InventoryFocus::Actions: return m_actionEntries;
        case InventoryFocus::Quests:
        case InventoryFocus::Codex:
        case InventoryFocus::COUNT: break;
    }
    return kEmpty;
}

std::vector<std::string> InventoryScreen::inventoryLabels() const {
    std::vector<std::string> out;
    for (const auto& entry : m_inventoryEntries) out.push_back(entry.label);
    return out;
}

std::vector<std::string> InventoryScreen::inventoryIconIds() const {
    std::vector<std::string> out;
    for (const auto& entry : m_inventoryEntries)
        out.push_back(entry.category == ItemCategory::Equipment ? "equipo" : "consumible");
    return out;
}

std::vector<std::string> InventoryScreen::equipmentLabels() const {
    std::vector<std::string> out;
    for (const auto& entry : m_equipmentEntries) out.push_back(entry.label);
    return out;
}

std::vector<std::string> InventoryScreen::statusLabels() const {
    std::vector<std::string> out = FichaVista::lineas(ficha());
    out.push_back("Moral: " + moraleLabel() + " (" + std::to_string(morale()) + ")");
    return out;
}

std::vector<std::string> InventoryScreen::actionLabels() const {
    const InventoryEntry& entry = selectedEntry();
    if (entry.action == InventoryAction::None) return {};
    
    std::vector<std::string> out;
    out.push_back("Confirmar: " + entry.label);
    out.push_back("Cancelar (ESC)");
    return out;
}

int InventoryScreen::health() const {
    if (m_session && m_session->playerBody()) {
        return m_session->playerBody()->health();
    }
    return 0;
}

int InventoryScreen::maxHealth() const {
    if (m_session && m_session->playerBody()) {
        return m_session->playerBody()->maxHealth();
    }
    return 0;
}

int InventoryScreen::mana() const {
    if (m_session && m_session->playerSkills()) {
        return m_session->playerSkills()->mana();
    }
    return 0;
}

int InventoryScreen::maxMana() const {
    if (m_session && m_session->playerSkills()) {
        return m_session->playerSkills()->maxMana();
    }
    return 0;
}

int InventoryScreen::gold() const { return m_session ? m_session->gold() : 0; }

int InventoryScreen::morale() const { return m_session ? m_session->morality().value() : 0; }

std::string InventoryScreen::moraleLabel() const {
    return moral::nombreTramo(moral::tramoDe(morale()));  // MORAL-01: los siete tramos
}

}  // namespace Game