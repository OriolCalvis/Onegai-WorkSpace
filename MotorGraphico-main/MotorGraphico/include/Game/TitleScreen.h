#pragma once

#include <string>
#include <vector>

#include "Project/Project.h"

namespace Game {

// Qué hace confirmar la entrada seleccionada del título.
enum class TitleAction { None, PlayCampaign, OpenProject, Continue, LoadGame, Quit };

struct TitleEntry {
    std::string label;      // texto visible en el menú
    std::string projectId;  // vacío en todo lo que no sea OpenProject
    TitleAction action = TitleAction::None;
};

// G-08 (rodaja inicial) — La pantalla de título: qué se puede jugar hoy.
//
// TODO el estado vive aquí, GL-free y testeable, siguiendo la convención
// de ProjectHub (ver su cabecera): Application la pinta con un
// HudCommandMenu y le pasa las teclas; esta clase no sabe nada de
// ventanas ni de OpenGL.
//
// La lista es fija: la campaña primero, los escenarios en trabajo
// después (los proyectos jugables del índice, en su orden) y Salir al
// final. Los proyectos sin entrada jugable no se ocultan en silencio:
// su id aparece en skipped() para que la pantalla explique por qué no
// están entre las opciones.
class TitleScreen {
public:
    // campaignProjectId es el proyecto que ES la campaña (hoy
    // boundington): no se lista dos veces; su opción es la campaña.
    static TitleScreen fromProjects(const std::vector<Editor::Project>& projects,
                                    const std::string& campaignProjectId = "boundington");

    // Navegación con wraparound (mismo criterio que HudCommandMenu: sin
    // borde muerto). No-op con la lista vacía (no puede pasar: siempre
    // hay campaña y Salir).
    void moveUp();
    void moveDown();

    TitleAction confirm() const;              // acción de la entrada seleccionada
    const TitleEntry& selectedEntry() const;  // entrada estática vacía si no hay
    std::size_t selected() const { return m_selected; }

    const std::vector<TitleEntry>& entries() const { return m_entries; }
    std::vector<std::string> labels() const;  // para HudCommandMenu::setOptions
    const std::vector<std::string>& skipped() const { return m_skipped; }

private:
    std::vector<TitleEntry> m_entries;
    std::vector<std::string> m_skipped;
    std::size_t m_selected = 0;
};

}  // namespace Game
