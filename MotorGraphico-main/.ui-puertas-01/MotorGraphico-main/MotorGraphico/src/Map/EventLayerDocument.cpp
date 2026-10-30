#include "Map/EventLayerDocument.h"

#include <algorithm>
#include <string>
#include <utility>

// E-01 — Implementacion de la capa de eventos.
//
// NOTA DE UBICACION. Las otras tres capas documento (tiles, entidades,
// regiones) tienen su implementacion dentro de src/Map/MapDocument.cpp.
// Esta va en su propio fichero A PROPOSITO: el repositorio lo escriben
// varios agentes a la vez y MapDocument.cpp es un fichero compartido por
// C-12, C-13, E-01 y F-03 (ver planificacion/MAPA_DEL_CODIGO.md).
// Anadirle trescientas lineas mientras otro trabaja ahi es como se
// pierden mananas enteras. Si algun dia el proyecto vuelve a tener un
// solo autor, mover esto a MapDocument.cpp es un corta y pega.

namespace {

// Profundidad de un arbol de comandos, contando 1 el nivel de arriba.
// Iterativa a proposito: validar es lo que protege contra un documento
// corrupto, y una validacion recursiva sobre un documento con diez mil
// niveles se lleva la pila por delante antes de poder rechazarlo.
std::size_t Profundidad(const std::vector<MapEventCommand>& comandos, std::size_t tope) {
    std::size_t maxima = 0;
    std::vector<std::pair<const std::vector<MapEventCommand>*, std::size_t>> pendientes;
    pendientes.push_back({&comandos, 1});
    while (!pendientes.empty()) {
        const auto actual = pendientes.back();
        pendientes.pop_back();
        if (actual.first->empty()) {
            continue;
        }
        maxima = std::max(maxima, actual.second);
        // En cuanto se pasa del tope no hace falta seguir bajando: quien
        // llama solo necesita saber que se ha pasado.
        if (actual.second > tope) {
            return actual.second;
        }
        for (const MapEventCommand& comando : *actual.first) {
            pendientes.push_back({&comando.children, actual.second + 1});
        }
    }
    return maxima;
}

Result<bool> ValidarComandos(const std::vector<MapEventCommand>& comandos,
                             const std::string& donde) {
    std::vector<const std::vector<MapEventCommand>*> pendientes{&comandos};
    while (!pendientes.empty()) {
        const std::vector<MapEventCommand>* lista = pendientes.back();
        pendientes.pop_back();
        for (const MapEventCommand& comando : *lista) {
            // Un comando sin tipo no es un comando desconocido —eso se
            // conserva tal cual, ver la nota del header—: es un comando
            // que no se puede ni nombrar en un diagnostico.
            if (comando.type.empty()) {
                return Result<bool>::Error(donde + ": hay un comando sin tipo");
            }
            if (comando.version < 1) {
                return Result<bool>::Error(donde + ": el comando '" + comando.type +
                                           "' tiene una version invalida");
            }
            pendientes.push_back(&comando.children);
        }
    }
    return Result<bool>::Ok(true);
}

Result<bool> ValidarPagina(const MapEventPage& pagina, const std::string& donde) {
    if (pagina.id.empty()) {
        return Result<bool>::Error(donde + ": hay una pagina sin id");
    }
    const std::string dondePagina = donde + ", pagina '" + pagina.id.value() + "'";
    for (const MapEventCondition& condicion : pagina.conditions) {
        // Una condicion que no nombra nada se evaluaria siempre igual y
        // el autor no tendria forma de saber por que su cofre no se abre.
        if (condicion.reference.empty()) {
            return Result<bool>::Error(dondePagina + ": hay una condicion que no nombra nada");
        }
        if (condicion.comparison.empty()) {
            return Result<bool>::Error(dondePagina + ": la condicion sobre '" +
                                       condicion.reference + "' no tiene comparacion");
        }
    }
    const Result<bool> comandos = ValidarComandos(pagina.commands, dondePagina);
    if (!comandos.isOk()) {
        return comandos;
    }
    const std::size_t profundidad =
        Profundidad(pagina.commands, EventLayerDocument::kMaxCommandDepth);
    if (profundidad > EventLayerDocument::kMaxCommandDepth) {
        return Result<bool>::Error(dondePagina + ": los comandos anidan " +
                                   std::to_string(profundidad) + " niveles y el maximo es " +
                                   std::to_string(EventLayerDocument::kMaxCommandDepth));
    }
    return Result<bool>::Ok(true);
}

Result<bool> ValidarEvento(const MapEventDocument& evento) {
    if (evento.id.empty()) {
        return Result<bool>::Error("un evento de mapa no puede tener un id vacio");
    }
    const std::string donde = "el evento '" + evento.id.value() + "'";
    // Cero paginas es legal: el autor crea el evento y lo rellena
    // despues, y el diseno de bloques (§9) permite guardar borradores
    // incompletos. Lo que no es legal es que dos paginas se llamen igual.
    for (std::size_t i = 0; i < evento.pages.size(); ++i) {
        const Result<bool> pagina = ValidarPagina(evento.pages[i], donde);
        if (!pagina.isOk()) {
            return pagina;
        }
        for (std::size_t j = i + 1; j < evento.pages.size(); ++j) {
            if (evento.pages[i].id == evento.pages[j].id) {
                return Result<bool>::Error(donde + ": dos paginas comparten el id '" +
                                           evento.pages[i].id.value() + "'");
            }
        }
    }
    return Result<bool>::Ok(true);
}

}  // namespace

