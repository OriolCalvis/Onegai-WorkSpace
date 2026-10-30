// H0-08 (Fase H) — La capa de disparadores. El contrato y las decisiones
// (incluidas las provisionales pendientes de DEC-10) están escritos en
// include/RPG/MapEventRunner.h; aquí solo está el cómo.

#include "RPG/MapEventRunner.h"

#include <cstddef>
#include <variant>

// Por ActivePageWithInventory y nada más: esta pieza no construye ningún
// anfitrión ni toca GameSession.
#include "RPG/SessionCommandHost.h"

namespace RPG {
namespace {

// Casilla propia y las cuatro contiguas, en el orden de
// GameSession::interact() (src/Game/GameSessionInteraccion.cpp:91-92):
// primero los pies, porque una puerta se pisa y a un PNJ se le habla de
// frente. Duplicado a propósito; unificarlo exige abrir GameSession.cpp,
// que comparten seis paquetes.
constexpr int kDesplazamientos[][2] = {{0, 0}, {0, -1}, {0, 1}, {-1, 0}, {1, 0}};

const EventLayerDocument* capaDeEventos(const MapLayer& capa) {
    return std::get_if<EventLayerDocument>(&capa.data());
}

bool mismaCasilla(GridCoord a, GridCoord b) { return a.x == b.x && a.y == b.y; }

}  // namespace

MapEventRunner::MapEventRunner(NarrativeState& narrative,
                               const std::vector<std::string>& inventory,
                               EventInterpreter& interpreter)
    : m_narrative(narrative), m_inventory(inventory), m_interpreter(interpreter) {}

std::vector<GridCoord> MapEventRunner::interactCandidates(GridCoord playerCell) {
    std::vector<GridCoord> candidatas;
    candidatas.reserve(sizeof(kDesplazamientos) / sizeof(kDesplazamientos[0]));
    for (const auto& d : kDesplazamientos) {
        candidatas.push_back(GridCoord{playerCell.x + d[0], playerCell.y + d[1]});
    }
    return candidatas;
}

bool MapEventRunner::blocksPlayer() const { return m_interpreter.blocksPlayer(); }

void MapEventRunner::resetTriggerState() {
    m_autoStarted.clear();
    m_hasLastCell = false;
    m_lastCell = GridCoord{0, 0};
    m_lastStarted.clear();
    m_diagnostic.clear();
}

const MapEventDocument* MapEventRunner::eventAt(GridCoord cell) const {
    if (m_map == nullptr) {
        return nullptr;
    }
    // El primer evento encontrado gana: una casilla, un evento
    // (EventLayerDocument::findAt), y si hubiera varias capas de eventos la
    // de arriba es la que se ve, igual que con las de tiles.
    for (const MapLayer& capa : m_map->layers()) {
        const EventLayerDocument* eventos = capaDeEventos(capa);
        if (eventos == nullptr) {
            continue;
        }
        const MapEventDocument* encontrado = eventos->findAt(cell);
        if (encontrado != nullptr) {
            return encontrado;
        }
    }
    return nullptr;
}

// Una página que no se pudo evaluar NO es una página falsa: se dice y no se
// arranca. Es la regla de E-04/E-09 y es la diferencia entre «no tiene la
// llave» y «no sé mirar el inventario desde aquí».
bool MapEventRunner::paginaActiva(const MapEventDocument& event, const MapEventPage** pagina,
                                  std::size_t* index) {
    *pagina = nullptr;
    *index = 0;
    const ActivePageResult activa = ActivePageWithInventory(event, m_narrative, m_inventory);
    if (!activa.trustworthy()) {
        m_diagnostic = "evento '" + event.id.value() + "': página no evaluable — " +
                       activa.diagnostic;
        return false;
    }
    if (activa.page == nullptr) {
        // Legítimo y silencioso: un cofre ya vaciado puede no tener página
        // activa. No es un fallo y no debe tapar el diagnóstico de otro.
        return false;
    }
    *pagina = activa.page;
    *index = activa.index;
    return true;
}

bool MapEventRunner::startPage(const MapEventDocument& event, std::size_t pageIndex) {
    const Result<bool> arranque = m_interpreter.start(event, pageIndex);
    if (!arranque.isOk()) {
        m_diagnostic = arranque.errorMessage();
        return false;
    }
    m_lastStarted = event.id.value();
    m_diagnostic.clear();
    return true;
}

std::size_t MapEventRunner::paginaAutomaticaYaArrancada(const std::string& eventId) const {
    for (const auto& entrada : m_autoStarted) {
        if (entrada.first == eventId) {
            return entrada.second;
        }
    }
    return 0;
}

void MapEventRunner::registrarAutomatica(const std::string& eventId, std::size_t pageIndex) {
    // índice + 1, con 0 = ninguna: así la página 0 se distingue de la
    // ausencia de entrada sin un map de opcionales.
    const std::size_t marca = pageIndex + 1;
    for (auto& entrada : m_autoStarted) {
        if (entrada.first == eventId) {
            entrada.second = marca;
            return;
        }
    }
    m_autoStarted.emplace_back(eventId, marca);
}

void MapEventRunner::dispararAutomaticos() {
    for (const MapLayer& capa : m_map->layers()) {
        const EventLayerDocument* eventos = capaDeEventos(capa);
        if (eventos == nullptr) {
            continue;
        }
        for (const MapEventDocument& event : eventos->events()) {
            const MapEventPage* pagina = nullptr;
            std::size_t index = 0;
            if (!paginaActiva(event, &pagina, &index)) {
                continue;
            }
            const bool automatico = pagina->trigger == EventTrigger::Autorun;
            const bool paralelo = pagina->trigger == EventTrigger::Parallel;
            if (!automatico && !paralelo) {
                continue;
            }
            if (m_interpreter.isRunning(event.id.value())) {
                continue;
            }
            // Una vez por página activa distinta: ver la cabecera. Sin esto,
            // un Autorun cuya página no cambia se rearma cada frame.
            if (paginaAutomaticaYaArrancada(event.id.value()) == index + 1) {
                continue;
            }
            if (startPage(event, index)) {
                registrarAutomatica(event.id.value(), index);
                if (automatico) {
                    // Ya hay algo que bloquea al jugador: nada más arranca
                    // en este frame. Los paralelos no cortan el recorrido.
                    return;
                }
            }
        }
    }
}

void MapEventRunner::dispararTouch(const MapEventFrame& frame) {
    // Solo al ENTRAR en la casilla. Sin esto, cada frame de pie sobre una
    // placa de presión volvería a arrancar el evento.
    if (m_hasLastCell && mismaCasilla(m_lastCell, frame.playerCell)) {
        return;
    }
    const MapEventDocument* event = eventAt(frame.playerCell);
    if (event == nullptr) {
        return;
    }
    const MapEventPage* pagina = nullptr;
    std::size_t index = 0;
    if (!paginaActiva(*event, &pagina, &index)) {
        return;
    }
    if (pagina->trigger != EventTrigger::Touch) {
        return;
    }
    if (m_interpreter.isRunning(event->id.value())) {
        return;
    }
    startPage(*event, index);
}

void MapEventRunner::dispararInteract(const MapEventFrame& frame) {
    for (const GridCoord& celda : interactCandidates(frame.playerCell)) {
        const MapEventDocument* event = eventAt(celda);
        if (event == nullptr) {
            continue;
        }
        const MapEventPage* pagina = nullptr;
        std::size_t index = 0;
        if (!paginaActiva(*event, &pagina, &index)) {
            // Una condición no evaluable en la casilla que el jugador está
            // mirando es exactamente el caso que hay que decir en voz alta:
            // acaba de pulsar acción y no ha pasado nada.
            return;
        }
        if (pagina->trigger != EventTrigger::Interact) {
            continue;
        }
        if (m_interpreter.isRunning(event->id.value())) {
            m_diagnostic = "evento '" + event->id.value() +
                           "': ya se está ejecutando, esta pulsación no arranca otra copia";
            return;
        }
        if (startPage(*event, index)) {
            return;
        }
    }
}

void MapEventRunner::update(const MapEventFrame& frame) {
    // Las ejecuciones terminadas en el frame anterior se limpian AHORA, no
    // cuando terminan: quien llama ha tenido el frame entero para leer su
    // desenlace en interpreter.runs(), y sin esta limpieza una partida
    // larga acumularía una ejecución —con su copia del evento— por cada
    // interacción desde el principio.
    m_interpreter.collectFinished();
    m_lastStarted.clear();

    if (m_map != nullptr) {
        dispararAutomaticos();
        // Un evento que bloquea al jugador cierra la puerta a los demás
        // disparadores de este frame: una escena no se interrumpe con otra
        // encima. Se vuelve a mirar después de cada etapa porque arrancar
        // un Autorun cambia la respuesta.
        if (!m_interpreter.blocksPlayer()) {
            dispararTouch(frame);
            if (frame.interactPressed && !m_interpreter.blocksPlayer()) {
                dispararInteract(frame);
            }
        }
    }

    // La casilla se registra SIEMPRE, también cuando algo bloquea y el
    // jugador no se ha movido. Si no, al terminar la escena el runner
    // creería que el jugador acaba de entrar en la casilla donde llevaba
    // diez segundos quieto y dispararía el Touch otra vez.
    m_lastCell = frame.playerCell;
    m_hasLastCell = true;

    m_interpreter.tick(frame.deltaMs);
}

}  // namespace RPG
