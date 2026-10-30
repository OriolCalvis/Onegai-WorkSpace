#pragma once

// U3-05.E04 -- Avanzar el cursor de un clip de animacion (pura, sin GL ni reloj).
// Tiempo: dt en segundos; el cursor guarda ms dentro del fotograma (double). Cada paso
// parte del estado del cursor y resta duraciones enteras (ms), sin acumular dt por separado.
// Presupuesto: se emiten exactamente hasta maxEventos eventos por llamada. Si un grupo de
// eventos de un fotograma no cabe, el cursor ya esta en ese fotograma y eventoPendiente es el
// indice del siguiente evento por emitir; la llamada siguiente sigue desde ahi, sin duplicar.
// Si el presupuesto se agota justo al llegar a una frontera con eventos, el cursor queda en el
// borde del fotograma anterior (enFotogramaMs == duracion).
// Tolerancia: una frontera se cruza si falta menos de 1e-6 ms, para que repartir dt en pasos
// pequenos no cambie en que tick se cruza (deriva de redondeo de la suma de dt).
// Continuar: mientras PasoClip::pendiente, llamar de nuevo con dt = tiempoPendiente.
// Fuera de dominio (maxEventos 0, dt no finito o negativo o con dt*1000 no finito, duracion
// <= 0, cursor o eventos incoherentes con el clip): cursor igual, sin eventos, pendiente=false.

#include <cstddef>
#include <vector>

#include "Game/ClipAnimacion.h"

namespace Game {

struct CursorClip {
    int fotograma = 0;
    double enFotogramaMs = 0.0;
    int vueltas = 0;
    bool terminado = false;
    bool iniciado = false;
    int eventoPendiente = 0;  // siguiente evento por emitir del fotograma actual (0 = ninguno a medias)
};

struct PasoClip {
    CursorClip cursor;
    std::vector<EventoFotograma> eventos;
    double tiempoPendiente = 0.0;  // segundos no consumidos por agotar el presupuesto
    bool pendiente = false;        // true siempre que el presupuesto corte el paso
};

// dt == 0 solo continua un paso pendiente.
PasoClip avanzaClip(const CursorClip& cursor, double dt, const ClipAnimacion& clip,
                    std::size_t maxEventos = 64);

}  // namespace Game
