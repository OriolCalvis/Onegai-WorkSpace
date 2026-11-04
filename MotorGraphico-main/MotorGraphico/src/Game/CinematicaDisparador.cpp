// E-DISPARADORES-1 (D0026) — Lo de las cinematicas que empiezan por una casilla: el punto de entrada y,
// mas adelante, el disparador (lleva, accion, radio y vista). Fichero propio: Cinematica.cpp ya pasa del techo.
#include <algorithm>
#include <cmath>

#include "Game/Cinematica.h"

namespace Game {

GridCoord Cinematica::puntoDeEntrada(const std::function<bool(const GridCoord&)>& transitable) const {
    if (!hayZona) return GridCoord{-1, -1};
    const GridCoord centro{(zonaDesde.x + zonaHasta.x) / 2, (zonaDesde.y + zonaHasta.y) / 2};
    if (!transitable || transitable(centro)) return centro;
    const int radioMax = std::max(zonaHasta.x - zonaDesde.x, zonaHasta.y - zonaDesde.y);
    for (int r = 1; r <= radioMax; ++r)
        for (int y = centro.y - r; y <= centro.y + r; ++y)
            for (int x = centro.x - r; x <= centro.x + r; ++x) {
                if (std::max(std::abs(x - centro.x), std::abs(y - centro.y)) != r) continue;  // solo el anillo
                if (x < zonaDesde.x || x > zonaHasta.x || y < zonaDesde.y || y > zonaHasta.y) continue;
                if (transitable(GridCoord{x, y})) return GridCoord{x, y};
            }
    return centro;  // ninguna transitable: el centro (el validador lo avisa)
}

}  // namespace Game
