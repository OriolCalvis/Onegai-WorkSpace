// H4-02 / E-11 — La ventana «Proyecto» de motor_editor.
//
// Antes era un bloque de EditorApplication::run que solo listaba proyectos y
// abria mapas. Ahora es su panel, con el arbol de proyectos y mapas (con su
// nombre visible, no el del fichero) y los formularios para crear un proyecto,
// crear un mapa eligiendo tamano y tileset, duplicarlo, renombrarlo, sacarlo
// del proyecto, decidir en que mapa empieza el juego y el titulo del juego.
//
// Solo pinta: lo que se decide vive en Editor::ProjectPanelState (sin ImGui,
// con prueba propia) y el documento abierto sigue siendo de la sesion, que es
// quien pregunta antes de perder cambios.
#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Math/Vector2.h"
#include "Editor/EditorDocumentSession.h"
#include "Editor/ProjectPanelState.h"

namespace Editor {

class ProjectPanel {
public:
    explicit ProjectPanel(std::string assetsRoot = "assets");

    // Lee los proyectos; si el indice falla, deja el motivo en status.
    void load(std::string& status);
    // openProject: el proyecto que se despliega al arrancar (--project).
    void draw(EditorDocumentSession& session, std::string& status, bool blocked,
              const std::string& openProject = {});

    // Siempre el mismo vector: quien guarde la referencia ve los cambios.
    const std::vector<Project>& projects() const { return m_state.projects(); }
    bool modalOpen() const { return m_state.formOpen(); }
    const char* search() const { return m_search.data(); }
    ProjectPanelState& state() { return m_state; }

    // Geometria logica de los controles, para el humo con raton real.
    Vector2 testPoint(const std::string& control) const;

private:
    void drawTree(EditorDocumentSession& session, std::string& status, bool blocked,
                  const std::string& openProject);
    void drawOpenMapActions(const EditorDocumentSession& session, bool blocked);
    void drawForm(EditorDocumentSession& session, std::string& status);
    void record(const std::string& control);
    // Ruta canonica del documento abierto ("" sin documento); se recalcula
    // solo cuando cambia, no en cada fotograma ni por cada mapa del arbol.
    const std::string& openCanonical(const EditorDocumentSession& session);

    ProjectPanelState m_state;
    std::map<std::string, Vector2> m_points;
    std::vector<char> m_search = std::vector<char>(128, '\0');
    std::vector<char> m_text = std::vector<char>(160, '\0');
    std::string m_boundForm;                  // formulario cuyo texto esta en m_text
    std::string m_previewFor, m_preview;      // id de fichero que saldria (cache por texto)
    std::string m_unfolded;                   // ultimo proyecto que se desplego solo
    std::string m_openPath, m_openCanonical;  // documento abierto (cache por ruta)
};

}  // namespace Editor
