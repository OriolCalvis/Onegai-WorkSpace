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
    if (root.has("mapDocument")) {
        if (!root["mapDocument"].isString() || root["mapDocument"].asString().empty()) {
            return Result<LevelDefinition>::Error("mapDocument debe ser una ruta no vacía");
        }
        level.mapDocumentPath = root["mapDocument"].asString();
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

        // G4-01 — Puerta cerrada por flags (ver ObjectSpawn::estaCerrada).
        const JsonValue& cerrada = objectValue["cerradaSi"];
        for (std::size_t c = 0; c < cerrada["todas"].size(); ++c)
            spawn.cerradaTodas.push_back(cerrada["todas"][c].asString());
        for (std::size_t c = 0; c < cerrada["ninguna"].size(); ++c)
            spawn.cerradaNinguna.push_back(cerrada["ninguna"][c].asString());
        spawn.beatCerrada = objectValue["beatCerrada"].asString();

        // Botin y evento de la instancia: opcionales como el resto de
        // campos locales (ver ObjectSpawn en LevelDefinition.h). Los ids
        // de loot NO se validan contra el catalogo aqui, igual que
        // objectId: resolverlos es cosa de quien instancia el nivel.
        const JsonValue& lootValue = objectValue["loot"];
        for (std::size_t l = 0; l < lootValue.size(); ++l) {
            std::string lootId = lootValue[l].asString();
            if (!lootId.empty()) {
                spawn.loot.push_back(std::move(lootId));
            }
        }
        spawn.lootTable = objectValue["lootTable"].asString();
        spawn.eventId = objectValue["eventId"].asString();
        spawn.disponibleDia = objectValue["disponibleDia"].asInt(0);  // AV18-07 (PO-52)

        // Overrides por instancia: todo es opcional para que el catalogo
        // siga siendo la fuente de los valores por defecto y los niveles
        // existentes carguen exactamente igual que antes.
        spawn.displayName = objectValue["displayName"].asString();
        spawn.variant = objectValue["variant"].asString();
        spawn.scale = static_cast<float>(objectValue["scale"].asNumber(1.0));
        if (spawn.scale <= 0.0f) {
            spawn.scale = 1.0f;
        }
        spawn.effectOverride = objectValue["effectOverride"].asString();
        const JsonValue& properties = objectValue["properties"];
        if (properties.isObject()) {
            for (const auto& pair : properties.objectValues()) {
                if (!pair.first.empty() && pair.second.isString()) {
                    spawn.properties[pair.first] = pair.second.asString();
                }
            }
        }

        level.objects.push_back(std::move(spawn));
    }

    // C-23 — Post-processat opcional per nivell. "fog" i "lighting"
    // son objectes opcionals: si no hi son, els camps queden als
    // valors per defecte (disabled) i Application no fa cap crida GL.
    const JsonValue& fogValue = root["fog"];
    if (fogValue.isObject()) {
        level.fogEnabled = fogValue["enabled"].asBool(false);
        level.fogRevealRadius = fogValue["revealRadius"].asInt(1);
    }
    const JsonValue& lightingValue = root["lighting"];
    if (lightingValue.isObject()) {
        level.lightingEnabled = lightingValue["enabled"].asBool(false);
        level.ambientR = static_cast<float>(lightingValue["ambientR"].asNumber(0.35));
        level.ambientG = static_cast<float>(lightingValue["ambientG"].asNumber(0.35));
        level.ambientB = static_cast<float>(lightingValue["ambientB"].asNumber(0.40));
    }

    // AV19-02 — Altura del terreno (opcional): regiones pintadas en orden, con rampa si traen «hasta».
    const JsonValue& alturasValue = root["alturas"];
    if (!alturasValue.isNull() && !alturasValue.isArray()) {
        return Result<LevelDefinition>::Error("\"alturas\" debe ser un array (o estar ausente)");
    }
    for (std::size_t i = 0; i < alturasValue.size(); ++i) {
        const JsonValue& a = alturasValue[i];
        const std::string donde = "alturas[" + std::to_string(i) + "]";
        if (!a.isObject() || !a.has("x") || !a.has("y") || !a.has("nivel")) {
            return Result<LevelDefinition>::Error(donde + " necesita x, y y nivel");
        }
        AlturaRegion r;
        r.x = a["x"].asInt(0);
        r.y = a["y"].asInt(0);
        r.w = a["w"].asInt(1);
        r.h = a["h"].asInt(1);
        r.nivel = a["nivel"].asInt(0);
        if (r.w < 1 || r.h < 1) return Result<LevelDefinition>::Error(donde + ": w y h deben ser 1 o mas");
        if (r.nivel < 0 || r.nivel > Alturas::kMax)
            return Result<LevelDefinition>::Error(donde + ": nivel fuera de 0.." + std::to_string(Alturas::kMax));
        if (a.has("hasta")) {
            r.conRampa = true;
            r.hasta = a["hasta"].asInt(0);
            if (r.hasta < 0 || r.hasta > Alturas::kMax)
                return Result<LevelDefinition>::Error(donde + ": hasta fuera de 0.." + std::to_string(Alturas::kMax));
            const std::string eje = a["eje"].asString("y");
            if (eje != "x" && eje != "y") return Result<LevelDefinition>::Error(donde + ": eje debe ser \"x\" o \"y\"");
            r.eje = eje[0];
        }
        level.alturas.push_back(r);
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
