#pragma once

// Una celda de TileMap: que region del atlas dibujar (ver TextureAtlas)
// y si bloquea el movimiento. m_tilesetID es el GID de Tiled (1-based,
// 0 = celda vacia, sin dibujar); TextureAtlas::getUV() lo resuelve a un
// UVRect concreto en tiempo de render, TileMap no conoce el atlas.
class Tile {
public:
    Tile() = default;
    Tile(int tilesetID, bool collision, int variant = 0)
        : m_tilesetID(tilesetID), m_variant(variant), m_collision(collision) {}

    bool isEmpty() const { return m_tilesetID == 0; }
    bool hasCollision() const { return m_collision; }
    int getTilesetID() const { return m_tilesetID; }
    int getVariant() const { return m_variant; }

private:
    int m_tilesetID = 0;
    int m_variant = 0;
    bool m_collision = false;
};
