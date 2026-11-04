#include "Core/Math/PoliticaCamara.h"

#include "TestHarness.h"

#include <cmath>
#include <limits>

namespace {

using PoliticaCamara::ZonaMuerta;
using PoliticaCamara::seguirConZonaMuerta;

const float kNaN = std::numeric_limits<float>::quiet_NaN();
const float kInf = std::numeric_limits<float>::infinity();
const ZonaMuerta kZona{96.0f, 48.0f};

bool cerca(float a, float b) { return std::fabs(a - b) <= 1e-4f; }

// Llama y exige Ok; devuelve el valor.
Vector2 sigue(Vector2 centro, float zoom, Vector2 actor, ZonaMuerta zona) {
    const auto r = seguirConZonaMuerta(centro, zoom, actor, zona);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

MOTOR_TEST_CASE(dentro_de_la_zona_no_se_mueve) {
    const Vector2 c{100.0f, 200.0f};
    const Vector2 r = sigue(c, 1.0f, {150.0f, 220.0f}, kZona);
    MOTOR_CHECK(r.x == c.x && r.y == c.y);
    const Vector2 s = sigue(c, 1.0f, c, kZona);
    MOTOR_CHECK(s.x == c.x && s.y == c.y);
}

MOTOR_TEST_CASE(cuatro_costados_dejan_al_actor_sobre_el_borde) {
    const Vector2 c{0.0f, 0.0f};
    Vector2 r = sigue(c, 1.0f, {200.0f, 0.0f}, kZona);  // derecha
    MOTOR_CHECK(cerca(r.x, 104.0f) && r.y == 0.0f);
    r = sigue(c, 1.0f, {-200.0f, 0.0f}, kZona);  // izquierda
    MOTOR_CHECK(cerca(r.x, -104.0f) && r.y == 0.0f);
    r = sigue(c, 1.0f, {0.0f, 100.0f}, kZona);  // abajo
    MOTOR_CHECK(r.x == 0.0f && cerca(r.y, 52.0f));
    r = sigue(c, 1.0f, {0.0f, -100.0f}, kZona);  // arriba
    MOTOR_CHECK(r.x == 0.0f && cerca(r.y, -52.0f));
}

MOTOR_TEST_CASE(ambos_ejes_a_la_vez) {
    const Vector2 r = sigue({10.0f, 20.0f}, 1.0f, {210.0f, -80.0f}, kZona);
    MOTOR_CHECK(cerca(r.x, 114.0f));   // 210 - 96
    MOTOR_CHECK(cerca(r.y, -32.0f));   // -80 + 48
}

MOTOR_TEST_CASE(solo_se_mueve_el_eje_que_sale) {
    const Vector2 r = sigue({0.0f, 0.0f}, 1.0f, {500.0f, 10.0f}, kZona);
    MOTOR_CHECK(cerca(r.x, 404.0f));
    MOTOR_CHECK(r.y == 0.0f);
}

MOTOR_TEST_CASE(exactamente_en_el_borde_no_se_mueve) {
    const Vector2 c{5.0f, 7.0f};
    Vector2 r = sigue(c, 1.0f, {5.0f + 96.0f, 7.0f + 48.0f}, kZona);
    MOTOR_CHECK(r.x == c.x && r.y == c.y);
    r = sigue(c, 1.0f, {5.0f - 96.0f, 7.0f - 48.0f}, kZona);
    MOTOR_CHECK(r.x == c.x && r.y == c.y);
}

// Frontera de la tolerancia: |d| == semi/zoom + 1e-4 EXACTO en double (semi=9999, zoom=10000,
// d=1). El centro no se mueve con <=; con < se empujaria 1e-4.
MOTOR_TEST_CASE(frontera_exacta_de_la_tolerancia_no_se_mueve) {
    const double lim = 9999.0f / static_cast<double>(10000.0f) + 1e-4;
    MOTOR_CHECK(lim == 1.0);  // premisa de la prueba
    const ZonaMuerta z{9999.0f, 9999.0f};
    Vector2 r = sigue({0.0f, 0.0f}, 10000.0f, {1.0f, -1.0f}, z);
    MOTOR_CHECK(r.x == 0.0f && r.y == 0.0f);
    r = sigue({0.0f, 0.0f}, 10000.0f, {-1.0f, 1.0f}, z);
    MOTOR_CHECK(r.x == 0.0f && r.y == 0.0f);
}

MOTOR_TEST_CASE(actor_a_mas_1e_3_del_borde_si_se_mueve) {
    const Vector2 r = sigue({0.0f, 0.0f}, 1.0f, {96.001f, 0.0f}, kZona);
    MOTOR_CHECK(r.x > 0.0f && cerca(r.x, 0.001f));
    MOTOR_CHECK(r.y == 0.0f);
}

MOTOR_TEST_CASE(zoom_cambia_la_zona_en_mundo) {
    // zoom 0.5: la zona en mundo es 192 x 96; zoom 2: 48 x 24.
    Vector2 r = sigue({0.0f, 0.0f}, 0.5f, {150.0f, 0.0f}, kZona);
    MOTOR_CHECK(r.x == 0.0f);
    r = sigue({0.0f, 0.0f}, 0.5f, {300.0f, 0.0f}, kZona);
    MOTOR_CHECK(cerca(r.x, 108.0f));
    r = sigue({0.0f, 0.0f}, 2.0f, {50.0f, 0.0f}, kZona);
    MOTOR_CHECK(cerca(r.x, 2.0f));
    r = sigue({0.0f, 0.0f}, 2.0f, {40.0f, 30.0f}, kZona);
    MOTOR_CHECK(r.x == 0.0f && cerca(r.y, 6.0f));
}

MOTOR_TEST_CASE(zona_cero_centra_siempre) {
    const ZonaMuerta cero{0.0f, 0.0f};
    Vector2 r = sigue({0.0f, 0.0f}, 1.0f, {37.5f, -12.25f}, cero);
    MOTOR_CHECK(cerca(r.x, 37.5f) && cerca(r.y, -12.25f));
    r = sigue({0.0f, 0.0f}, 1.7f, {-300.0f, 64.0f}, cero);
    MOTOR_CHECK(cerca(r.x, -300.0f) && cerca(r.y, 64.0f));
    // El actor ya centrado: no cambia.
    r = sigue({9.0f, 9.0f}, 1.0f, {9.0f, 9.0f}, cero);
    MOTOR_CHECK(r.x == 9.0f && r.y == 9.0f);
}

MOTOR_TEST_CASE(zona_mayor_que_la_vista_tolera_actor_lejano) {
    const ZonaMuerta enorme{100000.0f, 100000.0f};
    const Vector2 c{3.0f, 4.0f};
    Vector2 r = sigue(c, 1.0f, {5000.0f, -5000.0f}, enorme);
    MOTOR_CHECK(r.x == c.x && r.y == c.y);
    r = sigue(c, 1.0f, {200000.0f, 0.0f}, enorme);
    MOTOR_CHECK(cerca(r.x, 100000.0f) && r.y == c.y);
}

MOTOR_TEST_CASE(idempotente_aplicar_dos_veces_es_una) {
    const Vector2 actor{-333.0f, 211.0f};
    for (float zoom : {0.5f, 1.0f, 1.7f, 2.0f}) {
        const Vector2 a = sigue({10.0f, -10.0f}, zoom, actor, kZona);
        const Vector2 b = sigue(a, zoom, actor, kZona);
        MOTOR_CHECK(a.x == b.x && a.y == b.y);
    }
}

MOTOR_TEST_CASE(entradas_invalidas_dan_error) {
    const Vector2 c{0.0f, 0.0f}, a{10.0f, 10.0f};
    MOTOR_CHECK(!seguirConZonaMuerta(c, 0.0f, a, kZona).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta(c, -1.0f, a, kZona).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta(c, kNaN, a, kZona).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta(c, kInf, a, kZona).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta(c, 1.0f, a, {-1.0f, 0.0f}).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta(c, 1.0f, a, {0.0f, -0.5f}).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta(c, 1.0f, a, {kNaN, 0.0f}).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta(c, 1.0f, a, {0.0f, kInf}).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta({kNaN, 0.0f}, 1.0f, a, kZona).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta({0.0f, kInf}, 1.0f, a, kZona).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta(c, 1.0f, {kNaN, 0.0f}, kZona).isOk());
    MOTOR_CHECK(!seguirConZonaMuerta(c, 1.0f, {0.0f, -kInf}, kZona).isOk());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
