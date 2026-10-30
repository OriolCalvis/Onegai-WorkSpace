#include "Core/Math/Interpolacion.h"

#include "TestHarness.h"

#include <cfloat>
#include <cmath>
#include <limits>
#include <string>

namespace {

const double kNaN = std::numeric_limits<double>::quiet_NaN();
const double kInf = std::numeric_limits<double>::infinity();

void esperaOk(double transcurrido, double duracion, double esperado) {
    const auto r = Interpolacion::progreso(transcurrido, duracion);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == esperado);
}

void esperaError(double transcurrido, double duracion) {
    MOTOR_CHECK(!Interpolacion::progreso(transcurrido, duracion).isOk());
}

MOTOR_TEST_CASE(progreso_inicio_medio_y_final) {
    esperaOk(0.0, 1.0, 0.0);
    esperaOk(0.5, 1.0, 0.5);
    esperaOk(1.0, 2.0, 0.5);
    esperaOk(1.0, 1.0, 1.0);
}

MOTOR_TEST_CASE(progreso_acota_por_arriba_y_por_abajo) {
    esperaOk(3.0, 1.0, 1.0);
    esperaOk(-1.0, 1.0, 0.0);
}

MOTOR_TEST_CASE(progreso_duracion_cero_termina_siempre) {
    esperaOk(2.0, 0.0, 1.0);
    esperaOk(0.0, 0.0, 1.0);
    esperaOk(-1.0, 0.0, 1.0);
}

MOTOR_TEST_CASE(progreso_rechaza_valores_no_finitos) {
    esperaError(kNaN, 1.0);
    esperaError(kInf, 1.0);
    esperaError(-kInf, 1.0);
    esperaError(0.5, kNaN);
    esperaError(0.5, kInf);
    esperaError(0.5, -kInf);
    esperaError(kNaN, 0.0);
    esperaError(kInf, 0.0);
}

MOTOR_TEST_CASE(progreso_rechaza_duracion_negativa) {
    esperaError(1.0, -1.0);
    esperaError(0.0, -0.5);
}

using Interpolacion::TipoCurva;
using Interpolacion::curva;

const TipoCurva kTodas[] = {TipoCurva::Lineal, TipoCurva::EntradaCuadratica,
                            TipoCurva::SalidaCuadratica, TipoCurva::EntradaSalidaCuadratica,
                            TipoCurva::EntradaCubica, TipoCurva::SalidaCubica,
                            TipoCurva::EntradaSalidaCubica};
const double kTol = 1e-12;

bool cerca(double a, double b) { return std::fabs(a - b) <= kTol; }

MOTOR_TEST_CASE(curva_extremos_exactos) {
    for (TipoCurva k : kTodas) {
        MOTOR_CHECK(curva(k, 0.0) == 0.0);
        MOTOR_CHECK(curva(k, 1.0) == 1.0);
    }
}

MOTOR_TEST_CASE(curva_valores_a_mano) {
    MOTOR_CHECK(cerca(curva(TipoCurva::Lineal, 0.3), 0.3));
    MOTOR_CHECK(cerca(curva(TipoCurva::EntradaCuadratica, 0.5), 0.25));
    MOTOR_CHECK(cerca(curva(TipoCurva::SalidaCuadratica, 0.5), 0.75));
    MOTOR_CHECK(cerca(curva(TipoCurva::EntradaSalidaCuadratica, 0.25), 0.125));
    MOTOR_CHECK(cerca(curva(TipoCurva::EntradaSalidaCuadratica, 0.5), 0.5));
    MOTOR_CHECK(cerca(curva(TipoCurva::EntradaSalidaCuadratica, 0.75), 0.875));
    MOTOR_CHECK(cerca(curva(TipoCurva::EntradaCubica, 0.5), 0.125));
    MOTOR_CHECK(cerca(curva(TipoCurva::SalidaCubica, 0.5), 0.875));
    MOTOR_CHECK(cerca(curva(TipoCurva::EntradaSalidaCubica, 0.25), 0.0625));
    MOTOR_CHECK(cerca(curva(TipoCurva::EntradaSalidaCubica, 0.5), 0.5));
    MOTOR_CHECK(cerca(curva(TipoCurva::EntradaSalidaCubica, 0.75), 0.9375));
}

MOTOR_TEST_CASE(curva_simetrias) {
    const TipoCurva inOut[] = {TipoCurva::EntradaSalidaCuadratica, TipoCurva::EntradaSalidaCubica};
    const TipoCurva pares[][2] = {{TipoCurva::EntradaCuadratica, TipoCurva::SalidaCuadratica},
                                  {TipoCurva::EntradaCubica, TipoCurva::SalidaCubica}};
    for (int i = 0; i <= 100; ++i) {
        const double t = i / 100.0;
        for (TipoCurva k : inOut) MOTOR_CHECK(cerca(curva(k, t) + curva(k, 1.0 - t), 1.0));
        for (const auto& p : pares) MOTOR_CHECK(cerca(curva(p[1], t), 1.0 - curva(p[0], 1.0 - t)));
    }
}

