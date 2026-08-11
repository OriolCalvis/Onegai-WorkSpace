// Fase 6 (motor_grafico_gantt_rpg.puml): sistema de niveles y contenido en
// JSON. GL-free (igual que demo_camera.cpp/demo_tilemap.cpp): JsonValue y
// LevelLoader no dependen de OpenGL/GLFW, asi que este demo compila y
// corre en cualquier maquina con g++/clang++ C++17, sin ventana ni
// contexto GL.
//
// Verifica, en dos bloques:
//  1. JsonValue::parse(): objetos/arrays/strings/numeros/bool/null
//     anidados, accesores con default seguro ante tipos/campos que no
//     coinciden, y que un JSON invalido produce Result::Error (no lanza).
//  2. LevelLoader::loadFromString()/loadFromFile(): el esquema de nivel
//     completo (nombre, mapa, playerStart, enemies con patrulla y
//     habilidades opcionales), valores por defecto de campos opcionales,
//     y los errores fatales (falta "map", "enemies" no es array, JSON
//     invalido) devueltos como Result::Error con mensaje.
#include "Core/Json/JsonValue.h"
#include "Level/LevelLoader.h"

#include "Check.h"

#include <iostream>

namespace {

void testJsonValue() {
    // Documento anidado: objeto con array, numeros, bool, string, null.
    auto parsed = JsonValue::parse(R"({
        "name": "prueba",
        "count": 3,
        "ratio": 1.5,
        "active": true,
        "missing": null,
        "tags": ["a", "b", "c"],
        "nested": { "x": 10, "y": -2 }
    })");
    require(parsed.isOk());
    const JsonValue& root = parsed.value();
    require(root.isObject());
    require(root["name"].asString() == "prueba");
    require(root["count"].asInt() == 3);
    require(root["ratio"].asNumber() == 1.5);
    require(root["active"].asBool() == true);
    require(root["missing"].isNull());
    require(root["tags"].isArray());
    require(root["tags"].size() == 3);
    require(root["tags"][0].asString() == "a");
    require(root["tags"][2].asString() == "c");
    require(root["nested"]["x"].asInt() == 10);
    require(root["nested"]["y"].asInt() == -2);

    // Accesores con default: clave/indice ausente, o tipo que no
    // coincide, devuelven el default en vez de lanzar.
    require(root["no_existe"].isNull());
    require(root["no_existe"].asInt(42) == 42);
    require(root["name"].asInt(99) == 99);       // "prueba" no es Number
    require(root["tags"][99].isNull());          // indice fuera de rango
    require(root["count"].asString("x") == "x");  // 3 no es String

    // has()/size() sobre objeto.
    require(root.has("name"));
    require(!root.has("no_existe"));
    require(root.size() == 7);  // name, count, ratio, active, missing, tags, nested

    // Escapes de string soportados.
    auto escaped = JsonValue::parse(R"({"s": "linea1\nlinea2\t\"cita\""})");
    require(escaped.isOk());
    require(escaped.value()["s"].asString() == "linea1\nlinea2\t\"cita\"");

    // JSON invalido: Result::Error, no excepcion.
    auto badJson = JsonValue::parse("{ \"a\": }");
    require(!badJson.isOk());
    require(!badJson.errorMessage().empty());

    auto trailingGarbage = JsonValue::parse("{}  extra");
    require(!trailingGarbage.isOk());

    std::cout << "[JSON] parse(), accesores con default y errores de sintaxis correctos.\n";
}