EventLayerDocument::EventLayerDocument(std::vector<MapEventDocument> events)
    : m_events(std::move(events)) {}

const MapEventDocument* EventLayerDocument::find(const ids::EventId& id) const {
    for (const MapEventDocument& evento : m_events) {
        if (evento.id == id) {
            return &evento;
        }
    }
    return nullptr;
}

const MapEventDocument* EventLayerDocument::findAt(GridCoord position) const {
    for (const MapEventDocument& evento : m_events) {
        if (evento.position.x == position.x && evento.position.y == position.y) {
            return &evento;
        }
    }
    return nullptr;
}

Result<bool> EventLayerDocument::addEvent(MapEventDocument event) {
    const Result<bool> valido = ValidarEvento(event);
    if (!valido.isOk()) {
        return valido;
    }
    if (find(event.id) != nullptr) {
        return Result<bool>::Error("ya existe un evento con el id '" + event.id.value() + "'");
    }
    // Una casilla, un evento. Es la misma regla que el editor lleva
    // aplicando a los objetos desde siempre, y sin ella "el evento de
    // esta casilla" dejaria de ser una frase con sentido.
    const MapEventDocument* ocupante = findAt(event.position);
    if (ocupante != nullptr) {
        return Result<bool>::Error("la casilla ya la ocupa el evento '" + ocupante->id.value() +
                                   "'");
    }
    m_events.push_back(std::move(event));
    return Result<bool>::Ok(true);
}

Result<bool> EventLayerDocument::removeEvent(const ids::EventId& id) {
    const auto encontrado = std::find_if(m_events.begin(), m_events.end(),
                                         [&id](const MapEventDocument& e) { return e.id == id; });
    if (encontrado == m_events.end()) {
        return Result<bool>::Error("no existe el evento '" + id.value() + "'");
    }
    m_events.erase(encontrado);
    return Result<bool>::Ok(true);
}

Result<bool> EventLayerDocument::setEventPosition(const ids::EventId& id, GridCoord position) {
    const auto encontrado = std::find_if(m_events.begin(), m_events.end(),
                                         [&id](const MapEventDocument& e) { return e.id == id; });
    if (encontrado == m_events.end()) {
        return Result<bool>::Error("no existe el evento '" + id.value() + "'");
    }
    const MapEventDocument* ocupante = findAt(position);
    if (ocupante != nullptr && ocupante->id != id) {
        return Result<bool>::Error("la casilla ya la ocupa el evento '" + ocupante->id.value() +
                                   "'");
    }
    encontrado->position = position;
    return Result<bool>::Ok(true);
}

Result<bool> EventLayerDocument::setEventPages(const ids::EventId& id,
                                               std::vector<MapEventPage> pages) {
    const auto encontrado = std::find_if(m_events.begin(), m_events.end(),
                                         [&id](const MapEventDocument& e) { return e.id == id; });
    if (encontrado == m_events.end()) {
        return Result<bool>::Error("no existe el evento '" + id.value() + "'");
    }
    // Se valida ANTES de instalar: un juego de paginas invalido deja el
    // evento exactamente como estaba, no a medio cambiar.
    MapEventDocument candidato = *encontrado;
    candidato.pages = std::move(pages);
    const Result<bool> valido = ValidarEvento(candidato);
    if (!valido.isOk()) {
        return valido;
    }
    *encontrado = std::move(candidato);
    return Result<bool>::Ok(true);
}

Result<bool> EventLayerDocument::validate() const {
    for (std::size_t i = 0; i < m_events.size(); ++i) {
        const Result<bool> evento = ValidarEvento(m_events[i]);
        if (!evento.isOk()) {
            return evento;
        }
        for (std::size_t j = i + 1; j < m_events.size(); ++j) {
            if (m_events[i].id == m_events[j].id) {
                return Result<bool>::Error("dos eventos comparten el id '" +
                                           m_events[i].id.value() + "'");
            }
            if (m_events[i].position.x == m_events[j].position.x &&
                m_events[i].position.y == m_events[j].position.y) {
                return Result<bool>::Error("los eventos '" + m_events[i].id.value() + "' y '" +
                                           m_events[j].id.value() + "' ocupan la misma casilla");
            }
        }
    }
    return Result<bool>::Ok(true);
}
