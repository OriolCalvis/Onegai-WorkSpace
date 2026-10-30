// EST-20 (relevo A) — LO QUE COMPARTEN LAS CUATRO UNIDADES DE APPLICATION.
//
// Al partir Application.cpp (2493 lineas, 14 paquetes) en cuatro ficheros
// por flujo, un punado de ayudantes quedo con dos duenos. Estan aqui, y
// solo estos: la regla es que un ayudante sube a esta cabecera cuando lo
// usan DOS unidades, no antes. Lo que usa una sola se queda en su fichero,
// dentro de su anonymous namespace, donde no estorba a nadie.
//
// Es cabecera INTERNA del motor: ningun consumidor de Engine/Application.h
// la incluye, y no forma parte de la interfaz publica.

#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"

#include <cstdio>
#include <cstdlib>

namespace Engine {
namespace Interno {

// Colores compartidos de todos los paneles del HUD. Antes eran globales
// sueltos en Application.cpp, visibles a todo el translation unit porque
// los usaban tanto las funciones del anonymous namespace como los metodos
// de Application. Ahora los comparten dos unidades, que es exactamente el
// caso para el que existe este fichero.
    inline constexpr int kTileW = 64;
    inline constexpr int kTileH = 32;
    inline const Vector4 kPanelColor{0.05f, 0.05f, 0.08f, 0.82f};
    inline const Vector4 kBorderColor{0.55f, 0.55f, 0.65f, 0.9f};
    inline constexpr float kMenuPadding = 10.0f;

// Traza de arranque/bucle a stderr (SIN buffer, a diferencia de stdout:
// sobrevive a un SIGSEGV, que es justo cuando hace falta). Silenciosa
// salvo que se pida con la variable de entorno MOTOR_TRACE=1, asi que no
// ensucia la salida normal ni el modo humo de CI:
//
//   MOTOR_TRACE=1 ./juego 3
//
// Se queda en el codigo a proposito: un motor que solo se puede depurar
// recompilando con prints a mano es un motor que nadie depura.
//
// Van INLINE aqui, sin unidad propia: dos funciones de tres lineas no
// pagan un .cpp, y uno sin GL dentro de motor_core habria empeorado la
// mezcla de orillas que ya arrastra la libreria (EST-40/41).
inline bool traceEnabled() {
    static const bool enabled = std::getenv("MOTOR_TRACE") != nullptr;
    return enabled;
}

inline void trace(const char* phase) {
    if (traceEnabled()) {
        std::fprintf(stderr, "[TRACE] %s\n", phase);
    }
}


// --- EST-20 (relevo B) ---
// El sprite de personaje es cuadrado (64x64) pero el tile isometrico es un
// rombo 2:1 (64x32). Estas tres cosas deciden como se apoya el actor sobre
// su celda, y desde que loadLevel vive aparte las comparten dos unidades:
// init (que monta el jugador) y loadLevel (que reparte sprites a enemigos,
// PNJ y objetos).
inline constexpr int kCharacterDrawW = kTileW;             // 64: mismo ancho que el tile
inline constexpr int kCharacterDrawH = kTileW;             // 64: cuadrado (el sprite lo es)

// Offset de anclaje del sprite de personaje respecto a la esquina superior
// del rombo del tile (lo que devuelve IsoMath::gridToScreen). Centrado en X
// y con la base tocando la base del rombo en Y.
//
// gridToScreen devuelve la esquina SUPERIOR del rombo. El centro del rombo
// esta a (tileW/2, tileH/2); su base, a tileH. Queremos la base del sprite
// (pos.y + drawH) sobre la base del rombo (tileH):
//   anchorY = tileH - drawH = 32 - 64 = -32  (el sprite asoma por arriba)
//   anchorX = tileW/2 - drawW/2 = 0          (centrado, mismo ancho)
inline Vector2 characterAnchor() {
    return Vector2{static_cast<float>(kTileW - kCharacterDrawW) * 0.5f,
                   static_cast<float>(kTileH - kCharacterDrawH)};
}

}  // namespace Interno
}  // namespace Engine
