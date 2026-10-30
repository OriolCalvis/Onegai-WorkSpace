#pragma once

#include <algorithm>
#include <array>

#include "Core/Math/GridBounds.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Render/Camera.h"
#include "Runtime/RuntimeTileMap.h"

// C-19 — La UNICA funcion de culling del motor.
//
// Habia dos, copia literal la una de la otra: el
// visibleRangeFor() anonimo de IsometricRenderer.cpp y
// TileMap::visibleRange(). Dos copias del mismo calculo son dos sitios
// donde arreglar el mismo bug, y la clase de fallo que sale de ahi (una
// se corrige y la otra no) es silenciosa: el mapa se dibuja, solo que con
// una franja de mas o de menos.
//
// Header-only e inline por lo mismo que JsonWriter.h: no obliga a
// enlazar nada nuevo ni a mover TileMap/IsometricRenderer de libreria, y
// sigue siendo GL-free -- solo necesita Camera y RuntimeTileMap.
//
// Donde vive: en Render/ y no en Runtime/, porque el culling es del
// renderer. RuntimeTileMap NO depende de Camera (ver el comentario de
// TileMap::visibleRange) y esta funcion mantiene esa direccion: es ella
// la que conoce a los dos, no el mapa el que conoce a la camara.
//
// Que calcula: la caja de celdas que puede tocar el viewport, con UN
// margen de una celda por lado. El margen no es decorativo -- sin el, un
// tile alto (una torre, un arbol) cuya BASE cae justo fuera de la caja
// dejaria de dibujarse aunque su copa siguiera entrando en pantalla.
inline GridBounds VisibleGridRange(const RuntimeTileMap& map, const Camera& camera) {
    if (map.getWidth() <= 0 || map.getHeight() <= 0) {
        return GridBounds{};
    }

    // Las cuatro esquinas del viewport, llevadas a mundo y de ahi a
    // celda. En isometrico la proyeccion de un rectangulo de pantalla es
    // un rombo, asi que se toma la caja que lo contiene.
    const std::array<Vector2, 4> corners = {
        camera.screenToWorld(Vector2{0.0f, 0.0f}),
        camera.screenToWorld(Vector2{static_cast<float>(camera.viewportWidth()), 0.0f}),
        camera.screenToWorld(Vector2{0.0f, static_cast<float>(camera.viewportHeight())}),
        camera.screenToWorld(Vector2{static_cast<float>(camera.viewportWidth()),
                                     static_cast<float>(camera.viewportHeight())}),
    };

    int minX = map.screenToGrid(corners[0]).x;
    int maxX = minX;
    int minY = map.screenToGrid(corners[0]).y;
    int maxY = minY;
    for (int i = 1; i < 4; ++i) {
        const GridCoord grid = map.screenToGrid(corners[i]);
        minX = std::min(minX, grid.x);
        maxX = std::max(maxX, grid.x);
        minY = std::min(minY, grid.y);
        maxY = std::max(maxY, grid.y);
    }

    return GridBounds{std::max(0, minX - 1), std::min(map.getWidth() - 1, maxX + 1),
                      std::max(0, minY - 1), std::min(map.getHeight() - 1, maxY + 1)};
}
