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

// El centro esta dentro del paralelogramo del mapa (tolerancia 1e-3): lo prueba en coordenadas de
// casilla continuas (a,b) invirtiendo gridToScreen respecto a la esquina superior del rombo.
bool centroSobreRombo(const Vector2& p, const LimitesMapa& l, double hw = 32.0, double hh = 16.0) {
    const double columnas = l.rombo.derecho.y / hh, filas = l.rombo.izquierdo.y / hh;
    const double dx = p.x - l.rombo.superior.x, dy = p.y - l.rombo.superior.y;
    const double a = (dx / hw + dy / hh) * 0.5, b = (dy / hh - dx / hw) * 0.5;
    const double e = 1e-4;
    return a >= -e && a <= columnas + e && b >= -e && b <= filas + e;
}

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

// Decision de Oriol (r1): tras la caja, el centro se lleva al punto mas cercano del rombo.
// Caja da (-1616,300), que esta fuera del rombo (|dx|/2048+|dy|/1024 = 1,51); arista izquierda-
// superior dx+2dy=-2048 desde (32,1024): proyeccion (-1406,4; 719,2).
MOTOR_TEST_CASE(caso_esquina_superior_izquierda_sube_al_rombo) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 r = limita({-2000.0f, -50.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, -1406.4f) && cerca(r.y, 719.2f));
    MOTOR_CHECK(centroSobreRombo(r, l));
}

MOTOR_TEST_CASE(caso_zoom_3_esquina_inferior_derecha) {
    const LimitesMapa l = limites(64, 64);
    // Caja: (1946,667; 1948) (fuera del rombo); arista derecha-inferior dx+2dy=2048: proyeccion.
    const Vector2 r = limita({2080.0f, 2048.0f}, 3.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 1603.733333f) && cerca(r.y, 1262.133333f));
    MOTOR_CHECK(centroSobreRombo(r, l));
}

MOTOR_TEST_CASE(centro_exacto_min_mas_mitad_es_identidad) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 c{-1616.0f, 1024.0f};  // x = min+mitad, y sobre el eje horizontal del rombo
    const Vector2 r = limita(c, 1.0f, kVista, l);
    MOTOR_CHECK(r.x == c.x && r.y == c.y);
}

MOTOR_TEST_CASE(centro_a_1e_3_del_borde_se_recorta) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 r = limita({-1616.001f, 1024.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, -1616.0f) && cerca(r.y, 1024.0f));
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
    // Caja: (400, 248) (clamp en X, Y centrado), fuera del paralelogramo fino; la arista b=1
    // (desde (32,32), direccion (2,1)) da (412,8; 222,4), con Y fuera del rango colapsado
    // (248): se acerca al centro de la caja (496,248) hasta el limite de Y: el centro.
    MOTOR_CHECK(cerca(r.x, 496.0f) && cerca(r.y, 248.0f));
    MOTOR_CHECK(centroSobreRombo(r, l));
}

MOTOR_TEST_CASE(zoom_que_cambia_vuelve_a_caber_o_no) {
    const LimitesMapa l = limites(3, 2);  // 160 x 80 en mundo
    // zoom 0.15: la vista cubre 5333 x 4000: centrado. zoom 20: vista 40 x 30 cabe: clamp.
    Vector2 r = limita({0.0f, 0.0f}, 0.15f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 48.0f) && cerca(r.y, 40.0f));
    r = limita({-100.0f, -100.0f}, 20.0f, kVista, l);
    // Caja: (-12,15) fuera del paralelogramo; arista T-L: proyeccion (-9,2; 20,6).
    MOTOR_CHECK(cerca(r.x, -9.2f) && cerca(r.y, 20.6f));
    r = limita({500.0f, 500.0f}, 20.0f, kVista, l);
    // Caja: (108,65); arista R-B: (105,2; 59,4).
    MOTOR_CHECK(cerca(r.x, 105.2f) && cerca(r.y, 59.4f));
}

