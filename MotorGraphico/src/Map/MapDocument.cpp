#include "Map/MapDocument.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <type_traits>
#include <utility>

TileLayerDocument::TileLayerDocument(int width, int height, std::vector<int> gids)
    : m_width(width), m_height(height), m_gids(std::move(gids)) {}

Result<bool> TileLayerDocument::validate() const {
    if (m_width <= 0 || m_height <= 0) {
        return Result<bool>::Error("una capa de tiles debe tener dimensiones positivas");
    }

    const std::size_t expectedCells = static_cast<std::size_t>(m_width) *
                                      static_cast<std::size_t>(m_height);
    if (m_gids.size() != expectedCells) {
        return Result<bool>::Error("el número de GIDs no coincide con las dimensiones de la capa");
    }
    if (std::any_of(m_gids.begin(), m_gids.end(), [](int gid) { return gid < 0; })) {
        return Result<bool>::Error("un GID de tile no puede ser negativo");
    }
    return Result<bool>::Ok(true);
}

// --- C-12: mutacion de una capa de tiles ---

int TileLayerDocument::gidAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return 0;  // fuera de la cuadricula se lee como celda vacia, no revienta
    }
    return m_gids[static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) +
                  static_cast<std::size_t>(x)];
}

Result<bool> TileLayerDocument::setGid(int x, int y, int gid) {
    if (m_width <= 0 || m_height <= 0) {
        return Result<bool>::Error("no se puede pintar en una capa sin dimensiones");
    }
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return Result<bool>::Error("la celda (" + std::to_string(x) + "," + std::to_string(y) +
                                   ") esta fuera de la capa");
    }
    if (gid < 0) {
        return Result<bool>::Error("un GID de tile no puede ser negativo");
    }
    m_gids[static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) +
           static_cast<std::size_t>(x)] = gid;
    return Result<bool>::Ok(true);
}

Result<bool> TileLayerDocument::fill(int minX, int minY, int maxX, int maxY, int gid) {
    if (maxX < minX || maxY < minY) {
        return Result<bool>::Ok(true);  // area vacia: no pinta nada, y no es un error
    }
    if (gid < 0) {
        return Result<bool>::Error("un GID de tile no puede ser negativo");
    }
    // Se comprueban las esquinas ANTES de pintar: un rectangulo que se
    // sale a medias dejaria media capa pintada y media no.
    if (minX < 0 || minY < 0 || maxX >= m_width || maxY >= m_height) {
        return Result<bool>::Error("el area a rellenar se sale de la capa");
    }
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            m_gids[static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) +
                   static_cast<std::size_t>(x)] = gid;
        }
    }
    return Result<bool>::Ok(true);
}

EntityLayerDocument::EntityLayerDocument(std::vector<MapEntityDocument> entities)
    : m_entities(std::move(entities)) {}

Result<bool> EntityLayerDocument::validate() const {
    std::set<ids::EntityId> entityIds;
    for (const MapEntityDocument& entity : m_entities) {
        if (entity.id.empty()) {
            return Result<bool>::Error("una entidad de mapa no puede tener un id vacío");
        }
        if (entity.objectId.empty()) {
            return Result<bool>::Error("la entidad '" + entity.id.value() +
                                       "' no referencia un objeto");
        }
        if (!entityIds.insert(entity.id).second) {
            return Result<bool>::Error("id de entidad duplicado: '" + entity.id.value() + "'");
        }

        // --- C-13: los campos nuevos ---
        //
        // Se valida lo que es INCOHERENTE, no lo que está vacío: casi
        // todos estos campos son opcionales por diseño (una entidad sin
        // botín ni evento ni variante es lo normal). Lo que no puede
        // pasar es una caja de patrulla del revés, una escala que haga
        // desaparecer el sprite o una propiedad sin nombre.
        if (entity.patrolMin.x > entity.patrolMax.x ||
            entity.patrolMin.y > entity.patrolMax.y) {
            return Result<bool>::Error("la entidad '" + entity.id.value() +
                                       "' tiene una caja de patrulla invertida");
        }
        if (!std::isfinite(entity.scale) || entity.scale <= 0.0f) {
            return Result<bool>::Error("la entidad '" + entity.id.value() +
                                       "' tiene una escala no positiva");
        }
        for (const std::string& lootId : entity.loot) {
            if (lootId.empty()) {
                return Result<bool>::Error("la entidad '" + entity.id.value() +
                                           "' tiene una entrada de botín vacía");
            }
        }
        for (const auto& property : entity.properties) {
            if (property.first.empty()) {
                return Result<bool>::Error("la entidad '" + entity.id.value() +
                                           "' tiene una propiedad sin nombre");
            }
        }
    }
    return Result<bool>::Ok(true);
}