MOTOR_TEST_CASE(curva_monotona_en_rejilla) {
    for (TipoCurva k : kTodas) {
        double previo = curva(k, 0.0);
        for (int i = 1; i <= 1000; ++i) {
            const double actual = curva(k, i / 1000.0);
            MOTOR_CHECK(actual >= previo);
            previo = actual;
        }
    }
}

MOTOR_TEST_CASE(curva_coincide_con_formulas_de_camera) {
    for (int i = 0; i <= 100; ++i) {
        const double t = i / 100.0;
        // Literales copiados de Camera.cpp:22 y :24 (en double).
        const double inOutQuad = t < 0.5 ? 2.0 * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 2.0) / 2.0;
        const double outCubic = 1.0 - std::pow(1.0 - t, 3.0);
        MOTOR_CHECK(cerca(curva(TipoCurva::EntradaSalidaCuadratica, t), inOutQuad));
        MOTOR_CHECK(cerca(curva(TipoCurva::SalidaCubica, t), outCubic));
    }
}

MOTOR_TEST_CASE(curva_acota_fuera_de_rango_y_nan) {
    for (TipoCurva k : kTodas) {
        MOTOR_CHECK(curva(k, -0.5) == 0.0);
        MOTOR_CHECK(curva(k, 2.0) == 1.0);
        MOTOR_CHECK(curva(k, kNaN) == 0.0);
    }
}

MOTOR_TEST_CASE(curva_parser_acepta_los_siete_nombres) {
    const char* nombres[] = {"lineal", "entrada_cuadratica", "salida_cuadratica",
                             "entrada_salida_cuadratica", "entrada_cubica", "salida_cubica",
                             "entrada_salida_cubica"};
    for (int i = 0; i < 7; ++i) {
        const auto r = Interpolacion::curvaDesdeTexto(nombres[i]);
        MOTOR_CHECK(r.isOk());
        if (r.isOk()) MOTOR_CHECK(r.value() == kTodas[i]);
    }
}

MOTOR_TEST_CASE(curva_parser_rechaza_desconocidos) {
    const char* malos[] = {"", "Lineal", "LINEAL", "Entrada_Cuadratica", "entrada_cuadratica ",
                           " lineal", "cuadratica", "ease_in", "entrada cuadratica", "smoothstep"};
    for (const char* m : malos) MOTOR_CHECK(!Interpolacion::curvaDesdeTexto(m).isOk());
}

using Interpolacion::tweenEscalar;

bool cercaRel(double x, double esperado, double a, double b) {
    const double escala = std::fmax(1.0, std::fmax(std::fabs(a), std::fabs(b)));
    return std::fabs(x - esperado) <= kTol * escala;
}

MOTOR_TEST_CASE(tween_iguales_devuelve_a) {
    const double ps[] = {-3.0, 0.0, 0.1, 0.5, 0.9, 1.0, 7.0};
    for (double p : ps) {
        MOTOR_CHECK(tweenEscalar(0.1, 0.1, p) == 0.1);
        MOTOR_CHECK(tweenEscalar(-2.5, -2.5, p) == -2.5);
        MOTOR_CHECK(tweenEscalar(1e300, 1e300, p) == 1e300);
    }
    // Sin la guarda a==b la formula convexa no da exacto en estos casos (matan al mutante).
    MOTOR_CHECK(tweenEscalar(0.1, 0.1, 0.18) == 0.1);
    MOTOR_CHECK(tweenEscalar(0.3, 0.3, 0.08) == 0.3);
    for (int i = 0; i <= 100; ++i) {
        const double p = i / 100.0;
        MOTOR_CHECK(tweenEscalar(0.1, 0.1, p) == 0.1);
        MOTOR_CHECK(tweenEscalar(0.3, 0.3, p) == 0.3);
    }
}

MOTOR_TEST_CASE(tween_ascendente_y_descendente) {
    MOTOR_CHECK(cercaRel(tweenEscalar(10.0, 20.0, 0.25), 12.5, 10.0, 20.0));
    MOTOR_CHECK(cercaRel(tweenEscalar(20.0, 10.0, 0.25), 17.5, 10.0, 20.0));
    MOTOR_CHECK(cercaRel(tweenEscalar(0.0, 1.0, 0.3), 0.3, 0.0, 1.0));
    MOTOR_CHECK(cercaRel(tweenEscalar(1.0, 0.0, 0.3), 0.7, 0.0, 1.0));
}

