// EDITOR-UX-01 — El panel Inspector de motor_editor, fuera de
// EditorApplication.cpp: nombre, tamano, objetos, inicio, capas, seleccion y
// estado, cada dato con su icono y su tooltip (EditorAyuda).
#pragma once

#include <string>

#include "Core/Math/GridCoord.h"
#include "Editor/EditorDocumentSession.h"
#include "EditorIconos.h"
#include "Level/ObjectCatalog.h"

namespace Editor {

class InspectorPanel {
public:
    explicit InspectorPanel(EditorIconos& iconos) : m_iconos(iconos) {}
    void draw(const EditorDocumentSession& session, const ObjectCatalog& catalog, GridCoord selected,
              const std::string& status, float uiScale);

private:
    // Icono + texto en una fila con tooltip del control.
    void fila(const char* icono, const char* control, const char* titulo, const std::string& texto, float uiScale);

    EditorIconos& m_iconos;  // el del panel Herramientas: un solo atlas
};

}  // namespace Editor
