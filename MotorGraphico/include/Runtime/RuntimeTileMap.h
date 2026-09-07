#pragma once

#include <vector>

#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Runtime/RuntimeTile.h"

class RuntimeTileMapBuilder;

// Capa de runtime compilada desde un MapDocument. tiles mantiene una sola
// dimensión fila-major y conserva la identidad de la capa de autoría.
struct RuntimeTileLayer {
    ids::LayerId sourceLayerId;
    bool visible = true;
    float opacity = 1.0f;
    std::vector<RuntimeTile> tiles;
};

// Representación compilada de consulta: cada capa tiene una única copia de
// sus tiles, independiente del documento de autoría.
class RuntimeTileMap {
public:
    RuntimeTileMap() = default;

    // C-09: TileMap hereda de RuntimeTileMap y el renderer guarda
    // punteros a la base (IsometricRenderer, HudMinimap): sin
    // destructor virtual, destruir por la base es indefinido.
    virtual ~RuntimeTileMap() = default;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getTileWidth() const { return m_tileWidth; }
    int getTileHeight() const { return m_tileHeight; }
    int getLayerCount() const { return static_cast<int>(m_layers.size()); }

    const RuntimeTileLayer& layerAt(int layer) const;
    const RuntimeTile& getTile(int layer, int x, int y) const;

    GridCoord screenToGrid(const Vector2& screenPos) const;
    Vector2 gridToScreen(const GridCoord& grid) const;

protected:
    // Protegido para que TileMap pueda seguir importando TMX como adaptador
    // temporal. Solo RuntimeTileMapBuilder y ese adaptador construyen datos
    // de base; los consumidores solo los consultan.
    void replaceCompiledData(int width, int height, int tileWidth, int tileHeight,
                             std::vector<RuntimeTileLayer> layers);

private:
    friend class RuntimeTileMapBuilder;

    std::size_t tileIndex(int x, int y) const;

    int m_width = 0;
    int m_height = 0;
    int m_tileWidth = 0;
    int m_tileHeight = 0;
    std::vector<RuntimeTileLayer> m_layers;
};
