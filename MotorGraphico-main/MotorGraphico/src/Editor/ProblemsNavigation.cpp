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


// Ir ahi: de un problema a la celda, la entidad o el comando que lo causa.
//
// Es la mitad que sabe RECORRER un documento: bajar por las paginas de
// un evento, seguir una ruta de comandos anidados y decir en texto
// donde esta. Define ademas las dos piezas que el estado comparte.
//
// Salio de src/Editor/ProblemsState.cpp el 24/09/2026 (EST-31): aquel
// fichero tenia 643 lineas y tres trabajos dentro. Se movio, no se
// reescribio. El porque de cada decision sigue en el header.

#include "Editor/ProblemsState.h"

#include <algorithm>
#include <functional>
#include <set>

#include "Map/EventLayerDocument.h"
#include "RPG/EventCommand.h"

#include "Editor/ProblemsInternal.h"

namespace Editor {

namespace internal {

std::vector<const MapEventDocument*> EventosDe(const MapDocument& document) {
    std::vector<const MapEventDocument*> eventos;
    for (const MapLayer& capa : document.layers()) {
        const EventLayerDocument* capaEventos = std::get_if<EventLayerDocument>(&capa.data());
        if (capaEventos == nullptr) {
            continue;
        }
        for (const MapEventDocument& evento : capaEventos->events()) {
            eventos.push_back(&evento);
        }
    }
    return eventos;
}


const MapEventDocument* BuscarEvento(const MapDocument& document, const std::string& eventId) {
    for (const MapEventDocument* evento : EventosDe(document)) {
        if (evento->id.value() == eventId) {
            return evento;
        }
    }
    return nullptr;
}


const MapEventCommand* SeguirRuta(const std::vector<MapEventCommand>& raiz,
                                  const std::vector<std::size_t>& ruta) {
    if (ruta.empty()) {
        return nullptr;
    }
    const std::vector<MapEventCommand>* lista = &raiz;
    const MapEventCommand* comando = nullptr;
    for (std::size_t i = 0; i < ruta.size(); ++i) {
        if (i % 2 == 0) {
            if (lista == nullptr || ruta[i] >= lista->size()) {
                return nullptr;
            }
            comando = &(*lista)[ruta[i]];
            lista = nullptr;
        } else {
            if (comando == nullptr || ruta[i] >= comando->children.size()) {
                return nullptr;
            }
            lista = &comando->children[ruta[i]].children;
            comando = nullptr;
        }
    }
    // Una ruta que acaba en una rama y no en un comando está mal formada:
    // no señala nada seleccionable.
    return comando;
}

// --- Referencias -------------------------------------------------------


}  // namespace internal

namespace {

std::string RutaComoTexto(const std::vector<std::size_t>& ruta) {
    std::string texto;
    for (std::size_t i = 0; i < ruta.size(); ++i) {
        if (i > 0) {
            texto += '.';
        }
        texto += std::to_string(ruta[i]);
    }
    return texto;
}

// Camina una ruta de comandos. Las posiciones pares son índices de
// comando dentro de una lista; las impares, índices de RAMA del comando
// anterior (E-01 guarda cada rama como un hijo cuyos `children` son los
// comandos de esa rama; E-07 construye las rutas así). Devuelve nullptr
// en cuanto un tramo se sale de rango.

}  // namespace

using internal::BuscarEvento;
using internal::EventosDe;
using internal::SeguirRuta;

// ---------------------------------------------------------------------
// Claves
// ---------------------------------------------------------------------

std::string ProblemLocation::key() const {
    std::string clave = mapId;
    clave += '|';
    clave += eventId;
    clave += '|';
    if (hasPage) {
        clave += pageId;
    }
    clave += '|';
    if (hasCommand) {
        clave += RutaComoTexto(commandPath);
        clave += '#';
        clave += commandType;
    }
    return clave;
}

std::string ProblemEntry::key() const {
    return where.key() + "|" + message;
}

// ---------------------------------------------------------------------
// Ir ahí
// ---------------------------------------------------------------------

NavigationRequest ResolveNavigation(const MapDocument& document, const ProblemLocation& where) {
    NavigationRequest peticion;

    if (where.eventId.empty()) {
        peticion.outcome = NavigationOutcome::NoTarget;
        peticion.note = "este problema no señala ningún evento";
        return peticion;
    }
    if (!where.mapId.empty() && where.mapId != document.id().value()) {
        peticion.outcome = NavigationOutcome::MapGone;
        peticion.note = "el problema está en el mapa '" + where.mapId + "', no en este";
        return peticion;
    }

    const MapEventDocument* evento = BuscarEvento(document, where.eventId);
    if (evento == nullptr) {
        // Aquí está la trampa que E-15 tiene que evitar: NO se navega a
        // «el evento que haya en esa casilla». El evento se identifica
        // por id y, si no está, no hay destino.
        peticion.outcome = NavigationOutcome::EventGone;
        peticion.note = "el evento '" + where.eventId + "' ya no existe";
        return peticion;
    }

    peticion.target.mapId = document.id().value();
    peticion.target.eventId = evento->id.value();

    if (!where.hasPage) {
        peticion.outcome = NavigationOutcome::Ok;
        return peticion;
    }

    // La página se busca por id, no por índice: borrar la página 1 mueve
    // la 3 al hueco de la 2, y navegar «a la página 3» abriría otra.
    std::size_t indiceActual = 0;
    bool encontrada = false;
    for (std::size_t p = 0; p < evento->pages.size(); ++p) {
        if (evento->pages[p].id.value() == where.pageId) {
            indiceActual = p;
            encontrada = true;
            break;
        }
    }
    if (!encontrada) {
        peticion.outcome = NavigationOutcome::PageGone;
        peticion.note = "esa página del evento '" + evento->name + "' ya no existe";
        return peticion;
    }

    peticion.target.hasPage = true;
    peticion.target.pageId = where.pageId;
    peticion.target.pageIndex = indiceActual;

    if (!where.hasCommand) {
        peticion.outcome = NavigationOutcome::Ok;
        return peticion;
    }

    const MapEventCommand* comando =
        SeguirRuta(evento->pages[indiceActual].commands, where.commandPath);
    // Dos motivos para no seleccionar nada: que la ruta se salga, o que
    // lleve a un comando distinto del que el diagnóstico miraba. El
    // segundo es el caso silencioso: borrar el comando 2 deja otro en el
    // índice 2, y seleccionarlo sería señalar al inocente.
    if (comando == nullptr ||
        (!where.commandType.empty() && comando->type != where.commandType)) {
        peticion.outcome = NavigationOutcome::CommandGone;
        peticion.note = "el comando que señalaba este problema ya no está donde estaba";
        return peticion;
    }

    peticion.target.hasCommand = true;
    peticion.target.commandPath = where.commandPath;
    peticion.target.commandType = comando->type;
    peticion.outcome = NavigationOutcome::Ok;
    return peticion;
}

// ---------------------------------------------------------------------
// La lista
// ---------------------------------------------------------------------

void ProblemsState::refreshMap(const MapDocument& document) {
    setReport(document.id().value(), RPG::ValidateEvents(document), &document);
}

}  // namespace Editor
