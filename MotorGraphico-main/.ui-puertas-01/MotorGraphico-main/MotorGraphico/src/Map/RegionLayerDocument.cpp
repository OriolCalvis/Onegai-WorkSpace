// La capa de regiones: zonas rectangulares con nombre.
//
// Mismo contrato que las entidades —identidad, alta, baja, mover— pero
// sobre rectangulos. Separada porque cambia por motivos distintos: las
// regiones crecen con los disparadores, las entidades con el catalogo.
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
