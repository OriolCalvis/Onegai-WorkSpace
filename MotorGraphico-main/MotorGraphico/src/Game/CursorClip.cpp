// U3-05.E04 -- Avanzar el cursor de un clip (ver CursorClip.h).
#include "Game/CursorClip.h"

#include <cmath>

namespace Game {
namespace {

// Clip sin eventos: solo aritmetica (modulo sobre la duracion total), sin recorrer vueltas.
void avanzaSinEventos(CursorClip& k, double restoMs, const ClipAnimacion& clip) {
    double pos = k.enFotogramaMs + restoMs;
    for (int i = 0; i < k.fotograma; ++i) pos += clip.fotogramas[static_cast<std::size_t>(i)].duracionMs;
    const double total = clip.duracionTotal();
    if (!clip.bucle && pos >= total) {
        k.fotograma = static_cast<int>(clip.fotogramas.size()) - 1;
        k.enFotogramaMs = clip.fotogramas.back().duracionMs;
        k.terminado = true;
        return;
    }
    const double vueltas = std::floor(pos / total);
    k.vueltas += static_cast<int>(std::fmin(vueltas, 2147483647.0 - k.vueltas));
    pos = std::fmod(pos, total);
    k.fotograma = 0;
    while (pos >= clip.fotogramas[static_cast<std::size_t>(k.fotograma)].duracionMs) {
        pos -= clip.fotogramas[static_cast<std::size_t>(k.fotograma)].duracionMs;
        ++k.fotograma;
    }
    k.enFotogramaMs = pos;
}

// Entrada en un fotograma: sus eventos van enteros o no van. false = no caben en el presupuesto.
bool emite(PasoClip& p, const ClipAnimacion& clip, int fotograma, std::size_t max) {
    const auto evs = eventosDeFotograma(clip, fotograma);
    if (evs.empty()) return true;
    if (max == 0 || (!p.eventos.empty() && p.eventos.size() + evs.size() > max)) return false;
    for (const EventoFotograma* e : evs) p.eventos.push_back(*e);
    return true;
}

}  // namespace

PasoClip avanzaClip(const CursorClip& cursor, double dt, const ClipAnimacion& clip, std::size_t max) {
    PasoClip p;
    p.cursor = cursor;
    CursorClip& k = p.cursor;
    if (!std::isfinite(dt) || dt < 0.0 || clip.fotogramas.empty() || k.terminado) return p;
    double resto = dt * 1000.0;
    if (!std::isfinite(resto)) return p;  // dt finito pero enorme: dt*1000 desborda; se rechaza como dt invalido
    if (!k.iniciado) {
        if (!emite(p, clip, 0, max)) {
            p.tiempoPendiente = dt;
            return p;
        }
        k.iniciado = true;
        if (max > 0 && p.eventos.size() >= max) {
            p.tiempoPendiente = resto / 1000.0;
            return p;
        }
    }
    if (clip.eventos.empty()) {
        avanzaSinEventos(k, resto, clip);
        return p;
    }
    const int n = static_cast<int>(clip.fotogramas.size());
    for (;;) {
        const double dur = clip.fotogramas[static_cast<std::size_t>(k.fotograma)].duracionMs;
        const double libre = dur - k.enFotogramaMs;
        if (resto < libre) {
            k.enFotogramaMs += resto;
            break;
        }
        const bool ultimo = k.fotograma + 1 == n;
        if (ultimo && !clip.bucle) {
            k.enFotogramaMs = dur;
            k.terminado = true;
            break;
        }
        resto -= libre;
        k.enFotogramaMs = dur;
        const int siguiente = ultimo ? 0 : k.fotograma + 1;
        if (!emite(p, clip, siguiente, max)) {
            p.tiempoPendiente = resto / 1000.0;
            return p;
        }
        k.fotograma = siguiente;
        k.enFotogramaMs = 0.0;
        if (ultimo) ++k.vueltas;
        if (p.eventos.size() >= max && resto > 0.0) {
            p.tiempoPendiente = resto / 1000.0;
            return p;
        }
    }
    return p;
}

}  // namespace Game
