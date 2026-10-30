// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-12  (fase C, 5 j) - MapDocument es pot editar
//   C-13  (fase C, 3 j) - Les entitats del document no perden res
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// EL DOCUMENTO DE MAPA: sus capas y su validacion.
//
// 23/09/2026 (EST-27): de 724 lineas a ~340. Las tres clases de capa y la
// API de edicion salieron a ficheros propios; aqui queda lo que es del
// documento —crear capas, buscarlas, ordenarlas, mutarlas con garantia, y
// decir si el mapa entero se sostiene—.

#include "Map/MapDocument.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <type_traits>
#include <utility>

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

MapLayer MapLayer::events(MapLayerMeta meta, EventLayerDocument data) {
    return MapLayer(std::move(meta), std::move(data));
}

MapLayerType MapLayer::type() const {
    if (std::holds_alternative<TileLayerDocument>(m_data)) {
        return MapLayerType::Tile;
    }
    if (std::holds_alternative<EntityLayerDocument>(m_data)) {
        return MapLayerType::Entity;
    }
    if (std::holds_alternative<RegionLayerDocument>(m_data)) {
        return MapLayerType::Region;
    }
    return MapLayerType::Event;
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
            } else if constexpr (std::is_same_v<LayerData, RegionLayerDocument>) {
                for (const MapRegionDocument& region : layerData.regions()) {
                    if (region.bounds.minX < 0 || region.bounds.minY < 0 ||
                        region.bounds.maxX >= m_width || region.bounds.maxY >= m_height) {
                        return Result<bool>::Error("la región '" + region.id.value() +
                                                   "' está fuera de los límites del mapa");
                    }
                }
            } else {
                // E-01 — Un evento es una casilla con comportamiento: su
                // posicion es de ESTE mapa y se comprueba igual que la de
                // una entidad. Antes esta rama era el `else` de Region;
                // nombrar los cuatro tipos evita que anadir un quinto se
                // cuele validandose como evento.
                for (const MapEventDocument& event : layerData.events()) {
                    if (event.position.x < 0 || event.position.y < 0 ||
                        event.position.x >= m_width || event.position.y >= m_height) {
                        return Result<bool>::Error("el evento '" + event.id.value() +
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

