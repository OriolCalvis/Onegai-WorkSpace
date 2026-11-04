// U4-01.E06 — Adaptador ApplicationCamara (GL-free, sin Application): reinicio por centinela, orden de
// fotograma EJECUTADO con dobles (orden, vista de cada consumidor, pausa, movimiento reducido, combate), golpes
// alineados con la pose, titulo sin limites. Solo la lectura de posicionVista() por HUD/niebla/luces es textual.
#include "Engine/ApplicationCamara.h"

#include "TestHarness.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "HUD/Pergamino/GolpesEnPantalla.h"

namespace {

using PoliticaCamara::LimitesMapa;

bool cerca(float a, float b, float tol = 1e-3f) { return std::fabs(a - b) <= tol; }
bool iguales(const Vector2& a, const Vector2& b) { return a.x == b.x && a.y == b.y; }

LimitesMapa limites(int c, int f) {
    LimitesMapa l;
    MOTOR_CHECK(ApplicationCamara::limitesDelMapa(c, f, 64, 32, l));
    return l;
}

void colocaEn(Camera& cam, const Vector2& p, float zoom) {  // corte instantaneo en el siguiente update
    cam.transitionTo(p, zoom, 0.0f, Camera::Easing::Linear);
    cam.update(1.0f / 60.0f);
}

Vector2 aPantalla(const Vector2& mundo, const Vector2& vista, float zoom, int w, int h) {  // la de golpes, HUD, niebla, luces
    return {(mundo.x - vista.x) * zoom + static_cast<float>(w) * 0.5f, (mundo.y - vista.y) * zoom + static_cast<float>(h) * 0.5f};
}

Vector2 porMatriz(const Camera& cam, const Vector2& mundo) {
    const glm::vec4 n = cam.getViewProjectionMatrix() * glm::vec4(mundo.x, mundo.y, 0.0f, 1.0f);
    return {(n.x + 1.0f) * 0.5f * static_cast<float>(cam.viewportWidth()), (1.0f - n.y) * 0.5f * static_cast<float>(cam.viewportHeight())};
}

std::string leeFuente(const std::string& rel) {
    std::ifstream f(std::string(MOTOR_FUENTES_DIR) + "/" + rel);
    MOTOR_CHECK(f.good());
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
bool contiene(const std::string& s, const std::string& t) { return s.find(t) != std::string::npos; }
std::string sinComentarios(const std::string& s) {  // quita // y /* */ (un texto en un comentario no cuenta)
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s.compare(i, 2, "//") == 0) { while (i < s.size() && s[i] != '\n') ++i; o += '\n'; }
        else if (s.compare(i, 2, "/*") == 0) { const auto f = s.find("*/", i + 2); i = f == std::string::npos ? s.size() : f + 1; }
        else o += s[i];
    }
    return o;
}
void ponReducido(bool reducido) {
#ifdef _WIN32
    _putenv_s("MOTOR_MOVIMIENTO_REDUCIDO", reducido ? "1" : "");
#else
    if (reducido) setenv("MOTOR_MOVIMIENTO_REDUCIDO", "1", 1); else unsetenv("MOTOR_MOVIMIENTO_REDUCIDO");
#endif
}

// Doble del fotograma: registra el orden y la vista que ve cada consumidor.
struct Registro {
    std::vector<std::string> orden;
    Vector2 vistaCombate, vistaGolpes, vistaNiebla, posNiebla;
    float zoomGolpes = 1.0f;
    Pergamino::GolpesEnPantalla golpes;
    ApplicationCamara::PasosFotograma pasos(Camera& cam) {
        ApplicationCamara::PasosFotograma p;
        p.combate = [this, &cam] { orden.push_back("combate"); vistaCombate = cam.posicionVista(); };
        p.vistaGolpes = [this, &cam] {
            orden.push_back("golpes");
            vistaGolpes = cam.posicionVista();
            zoomGolpes = cam.zoom();
            golpes.ponVista(cam.posicionVista(), cam.zoom());
        };
        p.nieblaYLuces = [this, &cam] { orden.push_back("niebla"); vistaNiebla = cam.posicionVista(); posNiebla = cam.position(); };
        return p;
    }
};
ApplicationCamara::EntradaFotograma entradaBase(float dt = 1.0f / 60.0f) {
    ApplicationCamara::EntradaFotograma in;
    in.dt = dt;
    in.celda = GridCoord{2, 2};
    in.ultimaCelda = GridCoord{2, 2};  // sin cambio de casilla
    return in;
}

