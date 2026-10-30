// U3-05.E07a -- ClipActor (ver ClipActor.h).
#include "Game/ClipActor.h"

#include <cstddef>
#include <vector>

namespace Game {

namespace {
// Marca «despachando» mientras vive; se limpia aunque un puerto lance.
struct GuardaDespacho {
    explicit GuardaDespacho(bool& b) : m_b(b) { m_b = true; }
    ~GuardaDespacho() { m_b = false; }
    GuardaDespacho(const GuardaDespacho&) = delete;
    GuardaDespacho& operator=(const GuardaDespacho&) = delete;
    bool& m_b;
};
}  // namespace

ClipActor::ClipActor(IdClip id, ClipAnimacion clip) : m_id(std::move(id)), m_clip(std::move(clip)) {}

int ClipActor::avanza(double dt, PuertosEventoClip& eventos, DestinoFotograma& destino) {
    if (m_enDespacho) return 0;  // avanza() anidado desde un puerto: sin efectos
    const GuardaDespacho guarda(m_enDespacho);
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

void ClipActor::repinta(PuertosEventoClip& eventos) {
    if (m_enDespacho) return;  // anidado: sin efectos
    const GuardaDespacho guarda(m_enDespacho);
    const int generacion = m_generacion;
    // Copia por valor: un puerto que llame a cambia() reemplaza m_clip y dejaria punteros colgantes.
    std::vector<EventoFotograma> lote;
    for (const EventoFotograma* e : eventosDeFotograma(m_clip, m_cursor.fotograma)) lote.push_back(*e);
    for (const EventoFotograma& e : lote) {
        repintaVisual(e, eventos);
        if (m_generacion != generacion) break;
    }
}

}  // namespace Game