MOTOR_TEST_CASE(tween_extremos_exactos) {
    const double pares[][2] = {{0.1, 0.7}, {0.7, 0.1}, {-0.3, 0.9}, {1e16, 1.0}, {1.0, 1e16},
                               {1e-20, 3.0}, {-1e300, 1e300}, {3.0, 1e-20}};
    for (const auto& q : pares) {
        MOTOR_CHECK(tweenEscalar(q[0], q[1], 0.0) == q[0]);
        MOTOR_CHECK(tweenEscalar(q[0], q[1], 1.0) == q[1]);
    }
    // La formula ingenua a+(b-a)*1 no devuelve b en estos casos; la rama p>=1 si.
    const double a = 1.0, b = 1e-20;
    MOTOR_CHECK(a + (b - a) * 1.0 != b);
    MOTOR_CHECK(tweenEscalar(a, b, 1.0) == b);
}

MOTOR_TEST_CASE(tween_punto_medio_y_negativos) {
    MOTOR_CHECK(cercaRel(tweenEscalar(0.0, 10.0, 0.5), 5.0, 0.0, 10.0));
    MOTOR_CHECK(cercaRel(tweenEscalar(-10.0, 10.0, 0.5), 0.0, -10.0, 10.0));
    MOTOR_CHECK(cercaRel(tweenEscalar(-8.0, -4.0, 0.5), -6.0, -8.0, -4.0));
    MOTOR_CHECK(cercaRel(tweenEscalar(-4.0, -8.0, 0.25), -5.0, -8.0, -4.0));
    MOTOR_CHECK(std::isfinite(tweenEscalar(-1e308, 1e308, 0.5)));
    MOTOR_CHECK(cercaRel(tweenEscalar(-1e308, 1e308, 0.5), 0.0, -1e308, 1e308));
}

MOTOR_TEST_CASE(tween_acota_p_fuera_de_rango) {
    MOTOR_CHECK(tweenEscalar(2.0, 8.0, -0.5) == 2.0);
    MOTOR_CHECK(tweenEscalar(2.0, 8.0, -1e9) == 2.0);
    MOTOR_CHECK(tweenEscalar(2.0, 8.0, 1.5) == 8.0);
    MOTOR_CHECK(tweenEscalar(8.0, 2.0, 1e9) == 2.0);
    MOTOR_CHECK(tweenEscalar(8.0, 2.0, -2.0) == 8.0);
}

MOTOR_TEST_CASE(tween_extremos_conservan_signo_de_b) {
    MOTOR_CHECK(std::signbit(tweenEscalar(+0.0, -0.0, 1.0)));
    MOTOR_CHECK(!std::signbit(tweenEscalar(-0.0, +0.0, 1.0)));
    MOTOR_CHECK(std::signbit(tweenEscalar(+0.0, -0.0, 5.0)));
    MOTOR_CHECK(std::signbit(tweenEscalar(-0.0, +0.0, 0.0)));
    MOTOR_CHECK(!std::signbit(tweenEscalar(+0.0, -0.0, 0.0)));
    MOTOR_CHECK(std::signbit(tweenEscalar(-0.0, +0.0, -1.0)));  // p<=0 -> a (con su signo)
}

MOTOR_TEST_CASE(tween_rango_exacto_sin_tolerancia_y_monotono) {
    const double a = 1.1;
    const double pares[][2] = {{a, std::nextafter(a, 2.0)}, {std::nextafter(a, 2.0), a},
                               {-a, -std::nextafter(a, 2.0)}, {0.1, 0.7}, {1e16, 1.0}, {-3.0, 5e-324}};
    for (const auto& q : pares) {
        const double lo = std::fmin(q[0], q[1]), hi = std::fmax(q[0], q[1]);
        double prev = q[0];
        for (int i = 0; i <= 100000; ++i) {
            const double v = tweenEscalar(q[0], q[1], i / 100000.0);
            MOTOR_CHECK(v >= lo && v <= hi);
            MOTOR_CHECK(q[1] >= q[0] ? v >= prev : v <= prev);
            prev = v;
        }
    }
    // Caso de Codex: a*(1-p)+b*p daba 1.0999999999999999 < a con p=0.01.
    MOTOR_CHECK(tweenEscalar(a, std::nextafter(a, 2.0), 0.01) >= a);
}

MOTOR_TEST_CASE(tween_extremos_opuestos_enormes_son_finitos_y_acotados) {
    for (const double p : {0.25, 0.5, 0.75}) {
        const double v = tweenEscalar(-1.7e308, 1.7e308, p);
        MOTOR_CHECK(std::isfinite(v) && v >= -1.7e308 && v <= 1.7e308);
    }
    MOTOR_CHECK(tweenEscalar(-1.7e308, 1.7e308, 0.25) < tweenEscalar(-1.7e308, 1.7e308, 0.75));
}

using Interpolacion::tweenVec2;

