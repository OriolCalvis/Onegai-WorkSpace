#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridBounds.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Render/Tile.h"

class Camera;

// Mapa de tiles cargado desde TMX (formato de Tiled, XML). Varias capas
// (m_layers[layer][y][x]) para separar fondo/props/colision (Fase 2 del
// Gantt). screenToGrid/gridToScreen reutilizan IsoMath (mismas formulas
// que Camera::worldToGrid).
//
// Subconjunto de TMX soportado (lo suficiente para mapas dibujados a
// mano en Tiled sin funcionalidad avanzada; ver TileMap.cpp):
//  - <tileset> EMBEBIDO (sin "source" a un .tsx externo).
//  - <layer><data encoding="csv"> unicamente (ni XML plano ni
//    base64/zlib, que Tiled tambien puede generar).
//  - Colision por tile: <tile id="N"><properties><property
//    name="collision" type="bool" value="true"/></properties></tile>
//    dentro del <tileset>.
class TileMap {
public:
    // Nunca lanza: cualquier fallo de parseo (XML invalido, encoding no
    // soportado, numero de celdas que no cuadra...) se captura y se
    // convierte en Result<bool>::Error con un mensaje descriptivo, igual
    // que ResourceManager<T>::load() con loadFromDisk().
    Result<bool> loadFromFile(const std::string& path);

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getTileWidth() const { return m_tileWidth; }
    int getTileHeight() const { return m_tileHeight; }
    int getLayerCount() const { return static_cast<int>(m_layers.size()); }

    // --- Tileset declarado por el propio TMX (Tiled ya lo trae; antes se
    // ignoraba y cada app cargaba una textura fija con sus regiones
    // escritas a mano, lo que ataba TODOS los mapas al mismo puñado de
    // tiles). Con esto un mapa es autocontenido: dice con qué imagen se
    // dibuja y cómo está dividida, así que dos niveles pueden usar
    // tilesets distintos sin tocar código. ---

    // Ruta de <image source>, YA RESUELTA respecto al directorio del TMX
    // (en el archivo es relativa a él, ej. "../textures/x.png"): se puede
    // pasar tal cual a TextureManager::load. Vacía si el TMX no la trae.
    const std::string& tilesetImagePath() const { return m_tilesetImage; }

    // Tamaño de celda DEL TILESET (el <tileset tilewidth/tileheight>),
    // que no tiene por qué coincidir con el del mapa: el mapa dibuja
    // rombos de 64x32 con celdas de atlas de 16x16, por ejemplo.
    int getTilesetTileWidth() const { return m_tilesetTileWidth; }
    int getTilesetTileHeight() const { return m_tilesetTileHeight; }

    // Columnas del tileset y primer GID. Con ambos, el GID de una celda
    // se resuelve a (columna, fila) del atlas con la convención estándar
    // de Tiled: índice = gid - firstGid, recorriendo en orden de lectura.
    int getTilesetColumns() const { return m_tilesetColumns; }
    int getTilesetFirstGid() const { return m_tilesetFirstGid; }

    // Lanza std::out_of_range si layer/x/y estan fuera de rango: un
    // indice invalido aqui es un bug del llamador (ver
    // IsometricRenderer, Fase 3), no un fallo "esperable" de I/O como
    // loadFromFile().
    const Tile& getTile(int layer, int x, int y) const;
    Tile& getTile(int layer, int x, int y);

    // Wrappers finos sobre IsoMath (unica fuente de verdad de la proyeccion
    // isometrica, ver Core/Math/IsoMath.h): anaden el tileWidth/tileHeight
    // del mapa y delegan. NO reimplementan la formula -- fractura #4 del
    // analisis de coherencia (RESUELTA por delegacion, ver ARCHITECTURE.md).
    GridCoord screenToGrid(const Vector2& screenPos) const;
    Vector2 gridToScreen(const GridCoord& grid) const;

    // Rango de celdas [minX..maxX]x[minY..maxY] que intersecta el
    // viewport actual de "camera" (mas 1 celda de margen, para no dejar
    // huecos en los bordes por redondeo), acotado a los limites del
    // mapa. IsometricRenderer::renderLayer() itera solo este rango en
    // vez del mapa entero (Fase 2, "Culling y batching estatico"): para
    // un mapa grande, evita recorrer/dibujar miles de celdas fuera de
    // pantalla en cada frame.
    //
    // La transformacion mundo->grid es una cizalla (isometrica), asi que
    // el rectangulo del viewport en espacio mundo NO se corresponde con
    // un rectangulo en espacio grid: se convierten las 4 esquinas y se
    // toma el rectangulo delimitador de las 4 (por eso el resultado
    // puede incluir alguna celda realmente fuera de pantalla, nunca al
    // reves -- es una sobre-aproximacion segura, no un recorte exacto).
    GridBounds visibleRange(const Camera& camera) const;

private:
    void parseOrThrow(const std::string& path);

    std::vector<std::vector<std::vector<Tile>>> m_layers;  // [layer][y][x]
    int m_width = 0;
    int m_height = 0;
    int m_tileWidth = 0;
    int m_tileHeight = 0;

    // Tileset (ver los getters de arriba). Del PRIMER <tileset> del TMX:
    // el motor no soporta varios tilesets por mapa, y mezclar rangos de
    // GID de dos atlas distintos es justo el tipo de cosa que fallaría en
    // silencio -- mejor una sola fuente y explícita.
    std::string m_tilesetImage;
    int m_tilesetTileWidth = 0;
    int m_tilesetTileHeight = 0;
    int m_tilesetColumns = 0;
    int m_tilesetFirstGid = 1;
};
