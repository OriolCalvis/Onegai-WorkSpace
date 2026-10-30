// U3-05.E07a -- ClipActor (ver ClipActor.h).
#include "Game/ClipActor.h"

#include <cstddef>

namespace Game {

ClipActor::ClipActor(IdClip id, ClipAnimacion clip) : m_id(std::move(id)), m_clip(std::move(clip)) {}

int ClipActor::avanza(double dt, PuertosEventoClip& eventos, DestinoFotograma& destino) {
    int rechazados = 0;
    const int generacion = m_generacion;
    PasoClip p = avanzaClip(m_cursor, dt + m_resto, m_clip);
    m_resto = 0.0;
    bool reentrada = false;  // un puerto llamo a cambia() sobre este actor durante el despacho
    for (int i = 0; !reentrada; ++i) {
        for (const EventoFotograma& e : p.eventos) {
            if (!enrutaEvento(e, eventos).isOk()) ++rechazados;
            if (m_generacion != generacion) {
                reentrada = true;  // el resto del lote es del clip viejo: no se emite
                break;
            }
        }
        if (reentrada) break;
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
    if (politica == PoliticaCambio::ReiniciarSiDistinto && idNuevo == m_id) return;
    m_cursor = cambiaClip(m_cursor, m_id, idNuevo, nuevo, politica);
    m_id = idNuevo;
    m_clip = nuevo;
    m_resto = 0.0;        // el tiempo pendiente era del clip anterior
    m_ultimoSprite = 0;   // la entidad puede mostrar otro sprite: el proximo avanza lo reentrega
    ++m_generacion;       // avisa a un avanza() en curso (reentrada desde un puerto)
}

void ClipActor::repinta(PuertosEventoClip& eventos) const {
    for (const EventoFotograma* e : eventosDeFotograma(m_clip, m_cursor.fotograma)) {
        repintaVisual(*e, eventos);
    }
}

}  // namespace Game

