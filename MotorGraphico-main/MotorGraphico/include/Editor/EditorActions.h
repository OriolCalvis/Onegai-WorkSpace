#pragma once

#include "Editor/EditorDocumentSession.h"
#include <array>
#include <optional>
#include <string>

namespace Editor {

enum class EditorAction { Save, Close, Quit, Undo, Redo, Paint, Erase, Fill, PlayerStart, Objects, RemoveObject, Center, Help, Layers };
enum class ActionGroup { File, Edit, Tools, View };

struct ActionDefinition {
    EditorAction id;
    const char* label;
    ActionGroup group;
    const char* key;
    bool primary = false; // Cmd en macOS, Ctrl en las demás plataformas.
    bool shift = false;
    const char* alternateKey = "";
};

struct ActionInput {
    bool textEditing = false;
    bool itemActive = false;
    bool modal = false;
    bool viewportFocused = false;
};

// Tabla única para menús, botones, atajos y ayuda; sin dependencia de ImGui.
const std::array<ActionDefinition, 14>& editorActionDefinitions();
std::string shortcutLabel(const ActionDefinition& action, bool mac);

class EditorActions {
public:
    bool enabled(EditorAction action, const EditorDocumentSession& session) const;
    std::optional<EditorAction> shortcut(const std::string& key, bool primary, bool shift,
        bool alt, const ActionInput& input, const EditorDocumentSession& session) const;
    Result<bool> execute(EditorAction action, EditorDocumentSession& session);
    Result<bool> applyAt(EditorDocumentSession& session, GridCoord cell, int gid, bool erase = false,
                         const ObjectCatalog* catalog = nullptr, const std::string& objectId = {});
    EditorAction tool() const { return m_tool; }
    bool helpVisible() const { return m_help; }
    bool takeCenterRequest();
    bool takeLayersRequest();
private:
    EditorAction m_tool = EditorAction::Paint;
    bool m_help = false;
    bool m_center = false, m_layers = false;
};

} // namespace Editor
