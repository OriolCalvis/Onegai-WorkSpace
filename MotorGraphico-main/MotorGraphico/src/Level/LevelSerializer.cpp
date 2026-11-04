// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-07  (fase G, 2 j) - Congelacion del esquema
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Level/LevelSerializer.h"

#include "Core/Json/JsonWriter.h"

#include <vector>

namespace {

// Escriu un GridCoord com {"x": N, "y": N}.  Coincideix amb parseGridCoord
// (LevelLoader.cpp), que llegeix "x" i "y" com a enters.
void writeGridCoord(JsonWriter& w, const char* key, GridCoord coord) {
    w.key(key);
    w.beginObject();
    w.member("x", coord.x);
    w.member("y", coord.y);
    w.endObject();
}

// Escriu un vector<string> com un array JSON.
void writeStringArray(JsonWriter& w, const char* key,
                      const std::vector<std::string>& values) {
    w.key(key);
    w.beginArray();
    for (const std::string& v : values) {
        w.value(v);
    }
    w.endArray();
}

}  // namespace

std::string serializeLevelDefinition(const LevelDefinition& level) {
    JsonWriter w;
    w.beginObject();

    w.member("name", level.name);
    // El JSON usa "map" (no "mapPath"): el parser ho llegeix com a
    // root["map"].asString().
    w.member("map", level.mapPath);
    if (!level.mapDocumentPath.empty()) {
        w.member("mapDocument", level.mapDocumentPath);
    }
    writeGridCoord(w, "playerStart", level.playerStart);

    // AV19-02: la altura del terreno (solo si la hay, para no ensuciar los niveles planos).
    if (!level.alturas.empty()) {
        w.key("alturas");
        w.beginArray();
        for (const AlturaRegion& r : level.alturas) {
            w.beginObject();
            w.member("x", r.x);
            w.member("y", r.y);
            w.member("w", r.w);
            w.member("h", r.h);
            w.member("nivel", r.nivel);
            if (r.conRampa) {
                w.member("hasta", r.hasta);
                w.member("eje", std::string(1, r.eje));
            }
            w.endObject();
        }
        w.endArray();
    }

    // enemies
    w.key("enemies");
    w.beginArray();
    for (const EnemySpawn& spawn : level.enemies) {
        w.beginObject();
        w.member("type", spawn.type);
        writeGridCoord(w, "position", spawn.position);
        // patrolMin/patrolMax nomes si differiran de position (el
        // parser defaulta a position quan estan absents).
        if (spawn.patrolMin.x != spawn.position.x ||
            spawn.patrolMin.y != spawn.position.y ||
            spawn.patrolMax.x != spawn.position.x ||
            spawn.patrolMax.y != spawn.position.y) {
            writeGridCoord(w, "patrolMin", spawn.patrolMin);
            writeGridCoord(w, "patrolMax", spawn.patrolMax);
        }
        writeStringArray(w, "skills", spawn.skillIds);
        w.endObject();
    }
    w.endArray();

    // objects
    w.key("objects");
    w.beginArray();
    for (const ObjectSpawn& spawn : level.objects) {
        w.beginObject();
        w.member("objectId", spawn.objectId);
        writeGridCoord(w, "position", spawn.position);
        if (spawn.patrolMin.x != spawn.position.x ||
            spawn.patrolMin.y != spawn.position.y ||
            spawn.patrolMax.x != spawn.position.x ||
            spawn.patrolMax.y != spawn.position.y) {
            writeGridCoord(w, "patrolMin", spawn.patrolMin);
            writeGridCoord(w, "patrolMax", spawn.patrolMax);
        }
        if (!spawn.targetLevel.empty()) {
            w.member("targetLevel", spawn.targetLevel);
        }
        if (spawn.hasTargetPosition) {
            writeGridCoord(w, "targetPosition", spawn.targetPosition);
        }
        if (!spawn.cerradaTodas.empty() || !spawn.cerradaNinguna.empty()) {  // G4-01
            w.key("cerradaSi");
            w.beginObject();
            writeStringArray(w, "todas", spawn.cerradaTodas);
            writeStringArray(w, "ninguna", spawn.cerradaNinguna);
            w.endObject();
        }
        if (!spawn.beatCerrada.empty()) {
            w.member("beatCerrada", spawn.beatCerrada);
        }
        if (!spawn.loot.empty()) {
            writeStringArray(w, "loot", spawn.loot);
        }
        if (!spawn.lootTable.empty()) {
            w.member("lootTable", spawn.lootTable);
        }
        if (!spawn.eventId.empty()) {
            w.member("eventId", spawn.eventId);
        }
        if (spawn.disponibleDia > 0) {
            w.member("disponibleDia", spawn.disponibleDia);  // AV18-07 (PO-52)
        }
        if (!spawn.displayName.empty()) {
            w.member("displayName", spawn.displayName);
        }
        if (!spawn.variant.empty()) {
            w.member("variant", spawn.variant);
        }
        if (spawn.scale != 1.0f) {
            w.member("scale", spawn.scale);
        }
        if (!spawn.effectOverride.empty()) {
            w.member("effectOverride", spawn.effectOverride);
        }
        if (!spawn.properties.empty()) {
            // std::map: orden determinista de les claus.
            w.key("properties");
            w.beginObject();
            for (const auto& property : spawn.properties) {
                w.member(property.first, property.second);
            }
            w.endObject();
        }
        w.endObject();
    }
    w.endArray();

    // C-23 — Postprocessat per nivell (opcional).  Només s'escriu quan
    // fogEnabled/lightingEnabled esta activat, perque el parser nomes
    // els llegeix quan l'objecte existeix.
    if (level.fogEnabled) {
        w.key("fog");
        w.beginObject();
        w.member("enabled", level.fogEnabled);
        w.member("revealRadius", level.fogRevealRadius);
        w.endObject();
    }
    if (level.lightingEnabled) {
        w.key("lighting");
        w.beginObject();
        w.member("enabled", level.lightingEnabled);
        w.member("ambientR", level.ambientR);
        w.member("ambientG", level.ambientG);
        w.member("ambientB", level.ambientB);
        w.endObject();
    }

    w.endObject();
    return w.str();
}
