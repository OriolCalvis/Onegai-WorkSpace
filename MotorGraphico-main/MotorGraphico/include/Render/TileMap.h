// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-19  (fase C, 2 j) - Un sol culling i un sol contracte d'actualització
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridBounds.h"
#include "Runtime/RuntimeTile.h"
#include "Runtime/RuntimeTileMap.h"

class Camera;

// Adaptador temporal de TMX (formato de Tiled, XML) a RuntimeTileMap. Varias capas
// (m_layers[layer][y][x]) para separar fondo/props/colision (Fase 2 del
// Gantt). La representación de runtime, su culling y sus overrides viven en
// RuntimeTileMap; este tipo conserva el importador TMX y los metadatos de
// tileset para que los demos existentes sigan siendo válidos durante DOC-03.
//
// Subconjunto de TMX soportado (lo suficiente para mapas dibujados a
// mano en Tiled sin funcionalidad avanzada; ver TileMap.cpp):
//  - <tileset> EMBEBIDO (sin "source" a un .tsx externo).
//  - <layer><data encoding="csv"> unicamente (ni XML plano ni
//    base64/zlib, que Tiled tambien puede generar).
//  - Colision por tile: <tile id="N"><properties><property
//    name="collision" type="bool" value="true"/></properties></tile>
//    dentro del <tileset>.
class TileMap : public RuntimeTileMap {
public:
    // Nunca lanza: cualquier fallo de parseo (XML invalido, encoding no
    // soportado, numero de celdas que no cuadra...) se captura y se
    // convierte en Result<bool>::Error con un mensaje descriptivo, igual
    // que ResourceManager<T>::load() con loadFromDisk().
    Result<bool> loadFromFile(const std::string& path);

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

    // El resto de lo que declara el <tileset>. Se expone porque el editor
    // tiene que poder VOLVER A ESCRIBIRLO: guardaba siempre los valores
    // por defecto (test_checker, 2 tiles) y eso destruyo la declaracion de
    // ciudad_centro.tmx, que usa hasta el GID 34. Un mapa que se abre y se
    // guarda no puede perder de que atlas era.
    const std::string& getTilesetName() const { return m_tilesetName; }
    // Tal cual venia en el TMX ("../textures/x.png"), sin resolver contra
    // la carpeta del mapa: es lo que hay que volver a escribir.
    const std::string& getTilesetImageSource() const { return m_tilesetImageSource; }
    int getTilesetImageWidth() const { return m_tilesetImageWidth; }
    int getTilesetImageHeight() const { return m_tilesetImageHeight; }
    int getTilesetTileCount() const { return m_tilesetTileCount; }
    // GIDs (1-based) con collision=true, en el orden del TMX.
    const std::vector<int>& getCollisionGids() const { return m_collisionGids; }

    // API de compatibilidad del importador TMX: el cálculo de culling es
    // parte del renderer y RuntimeTileMap no depende de Camera.
    GridBounds visibleRange(const Camera& camera) const;

private:
    void parseOrThrow(const std::string& path);

    // Tileset (ver los getters de arriba). Del PRIMER <tileset> del TMX:
    // el motor no soporta varios tilesets por mapa, y mezclar rangos de
    // GID de dos atlas distintos es justo el tipo de cosa que fallaría en
    // silencio -- mejor una sola fuente y explícita.
    std::string m_tilesetImage;
    int m_tilesetTileWidth = 0;
    int m_tilesetTileHeight = 0;
    int m_tilesetColumns = 0;
    int m_tilesetFirstGid = 1;
    std::string m_tilesetName;
    std::string m_tilesetImageSource;
    int m_tilesetImageWidth = 0;
    int m_tilesetImageHeight = 0;
    int m_tilesetTileCount = 0;
    std::vector<int> m_collisionGids;
};
