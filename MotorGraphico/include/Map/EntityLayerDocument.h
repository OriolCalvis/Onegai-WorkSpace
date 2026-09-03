#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridCoord.h"

// Instancia de una entidad en un mapa. objectId se resolverá contra un
// catálogo en una fase posterior; este documento solo conserva su referencia
// estable y su posición, sin actores de runtime ni recursos gráficos.
struct MapEntityDocument {
    ids::EntityId id;
    ids::AssetId objectId;
    GridCoord position;
};

class EntityLayerDocument {
public:
    EntityLayerDocument() = default;
    explicit EntityLayerDocument(std::vector<MapEntityDocument> entities);

    const std::vector<MapEntityDocument>& entities() const { return m_entities; }

    // --- C-12: mutacion (ver el comentario de TileLayerDocument) ---
    //
    // Los limites del MAPA no se comprueban aqui -- esta capa no sabe en
    // que mapa vive. De eso se encarga MapDocument, que revalida la capa
    // contra sus dimensiones despues de cada mutacion.
    const MapEntityDocument* find(const ids::EntityId& id) const;
    Result<bool> addEntity(MapEntityDocument entity);
    Result<bool> removeEntity(const ids::EntityId& id);
    Result<bool> setEntityPosition(const ids::EntityId& id, GridCoord position);

    Result<bool> validate() const;

private:
    std::vector<MapEntityDocument> m_entities;
};
