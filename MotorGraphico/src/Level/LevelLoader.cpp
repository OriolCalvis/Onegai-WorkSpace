#include "Level/LevelLoader.h"

#include "Core/Json/JsonValue.h"

#include <fstream>
#include <sstream>

namespace {

// GridCoord desde un objeto JSON {"x": .., "y": ..}; si "value" no es un
// objeto (falta el campo, o el JSON trae otro tipo), devuelve
// defaultValue entero -- mismo criterio permisivo que JsonValue::asInt()
// (un campo de posicion ausente no es motivo para rechazar todo el
// nivel: los campos obligatorios de verdad, como "map", se comprueban
// aparte en parseDefinition()).
GridCoord parseGridCoord(const JsonValue& value, const GridCoord& defaultValue) {
    if (!value.isObject()) {
        return defaultValue;
    }
    GridCoord coord;
    coord.x = value["x"].asInt(defaultValue.x);
    coord.y = value["y"].asInt(defaultValue.y);
    return coord;
}

Result<LevelDefinition> parseDefinition(const JsonValue& root) {
    if (!root.isObject()) {
        return Result<LevelDefinition>::Error("El JSON de nivel debe ser un objeto en la raiz");
    }

    LevelDefinition level;
    level.name = root["name"].asString("(sin nombre)");
    level.mapPath = root["map"].asString();
    if (level.mapPath.empty()) {
        return Result<LevelDefinition>::Error(
            "El nivel no tiene campo \"map\" (ruta al TMX) o esta vacio");
    }
    level.playerStart = parseGridCoord(root["playerStart"], GridCoord{0, 0});

    const JsonValue& enemiesValue = root["enemies"];
    if (!enemiesValue.isNull() && !enemiesValue.isArray()) {
        return Result<LevelDefinition>::Error("\"enemies\" debe ser un array (o estar ausente)");
    }
    for (std::size_t i = 0; i < enemiesValue.size(); ++i) {
        const JsonValue& enemyValue = enemiesValue[i];
        EnemySpawn spawn;
        spawn.type = enemyValue["type"].asString();
        if (spawn.type.empty()) {
            return Result<LevelDefinition>::Error("enemies[" + std::to_string(i) +
                                                  "] no tiene campo \"type\" (o esta vacio)");
        }
        spawn.position = parseGridCoord(enemyValue["position"], GridCoord{0, 0});
        // Sin patrulla explicita en el JSON: patrolMin==patrolMax==
        // position (enemigo quieto, ver el comentario de EnemySpawn en
        // LevelDefinition.h).
        spawn.patrolMin = parseGridCoord(enemyValue["patrolMin"], spawn.position);
        spawn.patrolMax = parseGridCoord(enemyValue["patrolMax"], spawn.position);

        const JsonValue& skillsValue = enemyValue["skills"];
        for (std::size_t s = 0; s < skillsValue.size(); ++s) {
            std::string skillId = skillsValue[s].asString();
            if (!skillId.empty()) {
                spawn.skillIds.push_back(std::move(skillId));
            }
        }
        level.enemies.push_back(std::move(spawn));
    }

    // "objects" (Fase 10): mismo reparto obligatorio/opcional que
    // "enemies" -- el array puede faltar (nivel solo con "enemies", o
    // vacio), pero si esta, cada entrada necesita su "objectId". El id
    // NO se valida contra un ObjectCatalog aqui: LevelLoader no sabe que
    // catalogo usara quien instancie el nivel (puede cargar varios, ver
    // ObjectCatalog::loadFromString), asi que resolver ids es
    // responsabilidad del consumidor -- mismo criterio que EnemySpawn::
    // type y skillIds (texto libre contra catalogos externos).
    const JsonValue& objectsValue = root["objects"];
    if (!objectsValue.isNull() && !objectsValue.isArray()) {
        return Result<LevelDefinition>::Error("\"objects\" debe ser un array (o estar ausente)");
    }
    for (std::size_t i = 0; i < objectsValue.size(); ++i) {
        const JsonValue& objectValue = objectsValue[i];
        ObjectSpawn spawn;
        spawn.objectId = objectValue["objectId"].asString();
        if (spawn.objectId.empty()) {
            return Result<LevelDefinition>::Error("objects[" + std::to_string(i) +
                                                  "] no tiene campo \"objectId\" (o esta vacio)");
        }
        spawn.position = parseGridCoord(objectValue["position"], GridCoord{0, 0});
        spawn.patrolMin = parseGridCoord(objectValue["patrolMin"], spawn.position);
        spawn.patrolMax = parseGridCoord(objectValue["patrolMax"], spawn.position);

        // Transicion opcional (ver ObjectSpawn::targetLevel). La ruta NO
        // se valida aqui -- que exista ese archivo es cosa de quien
        // ejecute la transicion, igual que objectId no se valida contra
        // el catalogo (ver el comentario de arriba).
        spawn.targetLevel = objectValue["targetLevel"].asString();
        const JsonValue& targetPos = objectValue["targetPosition"];
        spawn.hasTargetPosition = targetPos.isObject();
        spawn.targetPosition = parseGridCoord(targetPos, spawn.position);

        level.objects.push_back(std::move(spawn));
    }

    return Result<LevelDefinition>::Ok(std::move(level));
}

}  // namespace

Result<LevelDefinition> LevelLoader::loadFromString(const std::string& jsonText) {
    Result<JsonValue> parsed = JsonValue::parse(jsonText);
    if (!parsed.isOk()) {
        return Result<LevelDefinition>::Error(parsed.errorMessage());
    }
    return parseDefinition(parsed.value());
}

Result<LevelDefinition> LevelLoader::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return Result<LevelDefinition>::Error("No se pudo abrir el archivo: " + path);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return loadFromString(buffer.str());
}