MOTOR_TEST_CASE(reinicio_con_centinela_durante_una_sacudida_deja_todo_a_cero) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    const LimitesMapa m32 = limites(3, 2);
    colocaEn(cam, Vector2{1500.0f, 1000.0f}, 1.0f);  // destino del mapa anterior
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 5);
    e.destino = Vector2{1500.0f, 1000.0f};
    e.conDestino = true;
    ApplicationCamara::compone(cam, e, 0.05f, nullptr, 1.0f);
    MOTOR_CHECK(!iguales(cam.posicionVista(), cam.position()));  // hay sacudida viva
    // Una celda cualquiera NO reinicia.
    MOTOR_CHECK(!ApplicationCamara::reiniciaSiCentinela(GridCoord{3, 4}, e, cam));
    MOTOR_CHECK(e.amplitudPx > 0.0f && e.conDestino);
    // loadLevel deja el centinela: reinicia ANTES del seguimiento.
    MOTOR_CHECK(ApplicationCamara::reiniciaSiCentinela(GridCoord{-9999, -9999}, e, cam));
    MOTOR_CHECK(e.amplitudPx == 0.0f && e.reloj == 0.0 && e.duracion == 0.0f && !e.conDestino);
    MOTOR_CHECK(iguales(cam.posicionVista(), cam.position()));
    MOTOR_CHECK(ApplicationCamara::reiniciaSiCentinela(GridCoord{-9999, -9999}, e, cam));  // idempotente
    // El siguiente fotograma del mapa nuevo centra: caso 3x2 => (48,40).
    ApplicationCamara::sigueJugador(cam, e, Vector2{1500.0f, 1000.0f}, &m32, ApplicationCamara::kZonaMuerta);
    for (int i = 0; i < 20; ++i) cam.update(1.0f / 60.0f);
    ApplicationCamara::compone(cam, e, 1.0f / 60.0f, &m32, 1.0f);
    MOTOR_CHECK(cerca(cam.posicionVista().x, 48.0f) && cerca(cam.posicionVista().y, 40.0f));
}

MOTOR_TEST_CASE(orden_de_fotograma_compone_con_la_posicion_de_despues_de_update) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{0.0f, 0.0f}, 1.0f);
    ApplicationCamara::sigueJugador(cam, e, Vector2{160.0f, 0.0f}, nullptr, PoliticaCamara::ZonaMuerta{});  // centrar siempre
    cam.update(0.08f);  // a mitad de la transicion de 0,16 s
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 9);
    ApplicationCamara::compone(cam, e, 1.0f / 60.0f, nullptr, 1.0f);
    const auto s = PoliticaCamara::desplazamientoSacudida({7.0f, 0.24f}, static_cast<double>(1.0f / 60.0f), 9, 1.0f);
    MOTOR_CHECK(s.isOk());
    MOTOR_CHECK(cam.position().x > 70.0f && cam.position().x < 90.0f);
    MOTOR_CHECK(cerca(cam.posicionVista().x, cam.position().x + s.value().x / cam.zoom(), 1e-4f));
    MOTOR_CHECK(cerca(cam.posicionVista().y, cam.position().y + s.value().y / cam.zoom(), 1e-4f));
}

