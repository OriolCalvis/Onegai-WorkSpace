// C2-04b — La geometría de un sprite caído o encorvado, sin GL: el cuadro de
// `esquina`/`tam` aplastado en altura (`aplasta`, 1 = entero) con los pies
// fijos y girado `giro` radianes alrededor de los pies (centro de su borde de
// abajo). Esquinas en el orden de SpriteBatch::submitQuad: arriba-izquierda,
// arriba-derecha, abajo-derecha, abajo-izquierda. En pantalla y crece hacia
// abajo: giro positivo = en sentido de las agujas del reloj.
#pragma once

#include <array>
#include <cmath>

#include "Core/Math/Vector2.h"

namespace Game {

inline std::array<Vector2, 4> cuadroGirado(const Vector2& esquina, const Vector2& tam, float giro, float aplasta) {
    const Vector2 pie{esquina.x + tam.x * 0.5f, esquina.y + tam.y};
    const float alto = tam.y * aplasta;
    const Vector2 local[4] = {{-tam.x * 0.5f, -alto}, {tam.x * 0.5f, -alto}, {tam.x * 0.5f, 0.0f}, {-tam.x * 0.5f, 0.0f}};
    const float c = std::cos(giro), s = std::sin(giro);
    std::array<Vector2, 4> fuera;
    for (int i = 0; i < 4; ++i)
        fuera[i] = Vector2{pie.x + local[i].x * c - local[i].y * s, pie.y + local[i].x * s + local[i].y * c};
    return fuera;
}

}  // namespace Game
