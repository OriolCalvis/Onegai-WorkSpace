// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 3 paquetes del corte B:
//   C-12  (fase C, 5 j) - MapDocument es pot editar
//   E-01  (fase E, 3 j) - L'esdeveniment és part del document de mapa
//   F-03  (fase F, 4 j) - Recàrrega en calent
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

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

    // C-23 — Post-procesat per nivell (niebla i llum dinàmica).
    // Quan cap dels dos esta activat, el runtime no crea cap objecte
    // ni fa cap crida GL (cost zero).
    bool fogEnabled() const { return m_fogEnabled; }
    int fogRevealRadius() const { return m_fogRevealRadius; }
    bool lightingEnabled() const { return m_lightingEnabled; }
    float ambientR() const { return m_ambientR; }
    float ambientG() const { return m_ambientG; }
    float ambientB() const { return m_ambientB; }
    void setFogEnabled(bool v) { m_fogEnabled = v; }
    void setFogRevealRadius(int v) { m_fogRevealRadius = v; }
    void setLightingEnabled(bool v) { m_lightingEnabled = v; }
    void setAmbientR(float v) { m_ambientR = v; }
    void setAmbientG(float v) { m_ambientG = v; }
    void setAmbientB(float v) { m_ambientB = v; }

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

    // -----------------------------------------------------------------
    // C-14 (2a rebanada) — Edicion POR LOTES de una capa de tiles.
    //
    // POR QUE. setTile() copia la capa entera, la revalida y la instala
    // en CADA llamada. Para un click es el contrato correcto. Para N
    // celdas es O(celdas_de_la_capa x N): un relleno por inundacion de
    // 10.000 celdas sobre un mapa de 200x200 copia 400 millones de
    // enteros y revalida 10.000 veces. Mientras la unica forma de tocar
    // N celdas fuera llamar N veces, EditorState no podia pasar a ser
    // una vista sobre este documento sin volverse inutilizable — que es
    // exactamente lo que C-14 pide y lo que bloqueaba el paquete.
    //
    // QUE CAMBIA Y QUE NO. Cambia el coste: UNA copia, UNA validacion y
    // UNA instalacion para todo el lote. NO cambia el contrato: si la
    // funcion devuelve error, o si el resultado no valida contra el
    // mapa, el documento queda EXACTAMENTE como estaba. La funcion
    // recibe la capa COPIA, nunca la viva, asi que no hay forma de
    // dejar una edicion a medias instalada.
    //
    // La funcion puede tocar las celdas que quiera y devolver Error para
    // abortar el lote entero (por ejemplo, al llegar a una celda fuera
    // del mapa a mitad de un pegado).
    // -----------------------------------------------------------------
    Result<bool> editTiles(const ids::LayerId& layerId,
                           const std::function<Result<bool>(TileLayerDocument&)>& edit);

    // Entidades (capas de entidades).
    Result<bool> addEntity(const ids::LayerId& layerId, MapEntityDocument entity);
    Result<bool> removeEntity(const ids::LayerId& layerId, const ids::EntityId& entityId);
    Result<bool> moveEntity(const ids::LayerId& layerId, const ids::EntityId& entityId,
                            GridCoord position);

    // E-01 — Eventos (capas de eventos). Mismo contrato que las
    // entidades: todo-o-nada, error devuelto y no lanzado, y el
    // documento revalida la capa contra las dimensiones del mapa, que es
    // lo unico que la capa por si sola no puede comprobar.
    //
    // Las paginas de un evento se cambian con setEventPages() sobre la
    // capa; aqui se expone la operacion equivalente para que el editor no
    // tenga que sacar la capa, mutarla y volver a meterla a mano.
    Result<bool> addEvent(const ids::LayerId& layerId, MapEventDocument event);
    Result<bool> removeEvent(const ids::LayerId& layerId, const ids::EventId& eventId);
    Result<bool> moveEvent(const ids::LayerId& layerId, const ids::EventId& eventId,
                           GridCoord position);
    Result<bool> setEventPages(const ids::LayerId& layerId, const ids::EventId& eventId,
                               std::vector<MapEventPage> pages);

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

    // C-23 — Post-procesat per nivell.
    bool m_fogEnabled = false;
    int m_fogRevealRadius = 1;
    bool m_lightingEnabled = false;
    float m_ambientR = 0.35f;
    float m_ambientG = 0.35f;
    float m_ambientB = 0.40f;
};
