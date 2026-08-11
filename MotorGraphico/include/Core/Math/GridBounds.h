#pragma once

// Rango de celdas [minX..maxX] x [minY..maxY] (ambos extremos inclusive).
// Usado para el culling de TileMap: en vez de recorrer todo el mapa cada
// frame, IsometricRenderer::renderLayer() solo itera este rango (ver
// TileMap::visibleRange() y Fase 2 del Gantt, "Culling y batching
// estatico").
struct GridBounds {
    int minX = 0;
    int maxX = -1;  // maxX < minX == rango vacio (sin celdas visibles)
    int minY = 0;
    int maxY = -1;

    bool isEmpty() const { return maxX < minX || maxY < minY; }
};
