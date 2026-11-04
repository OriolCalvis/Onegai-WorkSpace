// C2-04d — Una emoción de cinemática lista para pintar: dónde (pies, en px de mundo), qué
// (icono del atlas o texto literal) y cuánto se ve (0..1).
#pragma once

#include <string>

#include "Core/Math/Vector2.h"

namespace Pergamino {

struct EmocionVista {
    Vector2 mundo;
    std::string icono;
    float alfa = 1.0f;
};

}  // namespace Pergamino
