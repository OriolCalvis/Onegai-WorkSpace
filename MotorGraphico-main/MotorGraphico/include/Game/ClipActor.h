#pragma once

// U3-05.E07a -- Adaptador de un actor al reproductor de clips (gl-free).
// ClipActor posee el ClipAnimacion y su CursorClip. En cada avanza(dt): avanzaClip, enrutaEvento
// (UNA vez por ocurrencia, en orden) y entrega el sprite del fotograma actual (1-based, como
// FotogramaClip::sprite) al DestinoFotograma SOLO cuando cambia respecto al ultimo entregado
// (el primer avanza siempre entrega). No depende de Render ni de GL: el destino es una interfaz
// pura. Direccion de dependencias: Render NO conoce Game; quien conecta (E07b) adapta con
// DestinoFuncion (p. ej. [&](int s) { entidad.ponerFotograma(s); }).
// Drenado: mientras PasoClip::pendiente se vuelve a llamar con tiempoPendiente, con un tope de
// kMaxPasosPorAvance llamadas; si el tope se agota, el resto queda en el cursor (pendiente) y se
// retoma en el siguiente avanza (el tiempo no consumido se guarda y se suma a su dt), sin perder
// ni duplicar eventos.
// Eventos con tipo no reconocido (clip armado a mano; el lector ya los rechaza): no se enrutan y
// avanza() devuelve cuantos fueron rechazados, para que quien llama lo registre.
// Reentrada: un puerto de evento puede llamar a cambia() sobre este mismo actor (p. ej. «fin de
// ataque» vuelve al idle). Se detecta con un contador de generacion: avanza() deja de emitir el
// resto del lote del clip viejo, NO sobrescribe cursor ni resto con los del paso viejo y entrega el
// sprite del clip nuevo. Un cambia() que no cambia nada (ReiniciarSiDistinto con el mismo id) no
// cuenta. El actor solo conoce los cambios hechos por cambia(): si codigo externo cambia el sprite
// de la entidad sin pasar por aqui (play/showRestFrame sin control externo), el actor no lo sabe;
// lo gestiona el consumidor (E07b).
// Contrato de reentrada (avanza/repinta anidados): mientras avanza() o repinta() despachan a un
// puerto, una llamada anidada a avanza() o repinta() NO hace nada (avanza devuelve 0, sin mover el
// cursor ni entregar sprite); cambia() sigue permitido (ver generacion arriba). repinta() copia los
// eventos del fotograma por valor antes de despachar y se detiene si cambia la generacion. La marca
// se limpia por RAII aunque un puerto lance una excepcion.

#include <functional>
#include <utility>

#include "Game/ClipAnimacion.h"
#include "Game/CursorClip.h"
#include "Game/EventosClip.h"

namespace Game {

class DestinoFotograma {
public:
    virtual ~DestinoFotograma() = default;
    virtual void ponerFotograma(int sprite) = 0;
};

// Adaptador de una funcion a DestinoFotograma (para conectar sin que Render conozca Game).
class DestinoFuncion : public DestinoFotograma {
public:
    explicit DestinoFuncion(std::function<void(int)> f) : m_f(std::move(f)) {}
    void ponerFotograma(int sprite) override {
        if (m_f) m_f(sprite);
    }

private:
    std::function<void(int)> m_f;
};

class ClipActor {
public:
    static constexpr int kMaxPasosPorAvance = 16;

    ClipActor(IdClip id, ClipAnimacion clip);

    // Devuelve el numero de eventos rechazados por tipo desconocido (normalmente 0).
    int avanza(double dt, PuertosEventoClip& eventos, DestinoFotograma& destino);

    // Delegado en cambiaClip. Un cambio aceptado descarta el tiempo pendiente y obliga a reentregar
    // el sprite en el siguiente avanza. Clip nuevo sin fotogramas: no cambia nada. Con ReiniciarSiDistinto y
    // el mismo id no reinicia (ni cambia el clip poseido).
    void cambia(const IdClip& idNuevo, const ClipAnimacion& nuevo, PoliticaCambio politica);

    // Repinta (muestraVisual, repetible) los eventos visuales del fotograma actual; nunca aplica
    // logica ni suena. Para reconstruir el render sin avanzar el clip.
    void repinta(PuertosEventoClip& eventos);

    const CursorClip& cursor() const { return m_cursor; }
    const IdClip& id() const { return m_id; }

private:
    IdClip m_id;
    ClipAnimacion m_clip;
    CursorClip m_cursor;
    bool m_enDespacho = false;  // true mientras avanza()/repinta() llaman a los puertos
    int m_generacion = 0;    // +1 por cada cambia() aceptado; detecta reentrada en avanza()
    double m_resto = 0.0;    // segundos no consumidos si se agoto el tope de drenado
    int m_ultimoSprite = 0;  // 0 = nada entregado aun (los sprites son >= 1)
};

}  // namespace Game
