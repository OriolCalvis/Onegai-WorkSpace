#include "Runtime/RuntimeTileMap.h"

#include <stdexcept>
#include <utility>

#include "Core/Math/IsoMath.h"

const RuntimeTileLayer& RuntimeTileMap::layerAt(int layer) const {
    if (layer < 0 || layer >= static_cast<int>(m_layers.size())) {
        throw std::out_of_range("RuntimeTileMap::layerAt(" + std::to_string(layer) +
                                ") fuera de rango");
    }
    return m_layers[static_cast<std::size_t>(layer)];
}

std::size_t RuntimeTileMap::tileIndex(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        throw std::out_of_range("RuntimeTileMap::getTile fuera de rango");
    }
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) +
           static_cast<std::size_t>(x);
}

const RuntimeTile& RuntimeTileMap::getTile(int layer, int x, int y) const {
    const RuntimeTileLayer& selectedLayer = layerAt(layer);
    return selectedLayer.tiles[tileIndex(x, y)];
}

GridCoord RuntimeTileMap::screenToGrid(const Vector2& screenPos) const {
    return IsoMath::screenToGrid(screenPos, static_cast<float>(m_tileWidth),
                                 static_cast<float>(m_tileHeight));
}

Vector2 RuntimeTileMap::gridToScreen(const GridCoord& grid) const {
    return IsoMath::gridToScreen(grid, static_cast<float>(m_tileWidth),
                                 static_cast<float>(m_tileHeight));
}

void RuntimeTileMap::replaceCompiledData(int width, int height, int tileWidth, int tileHeight,
                                         std::vector<RuntimeTileLayer> layers) {
    m_width = width;
    m_height = height;
    m_tileWidth = tileWidth;
    m_tileHeight = tileHeight;
    m_layers = std::move(layers);
}
