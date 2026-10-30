#include "Core/Math/PoliticaCamara.h"

#include <cmath>
#include <limits>

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

// Tolerancia (px de mundo) para considerar un punto DENTRO del rombo: un punto proyectado y
// redondeado a float queda ~1e-4 fuera; con esto es estable (idempotencia).
constexpr double kToleranciaRomboPx = 1e-3;

struct P2 {
    double x;
    double y;
};

// Un eje de la caja. mitad = vista/(2*zoom) en mundo.
double limitarEje(double centro, double minimo, double maximo, double mitad) {
    if (maximo - minimo <= 2.0 * mitad) return (minimo + maximo) * 0.5;  // el mapa cabe: centrar
    if (centro < minimo + mitad) return minimo + mitad;
    if (centro > maximo - mitad) return maximo - mitad;
    return centro;
}

P2 aP2(const Vector2& v) { return {static_cast<double>(v.x), static_cast<double>(v.y)}; }

// Distancia con signo (positiva = dentro) a la arista a->b del contorno, orden superior, derecho,
// inferior, izquierdo (horario en pantalla, y hacia abajo).
double distanciaDentro(const P2& a, const P2& b, const P2& q) {
    const double ex = b.x - a.x, ey = b.y - a.y;
    return (ex * (q.y - a.y) - ey * (q.x - a.x)) / std::hypot(ex, ey);
}

P2 masCercanoEnSegmento(const P2& a, const P2& b, const P2& q) {
    const double ex = b.x - a.x, ey = b.y - a.y;
    double t = ((q.x - a.x) * ex + (q.y - a.y) * ey) / (ex * ex + ey * ey);
    t = std::fmin(1.0, std::fmax(0.0, t));
    return {a.x + t * ex, a.y + t * ey};
}

// Punto del contorno mas cercano a q (q mismo si ya esta dentro, con tolerancia).
P2 proyectarARombo(const RomboMapa& r, const P2& q) {
    const P2 v[4] = {aP2(r.superior), aP2(r.derecho), aP2(r.inferior), aP2(r.izquierdo)};
    bool dentro = true;
    for (int i = 0; i < 4; ++i) {
        if (distanciaDentro(v[i], v[(i + 1) % 4], q) < -kToleranciaRomboPx) dentro = false;
    }
    if (dentro) return q;
    P2 mejor = v[0];
    double mejorD = std::numeric_limits<double>::infinity();
    for (int i = 0; i < 4; ++i) {
        const P2 p = masCercanoEnSegmento(v[i], v[(i + 1) % 4], q);
        const double d = std::hypot(p.x - q.x, p.y - q.y);
        if (d < mejorD) {
            mejorD = d;
            mejor = p;
        }
    }
    return mejor;
}

bool rangoFinito(const LimitesMapa& l) {
    if (!std::isfinite(l.izquierda) || !std::isfinite(l.derecha) || !std::isfinite(l.arriba) ||
        !std::isfinite(l.abajo)) return false;
    if (!l.usaRombo) return true;
    const Vector2* v[4] = {&l.rombo.superior, &l.rombo.derecho, &l.rombo.inferior, &l.rombo.izquierdo};
    for (const Vector2* p : v) {
        if (!std::isfinite(p->x) || !std::isfinite(p->y)) return false;
    }
    return true;
}

// Fraccion maxima t en [0,1] de c -> r que no sale del rectangulo [lo, hi] (c dentro).
double fraccionEnRango(double c, double r, double lo, double hi) {
    if (r > hi) return (hi - c) / (r - c);
    if (r < lo) return (lo - c) / (r - c);
    return 1.0;
}

}  // namespace

Result<RomboMapa> romboDeMapa(int columnas, int filas, int anchoTile, int altoTile) {
    if (columnas <= 0 || filas <= 0 || anchoTile <= 0 || altoTile <= 0) {
        return Result<RomboMapa>::Error("PoliticaCamara::romboDeMapa: dimensiones <= 0");
    }
    const double hw = anchoTile * 0.5, hh = altoTile * 0.5;
    const double c = columnas, f = filas;
    // Origen de la casilla (gx,gy) = gridToScreen: ((gx-gy)*hw, (gx+gy)*hh).
    RomboMapa r;
    r.superior = {static_cast<float>(hw), 0.0f};
    r.derecho = {static_cast<float>((c - 1.0) * hw + anchoTile), static_cast<float>((c - 1.0) * hh + hh)};
    r.inferior = {static_cast<float>((c - f) * hw + hw), static_cast<float>((c + f - 2.0) * hh + altoTile)};
    r.izquierdo = {static_cast<float>(-(f - 1.0) * hw), static_cast<float>((f - 1.0) * hh + hh)};
    return Result<RomboMapa>::Ok(r);
}

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
    l.rombo = romboDeMapa(columnas, filas, anchoTile, altoTile).value();
    l.usaRombo = true;
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
    if (!rangoFinito(limites) || limites.derecha < limites.izquierda || limites.abajo < limites.arriba) {
        return Result<Vector2>::Error("PoliticaCamara::limitarAlMapa: limites no finitos o invertidos");
    }
    const double z = static_cast<double>(zoom);
    const double mx = vista.ancho / (2.0 * z), my = vista.alto / (2.0 * z);
    P2 q{limitarEje(centro.x, limites.izquierda, limites.derecha, mx),
         limitarEje(centro.y, limites.arriba, limites.abajo, my)};
    if (limites.usaRombo) {
        P2 r = proyectarARombo(limites.rombo, q);
        const double xmin = limites.izquierda + mx, xmax = limites.derecha - mx;
        const double ymin = limites.arriba + my, ymax = limites.abajo - my;
        const double cx = (limites.izquierda + limites.derecha) * 0.5;
        const double cy = (limites.arriba + limites.abajo) * 0.5;
        // Rango de la caja para el centro (colapsado al centro de la caja si el mapa cabe).
        const double lox = xmin <= xmax ? xmin : cx, hix = xmin <= xmax ? xmax : cx;
        const double loy = ymin <= ymax ? ymin : cy, hiy = ymin <= ymax ? ymax : cy;
        if (r.x != q.x || r.y != q.y) {
            const double t = std::fmin(fraccionEnRango(cx, r.x, lox, hix),
                                       fraccionEnRango(cy, r.y, loy, hiy));
            if (t < 1.0) r = {cx + t * (r.x - cx), cy + t * (r.y - cy)};
            q = r;
        }
    }
    // Eje ya valido: se devuelve el float original, sin redondeo.
    const float rx = (q.x == static_cast<double>(centro.x)) ? centro.x : static_cast<float>(q.x);
    const float ry = (q.y == static_cast<double>(centro.y)) ? centro.y : static_cast<float>(q.y);
    return Result<Vector2>::Ok(Vector2{rx, ry});
}

}  // namespace PoliticaCamara
