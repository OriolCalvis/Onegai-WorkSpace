#include <cstdio>
#include <fstream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Import/TmxMapImporter.h"
#include "Project/ProjectSerializer.h"
#include "Runtime/RuntimeTileMapBuilder.h"
#include "TestHarness.h"

namespace {

ids::MapId mapId(const char* value) {
    const Result<ids::MapId> parsed = ids::MapId::parse(value);
    if (!parsed.isOk()) {
        throw std::runtime_error(parsed.errorMessage());
    }
    return parsed.value();
}

std::string readText(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
}

const TileLayerDocument& onlyTileLayer(const MapDocument& document) {
    if (document.layers().size() != 1 || document.layers()[0].type() != MapLayerType::Tile) {
        throw std::runtime_error("fixture esperaba una única capa tile");
    }
    return std::get<TileLayerDocument>(document.layers()[0].data());
}

void checkEquivalentMap(const MapDocument& actual, const MapDocument& expected) {
    MOTOR_CHECK(actual.id() == expected.id());
    MOTOR_CHECK(actual.name() == expected.name());
    MOTOR_CHECK(actual.width() == expected.width());
    MOTOR_CHECK(actual.height() == expected.height());
    MOTOR_CHECK(actual.tileWidth() == expected.tileWidth());
    MOTOR_CHECK(actual.tileHeight() == expected.tileHeight());
    MOTOR_CHECK(actual.collisionGids() == expected.collisionGids());
    MOTOR_CHECK(actual.layers().size() == expected.layers().size());
    MOTOR_CHECK(actual.layers()[0].meta().id == expected.layers()[0].meta().id);
    MOTOR_CHECK(onlyTileLayer(actual).gids() == onlyTileLayer(expected).gids());
}

MOTOR_TEST_CASE(tmx_json_runtime_pipeline_matches_the_reviewable_fixture) {
    TmxMapImporter importer;
    const TmxMapImportReport imported = importer.importFile("assets/maps/test_map.tmx", mapId("test_map"));
    MOTOR_CHECK(!imported.hasErrors());

    const Result<MapDocument> expected =
        ProjectSerializer::deserializeMap(readText("tests/fixtures/maps/expected-map-v1.json"));
    MOTOR_CHECK(expected.isOk());
    checkEquivalentMap(imported.document, expected.value());

    const std::string savedPath = "map_round_trip_fixture.json";
    std::remove(savedPath.c_str());
    std::remove((savedPath + ".tmp").c_str());
    MOTOR_CHECK(ProjectSerializer::saveMapAtomic(savedPath, imported.document).isOk());
    const Result<MapDocument> loaded = ProjectSerializer::loadMapFile(savedPath);
    MOTOR_CHECK(loaded.isOk());
    checkEquivalentMap(loaded.value(), expected.value());

    const Result<RuntimeTileMap> runtime = RuntimeTileMapBuilder().build(loaded.value());
    MOTOR_CHECK(runtime.isOk());
    MOTOR_CHECK(runtime.value().getTile(0, 1, 1).getTilesetID() == 2);
    MOTOR_CHECK(runtime.value().getTile(0, 1, 1).hasCollision());
    MOTOR_CHECK(runtime.value().getTile(0, 0, 0).getTilesetID() == 1);
    MOTOR_CHECK(!runtime.value().getTile(0, 0, 0).hasCollision());

    std::remove(savedPath.c_str());
}

MOTOR_TEST_CASE(pipeline_detects_an_intentional_tile_difference) {
    TmxMapImporter importer;
    const TmxMapImportReport imported = importer.importFile("assets/maps/test_map.tmx", mapId("test_map"));
    MOTOR_CHECK(!imported.hasErrors());

    std::string altered = readText("tests/fixtures/maps/expected-map-v1.json");
    const std::size_t tiles = altered.find("\"tiles\": [1");
    MOTOR_CHECK(tiles != std::string::npos);
    altered.replace(tiles + std::string("\"tiles\": [").size(), 1, "9");
    const Result<MapDocument> changed = ProjectSerializer::deserializeMap(altered);
    MOTOR_CHECK(changed.isOk());
    MOTOR_CHECK(onlyTileLayer(imported.document).gids() != onlyTileLayer(changed.value()).gids());
}

// =====================================================================
// C-13 — Las entidades del documento no pierden nada.
//
// MapEntityDocument guardaba id, objectId y posición. ObjectSpawn -- lo
// que el juego carga hoy de los niveles reales -- guarda además once
// campos de autoría. Migrar un nivel habría perdido casi todo el trabajo
// del autor EN SILENCIO: el nivel seguiría cargando, solo que las
// puertas no llevarían a ningún sitio y los cofres estarían vacíos.
//
// Esta prueba puebla los once campos y los sigue por todo el camino real
// -- TMX -> MapDocument -> JSON en disco -> MapDocument -- comparándolos
// uno a uno. Si mañana alguien añade un campo a MapEntityDocument y se
// olvida del serializador, el que falla es este test.
// =====================================================================



ids::EntityId entityId(const char* value) {
    const Result<ids::EntityId> parsed = ids::EntityId::parse(value);
    if (!parsed.isOk()) {
        throw std::runtime_error(parsed.errorMessage());
    }
    return parsed.value();
}

ids::AssetId assetId(const char* value) {
    const Result<ids::AssetId> parsed = ids::AssetId::parse(value);
    if (!parsed.isOk()) {
        throw std::runtime_error(parsed.errorMessage());
    }
    return parsed.value();
}

ids::LayerId layerId(const char* value) {
    const Result<ids::LayerId> parsed = ids::LayerId::parse(value);
    if (!parsed.isOk()) {
        throw std::runtime_error(parsed.errorMessage());
    }
    return parsed.value();
}

bool sameCoord(GridCoord a, GridCoord b) { return a.x == b.x && a.y == b.y; }

// La entidad con TODO puesto: los once campos de ObjectSpawn, ninguno en
// su valor por defecto.
MapEntityDocument makeFullEntity() {
    MapEntityDocument entity;
    entity.id = entityId("puerta_posada");
    entity.objectId = assetId("puerta");
    entity.position = GridCoord{1, 1};
    entity.patrolMin = GridCoord{0, 0};
    entity.patrolMax = GridCoord{3, 2};
    entity.targetLevel = "assets/levels/interior_posada.json";
    entity.targetPosition = GridCoord{7, 9};
    entity.hasTargetPosition = true;
    entity.loot = {"llave_oxidada", "pocion"};
    entity.lootTable = "cofres_comunes";
    entity.eventId = "bnd_puerta_posada";
    entity.displayName = "Posada del Cuervo";
    entity.variant = "madera_vieja";
    entity.scale = 1.75f;
    entity.effectOverride = "abrir_con_llave";
    entity.properties = {{"cerrada", "true"}, {"llave", "llave_oxidada"}};
    return entity;
}

// Y la de al lado sin nada: comprueba que los defectos también van y
// vuelven, que es la otra mitad de "sin pérdida".
MapEntityDocument makeBareEntity() {
    MapEntityDocument entity;
    entity.id = entityId("arbusto");
    entity.objectId = assetId("arbusto");
    entity.position = GridCoord{2, 2};
    entity.patrolMin = entity.position;  // "sin patrulla", igual que ObjectSpawn
    entity.patrolMax = entity.position;
    return entity;
}

void checkSameEntity(const MapEntityDocument& actual, const MapEntityDocument& expected) {
    MOTOR_CHECK(actual.id == expected.id);
    MOTOR_CHECK(actual.objectId == expected.objectId);
    MOTOR_CHECK(sameCoord(actual.position, expected.position));
    MOTOR_CHECK(sameCoord(actual.patrolMin, expected.patrolMin));
    MOTOR_CHECK(sameCoord(actual.patrolMax, expected.patrolMax));
    MOTOR_CHECK(actual.targetLevel == expected.targetLevel);
    MOTOR_CHECK(actual.hasTargetPosition == expected.hasTargetPosition);
    if (expected.hasTargetPosition) {
        MOTOR_CHECK(sameCoord(actual.targetPosition, expected.targetPosition));
    }
    MOTOR_CHECK(actual.loot == expected.loot);
    MOTOR_CHECK(actual.lootTable == expected.lootTable);
    MOTOR_CHECK(actual.eventId == expected.eventId);
    MOTOR_CHECK(actual.displayName == expected.displayName);
    MOTOR_CHECK(actual.variant == expected.variant);
    MOTOR_CHECK(actual.scale == expected.scale);
    MOTOR_CHECK(actual.effectOverride == expected.effectOverride);
    MOTOR_CHECK(actual.properties == expected.properties);
}

const EntityLayerDocument& onlyEntityLayer(const MapDocument& document) {
    for (const MapLayer& layer : document.layers()) {
        if (layer.type() == MapLayerType::Entity) {
            return std::get<EntityLayerDocument>(layer.data());
        }
    }
    throw std::runtime_error("el documento no tiene capa de entidades");
}

MOTOR_TEST_CASE(entity_authoring_fields_survive_the_json_round_trip) {
    TmxMapImporter importer;
    const TmxMapImportReport imported =
        importer.importFile("assets/maps/test_map.tmx", mapId("test_map"));
    MOTOR_CHECK(!imported.hasErrors());

    MapDocument authored = imported.document;
    MOTOR_CHECK(authored.addLayer(MapLayer::entities(
                                      {layerId("entidades"), "Entidades", true, false, 1.0f, 20},
                                      EntityLayerDocument({makeFullEntity(), makeBareEntity()})))
                    .isOk());
    MOTOR_CHECK(authored.validate().isOk());

    const std::string savedPath = "map_entity_fields_round_trip.json";
    std::remove(savedPath.c_str());
    std::remove((savedPath + ".tmp").c_str());
    MOTOR_CHECK(ProjectSerializer::saveMapAtomic(savedPath, authored).isOk());

    const Result<MapDocument> loaded = ProjectSerializer::loadMapFile(savedPath);
    MOTOR_CHECK(loaded.isOk());
    MOTOR_CHECK(loaded.value().validate().isOk());

    const EntityLayerDocument& back = onlyEntityLayer(loaded.value());
    MOTOR_CHECK(back.entities().size() == 2);
    checkSameEntity(back.entities()[0], makeFullEntity());
    checkSameEntity(back.entities()[1], makeBareEntity());

    // Y el fichero es estable: guardar lo que se acaba de leer da los
    // mismos bytes. Sin esto, "va y vuelve" podria estar perdiendo algo
    // que la comparacion campo a campo no mirase.
    const std::string secondPath = "map_entity_fields_round_trip_2.json";
    std::remove(secondPath.c_str());
    MOTOR_CHECK(ProjectSerializer::saveMapAtomic(secondPath, loaded.value()).isOk());
    MOTOR_CHECK(readText(savedPath) == readText(secondPath));

    std::remove(savedPath.c_str());
    std::remove(secondPath.c_str());
}

// Compatibilidad: un mapa escrito ANTES de C-13 no trae ninguno de los
// campos nuevos. Tiene que cargar igual, con los defectos -- y con la
// patrulla rellenada a la posicion, que es como LevelLoader expresa "este
// objeto no patrulla" desde la Fase 6.
MOTOR_TEST_CASE(a_map_written_before_the_new_fields_still_loads) {
    const std::string legacy = R"({
      "schemaVersion": 1,
      "id": "test_map",
      "name": "test_map",
      "width": 4,
      "height": 3,
      "tileWidth": 64,
      "tileHeight": 32,
      "collisionGids": [],
      "layers": [
        {
          "id": "entidades", "name": "Entidades", "visible": true, "locked": false,
          "opacity": 1.0, "order": 20, "type": "entity",
          "entities": [
            {"id": "alcaldesa", "objectId": "npc_alcaldesa", "position": {"x": 3, "y": 1}}
          ]
        }
      ]
    })";

    const Result<MapDocument> loaded = ProjectSerializer::deserializeMap(legacy);
    MOTOR_CHECK(loaded.isOk());
    MOTOR_CHECK(loaded.value().validate().isOk());

    const MapEntityDocument& entity = onlyEntityLayer(loaded.value()).entities()[0];
    MOTOR_CHECK(sameCoord(entity.position, GridCoord{3, 1}));
    MOTOR_CHECK(sameCoord(entity.patrolMin, GridCoord{3, 1}));
    MOTOR_CHECK(sameCoord(entity.patrolMax, GridCoord{3, 1}));
    MOTOR_CHECK(entity.targetLevel.empty());
    MOTOR_CHECK(!entity.hasTargetPosition);
    MOTOR_CHECK(entity.loot.empty());
    MOTOR_CHECK(entity.scale == 1.0f);
    MOTOR_CHECK(entity.properties.empty());
}

