#include "Runtime/RuntimeTileMap.h"

#include <algorithm>
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

int RuntimeTileMap::findLayerIndex(const ids::LayerId& layerId) const {
    for (std::size_t index = 0; index < m_layers.size(); ++index) {
        if (m_layers[index].sourceLayerId == layerId) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

Result<bool> RuntimeTileMap::setTileOverride(const ids::LayerId& layerId, GridCoord position,
                                             RuntimeTile tile) {
    if (tile.getTilesetID() < 0) {
        return Result<bool>::Error("un override no puede tener un GID negativo");
    }
    const int layer = findLayerIndex(layerId);
    if (layer < 0) {
        return Result<bool>::Error("no existe la capa runtime '" + layerId.value() + "'");
    }
    if (position.x < 0 || position.y < 0 || position.x >= m_width || position.y >= m_height) {
        return Result<bool>::Error("la posición del override está fuera del mapa");
    }

    m_layers[static_cast<std::size_t>(layer)].tiles[tileIndex(position.x, position.y)] = tile;
    const auto existing = std::find_if(
        m_overrides.begin(), m_overrides.end(), [&layerId, position](const RuntimeTileOverride& it) {
            return it.layerId == layerId && it.position.x == position.x && it.position.y == position.y;
        });
    if (existing != m_overrides.end()) {
        existing->tile = tile;
    } else {
        m_overrides.push_back(RuntimeTileOverride{layerId, position, tile});
    }
    return Result<bool>::Ok(true);
}

void RuntimeTileMap::clearTileOverrides() {
    m_layers = m_baseLayers;
    m_overrides.clear();
}

void RuntimeTileMap::replaceCompiledData(int width, int height, int tileWidth, int tileHeight,
                                         std::vector<RuntimeTileLayer> layers) {
    m_width = width;
    m_height = height;
    m_tileWidth = tileWidth;
    m_tileHeight = tileHeight;
    m_baseLayers = layers;
    m_layers = std::move(layers);
    m_overrides.clear();
}
