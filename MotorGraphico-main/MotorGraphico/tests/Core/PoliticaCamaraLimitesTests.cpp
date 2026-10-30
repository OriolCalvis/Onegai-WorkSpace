#include "Core/Math/PoliticaCamara.h"

#include "TestHarness.h"

#include <cmath>
#include <limits>

namespace {

using PoliticaCamara::LimitesMapa;
using PoliticaCamara::TamanoVista;
using PoliticaCamara::limitarAlMapa;
using PoliticaCamara::limitesDeMapa;

const float kNaN = std::numeric_limits<float>::quiet_NaN();
const float kInf = std::numeric_limits<float>::infinity();
const TamanoVista kVista{800, 600};

// Tolerancia fijada en el header: 1e-3 px de mundo absolutos para |coord| <= 4096.
bool cerca(float a, float b, float tol = 1e-3f) { return std::fabs(a - b) <= tol; }

LimitesMapa limites(int c, int f, int tw = 64, int th = 32, float margen = 0.0f) {
    const auto r = limitesDeMapa(c, f, tw, th, margen);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

Vector2 limita(Vector2 centro, float zoom, TamanoVista v, const LimitesMapa& l) {
    const auto r = limitarAlMapa(centro, zoom, v, l);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

MOTOR_TEST_CASE(limites_mapa_64x64_y_3x2_y_1x1) {
    const LimitesMapa a = limites(64, 64);
    MOTOR_CHECK(a.izquierda == -2016.0f && a.derecha == 2080.0f);
    MOTOR_CHECK(a.arriba == 0.0f && a.abajo == 2048.0f);
    const LimitesMapa b = limites(3, 2);
    MOTOR_CHECK(b.izquierda == -32.0f && b.derecha == 128.0f);
    MOTOR_CHECK(b.arriba == 0.0f && b.abajo == 80.0f);
    const LimitesMapa c = limites(1, 1);
    MOTOR_CHECK(c.izquierda == 0.0f && c.derecha == 64.0f);
    MOTOR_CHECK(c.arriba == 0.0f && c.abajo == 32.0f);
}

MOTOR_TEST_CASE(margen_superior_sube_el_borde) {
    const LimitesMapa l = limites(3, 2, 64, 32, 96.0f);
    MOTOR_CHECK(l.arriba == -96.0f && l.abajo == 80.0f);
}

MOTOR_TEST_CASE(caso_zoom_1_7_centro_x_menos_2000) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 r = limita({-2000.0f, 1000.0f}, 1.7f, kVista, l);
    MOTOR_CHECK(cerca(r.x, -1780.705882f));
    MOTOR_CHECK(r.y == 1000.0f);  // el eje Y ya valido queda identico
}

MOTOR_TEST_CASE(caso_zoom_1_centro_origen_solo_recorta_y) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 r = limita({0.0f, 0.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(r.x == 0.0f && cerca(r.y, 300.0f));
}

MOTOR_TEST_CASE(caso_esquina_superior_izquierda) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 r = limita({-2000.0f, -50.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, -1616.0f) && cerca(r.y, 300.0f));
}

MOTOR_TEST_CASE(caso_zoom_3_esquina_inferior_derecha) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 r = limita({2080.0f, 2048.0f}, 3.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 1946.666667f) && cerca(r.y, 1948.0f));
}

MOTOR_TEST_CASE(centro_exacto_min_mas_mitad_es_identidad) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 c{-1616.0f, 300.0f};
    const Vector2 r = limita(c, 1.0f, kVista, l);
    MOTOR_CHECK(r.x == c.x && r.y == c.y);
}

MOTOR_TEST_CASE(centro_a_1e_3_del_borde_se_recorta) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 r = limita({-1616.001f, 299.999f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, -1616.0f) && cerca(r.y, 300.0f));
}

MOTOR_TEST_CASE(mapa_3x2_menor_que_la_vista_se_centra_en_ambos_ejes) {
    const LimitesMapa l = limites(3, 2);
    Vector2 r = limita({0.0f, 0.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 48.0f) && cerca(r.y, 40.0f));
    // Cambio de mapa: destino viejo lejano.
    r = limita({1500.0f, 1000.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 48.0f) && cerca(r.y, 40.0f));
}

MOTOR_TEST_CASE(mapa_menor_que_la_vista_solo_en_un_eje) {
    // 30 columnas x 1 fila: ancho 992 > 800; alto 496 < 600.
    const LimitesMapa l = limites(30, 1);
    MOTOR_CHECK(l.derecha == 992.0f && l.abajo == 496.0f);
    const Vector2 r = limita({-5000.0f, 900.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, l.izquierda + 400.0f));  // clamp en X
    MOTOR_CHECK(cerca(r.y, l.abajo * 0.5f));        // centrado en Y
}

