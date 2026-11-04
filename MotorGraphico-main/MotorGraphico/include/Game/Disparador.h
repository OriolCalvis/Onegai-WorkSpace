// E-DISPARADORES-1 (D0026) — El «disparador» de una cinematica: que la escena empiece cuando el jugador pisa su zona
// o se acerca a su casilla Y la ve, y que antes lo lleve (andando o con un fundido) a donde debe estar.
//
//   "disparador": {
//     "lleva":  {"a": [55, 53], "mira": "arriba", "como": "andando"},   // andando (defecto) | fundido
//     "accion": {"cinematica": "plaza_sermon"},                         // cinematica (defecto: la propia) | beat | marca
//     "once":   true,                                                   // E5 hara valer false (hoy: una vez por partida)
//     "radio":  5, "vision": false }                                    // opcionales: sobreescriben los del ambito
//
// Orden (D0026.5), en CatalogoCinematicas::laQueEmpieza: condiciones (nivel, requiere, sin, vista) -> la zona pisada
// O el jugador dentro del radio (Chebyshev) del foco -> si vino por el radio, linea de vision al foco -> lleva + accion.
// El foco es `lleva.a` o, sin el, el centro de la zona (Cinematica::puntoDeEntrada). Radio por ambito del nivel
// (Level/AmbitoNivel.h): interior pequeno = al entrar (sin vision), interior grande 5, exterior 10.
// GL-free.
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Game/LineaDeVision.h"
#include "Level/AmbitoNivel.h"

class JsonValue;

namespace Game {

struct Disparador {
    enum class Accion { Cinematica, Beat, Marca };  // `combate` llega con E4
    bool hay = false;
    bool hayLleva = false;
    GridCoord lleva;                // donde se pone al jugador (y el foco del radio y de la vision)
    std::string mira;               // rumbo al llegar; vacio: no se gira
    bool andando = true;            // false: con un fundido
    Accion accion = Accion::Cinematica;
    std::string accionId;           // Cinematica: vacio = la propia; Beat: el beat; Marca: la flag
    bool once = true;
    int radio = -1;                 // -1: el del ambito del nivel
    int vision = -1;                // -1: la del ambito (no en el interior pequeno); 0 no; 1 si
};

// Lo que la cinematica necesita saber del nivel donde esta el jugador (el motor lo rellena; las pruebas, a mano).
struct VistaDelNivel {
    AmbitoNivel ambito = AmbitoNivel::Exterior;  // ya resuelto (nunca Deducido)
    TapaLaVista tapa;                            // vacia: nada tapa la vista
};

// Quien es el jugador cuando empieza la escena y como se le lleva: `ruta` da las casillas de aqui a `destino`
// (GameSession::rutaAndando); vacia = no hay camino y se le lleva con un fundido (D0026.2).
struct LlevaDelJugador {
    GridCoord jugador;
    std::function<std::vector<GridCoord>(const GridCoord& destino)> ruta;
};

// El campo «disparador» de la cinematica `id` (objeto JSON ya comprobado como presente).
Result<Disparador> leeDisparador(const JsonValue& o, const std::string& id);

}  // namespace Game
