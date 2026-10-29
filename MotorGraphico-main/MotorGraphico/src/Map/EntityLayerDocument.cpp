// La capa de entidades: NPCs, objetos y cofres colocados en el mapa.
//
// Identidad propia por entidad (C-13): anadir dos veces el mismo id es
// un error, y mover una que no existe tambien. Lo que no se valide aqui
// acaba siendo un objeto invisible en la partida.
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
