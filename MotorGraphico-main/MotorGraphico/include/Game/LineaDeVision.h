// E-DISPARADORES-1 (D0026.8) — La linea de vision entre dos casillas, compartida por el tablero de combate
// (TableroCombate::seVen) y los disparadores de cinematica (Game/Disparador.h). GL-free y sin tablero.
#pragma once

#include <algorithm>
#include <cstdlib>
#include <functional>

#include "Core/Math/GridCoord.h"

namespace Game {

// ¿Tapa la casilla (x, y) la vista? Vacia: nada tapa.
using TapaLaVista = std::function<bool(int, int)>;

// Distancia de Chebyshev (la diagonal cuenta 1): la del alcance, la niebla y el tablero.
inline int distanciaChebyshev(const GridCoord& a, const GridCoord& b) {
    return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
}

// Bresenham de centro a centro; los extremos no cuentan (quien mira y a quien se mira pueden estar en una casilla
// «bloqueada» del nivel, como un banco). `tapa` se pregunta por cada casilla de en medio.
inline bool lineaDeVision(GridCoord a, GridCoord b, const TapaLaVista& tapa) {
    int x = a.x, y = a.y;
    const int dx = std::abs(b.x - a.x), dy = -std::abs(b.y - a.y);
    const int sx = a.x < b.x ? 1 : -1, sy = a.y < b.y ? 1 : -1;
    int err = dx + dy;
    while (!(x == b.x && y == b.y)) {
        const int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
        if (x == b.x && y == b.y) break;
        if (tapa && tapa(x, y)) return false;
    }
    return true;
}

}  // namespace Game