MOTOR_TEST_CASE(golpes_hud_y_matriz_coinciden_con_la_pose) {
    Camera cam(800, 600);
    cam.setZoom(2.0f);
    cam.setDesplazamientoVista(Vector2{1.5f, -1.0f});
    const Vector2 p = aPantalla(Vector2{100.0f, 50.0f}, cam.posicionVista(), cam.zoom(), 800, 600);
    MOTOR_CHECK(cerca(p.x, 597.0f, 1e-3f) && cerca(p.y, 402.0f, 1e-3f));
    // Usar position() (la logica) daria otro pixel: el criterio distingue las dos.
    MOTOR_CHECK(!cerca(aPantalla(Vector2{100.0f, 50.0f}, cam.position(), 2.0f, 800, 600).x, 597.0f, 0.5f));
    const Vector2 q = porMatriz(cam, Vector2{100.0f, 50.0f});
    MOTOR_CHECK(cerca(q.x, 597.0f, 0.01f) && cerca(q.y, 402.0f, 0.01f));
    const Vector2 w = cam.screenToWorld(p);
    MOTOR_CHECK(cerca(w.x, 100.0f, 0.01f) && cerca(w.y, 50.0f, 0.01f));
}

MOTOR_TEST_CASE(la_sacudida_cae_a_cero_exacto_y_no_toca_la_posicion_logica) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{300.0f, 400.0f}, 1.7f);
    const Vector2 antes = cam.position();
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 11);
    for (int i = 0; i < 40; ++i) {
        ApplicationCamara::compone(cam, e, 1.0f / 60.0f, nullptr, 1.0f);
        MOTOR_CHECK(iguales(cam.position(), antes));
    }
    MOTOR_CHECK(iguales(cam.posicionVista(), antes));  // cero exacto, sin deriva
    MOTOR_CHECK(e.amplitudPx == 0.0f && e.reloj == 0.0);
}

MOTOR_TEST_CASE(sacude_nueva_solo_sustituye_si_dura_mas) {
    EstadoCamaraJuego e;
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 1);
    e.reloj = 0.10;
    ApplicationCamara::sacude(e, 7.0f, 0.13f, 2);  // quedan 0,14: no dura mas
    MOTOR_CHECK(e.semilla == 1 && e.reloj == 0.10);
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 3);
    MOTOR_CHECK(e.semilla == 3 && e.reloj == 0.0);
}

MOTOR_TEST_CASE(seguimiento_con_zona_muerta_y_limites) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    const auto z = ApplicationCamara::kZonaMuerta;
    colocaEn(cam, Vector2{0.0f, 0.0f}, 1.0f);
    ApplicationCamara::sigueJugador(cam, e, Vector2{500.0f, 300.0f}, nullptr, z);  // sin destino: centra
    MOTOR_CHECK(e.conDestino && iguales(e.destino, Vector2{500.0f, 300.0f}) && cam.isTransitioning());
    for (int i = 0; i < 12; ++i) cam.update(1.0f / 60.0f);
    MOTOR_CHECK(!cam.isTransitioning());
    ApplicationCamara::sigueJugador(cam, e, Vector2{560.0f, 320.0f}, nullptr, z);  // dentro de +-96x48: quieta
    MOTOR_CHECK(iguales(e.destino, Vector2{500.0f, 300.0f}) && !cam.isTransitioning());
    ApplicationCamara::sigueJugador(cam, e, Vector2{700.0f, 300.0f}, nullptr, z);  // fuera: actor sobre el borde
    MOTOR_CHECK(cerca(e.destino.x, 604.0f) && cerca(e.destino.y, 300.0f) && cam.isTransitioning());
    const LimitesMapa m32 = limites(3, 2);  // con limites, el destino guardado ya viene recortado
    EstadoCamaraJuego e2;
    ApplicationCamara::sigueJugador(cam, e2, Vector2{1500.0f, 1000.0f}, &m32, z);
    MOTOR_CHECK(cerca(e2.destino.x, 48.0f) && cerca(e2.destino.y, 40.0f));
}

