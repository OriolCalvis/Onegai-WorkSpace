#include "Core/Math/PoliticaCamara.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

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

// splitmix64: mezcla la semilla para que semillas consecutivas no den el mismo primer sorteo.
std::uint64_t splitmix64(std::uint64_t x) {
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
}

// 24 bits -> [-1,1].
double aUnitario(std::uint64_t bits24) {
    return static_cast<double>(bits24 & 0xFFFFFFull) / 8388607.5 - 1.0;
}

constexpr double kFotogramasPorSegundo = 60.0;
constexpr double kFactorEjeY = 0.6;

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
    // Aristas alineadas con los ejes (cortes de la caja): resultado exacto, sin ruido de redondeo.
    if (ex == 0.0 && ey == 0.0) return a;
    if (ex == 0.0) return {a.x, std::fmin(std::fmax(a.y, b.y), std::fmax(std::fmin(a.y, b.y), q.y))};
    if (ey == 0.0) return {std::fmin(std::fmax(a.x, b.x), std::fmax(std::fmin(a.x, b.x), q.x)), a.y};
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

// Poligono convexo rombo ∩ [lox,hix] x [loy,hiy] (Sutherland-Hodgman, bordes inclusivos con eps).
// Vacio => lista vacia. Degenerado (segmento o punto) => vertices repetidos/colineales.
std::vector<P2> recortarRombo(const RomboMapa& rombo, double lox, double hix, double loy, double hiy) {
    constexpr double eps = 1e-9;
    std::vector<P2> poli = {aP2(rombo.superior), aP2(rombo.derecho), aP2(rombo.inferior),
                            aP2(rombo.izquierdo)};
    // Cuatro semiplanos: eje (0=x,1=y), cota y signo (+1: valor >= cota; -1: valor <= cota).
    const double cotas[4] = {lox, hix, loy, hiy};
    for (int h = 0; h < 4 && !poli.empty(); ++h) {
        const int eje = h / 2;
        const double signo = (h % 2 == 0) ? 1.0 : -1.0;
        auto dist = [&](const P2& p) { return signo * ((eje == 0 ? p.x : p.y) - cotas[h]) + eps; };
        std::vector<P2> salida;
        for (size_t k = 0; k < poli.size(); ++k) {
            const P2& a = poli[k];
            const P2& b = poli[(k + 1) % poli.size()];
            const double da = dist(a), db = dist(b);
            if (da >= 0.0) salida.push_back(a);
            if ((da >= 0.0) != (db >= 0.0)) {
                const double t = da / (da - db);
                P2 c{a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
                (eje == 0 ? c.x : c.y) = cotas[h];  // exacto sobre la cota recortada
                salida.push_back(c);
            }
        }
        poli.swap(salida);
    }
    return poli;
}

// Punto del poligono convexo (posiblemente degenerado) mas cercano a q: q mismo si esta dentro o a
// <= kToleranciaRomboPx; si no, el mas cercano sobre las aristas. Proyeccion euclidea: 1-Lipschitz.
P2 proyectarAPoligono(const std::vector<P2>& poli, const P2& q) {
    const size_t n = poli.size();
    P2 mejor = poli[0];
    double mejorD = std::numeric_limits<double>::infinity();
    double area2 = 0.0;
    for (size_t k = 0; k < n; ++k) {
        const P2& a = poli[k];
        const P2& b = poli[(k + 1) % n];
        area2 += a.x * b.y - b.x * a.y;
        const double l2 = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
        const P2 p = l2 > 0.0 ? masCercanoEnSegmento(a, b, q) : a;
        const double d = std::hypot(p.x - q.x, p.y - q.y);
        if (d < mejorD) { mejorD = d; mejor = p; }
    }
    if (mejorD <= kToleranciaRomboPx) return q;
    if (std::fabs(area2) > 1e-6) {
        bool dentro = true;
        for (size_t k = 0; k < n; ++k) {
            const P2& a = poli[k];
            const P2& b = poli[(k + 1) % n];
            const double cr = (b.x - a.x) * (q.y - a.y) - (b.y - a.y) * (q.x - a.x);
            if (area2 > 0.0 ? cr < 0.0 : cr > 0.0) dentro = false;
        }
        if (dentro) return q;
    }
    return mejor;
}

// Caja y rombo disjuntos: punto del ROMBO mas cercano al rectangulo (el par mas cercano de dos
// convexos disjuntos incluye un vertice). No depende del centro: es continuo en la frontera en que
// la interseccion se vacia (alli el contacto es el unico punto de la interseccion).
P2 contactoConRombo(const RomboMapa& r, double lox, double hix, double loy, double hiy) {
    const P2 vr[4] = {aP2(r.superior), aP2(r.derecho), aP2(r.inferior), aP2(r.izquierdo)};
    const P2 esq[4] = {{lox, loy}, {hix, loy}, {hix, hiy}, {lox, hiy}};
    P2 mejor = vr[0];
    double mejorD = std::numeric_limits<double>::infinity();
    for (const P2& v : vr) {  // vertice del rombo -> rectangulo
        const double d = std::hypot(v.x - std::fmin(hix, std::fmax(lox, v.x)),
                                    v.y - std::fmin(hiy, std::fmax(loy, v.y)));
        if (d < mejorD) { mejorD = d; mejor = v; }
    }
    for (const P2& c : esq) {  // esquina del rectangulo -> rombo
        const P2 b = proyectarARombo(r, c);
        const double d = std::hypot(b.x - c.x, b.y - c.y);
        if (d < mejorD) { mejorD = d; mejor = b; }
    }
    return mejor;
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
        const double xmin = limites.izquierda + mx, xmax = limites.derecha - mx;
        const double ymin = limites.arriba + my, ymax = limites.abajo - my;
        const double cx = (limites.izquierda + limites.derecha) * 0.5;
        const double cy = (limites.arriba + limites.abajo) * 0.5;
        // Rango de la caja para el centro (colapsado al centro de la caja si el mapa cabe).
        const double lox = xmin <= xmax ? xmin : cx, hix = xmin <= xmax ? xmax : cx;
        const double loy = ymin <= ymax ? ymin : cy, hiy = ymin <= ymax ? ymax : cy;
        // Resultado = proyeccion euclidea de centro sobre el poligono convexo caja ∩ rombo (continua,
        // 1-Lipschitz, idempotente; rango colapsado o casi colapsado incluidos). Vacio: contacto.
        const std::vector<P2> poli = recortarRombo(limites.rombo, lox, hix, loy, hiy);
        q = poli.empty() ? contactoConRombo(limites.rombo, lox, hix, loy, hiy)
                         : proyectarAPoligono(poli, P2{centro.x, centro.y});
    }
    // Eje ya valido: se devuelve el float original, sin redondeo.
    const float rx = (q.x == static_cast<double>(centro.x)) ? centro.x : static_cast<float>(q.x);
    const float ry = (q.y == static_cast<double>(centro.y)) ? centro.y : static_cast<float>(q.y);
    return Result<Vector2>::Ok(Vector2{rx, ry});
}

