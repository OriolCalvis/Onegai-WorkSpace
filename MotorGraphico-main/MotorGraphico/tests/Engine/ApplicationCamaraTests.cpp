// U4-01.E06 — Adaptador ApplicationCamara (GL-free, sin Application): reinicio por centinela, orden de
// fotograma, golpes alineados con la pose, titulo sin limites, pausa y movimiento reducido.
#include "Engine/ApplicationCamara.h"

#include "TestHarness.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

#include <glm/glm.hpp>

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

MOTOR_TEST_CASE(golpes_alineados_durante_sacudida_con_zoom_1_a_1_7) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    const LimitesMapa m = limites(64, 64);
    colocaEn(cam, Vector2{0.0f, 1000.0f}, 1.0f);
    cam.transitionTo(Vector2{0.0f, 1000.0f}, 1.7f, 0.5f, Camera::Easing::EaseInOutQuad);
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 3);
    bool tembloAlgo = false;
    for (int i = 0; i < 16; ++i) {
        cam.update(1.0f / 60.0f);
        ApplicationCamara::compone(cam, e, 1.0f / 60.0f, &m, 1.0f);
        const Vector2 mundo{40.0f, 1010.0f};
        const Vector2 a = aPantalla(mundo, cam.posicionVista(), cam.zoom(), 800, 600);  // golpes
        const Vector2 b = aPantalla(mundo, cam.posicionVista(), cam.zoom(), 800, 600);  // EstadoJuego
        const Vector2 c = porMatriz(cam, mundo);
        MOTOR_CHECK(std::fabs(a.x - c.x) < 0.01f && std::fabs(a.y - c.y) < 0.01f);
        MOTOR_CHECK(std::fabs(b.x - c.x) < 0.01f && std::fabs(b.y - c.y) < 0.01f);
        tembloAlgo = tembloAlgo || !iguales(cam.posicionVista(), cam.position());
    }
    MOTOR_CHECK(tembloAlgo);
    MOTOR_CHECK(cam.zoom() > 1.0f);
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

MOTOR_TEST_CASE(titulo_sin_limites_y_exploracion_con_limites) {
    const LimitesMapa m = limites(3, 2);
    MOTOR_CHECK(ApplicationCamara::limitesAplicables(true, true, m) == nullptr);
    MOTOR_CHECK(ApplicationCamara::limitesAplicables(false, false, m) == nullptr);
    MOTOR_CHECK(ApplicationCamara::limitesAplicables(false, true, m) == &m);
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{1500.0f, 1000.0f}, 1.0f);
    ApplicationCamara::compone(cam, e, 1.0f / 60.0f, nullptr, 1.0f);  // titulo
    MOTOR_CHECK(iguales(cam.posicionVista(), cam.position()));
    ApplicationCamara::compone(cam, e, 1.0f / 60.0f, &m, 1.0f);  // con limites se recorta al mapa
    MOTOR_CHECK(!iguales(cam.posicionVista(), cam.position()));
    MOTOR_CHECK(cerca(cam.posicionVista().x, 48.0f) && cerca(cam.posicionVista().y, 40.0f));
}

MOTOR_TEST_CASE(pausa_no_avanza_la_sacudida) {
    Camera cam(800, 600);
    EstadoCamaraJuego e;
    colocaEn(cam, Vector2{300.0f, 400.0f}, 1.0f);
    ApplicationCamara::sacude(e, 7.0f, 0.24f, 4);
    ApplicationCamara::compone(cam, e, 0.05f, nullptr, 1.0f);
    const double reloj = e.reloj;
    const Vector2 vista = cam.posicionVista();
    for (int i = 0; i < 30; ++i) ApplicationCamara::compone(cam, e, 0.0f, nullptr, 1.0f);  // dt = 0 en pausa
    MOTOR_CHECK(e.reloj == reloj && iguales(cam.posicionVista(), vista));
    const std::string bucle = leeFuente("src/Engine/ApplicationBucle.cpp");
    MOTOR_CHECK(contiene(bucle, "GameMode::Pause ? 0.0f : deltaTime"));
}

MOTOR_TEST_CASE(movimiento_reducido_apaga_la_sacudida) {
    MOTOR_CHECK(ApplicationCamara::factorMovimiento(true) == 0.0f);
    MOTOR_CHECK(ApplicationCamara::factorMovimiento(false) == 1.0f);
    for (const bool reducido : {true, false}) {
        Camera cam(800, 600);
        EstadoCamaraJuego e;
        colocaEn(cam, Vector2{300.0f, 400.0f}, 1.7f);
        ApplicationCamara::sacude(e, 7.0f, 0.24f, 21);
        bool movio = false;
        for (int i = 0; i < 14; ++i) {
            ApplicationCamara::compone(cam, e, 1.0f / 60.0f, nullptr, ApplicationCamara::factorMovimiento(reducido));
            movio = movio || !iguales(cam.posicionVista(), cam.position());
        }
        MOTOR_CHECK(movio == !reducido);
    }
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

// Vigilantes de los puntos de llamada que no se pueden ejecutar sin ventana ni Application.
MOTOR_TEST_CASE(los_consumidores_leen_posicion_de_vista_y_el_titulo_no_limita) {
    const std::string hud = leeFuente("src/Engine/ApplicationHud.cpp");
    MOTOR_CHECK(contiene(hud, "m_camera->posicionVista().x") && contiene(hud, "m_camera->posicionVista().y"));
    MOTOR_CHECK(!contiene(hud, "m_camera->position()"));
    for (const char* f : {"src/Render/FogOfWar.cpp", "src/Render/DynamicLightMap.cpp"}) {
        const std::string s = leeFuente(f);
        MOTOR_CHECK(contiene(s, "camera.posicionVista()") && !contiene(s, "camera.position()"));
    }
    const std::string cine = leeFuente("src/Engine/ApplicationCinematica.cpp");
    MOTOR_CHECK(contiene(cine, "ponVista(app.m_camera->posicionVista()") && !contiene(cine, "m_camera->position()"));
    const std::string bucle = leeFuente("src/Engine/ApplicationBucle.cpp");
    MOTOR_CHECK(contiene(bucle, "limitesAplicables(isAtTitle()") && contiene(bucle, "vistaGolpes(*this)"));
    const auto reinicio = bucle.find("reiniciaSiCentinela(m_lastCameraCell");
    const auto sobrescribe = bucle.find("m_lastCameraCell = playerCell");
    MOTOR_CHECK(reinicio != std::string::npos && sobrescribe != std::string::npos && reinicio < sobrescribe);
    const std::string titulo = leeFuente("src/Engine/ApplicationCinematicaTitulo.cpp");
    MOTOR_CHECK(contiene(titulo, "compone(*app.m_camera, estado.camara, dt, nullptr,"));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
