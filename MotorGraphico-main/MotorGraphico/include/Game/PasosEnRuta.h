// AV13-12 — Un dibujo que anda una ruta de casillas, paso a paso.
//
// En el combate por casillas la logica mueve de golpe (el tablero dice donde
// acaba cada uno); el dibujo tiene que andar la ruta, casilla a casilla, como
// el jugador al explorar (0,16 s por paso, AV14-09). Esto dice, dado el tiempo,
// por donde va el dibujo y hacia donde mira. Sin GL y sin reloj propio (solo
// cabecera: la usa ApplicationCinematica).
#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Core/Math/IsoMath.h"
#include "Core/Math/Vector2.h"

namespace Game {

struct PasosEnRuta {
    static constexpr float kPaso = 0.16f;  // lo que dura un paso del dibujo
    GridCoord desde{};
    std::vector<GridCoord> ruta;  // sin `desde`; la ultima es donde acaba
    float t = 0.0f;
    Vector2 base{};          // el desplazamiento del dibujo antes de andar
    std::string antes;       // su animacion antes de andar, para volver a ella

    bool activa() const { return !ruta.empty() && t < static_cast<float>(ruta.size()) * kPaso; }
    // El paso en curso: de `a` a `b`, con la fraccion andada.
    void paso(GridCoord& a, GridCoord& b, float& f) const {
        if (ruta.empty()) {
            a = b = desde;
            f = 1.0f;
            return;
        }
        const int n = static_cast<int>(ruta.size());
        const int k = std::min(n - 1, static_cast<int>(t / kPaso));
        a = k == 0 ? desde : ruta[static_cast<std::size_t>(k - 1)];
        b = ruta[static_cast<std::size_t>(k)];
        f = std::clamp((t - static_cast<float>(k) * kPaso) / kPaso, 0.0f, 1.0f);
    }
    // Donde va el dibujo, en el mundo (la misma proyeccion que las casillas).
    Vector2 dibujo(float anchoBaldosa, float altoBaldosa) const {
        GridCoord a, b;
        float f = 0.0f;
        paso(a, b, f);
        const Vector2 pa = IsoMath::gridToScreen(a, anchoBaldosa, altoBaldosa);
        const Vector2 pb = IsoMath::gridToScreen(b, anchoBaldosa, altoBaldosa);
        return Vector2{pa.x + (pb.x - pa.x) * f, pa.y + (pb.y - pa.y) * f};
    }
    // Hacia donde anda (el nombre de la animacion de andar).
    std::string andar() const {
        GridCoord a, b;
        float f = 0.0f;
        paso(a, b, f);
        const int dx = b.x - a.x, dy = b.y - a.y;
        return dy < 0 ? "walk_up" : dy > 0 ? "walk_down" : dx < 0 ? "walk_left" : "walk_right";
    }
};

}  // namespace Game