MOTOR_TEST_CASE(fotograma_orden_y_vista_de_cada_consumidor_durante_una_sacudida) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{300.0f, 400.0f}, 1.0f);
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 9);
    Registro r;
    ApplicationCamara::fotogramaCamara(cam, e, entradaBase(), r.pasos(cam));
    MOTOR_CHECK((r.orden == std::vector<std::string>{"combate", "golpes", "niebla"}));
    const auto s = PoliticaCamara::desplazamientoSacudida({7.0f, 0.24f}, static_cast<double>(1.0f / 60.0f), 9, 1.0f);
    MOTOR_CHECK(s.isOk());
    MOTOR_CHECK(!iguales(cam.posicionVista(), cam.position()));
    MOTOR_CHECK(cerca(cam.posicionVista().x, cam.position().x + s.value().x / cam.zoom(), 1e-4f));
    // Golpes y niebla/luces ven la vista de ESTE fotograma (ya con la sacudida); el combate, aun la anterior.
    MOTOR_CHECK(iguales(r.vistaGolpes, cam.posicionVista()) && iguales(r.vistaNiebla, cam.posicionVista()));
    MOTOR_CHECK(iguales(r.vistaCombate, r.posNiebla));
    MOTOR_CHECK(!iguales(r.vistaGolpes, r.posNiebla));
}

MOTOR_TEST_CASE(fotograma_camera_update_antes_de_componer_y_seguimiento_antes_del_combate) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{0.0f, 0.0f}, 1.0f);
    auto in = entradaBase(0.08f);
    in.celda = GridCoord{3, 2};  // cambia de casilla
    in.objetivo = Vector2{160.0f, 0.0f};
    Registro r;
    auto pasos = r.pasos(cam);
    bool vistoTransitando = false;
    pasos.combate = [&] { vistoTransitando = cam.isTransitioning(); };  // el seguimiento ya lanzo el tween
    const GridCoord nueva = ApplicationCamara::fotogramaCamara(cam, e, in, pasos);
    MOTOR_CHECK(vistoTransitando);
    MOTOR_CHECK(nueva.x == 3 && nueva.y == 2);
    MOTOR_CHECK(cam.position().x > 70.0f && cam.position().x < 90.0f);  // el update de este fotograma ya corrio
}

MOTOR_TEST_CASE(fotograma_en_batalla_o_sin_cambio_de_casilla_no_sigue) {
    for (const bool batalla : {true, false}) {
        Camera cam(800, 600);
        EstadoCamaraJuego e;
        colocaEn(cam, Vector2{0.0f, 0.0f}, 1.0f);
        auto in = entradaBase();
        in.objetivo = Vector2{500.0f, 300.0f};
        in.enBatalla = batalla;
        in.celda = GridCoord{5, 5};  // cambia
        in.combate = batalla ? &in : nullptr;
        Registro r;
        const GridCoord nueva = ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
        MOTOR_CHECK(batalla ? (nueva.x == 2 && !e.conDestino) : (nueva.x == 5 && e.conDestino));
        in.ultimaCelda = in.celda;  // misma casilla: nada
        EstadoCamaraJuego e2;
        ApplicationCamara::fotogramaCamara(cam, e2, in, r.pasos(cam));
        MOTOR_CHECK(!e2.conDestino);
    }
}

MOTOR_TEST_CASE(fotograma_pausa_congela_la_sacudida_pero_la_camara_sigue) {
    ponReducido(false);
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{300.0f, 400.0f}, 1.0f);
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 4);
    Registro r;
    ApplicationCamara::fotogramaCamara(cam, e, entradaBase(0.05f), r.pasos(cam));
    const double reloj = e.reloj;
    const Vector2 vista = cam.posicionVista();
    MOTOR_CHECK(reloj > 0.0);
    cam.transitionTo(Vector2{400.0f, 400.0f}, 1.0f, 0.5f, Camera::Easing::Linear);
    auto in = entradaBase(0.05f);
    in.pausa = true;
    for (int i = 0; i < 3; ++i) ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(e.reloj == reloj);                    // la sacudida no avanza con dt != 0 en pausa
    MOTOR_CHECK(cam.position().x > 300.0f);           // Camera::update si corre (la camara no se cuelga)
    const Vector2 d{cam.posicionVista().x - cam.position().x, cam.posicionVista().y - cam.position().y};
    MOTOR_CHECK(cerca(d.x, vista.x - 300.0f, 1e-4f) && cerca(d.y, vista.y - 400.0f, 1e-4f));  // mismo desplazamiento congelado
}

