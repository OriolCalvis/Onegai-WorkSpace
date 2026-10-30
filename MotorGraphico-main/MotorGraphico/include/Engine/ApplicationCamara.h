// U4-01.E06 — La camara virtual en el juego: seguimiento con zona muerta, limites de mapa y sacudida
// con caida, sobre la politica pura (Core/Math/PoliticaCamara.h). Funciones libres: reciben Camera& y
// el estado, nunca Application& (sin friend nuevo). Contrato: planificacion/pla_unity/descomposicio/
// contratos/U4-01_camara.md, E06.
#pragma once

#include <cstdint>
#include <functional>

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
    const void* combate = nullptr;  // identidad del combate vigente (BattleState*); null = exploracion
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
// Caja y rombo del mapa cargado. false si el mapa no es valido (0 columnas...).
bool limitesDelMapa(int columnas, int filas, int anchoTile, int altoTile, PoliticaCamara::LimitesMapa& salida);
// D-O4: movimiento reducido => sacudida apagada (0). La entrada es MOTOR_MOVIMIENTO_REDUCIDO.
float factorMovimiento(bool reducido);
bool movimientoReducidoEnEntorno();  // lee MOTOR_MOVIMIENTO_REDUCIDO en cada llamada (barato; las pruebas lo cambian)
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

// Fotograma de exploracion/combate (lo llama Application::update UNA vez). D-O6: limites en exploracion y combate;
// el titulo nunca pasa por aqui (actualizaTitulo, sin limites).
struct EntradaFotograma {
    float dt = 0.0f;
    bool pausa = false;                // la pausa congela la sacudida (compone con dt = 0); Camera::update sigue
    bool enBatalla = false;            // sin seguimiento (AV13-12: lo lleva el combate)
    GridCoord celda;                   // casilla del jugador
    GridCoord ultimaCelda;             // m_lastCameraCell (centinela {-9999,-9999} = mapa nuevo)
    Vector2 objetivo;                  // centro de la casilla (con desfase de altura); solo se lee si cambia de casilla
    const void* combate = nullptr;     // combate vigente (null = ninguno): su cambio reinicia la camara
    const PoliticaCamara::LimitesMapa* limites = nullptr;  // nullptr = sin mapa
};
// Consumidores del fotograma, en su orden (los dobles de las pruebas registran quien ve que vista).
struct PasosFotograma {
    std::function<void()> combate;         // Cinematica::combate: transitionTo de combate, golpes, camara cerca
    std::function<void()> vistaGolpes;     // golpes: ponVista(posicionVista, zoom)
    std::function<void()> nieblaYLuces;    // niebla y mapa de luces: leen posicionVista
};
// Orden: 0 limites ya en la entrada; 1 centinela => reinicia; 2 seguimiento con zona muerta (si cambia de casilla
// y no hay batalla); 3 combate; 3b cambio de combate => reinicia; 4 Camera::update; 5 compone; 6 vistaGolpes;
// 7 niebla y luces. Devuelve la nueva ultimaCelda.
GridCoord fotogramaCamara(Camera& cam, EstadoCamaraJuego& e, const EntradaFotograma& in, const PasosFotograma& pasos);
// Titulo (D-O6): Camera::update y compone sin limites y sin sacudida (deja el desplazamiento a 0).
void actualizaTitulo(Camera& cam, EstadoCamaraJuego& e, float dt);

}  // namespace ApplicationCamara
