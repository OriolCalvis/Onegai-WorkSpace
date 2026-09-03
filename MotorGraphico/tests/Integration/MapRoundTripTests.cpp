#include <cstdio>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <variant>

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

}  // namespace

int main() { return motor_test::runAllTests(); }
