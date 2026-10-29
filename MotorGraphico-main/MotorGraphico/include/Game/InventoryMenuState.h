#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Game/GameSession.h"

// G-12 — Inventario, equipo y ficha utilizables. MITAD SIN INTERFAZ.
//
// QUE RESUELVE. El menu de inventario de GDB-12 (Game::InventoryScreen)
// enseña una fila por objeto, sin cantidades ni categorias, y sus tres
// acciones llamaban a stubs vacios de GameSession. Este es el modelo que
// pide el criterio de G-12: listas por categoria con cantidad, descripcion
// y acciones Usar/Equipar/Desequipar; las acciones imposibles dicen POR
// QUE y no gastan nada; elegir destinatario cuando toca (y cancelarlo no
// gasta); previsualizar el cambio de equipo. Las reglas son las de
// GameSession (useItem/equipItem/unequipItem): aqui no se repite ninguna.
//
// LA SELECCION ES POR IDENTIDAD. Usar la ultima pocion hace desaparecer
// su fila; el cursor no salta a la fila que ocupe su hueco, pasa a la
// siguiente de la lista (mismo criterio que el foco de E-15).
//
// La vista (Application/InventoryScene, GL) no esta aqui a proposito:
// hoy dibuja Game::InventoryScreen y tiene que pasar a leer este modelo.

namespace Game {

enum class ItemCategory { Consumable, Equipment, Quest, Other };
const char* ItemCategoryName(ItemCategory category);

// TIENDA-02 — Como se nombra, se clasifica y se describe un objeto
// cualquiera, este en la mochila o en el mostrador de una tienda (salieron
// de InventoryMenuState, que los usa igual).
std::string nombreObjeto(const GameSession& session, const std::string& objectId);
ItemCategory clasificaObjeto(const GameSession& session, const std::string& objectId);
std::string describeObjeto(const GameSession& session, const std::string& objectId, ItemCategory category);

struct ItemAction {
    bool allowed = false;
    std::string reason;  // vacio si allowed
};

struct InventoryRow {
    std::string objectId;
    std::string name;
    std::string description;
    ItemCategory category = ItemCategory::Other;
    int quantity = 0;
    ItemAction use;
    ItemAction equip;
};

struct EquipmentSlotRow {
    RPG::EquipSlot slot = RPG::EquipSlot::Head;
    std::string label;        // "Cabeza", "Mano principal"...
    std::string equipmentId;  // vacio si no lleva nada
    std::string name;         // nombre de la pieza o "(vacío)"
    ItemAction unequip;
};

class InventoryMenuState {
public:
    explicit InventoryMenuState(GameSession& session);

    // Vuelve a leer la sesion. Se llama solo tras cada accion; hay que
    // llamarlo a mano si la mochila cambia por fuera (una compra).
    void refresh();

    // --- Listas ---
    ItemCategory category() const { return m_category; }
    void setCategory(ItemCategory category);
    // Cuantos objetos distintos hay en cada categoria (para las pestañas).
    std::size_t countIn(ItemCategory category) const;
    const std::vector<InventoryRow>& rows() const { return m_visible; }
    const std::vector<EquipmentSlotRow>& equipment() const { return m_slots; }

    // --- Seleccion ---
    bool select(const std::string& objectId);
    void moveSelection(int delta);
    const std::string& selectedId() const { return m_selected; }
    std::optional<std::size_t> selectedRow() const;
    const InventoryRow* selectedEntry() const;

    // --- Usar, con destinatario ---
    // Pide usar el objeto elegido. Si hay que elegir destinatario, queda
    // pendiente y NO se gasta nada hasta confirmTarget(); cancelTarget()
    // lo descarta sin gastar. Si no lo hay, o no se puede usar, devuelve
    // el motivo.
    Result<bool> requestUse();
    bool hasPendingTarget() const { return !m_pendingItem.empty(); }
    const std::vector<std::string>& targets() const { return m_targets; }
    Result<bool> confirmTarget(std::size_t target);
    void cancelTarget();

    // --- Equipo ---
    Result<bool> equipSelected();
    Result<GameSession::EquipPreview> previewSelected() const;
    Result<bool> unequip(RPG::EquipSlot slot);

    // Ultimo resultado, para la barra del menu.
    const std::string& message() const { return m_message; }

private:
    void rebuildVisible();
    Result<bool> remember(Result<bool> result, const std::string& okText);

    GameSession& m_session;
    ItemCategory m_category = ItemCategory::Consumable;
    std::vector<InventoryRow> m_all;
    std::vector<InventoryRow> m_visible;
    std::vector<EquipmentSlotRow> m_slots;
    std::string m_selected;
    std::string m_pendingItem;
    std::vector<std::string> m_targets;
    std::string m_message;
};

}  // namespace Game

// Modelo GL-free compartido por InventoryScreen y sus pruebas; vive en motor_level.
