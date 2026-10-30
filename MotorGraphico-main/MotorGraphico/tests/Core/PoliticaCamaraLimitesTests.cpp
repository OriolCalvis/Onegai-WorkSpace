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

// r4: el resultado es el punto de caja ∩ rombo mas cercano al centro (proyeccion convexa). Rango
// x>=-1616, y>=300; la arista T-L del rombo (x+2y=32) corta x=-1616 en y=824; el punto de esa
// arista mas cercano a (-2000,-50) es (-1573,6; 802,8).
MOTOR_TEST_CASE(caso_esquina_superior_izquierda_sube_al_rombo) {
    const LimitesMapa l = limites(64, 64);
    const Vector2 r = limita({-2000.0f, -50.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, -1573.6f) && cerca(r.y, 802.8f));
    MOTOR_CHECK(centroSobreRombo(r, l));
}

MOTOR_TEST_CASE(caso_zoom_3_esquina_inferior_derecha) {
    const LimitesMapa l = limites(64, 64);
    // Rango x<=1946,667, y<=1948: punto mas cercano a (2080,2048) de caja ∩ rombo, sobre la arista
    // derecha-inferior (proyeccion convexa): (1670,4; 1228,8).
    const Vector2 r = limita({2080.0f, 2048.0f}, 3.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 1670.4f) && cerca(r.y, 1228.8f));
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
    // Caja: (400, 248) (clamp en X, Y centrado), fuera del paralelogramo fino. Rango colapsado en Y
    // (segmento y=248, x en [400,592]) y rombo = x en [464,528]: el punto mas cercano a (400,248)
    // es (464,248) (proyeccion exacta sobre la interseccion; antes se volvia al ancla (496,248)).
    MOTOR_CHECK(cerca(r.x, 464.0f) && cerca(r.y, 248.0f));
    MOTOR_CHECK(centroSobreRombo(r, l));
}

MOTOR_TEST_CASE(zoom_que_cambia_vuelve_a_caber_o_no) {
    const LimitesMapa l = limites(3, 2);  // 160 x 80 en mundo
    // zoom 0.15: la vista cubre 5333 x 4000: centrado. zoom 20: vista 40 x 30 cabe: clamp.
    Vector2 r = limita({0.0f, 0.0f}, 0.15f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 48.0f) && cerca(r.y, 40.0f));
    r = limita({-100.0f, -100.0f}, 20.0f, kVista, l);
    // Rango x [-12,108], y [15,65]; la esquina (-12,22) esta en la arista T-L: (-12; 22).
    MOTOR_CHECK(cerca(r.x, -12.0f) && cerca(r.y, 22.0f));
    r = limita({500.0f, 500.0f}, 20.0f, kVista, l);
    // Esquina (108,58) del poligono caja ∩ rombo (arista R-B corta x=108 en y=58).
    MOTOR_CHECK(cerca(r.x, 108.0f) && cerca(r.y, 58.0f));
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
// ese punto: rango-rombo vacio, gana el rombo: el punto del rombo mas cercano al RANGO (48,-8), no
// al centro (continuo en la frontera en que se vacia): arista T-R, t=24/5 => (41,6; 4,8).
MOTOR_TEST_CASE(r2_margen_96_mapa_3x2_gana_el_rombo) {
    const LimitesMapa l = limites(3, 2, 64, 32, 96.0f);
    const Vector2 r = limita({0.0f, 0.0f}, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 41.6f) && cerca(r.y, 4.8f));
    MOTOR_CHECK(centroSobreRombo(r, l));
    const Vector2 s = limita(r, 1.0f, kVista, l);
    MOTOR_CHECK(cerca(s.x, r.x) && cerca(s.y, r.y));
}

// Mapa 1x1, margen 96, zoom 0,15: rango colapsado en (32,-32) fuera del rombo: el punto del rombo
// mas cercano es el vertice superior (32,0).
MOTOR_TEST_CASE(r2_margen_96_mapa_1x1_zoom_015) {
    const LimitesMapa l = limites(1, 1, 64, 32, 96.0f);
    const Vector2 r = limita({1.575f, -33.55f}, 0.15f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 32.0f) && cerca(r.y, 0.0f));
    MOTOR_CHECK(centroSobreRombo(r, l));
}

