// U4-01.E06 — La camara virtual en el juego: seguimiento con zona muerta, limites de mapa y sacudida
// con caida, sobre la politica pura (Core/Math/PoliticaCamara.h). Funciones libres: reciben Camera& y
// el estado, nunca Application& (sin friend nuevo). Contrato: planificacion/pla_unity/descomposicio/
// contratos/U4-01_camara.md, E06.
#pragma once

#include <cstdint>

#include "Core/Math/GridCoord.h"
#include "Core/Math/PoliticaCamara.h"
#include "Core/Math/Vector2.h"
#include "Render/Camera.h"

// Propietario: EstadoCinematicas::camara. Nace con el y se reinicia al cambiar de mapa y al entrar/salir del combate.
struct EstadoCamaraJuego {
    Vector2 destino;                // destino vigente del seguimiento, YA LIMITADO (D-T12)
    bool conDestino = false;
    float amplitudPx = 0.0f;        // sacudida: amplitud en pixeles de PANTALLA
    float duracion = 0.0f;
    double reloj = 0.0;
    std::uint32_t semilla = 0;
};

namespace ApplicationCamara {

// Valores de producto (Oriol, 30/10). D-O1: zona muerta +-96x48 px de pantalla. D-O2: sacudida con los
// valores de hoy (4 px de mundo, 0,14 s danio / 0,24 s critico) pero con caida a cero.
inline constexpr PoliticaCamara::ZonaMuerta kZonaMuerta{96.0f, 48.0f};
inline constexpr float kSacudidaMundoPx = 4.0f;
inline constexpr float kSacudidaDanio = 0.14f;
inline constexpr float kSacudidaCritico = 0.24f;
inline constexpr float kMargenSuperiorPx = 0.0f;  // D-T7: el romboDeMapa manda; sin margen extra en la caja

// Cambio de mapa, entrar/salir de combate: sin destino, sacudida a 0, desplazamiento a 0. Idempotente.
void reinicia(EstadoCamaraJuego& e, Camera& cam);
// Cambio de mapa: loadLevel deja m_lastCameraCell en el centinela {-9999,-9999}; se llama ANTES de que el
// seguimiento lo sobrescriba. Devuelve true si reinicio.
bool reiniciaSiCentinela(const GridCoord& ultimaCelda, EstadoCamaraJuego& e, Camera& cam);
// D-O6: limites en exploracion y combate, NO en el titulo (ni sin mapa). nullptr = sin limites.
const PoliticaCamara::LimitesMapa* limitesAplicables(bool enTitulo, bool hayMapa,
                                                     const PoliticaCamara::LimitesMapa& limites);
// Caja y rombo del mapa cargado. false si el mapa no es valido (0 columnas...).
bool limitesDelMapa(int columnas, int filas, int anchoTile, int altoTile, PoliticaCamara::LimitesMapa& salida);
// D-O4: movimiento reducido => sacudida apagada (0). La entrada es MOTOR_MOVIMIENTO_REDUCIDO.
float factorMovimiento(bool reducido);
bool movimientoReducidoEnEntorno();
// Seguimiento: solo al CAMBIAR de casilla y fuera de combate. Zona muerta, luego limites; guarda el destino
// (ya limitado) y solo lanza el transitionTo (0,16 s lineal) si cambia.
void sigueJugador(Camera& cam, EstadoCamaraJuego& e, const Vector2& objetivoMundo,
                  const PoliticaCamara::LimitesMapa* limites, const PoliticaCamara::ZonaMuerta& zona);
// Dispara la sacudida (amplitud en pixeles de pantalla). Una nueva solo sustituye a la vigente si dura mas.
void sacude(EstadoCamaraJuego& e, float amplitudPx, float duracion, std::uint32_t semilla);
// Tras Camera::update(dt): avanza el reloj de la sacudida (dt = 0 en pausa), compone la pose y entrega el
// desplazamiento de vista a la camara. limites == nullptr: titulo / sin mapa.
void compone(Camera& cam, EstadoCamaraJuego& e, float dt, const PoliticaCamara::LimitesMapa* limites,
             float factorMovimiento);

}  // namespace ApplicationCamara
