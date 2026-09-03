#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridBounds.h"

// Región rectangular de autoría. Los futuros sistemas de eventos y zonas
// pueden añadir propiedades a esta ficha sin convertir la capa en una entidad
// de runtime.
struct MapRegionDocument {
    ids::RegionId id;
    std::string name;
    GridBounds bounds;
};

class RegionLayerDocument {
public:
    RegionLayerDocument() = default;
    explicit RegionLayerDocument(std::vector<MapRegionDocument> regions);

    const std::vector<MapRegionDocument>& regions() const { return m_regions; }

    // --- C-12: mutacion (ver el comentario de TileLayerDocument) ---
    const MapRegionDocument* find(const ids::RegionId& id) const;
    Result<bool> addRegion(MapRegionDocument region);
    Result<bool> removeRegion(const ids::RegionId& id);
    Result<bool> setRegionBounds(const ids::RegionId& id, GridBounds bounds);

    Result<bool> validate() const;

private:
    std::vector<MapRegionDocument> m_regions;
};