// 30x1, margen 96, zoom 1,5: rango x [266,667; 725,333], y [104; 296]. Poligono caja ∩ rombo: el
// borde y=296 corta la arista T-R (x=32+2y) en x=624; el cercano a (643,296) es (624; 296).
MOTOR_TEST_CASE(r2_margen_96_mapa_30x1_proyeccion_convexa) {
    const LimitesMapa l = limites(30, 1, 64, 32, 96.0f);
    const Vector2 r = limita({643.0f, 296.0f}, 1.5f, kVista, l);
    MOTOR_CHECK(cerca(r.x, 624.0f) && cerca(r.y, 296.0f));
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

// r4: la proyeccion convexa sobre caja ∩ rombo es continua en zoom (tween de combate) y 1-Lipschitz
// en el centro, tambien con el rango casi colapsado y al vaciarse la interseccion.
double distancia(const Vector2& a, const Vector2& b) { return std::hypot(a.x - b.x, a.y - b.y); }

void tweenFino(int c, int f, float margen, Vector2 actor) {
    const LimitesMapa l = limites(c, f, 64, 32, margen);
    Vector2 prev = limita(actor, 1.0f, kVista, l);
    for (int i = 1; i <= 7000; ++i) {
        const Vector2 r = limita(actor, 1.0f + 1e-4f * static_cast<float>(i), kVista, l);
        MOTOR_CHECK(distancia(prev, r) <= 0.5);  // un salto de decenas de px se detecta
        prev = r;
    }
}

MOTOR_TEST_CASE(r4_tween_de_zoom_1_a_1_7_es_continuo) {
    tweenFino(30, 1, 0.0f, {32.0f, 16.0f});    // casilla (0,0) de 30x1
    tweenFino(20, 4, 96.0f, {384.0f, 192.0f});  // casilla (11,0) de 20x4 con margen 96
    tweenFino(30, 1, 0.0f, {464.0f, 248.0f});
}

MOTOR_TEST_CASE(r4_continuidad_en_zoom_al_vaciarse_la_interseccion) {
    struct Caso { int c, f; float m; Vector2 centro; float z0, z1; };
    const Caso casos[] = {
        {40, 2, 96.0f, {1387.0f, 672.3f}, 0.62499f, 0.62501f},  // antes 787 px
        {30, 1, 96.0f, {1074.0f, 396.0f}, 0.92591f, 0.92595f},  // antes 626 px
        {3, 2, 96.0f, {177.7f, -9.0f}, 4.1665f, 4.1667f},       // antes 89 px
        {1, 1, 96.0f, {155.5f, 55.2f}, 9.3743f, 9.3748f},       // antes 36 px
        {40, 2, 0.0f, {1108.3f, 305.0f}, 0.89285f, 0.8929f},
    };
    for (const Caso& k : casos) {
        const LimitesMapa l = limites(k.c, k.f, 64, 32, k.m);
        MOTOR_CHECK(distancia(limita(k.centro, k.z0, kVista, l), limita(k.centro, k.z1, kVista, l)) <= 0.5);
    }
}

MOTOR_TEST_CASE(r4_proyeccion_es_1_lipschitz_con_rango_ancho) {
    const LimitesMapa l = limites(40, 2, 64, 32, 96.0f);
    const Vector2 a{716.02f, 326.13f}, b{715.77f, 326.07f};
    MOTOR_CHECK(distancia(limita(a, 0.9f, kVista, l), limita(b, 0.9f, kVista, l)) <=
                distancia(a, b) + 1e-3);  // antes: entrada 0,26 -> salida 2,5
    for (int i = 0; i < 200; ++i) {
        const Vector2 p{600.0f + 0.7f * static_cast<float>(i), 326.0f}, q{p.x + 0.05f, p.y + 0.01f};
        MOTOR_CHECK(distancia(limita(p, 0.9f, kVista, l), limita(q, 0.9f, kVista, l)) <=
                    distancia(p, q) + 1e-3);
    }
}

}  // namespace

int main() { return motor_test::runAllTests(); }