MOTOR_TEST_CASE(fotograma_movimiento_reducido_apaga_la_sacudida) {
    MOTOR_CHECK(ApplicationCamara::factorMovimiento(true) == 0.0f);
    MOTOR_CHECK(ApplicationCamara::factorMovimiento(false) == 1.0f);
    for (const bool reducido : {true, false}) {
        ponReducido(reducido);
        Camera cam(800, 600);
        EstadoCamaraJuego e;
        colocaEn(cam, Vector2{300.0f, 400.0f}, 1.7f);
        ApplicationCamara::sacude(e, 7.0f, 0.24f, 21);
        Registro r;
        bool movio = false;
        for (int i = 0; i < 14; ++i) {
            ApplicationCamara::fotogramaCamara(cam, e, entradaBase(), r.pasos(cam));
            movio = movio || !iguales(cam.posicionVista(), cam.position());
        }
        MOTOR_CHECK(movio == !reducido);
    }
    ponReducido(false);
}

MOTOR_TEST_CASE(fotograma_ciclo_de_combate_reinicia_al_entrar_y_al_salir) {
    ponReducido(false);
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{300.0f, 400.0f}, 1.0f);
    int combate = 0;
    auto in = entradaBase();
    Registro r;
    const auto sacudeYDestino = [&] {
        ApplicationCamara::sacude(e, 7.0f, 0.24f, 3);
        e.destino = Vector2{1.0f, 2.0f};
        e.conDestino = true;
    };
    sacudeYDestino();
    in.enBatalla = true;
    in.combate = &combate;  // ENTRA
    ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(e.amplitudPx == 0.0f && !e.conDestino && iguales(cam.posicionVista(), cam.position()) && e.combate == &combate);
    sacudeYDestino();  // dentro del mismo combate NO reinicia (los golpes sacuden)
    ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(e.amplitudPx > 0.0f && e.conDestino && !iguales(cam.posicionVista(), cam.position()));
    in.enBatalla = false;
    in.combate = nullptr;  // SALE
    ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(e.amplitudPx == 0.0f && !e.conDestino && iguales(cam.posicionVista(), cam.position()) && e.combate == nullptr);
    sacudeYDestino();  // fuera de combate sigue sin reiniciar
    ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(e.amplitudPx > 0.0f && e.conDestino);
}

MOTOR_TEST_CASE(fotograma_centinela_reinicia_antes_del_seguimiento) {
    ponReducido(false);
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    const LimitesMapa m32 = limites(3, 2);
    colocaEn(cam, Vector2{1500.0f, 1000.0f}, 1.0f);
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 5);
    e.destino = Vector2{1500.0f, 1000.0f};
    e.conDestino = true;
    auto in = entradaBase();
    in.ultimaCelda = GridCoord{-9999, -9999};
    in.objetivo = Vector2{1500.0f, 1000.0f};
    in.limites = &m32;
    Registro r;
    const GridCoord nueva = ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(nueva.x == 2 && nueva.y == 2);
    MOTOR_CHECK(e.amplitudPx == 0.0f);                              // la sacudida del mapa anterior murio
    MOTOR_CHECK(iguales(e.destino, Vector2{48.0f, 40.0f}));         // el seguimiento centro (no zona muerta del destino viejo)
    in.ultimaCelda = nueva;
    for (int i = 0; i < 20; ++i) ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(cerca(cam.posicionVista().x, 48.0f) && cerca(cam.posicionVista().y, 40.0f));
}

