// EDITOR-UX-01 — El panel Herramientas de motor_editor y la ventana de
// ayuda, fuera de EditorApplication.cpp (que solo puede adelgazar): cada
// herramienta con su icono, su atajo y su explicacion (EditorAyuda), la
// paleta de baldosas y el catalogo de objetos con tooltip, y el pie que
// dice que hace cada boton del raton con la herramienta activa.
#pragma once

#include <map>
#include <string>

#include "Core/Math/Vector2.h"
#include "Editor/EditorActions.h"
#include "Editor/EditorDocumentSession.h"
#include "EditorIconos.h"
#include "EditorVistaDeMapa.h"

namespace Editor {

struct ToolsContext {
    EditorDocumentSession& session;
    EditorActions& actions;
    MapView& map;
    int& selectedGid;
    std::string& selectedObject;
    char* objectSearch;          // buffer de ImGui (objectSearchSize bytes)
    std::size_t objectSearchSize;
    std::string& status;
    bool blocked;
    float uiScale;
    bool mac;                    // etiquetas de atajo con Cmd en vez de Ctrl
};

class ToolsPanel {
public:
    void draw(const ToolsContext& ctx);
    // "Ayuda de controles" (F1): todas las acciones con icono, atajo y texto.
    void drawHelp(const ToolsContext& ctx, const ImGuiViewport* viewport);
    // Centro del control, para el harness de humo: "fill", "objectSearch",
    // "objectRow" (la fila "pocion").
    Vector2 testPoint(const std::string& control) const;
    EditorIconos& iconos() { return m_iconos; }

private:
    void record(const std::string& control);
    void drawTools(const ToolsContext& ctx);
    void drawObjects(const ToolsContext& ctx);
    void drawTiles(const ToolsContext& ctx);

    EditorIconos m_iconos;
    std::map<std::string, Vector2> m_points;
};

}  // namespace Editor
