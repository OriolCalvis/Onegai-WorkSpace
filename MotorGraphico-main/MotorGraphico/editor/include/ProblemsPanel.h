// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-15  (fase E, 3 j) - Problemes i cerca d'esdeveniments
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include "Editor/ProblemsState.h"
#include "Map/MapDocument.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace Editor {

// E-15 — Panel ImGui de problemas y busqueda de eventos.
//
// Consume ProblemsState (la mitad GL-free que ya existe) y la dibuja.
// El estado no sabe que existe este panel; mismo reparto que E-07
// (validacion abajo, dibujo arriba) y E-12 (LayerPanel consume
// EditorDocumentSession sin poseerla).
//
// QUE DIBUJA. Tres pestanas: Problemas (la lista de E-07 con severidad,
// sitio y «Ir ahi»), Buscar (eventos por nombre), Referencias (usos de
// una bandera/variable/interruptor). La barra inferior dice cuantos
// errores y avisos hay, y si se puede probar.
//
// NAVEGACION. El panel NO manipula la seleccion del editor directamente.
// Produce un NavigationRequest y lo entrega al callback; el editor
// decide que hacer con el. Asi el panel se puede probar sin ventana:
// se le pasa un callback que registra lo que recibe.
//
// FOCO. El panel pide foco con requestFocus() (atajo del menu View).
// El foco del problema se gestiona desde ProblemsState; el panel solo
// lo refleja.

class ProblemsPanel {
public:
    using NavigateCallback = std::function<void(const NavigationRequest&)>;

    void draw(ProblemsState& state, const MapDocument* document,
              std::string& status, NavigateCallback onNavigate = {});

    void requestFocus() { m_focusRequested = true; }

    // Posiciones logicas de controles para el humo con raton real.
    struct Point { float x = 0; float y = 0; };
    Point testPoint(const std::string& control) const;

private:
    void drawProblemList(ProblemsState& state, const MapDocument* document,
                         std::string& status, const NavigateCallback& onNavigate);
    void drawSearch(const MapDocument* document);
    void drawReferences(const MapDocument* document);
    void record(const std::string& control);

    enum class Tab { Problems, Search, References };
    Tab m_tab = Tab::Problems;

    char m_searchQuery[128] = {};
    std::vector<SearchHit> m_searchResults;

    static constexpr const char* kRefKindNames[] = {
        "Bandera", "Variable", "Interruptor local",
        "Evento", "Mapa", "Objeto"
    };
    int m_refKindIndex = 0;
    char m_refName[128] = {};
    std::vector<SearchHit> m_refResults;
    std::vector<std::string> m_refNameList;
    int m_refNameListIndex = -1;

    bool m_focusRequested = false;
    std::map<std::string, Point> m_points;
};

}  // namespace Editor