Result<Vector2> desplazamientoSacudida(const EstadoSacudida& s, double transcurrido,
                                       std::uint32_t semilla, float factorMovimiento) {
    if (!std::isfinite(s.amplitudPx) || !std::isfinite(s.duracion) || !std::isfinite(transcurrido)) {
        return Result<Vector2>::Error("PoliticaCamara::desplazamientoSacudida: estado o tiempo no finito");
    }
    if (!std::isfinite(factorMovimiento) || factorMovimiento < 0.0f || factorMovimiento > 1.0f) {
        return Result<Vector2>::Error("PoliticaCamara::desplazamientoSacudida: factorMovimiento fuera de [0,1]");
    }
    if (s.amplitudPx <= 0.0f || s.duracion <= 0.0f || factorMovimiento == 0.0f ||
        transcurrido < 0.0 || transcurrido >= static_cast<double>(s.duracion)) {
        return Result<Vector2>::Ok(Vector2{0.0f, 0.0f});
    }
    // El indice de fotograma debe caber en uint64 (conversion fuera de rango = UB).
    if (!(transcurrido * kFotogramasPorSegundo < 9223372036854775808.0)) {
        return Result<Vector2>::Error("PoliticaCamara::desplazamientoSacudida: transcurrido fuera de rango");
    }
    const double restante = 1.0 - transcurrido / static_cast<double>(s.duracion);
    const double amp = static_cast<double>(s.amplitudPx) * static_cast<double>(factorMovimiento) *
                       restante * restante;
    const std::uint64_t fotograma = static_cast<std::uint64_t>(transcurrido * kFotogramasPorSegundo);
    const std::uint64_t h = splitmix64((static_cast<std::uint64_t>(semilla) << 32) ^ splitmix64(fotograma));
    const double ux = aUnitario(h);
    const double uy = aUnitario(h >> 32);
    return Result<Vector2>::Ok(Vector2{static_cast<float>(amp * ux),
                                       static_cast<float>(amp * kFactorEjeY * uy)});
}

Result<PoseCamara> componerPose(const Vector2& posicionLogica, float zoom,
                                const Vector2& sacudidaPantalla, const TamanoVista& vista,
                                const LimitesMapa* limites, const PoliticaPose& politica) {
    if (!std::isfinite(zoom) || zoom <= 0.0f || !std::isfinite(posicionLogica.x) ||
        !std::isfinite(posicionLogica.y) || !std::isfinite(sacudidaPantalla.x) ||
        !std::isfinite(sacudidaPantalla.y)) {
        return Result<PoseCamara>::Error("PoliticaCamara::componerPose: zoom, posicion o sacudida invalidos");
    }
    Vector2 base = posicionLogica;
    if (limites != nullptr) {
        const auto b = limitarAlMapa(posicionLogica, zoom, vista, *limites);
        if (!b.isOk()) return Result<PoseCamara>::Error(b.errorMessage());
        base = b.value();
    }
    // Unica division por zoom: pixeles de pantalla -> pixeles de mundo. Un cero exacto no desplaza.
    const double z = static_cast<double>(zoom);
    Vector2 v = base;
    if (sacudidaPantalla.x != 0.0f) v.x = static_cast<float>(static_cast<double>(base.x) + sacudidaPantalla.x / z);
    if (sacudidaPantalla.y != 0.0f) v.y = static_cast<float>(static_cast<double>(base.y) + sacudidaPantalla.y / z);
    if (limites != nullptr && politica.sacudidaDentroDeLimites) {
        const auto r = limitarAlMapa(v, zoom, vista, *limites);
        if (!r.isOk()) return Result<PoseCamara>::Error(r.errorMessage());
        v = r.value();
    }
    return Result<PoseCamara>::Ok(PoseCamara{base, v, zoom});
}

}  // namespace PoliticaCamara
