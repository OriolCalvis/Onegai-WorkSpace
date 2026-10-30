// U3-05.E06 -- Enrutar eventos de clip a su puerto (ver EventosClip.h).
#include "Game/EventosClip.h"

namespace Game {

Result<bool> enrutaEvento(const EventoFotograma& e, PuertosEventoClip& puertos) {
    if (e.tipo == "logico") {
        puertos.aplicaLogico(e);
    } else if (e.tipo == "visual") {
        puertos.muestraVisual(e);
    } else if (e.tipo == "sonido") {
        puertos.suena(e);
    } else {
        return Result<bool>::Error("tipo de evento no reconocido: " + e.tipo);
    }
    return Result<bool>::Ok(true);
}

}  // namespace Game