MOTOR_TEST_CASE(zoom_que_cambia_vuelve_a_caber_o_no) {
    const LimitesMapa l = limites(3, 2);  // 160 x 80 en mundo
    // zoom 0.15: la vista cubre 5333 x 4000: centrado. zoom 20: vista 40 x 30 cabe: clamp.
    Vector2 r = limita({0.0f, 0.0f}, 0.15f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 48.0f) && cerca(r.y, 40.0f));
    r = limita({-100.0f, -100.0f}, 20.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, -32.0f + 20.0f) && cerca(r.y, 15.0f));
    r = limita({500.0f, 500.0f}, 20.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 128.0f - 20.0f) && cerca(r.y, 80.0f - 15.0f));
}

MOTOR_TEST_CASE(mapa_que_justo_cabe_se_centra) {
    const LimitesMapa l{0.0f, 800.0f, 0.0f, 600.0f};
    const Vector2 r = limita({123.0f, -45.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(r.x == 400.0f && r.y == 300.0f);
}

MOTOR_TEST_CASE(precision_con_coordenadas_grandes) {
    // Limites a +-3000: el error de un redondeo a float (ulp 2.4e-4 a 2048..4096) cabe en 1e-3.
    const LimitesMapa l{-3000.0f, 3000.0f, -3000.0f, 3000.0f};
    const TamanoVista v{801, 599};
    for (float zoom : {0.7f, 1.0f, 1.7f, 2.9f}) {
        const double mx = 801.0 / (2.0 * zoom), my = 599.0 / (2.0 * zoom);
        Vector2 r = limita({-3000.0f, -3000.0f}, zoom, v, l);
        MOTOR_CHECK(std::fabs(r.x - (-3000.0 + mx)) <= 1e-3);
        MOTOR_CHECK(std::fabs(r.y - (-3000.0 + my)) <= 1e-3);
        r = limita({3000.0f, 3000.0f}, zoom, v, l);
        MOTOR_CHECK(std::fabs(r.x - (3000.0 - mx)) <= 1e-3);
        MOTOR_CHECK(std::fabs(r.y - (3000.0 - my)) <= 1e-3);
        // Idempotente: lo limitado ya es valido.
        const Vector2 s = limita(r, zoom, v, l);
        MOTOR_CHECK(std::fabs(s.x - r.x) <= 1e-3 && std::fabs(s.y - r.y) <= 1e-3);
    }
}

MOTOR_TEST_CASE(entradas_invalidas_dan_error) {
    MOTOR_CHECK(!limitesDeMapa(0, 4, 64, 32).isOk());
    MOTOR_CHECK(!limitesDeMapa(4, -1, 64, 32).isOk());
    MOTOR_CHECK(!limitesDeMapa(4, 4, 0, 32).isOk());
    MOTOR_CHECK(!limitesDeMapa(4, 4, 64, 0).isOk());
    MOTOR_CHECK(!limitesDeMapa(4, 4, 64, 32, -1.0f).isOk());
    MOTOR_CHECK(!limitesDeMapa(4, 4, 64, 32, kNaN).isOk());
    const LimitesMapa l = limites(8, 8);
    const Vector2 c{0.0f, 0.0f};
    MOTOR_CHECK(!limitarAlMapa(c, 0.0f, kVista, l).isOk());
    MOTOR_CHECK(!limitarAlMapa(c, -1.0f, kVista, l).isOk());
    MOTOR_CHECK(!limitarAlMapa(c, kNaN, kVista, l).isOk());
    MOTOR_CHECK(!limitarAlMapa(c, kInf, kVista, l).isOk());
    MOTOR_CHECK(!limitarAlMapa(c, 1.0f, {0, 600}, l).isOk());
    MOTOR_CHECK(!limitarAlMapa(c, 1.0f, {800, -1}, l).isOk());
    MOTOR_CHECK(!limitarAlMapa({kNaN, 0.0f}, 1.0f, kVista, l).isOk());
    MOTOR_CHECK(!limitarAlMapa({0.0f, kInf}, 1.0f, kVista, l).isOk());
    MOTOR_CHECK(!limitarAlMapa(c, 1.0f, kVista, {10.0f, 0.0f, 0.0f, 10.0f}).isOk());
    MOTOR_CHECK(!limitarAlMapa(c, 1.0f, kVista, {0.0f, 10.0f, 10.0f, 0.0f}).isOk());
    MOTOR_CHECK(!limitarAlMapa(c, 1.0f, kVista, {kNaN, 10.0f, 0.0f, 10.0f}).isOk());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
