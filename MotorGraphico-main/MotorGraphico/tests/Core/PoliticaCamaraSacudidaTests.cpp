#include "Core/Math/PoliticaCamara.h"

#include "TestHarness.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace {

using PoliticaCamara::EstadoSacudida;
using PoliticaCamara::desplazamientoSacudida;

const double kNaN = std::numeric_limits<double>::quiet_NaN();
const double kInf = std::numeric_limits<double>::infinity();
const EstadoSacudida kS{8.0f, 0.24f};

Vector2 sacude(const EstadoSacudida& s, double t, std::uint32_t semilla, float factor = 1.0f) {
    const auto r = desplazamientoSacudida(s, t, semilla, factor);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

bool mismosBits(const Vector2& a, const Vector2& b) {
    return std::memcmp(&a, &b, sizeof(Vector2)) == 0;
}

double envolvente(const EstadoSacudida& s, double t) {
    const double k = 1.0 - t / s.duracion;
    return k * k;
}

MOTOR_TEST_CASE(misma_entrada_mismo_resultado_bit_a_bit) {
    for (int i = 0; i < 20; ++i) {
        const double t = i * 0.011;
        MOTOR_CHECK(mismosBits(sacude(kS, t, 42u), sacude(kS, t, 42u)));
    }
}

MOTOR_TEST_CASE(semillas_distintas_dan_secuencias_distintas) {
    int distintos = 0;
    for (int i = 0; i < 10; ++i) {
        const double t = i * 0.016;
        const Vector2 a = sacude(kS, t, 7u), b = sacude(kS, t, 8u);
        if (!mismosBits(a, b)) ++distintos;
    }
    MOTOR_CHECK(distintos >= 9);
    // Semillas consecutivas: el primer sorteo (t = 0) tampoco coincide.
    MOTOR_CHECK(!mismosBits(sacude(kS, 0.0, 1u), sacude(kS, 0.0, 2u)));
}

MOTOR_TEST_CASE(cota_por_envolvente_en_cada_t) {
    for (std::uint32_t semilla : {0u, 1u, 7u, 123456u}) {
        for (int i = 0; i < 240; ++i) {
            const double t = i * 0.001;
            const Vector2 d = sacude(kS, t, semilla);
            const double cota = kS.amplitudPx * envolvente(kS, t) * 1.000001 + 1e-6;
            MOTOR_CHECK(std::fabs(d.x) <= cota);
            MOTOR_CHECK(std::fabs(d.y) <= 0.6 * cota);
        }
    }
}

MOTOR_TEST_CASE(la_envolvente_cae_a_cero_al_final) {
    double maxIni = 0.0, maxFin = 0.0;
    for (std::uint32_t semilla = 0; semilla < 50; ++semilla) {
        maxIni = std::fmax(maxIni, std::fabs(sacude(kS, 0.01, semilla).x));
        maxFin = std::fmax(maxFin, std::fabs(sacude(kS, 0.239, semilla).x));
    }
    MOTOR_CHECK(maxIni > 1.0);
    MOTOR_CHECK(maxFin < 1e-3);
    // Y la amplitud real se acerca a la cota al principio (el ruido llena el rango).
    MOTOR_CHECK(maxIni > 0.8 * kS.amplitudPx * envolvente(kS, 0.01));
}

MOTOR_TEST_CASE(fuera_de_la_duracion_es_cero_exacto) {
    for (double t : {0.24, 0.2400001, 1.0, 1e9, -1e-9, -5.0}) {
        const Vector2 d = sacude(kS, t, 3u);
        MOTOR_CHECK(d.x == 0.0f && d.y == 0.0f);
    }
    const Vector2 antes = sacude(kS, 0.0, 3u);
    MOTOR_CHECK(antes.x != 0.0f || antes.y != 0.0f);
}

MOTOR_TEST_CASE(factor_movimiento_cero_apaga_y_medio_escala_a_la_mitad) {
    for (int i = 0; i < 15; ++i) {
        const double t = i * 0.015;
        const Vector2 apagada = sacude(kS, t, 9u, 0.0f);
        MOTOR_CHECK(apagada.x == 0.0f && apagada.y == 0.0f);
        const Vector2 pleno = sacude(kS, t, 9u, 1.0f);
        const Vector2 medio = sacude(kS, t, 9u, 0.5f);
        MOTOR_CHECK(std::fabs(medio.x - 0.5f * pleno.x) <= 1e-5f);
        MOTOR_CHECK(std::fabs(medio.y - 0.5f * pleno.y) <= 1e-5f);
        MOTOR_CHECK(std::fabs(pleno.x) > 0.0f || t > 0.2);
    }
}

MOTOR_TEST_CASE(es_funcion_de_t_no_acumulativa) {
    // Llegar a t = 0,1 tras muestrear con pasos distintos no cambia el valor.
    const Vector2 directo = sacude(kS, 0.1, 5u);
    for (double paso : {0.001, 0.0166, 0.05}) {
        for (double t = 0.0; t < 0.1 - 1e-12; t += paso) (void)sacude(kS, t, 5u);
        MOTOR_CHECK(mismosBits(directo, sacude(kS, 0.1, 5u)));
    }
    // Mismo fotograma (60 Hz) => mismo ruido; cambia solo la envolvente.
    const Vector2 a = sacude(kS, 0.100, 5u), b = sacude(kS, 0.105, 5u);
    const double cociente = envolvente(kS, 0.105) / envolvente(kS, 0.100);
    MOTOR_CHECK(std::fabs(b.x - a.x * cociente) <= 1e-4);
}

MOTOR_TEST_CASE(estado_degenerado_da_cero_exacto) {
    for (const EstadoSacudida& s : {EstadoSacudida{0.0f, 0.2f}, EstadoSacudida{-3.0f, 0.2f},
                                    EstadoSacudida{4.0f, 0.0f}, EstadoSacudida{4.0f, -1.0f}}) {
        const Vector2 d = sacude(s, 0.05, 1u);
        MOTOR_CHECK(d.x == 0.0f && d.y == 0.0f);
    }
}

MOTOR_TEST_CASE(entradas_invalidas_dan_error) {
    const float fNaN = std::numeric_limits<float>::quiet_NaN();
    MOTOR_CHECK(!desplazamientoSacudida({fNaN, 0.2f}, 0.1, 1u).isOk());
    MOTOR_CHECK(!desplazamientoSacudida({4.0f, fNaN}, 0.1, 1u).isOk());
    MOTOR_CHECK(!desplazamientoSacudida({4.0f, 0.2f}, kNaN, 1u).isOk());
    MOTOR_CHECK(!desplazamientoSacudida({4.0f, 0.2f}, kInf, 1u).isOk());
    MOTOR_CHECK(!desplazamientoSacudida({4.0f, 0.2f}, 0.1, 1u, -0.1f).isOk());
    MOTOR_CHECK(!desplazamientoSacudida({4.0f, 0.2f}, 0.1, 1u, 1.5f).isOk());
    MOTOR_CHECK(!desplazamientoSacudida({4.0f, 0.2f}, 0.1, 1u, fNaN).isOk());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
