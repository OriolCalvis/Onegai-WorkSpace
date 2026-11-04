#include "Core/Math/Interpolacion.h"

#include <cmath>

namespace Interpolacion {

Result<double> progreso(double transcurrido, double duracion) {
    if (!std::isfinite(transcurrido)) {
        return Result<double>::Error("Interpolacion::progreso: transcurrido no es finito");
    }
    if (!std::isfinite(duracion)) {
        return Result<double>::Error("Interpolacion::progreso: duracion no es finita");
    }
    if (duracion < 0.0) {
        return Result<double>::Error("Interpolacion::progreso: duracion negativa");
    }
    if (duracion == 0.0) {
        return Result<double>::Ok(1.0);
    }
    const double p = transcurrido / duracion;
    if (p <= 0.0) return Result<double>::Ok(0.0);
    if (p >= 1.0) return Result<double>::Ok(1.0);
    return Result<double>::Ok(p);
}

double curva(TipoCurva tipo, double t) {
    if (std::isnan(t) || t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    const double u = 1.0 - t;
    const double v = -2.0 * t + 2.0;
    switch (tipo) {
        case TipoCurva::Lineal: return t;
        case TipoCurva::EntradaCuadratica: return t * t;
        case TipoCurva::SalidaCuadratica: return 1.0 - u * u;
        case TipoCurva::EntradaSalidaCuadratica:
            return t < 0.5 ? 2.0 * t * t : 1.0 - v * v / 2.0;
        case TipoCurva::EntradaCubica: return t * t * t;
        case TipoCurva::SalidaCubica: return 1.0 - u * u * u;
        case TipoCurva::EntradaSalidaCubica:
            return t < 0.5 ? 4.0 * t * t * t : 1.0 - v * v * v / 2.0;
    }
    return t;
}

double tweenEscalar(double a, double b, double p) {
    // Las guardes de p van primero: p<=0 -> a y p>=1 -> b exactos (con el signo de b aunque a==b).
    if (p <= 0.0) return a;
    if (p >= 1.0) return b;
    if (std::isnan(p)) return std::nan("");
    if (a == b) return a;
    // Monotona en p: p*(b-a) y la suma son monotonas con redondeo al mas cercano.
    // Si b-a desborda (extremos opuestos enormes) se reparte en mitades, que no desbordan:
    // 2*(a/2 + p*(b/2 - a/2)); el *2 final solo es inexacto si el resultado excede DBL_MAX (se acota).
    const double d = b - a;
    const double x = std::isfinite(d) ? a + p * d : 2.0 * (0.5 * a + p * (0.5 * b - 0.5 * a));
    const double lo = a < b ? a : b;
    const double hi = a < b ? b : a;
    return x < lo ? lo : (x > hi ? hi : x);
}

Vector2 tweenVec2(const Vector2& a, const Vector2& b, double p) {
    return {static_cast<float>(tweenEscalar(a.x, b.x, p)),
            static_cast<float>(tweenEscalar(a.y, b.y, p))};
}

Result<TipoCurva> curvaDesdeTexto(const std::string& texto) {
    struct Par { const char* nombre; TipoCurva tipo; };
    static const Par kTabla[] = {
        {"lineal", TipoCurva::Lineal},
        {"entrada_cuadratica", TipoCurva::EntradaCuadratica},
        {"salida_cuadratica", TipoCurva::SalidaCuadratica},
        {"entrada_salida_cuadratica", TipoCurva::EntradaSalidaCuadratica},
        {"entrada_cubica", TipoCurva::EntradaCubica},
        {"salida_cubica", TipoCurva::SalidaCubica},
        {"entrada_salida_cubica", TipoCurva::EntradaSalidaCubica},
    };
    for (const Par& p : kTabla) {
        if (texto == p.nombre) return Result<TipoCurva>::Ok(p.tipo);
    }
    return Result<TipoCurva>::Error("Interpolacion::curvaDesdeTexto: curva desconocida '" + texto + "'");
}

}  // namespace Interpolacion