MOTOR_TEST_CASE(mapa_que_justo_cabe_se_centra) {
    const LimitesMapa l{0.0f, 800.0f, 0.0f, 600.0f};
    const Vector2 r = limita({123.0f, -45.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(r.x == 400.0f && r.y == 300.0f);
}

MOTOR_TEST_CASE(vertices_del_rombo_con_gridToScreen) {
    const auto r = PoliticaCamara::romboDeMapa(3, 2, 64, 32);
    MOTOR_CHECK(r.isOk());
    const auto& v = r.value();
    MOTOR_CHECK(v.superior.x == 32.0f && v.superior.y == 0.0f);
    MOTOR_CHECK(v.derecho.x == 128.0f && v.derecho.y == 48.0f);
    MOTOR_CHECK(v.inferior.x == 64.0f && v.inferior.y == 80.0f);
    MOTOR_CHECK(v.izquierdo.x == -32.0f && v.izquierdo.y == 32.0f);
    const auto g = PoliticaCamara::romboDeMapa(64, 64, 64, 32).value();
    MOTOR_CHECK(g.superior.x == 32.0f && g.derecho.x == 2080.0f && g.derecho.y == 1024.0f);
    MOTOR_CHECK(g.inferior.x == 32.0f && g.inferior.y == 2048.0f);
    MOTOR_CHECK(g.izquierdo.x == -2016.0f && g.izquierdo.y == 1024.0f);
    MOTOR_CHECK(!PoliticaCamara::romboDeMapa(0, 2, 64, 32).isOk());
}

MOTOR_TEST_CASE(barrido_el_centro_siempre_cae_sobre_terreno) {
    for (int dim : {1, 3, 64}) {
        const LimitesMapa l = limites(dim, dim == 3 ? 2 : dim);
        for (float zoom : {0.15f, 1.0f, 1.7f, 3.0f}) {
            for (float x = -3000.0f; x <= 3000.0f; x += 250.0f) {
                for (float y = -1000.0f; y <= 3000.0f; y += 250.0f) {
                    const Vector2 r = limita({x, y}, zoom, kVista, l);
                    MOTOR_CHECK(centroSobreRombo(r, l));
                    // El tile bajo el centro existe: sus indices (floor de a,b) caen en el mapa.
                    const Vector2 d = r - l.rombo.superior;
                    const double a = (d.x / 32.0 + d.y / 16.0) * 0.5, b = (d.y / 16.0 - d.x / 32.0) * 0.5;
                    MOTOR_CHECK(std::floor(a + 1e-4) >= 0 && std::floor(b + 1e-4) >= 0);
                    // Idempotente (tolerancia 1e-3) y dentro de la caja.
                    const Vector2 s = limita(r, zoom, kVista, l);
                    MOTOR_CHECK(cerca(s.x, r.x) && cerca(s.y, r.y));
                    MOTOR_CHECK(r.x >= l.izquierda - 1e-3f && r.x <= l.derecha + 1e-3f);
                    MOTOR_CHECK(r.y >= l.arriba - 1e-3f && r.y <= l.abajo + 1e-3f);
                }
            }
        }
    }
}

MOTOR_TEST_CASE(las_cuatro_esquinas_del_mapa_se_alcanzan) {
    // Los vertices del rombo son validos como centro (zoom 20: la vista cabe holgada).
    const LimitesMapa l = limites(64, 64);
    const Vector2 vs[4] = {l.rombo.superior, l.rombo.derecho, l.rombo.inferior, l.rombo.izquierdo};
    for (const Vector2& v : vs) {
        const Vector2 r = limita(v, 20.0f, kVista, l);
        MOTOR_CHECK(centroSobreRombo(r, l));
        // Cada vertice se acerca a <= 40 px del propio vertice (la caja de zoom 20 es 20 x 15).
        MOTOR_CHECK(std::hypot(r.x - v.x, r.y - v.y) <= 25.0);
    }
}

MOTOR_TEST_CASE(centro_ya_sobre_terreno_y_dentro_de_caja_es_identidad) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 c{123.5f, 987.25f};
    const Vector2 r = limita(c, 1.0f, kVista, l);
    MOTOR_CHECK(r.x == c.x && r.y == c.y);
}

MOTOR_TEST_CASE(sin_rombo_solo_se_aplica_la_caja) {
    LimitesMapa l = limites(64, 64);
    l.usaRombo = false;
    const Vector2 r = limita({-2000.0f, -50.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, -1616.0f) && cerca(r.y, 300.0f));
}

// r2: con margen superior el centro de la CAJA (48,-8) esta fuera del rombo y el rango colapsa a
// ese punto: rango-rombo vacio, gana el rombo. Cerca de (0,0): arista T-L desde (32,0), dir (-2,1):
// t=64/5=12,8 => (6,4; 12,8).
MOTOR_TEST_CASE(r2_margen_96_mapa_3x2_gana_el_rombo) {
    const LimitesMapa l = limites(3, 2, 64, 32, 96.0f);
    const Vector2 r = limita({0.0f, 0.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 6.4f) && cerca(r.y, 12.8f));
    MOTOR_CHECK(centroSobreRombo(r, l));
    const Vector2 s = limita(r, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(s.x, r.x) && cerca(s.y, r.y));
}

// Mapa 1x1, margen 96, zoom 0,15: rango colapsado en (32,-32) fuera del rombo. Arista T-L desde
// (32,0): t=(60,85-33,55)/5=5,46 => (21,08; 5,46).
MOTOR_TEST_CASE(r2_margen_96_mapa_1x1_zoom_015) {
    const LimitesMapa l = limites(1, 1, 64, 32, 96.0f);
    const Vector2 r = limita({1.575f, -33.55f}, 0.15f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 21.08f) && cerca(r.y, 5.46f));
    MOTOR_CHECK(centroSobreRombo(r, l));
}

