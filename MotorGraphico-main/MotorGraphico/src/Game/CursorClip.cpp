// U3-05.E04 -- Avanzar el cursor de un clip (ver CursorClip.h).
#include "Game/CursorClip.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <utility>

namespace Game {
namespace {

// Tolerancia de frontera (ms): falta menos que esto = se cruza. Absorbe la deriva de sumar dt,
// pero introduce un sesgo de hasta 1e-6 ms por frontera cruzada (puede adelantar el cursor en
// casos alineados de forma adversarial). No se corrige: se documenta.
constexpr double kTolFronteraMs = 1e-6;

// Entrada valida: ver dominio en CursorClip.h.
bool enDominio(const CursorClip& k, double dt, const ClipAnimacion& clip, std::size_t max) {
    if (max == 0 || clip.fotogramas.empty() || !std::isfinite(dt) || dt < 0.0) return false;
    if (!std::isfinite(dt * 1000.0)) return false;
    const int n = static_cast<int>(clip.fotogramas.size());
    for (const FotogramaClip& f : clip.fotogramas)
        if (f.duracionMs <= 0) return false;
    for (const EventoFotograma& e : clip.eventos)
        if (e.fotograma < 0 || e.fotograma >= n) return false;
    if (k.fotograma < 0 || k.fotograma >= n || k.vueltas < 0) return false;
    const double dur = clip.fotogramas[static_cast<std::size_t>(k.fotograma)].duracionMs;
    if (!std::isfinite(k.enFotogramaMs) || k.enFotogramaMs < 0.0 || k.enFotogramaMs > dur) return false;
    if (k.eventoPendiente < 0) return false;
    if (k.eventoPendiente > 0) {
        const std::size_t cuantos = eventosDeFotograma(clip, k.fotograma).size();
        if (!k.iniciado || static_cast<std::size_t>(k.eventoPendiente) >= cuantos) return false;
    }
    return true;
}

// Clip sin eventos: solo aritmetica (modulo sobre la duracion total), sin recorrer vueltas.
void avanzaSinEventos(CursorClip& k, double restoMs, const ClipAnimacion& clip) {
    double pos = k.enFotogramaMs + restoMs;
    for (int i = 0; i < k.fotograma; ++i) pos += clip.fotogramas[static_cast<std::size_t>(i)].duracionMs;
    const double total = clip.duracionTotal();
    const int n = static_cast<int>(clip.fotogramas.size());
    if (!clip.bucle && pos + kTolFronteraMs >= total) {
        k.fotograma = n - 1;
        k.enFotogramaMs = clip.fotogramas.back().duracionMs;
        k.terminado = true;
        return;
    }
    const double desplazada = pos + kTolFronteraMs;
    const double vueltas = std::floor(desplazada / total);
    k.vueltas += static_cast<int>(std::fmin(vueltas, 2147483647.0 - k.vueltas));
    pos = std::fmax(0.0, std::fmod(desplazada, total) - kTolFronteraMs);
    k.fotograma = 0;
    while (k.fotograma + 1 < n &&
           pos + kTolFronteraMs >= clip.fotogramas[static_cast<std::size_t>(k.fotograma)].duracionMs) {
        pos -= clip.fotogramas[static_cast<std::size_t>(k.fotograma)].duracionMs;
        ++k.fotograma;
    }
    k.enFotogramaMs = std::fmax(0.0, pos);
}

// Emite los eventos del fotograma actual desde eventoPendiente, hasta max. true = quedan por emitir.
bool volcar(PasoClip& p, const std::vector<const EventoFotograma*>& evs, std::size_t max) {
    std::size_t i = static_cast<std::size_t>(p.cursor.eventoPendiente);
    while (i < evs.size() && p.eventos.size() < max) p.eventos.push_back(*evs[i++]);
    const bool quedan = i < evs.size();
    p.cursor.eventoPendiente = quedan ? static_cast<int>(i) : 0;
    return quedan;
}

PasoClip corta(PasoClip& p, double restoMs) {
    p.pendiente = true;
    p.tiempoPendiente = restoMs / 1000.0;
    return std::move(p);
}

}  // namespace

PasoClip avanzaClip(const CursorClip& cursor, double dt, const ClipAnimacion& clip, std::size_t max) {
    PasoClip p;
    p.cursor = cursor;
    CursorClip& k = p.cursor;
    if (!enDominio(cursor, dt, clip, max) || k.terminado) return p;
    double resto = dt * 1000.0;
    if (!k.iniciado || k.eventoPendiente > 0) {
        k.iniciado = true;
        if (volcar(p, eventosDeFotograma(clip, k.fotograma), max)) return corta(p, resto);
        if (p.eventos.size() >= max && resto > 0.0) return corta(p, resto);
    }
    if (clip.eventos.empty()) {
        avanzaSinEventos(k, resto, clip);
        return p;
    }
    const int n = static_cast<int>(clip.fotogramas.size());
    for (;;) {
        const double dur = clip.fotogramas[static_cast<std::size_t>(k.fotograma)].duracionMs;
        const double libre = dur - k.enFotogramaMs;
        if (resto < libre - kTolFronteraMs) {
            k.enFotogramaMs += resto;
            break;
        }
        const bool ultimo = k.fotograma + 1 == n;
        if (ultimo && !clip.bucle) {
            k.enFotogramaMs = dur;
            k.terminado = true;
            break;
        }
        resto = std::fmax(0.0, resto - libre);
        k.enFotogramaMs = dur;
        const int siguiente = ultimo ? 0 : k.fotograma + 1;
        const auto evs = eventosDeFotograma(clip, siguiente);
        if (!evs.empty() && p.eventos.size() >= max) return corta(p, resto);
        k.fotograma = siguiente;
        k.enFotogramaMs = 0.0;
        k.eventoPendiente = 0;
        if (ultimo && k.vueltas < INT_MAX) ++k.vueltas;
        if (volcar(p, evs, max)) return corta(p, resto);
        if (p.eventos.size() >= max && resto > 0.0) return corta(p, resto);
    }
    return p;
}

CursorClip cambiaClip(const CursorClip& actual, const IdClip& idActual, const IdClip& idNuevo,
                      const ClipAnimacion& nuevo, PoliticaCambio politica) {
    if (nuevo.fotogramas.empty()) return actual;
    const bool reinicia = politica == PoliticaCambio::ReiniciarSiempre ||
                          (politica == PoliticaCambio::ReiniciarSiDistinto && idNuevo != idActual);
    if (reinicia) return CursorClip{};
    if (politica == PoliticaCambio::ReiniciarSiDistinto) return actual;
    // ConservarFase: el pendiente del clip anterior se descarta; la posicion se acota al nuevo.
    CursorClip k = actual;
    const int ultimo = static_cast<int>(nuevo.fotogramas.size()) - 1;
    k.fotograma = std::clamp(actual.fotograma, 0, ultimo);
    const double dur = nuevo.fotogramas[static_cast<std::size_t>(k.fotograma)].duracionMs;
    k.enFotogramaMs = std::clamp(actual.enFotogramaMs, 0.0, dur);
    k.eventoPendiente = 0;
    k.terminado = actual.terminado && !nuevo.bucle && k.fotograma == ultimo;
    if (k.terminado) k.enFotogramaMs = dur;
    return k;
}

}  // namespace Game
