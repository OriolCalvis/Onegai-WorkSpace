// U4-01.E07 — Verificacion de la camara del juego: zona muerta, esquinas, mapas pequenos, sacudida critica en
// esquina y cambio de mapa durante una sacudida. EJECUTA fotogramaCamara + Camera reales (viewport 800x600, tiles
// 64x32, zoom 1 y 1,7 de combate). GL-free. Los casos marcados CONOCIDO: describen el comportamiento ACTUAL
// (preexistente) y no son un arreglo: si cambian, la prueba debe revisarse a proposito. La aprobacion visual de
// Oriol (D-O5) NO se prueba aqui: ver out/fabrica/REPLICA-R1/U4-01.E07_verificacion.md (raiz del worktree).
#include "Engine/ApplicationCamara.h"

#include "TestHarness.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>

#include "Core/Math/IsoMath.h"

namespace {

using PoliticaCamara::LimitesMapa;
using PoliticaCamara::RomboMapa;

constexpr float kDt = 1.0f / 60.0f;
constexpr float kTol = 1e-2f;  // px de mundo; las cotas del rombo/caja son de 1e-3 (politica) + float

bool iguales(const Vector2& a, const Vector2& b) { return a.x == b.x && a.y == b.y; }
bool cerca(float a, float b, float tol = kTol) { return std::fabs(a - b) <= tol; }

void ponEntorno(const char* valor) {  // valor == nullptr: la variable queda sin definir
#ifdef _WIN32
    _putenv_s("MOTOR_MOVIMIENTO_REDUCIDO", valor ? valor : "");
#else
    if (valor) setenv("MOTOR_MOVIMIENTO_REDUCIDO", valor, 1); else unsetenv("MOTOR_MOVIMIENTO_REDUCIDO");
#endif
}
// Guarda el valor que tenia MOTOR_MOVIMIENTO_REDUCIDO al arrancar el proceso y lo restaura al salir.
struct GuardaEntorno {
    bool habia = false;
    std::string previo;
    GuardaEntorno() {
        if (const char* v = std::getenv("MOTOR_MOVIMIENTO_REDUCIDO")) { habia = true; previo = v; }
    }
    ~GuardaEntorno() { ponEntorno(habia ? previo.c_str() : nullptr); }
} guardaEntorno;
void ponReducido(bool reducido) { ponEntorno(reducido ? "1" : nullptr); }

LimitesMapa limitesDe(int c, int f) {
    LimitesMapa l;
    MOTOR_CHECK(ApplicationCamara::limitesDelMapa(c, f, 64, 32, l));
    return l;
}

// Centro del rombo de una casilla (pixel de mundo del actor, como lo calcula el bucle sin desfase de altura).
Vector2 centroCelda(const GridCoord& g) {
    const Vector2 o = IsoMath::gridToScreen(g, 64, 32);
    return Vector2{o.x + 32.0f, o.y + 16.0f};
}

// true si p cae dentro del paralelogramo del mapa (todas las aristas con p a la izquierda, con tolerancia en px).
bool dentroRombo(const Vector2& p, const RomboMapa& r, float tol) {
    const Vector2 v[4] = {r.superior, r.derecho, r.inferior, r.izquierdo};
    for (int i = 0; i < 4; ++i) {
        const Vector2 a = v[i], b = v[(i + 1) % 4];
        const double ex = b.x - a.x, ey = b.y - a.y;
        const double cruz = ex * (p.y - a.y) - ey * (p.x - a.x);
        if (cruz / std::sqrt(ex * ex + ey * ey) < -tol) return false;
    }
    return true;
}

// Escena de juego: Camera + estado + limites reales; cada paso() es un fotograma de Application::update.
struct Escena {
    Camera cam{800, 600};
    EstadoCamaraJuego e;
    LimitesMapa lim;
    int columnas = 0, filas = 0;
    GridCoord ultima{-9999, -9999};  // centinela: mapa recien cargado
    float excesoCaja = 0.0f;         // mayor salida de la vista respecto a LimitesMapa (ejes que caben)
    float fueraRombo = 0.0f;         // indicador: 1 si en algun fotograma el centro de la vista cayo fuera del rombo, 0 si nunca
    int fotogramas = 0;