// 30x1, margen 96, zoom 1,5: rango x [266,667; 725,333], y [104; 296]; ancla = centroide del rombo
// (496,248) (en el rango). Caja: (643,296); rombo: arista T-R, t=303,6 => (639,2; 303,6), fuera del
// rango en Y; vuelta por el segmento: t=48/55,6 => (619,6259; 296).
MOTOR_TEST_CASE(r2_margen_96_mapa_30x1_vuelve_por_el_segmento_desde_el_ancla) {
    const LimitesMapa l = limites(30, 1, 64, 32, 96.0f);
    const Vector2 r = limita({643.0f, 296.0f}, 1.5f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 619.6259f, 2e-3f) && cerca(r.y, 296.0f));
    MOTOR_CHECK(centroSobreRombo(r, l));
    const Vector2 s = limita(r, 1.5f, kVista, l);
    MOTOR_CHECK(cerca(s.x, r.x) && cerca(s.y, r.y));
}

// Barrido con margen 0/96/200, mapas cuadrados y no cuadrados, varios zooms: centro sobre el
// rombo, idempotente, y en el rango de la caja cuando rango-rombo no es vacio (el ancla
// es el centroide del rombo o, si este no cae en el rango, se comprueba solo con el segundo).
MOTOR_TEST_CASE(r2_barrido_con_margen_sobre_rombo_e_idempotente) {
    const int mapas[5][2] = {{1, 1}, {3, 2}, {30, 1}, {8, 8}, {20, 5}};
    for (float margen : {0.0f, 96.0f, 200.0f}) {
        for (const auto& m : mapas) {
            const LimitesMapa l = limites(m[0], m[1], 64, 32, margen);
            for (float zoom : {0.15f, 0.7f, 1.5f, 3.0f}) {
                const double mx = 400.0 / zoom, my = 300.0 / zoom;
                for (float x = -1500.0f; x <= 1500.0f; x += 190.0f) {
                    for (float y = -400.0f; y <= 1200.0f; y += 130.0f) {
                        const Vector2 r = limita({x, y}, zoom, kVista, l);
                        MOTOR_CHECK(centroSobreRombo(r, l));
                        const Vector2 s = limita(r, zoom, kVista, l);
                        MOTOR_CHECK(cerca(s.x, r.x) && cerca(s.y, r.y));
                        // Si el centroide del rombo esta en el rango de la caja, el rango no
                        // es vacio con el rombo y el resultado debe estar en el rango.
                        const double cx = (l.izquierda + l.derecha) * 0.5, cy = (l.arriba + l.abajo) * 0.5;
                        const double xl = l.izquierda + mx <= l.derecha - mx ? l.izquierda + mx : cx;
                        const double xh = l.izquierda + mx <= l.derecha - mx ? l.derecha - mx : cx;
                        const double yl = l.arriba + my <= l.abajo - my ? l.arriba + my : cy;
                        const double yh = l.arriba + my <= l.abajo - my ? l.abajo - my : cy;
                        const double gx = (l.rombo.superior.x + l.rombo.derecho.x + l.rombo.inferior.x + l.rombo.izquierdo.x) / 4.0;
                        const double gy = (l.rombo.superior.y + l.rombo.derecho.y + l.rombo.inferior.y + l.rombo.izquierdo.y) / 4.0;
                        if (gx >= xl && gx <= xh && gy >= yl && gy <= yh) {
                            MOTOR_CHECK(r.x >= xl - 1e-3 && r.x <= xh + 1e-3);
                            MOTOR_CHECK(r.y >= yl - 1e-3 && r.y <= yh + 1e-3);
                        }
                    }
                }
            }
        }
    }
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