bool igualVec(const Vector2& v, float x, float y) { return v.x == x && v.y == y; }

MOTOR_TEST_CASE(tweenVec2_extremos_exactos) {
    const Vector2 a{0.1f, -3.3f}, b{7.7f, 1e-20f};
    MOTOR_CHECK(igualVec(tweenVec2(a, b, 0.0), a.x, a.y));
    MOTOR_CHECK(igualVec(tweenVec2(a, b, 1.0), b.x, b.y));
}

MOTOR_TEST_CASE(tweenVec2_punto_medio_y_descenso) {
    MOTOR_CHECK(igualVec(tweenVec2({0.0f, 0.0f}, {10.0f, 20.0f}, 0.5), 5.0f, 10.0f));
    MOTOR_CHECK(igualVec(tweenVec2({10.0f, 20.0f}, {0.0f, 0.0f}, 0.25), 7.5f, 15.0f));
}

MOTOR_TEST_CASE(tweenVec2_componentes_mixtas) {
    // x sube, y baja: cada componente es independiente (detecta x/y intercambiados).
    MOTOR_CHECK(igualVec(tweenVec2({0.0f, 100.0f}, {10.0f, 0.0f}, 0.25), 2.5f, 75.0f));
    MOTOR_CHECK(igualVec(tweenVec2({5.0f, 1.0f}, {5.0f, 9.0f}, 0.5), 5.0f, 5.0f));
}

MOTOR_TEST_CASE(tweenVec2_acota_p_fuera_de_rango) {
    const Vector2 a{2.0f, 8.0f}, b{8.0f, 2.0f};
    MOTOR_CHECK(igualVec(tweenVec2(a, b, -0.5), a.x, a.y));
    MOTOR_CHECK(igualVec(tweenVec2(a, b, -1e9), a.x, a.y));
    MOTOR_CHECK(igualVec(tweenVec2(a, b, 1.5), b.x, b.y));
    MOTOR_CHECK(igualVec(tweenVec2(a, b, 1e9), b.x, b.y));
}

MOTOR_TEST_CASE(tweenVec2_iguales_devuelve_a) {
    const Vector2 a{0.1f, 0.3f};
    for (int i = 0; i <= 100; ++i) {
        MOTOR_CHECK(igualVec(tweenVec2(a, a, i / 100.0), a.x, a.y));
    }
}

float viaDouble(float a, float b, double p) {
    return static_cast<float>(static_cast<double>(a) * (1.0 - p) + static_cast<double>(b) * p);
}

MOTOR_TEST_CASE(tweenVec2_calcula_en_double_y_convierte_una_vez) {
    // p interiores no exactos en float: mezclar en float daria otro resultado (p.ej. 1.46800005f).
    const double ps[] = {0.18, 0.37, 0.91};
    const float pares[][2] = {{0.1f, 7.7f}, {7.7f, 0.1f}, {-3.3f, 0.7f}};
    for (double p : ps) {
        for (const auto& q : pares) {
            const Vector2 r = tweenVec2({q[0], q[1]}, {q[1], q[0]}, p);
            MOTOR_CHECK(r.x == viaDouble(q[0], q[1], p));
            MOTOR_CHECK(r.y == viaDouble(q[1], q[0], p));
        }
    }
}

MOTOR_TEST_CASE(tweenVec2_extremos_float_max) {
    const Vector2 r = tweenVec2({-FLT_MAX, FLT_MAX}, {FLT_MAX, -FLT_MAX}, 0.5);
    MOTOR_CHECK(std::isfinite(r.x) && std::isfinite(r.y));
    MOTOR_CHECK(igualVec(r, 0.0f, 0.0f));
    MOTOR_CHECK(igualVec(tweenVec2({-FLT_MAX, 0.0f}, {FLT_MAX, 0.0f}, 1.0), FLT_MAX, 0.0f));
}

MOTOR_TEST_CASE(tweenVec2_subnormales) {
    const float sub = FLT_MIN / 4.0f;  // subnormal
    MOTOR_CHECK(sub > 0.0f && sub < FLT_MIN);
    const Vector2 a{0.0f, sub}, b{sub, 0.0f};
    MOTOR_CHECK(igualVec(tweenVec2(a, b, 0.0), a.x, a.y));
    MOTOR_CHECK(igualVec(tweenVec2(a, b, 1.0), b.x, b.y));
    const Vector2 m = tweenVec2(a, b, 0.5);
    MOTOR_CHECK(m.x == viaDouble(a.x, b.x, 0.5) && m.y == viaDouble(a.y, b.y, 0.5));
    MOTOR_CHECK(m.x == sub / 2.0f && m.y == sub / 2.0f);
    MOTOR_CHECK(igualVec(tweenVec2({sub, sub}, {sub, sub}, 0.3), sub, sub));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
