#pragma once

#include <unordered_map>

#include "Core/Math/UVRect.h"

class Texture;

// Recorta subregiones (columna, fila) de un atlas en un unico Texture y
// devuelve el UVRect correspondiente para SpriteBatch::submit(). No posee
// la textura (puntero no propietario): el ciclo de vida lo gestiona
// TextureManager, igual que en el resto del motor.
class TextureAtlas {
public:
    TextureAtlas(Texture* texture, int tileWidth, int tileHeight);

    // Registra la region (col, row) del atlas bajo "id" (normalmente el
    // GID de Tiled que tambien usa Tile::getTilesetID()).
    void defineRegion(int id, int col, int row);

    // UVRect ya calculado para "id". Si "id" no se registro devuelve la
    // region completa (0,0,1,1) -- UVRect{} por defecto -- en vez de
    // lanzar: es una consulta de hot path (una por sprite y frame) y un
    // id sin definir se ve como "textura entera" en vez de crashear.
    UVRect getUV(int id) const;

    Texture* texture() const { return m_texture; }

private:
    Texture* m_texture;
    int m_tileWidth;
    int m_tileHeight;
    std::unordered_map<int, UVRect> m_regions;
};
