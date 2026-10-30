#pragma once

#include "Core/Errors/Result.h"
#include "Core/Math/Vector2.h"

namespace Editor {

// Encuadre calculado, sin estado de camara ni dependencia de la ventana.
struct MapFraming {
    Vector2 center;
    float zoom = 1.0f;
};

// Incluye los rectangulos completos de las baldosas y un margen del 5%
// por lado. No amplia mapas pequenos; respeta el minimo 0.15 de la rueda
// del editor. Por debajo de ese minimo, centra aunque no quepa todo.
Result<MapFraming> fitMap(int columns, int rows, int tileWidth, int tileHeight,
                          int viewportWidth, int viewportHeight);

}  // namespace Editor
