// U3-05.E06 -- Enrutar eventos de clip a su puerto (ver EventosClip.h).
#include "Game/EventosClip.h"

namespace Game {

Result<bool> enrutaEvento(const EventoFotograma& e, PuertosEventoClip& puertos) {
    if (!tipoEventoValido(e.tipo)) {
        return Result<bool>::Error("tipo de evento no reconocido: " + e.tipo);
    }
    if (e.tipo == "logico") {
        puertos.aplicaLogico(e);
    } else if (e.tipo == "visual") {
        puertos.muestraVisual(e);
    } else {
        puertos.suena(e);
    }
    return Result<bool>::Ok(true);
}

void repintaVisual(const EventoFotograma& e, PuertosEventoClip& puertos) {
    if (e.tipo == "visual") puertos.muestraVisual(e);
}

}  // namespace Game
