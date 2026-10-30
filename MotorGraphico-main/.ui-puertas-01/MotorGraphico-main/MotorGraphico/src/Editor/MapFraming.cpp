#include "Editor/MapFraming.h"

#include <algorithm>

namespace Editor {

Result<MapFraming> fitMap(int columns, int rows, int tileWidth, int tileHeight,
                          int viewportWidth, int viewportHeight) {
    if (columns <= 0 || rows <= 0 || tileWidth <= 0 || tileHeight <= 0 ||
        viewportWidth <= 0 || viewportHeight <= 0)
        return Result<MapFraming>::Error("No se puede encuadrar un mapa o visor sin dimensiones positivas");

    // El renderer coloca un rectangulo tw x th en cada origen isometrico.
    // double antes de operar evita overflow entero en mapas muy grandes.
    const double halfWidth = tileWidth * 0.5;
    const double halfHeight = tileHeight * 0.5;
    const double left = -(rows - 1.0) * halfWidth;
    const double right = (columns - 1.0) * halfWidth + tileWidth;
    const double bottom = (columns + static_cast<double>(rows)) * halfHeight;
    const double zoom = std::min(viewportWidth * 0.90 / (right - left),
                                 viewportHeight * 0.90 / bottom);
    return Result<MapFraming>::Ok({
        {static_cast<float>((left + right) * 0.5), static_cast<float>(bottom * 0.5)},
        static_cast<float>(std::clamp(zoom, 0.15, 1.0))});
}

}  // namespace Editor
