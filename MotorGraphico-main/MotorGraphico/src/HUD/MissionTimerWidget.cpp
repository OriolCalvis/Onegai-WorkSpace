// H3-06 (Gantt Fase H) — temporizadores de mision visibles.
// Ver include/HUD/MissionTimerWidget.h.
//
// MONTAJE PENDIENTE (bloqueado por ficheros sucios con filas vivas, regla 2):
//   1. include/Engine/Application.h — M (H1-03/H1-05 + MenuJugador en
//      vuelo): un `Game::MissionTimerWidget m_missionTimer`.
//   2. Alimentarlo cada frame en exploracion, donde ya corre el reloj
//      (src/Engine/ApplicationBucle.cpp:254
//      `m_session->updateNarrativeTimers(dtActivo, m_currentLevelPath)`):
//      `widget.show(session.narrativeState().timers(), script.timers)`
//      con el script de la aventura activa. Nada de un segundo reloj para
//      pintar: el widget lee NarrativeState, no avanza tiempo.
//   3. Pintar: src/Engine/ApplicationHud.cpp — M: volcar displayLines()
//      en la zona de mision del HUD; hide() cuando no hay aventura activa.
//      La congelacion (dt=0 en combate/dialogo/pausa/AFK) ya la hace el
//      motor: el widget ensena segundos que no bajan solos.
// Sin este montaje la pieza es decision sin orquesta (mitad GL-free, como
// H2-01/H2-03/H0-08/H3-01): el criterio de cierre H3-06 la pide visible.
//
// MONTADO (05/10, Claude): el pergamino lo pinta en el retal del objetivo
// con relojesVisibles() (la misma regla que displayLines(), sacada aqui
// para que no haya dos); este widget de texto sigue para quien lo quiera.
#include "HUD/MissionTimerWidget.h"

#include <cstdio>
#include <string>

namespace Game {

namespace {

const RPG::TimerDefinition* findDef(const std::vector<RPG::TimerDefinition>& defs,
                                    const std::string& id) {
    for (const RPG::TimerDefinition& def : defs) {
        if (def.id == id) {
            return &def;
        }
    }
    return nullptr;
}

}  // namespace

// Segundos -> "M:SS" (siempre con dos digitos de segundos: "1:30", "0:05").
// Trunca hacia abajo: lo que se ve nunca promete mas de lo que queda.
std::string formatoReloj(float seconds) {
    if (seconds < 0.0f) {
        seconds = 0.0f;
    }
    const int total = static_cast<int>(seconds);
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%d:%02d", total / 60, total % 60);
    return buffer;
}

// H3-06 — La regla de que se ve (la usan displayLines() y el pergamino).
std::vector<RelojVisible> relojesVisibles(const std::vector<RPG::TimerState>& states,
                                          const std::vector<RPG::TimerDefinition>& definitions) {
    std::vector<RelojVisible> out;
    for (const RPG::TimerState& state : states) {
        if (!state.activo || state.expirado) {
            continue;
        }
        const RPG::TimerDefinition* def = findDef(definitions, state.id);
        if (def != nullptr &&
            def->presentacion == RPG::TimerDefinition::Presentacion::Secreto) {
            continue;  // 2.18: lo secreto no se dibuja.
        }
        RelojVisible r;
        r.id = state.id;
        r.nombre = def != nullptr ? def->nombre : std::string();
        r.restante = state.restante;
        r.total = def != nullptr ? def->duracion : 0.0f;
        r.barra = def != nullptr && def->presentacion == RPG::TimerDefinition::Presentacion::Barra &&
                  def->duracion > 0.0f;
        out.push_back(r);
    }
    return out;
}

void MissionTimerWidget::show(const std::vector<RPG::TimerState>& states,
                              const std::vector<RPG::TimerDefinition>& definitions) {
    m_visible = true;
    m_lines.clear();
    for (const RelojVisible& r : relojesVisibles(states, definitions)) {
        const std::string time = formatoReloj(r.restante);
        const std::string& rotulo = r.nombre.empty() ? r.id : r.nombre;
        if (r.barra) {
            float fraction = r.restante / r.total;
            if (fraction < 0.0f) {
                fraction = 0.0f;
            }
            if (fraction > 1.0f) {
                fraction = 1.0f;
            }
            int filled = static_cast<int>(fraction * 10.0f);
            if (filled < 0) {
                filled = 0;
            }
            if (filled > 10) {
                filled = 10;
            }
            std::string bar = "[";
            for (int i = 0; i < 10; ++i) {
                bar += (i < filled) ? '#' : '-';
            }
            bar += "]";
            m_lines.push_back(rotulo + ": " + bar + " " + time);
        } else {
            m_lines.push_back(rotulo + ": " + time);
        }
    }
}

void MissionTimerWidget::hide() { m_visible = false; }

bool MissionTimerWidget::visible() const { return m_visible; }

std::size_t MissionTimerWidget::shownCount() const {
    return m_visible ? m_lines.size() : 0;
}

std::vector<std::string> MissionTimerWidget::displayLines() const {
    return m_visible ? m_lines : std::vector<std::string>{};
}

}  // namespace Game
