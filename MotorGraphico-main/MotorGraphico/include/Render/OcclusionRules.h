#pragma once

#include "Core/Math/GridCoord.h"

// Regla pura para decidir si un tile solido tapa visualmente al personaje.
// En la proyeccion isometrica de este motor la profundidad en pantalla es
// x + y; por eso solo se atenúan los obstaculos que quedan por delante del
// foco, dentro de una pequena ventana diagonal. Asi una muralla cercana no
// esconde al jugador sin hacer translucido todo el primer plano del mapa.
namespace OcclusionRules {

inline bool shouldFadeTile(const GridCoord& focus, const GridCoord& tile, bool blocksMovement) {
    if (!blocksMovement) {
        return false;
    }

    const int depth = (tile.x + tile.y) - (focus.x + focus.y);
    const int lateral = (tile.x - tile.y) - (focus.x - focus.y);
    const int lateralDistance = lateral < 0 ? -lateral : lateral;

    // Dos filas por delante cubren paredes y props inmediatos. Es una zona
    // deliberadamente corta: los objetos lejanos no compiten con la lectura
    // del escenario ni producen un efecto de "mapa fantasma".
    return depth >= 1 && depth <= 2 && lateralDistance <= 2;
}

}  // namespace OcclusionRules
