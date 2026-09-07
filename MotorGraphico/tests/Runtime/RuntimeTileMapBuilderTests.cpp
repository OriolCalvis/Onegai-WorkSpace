#include <vector>
#include <stdexcept>

#include "Map/MapDocument.h"
#include "Runtime/RuntimeTileMapBuilder.h"
#include "TestHarness.h"

namespace {

template <class Id>
Id id(const char* value) {
    const Result<Id> result = Id::parse(value);
    if (!result.isOk()) {
        throw std::runtime_error(result.errorMessage());
    }
    return result.value();
}

MapDocument makeDocument() {
    MapDocument document(id<ids::MapId>("pradera"), "Pradera", 3, 2, 64, 32);
    document.addLayer(MapLayer::tiles({id<ids::LayerId>("suelo"), "Suelo", true, false, 0.75f, 10},
                                      TileLayerDocument(3, 2, {1, 2, 3, 4, 5, 6})));
    document.addLayer(MapLayer::entities(
        {id<ids::LayerId>("entidades"), "Entidades", true, false, 1.0f, 20},
        EntityLayerDocument(
            {{id<ids::EntityId>("guia"), id<ids::AssetId>("npc_guia"), {1, 1}}})));
    document.addLayer(MapLayer::regions(
        {id<ids::LayerId>("regiones"), "Regiones", true, false, 1.0f, 30},
        RegionLayerDocument({{id<ids::RegionId>("salida"), "Salida", {2, 2, 0, 1}}})));
    document.setCollisionGids({2});
    return document;
}

MOTOR_TEST_CASE(builder_compiles_only_tile_layers_from_valid_document) {
    RuntimeTileMapBuilder builder;
    const Result<RuntimeTileMap> result = builder.build(makeDocument());

    MOTOR_CHECK(result.isOk());
    const RuntimeTileMap& runtime = result.value();
    MOTOR_CHECK(runtime.getWidth() == 3);
    MOTOR_CHECK(runtime.getHeight() == 2);
    MOTOR_CHECK(runtime.getLayerCount() == 1);
    MOTOR_CHECK(runtime.layerAt(0).sourceLayerId == id<ids::LayerId>("suelo"));
    MOTOR_CHECK(runtime.layerAt(0).opacity == 0.75f);
    MOTOR_CHECK(runtime.getTile(0, 2, 1).getTilesetID() == 6);
    MOTOR_CHECK(!runtime.getTile(0, 2, 1).hasCollision());
    MOTOR_CHECK(runtime.getTile(0, 1, 0).hasCollision());
}

MOTOR_TEST_CASE(compiled_runtime_is_independent_from_later_document_edits) {
    MapDocument document = makeDocument();
    RuntimeTileMapBuilder builder;
    const Result<RuntimeTileMap> first = builder.build(document);
    MOTOR_CHECK(first.isOk());

    MOTOR_CHECK(document.setTile(id<ids::LayerId>("suelo"), {1, 0}, 99).isOk());
    MOTOR_CHECK(document.setCollisionGids({99}).isOk());
    const Result<RuntimeTileMap> rebuilt = builder.build(document);
    MOTOR_CHECK(rebuilt.isOk());

    MOTOR_CHECK(first.value().getTile(0, 1, 0).getTilesetID() == 2);
    MOTOR_CHECK(first.value().getTile(0, 1, 0).hasCollision());
    MOTOR_CHECK(rebuilt.value().getTile(0, 1, 0).getTilesetID() == 99);
    MOTOR_CHECK(rebuilt.value().getTile(0, 1, 0).hasCollision());
}

MOTOR_TEST_CASE(runtime_preserves_isometric_coordinates_and_checked_access) {
    const Result<RuntimeTileMap> result = RuntimeTileMapBuilder{}.build(makeDocument());
    MOTOR_CHECK(result.isOk());
    const RuntimeTileMap& runtime = result.value();
    MOTOR_CHECK(runtime.getTileWidth() == 64);
    MOTOR_CHECK(runtime.getTileHeight() == 32);
    const Vector2 screen = runtime.gridToScreen({2, 1});
    MOTOR_CHECK(screen.x == 32.0f);
    MOTOR_CHECK(screen.y == 48.0f);
    const GridCoord grid = runtime.screenToGrid(screen);
    MOTOR_CHECK(grid.x == 2 && grid.y == 1);

    const auto rejects = [](auto query) {
        try {
            query();
        } catch (const std::out_of_range&) {
            return true;
        }
        return false;
    };
    MOTOR_CHECK(rejects([&] { runtime.layerAt(-1); }));
    MOTOR_CHECK(rejects([&] { runtime.layerAt(1); }));
    MOTOR_CHECK(rejects([&] { runtime.getTile(0, -1, 0); }));
    MOTOR_CHECK(rejects([&] { runtime.getTile(0, 0, -1); }));
    MOTOR_CHECK(rejects([&] { runtime.getTile(0, 3, 0); }));
    MOTOR_CHECK(rejects([&] { runtime.getTile(0, 0, 2); }));
}

MOTOR_TEST_CASE(builder_rejects_an_invalid_document) {
    RuntimeTileMapBuilder builder;
    const Result<RuntimeTileMap> result =
        builder.build(MapDocument(ids::MapId{}, "Sin identidad", 2, 2, 64, 32));

    MOTOR_CHECK(!result.isOk());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
