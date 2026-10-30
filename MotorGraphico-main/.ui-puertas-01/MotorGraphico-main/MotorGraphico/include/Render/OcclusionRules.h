#pragma once

#include "Core/Math/GridCoord.h"

// Regla pura para decidir si un tile solido tapa visualmente al personaje.
// En la proyeccion isometrica de este motor la profundidad en pantalla es
// x + y; por eso solo se atenúan los obstaculos que quedan por delante del
// foco, dentro de una pequena ventana diagonal. Asi una muralla cercana no
// esconde al jugador sin hacer translucido todo el primer plano del mapa.
namespace OcclusionRules {

// AV16-13 — Solo se funde lo que de verdad tapa al jugador: lo que esta delante en LA VISTA. El jugador
// ocupa una baldosa de ancho en pantalla, y un objeto a dos casillas de lado (|lateral| = 2) queda a un
// lado entero, sin taparle: antes se fundia tambien (ventana de +-2) y parecia que "lo de al lado" se
// iba. Con +-1 quedan las casillas que SI se solapan con el sprite: delante, delante-izquierda y
// delante-derecha.
inline constexpr int kLateralMaximo = 1;

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
    return depth >= 1 && depth <= 2 && lateralDistance <= kLateralMaximo;
}

// AV15-03 — Los objetos de decorado que bloquean el paso (arboles, farolas, banderas, jardineras...) tapan al
// jugador igual que un muro, y mas: asoman por encima de su celda dos o tres filas, como los bloques de
// BloquesNivel (una fila mas que un tile). Los actores (PNJ, enemigos) no se funden: se ven con el jugador.
inline constexpr float kAlfaProp = 0.40f;

// Solo la geometria: ¿la celda del objeto cae en la ventana por delante del foco?
inline bool propInFadeWindow(const GridCoord& focus, const GridCoord& prop) {
    const int depth = (prop.x + prop.y) - (focus.x + focus.y);
    const int lateral = (prop.x - prop.y) - (focus.x - focus.y);
    const int lateralDistance = lateral < 0 ? -lateral : lateral;
    return depth >= 1 && depth <= 3 && lateralDistance <= kLateralMaximo;
}

inline bool shouldFadeProp(const GridCoord& focus, const GridCoord& prop, bool blocksMovement, bool isActor) {
    return blocksMovement && !isActor && propInFadeWindow(focus, prop);
}

}  // namespace OcclusionRules