// --- C-12: mutacion de una capa de entidades ---

const MapEntityDocument* EntityLayerDocument::find(const ids::EntityId& id) const {
    for (const MapEntityDocument& entity : m_entities) {
        if (entity.id == id) {
            return &entity;
        }
    }
    return nullptr;
}

Result<bool> EntityLayerDocument::addEntity(MapEntityDocument entity) {
    if (entity.id.empty()) {
        return Result<bool>::Error("una entidad de mapa no puede tener un id vacio");
    }
    if (entity.objectId.empty()) {
        return Result<bool>::Error("la entidad '" + entity.id.value() +
                                   "' no referencia un objeto");
    }
    if (find(entity.id) != nullptr) {
        return Result<bool>::Error("id de entidad duplicado: '" + entity.id.value() + "'");
    }
    m_entities.push_back(std::move(entity));
    return Result<bool>::Ok(true);
}

Result<bool> EntityLayerDocument::removeEntity(const ids::EntityId& id) {
    const auto it = std::find_if(m_entities.begin(), m_entities.end(),
                                 [&id](const MapEntityDocument& e) { return e.id == id; });
    if (it == m_entities.end()) {
        return Result<bool>::Error("no hay ninguna entidad '" + id.value() + "' en la capa");
    }
    m_entities.erase(it);
    return Result<bool>::Ok(true);
}

Result<bool> EntityLayerDocument::setEntityPosition(const ids::EntityId& id, GridCoord position) {
    const auto it = std::find_if(m_entities.begin(), m_entities.end(),
                                 [&id](const MapEntityDocument& e) { return e.id == id; });
    if (it == m_entities.end()) {
        return Result<bool>::Error("no hay ninguna entidad '" + id.value() + "' en la capa");
    }
    it->position = position;
    return Result<bool>::Ok(true);
}

RegionLayerDocument::RegionLayerDocument(std::vector<MapRegionDocument> regions)
    : m_regions(std::move(regions)) {}

Result<bool> RegionLayerDocument::validate() const {
    std::set<ids::RegionId> regionIds;
    for (const MapRegionDocument& region : m_regions) {
        if (region.id.empty()) {
            return Result<bool>::Error("una región de mapa no puede tener un id vacío");
        }
        if (region.name.empty()) {
            return Result<bool>::Error("la región '" + region.id.value() +
                                       "' no puede tener un nombre vacío");
        }
        if (region.bounds.isEmpty()) {
            return Result<bool>::Error("la región '" + region.id.value() + "' tiene límites vacíos");
        }
        if (!regionIds.insert(region.id).second) {
            return Result<bool>::Error("id de región duplicado: '" + region.id.value() + "'");
        }
    }
    return Result<bool>::Ok(true);
}

// --- C-12: mutacion de una capa de regiones ---

const MapRegionDocument* RegionLayerDocument::find(const ids::RegionId& id) const {
    for (const MapRegionDocument& region : m_regions) {
        if (region.id == id) {
            return &region;
        }
    }
    return nullptr;
}

Result<bool> RegionLayerDocument::addRegion(MapRegionDocument region) {
    if (region.id.empty()) {
        return Result<bool>::Error("una region de mapa no puede tener un id vacio");
    }
    if (region.name.empty()) {
        return Result<bool>::Error("la region '" + region.id.value() +
                                   "' no puede tener un nombre vacio");
    }
    if (region.bounds.isEmpty()) {
        return Result<bool>::Error("la region '" + region.id.value() + "' tiene limites vacios");
    }
    if (find(region.id) != nullptr) {
        return Result<bool>::Error("id de region duplicado: '" + region.id.value() + "'");
    }
    m_regions.push_back(std::move(region));
    return Result<bool>::Ok(true);
}

