// U4-01.E06 — Ver include/Engine/ApplicationCamara.h.
#include "Engine/ApplicationCamara.h"

#include <cstdlib>

namespace ApplicationCamara {

namespace {
PoliticaCamara::TamanoVista vistaDe(const Camera& cam) { return {cam.viewportWidth(), cam.viewportHeight()}; }
}  // namespace

void reinicia(EstadoCamaraJuego& e, Camera& cam) {
    e = EstadoCamaraJuego{};
    cam.setDesplazamientoVista(Vector2{});
}

bool reiniciaSiCentinela(const GridCoord& ultimaCelda, EstadoCamaraJuego& e, Camera& cam) {
    if (ultimaCelda.x != -9999 || ultimaCelda.y != -9999) return false;
    reinicia(e, cam);
    return true;
}

const PoliticaCamara::LimitesMapa* limitesAplicables(bool enTitulo, bool hayMapa,
                                                     const PoliticaCamara::LimitesMapa& limites) {
    return (enTitulo || !hayMapa) ? nullptr : &limites;
}

bool limitesDelMapa(int columnas, int filas, int anchoTile, int altoTile, PoliticaCamara::LimitesMapa& salida) {
    const auto r = PoliticaCamara::limitesDeMapa(columnas, filas, anchoTile, altoTile, kMargenSuperiorPx);
    if (!r.isOk()) return false;
    salida = r.value();
    return true;
}

float factorMovimiento(bool reducido) { return reducido ? 0.0f : 1.0f; }

bool movimientoReducidoEnEntorno() {
    static const bool reducido = std::getenv("MOTOR_MOVIMIENTO_REDUCIDO") != nullptr;  // como el HUD
    return reducido;
}

void sigueJugador(Camera& cam, EstadoCamaraJuego& e, const Vector2& objetivoMundo,
                  const PoliticaCamara::LimitesMapa* limites, const PoliticaCamara::ZonaMuerta& zona) {
    Vector2 destino = objetivoMundo;  // sin destino previo: centra en el objetivo
    if (e.conDestino) {
        const auto s = PoliticaCamara::seguirConZonaMuerta(e.destino, cam.zoom(), objetivoMundo, zona);
        if (s.isOk()) destino = s.value();
    }
    if (limites != nullptr) {
        const auto l = PoliticaCamara::limitarAlMapa(destino, cam.zoom(), vistaDe(cam), *limites);
        if (l.isOk()) destino = l.value();
    }
    const bool igual = e.conDestino && destino.x == e.destino.x && destino.y == e.destino.y;
    e.destino = destino;
    e.conDestino = true;
    if (!igual) cam.transitionTo(destino, cam.zoom(), 0.16f, Camera::Easing::Linear);
}

void sacude(EstadoCamaraJuego& e, float amplitudPx, float duracion, std::uint32_t semilla) {
    const float queda = e.amplitudPx > 0.0f ? e.duracion - static_cast<float>(e.reloj) : 0.0f;
    if (!(amplitudPx > 0.0f) || !(duracion > queda)) return;
    e.amplitudPx = amplitudPx;
    e.duracion = duracion;
    e.reloj = 0.0;
    e.semilla = semilla;
}

void compone(Camera& cam, EstadoCamaraJuego& e, float dt, const PoliticaCamara::LimitesMapa* limites,
             float factorMovimiento) {
    Vector2 sacudida{};
    if (e.amplitudPx > 0.0f) {
        e.reloj += static_cast<double>(dt);
        const auto s = PoliticaCamara::desplazamientoSacudida({e.amplitudPx, e.duracion}, e.reloj, e.semilla,
                                                              factorMovimiento);
        if (s.isOk()) sacudida = s.value();
        if (e.reloj >= static_cast<double>(e.duracion)) {  // acabada: a cero exacto
            e.amplitudPx = 0.0f;
            e.duracion = 0.0f;
            e.reloj = 0.0;
        }
    }
    const auto pose = PoliticaCamara::componerPose(cam.position(), cam.zoom(), sacudida, vistaDe(cam), limites);
    cam.setDesplazamientoVista(pose.isOk() ? pose.value().vista - cam.position() : Vector2{});
}

}  // namespace ApplicationCamara
