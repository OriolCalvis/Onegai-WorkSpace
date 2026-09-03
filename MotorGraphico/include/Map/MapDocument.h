#pragma once

#include <functional>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridBounds.h"
#include "Core/Math/GridCoord.h"
#include "Map/MapLayer.h"

// Fuente de verdad editable de un mapa. No conoce TileMap, texturas, OpenGL ni
// widgets: los consumidores de runtime compilarán este documento validado en
// sus propias estructuras en DOC-03.
class MapDocument {
public:
    MapDocument() = default;
    MapDocument(ids::MapId id, std::string name, int width, int height, int tileWidth,
                int tileHeight);

    const ids::MapId& id() const { return m_id; }
    const std::string& name() const { return m_name; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    int tileWidth() const { return m_tileWidth; }
    int tileHeight() const { return m_tileHeight; }
    const std::vector<int>& collisionGids() const { return m_collisionGids; }
    const std::vector<MapLayer>& layers() const { return m_layers; }

    // Inserta por order sin alterar la posición relativa de las capas ya
    // existentes. IDs y órdenes duplicados, así como datos fuera del mapa,
    // se rechazan antes de modificar el documento.
    Result<bool> addLayer(MapLayer layer);
    Result<bool> setCollisionGids(std::vector<int> gids);
    Result<bool> validate() const;

    // ---------------------------------------------------------------
    // C-12 — API de mutacion.
    //
    // Hasta aqui el documento era de escritura unica: addLayer y nada
    // mas. Ni borrar una capa, ni reordenarlas, ni tocar una celda, ni
    // mover una entidad. Mientras siguiera asi, migrar el editor a este
    // documento era imposible por construccion, no por falta de ganas.
    //
    // CONTRATO COMUN de todo lo que hay debajo:
    //   - Todo-o-nada. La mutacion se aplica sobre una COPIA de la capa,
    //     se revalida contra el mapa (dimensiones, limites, ids unicos) y
    //     solo si pasa se instala. Un fallo deja el documento EXACTAMENTE
    //     como estaba.
    //   - validate() sigue siendo cierto despues de cada llamada. Es la
    //     diferencia entre una API de edicion y un puñado de setters: las
    //     invariantes que antes solo comprobaba validate() ahora las
    //     mantiene cada mutacion.
    //   - El error se devuelve, no se lanza. Editar es una accion de
    //     usuario: "esa celda esta fuera del mapa" es un mensaje, no un
    //     crash.
    // ---------------------------------------------------------------

    // Capas.
    Result<bool> removeLayer(const ids::LayerId& layerId);
    // Cambia los metadatos (nombre, visibilidad, bloqueo, opacidad, orden
    // y hasta el id). El TIPO no se toca: para eso hay que crear otra
    // capa. Reordena el vector si el order cambia.
    Result<bool> setLayerMeta(const ids::LayerId& layerId, MapLayerMeta meta);
    // Atajo del caso comun de reordenar: mover una capa a otro order.
    Result<bool> moveLayer(const ids::LayerId& layerId, int newOrder);

    // Celdas (capas de tiles).
    Result<bool> setTile(const ids::LayerId& layerId, GridCoord cell, int gid);
    Result<bool> fillTiles(const ids::LayerId& layerId, GridBounds area, int gid);

    // Entidades (capas de entidades).
    Result<bool> addEntity(const ids::LayerId& layerId, MapEntityDocument entity);
    Result<bool> removeEntity(const ids::LayerId& layerId, const ids::EntityId& entityId);
    Result<bool> moveEntity(const ids::LayerId& layerId, const ids::EntityId& entityId,
                            GridCoord position);

    // Regiones (capas de regiones).
    Result<bool> addRegion(const ids::LayerId& layerId, MapRegionDocument region);
    Result<bool> removeRegion(const ids::LayerId& layerId, const ids::RegionId& regionId);
    Result<bool> setRegionBounds(const ids::LayerId& layerId, const ids::RegionId& regionId,
                                 GridBounds bounds);

    // Consulta por id, sin tener que recorrer layers() a mano.
    const MapLayer* findLayer(const ids::LayerId& layerId) const;

private:
    Result<bool> validateLayerForMap(const MapLayer& layer) const;

    MapLayer* findLayerMutable(const ids::LayerId& layerId);
    // Copia -> muta -> revalida -> instala. El corazon del contrato de
    // arriba, en un solo sitio para que ninguna mutacion pueda saltarselo
    // por descuido.
    Result<bool> mutateLayer(const ids::LayerId& layerId,
                             const std::function<Result<bool>(MapLayer&)>& mutation);
    // Reordena m_layers por meta().order (estable) tras un cambio de
    // orden: validate() exige que el vector este ordenado.
    void sortLayersByOrder();

    ids::MapId m_id;
    std::string m_name;
    int m_width = 0;
    int m_height = 0;
    int m_tileWidth = 0;
    int m_tileHeight = 0;
    std::vector<int> m_collisionGids;
    std::vector<MapLayer> m_layers;
};
