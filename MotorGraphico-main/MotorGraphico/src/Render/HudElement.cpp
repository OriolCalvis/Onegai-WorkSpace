#include "Render/HudElement.h"

namespace {

// Fraccion horizontal/vertical del anchor dentro del viewport: 0 = borde
// izquierdo/superior, 0.5 = centro, 1 = borde derecho/inferior. Reduce
// los 9 valores de HudAnchor a dos numeros, para no repetir la formula de
// resolveTopLeft() nueve veces.
struct AnchorFraction {
    float h;
    float v;
};

AnchorFraction anchorFraction(HudAnchor anchor) {
    switch (anchor) {
        case HudAnchor::TopLeft:
            return {0.0f, 0.0f};
        case HudAnchor::TopCenter:
            return {0.5f, 0.0f};
        case HudAnchor::TopRight:
            return {1.0f, 0.0f};
        case HudAnchor::CenterLeft:
            return {0.0f, 0.5f};
        case HudAnchor::Center:
            return {0.5f, 0.5f};
        case HudAnchor::CenterRight:
            return {1.0f, 0.5f};
        case HudAnchor::BottomLeft:
            return {0.0f, 1.0f};
        case HudAnchor::BottomCenter:
            return {0.5f, 1.0f};
        case HudAnchor::BottomRight:
            return {1.0f, 1.0f};
    }
    return {0.0f, 0.0f};  // inalcanzable, pero algunos compiladores lo exigen
}

}  // namespace

Vector2 HudTransform::resolveTopLeft(int viewportWidth, int viewportHeight) const {
    AnchorFraction frac = anchorFraction(anchor);

    // Punto de anclaje en pantalla (origen arriba-izquierda).
    float anchorX = frac.h * static_cast<float>(viewportWidth);
    float anchorY = frac.v * static_cast<float>(viewportHeight);

    // El propio rectangulo se alinea respecto al anchor con la MISMA
    // fraccion: borde izquierdo pegado al anchor si frac.h=0 (no hay que
    // retroceder nada), centrado si frac.h=0.5 (retrocede la mitad del
    // ancho), borde derecho pegado si frac.h=1 (retrocede el ancho
    // entero). Idem vertical con size.y.
    float left = anchorX - frac.h * size.x;
    float top = anchorY - frac.v * size.y;

    // El offset siempre empuja HACIA EL INTERIOR de la pantalla: en el
    // lado izquierdo/superior (frac 0) sumar offset ya empuja hacia
    // adentro; en el lado derecho/inferior (frac 1) hay que RESTAR para
    // que tambien empuje hacia adentro (sumar lo sacaria de pantalla). En
    // el eje centrado (frac 0.5) no hay una "direccion de fuera" clara:
    // se suma tal cual, como ajuste fino sin significado direccional.
    float signX = (frac.h >= 1.0f) ? -1.0f : 1.0f;
    float signY = (frac.v >= 1.0f) ? -1.0f : 1.0f;

    return Vector2{left + offset.x * signX, top + offset.y * signY};
}
