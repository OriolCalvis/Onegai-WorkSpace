// Presentacion del paso del jugador; la posicion logica sigue en GameSession.
#pragma once
#include "Core/Math/Vector2.h"

struct PlayerMotionData {
    Vector2 baseOffset;
    Vector2 displacement;
    float elapsed = 0.0f;
    float duration = 0.16f;  // AV14-09: un poco mas que la cadencia de la tecla
    float tileWidth = 64.0f;
    float tileHeight = 32.0f;
    int startSortKey = 0;
    bool active = false;
    // Paso bloqueado: el sprite se gira hacia el muro y rebota sin cambiar de
    // casilla (ida y vuelta sobre displacement, nunca cambia gridPosition).
    bool bump = false;
    // ESTETICA-01: segundos parado desde el ultimo paso. Pasado un momento (no
    // entre dos pasos seguidos), el sprite pasa al reposo de su rumbo.
    float parado = 0.0f;
};
