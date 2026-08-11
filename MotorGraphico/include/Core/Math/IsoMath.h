#pragma once

#include <cmath>

#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"

// Proyeccion isometrica 2:1 (la habitual en pixel art: tileWidth = 2 *
// tileHeight, p.ej. 64x32). El tile (0,0) queda en el origen; +x crece
// hacia abajo-derecha, +y hacia abajo-izquierda en espacio mundo/pantalla.
//
// gridToScreen/screenToGrid son funciones puras e inversas entre si (sin
// floor, screenToGrid(gridToScreen(g)) == g exacto). screenToGrid aplica
// floor() al final porque un punto continuo cae dentro del rombo de una
// celda entera; ese floor es lo que convierte "punto bajo el cursor" en
// "que tile es".
//
// Reutilizadas por Camera::worldToGrid ahora, y por TileMap::gridToScreen
// / screenToGrid en cuanto exista (Fase 2, ver motor_grafico_clases.puml)
// para no duplicar la formula en dos sitios.
namespace IsoMath {

inline Vector2 gridToScreen(const GridCoord& grid, float tileWidth, float tileHeight) {
    return Vector2{
        static_cast<float>(grid.x - grid.y) * (tileWidth * 0.5f),
        static_cast<float>(grid.x + grid.y) * (tileHeight * 0.5f),
    };
}

inline GridCoord screenToGrid(const Vector2& screenPos, float tileWidth, float tileHeight) {
    const float halfW = tileWidth * 0.5f;
    const float halfH = tileHeight * 0.5f;
    const float gx = (screenPos.x / halfW + screenPos.y / halfH) * 0.5f;
    const float gy = (screenPos.y / halfH - screenPos.x / halfW) * 0.5f;
    return GridCoord{
        static_cast<int>(std::floor(gx)),
        static_cast<int>(std::floor(gy)),
    };
}

}  // namespace IsoMath
