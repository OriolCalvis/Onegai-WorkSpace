// La API de edicion del documento: trece operaciones que delegan en su capa.
//
// Todas pasan por mutateLayer, que es la garantia de C-12: no se toca
// una capa a espaldas del documento. Estan juntas y aparte porque son
// la superficie que usa el editor, y crece cada vez que aparece un tipo
// de capa nuevo — sin arrastrar consigo la gestion de capas ni validate.
//
// Salio de src/Map/MapDocument.cpp el 23/09/2026 (EST-27): aquel
// fichero tenia 724 lineas y cuatro responsabilidades dentro. Se
// movio, no se reescribio. Los paquetes que mandan siguen siendo
// C-12 y C-13; ver planificacion/PLAN_ESTABILIZACION.md.

#include "Map/MapDocument.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <type_traits>
#include <utility>

Result<bool> MapDocument::setTile(const ids::LayerId& layerId, GridCoord cell, int gid) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        TileLayerDocument* tiles = std::get_if<TileLayerDocument>(&layer.mutableData());
        if (tiles == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de tiles");
        }
        return tiles->setGid(cell.x, cell.y, gid);
    });
}

Result<bool> MapDocument::fillTiles(const ids::LayerId& layerId, GridBounds area, int gid) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        TileLayerDocument* tiles = std::get_if<TileLayerDocument>(&layer.mutableData());
        if (tiles == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de tiles");
        }
        return tiles->fill(area.minX, area.minY, area.maxX, area.maxY, gid);
    });
}

// C-14 (2a rebanada). Reutiliza mutateLayer, que es quien hace la copia,
// la revalidacion y la instalacion: por eso todo el lote cuesta UNA de
// cada, y por eso el contrato de todo-o-nada es el mismo que el de
// setTile() sin tener que repetirlo aqui.
Result<bool> MapDocument::editTiles(const ids::LayerId& layerId,
                                    const std::function<Result<bool>(TileLayerDocument&)>& edit) {
    // Una funcion vacia no es "no hacer nada": es una llamada mal
    // construida, y tragarsela dejaria al llamador creyendo que su lote
    // se aplico.
    if (!edit) {
        return Result<bool>::Error("editTiles: falta la funcion de edicion");
    }
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        TileLayerDocument* tiles = std::get_if<TileLayerDocument>(&layer.mutableData());
        if (tiles == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de tiles");
        }
        return edit(*tiles);
    });
}

Result<bool> MapDocument::addEntity(const ids::LayerId& layerId, MapEntityDocument entity) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        EntityLayerDocument* entities = std::get_if<EntityLayerDocument>(&layer.mutableData());
        if (entities == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de entidades");
        }
        return entities->addEntity(std::move(entity));
    });
}

Result<bool> MapDocument::removeEntity(const ids::LayerId& layerId,
                                       const ids::EntityId& entityId) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        EntityLayerDocument* entities = std::get_if<EntityLayerDocument>(&layer.mutableData());
        if (entities == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de entidades");
        }
        return entities->removeEntity(entityId);
    });
}

Result<bool> MapDocument::moveEntity(const ids::LayerId& layerId, const ids::EntityId& entityId,
                                     GridCoord position) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        EntityLayerDocument* entities = std::get_if<EntityLayerDocument>(&layer.mutableData());
        if (entities == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de entidades");
        }
        return entities->setEntityPosition(entityId, position);
    });
}

Result<bool> MapDocument::addRegion(const ids::LayerId& layerId, MapRegionDocument region) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        RegionLayerDocument* regions = std::get_if<RegionLayerDocument>(&layer.mutableData());
        if (regions == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de regiones");
        }
        return regions->addRegion(std::move(region));
    });
}

Result<bool> MapDocument::removeRegion(const ids::LayerId& layerId,
                                       const ids::RegionId& regionId) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        RegionLayerDocument* regions = std::get_if<RegionLayerDocument>(&layer.mutableData());
        if (regions == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de regiones");
        }
        return regions->removeRegion(regionId);
    });
}

Result<bool> MapDocument::setRegionBounds(const ids::LayerId& layerId,
                                          const ids::RegionId& regionId, GridBounds bounds) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        RegionLayerDocument* regions = std::get_if<RegionLayerDocument>(&layer.mutableData());
        if (regions == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de regiones");
        }
        return regions->setRegionBounds(regionId, bounds);
    });
}

// =====================================================================
// E-01 — Eventos
//
// Las cuatro operaciones siguen el mismo camino que las de entidades:
// mutateLayer copia la capa, deja que la mutacion la toque, revalida
// contra el mapa e instala. Lo unico propio de aqui es sacar el
// EventLayerDocument del variant y decir que la capa no es de eventos
// cuando no lo es.
//
// El get_if va repetido en las cuatro y no en un ayudante suelto porque
// mutableData() es privado y solo MapDocument es su friend: esa es la
// garantia de C-12 —no se puede tocar una capa a espaldas del documento—
// y sacar el acceso a una funcion libre la rompería.
// =====================================================================

Result<bool> MapDocument::addEvent(const ids::LayerId& layerId, MapEventDocument event) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        EventLayerDocument* events = std::get_if<EventLayerDocument>(&layer.mutableData());
        if (events == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de eventos");
        }
        return events->addEvent(std::move(event));
    });
}

Result<bool> MapDocument::removeEvent(const ids::LayerId& layerId, const ids::EventId& eventId) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        EventLayerDocument* events = std::get_if<EventLayerDocument>(&layer.mutableData());
        if (events == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de eventos");
        }
        return events->removeEvent(eventId);
    });
}

Result<bool> MapDocument::moveEvent(const ids::LayerId& layerId, const ids::EventId& eventId,
                                    GridCoord position) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        EventLayerDocument* events = std::get_if<EventLayerDocument>(&layer.mutableData());
        if (events == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de eventos");
        }
        return events->setEventPosition(eventId, position);
    });
}

Result<bool> MapDocument::setEventPages(const ids::LayerId& layerId, const ids::EventId& eventId,
                                        std::vector<MapEventPage> pages) {
    return mutateLayer(layerId, [&](MapLayer& layer) -> Result<bool> {
        EventLayerDocument* events = std::get_if<EventLayerDocument>(&layer.mutableData());
        if (events == nullptr) {
            return Result<bool>::Error("la capa '" + layerId.value() + "' no es de eventos");
        }
        return events->setEventPages(eventId, std::move(pages));
    });
}
