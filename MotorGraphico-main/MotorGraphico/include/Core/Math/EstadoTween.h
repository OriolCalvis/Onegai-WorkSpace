#pragma once

#include "Core/Errors/Result.h"
#include "Core/Math/Interpolacion.h"

// U3-07.E06 — Estado de un tween y avance puro (sin GL, sin heap, sin callbacks).
// Sala limpia. Compatible con Camera::update (Camera.cpp:34-41): duracion 0 termina en el
// primer avance, incluso con dt == 0. El llamador reacciona a PasoTween::acabaDeTerminar.
namespace Interpolacion {

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
//  - duracion > 0: dt == 0 no avanza; si transcurrido + dt >= duracion termina y
//    transcurrido se fija a duracion (la sobra de dt se descarta; asi avanzar en n trozos
//    equivale a avanzar de una vez).
Result<PasoTween> avanzaTween(const EstadoTween& estado, double dt);

}  // namespace Interpolacion
