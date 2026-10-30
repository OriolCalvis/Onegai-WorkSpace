// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   E-03  (fase E, 4 j) - Planificador, esperes i processos paral·lels
//   E-08  (fase E, 5 j) - Depuració pas a pas
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "RPG/EventInterpreter.h"

#include <algorithm>
#include <memory>
#include <utility>

// E-03 — Implementación del planificador. Ver la nota larga del header:
// aquí solo el cómo.

namespace RPG {

namespace {

// La página de un evento, o nullptr si el índice no existe.
const MapEventPage* PaginaDe(const MapEventDocument& event, std::size_t pageIndex) {
    return pageIndex < event.pages.size() ? &event.pages[pageIndex] : nullptr;
}

}  // namespace

Result<bool> EventInterpreter::start(const MapEventDocument& event, std::size_t pageIndex) {
    if (event.id.empty()) {
        return Result<bool>::Error("no se puede ejecutar un evento sin id");
    }
    // No se reentra en el mismo evento mientras está activo (diseño de
    // bloques §8). Sin esto, machacar la tecla de acción sobre un PNJ
    // arranca cinco copias de su diálogo y las recompensas se entregan
    // cinco veces.
    if (isRunning(event.id.value())) {
        return Result<bool>::Error("el evento '" + event.id.value() + "' ya se está ejecutando");
    }
    const MapEventPage* pagina = PaginaDe(event, pageIndex);
    if (pagina == nullptr) {
        return Result<bool>::Error("el evento '" + event.id.value() + "' no tiene la página " +
                                   std::to_string(pageIndex));
    }

    auto ejecucionPtr = std::make_unique<Execution>();
    Execution& ejecucion = *ejecucionPtr;
    ejecucion.event = event;
    ejecucion.run.eventId = event.id.value();
    ejecucion.run.pageIndex = pageIndex;
    ejecucion.run.parallel = pagina->trigger == EventTrigger::Parallel;
    ejecucion.run.state = EventRunState::Ready;
    // El marco apunta a la copia PROPIA del evento, no al documento del
    // llamante: si el autor edita el mapa mientras esto corre, la
    // ejecución sigue con la versión con la que arrancó en vez de saltar
    // a comandos de otro árbol a media frase.
    const MapEventPage* propia = PaginaDe(ejecucion.event, pageIndex);
    ejecucion.stack.push_back(Frame{&propia->commands, 0});

    m_runs.push_back(ejecucion.run);
    m_executions.push_back(std::move(ejecucionPtr));
    return Result<bool>::Ok(true);
}

void EventInterpreter::tick(int deltaMs) {
    if (deltaMs < 0) {
        deltaMs = 0;
    }
    // E-08 — En pausa no avanza nadie por su cuenta. El tiempo TAMPOCO:
    // si las esperas siguieran corriendo con el depurador parado, el
    // autor volveria de mirar el estado y se encontraria la escena en
    // otro sitio.
    if (m_mode == RunMode::Paused) {
        return;
    }
    // En orden de arranque, siempre. El orden de ejecuciones simultáneas
    // tiene que ser determinista (§8): si dependiera del orden de un
    // unordered_map, dos partidas con los mismos pasos darían resultados
    // distintos y no habría forma de reproducir un fallo.
    for (auto& puntero : m_executions) {
        Execution& ejecucion = *puntero;
        if (ejecucion.run.state == EventRunState::Finished ||
            ejecucion.run.state == EventRunState::Cancelled ||
            ejecucion.run.state == EventRunState::Failed) {
            continue;
        }

        // 1. Esperas por tiempo.
        if (ejecucion.run.state == EventRunState::Waiting && !ejecucion.waitingOnHost) {
            ejecucion.waitRemainingMs -= deltaMs;
            if (ejecucion.waitRemainingMs > 0) {
                continue;
            }
            ejecucion.waitRemainingMs = 0;
            ejecucion.run.state = EventRunState::Running;
        }

        // 2. Esperas abiertas: se pregunta al anfitrión UNA vez por tick.
        if (ejecucion.run.state == EventRunState::Waiting && ejecucion.waitingOnHost) {
            if (ejecucion.waitCommand == nullptr ||
                !m_host.waitResolved(*ejecucion.waitCommand)) {
                continue;
            }
            ejecucion.waitingOnHost = false;
            ejecucion.waitCommand = nullptr;
            ejecucion.run.state = EventRunState::Running;
        }

        // 3. Avanzar, con presupuesto. Al agotarlo se cede el frame
        // aunque quede secuencia: una lista de mil comandos sin esperas
        // no puede congelar el juego.
        ejecucion.run.state = EventRunState::Running;
        for (std::size_t paso = 0; paso < kCommandsPerTick; ++paso) {
            if (ejecucion.run.state != EventRunState::Running) {
                break;
            }
            stepOne(ejecucion);
            // E-08 — Un punto de parada pausa el intérprete entero. Sin
            // este corte, el presupuesto seguiría gastándose y el
            // depurador pararía 63 comandos más tarde.
            if (m_mode == RunMode::Paused) {
                break;
            }
        }
    }

    // El espejo público se reconstruye entero: es pequeño y así no puede
    // quedarse desincronizado por un camino que olvide actualizarlo.
    m_runs.clear();
    m_runs.reserve(m_executions.size());
    for (const auto& puntero : m_executions) {
        const Execution& ejecucion = *puntero;
        m_runs.push_back(ejecucion.run);
    }
}

void EventInterpreter::stepOne(Execution& ejecucion) {
    // Se acabaron los marcos: la secuencia ha terminado.
    while (!ejecucion.stack.empty()) {
        Frame& marco = ejecucion.stack.back();
        if (marco.index < marco.commands->size()) {
            break;
        }
        // Fin de una rama: se vuelve al marco de arriba. No hay "fin de
        // condición" huérfano porque las ramas son hijas del comando, no
        // marcadores sueltos en una lista plana.
        ejecucion.stack.pop_back();
    }
    if (ejecucion.stack.empty()) {
        finish(ejecucion, EventRunState::Finished);
        return;
    }

    Frame& marco = ejecucion.stack.back();

    // E-08 — Punto de parada. Se mira ANTES de ejecutar: el autor quiere
    // ver el estado con el que el comando va a correr, no el que deja.
    const Breakpoint* parada = breakpointAt(ejecucion);
    if (parada != nullptr && !ejecucion.skipBreakpointOnce) {
        m_lastHit = *parada;
        m_hit = true;
        m_mode = RunMode::Paused;
        return;   // sin ejecutar ni avanzar el indice
    }
    // Pasado el punto una vez, la marca se consume: si el autor vuelve a
    // llegar aqui (un bucle, otra interaccion) tiene que volver a parar.
    ejecucion.skipBreakpointOnce = false;

    const MapEventCommand& comando = (*marco.commands)[marco.index];
    // E-08 — La traza se anota con la ruta del comando ANTES de avanzar
    // el indice, que es la coordenada que el editor usa para resaltarlo.
    TraceEntry entrada;
    entrada.eventId = ejecucion.run.eventId;
    entrada.pageIndex = ejecucion.run.pageIndex;
    entrada.commandPath = RutaActual(ejecucion);
    entrada.commandType = comando.type;
    m_trace.push_back(std::move(entrada));

    ++marco.index;   // se avanza ANTES de ejecutar: si el comando abre una
                     // rama, al volver de ella se sigue por el siguiente y
                     // no se repite el control eternamente.
    const CommandResult resultado = m_host.execute(comando);
    applyResult(ejecucion, comando, resultado);
}

// ---------------------------------------------------------------------
// E-08 — Depuracion paso a paso
// ---------------------------------------------------------------------

std::vector<std::size_t> EventInterpreter::RutaActual(const Execution& execution) {
    std::vector<std::size_t> ruta;
    ruta.reserve(execution.stack.size());
    for (const Frame& marco : execution.stack) {
        ruta.push_back(marco.index);
    }
    return ruta;
}

EventInterpreter::Execution* EventInterpreter::find(const std::string& eventId) {
    for (auto& puntero : m_executions) {
        if (puntero->run.eventId == eventId) {
            return puntero.get();
        }
    }
    return nullptr;
}

const EventInterpreter::Execution* EventInterpreter::find(const std::string& eventId) const {
    for (const auto& puntero : m_executions) {
        if (puntero->run.eventId == eventId) {
            return puntero.get();
        }
    }
    return nullptr;
}

const Breakpoint* EventInterpreter::breakpointAt(const Execution& execution) const {
    if (m_breakpoints.empty() || execution.stack.empty()) {
        return nullptr;
    }
    const std::vector<std::size_t> ruta = RutaActual(execution);
    for (const Breakpoint& punto : m_breakpoints) {
        if (punto.eventId == execution.run.eventId &&
            punto.pageIndex == execution.run.pageIndex && punto.commandPath == ruta) {
            return &punto;
        }
    }
    return nullptr;
}

void EventInterpreter::setMode(RunMode mode) {
    if (mode == RunMode::Continuous && m_hit) {
        for (auto& puntero : m_executions) {
            if (puntero->run.eventId == m_lastHit.eventId) {
                puntero->skipBreakpointOnce = true;
            }
        }
        m_hit = false;
    }
    m_mode = mode;
}

void EventInterpreter::addBreakpoint(Breakpoint breakpoint) {
    for (const Breakpoint& punto : m_breakpoints) {
        if (punto == breakpoint) {
            return;   // ponerlo dos veces no lo duplica
        }
    }
    m_breakpoints.push_back(std::move(breakpoint));
}

bool EventInterpreter::removeBreakpoint(const Breakpoint& breakpoint) {
    const auto encontrado =
        std::find(m_breakpoints.begin(), m_breakpoints.end(), breakpoint);
    if (encontrado == m_breakpoints.end()) {
        return false;
    }
    m_breakpoints.erase(encontrado);
    return true;
}

void EventInterpreter::clearBreakpoints() {
    m_breakpoints.clear();
    m_hit = false;
}

const MapEventCommand* EventInterpreter::nextCommand(const std::string& eventId) const {
    const Execution* ejecucion = find(eventId);
    if (ejecucion == nullptr || ejecucion->stack.empty()) {
        return nullptr;
    }
    const Frame& marco = ejecucion->stack.back();
    if (marco.commands == nullptr || marco.index >= marco.commands->size()) {
        return nullptr;
    }
    return &(*marco.commands)[marco.index];
}

std::vector<std::size_t> EventInterpreter::nextCommandPath(const std::string& eventId) const {
    const Execution* ejecucion = find(eventId);
    return ejecucion == nullptr ? std::vector<std::size_t>{} : RutaActual(*ejecucion);
}

bool EventInterpreter::stepOnce() {
    m_stepBlocked.clear();
    m_stepNote.clear();
    // Reanudar desde un punto de parada: el paso siguiente no vuelve a
    // pararse en el mismo sitio, o no se avanzaria nunca.
    for (auto& puntero : m_executions) {
        Execution& ejecucion = *puntero;
        if (ejecucion.run.state == EventRunState::Finished ||
            ejecucion.run.state == EventRunState::Cancelled ||
            ejecucion.run.state == EventRunState::Failed) {
            continue;
        }
        // UNA ESPERA NO SE SALTA EN SILENCIO (especificacion §15.3). Se
        // atraviesa —en pausa el tiempo no corre, asi que no hacerlo
        // dejaria el depurador trabado para siempre— y se ANOTA, para que
        // el autor pueda distinguir un recorrido que el juego hace de uno
        // que solo hace el depurador.
        if (ejecucion.run.state == EventRunState::Waiting) {
            m_stepNote = "se saltó una espera del evento '" + ejecucion.run.eventId +
                         "': el juego real habría esperado";
            ejecucion.waitRemainingMs = 0;
            ejecucion.waitingOnHost = false;
            ejecucion.waitCommand = nullptr;
            ejecucion.run.state = EventRunState::Running;
            m_runs.clear();
            for (const auto& p : m_executions) {
                m_runs.push_back(p->run);
            }
            return true;
        }
        // Reanudar desde un punto de parada: se pasa por encima de ESE
        // punto una vez, o no se avanzaria nunca.
        if (m_hit && m_lastHit.eventId == ejecucion.run.eventId) {
            ejecucion.skipBreakpointOnce = true;
            m_hit = false;
        }
        ejecucion.run.state = EventRunState::Running;
        stepOne(ejecucion);
        m_runs.clear();
        for (const auto& p : m_executions) {
            m_runs.push_back(p->run);
        }
        return true;
    }
    if (m_stepBlocked.empty()) {
        m_stepBlocked = "no hay ninguna ejecución viva que avanzar";
    }
    return false;
}

void EventInterpreter::applyResult(Execution& ejecucion, const MapEventCommand& comando,
                                   const CommandResult& resultado) {
    switch (resultado.outcome) {
        case CommandOutcome::Done:
            break;

        case CommandOutcome::Wait:
            ejecucion.run.state = EventRunState::Waiting;
            if (resultado.waitMs > 0) {
                ejecucion.waitRemainingMs = resultado.waitMs;
                ejecucion.waitingOnHost = false;
                ejecucion.waitCommand = nullptr;
            } else {
                // Sin duración conocida: la resuelve el anfitrión. Se
                // guarda el comando porque waitResolved() lo necesita
                // para saber de qué espera habla.
                ejecucion.waitingOnHost = true;
                ejecucion.waitCommand = &comando;
                ejecucion.waitRemainingMs = 0;
            }
            break;

        case CommandOutcome::Branch: {
            // Una rama fuera de rango NO es un error: un `si` cuya
            // condición es falsa y no tiene `si no` simplemente no entra
            // en ningún sitio y la secuencia sigue.
            if (resultado.branch >= 0 &&
                static_cast<std::size_t>(resultado.branch) < comando.children.size()) {
                const MapEventCommand& rama = comando.children[
                    static_cast<std::size_t>(resultado.branch)];
                ejecucion.stack.push_back(Frame{&rama.children, 0});
            }
            break;
        }

        case CommandOutcome::EndEvent:
            // DEC-30 — Cambiar de mapa termina el evento local. Lo que
            // quede detrás no se ejecuta, y eso es correcto: el autor lo
            // verá como inalcanzable en el validador (E-07).
            finish(ejecucion, EventRunState::Finished);
            break;

        case CommandOutcome::Failed:
            finish(ejecucion, EventRunState::Failed,
                   resultado.diagnostic.empty()
                       ? "el comando '" + comando.type + "' falló sin decir por qué"
                       : resultado.diagnostic);
            break;
    }
}

void EventInterpreter::finish(Execution& ejecucion, EventRunState state, std::string diagnostic) {
    ejecucion.run.state = state;
    ejecucion.run.diagnostic = std::move(diagnostic);
    ejecucion.stack.clear();
    ejecucion.waitingOnHost = false;
    ejecucion.waitCommand = nullptr;
    ejecucion.waitRemainingMs = 0;
}

bool EventInterpreter::cancel(const std::string& eventId) {
    for (auto& puntero : m_executions) {
        Execution& ejecucion = *puntero;
        if (ejecucion.run.eventId != eventId) {
            continue;
        }
        if (ejecucion.run.state == EventRunState::Finished ||
            ejecucion.run.state == EventRunState::Cancelled ||
            ejecucion.run.state == EventRunState::Failed) {
            continue;
        }
        // Cancelar no deshace: lo que el evento ya hizo, hecho está. Solo
        // impide que siga.
        finish(ejecucion, EventRunState::Cancelled);
        for (EventRun& run : m_runs) {
            if (run.eventId == eventId) {
                run.state = EventRunState::Cancelled;
            }
        }
        return true;
    }
    return false;
}

void EventInterpreter::cancelAll() {
    for (auto& puntero : m_executions) {
        Execution& ejecucion = *puntero;
        if (ejecucion.run.state == EventRunState::Ready ||
            ejecucion.run.state == EventRunState::Running ||
            ejecucion.run.state == EventRunState::Waiting) {
            finish(ejecucion, EventRunState::Cancelled);
        }
    }
    m_runs.clear();
    for (const auto& puntero : m_executions) {
        const Execution& ejecucion = *puntero;
        m_runs.push_back(ejecucion.run);
    }
}

bool EventInterpreter::isRunning(const std::string& eventId) const {
    for (const auto& puntero : m_executions) {
        const Execution& ejecucion = *puntero;
        if (ejecucion.run.eventId == eventId &&
            (ejecucion.run.state == EventRunState::Ready ||
             ejecucion.run.state == EventRunState::Running ||
             ejecucion.run.state == EventRunState::Waiting)) {
            return true;
        }
    }
    return false;
}

std::vector<EventRun> EventInterpreter::active() const {
    std::vector<EventRun> salida;
    for (const auto& puntero : m_executions) {
        const Execution& ejecucion = *puntero;
        if (ejecucion.run.state == EventRunState::Ready ||
            ejecucion.run.state == EventRunState::Running ||
            ejecucion.run.state == EventRunState::Waiting) {
            salida.push_back(ejecucion.run);
        }
    }
    return salida;
}

void EventInterpreter::collectFinished() {
    m_executions.erase(std::remove_if(m_executions.begin(), m_executions.end(),
                                      [](const std::unique_ptr<Execution>& e) {
                                          return e->run.state == EventRunState::Finished ||
                                                 e->run.state == EventRunState::Cancelled ||
                                                 e->run.state == EventRunState::Failed;
                                      }),
                       m_executions.end());
    m_runs.clear();
    for (const auto& puntero : m_executions) {
        const Execution& ejecucion = *puntero;
        m_runs.push_back(ejecucion.run);
    }
}

bool EventInterpreter::blocksPlayer() const {
    for (const auto& puntero : m_executions) {
        const Execution& ejecucion = *puntero;
        if (ejecucion.run.parallel) {
            continue;
        }
        if (ejecucion.run.state == EventRunState::Ready ||
            ejecucion.run.state == EventRunState::Running ||
            ejecucion.run.state == EventRunState::Waiting) {
            return true;
        }
    }
    return false;
}

}  // namespace RPG
