#include "Editor/EditorActions.h"

namespace Editor {

const std::array<ActionDefinition, 14>& editorActionDefinitions() {
    static constexpr std::array<ActionDefinition, 14> definitions{{
        {EditorAction::Save, "Guardar", ActionGroup::File, "S", true},
        {EditorAction::Close, "Cerrar mapa", ActionGroup::File, "W", true},
        {EditorAction::Quit, "Salir", ActionGroup::File, "Q", true},
        {EditorAction::Undo, "Deshacer", ActionGroup::Edit, "Z", true},
        {EditorAction::Redo, "Rehacer", ActionGroup::Edit, "Z", true, true, "Y"},
        {EditorAction::Paint, "Pintar", ActionGroup::Tools, "B"},
        {EditorAction::Erase, "Borrar", ActionGroup::Tools, "E"},
        {EditorAction::Fill, "Rellenar", ActionGroup::Tools, "G"},
        {EditorAction::PlayerStart, "Inicio", ActionGroup::Tools, "P"},
        {EditorAction::Objects, "Objetos", ActionGroup::Tools, "O"},
        {EditorAction::RemoveObject, "Borrar objeto", ActionGroup::Tools, "X"},
        {EditorAction::Center, "Centrar mapa", ActionGroup::View, "Home"},
        {EditorAction::Help, "Ayuda de controles", ActionGroup::View, "F1"},
        {EditorAction::Layers, "Capas", ActionGroup::View, "F2"},
    }};
    return definitions;
}

std::string shortcutLabel(const ActionDefinition& action, bool mac) {
    const std::string prefix = action.primary ? (mac ? "Cmd+" : "Ctrl+") : "";
    return prefix + (action.shift ? "Shift+" : "") + action.key +
        (*action.alternateKey ? " / " + prefix + action.alternateKey : "");
}

bool EditorActions::enabled(EditorAction action, const EditorDocumentSession& session) const {
    if (session.confirmationPending() || session.quitReady()) return false;
    if (action == EditorAction::Quit || action == EditorAction::Help) return true;
    if (!session.hasDocument()) return false;
    if (action == EditorAction::Undo) return session.state().canUndo();
    if (action == EditorAction::Redo) return session.state().canRedo();
    return true;
}

std::optional<EditorAction> EditorActions::shortcut(const std::string& key, bool primary,
    bool shift, bool alt, const ActionInput& input, const EditorDocumentSession& session) const {
    if (input.textEditing || input.itemActive || input.modal || alt) return std::nullopt;
    for (const auto& action : editorActionDefinitions()) {
        const bool matches = (key == action.key && shift == action.shift) ||
            (*action.alternateKey && key == action.alternateKey && !shift);
        if (!matches || primary != action.primary || !enabled(action.id, session)) continue;
        if ((action.group == ActionGroup::Tools || action.id == EditorAction::Center) &&
            !input.viewportFocused) continue;
        return action.id;
    }
    return std::nullopt;
}

Result<bool> EditorActions::execute(EditorAction action, EditorDocumentSession& session) {
    if (!enabled(action, session)) return Result<bool>::Ok(false);
    switch (action) {
        case EditorAction::Save: return session.save();
        case EditorAction::Close: return session.request(EditorDocumentSession::Action::Close);
        case EditorAction::Quit: return session.request(EditorDocumentSession::Action::Quit);
        case EditorAction::Undo: session.undo(); break;
        case EditorAction::Redo: session.redo(); break;
        case EditorAction::Paint:
        case EditorAction::Erase:
        case EditorAction::Fill:
        case EditorAction::Objects:
        case EditorAction::RemoveObject:
        case EditorAction::PlayerStart: m_tool = action; break;
        case EditorAction::Center: m_center = true; break;
        case EditorAction::Layers: m_layers = true; break;
        case EditorAction::Help: m_help = !m_help; break;
    }
    return Result<bool>::Ok(true);
}

Result<bool> EditorActions::applyAt(EditorDocumentSession& session, GridCoord cell, int gid, bool erase,
                                  const ObjectCatalog* catalog, const std::string& objectId) {
    if (!enabled(m_tool, session)) return Result<bool>::Ok(false);
    if (m_tool == EditorAction::RemoveObject || (m_tool == EditorAction::Objects && erase))
        return session.removeObject(cell);
    if (m_tool == EditorAction::Objects) {
        if (!catalog) return Result<bool>::Error("Este mapa no tiene un catálogo de proyecto disponible");
        return session.placeObject(cell, objectId, *catalog);
    }
    if (erase || m_tool == EditorAction::Erase) session.paint(cell.x, cell.y, 0);
    else if (m_tool == EditorAction::Fill) session.fill(cell.x, cell.y, gid);
    else if (m_tool == EditorAction::PlayerStart) session.playerStart(cell);
    else session.paint(cell.x, cell.y, gid);
    return Result<bool>::Ok(true);
}

bool EditorActions::takeLayersRequest() {
    const bool requested = m_layers; m_layers = false; return requested;
}

bool EditorActions::takeCenterRequest() {
    const bool requested = m_center;
    m_center = false;
    return requested;
}

} // namespace Editor
