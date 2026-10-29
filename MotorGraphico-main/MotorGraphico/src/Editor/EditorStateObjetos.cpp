// Objetos del mapa: colocar, mover, copiar, pegar y sus propiedades.
//
// Un objeto colocado tiene identidad y una docena de propiedades de
// instancia (botin, patrulla, destino de puerta, evento...). Aqui esta
// todo lo que las escribe, y el portapapeles que las duplica.
//
// Salio de src/Editor/EditorState.cpp el 24/09/2026 (EST-24): aquel fichero
// tenia 1075 lineas y seis paquetes encima (C-14, C-15, E-12, G-01, G-02,
// G-03). Se movio, no se reescribio.

#include "Editor/EditorState.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "Level/LevelDocumentBridge.h"
#include "Core/Json/JsonWriter.h"
#include <sstream>
#include <vector>


#include "Editor/EditorStateHelpers.h"

using EditorInternal::newLayerMeta;
using EditorInternal::replaceLayer;

namespace {

bool containsEntityId(const MapDocument& document, const ids::EntityId& id) {
    for (const MapLayer& layer : document.layers()) {
        if (layer.type() == MapLayerType::Entity &&
            std::get<EntityLayerDocument>(layer.data()).find(id) != nullptr) return true;
    }
    return false;
}

bool sameCell(GridCoord a, GridCoord b) { return a.x == b.x && a.y == b.y; }

bool inside(EditorState::Rect rect, GridCoord cell) {
    return cell.x >= rect.x0 && cell.x <= rect.x1 && cell.y >= rect.y0 && cell.y <= rect.y1;
}

GridCoord clampCell(GridCoord cell, int width, int height) {
    return {std::clamp(cell.x, 0, width - 1), std::clamp(cell.y, 0, height - 1)};
}


void translateEntity(MapEntityDocument& entity, int x, int y) {
    entity.position = {entity.position.x + x, entity.position.y + y};
    entity.patrolMin = {entity.patrolMin.x + x, entity.patrolMin.y + y};
    entity.patrolMax = {entity.patrolMax.x + x, entity.patrolMax.y + y};
}


}  // namespace

bool EditorState::insertSpawn(MapDocument& document, const ObjectSpawn& spawn,
                              const ids::LayerId* preferredLayer) {
    // Preparacion sobre copia: un dato invalido no borra el objeto que habia.
    MapDocument next = document;
    ids::LayerId destination;
    for (const MapLayer& layer : document.layers()) {
        if (layer.type() != MapLayerType::Entity) continue;
        if (destination.empty() && !layer.meta().locked) destination = layer.meta().id;
        if (preferredLayer != nullptr && layer.meta().id == *preferredLayer &&
            !layer.meta().locked) destination = *preferredLayer;
        for (const auto& entity : std::get<EntityLayerDocument>(layer.data()).entities()) {
            if (entity.position.x != spawn.position.x || entity.position.y != spawn.position.y) continue;
            if (layer.meta().locked) return false;
            if (preferredLayer == nullptr) destination = layer.meta().id;
            if (!next.removeEntity(layer.meta().id, entity.id).isOk()) return false;
        }
    }
    if (destination.empty()) {
        MapLayerMeta meta = newLayerMeta(next, "objetos");
        meta.name = "objetos";
        destination = meta.id;
        if (!next.addLayer(MapLayer::entities(meta, EntityLayerDocument{})).isOk()) return false;
    }
    auto id = level_document::MakeEntityId(spawn.objectId, spawn.position);
    if (!id.isOk()) return false;
    for (int suffix = 2; containsEntityId(next, id.value()); ++suffix) {
        id = level_document::MakeEntityId(spawn.objectId, spawn.position, suffix);
        if (!id.isOk()) return false;
    }
    auto entity = level_document::ToEntityDocument(spawn, id.value());
    if (!entity.isOk() || !next.addEntity(destination, std::move(entity.value())).isOk()) return false;
    document = std::move(next);
    return true;
}

void EditorState::placeObject(int x, int y, const std::string& objectId) {
    if (!inBounds(x, y) || objectId.empty()) return;
    const ObjectSpawn* existing = objectAt(x, y);
    if (existing != nullptr && existing->objectId == objectId && existing->targetLevel.empty()) return;
    ObjectSpawn spawn;
    spawn.objectId = objectId;
    spawn.position = {x, y};
    spawn.patrolMin = spawn.position;
    spawn.patrolMax = spawn.position;
    placeSpawn(spawn);
}

void EditorState::placeSpawn(const ObjectSpawn& spawn) {
    if (!inBounds(spawn.position.x, spawn.position.y) || spawn.objectId.empty()) return;
    MapDocument next = m_document;
    if (insertSpawn(next, spawn)) commitDocument(std::move(next));
}

void EditorState::removeObjectAt(int x, int y) {
    MapDocument next = m_document;
    bool changed = false;
    for (const MapLayer& layer : m_document.layers()) {
        if (layer.type() != MapLayerType::Entity) continue;
        for (const auto& entity : std::get<EntityLayerDocument>(layer.data()).entities()) {
            if (entity.position.x != x || entity.position.y != y) continue;
            if (layer.meta().locked) return;
            if (!next.removeEntity(layer.meta().id, entity.id).isOk()) return;
            changed = true;
        }
    }
    if (changed) commitDocument(std::move(next));
}

