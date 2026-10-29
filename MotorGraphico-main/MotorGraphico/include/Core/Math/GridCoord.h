#pragma once

// Coordenada entera en el grid isometrico (columna/fila del TileMap),
// distinta de Vector2 (posicion continua en mundo/pantalla). Ver
// Camera::worldToGrid y TileMap en motor_grafico_clases.puml.
struct GridCoord {
    int x = 0;
    int y = 0;
};