    Escena(int c, int f, float zoom) : lim(limitesDe(c, f)), columnas(c), filas(f) {
        cam.transitionTo(Vector2{0.0f, 0.0f}, zoom, 0.0f, Camera::Easing::Linear);
        cam.update(kDt);
    }
    void paso(const GridCoord& celda, const Vector2& objetivo, float dt = kDt, bool vigila = true) {
        ApplicationCamara::EntradaFotograma in;
        in.dt = dt;
        in.celda = celda;
        in.ultimaCelda = ultima;
        in.objetivo = objetivo;
        in.limites = &lim;
        ultima = ApplicationCamara::fotogramaCamara(cam, e, in, ApplicationCamara::PasosFotograma{});
        ++fotogramas;
        if (vigila) mide();
    }
    void llega(const GridCoord& celda, int n = 12) {  // casilla nueva + los fotogramas que dura el tween (0,16 s)
        for (int i = 0; i < n; ++i) paso(celda, centroCelda(celda));
    }
    // Mide la vista del fotograma actual contra la caja y el rombo (centro sobre terreno).
    void mide() {
        const Vector2 v = cam.posicionVista();
        const float mx = 400.0f / cam.zoom(), my = 300.0f / cam.zoom();
        auto exceso = [&](float centro, float mitad, float min, float max) {
            if (max - min <= 2.0f * mitad) return std::fabs(centro - 0.5f * (min + max));  // eje que no cabe: centrado
            return std::max(0.0f, std::max(min + mitad - centro, centro - (max - mitad)));
        };
        excesoCaja = std::max({excesoCaja, exceso(v.x, mx, lim.izquierda, lim.derecha), exceso(v.y, my, lim.arriba, lim.abajo)});
        const auto& r = lim.rombo;
        if (!dentroRombo(v, r, kTol)) fueraRombo = std::max(fueraRombo, 1.0f);
    }
};

// ---- Zona muerta -------------------------------------------------------------------------------------------------

MOTOR_TEST_CASE(sacudida_del_producto_es_4_px_de_mundo_con_0_14_y_0_24_s) {  // D-O2 (valores fijados en E06, aqui se vigilan)
    MOTOR_CHECK(ApplicationCamara::kSacudidaMundoPx == 4.0f);
    MOTOR_CHECK(ApplicationCamara::kSacudidaDanio == 0.14f && ApplicationCamara::kSacudidaCritico == 0.24f);
}

MOTOR_TEST_CASE(sacudida_critica_decrece_con_la_envolvente_y_acaba_a_los_0_24_s) {
    ponReducido(false);
    float temprano = 0.0f, tardio = 0.0f;
    for (std::uint32_t semilla = 1; semilla <= 8; ++semilla) {
        Escena s(40, 40, 1.0f);
        s.llega(GridCoord{20, 20}, 30);
        const Vector2 base = s.cam.posicionVista();
        ApplicationCamara::sacude(s.e, 4.0f, 0.24f, semilla);
        for (int k = 1; k <= 18; ++k) {  // fotograma k: reloj = k/60 s
            s.paso(GridCoord{20, 20}, centroCelda(GridCoord{20, 20}));
            const float dx = s.cam.posicionVista().x - base.x, dy = s.cam.posicionVista().y - base.y;
            const double t = k / 60.0;
            const double env = t >= 0.24 ? 0.0 : (1.0 - t / 0.24) * (1.0 - t / 0.24);
            MOTOR_CHECK(std::fabs(dx) <= 4.0 * env + 1e-4 && std::fabs(dy) <= 4.0 * 0.6 * env + 1e-4);  // cota (1-t/d)^2 * 4 px
            if (k <= 4) temprano += std::fabs(dx) + std::fabs(dy);
            if (k >= 11 && k <= 14) tardio += std::fabs(dx) + std::fabs(dy);
            if (k >= 15) MOTOR_CHECK(dx == 0.0f && dy == 0.0f);  // 0,25 s > 0,24 s: cero exacto
        }
    }
    std::cout << "  [medida] caida: suma |d| fotogramas 1-4 = " << temprano << ", fotogramas 11-14 = " << tardio << " px\n";
    MOTOR_CHECK(temprano > 4.0f * tardio);  // decrece de verdad (no solo termina en 0)
}

MOTOR_TEST_CASE(zona_muerta_del_producto_es_96_por_48_px_de_pantalla) {  // D-O1 (valor fijado en E06, aqui se vigila)
    MOTOR_CHECK(ApplicationCamara::kZonaMuerta.semiAnchoPx == 96.0f && ApplicationCamara::kZonaMuerta.semiAltoPx == 48.0f);
}

MOTOR_TEST_CASE(actor_en_el_centro_o_en_la_zona_no_mueve_la_camara_en_120_fotogramas) {
    for (const float zoom : {1.0f, 1.7f}) {
        Escena s(40, 40, zoom);
        const GridCoord a{20, 20};
        s.llega(a, 30);  // sin destino previo: centra y se asienta
        const Vector2 vista0 = s.cam.posicionVista(), destino0 = s.e.destino;
        MOTOR_CHECK(cerca(vista0.x, centroCelda(a).x) && cerca(vista0.y, centroCelda(a).y));
        // Casillas vecinas cuyo centro cae dentro de +-96x48 px de pantalla (1 tile: +-32x16 de mundo): alterna A/B cada
        // fotograma, de modo que el seguimiento corre 120 veces y NUNCA debe moverse.
        const GridCoord b{21, 20}, c{20, 21};
        for (int i = 0; i < 120; ++i) {
            const GridCoord g = (i % 2 == 0) ? b : c;
            s.paso(g, centroCelda(g));
            MOTOR_CHECK(iguales(s.cam.posicionVista(), vista0) && iguales(s.e.destino, destino0) && !s.cam.isTransitioning());
        }
    }
}

MOTOR_TEST_CASE(actor_en_cada_borde_de_la_zona_no_vibra_en_120_fotogramas_y_pasado_el_borde_se_mueve_una_sola_vez) {
    struct Borde { float dx, dy; };
    for (const float zoom : {1.0f, 1.7f}) {
        for (const Borde b : {Borde{96.0f, 0.0f}, Borde{-96.0f, 0.0f}, Borde{0.0f, 48.0f}, Borde{0.0f, -48.0f}}) {
            for (const float extra : {0.0f, -0.5f, 0.5f}) {  // exactamente en el borde / 0,5 px de pantalla dentro / fuera
                Escena s(40, 40, zoom);
                const Vector2 c = centroCelda(GridCoord{20, 20});
                s.cam.transitionTo(c, zoom, 0.0f, Camera::Easing::Linear);
                s.cam.update(kDt);
                s.e.destino = c;
                s.e.conDestino = true;
                s.ultima = GridCoord{20, 20};
                const float ex = b.dx == 0.0f ? 0.0f : (std::fabs(b.dx) + extra) * (b.dx < 0 ? -1.0f : 1.0f) / zoom;
                const float ey = b.dy == 0.0f ? 0.0f : (std::fabs(b.dy) + extra) * (b.dy < 0 ? -1.0f : 1.0f) / zoom;
                const Vector2 objetivo{c.x + ex, c.y + ey};
                int cambios = 0;
                Vector2 previo = s.e.destino;
                for (int i = 0; i < 120; ++i) {
                    const GridCoord g = (i % 2 == 0) ? GridCoord{21, 20} : GridCoord{20, 21};  // fuerza el seguimiento cada fotograma
                    s.paso(g, objetivo);
                    if (!iguales(s.e.destino, previo)) ++cambios;
                    previo = s.e.destino;
                }
                if (extra <= 0.0f) {
                    MOTOR_CHECK(cambios == 0);  // en el borde (o dentro): ningun movimiento, ninguna transicion relanzada
                    MOTOR_CHECK(iguales(s.cam.posicionVista(), c) && !s.cam.isTransitioning());
                } else {
                    MOTOR_CHECK(cambios == 1);  // fuera: empuja una vez, deja al actor sobre el borde y se queda quieta
                    MOTOR_CHECK(!s.cam.isTransitioning());
                    // La vista queda con el actor SOBRE el borde de la zona, en cada eje (el eje que no sale no se mueve).
                    const float esperadoX = c.x + (b.dx == 0.0f ? 0.0f : ex - (b.dx < 0 ? -1.0f : 1.0f) * 96.0f / zoom);
                    const float esperadoY = c.y + (b.dy == 0.0f ? 0.0f : ey - (b.dy < 0 ? -1.0f : 1.0f) * 48.0f / zoom);
                    MOTOR_CHECK(cerca(s.cam.posicionVista().x, esperadoX, 1e-3f));
                    MOTOR_CHECK(cerca(s.cam.posicionVista().y, esperadoY, 1e-3f));
                }
            }
        }
    }
}

MOTOR_TEST_CASE(actor_a_5e_5_px_fuera_del_borde_cuenta_como_borde_tolerancia_de_frontera) {
    // |dif| <= semi/zoom + 1e-4 se devuelve identico. 5e-5 px de mundo (zoom 1; en X, donde el float aun lo representa).
    Escena s(40, 40, 1.0f);
    const Vector2 c = centroCelda(GridCoord{20, 20});
    s.cam.transitionTo(c, 1.0f, 0.0f, Camera::Easing::Linear);
    s.cam.update(kDt);
    s.e.destino = c;
    s.e.conDestino = true;
    s.ultima = GridCoord{20, 20};
    const Vector2 objetivo{c.x + 96.0f + 5e-5f, c.y};
    MOTOR_CHECK(objetivo.x > c.x + 96.0f);  // de verdad fuera del borde
    for (int i = 0; i < 120; ++i) s.paso((i % 2 == 0) ? GridCoord{21, 20} : GridCoord{20, 21}, objetivo);
    MOTOR_CHECK(iguales(s.e.destino, c) && iguales(s.cam.posicionVista(), c) && !s.cam.isTransitioning());
}

// ---- Esquinas y bordes del mapa ----------------------------------------------------------------------------------

// Recorre el contorno COMPLETO de las casillas (cada casilla del borde, una a una) y comprueba cada fotograma.
void recorreBorde(Escena& s) {
    const int c = s.columnas, f = s.filas;
    std::vector<GridCoord> ruta;
    for (int x = 0; x < c; ++x) ruta.push_back({x, 0});
    for (int y = 1; y < f; ++y) ruta.push_back({c - 1, y});
    for (int x = c - 2; x >= 0; --x) ruta.push_back({x, f - 1});
    for (int y = f - 2; y > 0; --y) ruta.push_back({0, y});
    for (const GridCoord& g : ruta) s.llega(g, 11);
    for (const GridCoord g : {GridCoord{0, 0}, GridCoord{c - 1, 0}, GridCoord{c - 1, f - 1}, GridCoord{0, f - 1}})  // teletransporte a esquinas
        s.llega(g, 12);
}

MOTOR_TEST_CASE(borde_completo_de_mapas_grandes_nunca_excede_limites_y_el_centro_esta_sobre_terreno) {
    struct Caso { int c, f; };
    for (const Caso k : {Caso{20, 20}, Caso{30, 12}, Caso{12, 30}, Caso{40, 40}, Caso{64, 64}, Caso{100, 100}, Caso{100, 40}}) {
        for (const float zoom : {1.0f, 1.7f}) {
            Escena s(k.c, k.f, zoom);
            recorreBorde(s);
            std::cout << "  [medida] borde " << k.c << "x" << k.f << " zoom " << zoom << ": " << s.fotogramas
                      << " fotogramas, exceso de caja max " << s.excesoCaja << " px, centro fuera de rombo "
                      << (s.fueraRombo > 0.0f ? "SI" : "no") << "\n";
            MOTOR_CHECK(s.fueraRombo == 0.0f);   // D-O3 (Oriol): el centro de la pantalla SIEMPRE sobre terreno
            MOTOR_CHECK(s.excesoCaja <= kTol);   // y la vista dentro de LimitesMapa (eje que no cabe: centrado)
        }
    }
}

MOTOR_TEST_CASE(borde_completo_de_mapas_medianos_y_estrechos_a_zoom_de_combate) {
    struct Caso { int c, f; };
    for (const Caso k : {Caso{8, 6}, Caso{6, 8}, Caso{12, 3}, Caso{3, 12}, Caso{10, 10}}) {  // algunos no caben en un eje
        for (const float zoom : {1.0f, 1.7f}) {
            Escena s(k.c, k.f, zoom);
            recorreBorde(s);
            std::cout << "  [medida] borde " << k.c << "x" << k.f << " zoom " << zoom << ": exceso de caja max "
                      << s.excesoCaja << " px, centro fuera de rombo " << (s.fueraRombo > 0.0f ? "SI" : "no") << "\n";
            MOTOR_CHECK(s.fueraRombo == 0.0f);
            MOTOR_CHECK(s.excesoCaja <= kTol);
        }
    }
}

MOTOR_TEST_CASE(mapas_pequenos_1x1_y_3x2_quedan_centrados_desde_cualquier_casilla) {  // MapFramingTests.cpp:69 y :110
    struct Caso { int c, f; float cx, cy; };
    for (const Caso k : {Caso{1, 1, 32.0f, 16.0f}, Caso{3, 2, 48.0f, 40.0f}}) {
        for (const float zoom : {1.0f, 1.7f}) {
            for (int y = 0; y < k.f; ++y) {
                for (int x = 0; x < k.c; ++x) {
                    Escena s(k.c, k.f, zoom);
                    s.llega(GridCoord{x, y}, 30);
                    const Vector2 v = s.cam.posicionVista();
                    MOTOR_CHECK(cerca(v.x, k.cx) && cerca(v.y, k.cy));  // el mapa no cabe en la vista: queda centrado
                    MOTOR_CHECK(s.fueraRombo == 0.0f);
                }
            }
        }
    }
}

// ---- Golpe critico en esquina ------------------------------------------------------------------------------------

struct Medida { float desplazamientoMax; int fotogramasConDesplazamiento; bool dentroSiempre; };

// Pone al actor en una esquina, dispara un critico (0,24 s; amplitud = 4 px de mundo * zoom, como ApplicationPantallas)
// y mide durante 20 fotogramas cuanto se separa la vista de la base y si sale de limites/rombo.
Medida criticoEnEsquina(const GridCoord& esquina, int c, int f, float zoom, std::uint32_t semilla,
                        const Vector2& desfase = Vector2{32.0f, 16.0f}) {  // desfase dentro del rombo de la casilla (defecto: su centro)
    Escena s(c, f, zoom);
    const Vector2 o = IsoMath::gridToScreen(esquina, 64, 32);
    const Vector2 objetivo{o.x + desfase.x, o.y + desfase.y};
    for (int i = 0; i < 30; ++i) s.paso(esquina, objetivo);
    const Vector2 base = s.cam.posicionVista();
    s.excesoCaja = 0.0f;
    s.fueraRombo = 0.0f;
    ApplicationCamara::sacude(s.e, 4.0f * zoom, 0.24f, semilla);  // literales: 4 px de mundo y 0,24 s (D-O2); las constantes se vigilan aparte
    Medida m{0.0f, 0, true};
    for (int i = 0; i < 20; ++i) {
        s.paso(esquina, objetivo);  // sin cambio de casilla: el seguimiento no corre
        const Vector2 v = s.cam.posicionVista();
        const float d = std::hypot(v.x - base.x, v.y - base.y);
        if (d > 0.0f) ++m.fotogramasConDesplazamiento;
        m.desplazamientoMax = std::max(m.desplazamientoMax, d);
    }
    m.dentroSiempre = s.fueraRombo == 0.0f && s.excesoCaja <= kTol;
    return m;
}

MOTOR_TEST_CASE(critico_en_las_cuatro_esquinas_no_saca_la_vista_del_mapa) {
    ponReducido(false);
    const int c = 20, f = 20;
    for (const float zoom : {1.0f, 1.7f}) {
        for (const GridCoord g : {GridCoord{0, 0}, GridCoord{c - 1, 0}, GridCoord{c - 1, f - 1}, GridCoord{0, f - 1}}) {
            const Medida m = criticoEnEsquina(g, c, f, zoom, 7);
            std::cout << "  [medida] critico esquina (" << g.x << "," << g.y << ") zoom " << zoom << ": desplazamiento max "
                      << m.desplazamientoMax << " px de mundo en " << m.fotogramasConDesplazamiento << "/20 fotogramas\n";
            MOTOR_CHECK(m.dentroSiempre);  // D-T6: la sacudida se recorta a los limites (en la esquina el temblor es unilateral)
        }
    }
    // En el centro del mapa la misma sacudida SI se ve (sin recorte): referencia para que la esquina no sea un cero trivial.
    Escena s(40, 40, 1.0f);
    s.llega(GridCoord{20, 20}, 30);
    const Vector2 base = s.cam.posicionVista();
    ApplicationCamara::sacude(s.e, 4.0f, 0.24f, 7);
    float maxCentro = 0.0f;
    for (int i = 0; i < 20; ++i) {
        s.paso(GridCoord{20, 20}, centroCelda(GridCoord{20, 20}));
        maxCentro = std::max(maxCentro, std::hypot(s.cam.posicionVista().x - base.x, s.cam.posicionVista().y - base.y));
    }
    std::cout << "  [medida] critico en el centro (referencia) zoom 1: desplazamiento max " << maxCentro << " px\n";
    MOTOR_CHECK(maxCentro > 0.5f);
    MOTOR_CHECK(iguales(s.cam.posicionVista(), base));  // la sacudida termina en cero exacto
}

MOTOR_TEST_CASE(critico_en_el_borde_diagonal_de_un_mapa_grande_no_saca_el_centro_del_rombo) {
    // Mapa 100x100: el rectangulo de la caja asoma fuera del rombo, asi que aqui la base YA esta sobre la arista y
    // la sacudida hacia fuera solo la frena la proyeccion sobre el rombo (D-O3). Puntos medios de las cuatro aristas.
    ponReducido(false);
    const int c = 100, f = 100;
    int conDesplazamiento = 0, total = 0;
    for (const float zoom : {1.0f, 1.7f}) {
        // Casilla y vertice de su rombo que cae SOBRE la arista (NO, NE, SE, SO): el actor esta justo en el borde del terreno.
        struct Arista { GridCoord g; Vector2 desfase; };
        for (const Arista a : {Arista{{0, 50}, {0.0f, 16.0f}}, Arista{{50, 0}, {32.0f, 0.0f}}, Arista{{c - 1, 50}, {64.0f, 16.0f}},
                               Arista{{50, f - 1}, {32.0f, 32.0f}}}) {
            for (std::uint32_t semilla = 1; semilla <= 8; ++semilla) {
                const Medida m = criticoEnEsquina(a.g, c, f, zoom, semilla, a.desfase);
                MOTOR_CHECK(m.dentroSiempre);
                ++total;
                if (m.desplazamientoMax > 0.0f) ++conDesplazamiento;
            }
        }
    }
    std::cout << "  [medida] critico en aristas 100x100: " << conDesplazamiento << "/" << total << " sacudidas con desplazamiento, siempre sobre terreno\n";
    MOTOR_CHECK(conDesplazamiento > 0);
}

MOTOR_TEST_CASE(critico_en_esquina_con_movimiento_reducido_no_mueve_la_vista_ni_un_pixel) {
    const int c = 20, f = 20;
    for (const float zoom : {1.0f, 1.7f}) {
        for (const GridCoord g : {GridCoord{0, 0}, GridCoord{c - 1, 0}, GridCoord{c - 1, f - 1}, GridCoord{0, f - 1}}) {
            ponReducido(true);
            const Medida m = criticoEnEsquina(g, c, f, zoom, 7);
            ponReducido(false);
            MOTOR_CHECK(m.desplazamientoMax == 0.0f && m.fotogramasConDesplazamiento == 0 && m.dentroSiempre);  // D-O4
        }
    }
    ponReducido(false);
}

// ---- Cambio de mapa durante una sacudida -------------------------------------------------------------------------

MOTOR_TEST_CASE(cambio_de_mapa_durante_una_sacudida_reinicia_y_aplica_los_limites_nuevos_en_el_mismo_fotograma) {
    ponReducido(false);
    Escena s(40, 40, 1.0f);
    s.llega(GridCoord{35, 35}, 30);  // lejos, en el mapa grande
    ApplicationCamara::sacude(s.e, 4.0f, 0.24f, 3);
    for (int i = 0; i < 4; ++i) s.paso(GridCoord{35, 35}, centroCelda(GridCoord{35, 35}));
    MOTOR_CHECK(s.e.amplitudPx > 0.0f && s.e.reloj > 0.0);  // sacudida viva
    // loadLevel: mapa 3x2 nuevo, centinela, celda de aparicion (1,1). Un solo fotograma.
    s.lim = limitesDe(3, 2);
    s.columnas = 3;
    s.filas = 2;
    s.ultima = GridCoord{-9999, -9999};
    s.paso(GridCoord{1, 1}, centroCelda(GridCoord{1, 1}));
    MOTOR_CHECK(s.e.amplitudPx == 0.0f && s.e.duracion == 0.0f && s.e.reloj == 0.0);  // sacudida muerta
    MOTOR_CHECK(s.e.conDestino && s.ultima.x == 1 && s.ultima.y == 1);  // y el seguimiento ya corrio con el mapa nuevo
    // La posicion logica aun viaja desde el mapa viejo (tween 0,16 s), pero la VISTA ya esta recortada al mapa nuevo.
    const auto esperada = PoliticaCamara::limitarAlMapa(s.cam.position(), s.cam.zoom(), {800, 600}, s.lim);
    MOTOR_CHECK(esperada.isOk() && iguales(s.cam.posicionVista(), esperada.value()));
    MOTOR_CHECK(dentroRombo(s.cam.posicionVista(), s.lim.rombo, kTol));
    s.excesoCaja = 0.0f;
    for (int i = 0; i < 20; ++i) s.paso(GridCoord{1, 1}, centroCelda(GridCoord{1, 1}));
    MOTOR_CHECK(cerca(s.cam.posicionVista().x, 48.0f) && cerca(s.cam.posicionVista().y, 40.0f));  // 3x2 centrado
    MOTOR_CHECK(s.excesoCaja <= kTol && s.fueraRombo == 0.0f);
}

// ---- CASOS CONOCIDOS (caracterizacion, preexistentes; NO se arreglan en E07) --------------------------------------

MOTOR_TEST_CASE(conocido_salida_de_combate_y_cambio_de_casilla_deja_el_zoom_intermedio) {
    ponReducido(false);
    Escena s(40, 40, 1.0f);
    const GridCoord a{20, 20};
    s.llega(a, 30);
    int combate = 0;  // identidad del combate (cualquier direccion)
    auto fotograma = [&](const GridCoord& celda, const void* idCombate, bool enBatalla, const Vector2& obj,
                         const ApplicationCamara::PasosFotograma& pasos) {
        ApplicationCamara::EntradaFotograma in;
        in.dt = kDt; in.celda = celda; in.ultimaCelda = s.ultima; in.objetivo = obj;
        in.combate = idCombate; in.enBatalla = enBatalla; in.limites = &s.lim;
        s.ultima = ApplicationCamara::fotogramaCamara(s.cam, s.e, in, pasos);
    };
    ApplicationCamara::PasosFotograma entra;  // ApplicationCinematica: transitionTo(centroCombate, zoomCombate, 0.5s)
    entra.combate = [&] { s.cam.transitionTo(centroCelda(a), 1.7f, 0.5f, Camera::Easing::EaseInOutQuad); };
    fotograma(a, &combate, true, centroCelda(a), entra);
    for (int i = 0; i < 40; ++i) fotograma(a, &combate, true, centroCelda(a), ApplicationCamara::PasosFotograma{});
    MOTOR_CHECK(cerca(s.cam.zoom(), 1.7f));
    // Salida: ApplicationCinematica:342 lanza transitionTo(aqui, zoomAntes, 0.6s). A los 10 fotogramas el jugador cambia de casilla.
    ApplicationCamara::PasosFotograma sale;
    sale.combate = [&] { s.cam.transitionTo(centroCelda(a), 1.0f, 0.6f, Camera::Easing::EaseInOutQuad); };
    fotograma(a, nullptr, false, centroCelda(a), sale);
    for (int i = 0; i < 9; ++i) fotograma(a, nullptr, false, centroCelda(a), ApplicationCamara::PasosFotograma{});
    const float zoomAlCambiar = s.cam.zoom();
    MOTOR_CHECK(zoomAlCambiar > 1.05f && zoomAlCambiar < 1.65f);  // en pleno tween de salida
    const GridCoord b{22, 20};  // fuera de la zona muerta: sigueJugador relanza el tween con cam.zoom() intermedio
    for (int i = 0; i < 90; ++i) fotograma(b, nullptr, false, centroCelda(b), ApplicationCamara::PasosFotograma{});
    std::cout << "  [medida] CONOCIDO zoom al cambiar de casilla " << zoomAlCambiar << ", zoom final tras 1,5 s " << s.cam.zoom() << "\n";
    // CONOCIDO: el zoom NO vuelve a 1,0 (queda en el valor intermedio de aquel instante). Si esto pasa a 1,0 alguien lo arreglo: borrar el caso.
    MOTOR_CHECK(!s.cam.isTransitioning() && cerca(s.cam.zoom(), zoomAlCambiar, 1e-3f) && s.cam.zoom() > 1.05f);
}

// Caracteriza el MECANISMO (misma identidad de combate => fotogramaCamara no reinicia), no el defecto: este esta en quien llama
// (ApplicationBucle.cpp:180 pasa battle(); GameSessionInteraccion.cpp:300/440 hace reset + make_unique y puede repetir la direccion).
// Por eso la prueba NO se pondra roja cuando se arregle a quien llama (p. ej. contador de generacion): se puede conservar tal cual.
MOTOR_TEST_CASE(conocido_oleadas_con_el_mismo_puntero_de_combate_no_reinician_la_camara) {
    ponReducido(false);
    Escena s(40, 40, 1.0f);
    const GridCoord a{20, 20};
    s.llega(a, 30);
    int oleada1 = 0;
    auto fotograma = [&](const void* idCombate) {
        ApplicationCamara::EntradaFotograma in;
        in.dt = kDt; in.celda = a; in.ultimaCelda = s.ultima; in.objetivo = centroCelda(a);
        in.combate = idCombate; in.enBatalla = true; in.limites = &s.lim;
        s.ultima = ApplicationCamara::fotogramaCamara(s.cam, s.e, in, ApplicationCamara::PasosFotograma{});
    };
    fotograma(&oleada1);  // entra en combate: reinicia y adopta la identidad
    MOTOR_CHECK(s.e.combate == &oleada1);
    ApplicationCamara::sacude(s.e, 7.0f, 0.24f, 11);
    s.e.destino = Vector2{123.0f, 456.0f};  // destino de la oleada 1
    s.e.conDestino = true;
    fotograma(&oleada1);
    MOTOR_CHECK(s.e.amplitudPx > 0.0f && s.e.conDestino);
    // m_battle.reset() + make_unique<BattleState> puede devolver la MISMA direccion: oleada 2 = mismo puntero.
    fotograma(&oleada1);
    // CONOCIDO: sin pasar por exploracion no hay "cambio de combate": la sacudida y el destino de la oleada 1 siguen vivos.
    MOTOR_CHECK(s.e.amplitudPx > 0.0f && s.e.conDestino && iguales(s.e.destino, Vector2{123.0f, 456.0f}));
    // Contraste: pasando por exploracion (nullptr) SI reinicia (ya cubierto en ApplicationCamaraTests).
    fotograma(nullptr);
    MOTOR_CHECK(s.e.amplitudPx == 0.0f && !s.e.conDestino);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
