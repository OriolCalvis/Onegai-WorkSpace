#pragma once

// Celda compacta consumida por el runtime. No contiene UVs, texturas ni
// recursos OpenGL: el renderer resuelve el GID contra su atlas cuando dibuja.
class RuntimeTile {
public:
    RuntimeTile() = default;
    RuntimeTile(int tilesetID, bool collision, int variant = 0)
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
