// U3-05.E07a -- ClipActor (ver ClipActor.h).
#include "Game/ClipActor.h"

#include <cstddef>

namespace Game {

ClipActor::ClipActor(IdClip id, ClipAnimacion clip) : m_id(std::move(id)), m_clip(std::move(clip)) {}

int ClipActor::avanza(double dt, PuertosEventoClip& eventos, DestinoFotograma& destino) {
    int rechazados = 0;
    PasoClip p = avanzaClip(m_cursor, dt + m_resto, m_clip);
    m_resto = 0.0;
    for (int i = 0;; ++i) {
        for (const EventoFotograma& e : p.eventos) {
            if (!enrutaEvento(e, eventos).isOk()) ++rechazados;
        }
        m_cursor = p.cursor;
        if (!p.pendiente) break;
        if (i + 1 >= kMaxPasosPorAvance) {
            m_resto = p.tiempoPendiente;
            break;
        }
        p = avanzaClip(m_cursor, p.tiempoPendiente, m_clip);
    }
    const auto f = static_cast<std::size_t>(m_cursor.fotograma);
    if (f < m_clip.fotogramas.size()) {
        const int sprite = m_clip.fotogramas[f].sprite;
        if (sprite != m_ultimoSprite) {
            m_ultimoSprite = sprite;
            destino.ponerFotograma(sprite);
        }
    }
    return rechazados;
}

void ClipActor::cambia(const IdClip& idNuevo, const ClipAnimacion& nuevo, PoliticaCambio politica) {
    if (nuevo.fotogramas.empty()) return;
    const bool mismo = idNuevo == m_id;
    m_cursor = cambiaClip(m_cursor, m_id, idNuevo, nuevo, politica);
    if (politica == PoliticaCambio::ReiniciarSiDistinto && mismo) return;
    m_id = idNuevo;
    m_clip = nuevo;
    m_resto = 0.0;        // el tiempo pendiente era del clip anterior
    m_ultimoSprite = 0;   // la entidad puede mostrar otro sprite: el proximo avanza lo reentrega
}

void ClipActor::repinta(PuertosEventoClip& eventos) const {
    for (const EventoFotograma* e : eventosDeFotograma(m_clip, m_cursor.fotograma)) {
        repintaVisual(*e, eventos);
    }
}

}  // namespace Game