// Un documento sin campos de autoria produce EXACTAMENTE el mismo JSON
// que antes de C-13: los campos nuevos solo se escriben si difieren del
// defecto, asi que ningun fichero del repositorio cambia por recompilar.
MOTOR_TEST_CASE(entities_without_authoring_fields_write_the_same_json_as_before) {
    MapDocument map(mapId("test_map"), "test_map", 4, 3, 64, 32);
    MapEntityDocument bare = makeBareEntity();
    MOTOR_CHECK(map.addLayer(MapLayer::entities(
                                 {layerId("entidades"), "Entidades", true, false, 1.0f, 20},
                                 EntityLayerDocument({bare})))
                    .isOk());

    const Result<std::string> text = ProjectSerializer::serializeMap(map);
    MOTOR_CHECK(text.isOk());
    MOTOR_CHECK(text.value().find("patrolMin") == std::string::npos);
    MOTOR_CHECK(text.value().find("targetLevel") == std::string::npos);
    MOTOR_CHECK(text.value().find("scale") == std::string::npos);
    MOTOR_CHECK(text.value().find("properties") == std::string::npos);
}


MOTOR_TEST_CASE(malformed_optional_entity_fields_fail_instead_of_disappearing) {
    const std::string prefix = R"({"schemaVersion":1,"id":"test_map","name":"Test",
        "width":4,"height":3,"tileWidth":64,"tileHeight":32,"collisionGids":[],
        "layers":[{"id":"entidades","name":"Entities","visible":true,"locked":false,
        "opacity":1,"order":0,"type":"entity","entities":[
        {"id":"actor","objectId":"npc_alcaldesa","position":{"x":1,"y":1},)";
    const std::string suffix = "}]}]}";
    const std::vector<std::pair<std::string, std::string>> invalid{
        {R"("patrolMin":{"x":0,"y":0})", "patrol"},
        {R"("patrolMax":{"x":2,"y":2})", "patrol"},
        {R"("patrolMin":null,"patrolMax":{"x":2,"y":2})", "patrolMin"},
        {R"("patrolMin":{"x":0.5,"y":0},"patrolMax":{"x":2,"y":2})", "x"},
        {R"("targetPosition":{"x":1e40,"y":0})", "x"},
        {R"("targetPosition":"1,2")", "targetPosition"},
        {R"("targetLevel":false)", "targetLevel"},
        {R"("loot":{})", "loot"},
        {R"("loot":["coin",4])", "loot"},
        {R"("lootTable":[])", "lootTable"},
        {R"("eventId":true)", "eventId"},
        {R"("displayName":17)", "displayName"},
        {R"("variant":{})", "variant"},
        {R"("effectOverride":null)", "effectOverride"},
        {R"("scale":"2")", "scale"},
        {R"("scale":1e100)", "scale"},
        {R"("scale":1e-100)", "scale"},
        {R"("properties":[])", "properties"},
        {R"("properties":{"quest":true})", "properties.quest"},
    };
    for (const auto& entry : invalid) {
        const auto result = ProjectSerializer::deserializeMap(prefix + entry.first + suffix);
        MOTOR_CHECK(!result.isOk());
        MOTOR_CHECK(result.errorMessage().find(entry.second) != std::string::npos);
    }
    // Presentes y vacíos siguen siendo valores válidos, distintos de null.
    const auto valid = ProjectSerializer::deserializeMap(prefix +
        R"("displayName":"","targetLevel":"","properties":{},"loot":[],"scale":1)" + suffix);
    MOTOR_CHECK(valid.isOk());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
