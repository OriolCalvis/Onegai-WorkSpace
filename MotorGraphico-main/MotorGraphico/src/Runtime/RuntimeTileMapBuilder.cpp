#include "Runtime/RuntimeTileMapBuilder.h"

#include <algorithm>
#include <utility>
#include <variant>
#include <vector>

Result<RuntimeTileMap> RuntimeTileMapBuilder::build(const MapDocument& document) const {
    const Result<bool> documentResult = document.validate();
    if (!documentResult.isOk()) {
        return Result<RuntimeTileMap>::Error("no se puede compilar MapDocument: " +
                                              documentResult.errorMessage());
    }

    std::vector<RuntimeTileLayer> compiledLayers;
    for (const MapLayer& layer : document.layers()) {
        if (layer.type() != MapLayerType::Tile) {
            continue;
        }

        const TileLayerDocument& tileData = std::get<TileLayerDocument>(layer.data());
        RuntimeTileLayer compiled;
        compiled.sourceLayerId = layer.meta().id;
        compiled.visible = layer.meta().visible;
        compiled.opacity = layer.meta().opacity;
        compiled.tiles.reserve(tileData.gids().size());
        for (int gid : tileData.gids()) {
            const bool collision = std::binary_search(document.collisionGids().begin(),
                                                      document.collisionGids().end(), gid);
            compiled.tiles.emplace_back(gid, collision);
        }
        compiledLayers.push_back(std::move(compiled));
    }

    RuntimeTileMap runtime;
    runtime.replaceCompiledData(document.width(), document.height(), document.tileWidth(),
                                document.tileHeight(), std::move(compiledLayers));
    return Result<RuntimeTileMap>::Ok(std::move(runtime));
}