Result<bool> RegionLayerDocument::removeRegion(const ids::RegionId& id) {
    const auto it = std::find_if(m_regions.begin(), m_regions.end(),
                                 [&id](const MapRegionDocument& r) { return r.id == id; });
    if (it == m_regions.end()) {
        return Result<bool>::Error("no hay ninguna region '" + id.value() + "' en la capa");
    }
    m_regions.erase(it);
    return Result<bool>::Ok(true);
}

Result<bool> RegionLayerDocument::setRegionBounds(const ids::RegionId& id, GridBounds bounds) {
    const auto it = std::find_if(m_regions.begin(), m_regions.end(),
                                 [&id](const MapRegionDocument& r) { return r.id == id; });
    if (it == m_regions.end()) {
        return Result<bool>::Error("no hay ninguna region '" + id.value() + "' en la capa");
    }
    if (bounds.isEmpty()) {
        return Result<bool>::Error("la region '" + id.value() + "' no puede quedar vacia");
    }
    it->bounds = bounds;
    return Result<bool>::Ok(true);
}

MapLayer::MapLayer(MapLayerMeta meta, MapLayerData data)
    : m_meta(std::move(meta)), m_data(std::move(data)) {}

MapLayer MapLayer::tiles(MapLayerMeta meta, TileLayerDocument data) {
    return MapLayer(std::move(meta), std::move(data));
}

MapLayer MapLayer::entities(MapLayerMeta meta, EntityLayerDocument data) {
    return MapLayer(std::move(meta), std::move(data));
}

MapLayer MapLayer::regions(MapLayerMeta meta, RegionLayerDocument data) {
    return MapLayer(std::move(meta), std::move(data));
}

MapLayerType MapLayer::type() const {
    if (std::holds_alternative<TileLayerDocument>(m_data)) {
        return MapLayerType::Tile;
    }
    if (std::holds_alternative<EntityLayerDocument>(m_data)) {
        return MapLayerType::Entity;
    }
    return MapLayerType::Region;
}

Result<bool> MapLayer::validate() const {
    if (m_meta.id.empty()) {
        return Result<bool>::Error("una capa de mapa no puede tener un id vacío");
    }
    if (m_meta.name.empty()) {
        return Result<bool>::Error("la capa '" + m_meta.id.value() +
                                   "' no puede tener un nombre vacío");
    }
    if (m_meta.order < 0) {
        return Result<bool>::Error("la capa '" + m_meta.id.value() + "' tiene un orden negativo");
    }
    if (!std::isfinite(m_meta.opacity) || m_meta.opacity < 0.0f || m_meta.opacity > 1.0f) {
        return Result<bool>::Error("la capa '" + m_meta.id.value() +
                                   "' tiene una opacidad fuera de [0, 1]");
    }

    return std::visit([](const auto& layerData) { return layerData.validate(); }, m_data);
}

MapDocument::MapDocument(ids::MapId id, std::string name, int width, int height, int tileWidth,
                         int tileHeight)
    : m_id(std::move(id)),
      m_name(std::move(name)),
      m_width(width),
      m_height(height),
      m_tileWidth(tileWidth),
      m_tileHeight(tileHeight) {}

