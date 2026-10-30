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


// EL ESTADO de la lista de problemas: que hay, en que orden y cual tiene
// el foco.
//
// 24/09/2026 (EST-31): de 643 lineas a ~215. La navegacion («ir ahi») y la
// busqueda salieron a ficheros propios; aqui queda lo que es de la lista.

#include "Editor/ProblemsState.h"

#include <algorithm>
#include <functional>
#include <set>

#include "Map/EventLayerDocument.h"
#include "RPG/EventCommand.h"

#include "Editor/ProblemsInternal.h"

namespace Editor {

using internal::BuscarEvento;
using internal::SeguirRuta;

void ProblemsState::setReport(const std::string& mapId, const RPG::ValidationReport& report,
                              const MapDocument* document) {
    const std::string claveAnterior = hasFocus() ? m_problems[m_focusIndex].key() : std::string();
    const bool teniaFoco = hasFocus();

    // Fuera los de este mapa; los de los demás se quedan.
    m_problems.erase(std::remove_if(m_problems.begin(), m_problems.end(),
                                    [&mapId](const ProblemEntry& p) {
                                        return p.where.mapId == mapId;
                                    }),
                     m_problems.end());

    for (const RPG::Diagnostic& d : report.diagnostics) {
        ProblemEntry entrada;
        entrada.severity = d.severity;
        entrada.message = d.message;
        entrada.where.mapId = mapId;
        entrada.where.eventId = d.eventId;
        entrada.where.hasPage = d.hasPage;
        entrada.where.pageIndex = d.pageIndex;
        entrada.where.hasCommand = !d.commandPath.empty();
        entrada.where.commandPath = d.commandPath;

        // Aquí se ata el diagnóstico a la identidad del documento. Es el
        // único momento en que se sabe seguro a qué página y a qué
        // comando se refería: después, el documento puede haber cambiado.
        if (document != nullptr && !d.eventId.empty()) {
            const MapEventDocument* evento = BuscarEvento(*document, d.eventId);
            if (evento != nullptr && d.hasPage && d.pageIndex < evento->pages.size()) {
                const MapEventPage& pagina = evento->pages[d.pageIndex];
                entrada.where.pageId = pagina.id.value();
                if (entrada.where.hasCommand) {
                    const MapEventCommand* comando = SeguirRuta(pagina.commands, d.commandPath);
                    if (comando != nullptr) {
                        entrada.where.commandType = comando->type;
                    }
                }
            }
        }
        m_problems.push_back(std::move(entrada));
    }

    rebuildOrder();
    restoreFocus(claveAnterior, teniaFoco);
}

void ProblemsState::removeMap(const std::string& mapId) {
    const std::string claveAnterior = hasFocus() ? m_problems[m_focusIndex].key() : std::string();
    const bool teniaFoco = hasFocus();
    m_problems.erase(std::remove_if(m_problems.begin(), m_problems.end(),
                                    [&mapId](const ProblemEntry& p) {
                                        return p.where.mapId == mapId;
                                    }),
                     m_problems.end());
    restoreFocus(claveAnterior, teniaFoco);
}

void ProblemsState::clear() {
    m_problems.clear();
    m_focusIndex = static_cast<std::size_t>(-1);
    m_lastFocusChange = FocusChange::None;
}

void ProblemsState::rebuildOrder() {
    // Errores arriba: son los que bloquean probar, y son los que el autor
    // tiene que mirar primero. Dentro de cada grupo, por sitio, para que
    // dos problemas del mismo evento salgan juntos.
    std::stable_sort(m_problems.begin(), m_problems.end(),
                     [](const ProblemEntry& a, const ProblemEntry& b) {
                         if (a.isError() != b.isError()) {
                             return a.isError();
                         }
                         if (a.where.mapId != b.where.mapId) {
                             return a.where.mapId < b.where.mapId;
                         }
                         if (a.where.eventId != b.where.eventId) {
                             return a.where.eventId < b.where.eventId;
                         }
                         if (a.where.pageIndex != b.where.pageIndex) {
                             return a.where.pageIndex < b.where.pageIndex;
                         }
                         return a.where.commandPath < b.where.commandPath;
                     });
}

void ProblemsState::restoreFocus(const std::string& previousKey, bool hadFocus) {
    if (!hadFocus) {
        m_focusIndex = static_cast<std::size_t>(-1);
        m_lastFocusChange = FocusChange::None;
        return;
    }
    for (std::size_t i = 0; i < m_problems.size(); ++i) {
        if (m_problems[i].key() == previousKey) {
            const bool mismoSitio = (i == m_focusIndex);
            m_focusIndex = i;
            m_lastFocusChange = mismoSitio ? FocusChange::Unchanged : FocusChange::Kept;
            return;
        }
    }
    // El problema enfocado ya no está: lo más probable es que el autor
    // acabe de arreglarlo. Se le deja mirando el siguiente, no el
    // primero de la lista ni ninguno: seguir bajando es lo que iba a
    // hacer de todos modos.
    if (m_problems.empty()) {
        m_focusIndex = static_cast<std::size_t>(-1);
        m_lastFocusChange = FocusChange::Lost;
        return;
    }
    if (m_focusIndex >= m_problems.size()) {
        m_focusIndex = m_problems.size() - 1;
    }
    m_lastFocusChange = FocusChange::MovedToNext;
}

std::size_t ProblemsState::errorCount() const {
    std::size_t n = 0;
    for (const ProblemEntry& p : m_problems) {
        if (p.isError()) {
            ++n;
        }
    }
    return n;
}

std::size_t ProblemsState::warningCount() const {
    return m_problems.size() - errorCount();
}

const ProblemEntry* ProblemsState::focused() const {
    return hasFocus() ? &m_problems[m_focusIndex] : nullptr;
}

bool ProblemsState::focusOn(const std::string& key) {
    for (std::size_t i = 0; i < m_problems.size(); ++i) {
        if (m_problems[i].key() == key) {
            m_focusIndex = i;
            m_lastFocusChange = FocusChange::Kept;
            return true;
        }
    }
    return false;
}

bool ProblemsState::focusIndexAt(std::size_t index) {
    if (index >= m_problems.size()) {
        return false;
    }
    m_focusIndex = index;
    m_lastFocusChange = FocusChange::Kept;
    return true;
}

void ProblemsState::focusNext() {
    if (m_problems.empty()) {
        return;
    }
    m_focusIndex = hasFocus() ? std::min(m_focusIndex + 1, m_problems.size() - 1) : 0;
    m_lastFocusChange = FocusChange::Kept;
}

void ProblemsState::focusPrevious() {
    if (m_problems.empty()) {
        return;
    }
    if (!hasFocus()) {
        m_focusIndex = 0;
    } else if (m_focusIndex > 0) {
        --m_focusIndex;
    }
    m_lastFocusChange = FocusChange::Kept;
}

void ProblemsState::clearFocus() {
    m_focusIndex = static_cast<std::size_t>(-1);
    m_lastFocusChange = FocusChange::None;
}

NavigationRequest ProblemsState::navigate(const MapDocument& document) const {
    const ProblemEntry* entrada = focused();
    if (entrada == nullptr) {
        NavigationRequest peticion;
        peticion.outcome = NavigationOutcome::NoTarget;
        peticion.note = "no hay ningún problema seleccionado";
        return peticion;
    }
    return ResolveNavigation(document, entrada->where);
}

// ---------------------------------------------------------------------
// Búsqueda
// ---------------------------------------------------------------------


}  // namespace Editor
