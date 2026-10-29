#include "Level/LevelDocumentBridge.h"

#include <set>
#include <string>
#include <utility>
#include <variant>

namespace level_document {

Result<ids::EntityId> MakeEntityId(const std::string& objectId, GridCoord position,
                                   int disambiguator) {
    if (objectId.empty()) {
        return Result<ids::EntityId>::Error("un spawn sin objectId no puede tener id de entidad");
    }
    // Las coordenadas negativas no caben en [a-z0-9_]+ y, de todas
    // formas, no son celdas de ningún mapa: se rechazan aquí en vez de
    // producir un id inválido que falle una capa más abajo con un
    // mensaje peor.
    if (position.x < 0 || position.y < 0) {
        return Result<ids::EntityId>::Error("el spawn '" + objectId +
                                            "' está en una celda negativa");
    }

    std::string value = objectId + "_" + std::to_string(position.x) + "_" +
                        std::to_string(position.y);
    if (disambiguator > 1) {
        value += "_" + std::to_string(disambiguator);
    }

    Result<ids::EntityId> parsed = ids::EntityId::parse(value);
    if (!parsed.isOk()) {
        return Result<ids::EntityId>::Error("el objectId '" + objectId +
                                            "' no es un identificador válido ([a-z0-9_]+): " +
                                            parsed.errorMessage());
    }
    return parsed;
}

Result<MapEntityDocument> ToEntityDocument(const ObjectSpawn& spawn, ids::EntityId entityId) {
    const Result<ids::AssetId> objectId = ids::AssetId::parse(spawn.objectId);
    if (!objectId.isOk()) {
        return Result<MapEntityDocument>::Error("el objectId '" + spawn.objectId +
                                                "' no es un identificador válido ([a-z0-9_]+)");
    }

    MapEntityDocument entity;
    entity.id = std::move(entityId);
    entity.objectId = objectId.value();
    entity.position = spawn.position;
    entity.patrolMin = spawn.patrolMin;
    entity.patrolMax = spawn.patrolMax;
    entity.targetLevel = spawn.targetLevel;
    entity.targetPosition = spawn.targetPosition;
    entity.hasTargetPosition = spawn.hasTargetPosition;
    entity.loot = spawn.loot;
    entity.lootTable = spawn.lootTable;
    entity.eventId = spawn.eventId;
    entity.displayName = spawn.displayName;
    entity.variant = spawn.variant;
    entity.scale = spawn.scale;
    entity.effectOverride = spawn.effectOverride;
    entity.properties = spawn.properties;
    return Result<MapEntityDocument>::Ok(std::move(entity));
}

ObjectSpawn ToObjectSpawn(const MapEntityDocument& entity) {
    ObjectSpawn spawn;
    spawn.objectId = entity.objectId.value();
    spawn.position = entity.position;
    spawn.patrolMin = entity.patrolMin;
    spawn.patrolMax = entity.patrolMax;
    spawn.targetLevel = entity.targetLevel;
    spawn.targetPosition = entity.targetPosition;
    spawn.hasTargetPosition = entity.hasTargetPosition;
    spawn.loot = entity.loot;
    spawn.lootTable = entity.lootTable;
    spawn.eventId = entity.eventId;
    spawn.displayName = entity.displayName;
    spawn.variant = entity.variant;
    spawn.scale = entity.scale;
    spawn.effectOverride = entity.effectOverride;
    spawn.properties = entity.properties;
    return spawn;
}

Result<EntityLayerDocument> ToEntityLayer(const std::vector<ObjectSpawn>& spawns) {
    std::vector<MapEntityDocument> entities;
    entities.reserve(spawns.size());
    std::set<ids::EntityId> used;

    for (const ObjectSpawn& spawn : spawns) {
        // Desambiguación por celda compartida. El bucle está acotado por
        // el número de spawns: como mucho hay tantos en la misma celda
        // como spawns tiene el nivel.
        Result<ids::EntityId> entityId = Result<ids::EntityId>::Error("sin generar");
        for (int attempt = 1; attempt <= static_cast<int>(spawns.size()) + 1; ++attempt) {
            entityId = MakeEntityId(spawn.objectId, spawn.position, attempt);
            if (!entityId.isOk()) {
                return Result<EntityLayerDocument>::Error(entityId.errorMessage());
            }
            if (used.find(entityId.value()) == used.end()) {
                break;
            }
        }
        if (used.find(entityId.value()) != used.end()) {
            return Result<EntityLayerDocument>::Error(
                "no se pudo dar un id único a un spawn de '" + spawn.objectId + "'");
        }

        Result<MapEntityDocument> entity = ToEntityDocument(spawn, entityId.value());
        if (!entity.isOk()) {
            // Todo-o-nada: se devuelve el error y NO una capa a medias.
            return Result<EntityLayerDocument>::Error(entity.errorMessage());
        }
        used.insert(entityId.value());
        entities.push_back(std::move(entity.value()));
    }

    return Result<EntityLayerDocument>::Ok(EntityLayerDocument(std::move(entities)));
}

std::vector<ObjectSpawn> ToObjectSpawns(const MapDocument& document) {
    std::vector<ObjectSpawn> spawns;
    for (const MapLayer& layer : document.layers()) {
        if (layer.type() != MapLayerType::Entity) {
            continue;
        }
        const EntityLayerDocument& entities = std::get<EntityLayerDocument>(layer.data());
        spawns.reserve(spawns.size() + entities.entities().size());
        for (const MapEntityDocument& entity : entities.entities()) {
            spawns.push_back(ToObjectSpawn(entity));
        }
    }
    return spawns;
}

LevelDefinition ToLevelDefinition(const MapDocument& document) {
    LevelDefinition level;
    level.name = document.name();
    level.mapPath = "";  // se rellena al guardar el nivel, no aquí
    level.playerStart = {0, 0};  // se rellena al guardar el nivel
    level.mapDocumentPath = "";
    level.fogEnabled = document.fogEnabled();
    level.fogRevealRadius = document.fogRevealRadius();
    level.lightingEnabled = document.lightingEnabled();
    level.ambientR = document.ambientR();
    level.ambientG = document.ambientG();
    level.ambientB = document.ambientB();
    return level;
}

void ApplyToMapDocument(const LevelDefinition& level, MapDocument& document) {
    document.setFogEnabled(level.fogEnabled);
    document.setFogRevealRadius(level.fogRevealRadius);
    document.setLightingEnabled(level.lightingEnabled);
    document.setAmbientR(level.ambientR);
    document.setAmbientG(level.ambientG);
    document.setAmbientB(level.ambientB);
}

}  // namespace level_document
