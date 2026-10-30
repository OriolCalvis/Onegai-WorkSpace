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

namespace {

// Un eje de limitarAlMapa. mitad = vista/(2*zoom) en mundo.
double limitarEje(double centro, double minimo, double maximo, double mitad) {
    if (maximo - minimo <= 2.0 * mitad) return (minimo + maximo) * 0.5;  // el mapa cabe: centrar
    if (centro < minimo + mitad) return minimo + mitad;
    if (centro > maximo - mitad) return maximo - mitad;
    return centro;
}

}  // namespace

Result<LimitesMapa> limitesDeMapa(int columnas, int filas, int anchoTile, int altoTile,
                                  float margenSuperiorPx) {
    if (columnas <= 0 || filas <= 0 || anchoTile <= 0 || altoTile <= 0) {
        return Result<LimitesMapa>::Error("PoliticaCamara::limitesDeMapa: dimensiones <= 0");
    }
    if (!std::isfinite(margenSuperiorPx) || margenSuperiorPx < 0.0f) {
        return Result<LimitesMapa>::Error("PoliticaCamara::limitesDeMapa: margen negativo o no finito");
    }
    const double mitadAncho = anchoTile * 0.5;
    const double mitadAlto = altoTile * 0.5;
    LimitesMapa l;
    l.izquierda = static_cast<float>(-(filas - 1.0) * mitadAncho);
    l.derecha = static_cast<float>((columnas - 1.0) * mitadAncho + anchoTile);
    l.arriba = static_cast<float>(-static_cast<double>(margenSuperiorPx));
    l.abajo = static_cast<float>((columnas + static_cast<double>(filas)) * mitadAlto);
    return Result<LimitesMapa>::Ok(l);
}

Result<Vector2> limitarAlMapa(const Vector2& centro, float zoom, const TamanoVista& vista,
                              const LimitesMapa& limites) {
    if (!std::isfinite(zoom) || zoom <= 0.0f) {
        return Result<Vector2>::Error("PoliticaCamara::limitarAlMapa: zoom no finito o <= 0");
    }
    if (vista.ancho <= 0 || vista.alto <= 0) {
        return Result<Vector2>::Error("PoliticaCamara::limitarAlMapa: vista sin dimensiones positivas");
    }
    if (!std::isfinite(centro.x) || !std::isfinite(centro.y)) {
        return Result<Vector2>::Error("PoliticaCamara::limitarAlMapa: centro no finito");
    }
    if (!std::isfinite(limites.izquierda) || !std::isfinite(limites.derecha) ||
        !std::isfinite(limites.arriba) || !std::isfinite(limites.abajo) ||
        limites.derecha < limites.izquierda || limites.abajo < limites.arriba) {
        return Result<Vector2>::Error("PoliticaCamara::limitarAlMapa: limites no finitos o invertidos");
    }
    const double z = static_cast<double>(zoom);
    const double x = limitarEje(centro.x, limites.izquierda, limites.derecha, vista.ancho / (2.0 * z));
    const double y = limitarEje(centro.y, limites.arriba, limites.abajo, vista.alto / (2.0 * z));
    // Eje ya valido: se devuelve el float original, sin redondeo.
    const float rx = (x == static_cast<double>(centro.x)) ? centro.x : static_cast<float>(x);
    const float ry = (y == static_cast<double>(centro.y)) ? centro.y : static_cast<float>(y);
    return Result<Vector2>::Ok(Vector2{rx, ry});
}

}  // namespace PoliticaCamara