MOTOR_TEST_CASE(titulo_actualiza_sin_limites_ni_sacudida_y_deja_la_vista_en_cero) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{1500.0f, 1000.0f}, 1.0f);  // lejos de cualquier mapa: sin limites no se recorta
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 2);    // resto de una partida anterior
    cam.setDesplazamientoVista(Vector2{9.0f, 9.0f});
    ApplicationCamara::actualizaTitulo(cam, e, 1.0f / 60.0f);
    MOTOR_CHECK(iguales(cam.posicionVista(), cam.position()));
    MOTOR_CHECK(cerca(cam.position().x, 1500.0f) && cerca(cam.position().y, 1000.0f));
    cam.transitionTo(Vector2{1600.0f, 1000.0f}, 1.0f, 0.5f, Camera::Easing::Linear);  // el titulo pasea: update corre
    ApplicationCamara::actualizaTitulo(cam, e, 0.25f);
    MOTOR_CHECK(cam.position().x > 1540.0f && iguales(cam.posicionVista(), cam.position()));
    // Con limites (exploracion) el mismo punto SI se recorta: la diferencia es el nullptr del titulo.
    const LimitesMapa m = limites(3, 2);
    Registro r;
    auto in = entradaBase();
    in.limites = &m;
    ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(!iguales(cam.posicionVista(), cam.position()));
}

MOTOR_TEST_CASE(golpes_alineados_durante_sacudida_con_zoom_1_a_1_7) {
    ponReducido(false);
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    const LimitesMapa m = limites(64, 64);
    colocaEn(cam, Vector2{0.0f, 1000.0f}, 1.0f);
    cam.transitionTo(Vector2{0.0f, 1000.0f}, 1.7f, 0.5f, Camera::Easing::EaseInOutQuad);
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 3);
    Registro r;
    r.golpes.lanza(Vector2{40.0f, 1010.0f}, "-5", Pergamino::GolpesEnPantalla::Tipo::Dano);
    const Vector2 mundo = r.golpes.golpes()[0].mundo;
    bool tembloAlgo = false;
    auto in = entradaBase();
    in.limites = &m;
    for (int i = 0; i < 16; ++i) {
        ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
        const Vector2 a = r.golpes.aPantalla(mundo, 800, 600);                       // la proyeccion REAL de los golpes
        const Vector2 b = aPantalla(mundo, cam.posicionVista(), cam.zoom(), 800, 600);  // la formula de EstadoJuego/HUD (camaraX/Y, zoom)
        const Vector2 c = porMatriz(cam, mundo);                                     // la matriz de la camara
        MOTOR_CHECK(std::fabs(a.x - c.x) < 0.01f && std::fabs(a.y - c.y) < 0.01f);
        MOTOR_CHECK(std::fabs(b.x - c.x) < 0.01f && std::fabs(b.y - c.y) < 0.01f);
        const Vector2 w = cam.screenToWorld(a);
        MOTOR_CHECK(std::fabs(w.x - mundo.x) < 0.02f && std::fabs(w.y - mundo.y) < 0.02f);
        tembloAlgo = tembloAlgo || !iguales(cam.posicionVista(), cam.position());
    }
    MOTOR_CHECK(tembloAlgo);
    MOTOR_CHECK(cam.zoom() > 1.0f);
}

MOTOR_TEST_CASE(fotograma_compone_con_la_posicion_de_este_update_y_los_limites) {
    ponReducido(false);
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    const LimitesMapa m = limites(3, 2);  // mapa pequeno: la vista se recorta siempre
    colocaEn(cam, Vector2{0.0f, 0.0f}, 1.0f);
    cam.transitionTo(Vector2{1000.0f, 0.0f}, 1.0f, 0.16f, Camera::Easing::Linear);  // en movimiento, a mitad de tween
    Registro r;
    auto in = entradaBase(0.08f);
    in.limites = &m;
    ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(cam.position().x > 400.0f);  // el update de ESTE fotograma ya corrio antes de componer
    const auto p = PoliticaCamara::componerPose(cam.position(), cam.zoom(), Vector2{}, {800, 600}, &m);
    MOTOR_CHECK(p.isOk());
    MOTOR_CHECK(cerca(cam.posicionVista().x, p.value().vista.x) && cerca(cam.posicionVista().y, p.value().vista.y));
}

