#pragma once

// U3-05.E04 -- Avanzar el cursor de un clip de animacion (pura, sin GL ni reloj).
// Tiempo: dt en segundos; el cursor guarda ms dentro del fotograma (double). Cada paso
// parte del estado del cursor y resta duraciones enteras (ms), sin acumular dt por separado.
// Presupuesto: un fotograma entra con TODOS sus eventos o no entra (si no caben y ya se
// emitio algo, el cursor queda en el borde del fotograma anterior: enFotogramaMs == duracion).
// Continuar = llamar de nuevo con dt = tiempoPendiente (o 0 si el consumidor ya lo sumo).

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
};

struct PasoClip {
    CursorClip cursor;
    std::vector<EventoFotograma> eventos;
    double tiempoPendiente = 0.0;  // segundos no consumidos por agotar el presupuesto
};

// dt no finito, < 0 o tan grande que dt*1000 desborda: cursor igual y sin eventos. dt == 0 solo continua un paso pendiente.
PasoClip avanzaClip(const CursorClip& cursor, double dt, const ClipAnimacion& clip,
                    std::size_t maxEventos = 64);

}  // namespace Game
