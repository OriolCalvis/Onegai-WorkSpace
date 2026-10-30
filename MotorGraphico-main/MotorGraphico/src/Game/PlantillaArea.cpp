// AV13-08 — Ver Game/PlantillaArea.h: formas y corte por vista sin estado.
#include "Game/PlantillaArea.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>

namespace Game {
namespace {

using Entero = std::int64_t;

int signo(Entero n) { return (n > 0) - (n < 0); }

// El mismo desempate de Bresenham que TableroCombate::seVen. Se usan
// enteros amplios para que restar coordenadas o duplicar el error no desborde.
struct Rayo {
    Entero x, y, dx, dy, error;
    int sx, sy;

    Rayo(GridCoord a, GridCoord b)
        : x(a.x), y(a.y), dx(std::abs(Entero(b.x) - a.x)),
          dy(-std::abs(Entero(b.y) - a.y)), error(dx + dy),
          sx(signo(Entero(b.x) - a.x)), sy(signo(Entero(b.y) - a.y)) {}

    void avanza() {
        const Entero doble = 2 * error;
        if (doble >= dy) { error += dy; x += sx; }
        if (doble <= dx) { error += dx; y += sy; }
    }
};

bool seVen(GridCoord origen, GridCoord destino,
           const std::function<bool(int, int)>& bloquea) {
    if (!bloquea) return true;
    Rayo r(origen, destino);
    while (r.x != destino.x || r.y != destino.y) {
        r.avanza();
        if (r.x == destino.x && r.y == destino.y) break;
        if (bloquea(static_cast<int>(r.x), static_cast<int>(r.y))) return false;
    }
    return true;
}

void agrega(std::vector<GridCoord>& resultado, Entero x, Entero y, GridCoord origen,
            const std::function<bool(int, int)>& bloquea) {
    constexpr Entero minimo = std::numeric_limits<int>::min();
    constexpr Entero maximo = std::numeric_limits<int>::max();
    if (x < minimo || x > maximo || y < minimo || y > maximo) return;
    const GridCoord c{static_cast<int>(x), static_cast<int>(y)};
    if (seVen(origen, c, bloquea)) resultado.push_back(c);
}

}  // namespace

std::vector<GridCoord> casillas(const PlantillaArea& plantilla, GridCoord origen,
                                GridCoord haciaDonde,
                                const std::function<bool(int, int)>& bloquea) {
    std::vector<GridCoord> resultado;
    const Entero n = plantilla.tamano;
    if (n < 0) return resultado;
    const int sx = signo(Entero(haciaDonde.x) - origen.x);
    const int sy = signo(Entero(haciaDonde.y) - origen.y);

    if (plantilla.forma == PlantillaArea::Forma::Linea) {
        if (sx == 0 && sy == 0) return resultado;
        Rayo r(origen, haciaDonde);
        resultado.reserve(static_cast<std::size_t>(n));
        for (Entero paso = 0; paso < n; ++paso) {
            r.avanza();
            agrega(resultado, r.x, r.y, origen, bloquea);
        }
        std::sort(resultado.begin(), resultado.end(), [](GridCoord a, GridCoord b) {
            return a.y < b.y || (a.y == b.y && a.x < b.x);
        });
        return resultado;
    }

    if (plantilla.forma == PlantillaArea::Forma::Cuadrado) {
        for (Entero y = 0; y < n; ++y)
            for (Entero x = 0; x < n; ++x)
                agrega(resultado, Entero(origen.x) - n / 2 + x,
                       Entero(origen.y) - n / 2 + y, origen, bloquea);
        return resultado;
    }

    const bool cono = plantilla.forma == PlantillaArea::Forma::Cono;
    if (!cono && plantilla.forma != PlantillaArea::Forma::Circulo) return resultado;
    if (cono && sx == 0 && sy == 0) return resultado;
    for (Entero y = -n; y <= n; ++y) {
        for (Entero x = -n; x <= n; ++x) {
            if (x * x + y * y > n * n) continue;
            if (cono) {
                // Frente positivo y angulo <= 45 grados respecto al eje.
                // Producto escalar/cruzado evita trigonometria y redondeos,
                // tambien en las cuatro direcciones diagonales.
                const Entero frente = x * sx + y * sy;
                const Entero lateral = x * sy - y * sx;
                if (frente <= 0 || std::abs(lateral) > frente) continue;
            }
            agrega(resultado, Entero(origen.x) + x, Entero(origen.y) + y, origen, bloquea);
        }
    }
    return resultado;
}

}  // namespace Game