Result<bool> MapDocument::validateLayerForMap(const MapLayer& layer) const {
    const Result<bool> layerResult = layer.validate();
    if (!layerResult.isOk()) {
        return layerResult;
    }

    return std::visit(
        [this](const auto& layerData) -> Result<bool> {
            using LayerData = std::decay_t<decltype(layerData)>;
            if constexpr (std::is_same_v<LayerData, TileLayerDocument>) {
                if (layerData.width() != m_width || layerData.height() != m_height) {
                    return Result<bool>::Error(
                        "las dimensiones de una capa de tiles deben coincidir con el mapa");
                }
            } else if constexpr (std::is_same_v<LayerData, EntityLayerDocument>) {
                const auto insideMap = [this](GridCoord cell) {
                    return cell.x >= 0 && cell.y >= 0 && cell.x < m_width && cell.y < m_height;
                };
                for (const MapEntityDocument& entity : layerData.entities()) {
                    if (!insideMap(entity.position)) {
                        return Result<bool>::Error("la entidad '" + entity.id.value() +
                                                   "' está fuera de los límites del mapa");
                    }
                    // C-13 — La patrulla son celdas de ESTE mapa y se
                    // comprueban igual. targetPosition NO: es una celda
                    // del mapa de destino, que este documento no conoce.
                    if (!insideMap(entity.patrolMin) || !insideMap(entity.patrolMax)) {
                        return Result<bool>::Error("la patrulla de la entidad '" +
                                                   entity.id.value() +
                                                   "' está fuera de los límites del mapa");
                    }
                }
            } else {
                for (const MapRegionDocument& region : layerData.regions()) {
                    if (region.bounds.minX < 0 || region.bounds.minY < 0 ||
                        region.bounds.maxX >= m_width || region.bounds.maxY >= m_height) {
                        return Result<bool>::Error("la región '" + region.id.value() +
                                                   "' está fuera de los límites del mapa");
                    }
                }
            }
            return Result<bool>::Ok(true);
        },
        layer.data());
}

Result<bool> MapDocument::addLayer(MapLayer layer) {
    if (m_id.empty() || m_name.empty() || m_width <= 0 || m_height <= 0 || m_tileWidth <= 0 ||
        m_tileHeight <= 0) {
        return Result<bool>::Error("no se puede añadir una capa a un mapa con metadatos inválidos");
    }

    const Result<bool> layerResult = validateLayerForMap(layer);
    if (!layerResult.isOk()) {
        return layerResult;
    }

    for (const MapLayer& existing : m_layers) {
        if (existing.meta().id == layer.meta().id) {
            return Result<bool>::Error("id de capa duplicado: '" + layer.meta().id.value() + "'");
        }
        if (existing.meta().order == layer.meta().order) {
            return Result<bool>::Error("orden de capa duplicado: " +
                                       std::to_string(layer.meta().order));
        }
    }

    const auto insertionPoint = std::upper_bound(
        m_layers.begin(), m_layers.end(), layer.meta().order,
        [](int order, const MapLayer& existing) { return order < existing.meta().order; });
    m_layers.insert(insertionPoint, std::move(layer));
    return Result<bool>::Ok(true);
}

Result<bool> MapDocument::setCollisionGids(std::vector<int> gids) {
    if (std::any_of(gids.begin(), gids.end(), [](int gid) { return gid <= 0; })) {
        return Result<bool>::Error("un GID de colisión debe ser positivo");
    }
    std::sort(gids.begin(), gids.end());
    const auto duplicate = std::adjacent_find(gids.begin(), gids.end());
    if (duplicate != gids.end()) {
        return Result<bool>::Error("un GID de colisión está duplicado: " +
                                   std::to_string(*duplicate));
    }
    m_collisionGids = std::move(gids);
    return Result<bool>::Ok(true);
}

// ---------------------------------------------------------------------
// C-12 — API de mutacion del documento. Ver el contrato en MapDocument.h.
// ---------------------------------------------------------------------

const MapLayer* MapDocument::findLayer(const ids::LayerId& layerId) const {
    for (const MapLayer& layer : m_layers) {
        if (layer.meta().id == layerId) {
            return &layer;
        }
    }
    return nullptr;
}

MapLayer* MapDocument::findLayerMutable(const ids::LayerId& layerId) {
    for (MapLayer& layer : m_layers) {
        if (layer.meta().id == layerId) {
            return &layer;
        }
    }
    return nullptr;
}

void MapDocument::sortLayersByOrder() {
    std::stable_sort(m_layers.begin(), m_layers.end(),
                     [](const MapLayer& a, const MapLayer& b) {
                         return a.meta().order < b.meta().order;
                     });
}

Result<bool> MapDocument::mutateLayer(const ids::LayerId& layerId,
                                      const std::function<Result<bool>(MapLayer&)>& mutation) {
    MapLayer* layer = findLayerMutable(layerId);
    if (layer == nullptr) {
        return Result<bool>::Error("no existe la capa '" + layerId.value() + "'");
    }

    // Copia -> muta -> revalida -> instala. La copia es lo que hace que
    // una mutacion invalida no deje el documento a medias: si algo falla,
    // lo que se tira es la copia.
    MapLayer candidate = *layer;
    const Result<bool> applied = mutation(candidate);
    if (!applied.isOk()) {
        return applied;
    }
    const Result<bool> valid = validateLayerForMap(candidate);
    if (!valid.isOk()) {
        return valid;
    }
    *layer = std::move(candidate);
    return Result<bool>::Ok(true);
}

