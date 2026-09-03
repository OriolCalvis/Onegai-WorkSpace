#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Runtime/RuntimeTile.h"

class RuntimeTileMapBuilder;

// Capa de runtime compilada desde un MapDocument. tiles mantiene una sola
// dimensión fila-major y el ID de origen permite aplicar overrides de partida
// sin volver a mirar ni mutar el documento de autoría.
struct RuntimeTileLayer {
    ids::LayerId sourceLayerId;
    bool visible = true;
    float opacity = 1.0f;
    std::vector<RuntimeTile> tiles;
};

struct RuntimeTileOverride {
    ids::LayerId layerId;
    GridCoord position;
    RuntimeTile tile;
};

// Representación compacta y mutable solo a nivel de partida. El mapa base
// compilado permanece intacto: setTileOverride() registra cambios de sesión y
// clearTileOverrides() recupera exactamente el resultado del builder.
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
    Result<bool> setTileOverride(const ids::LayerId& layerId, GridCoord position,
                                 RuntimeTile tile);
    void clearTileOverrides();
    const std::vector<RuntimeTileOverride>& tileOverrides() const { return m_overrides; }

protected:
    // Protegido para que TileMap pueda seguir importando TMX como adaptador
    // temporal. Solo RuntimeTileMapBuilder y ese adaptador construyen datos
    // de base; los consumidores solo los consultan u ofrecen overrides.
    void replaceCompiledData(int width, int height, int tileWidth, int tileHeight,
                             std::vector<RuntimeTileLayer> layers);

private:
    friend class RuntimeTileMapBuilder;

    int findLayerIndex(const ids::LayerId& layerId) const;
    std::size_t tileIndex(int x, int y) const;

    int m_width = 0;
    int m_height = 0;
    int m_tileWidth = 0;
    int m_tileHeight = 0;
    std::vector<RuntimeTileLayer> m_baseLayers;
    std::vector<RuntimeTileLayer> m_layers;
    std::vector<RuntimeTileOverride> m_overrides;
};
