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

MOTOR_TEST_CASE(runtime_tile_overrides_are_session_state_not_document_mutations) {
    MapDocument document = makeDocument();
    RuntimeTileMapBuilder builder;
    Result<RuntimeTileMap> result = builder.build(document);
    MOTOR_CHECK(result.isOk());
    RuntimeTileMap& runtime = result.value();

    MOTOR_CHECK(runtime.setTileOverride(id<ids::LayerId>("suelo"), {1, 0}, RuntimeTile(99, true))
                    .isOk());
    MOTOR_CHECK(runtime.getTile(0, 1, 0).getTilesetID() == 99);
    MOTOR_CHECK(runtime.getTile(0, 1, 0).hasCollision());
    MOTOR_CHECK(runtime.tileOverrides().size() == 1);
    MOTOR_CHECK(std::get<TileLayerDocument>(document.layers()[0].data()).gids()[1] == 2);

    runtime.clearTileOverrides();
    MOTOR_CHECK(runtime.getTile(0, 1, 0).getTilesetID() == 2);
    MOTOR_CHECK(runtime.tileOverrides().empty());
}

MOTOR_TEST_CASE(builder_rejects_an_invalid_document) {
    RuntimeTileMapBuilder builder;
    const Result<RuntimeTileMap> result =
        builder.build(MapDocument(ids::MapId{}, "Sin identidad", 2, 2, 64, 32));

    MOTOR_CHECK(!result.isOk());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
