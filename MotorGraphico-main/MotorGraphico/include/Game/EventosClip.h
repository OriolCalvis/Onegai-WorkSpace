#pragma once

// U3-05.E06 -- Separar el evento logico del visual (puertos). Pura, sin GL ni audio.
// Un EventoFotograma (ClipAnimacion.h) lleva `tipo`: "logico" | "visual" | "sonido". Cada tipo va
// a UN puerto distinto; quien implementa el puerto logico (dano, flags) no es el visual, de modo
// que reconstruir o repintar el render nunca puede llamar a aplicaLogico (no se aplica dano dos veces).
// Patron EscenarioCinematica (Cinematica.h): interfaz pura inyectada; las pruebas ponen una falsa.
// Esta pieza NO llama a AudioDirector: el puerto de sonido real es el adaptador de E07.
// Quien llama decide cuando enrutar (p. ej. los eventos de PasoClip::eventos, en orden, una vez cada
// uno); enrutaEvento no guarda estado ni deduplica.

#include "Core/Errors/Result.h"
#include "Game/ClipAnimacion.h"

namespace Game {

class PuertosEventoClip {
public:
    virtual ~PuertosEventoClip() = default;
    virtual void aplicaLogico(const EventoFotograma&) = 0;   // dano, flags: UNA vez
    virtual void muestraVisual(const EventoFotograma&) = 0;  // particula/flash: repetible
    virtual void suena(const EventoFotograma&) = 0;          // sonido
};

// Entrega el evento a exactamente un puerto segun `tipo`. Ok(true) = un puerto lo recibio.
// `tipo` desconocido (incluido vacio o con otra capitalizacion) = Error("tipo de evento no
// reconocido: <tipo>") y ningun puerto llamado. Ok(false) esta reservado para "evento valido pero
// sin destino" y hoy no se devuelve nunca: se mantiene Result<bool> por el contrato de E06, para
// poder anadir tipos sin destino sin cambiar la firma.
Result<bool> enrutaEvento(const EventoFotograma& e, PuertosEventoClip& puertos);

}  // namespace Game
