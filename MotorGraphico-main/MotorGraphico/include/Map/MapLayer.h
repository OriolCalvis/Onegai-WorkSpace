// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-12  (fase C, 5 j) - MapDocument es pot editar
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <string>
#include <utility>
#include <variant>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Map/EntityLayerDocument.h"
#include "Map/EventLayerDocument.h"
#include "Map/RegionLayerDocument.h"
#include "Map/TileLayerDocument.h"

// E-01 anade Event como cuarto tipo. El orden del enum no es el del
// variant y no debe darse por bueno: type() lo resuelve explicitamente.
enum class MapLayerType { Tile, Entity, Region, Event };

// Metadatos comunes a toda capa. order es un valor explícito, no la posición
// accidental en un vector, para conservar un orden determinista al guardar,
// cargar y sincronizar documentos.
struct MapLayerMeta {
    ids::LayerId id;
    std::string name;
    bool visible = true;
    bool locked = false;
    float opacity = 1.0f;
    int order = 0;
};

using MapLayerData =
    std::variant<TileLayerDocument, EntityLayerDocument, RegionLayerDocument, EventLayerDocument>;

// El tipo de una capa es inmutable: el variant solo se recibe al construirla y
// se expone mediante una referencia const. Convertir Tile -> Entity o Region
// exige crear otra capa y migrar su contenido de forma explícita.
class MapLayer {
public:
    static MapLayer tiles(MapLayerMeta meta, TileLayerDocument data);
    static MapLayer entities(MapLayerMeta meta, EntityLayerDocument data);
    static MapLayer regions(MapLayerMeta meta, RegionLayerDocument data);
    // E-01 — Los eventos de un mapa. Ver Map/EventLayerDocument.h.
    static MapLayer events(MapLayerMeta meta, EventLayerDocument data);

    const MapLayerMeta& meta() const { return m_meta; }
    MapLayerType type() const;
    const MapLayerData& data() const { return m_data; }

    Result<bool> validate() const;

private:
    MapLayer(MapLayerMeta meta, MapLayerData data);

    // C-12 — Acceso mutable SOLO para MapDocument. Es lo que mantiene la
    // promesa del paquete: se puede editar una capa, pero no a espaldas
    // del documento -- toda mutacion pasa por MapDocument, que trabaja
    // sobre una copia, la revalida contra las dimensiones del mapa y solo
    // entonces la instala. Publicos, estos dos metodos permitirian dejar
    // una entidad fuera del mapa sin que nadie se enterase.
    //
    // El TIPO de la capa sigue siendo inmutable: el variant se puede
    // modificar por dentro, pero convertir Tile -> Entity exige crear
    // otra capa, igual que antes.
    friend class MapDocument;
    MapLayerMeta& mutableMeta() { return m_meta; }
    MapLayerData& mutableData() { return m_data; }

    MapLayerMeta m_meta;
    MapLayerData m_data;
};
