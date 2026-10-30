#pragma once

#include <string>
#include <optional>
#include <vector>

#include "Game/Codice.h"
#include "Game/FichaVista.h"
#include "Game/GameSession.h"
#include "Game/InventoryMenuState.h"

namespace Game {

// Dónde está el foco dentro de la pantalla de inventario/estado.
// MISIONES-01: Quests (diario) y Codex (codice) son pestanas de solo lectura
// cuyas filas construye el HUD desde la narrativa; aqui solo se navega.
enum class InventoryFocus { Inventory, Equipment, Status, Actions, Quests, Codex, COUNT };

// Qué hacer al confirmar (depende del foco).
enum class InventoryAction {
    None,
    UseItem,
    EquipItem,
    UnequipItem,
    DropItem,
    ViewStats,
    ViewTraits,
    ViewConditions,
    ConfirmTarget,
    CancelTarget,
    Close
};

struct InventoryEntry {
    std::string label;
    InventoryAction action = InventoryAction::None;
    std::string itemId;   // para items del inventario
    int slotIndex = -1;   // para slots de equipamiento
    ItemCategory category = ItemCategory::Other;
    std::string reason;
};

// GDB-12.3 — Pantalla de Inventario / Estado / Equipo.
//
// GL-free, testeable. Application la pinta con HudCommandMenu y
// HudWidgetPanel. Estado del jugador viene de GameSession (CharacterSheet
// si hay, legacy fallback si no).
class InventoryScreen {
public:
    // Crea la pantalla con referencia a la sesión (para leer estado).
    explicit InventoryScreen(GameSession& session);
    
    // Constructor por defecto para testing (estado vacío).
    InventoryScreen() { rebuildEntries(); }

    // Navegación: tabs (foco) + lista dentro del tab.
    void moveTabLeft();
    void moveTabRight();
    void moveUp();
    void moveDown();

    InventoryAction confirm() const;
    Result<bool> executeSelected();
    void refresh();
    const std::string& message() const { return m_message; }
    std::string panelText(std::size_t maxRows = 3) const;
    const InventoryEntry& selectedEntry() const;
    InventoryFocus focus() const { return m_focus; }
    std::size_t selectedInFocus() const { return m_selectedInFocus; }
    const std::vector<InventoryEntry>& entriesInFocus() const;

    // Datos para pintar (HUD widgets usan estos getters).
    std::vector<std::string> inventoryLabels() const;
    std::vector<std::string> equipmentLabels() const;
    std::vector<std::string> statusLabels() const;
    // FICHA-01: la ficha completa (stats desglosados, equipo, rasgos,
    // condiciones) para el widget con iconos; statusLabels() es su texto.
    FichaDatos ficha() const;
    // Icono de cada objeto de inventoryLabels(): "equipo" si es una pieza
    // del catalogo de equipo, "consumible" si no.
    std::vector<std::string> inventoryIconIds() const;
    // Pestanas sin entradas propias (Quests/Codex): cuantas filas tiene la
    // lista que pinta el HUD, para que W/S la recorran.
    void setExternalRowCount(std::size_t rows) { m_externalRows = rows; }
    // CODICE-01: el codice con sus catalogos; lo construye el HUD con la
    // narrativa de la partida (que la sesion no expone).
    const Codice& codice() const { return m_codice; }
    std::vector<std::string> actionLabels() const;

    // Estado del jugador para HUD (CharacterSheet o legacy).
    int health() const;
    int maxHealth() const;
    int mana() const;
    int maxMana() const;
    int gold() const;
    int morale() const;
    std::string moraleLabel() const;

private:
    GameSession* m_session = nullptr;
    std::optional<InventoryMenuState> m_menu; // Modelo de acciones, vive con la pantalla.
    std::string m_message; // Ultimo resultado visible de una accion.
    FichaVista m_fichaVista;  // catalogos para nombrar la ficha (FICHA-01)
    Codice m_codice;          // catalogos del codice (CODICE-01)
    InventoryFocus m_focus = InventoryFocus::Inventory;
    std::size_t m_selectedInFocus = 0;
    std::size_t m_externalRows = 0;
    std::vector<InventoryEntry> m_inventoryEntries;
    std::vector<InventoryEntry> m_equipmentEntries;
    std::vector<InventoryEntry> m_statusEntries;
    std::vector<InventoryEntry> m_actionEntries;

    void rebuildEntries();
};

}  // namespace Game