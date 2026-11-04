// U4-07.E05 — Emisor, presupuesto global y avanzarEmisor (ver include/Game/Emisor/Emisor.h).
#include <algorithm>
#include <cmath>

#include "Game/Emisor/Emisor.h"
#include "Game/Emisor/MovimientoParticula.h"

namespace Game::Emisor {

namespace {
constexpr double kTolDuracion = 1e-9;   // mismo patron que kTolEmision: 9 ticks de 1/60 deben cruzar 0,15

std::size_t aforoDe(const DefinicionEmisor& d) {
    return static_cast<std::size_t>(std::clamp(d.maximo, 0, kMaxPorEmisor));
}
}  // namespace

Emisor::Emisor(const DefinicionEmisor& d, Vector2 o, GrupoDeDibujo g)
    : Emisor(d, tipoDeParticula(d.familia), o, g) {}

Emisor::Emisor(const DefinicionEmisor& d, const TipoDeParticula& tipo, Vector2 o, GrupoDeDibujo g)
    : origen(o), grupo(g), m_def(d), m_tipo(tipo), m_azar(crearAzarEmisor(d.semilla)), m_buffer(aforoDe(d)) {}

bool Emisor::acabado() const {
    return m_def.duracion > 0.0 && m_tiempo >= m_def.duracion - kTolDuracion && m_buffer.vivas() == 0;
}

bool PresupuestoParticulas::reserva() {
    if (vivas >= maximo) return false;
    ++vivas;
    return true;
}

void PresupuestoParticulas::libera(std::size_t n) { vivas -= std::min(n, vivas); }

void PresupuestoParticulas::recuenta(const std::vector<Emisor>& e) {
    vivas = 0;
    for (const auto& em : e) vivas += em.buffer().vivas();
}

Result<PasoParticulas> avanzarEmisor(Emisor& e, double dt, double escalaTasa, PresupuestoParticulas& p) {
    using R = Result<PasoParticulas>;
    if (!std::isfinite(dt) || dt < 0.0) return R::Error("avanzarEmisor: dt debe ser finito y >= 0");
    if (!std::isfinite(escalaTasa) || escalaTasa < 0.0) return R::Error("avanzarEmisor: escalaTasa debe ser finito y >= 0");
    const DefinicionEmisor& d = e.m_def;
    const double tasa = d.tasa * escalaTasa;
    if (tasa > kMaxTasa) return R::Error("avanzarEmisor: tasa*escalaTasa supera kMaxTasa");

    PasoParticulas paso;
    if (dt == 0.0) return R::Ok(paso);   // pausa (D-O6): nada cambia
    paso.recortado = dt > kMaxPasoEmision;
    dt = std::min(dt, kMaxPasoEmision);

    // (1) nunca se emite mas alla de `duracion`
    double dtEmision = dt;
    if (d.duracion > 0.0) {
        const double resta = d.duracion - e.m_tiempo;
        dtEmision = resta <= kTolDuracion ? 0.0 : std::min(dt, resta);
    }
    // Pura (no muta): si falla, el emisor sigue intacto.
    const auto emi = emisiones(e.m_estado, dtEmision, tasa);
    if (!emi.isOk()) return R::Error(emi.errorMessage());

    // (2) tiempo; (3) envejecer, recolocar en forma cerrada y compactar (libera ANTES de nacer)
    e.m_tiempo += dt;
    const float dtf = static_cast<float>(dt);
    for (auto& q : e.m_buffer.datos()) {
        q.edad += dtf;
        if (q.edad < q.vida) colocaParticula(e.m_tipo, q);
    }
    const std::size_t caducadas = e.m_buffer.compacta();
    p.libera(caducadas);
    paso.caducadas = static_cast<int>(caducadas);

    // (4) corte: vaciar y no nacer nada
    if (d.cortar && d.duracion > 0.0 && e.m_tiempo >= d.duracion - kTolDuracion) {
        const std::size_t vaciadas = e.m_buffer.vacia();
        p.libera(vaciadas);
        paso.caducadas += static_cast<int>(vaciadas);
        return R::Ok(paso);
    }

    // (5) nacer en orden: reserva ANTES de sortear
    e.m_estado = emi.value().estado;
    for (int i = 0; i < emi.value().cuantas; ++i) {
        if (e.m_buffer.llena() || !p.reserva()) {
            ++paso.rechazadas;
            continue;
        }
        Particula q = crearParticula(d, e.m_tipo, *e.m_azar);
        q.k = static_cast<std::uint16_t>(e.m_nacidas % kModuloK);
        colocaParticula(e.m_tipo, q);
        e.m_buffer.anade(q);
        ++e.m_nacidas;
        ++paso.nacidas;
    }
    paso.recortado = paso.recortado || emi.value().recortado;
    return R::Ok(paso);
}

std::size_t podaAcabados(std::vector<Emisor>& emisores, PresupuestoParticulas& p) {
    const std::size_t antes = emisores.size();
    emisores.erase(std::remove_if(emisores.begin(), emisores.end(), [](const Emisor& e) { return e.acabado(); }),
                   emisores.end());
    p.recuenta(emisores);
    return antes - emisores.size();
}

void olvida(std::vector<Emisor>& emisores, PresupuestoParticulas& p) {
    emisores.clear();
    p.recuenta(emisores);
}

}  // namespace Game::Emisor
