#pragma once

#include <string>
#include <utility>
#include <variant>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Map/EntityLayerDocument.h"
#include "Map/RegionLayerDocument.h"
#include "Map/TileLayerDocument.h"

enum class MapLayerType { Tile, Entity, Region };

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

using MapLayerData = std::variant<TileLayerDocument, EntityLayerDocument, RegionLayerDocument>;

// El tipo de una capa es inmutable: el variant solo se recibe al construirla y
// se expone mediante una referencia const. Convertir Tile -> Entity o Region
// exige crear otra capa y migrar su contenido de forma explícita.
class MapLayer {
public:
    static MapLayer tiles(MapLayerMeta meta, TileLayerDocument data);
    static MapLayer entities(MapLayerMeta meta, EntityLayerDocument data);
    static MapLayer regions(MapLayerMeta meta, RegionLayerDocument data);

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
