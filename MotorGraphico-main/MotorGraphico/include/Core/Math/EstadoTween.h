#pragma once

#include "Core/Errors/Result.h"
#include "Core/Math/Interpolacion.h"

// U3-07.E06 — Estado de un tween y avance puro (sin GL, sin heap, sin callbacks).
// Sala limpia. Compatible con Camera::update (Camera.cpp:34-41): duracion 0 termina en el
// primer avance, incluso con dt == 0. El llamador reacciona a PasoTween::acabaDeTerminar.
//
// Camera (E08): Camera::update acumula en float (Camera.cpp:36); con 60 x (1.0f/60) llega a
// 0.99999970 y sigue activa, mientras avanzaTween en double llega a 1.00000005 y termina un
// fotograma antes. Decision: en E08 Camera CONSERVA su reloj float y solo reutiliza
// progreso/curva/tweenVec2; si migra a avanzaTween sera con prueba diferencial del fotograma
// de fin.
namespace Interpolacion {

// Tolerancia relativa de final: la suma de trozos puede quedar por debajo de duracion por
// redondeo (10 x 0.1 = 0.9999999999999999). Se termina si
// transcurrido + dt >= duracion - 1e-9 * duracion (y transcurrido se fija a duracion).
struct EstadoTween {
    double duracion = 0;
    double transcurrido = 0;
    TipoCurva curva = TipoCurva::Lineal;
    bool pausado = false;
    bool terminado = false;
};

struct PasoTween {
    EstadoTween estado;           // estado tras el avance
    double progreso = 0;          // Interpolacion::progreso(transcurrido, duracion)
    double valorCurvado = 0;      // Interpolacion::curva(curva, progreso)
    bool acabaDeTerminar = false; // verdadero SOLO en la llamada que cruza el final
};

// Avanza el tween dt segundos.
//  - Estado invalido (duracion o transcurrido no finitos, duracion < 0, transcurrido < 0)
//    o dt no finito o dt < 0 -> Error (se valida siempre, incluso pausado o terminado).
//  - pausado (manda sobre todo) o terminado -> mismo estado, acabaDeTerminar = false.
//  - duracion == 0 (no pausado, no terminado) -> termina ya, incluso con dt == 0.
//  - duracion > 0: dt == 0 no avanza (aunque transcurrido ya sea >= duracion: termina en el
//    siguiente avance con dt > 0); si transcurrido + dt >= duracion*(1-1e-9) termina y
//    transcurrido se fija a duracion (la sobra de dt se descarta; asi avanzar en n trozos
//    equivale a avanzar de una vez).
Result<PasoTween> avanzaTween(const EstadoTween& estado, double dt);

}  // namespace Interpolacion