void EditorState::editObject(int x, int y, const std::function<bool(ObjectSpawn&)>& edit) {
    for (const MapLayer& layer : m_document.layers()) {
        if (layer.type() != MapLayerType::Entity) continue;
        std::vector<MapEntityDocument> entities = std::get<EntityLayerDocument>(layer.data()).entities();
        for (MapEntityDocument& entity : entities) {
            if (entity.position.x != x || entity.position.y != y) continue;
            if (layer.meta().locked) return;
            ObjectSpawn spawn = level_document::ToObjectSpawn(entity);
            if (!edit(spawn)) return;
            auto edited = level_document::ToEntityDocument(spawn, entity.id);
            if (!edited.isOk()) return;
            entity = std::move(edited.value());
            MapDocument next = m_document;
            if (replaceLayer(next, MapLayer::entities(layer.meta(), EntityLayerDocument(std::move(entities)))))
                commitDocument(std::move(next));
            return;
        }
    }
}

void EditorState::setObjectTransition(int x, int y, const std::string& targetLevel) {
    if (targetLevel.empty()) return;
    editObject(x, y, [&](ObjectSpawn& object) {
        if (object.targetLevel == targetLevel && !object.hasTargetPosition) return false;
        object.targetLevel = targetLevel;
        object.hasTargetPosition = false;
        return true;
    });
}

void EditorState::setPlayerStart(GridCoord position) {
    if (!inBounds(position.x, position.y) ||
        (position.x == m_playerStart.x && position.y == m_playerStart.y)) return;
    recordUndo();
    m_playerStart = position;
}

void EditorState::setSelection(int x0, int y0, int x1, int y1) {
    m_selection = {std::max(0, std::min(x0, x1)), std::max(0, std::min(y0, y1)),
                   std::min(width() - 1, std::max(x0, x1)),
                   std::min(height() - 1, std::max(y0, y1))};
}

void EditorState::clearSelection() { m_selection = Rect{}; }

void EditorState::copySelection() {
    if (!m_selection.valid()) return;
    Clipboard copy;
    copy.width = m_selection.width();
    copy.height = m_selection.height();
    copy.layers.resize(static_cast<std::size_t>(layerCount()));
    for (int index = 0; index < layerCount(); ++index) {
        for (int y = m_selection.y0; y <= m_selection.y1; ++y)
            for (int x = m_selection.x0; x <= m_selection.x1; ++x)
                copy.layers[static_cast<std::size_t>(index)].push_back(tileAt(index, x, y));
    }
    for (const MapLayer& layer : m_document.layers()) {
        if (layer.type() != MapLayerType::Entity) continue;
        for (const MapEntityDocument& source : std::get<EntityLayerDocument>(layer.data()).entities()) {
            if (!inside(m_selection, source.position)) continue;
            MapEntityDocument entity = source;
            translateEntity(entity, -m_selection.x0, -m_selection.y0);
            copy.entities.push_back({layer.meta().id, std::move(entity)});
        }
    }
    m_clipboard = std::move(copy);
}

void EditorState::pasteAt(int x, int y) {
    if (!hasClipboard()) return;
    MapDocument next = m_document;
    bool changed = false;
    for (int index = 0; index < layerCount() && index < static_cast<int>(m_clipboard.layers.size()); ++index) {
        const MapLayer& layer = *tileLayer(index);
        if (layer.meta().locked) continue;
        TileLayerDocument tiles = std::get<TileLayerDocument>(layer.data());
        bool layerChanged = false;
        for (int dy = 0; dy < m_clipboard.height; ++dy) {
            for (int dx = 0; dx < m_clipboard.width; ++dx) {
                if (!inBounds(x + dx, y + dy)) continue;
                const int gid = m_clipboard.layers[static_cast<std::size_t>(index)]
                    [static_cast<std::size_t>(dy) * m_clipboard.width + dx];
                if (tiles.gidAt(x + dx, y + dy) == gid) continue;
                tiles.setGid(x + dx, y + dy, gid);
                layerChanged = true;
            }
        }
        if (layerChanged) {
            if (!replaceLayer(next, MapLayer::tiles(layer.meta(), std::move(tiles)))) return;
            changed = true;
        }
    }
    for (const Clipboard::Entity& source : m_clipboard.entities) {
        MapEntityDocument entity = source.data;
        translateEntity(entity, x, y);
        if (!inBounds(entity.position.x, entity.position.y)) continue;
        // La patrulla se traslada con el objeto y se recorta al nuevo mapa.
        entity.patrolMin = clampCell(entity.patrolMin, width(), height());
        entity.patrolMax = clampCell(entity.patrolMax, width(), height());
        if (!insertSpawn(next, level_document::ToObjectSpawn(entity), &source.layer)) return;
        changed = true;
    }
    if (changed) commitDocument(std::move(next));
}