void testLevelLoader() {
    // Nivel completo, con dos enemigos: uno con patrulla explicita y
    // habilidades, otro sin patrulla (queda en su "position") y sin
    // habilidades.
    const std::string levelJson = R"({
        "name": "Cueva de entrada",
        "map": "assets/maps/test_map.tmx",
        "playerStart": { "x": 1, "y": 1 },
        "enemies": [
            {
                "type": "slime",
                "position": { "x": 3, "y": 0 },
                "patrolMin": { "x": 0, "y": 0 },
                "patrolMax": { "x": 3, "y": 0 },
                "skills": ["golpe_gelatinoso"]
            },
            {
                "type": "goblin",
                "position": { "x": 2, "y": 2 }
            }
        ]
    })";

    auto result = LevelLoader::loadFromString(levelJson);
    require(result.isOk());
    const LevelDefinition& level = result.value();
    require(level.name == "Cueva de entrada");
    require(level.mapPath == "assets/maps/test_map.tmx");
    require(level.playerStart.x == 1 && level.playerStart.y == 1);
    require(level.enemies.size() == 2);

    const EnemySpawn& slime = level.enemies[0];
    require(slime.type == "slime");
    require(slime.position.x == 3 && slime.position.y == 0);
    require(slime.patrolMin.x == 0 && slime.patrolMax.x == 3);
    require(slime.skillIds.size() == 1 && slime.skillIds[0] == "golpe_gelatinoso");

    const EnemySpawn& goblin = level.enemies[1];
    require(goblin.type == "goblin");
    // Sin patrolMin/patrolMax en el JSON: ambos caen en position (sin
    // patrulla, ver el comentario de EnemySpawn en LevelDefinition.h).
    require(goblin.patrolMin.x == goblin.position.x && goblin.patrolMin.y == goblin.position.y);
    require(goblin.patrolMax.x == goblin.position.x && goblin.patrolMax.y == goblin.position.y);
    require(goblin.skillIds.empty());

    std::cout << "[LEVEL] loadFromString() con patrulla/habilidades opcionales correcto.\n";

    // "name" opcional: falta -> "(sin nombre)".
    auto noName = LevelLoader::loadFromString(R"({"map": "x.tmx"})");
    require(noName.isOk());
    require(noName.value().name == "(sin nombre)");
    require(noName.value().enemies.empty());

    // Errores fatales: falta "map", "enemies" con tipo incorrecto, enemigo
    // sin "type", JSON invalido -- todos Result::Error con mensaje, nunca
    // una excepcion sin capturar.
    auto noMap = LevelLoader::loadFromString(R"({"name": "sin mapa"})");
    require(!noMap.isOk());

    auto badEnemies = LevelLoader::loadFromString(R"({"map": "x.tmx", "enemies": 5})");
    require(!badEnemies.isOk());

    auto enemyNoType = LevelLoader::loadFromString(R"({"map": "x.tmx", "enemies": [{}]})");
    require(!enemyNoType.isOk());

    auto invalidJson = LevelLoader::loadFromString("{ not json");
    require(!invalidJson.isOk());

    std::cout << "[LEVEL] defaults de campos opcionales y errores fatales correctos.\n";

    // loadFromFile() contra el JSON de ejemplo del repo. Ruta relativa,
    // igual que assets/maps/test_map.tmx en demo_tilemap.cpp: se espera
    // ejecutar con cwd = MotorGraphico/ (o build/, donde CMake copia
    // assets/, ver CMakeLists.txt). A diferencia de demo_tilemap.cpp
    // (que SI trata el fallo como fatal, porque necesita el mapa para el
    // resto del demo), aqui el resto de las comprobaciones no dependen
    // de este archivo -- asi que un fallo de ruta se avisa mas que
    // aborta, para no perder el resto de la cobertura si este demo se
    // ejecuta a mano desde un directorio distinto.
    auto fromFile = LevelLoader::loadFromFile("assets/levels/test_level.json");
    if (fromFile.isOk()) {
        require(fromFile.value().name == "Cueva de entrada");
        require(fromFile.value().enemies.size() == 2);
        std::cout << "[LEVEL] loadFromFile(\"assets/levels/test_level.json\") correcto.\n";
    } else {
        std::cout << "[LEVEL] aviso: no se pudo abrir assets/levels/test_level.json desde el "
                     "directorio actual (" << fromFile.errorMessage()
                  << "); se omite (no es un fallo del parser).\n";
    }
}

}  // namespace

int main() {
    testJsonValue();
    testLevelLoader();
    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