MOTOR_TEST_CASE(fotograma_el_tween_del_combate_corre_en_su_fotograma) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{0.0f, 0.0f}, 1.0f);
    Registro r;
    auto pasos = r.pasos(cam);
    pasos.combate = [&] { cam.transitionTo(Vector2{200.0f, 0.0f}, 1.7f, 0.0f, Camera::Easing::Linear); };  // corte
    ApplicationCamara::fotogramaCamara(cam, e, entradaBase(), pasos);
    MOTOR_CHECK(cerca(cam.position().x, 200.0f) && cerca(cam.zoom(), 1.7f));  // el callback va ANTES de Camera::update
}

MOTOR_TEST_CASE(fotograma_seguimiento_usa_la_zona_muerta_del_producto) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{0.0f, 0.0f}, 1.0f);
    e.destino = Vector2{0.0f, 0.0f};
    e.conDestino = true;
    Registro r;
    auto in = entradaBase();
    in.celda = GridCoord{3, 2};
    in.objetivo = Vector2{50.0f, 20.0f};  // dentro de +-96x48
    const GridCoord nueva = ApplicationCamara::fotogramaCamara(cam, e, in, r.pasos(cam));
    MOTOR_CHECK(nueva.x == 3 && iguales(e.destino, Vector2{0.0f, 0.0f}) && !cam.isTransitioning());
}

// Vigilante textual (lo unico que no se ejecuta sin ventana): quien lee posicionVista() y quien llama a quien.
// Se mira el codigo SIN comentarios y solo la presencia de la llamada; el orden y los efectos los prueban los casos de arriba.
MOTOR_TEST_CASE(los_consumidores_leen_posicion_de_vista_y_los_puntos_de_llamada_existen) {
    const std::string hud = leeFuente("src/Engine/ApplicationHud.cpp");
    MOTOR_CHECK(contiene(hud, "m_camera->posicionVista().x") && contiene(hud, "m_camera->posicionVista().y"));
    MOTOR_CHECK(!contiene(hud, "m_camera->position()"));
    for (const char* f : {"src/Render/FogOfWar.cpp", "src/Render/DynamicLightMap.cpp"}) {
        const std::string c = leeFuente(f);
        MOTOR_CHECK(contiene(c, "camera.posicionVista()") && !contiene(c, "camera.position()"));
    }
    const std::string cine = sinComentarios(leeFuente("src/Engine/ApplicationCinematica.cpp"));
    MOTOR_CHECK(contiene(cine, "ponVista(app.m_camera->posicionVista()") && !contiene(cine, "m_camera->position()"));
    MOTOR_CHECK(!contiene(cine, "ApplicationCamara::reinicia"));  // el ciclo de combate vive en fotogramaCamara
    const std::string bucle = sinComentarios(leeFuente("src/Engine/ApplicationBucle.cpp"));
    MOTOR_CHECK(contiene(bucle, "m_lastCameraCell = ApplicationCamara::fotogramaCamara(*m_camera, m_cine->camara"));
    // La conexion de Application::update con la entrada del fotograma (no ejecutable sin ventana): una por asignacion.
    for (const char* t : {"entrada.combate = m_session->battle()", "entrada.pausa = m_session->mode() == GameMode::Pause",
                          "entrada.enBatalla = m_session->mode() == GameMode::Battle", "entrada.limites = hayMapa ? &limitesMapa : nullptr",
                          "entrada.ultimaCelda = m_lastCameraCell", "entrada.celda = playerCell", "entrada.dt = deltaTime",
                          "pasos.combate = [this, deltaTime] { ApplicationCinematica::combate(*this, deltaTime); }",
                          "pasos.vistaGolpes = [this] { ApplicationCinematica::vistaGolpes(*this); }", "pasos.nieblaYLuces = nieblaYLuces"})
        MOTOR_CHECK(contiene(bucle, t));
    const std::string titulo = sinComentarios(leeFuente("src/Engine/ApplicationCinematicaTitulo.cpp"));
    MOTOR_CHECK(contiene(titulo, "ApplicationCamara::actualizaTitulo("));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