void EditorState::eraseSelection() {
    if (!m_selection.valid()) return;
    MapDocument next = m_document;
    bool changed = false;
    for (const MapLayer& layer : m_document.layers()) {
        if (layer.meta().locked) continue;
        if (layer.type() == MapLayerType::Tile) {
            TileLayerDocument tiles = std::get<TileLayerDocument>(layer.data());
            bool layerChanged = false;
            for (int y = m_selection.y0; y <= m_selection.y1; ++y) {
                for (int x = m_selection.x0; x <= m_selection.x1; ++x) {
                    if (tiles.gidAt(x, y) == 0) continue;
                    tiles.setGid(x, y, 0);
                    layerChanged = true;
                }
            }
            if (layerChanged) {
                if (!replaceLayer(next, MapLayer::tiles(layer.meta(), std::move(tiles)))) return;
                changed = true;
            }
        } else if (layer.type() == MapLayerType::Entity) {
            for (const auto& entity : std::get<EntityLayerDocument>(layer.data()).entities()) {
                if (!inside(m_selection, entity.position)) continue;
                if (!next.removeEntity(layer.meta().id, entity.id).isOk()) return;
                changed = true;
            }
        }
    }
    if (changed) commitDocument(std::move(next));
}

void EditorState::setObjectPatrol(int x, int y, GridCoord min, GridCoord max) {
    const GridCoord a = clampCell({std::min(min.x, max.x), std::min(min.y, max.y)}, width(), height());
    const GridCoord b = clampCell({std::max(min.x, max.x), std::max(min.y, max.y)}, width(), height());
    editObject(x, y, [&](ObjectSpawn& object) {
        if (sameCell(object.patrolMin, a) && sameCell(object.patrolMax, b)) return false;
        object.patrolMin = a;
        object.patrolMax = b;
        return true;
    });
}

void EditorState::setObjectTargetPosition(int x, int y, GridCoord target) {
    editObject(x, y, [&](ObjectSpawn& object) {
        if (object.hasTargetPosition && sameCell(object.targetPosition, target)) return false;
        object.targetPosition = target;
        object.hasTargetPosition = true;
        return true;
    });
}

void EditorState::setObjectScale(int x, int y, float scale) {
    if (!std::isfinite(scale)) return;
    const float limited = std::clamp(scale, 0.25f, 4.0f);
    editObject(x, y, [&](ObjectSpawn& object) {
        if (object.scale == limited) return false;
        object.scale = limited;
        return true;
    });
}

void EditorState::setObjectTargetLevel(int x, int y, const std::string& targetLevel) {
    editObject(x, y, [&](ObjectSpawn& object) {
        if (object.targetLevel == targetLevel && (!targetLevel.empty() || !object.hasTargetPosition))
            return false;
        object.targetLevel = targetLevel;
        object.targetPosition = object.position;
        object.hasTargetPosition = false;
        return true;
    });
}

void EditorState::addObjectLoot(int x, int y, const std::string& objectId) {
    if (objectId.empty()) return;
    editObject(x, y, [&](ObjectSpawn& object) {
        if (std::find(object.loot.begin(), object.loot.end(), objectId) != object.loot.end()) return false;
        object.loot.push_back(objectId);
        return true;
    });
}

void EditorState::removeLootAt(int x, int y, std::size_t index) {
    editObject(x, y, [&](ObjectSpawn& object) {
        if (index >= object.loot.size()) return false;
        object.loot.erase(object.loot.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
    });
}

void EditorState::setObjectProperty(int x, int y, const std::string& key, const std::string& value) {
    if (key.empty()) return;
    editObject(x, y, [&](ObjectSpawn& object) {
        const auto found = object.properties.find(key);
        if (found != object.properties.end() && found->second == value) return false;
        object.properties[key] = value;
        return true;
    });
}

void EditorState::removeObjectProperty(int x, int y, const std::string& key) {
    editObject(x, y, [&](ObjectSpawn& object) { return object.properties.erase(key) != 0; });
}


void EditorState::setObjectDisplayName(int x, int y, const std::string& value) {
    editObject(x, y, [&](ObjectSpawn& object) {
        if (object.displayName == value) return false;
        object.displayName = value;
        return true;
    });
}

void EditorState::setObjectVariant(int x, int y, const std::string& value) {
    editObject(x, y, [&](ObjectSpawn& object) {
        if (object.variant == value) return false;
        object.variant = value;
        return true;
    });
}

void EditorState::setObjectEffectOverride(int x, int y, const std::string& value) {
    editObject(x, y, [&](ObjectSpawn& object) {
        if (object.effectOverride == value) return false;
        object.effectOverride = value;
        return true;
    });
}

void EditorState::setObjectLootTable(int x, int y, const std::string& value) {
    editObject(x, y, [&](ObjectSpawn& object) {
        if (object.lootTable == value) return false;
        object.lootTable = value;
        return true;
    });
}

void EditorState::setObjectEvent(int x, int y, const std::string& value) {
    editObject(x, y, [&](ObjectSpawn& object) {
        if (object.eventId == value) return false;
        object.eventId = value;
        return true;
    });
}

