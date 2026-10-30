#include "Core/Math/PoliticaCamara.h"

#include <cmath>

namespace PoliticaCamara {

namespace {

// Tolerancia de frontera en pixeles de mundo: evita vibracion cuando el actor esta en el borde.
constexpr double kToleranciaBordePx = 1e-4;

// Un eje: devuelve el centro nuevo (double). semiMundo = semi/zoom.
double seguirEje(double centro, double actor, double semiMundo) {
    const double d = actor - centro;
    if (std::fabs(d) <= semiMundo + kToleranciaBordePx) return centro;
    return d > 0.0 ? actor - semiMundo : actor + semiMundo;
}

}  // namespace

Result<Vector2> seguirConZonaMuerta(const Vector2& centroActual, float zoom,
                                    const Vector2& actorMundo, const ZonaMuerta& zona) {
    if (!std::isfinite(zoom) || zoom <= 0.0f) {
        return Result<Vector2>::Error("PoliticaCamara::seguirConZonaMuerta: zoom no finito o <= 0");
    }
    if (!std::isfinite(zona.semiAnchoPx) || !std::isfinite(zona.semiAltoPx) ||
        zona.semiAnchoPx < 0.0f || zona.semiAltoPx < 0.0f) {
        return Result<Vector2>::Error("PoliticaCamara::seguirConZonaMuerta: zona negativa o no finita");
    }
    if (!std::isfinite(centroActual.x) || !std::isfinite(centroActual.y) ||
        !std::isfinite(actorMundo.x) || !std::isfinite(actorMundo.y)) {
        return Result<Vector2>::Error("PoliticaCamara::seguirConZonaMuerta: centro o actor no finito");
    }
    const double z = static_cast<double>(zoom);
    const double x = seguirEje(centroActual.x, actorMundo.x, static_cast<double>(zona.semiAnchoPx) / z);
    const double y = seguirEje(centroActual.y, actorMundo.y, static_cast<double>(zona.semiAltoPx) / z);
    return Result<Vector2>::Ok(Vector2{static_cast<float>(x), static_cast<float>(y)});
}

}  // namespace PoliticaCamara
