// U4-07.E05 — ver include/Game/Emisor/MovimientoParticula.h.
#include "Game/Emisor/MovimientoParticula.h"

#include <cmath>

namespace Game::Emisor {

void colocaParticula(const TipoDeParticula& tipo, Particula& p) {
    using M = MovimientoDeParticula;
    const double t = p.edad;
    const double k = p.k;
    const double vx = tipo.vx, vy = tipo.vy, lado = p.lado;
    const double vida = p.vida > 0.0f ? static_cast<double>(p.vida) : 1.0;
    double x = 0.0, y = 0.0;
    switch (tipo.movimiento) {
        case M::Salpica: {
            const double g = 2.0 * std::abs(vy) / vida;   // vuelve a su altura al final de SU vida
            x = p.vx0 * t + lado * vx * t;
            y = p.vy0 * t + vy * t + 0.5 * g * t * t;
            break;
        }
        case M::Deriva: {
            const double sentido = lado < 0.0 ? -1.0 : 1.0;
            x = p.vx0 * t + sentido * vx * t + std::sin(t * 5.0 + k) * 0.6;
            y = p.vy0 * t + vy * t;
            break;
        }
        case M::Converge: {
            const double r = p.radio0 * (1.0 - t / vida);
            x = r * std::cos(p.ang0);
            y = r * std::sin(p.ang0);
            break;
        }
        case M::Orbita: {
            const double ang = p.ang0 + t * vx * 0.5;
            const double r = (p.radio0 - 6.0) * 0.75 + 5.0;
            x = r * std::cos(ang);
            y = r * 0.5 * std::sin(ang) + vy * t;
            break;
        }
        case M::Expande:
            break;
        default:   // Sube, Cae, Chispazo
            x = p.vx0 * t + vx * std::sin(t * 3.0 + k) * 0.4 + lado * vx * t * 0.5;
            y = p.vy0 * t + vy * t;
            break;
    }
    p.x = static_cast<float>(x);
    p.y = static_cast<float>(y);
}

bool estaOculta(const TipoDeParticula& tipo, const Particula& p) {
    return (tipo.forma == FormaDeParticula::Rayo || tipo.movimiento == MovimientoDeParticula::Chispazo) &&
           p.edad > kEdadRayo;
}

}  // namespace Game::Emisor