Result<bool> MapDocument::removeLayer(const ids::LayerId& layerId) {
    const auto it = std::find_if(m_layers.begin(), m_layers.end(),
                                 [&layerId](const MapLayer& layer) {
                                     return layer.meta().id == layerId;
                                 });
    if (it == m_layers.end()) {
        return Result<bool>::Error("no existe la capa '" + layerId.value() + "'");
    }
    m_layers.erase(it);
    return Result<bool>::Ok(true);
}

Result<bool> MapDocument::setLayerMeta(const ids::LayerId& layerId, MapLayerMeta meta) {
    MapLayer* layer = findLayerMutable(layerId);
    if (layer == nullptr) {
        return Result<bool>::Error("no existe la capa '" + layerId.value() + "'");
    }

    // Id y order nuevos no pueden chocar con OTRA capa (consigo misma si,
    // que es el caso de no cambiarlos).
    for (const MapLayer& existing : m_layers) {
        if (existing.meta().id == layerId) {
            continue;
        }
        if (existing.meta().id == meta.id) {
            return Result<bool>::Error("id de capa duplicado: '" + meta.id.value() + "'");
        }
        if (existing.meta().order == meta.order) {
            return Result<bool>::Error("orden de capa duplicado: " +
                                       std::to_string(meta.order));
        }
    }

    MapLayer candidate = *layer;
    candidate.mutableMeta() = std::move(meta);
    const Result<bool> valid = validateLayerForMap(candidate);
    if (!valid.isOk()) {
        return valid;
    }
    *layer = std::move(candidate);
    sortLayersByOrder();
    return Result<bool>::Ok(true);
}

Result<bool> MapDocument::moveLayer(const ids::LayerId& layerId, int newOrder) {
    const MapLayer* layer = findLayer(layerId);
    if (layer == nullptr) {
        return Result<bool>::Error("no existe la capa '" + layerId.value() + "'");
    }
    MapLayerMeta meta = layer->meta();
    meta.order = newOrder;
    return setLayerMeta(layerId, std::move(meta));
}

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

Result<bool> MapDocument::validate() const {
    if (m_id.empty()) {
        return Result<bool>::Error("un mapa no puede tener un id vacío");
    }
    if (m_name.empty()) {
        return Result<bool>::Error("un mapa no puede tener un nombre vacío");
    }
    if (m_width <= 0 || m_height <= 0 || m_tileWidth <= 0 || m_tileHeight <= 0) {
        return Result<bool>::Error("las dimensiones del mapa y de sus tiles deben ser positivas");
    }
    if (std::any_of(m_collisionGids.begin(), m_collisionGids.end(),
                    [](int gid) { return gid <= 0; }) ||
        !std::is_sorted(m_collisionGids.begin(), m_collisionGids.end()) ||
        std::adjacent_find(m_collisionGids.begin(), m_collisionGids.end()) != m_collisionGids.end()) {
        return Result<bool>::Error("los GIDs de colisión deben ser positivos, únicos y ordenados");
    }

    std::set<ids::LayerId> layerIds;
    std::set<int> layerOrders;
    int previousOrder = -1;
    for (const MapLayer& layer : m_layers) {
        const Result<bool> layerResult = validateLayerForMap(layer);
        if (!layerResult.isOk()) {
            return layerResult;
        }
        if (!layerIds.insert(layer.meta().id).second) {
            return Result<bool>::Error("id de capa duplicado: '" + layer.meta().id.value() + "'");
        }
        if (!layerOrders.insert(layer.meta().order).second) {
            return Result<bool>::Error("orden de capa duplicado: " +
                                       std::to_string(layer.meta().order));
        }
        if (layer.meta().order < previousOrder) {
            return Result<bool>::Error("las capas no están ordenadas de forma estable");
        }
        previousOrder = layer.meta().order;
    }
    return Result<bool>::Ok(true);
}
