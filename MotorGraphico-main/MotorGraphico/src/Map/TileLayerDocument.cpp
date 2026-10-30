// La capa de baldosas: la rejilla de gids y sus limites.
//
// Sabe su tamano y que una celda fuera de rango es un error con nombre,
// no un acceso invalido. Es la unica que toca memoria contigua grande,
// asi que es la unica que tiene que preocuparse por el indice.
//
// Salio de src/Map/MapDocument.cpp el 23/09/2026 (EST-27): aquel
// fichero tenia 724 lineas y cuatro responsabilidades dentro. Se
// movio, no se reescribio. Los paquetes que mandan siguen siendo
// C-12 y C-13; ver planificacion/PLAN_ESTABILIZACION.md.

#include "Map/MapDocument.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <type_traits>
#include <utility>

TileLayerDocument::TileLayerDocument(int width, int height, std::vector<int> gids)
    : m_width(width), m_height(height), m_gids(std::move(gids)) {}

Result<bool> TileLayerDocument::validate() const {
    if (m_width <= 0 || m_height <= 0) {
        return Result<bool>::Error("una capa de tiles debe tener dimensiones positivas");
    }

    const std::size_t expectedCells = static_cast<std::size_t>(m_width) *
                                      static_cast<std::size_t>(m_height);
    if (m_gids.size() != expectedCells) {
        return Result<bool>::Error("el número de GIDs no coincide con las dimensiones de la capa");
    }
    if (std::any_of(m_gids.begin(), m_gids.end(), [](int gid) { return gid < 0; })) {
        return Result<bool>::Error("un GID de tile no puede ser negativo");
    }
    return Result<bool>::Ok(true);
}

// --- C-12: mutacion de una capa de tiles ---

int TileLayerDocument::gidAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return 0;  // fuera de la cuadricula se lee como celda vacia, no revienta
    }
    return m_gids[static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) +
                  static_cast<std::size_t>(x)];
}

Result<bool> TileLayerDocument::setGid(int x, int y, int gid) {
    if (m_width <= 0 || m_height <= 0) {
        return Result<bool>::Error("no se puede pintar en una capa sin dimensiones");
    }
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return Result<bool>::Error("la celda (" + std::to_string(x) + "," + std::to_string(y) +
                                   ") esta fuera de la capa");
    }
    if (gid < 0) {
        return Result<bool>::Error("un GID de tile no puede ser negativo");
    }
    m_gids[static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) +
           static_cast<std::size_t>(x)] = gid;
    return Result<bool>::Ok(true);
}

Result<bool> TileLayerDocument::fill(int minX, int minY, int maxX, int maxY, int gid) {
    if (maxX < minX || maxY < minY) {
        return Result<bool>::Ok(true);  // area vacia: no pinta nada, y no es un error
    }
    if (gid < 0) {
        return Result<bool>::Error("un GID de tile no puede ser negativo");
    }
    // Se comprueban las esquinas ANTES de pintar: un rectangulo que se
    // sale a medias dejaria media capa pintada y media no.
    if (minX < 0 || minY < 0 || maxX >= m_width || maxY >= m_height) {
        return Result<bool>::Error("el area a rellenar se sale de la capa");
    }
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            m_gids[static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width) +
                   static_cast<std::size_t>(x)] = gid;
        }
    }
    return Result<bool>::Ok(true);
}

EntityLayerDocument::EntityLayerDocument(std::vector<MapEntityDocument> entities)
    : m_entities(std::move(entities)) {}
