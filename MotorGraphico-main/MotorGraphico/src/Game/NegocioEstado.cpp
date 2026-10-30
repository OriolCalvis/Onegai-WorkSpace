// AV15-05a — ver include/Game/NegocioEstado.h.
#include "Game/NegocioEstado.h"

#include <algorithm>

#include "Game/GameSession.h"

namespace Game {

namespace {

std::string deOro(int n) { return std::to_string(n) + " de oro"; }

int alquilerDe(const GameSession& s) {
    for (const OwnedBusiness& b : s.businesses())
        if (b.objectId == s.currentBusinessId()) return b.rentPercent;
    return 100;
}

// Subir o bajar 10: lo que rentaría y la moral que mueve (la regla de
// GameSession::adjustCurrentRent: un punto por cada 5 de alquiler).
FilaNegocio filaAlquiler(const GameSession& s, int delta) {
    FilaNegocio f;
    f.accion = delta > 0 ? AccionNegocio::SubirAlquiler : AccionNegocio::BajarAlquiler;
    f.texto = delta > 0 ? "Subir el alquiler" : "Bajar el alquiler";
    const int ahora = alquilerDe(s);
    const int luego = std::clamp(ahora + delta, GameSession::kMinRent, GameSession::kMaxRent);
    f.valor = std::to_string(luego) + " %";
    if (luego == ahora) {
        f.apagada = true;
        f.motivo = delta > 0 ? "Ya está en lo más alto." : "Ya está en lo más bajo.";
        return f;
    }
    const int renta = GameSession::incomeFor(s.currentBusinessIncome(), luego);
    const int moral = -(luego - ahora) / 5;
    f.motivo = "Te dará " + deOro(renta) + " por ciclo (ocupación " +
               std::to_string(GameSession::occupancyPercent(luego)) + " %); moral " + (moral > 0 ? "+" : "") +
               std::to_string(moral) + ".";
    return f;
}

}  // namespace

std::vector<FilaNegocio> NegocioEstado::filas(const GameSession& s) {
    std::vector<FilaNegocio> out;
    if (s.currentBusinessId().empty()) return out;
    const int precio = s.currentBusinessPrice();
    if (!s.ownsBusiness(s.currentBusinessId())) {
        FilaNegocio comprar;
        comprar.accion = AccionNegocio::Comprar;
        comprar.texto = "Comprar";
        comprar.valor = std::to_string(precio) + " ORO";
        if (s.gold() < precio) {
            comprar.apagada = true;
            comprar.motivo = "Te faltan " + deOro(precio - s.gold()) + ".";
        } else {
            comprar.motivo = "Te dará " + deOro(s.currentBusinessIncome()) + " por ciclo.";
        }
        out.push_back(comprar);
    } else {
        out.push_back(filaAlquiler(s, +10));
        out.push_back(filaAlquiler(s, -10));
        FilaNegocio revender;
        revender.accion = AccionNegocio::Revender;
        revender.texto = "Revender";
        revender.valor = std::to_string(reventa(precio)) + " ORO";
        revender.motivo = "Recibes " + std::to_string(reventa(precio)) + " de los " + std::to_string(precio) +
                          " que pagaste.";
        out.push_back(revender);
    }
    FilaNegocio salir;
    salir.texto = "Salir";
    out.push_back(salir);
    return out;
}

void NegocioEstado::mueve(int delta, std::size_t filas) {
    m_filas = filas;
    if (!m_pregunta.vacia()) {
        if (delta != 0) m_pregunta.mueve();
        return;
    }
    if (filas == 0) return;
    const long n = static_cast<long>(filas);
    m_foco = static_cast<std::size_t>(((static_cast<long>(foco(filas)) + delta) % n + n) % n);
}

void NegocioEstado::moveDown() { mueve(+1, m_filas); }

void NegocioEstado::dice(const std::string& que, const std::string& detalle, bool error) {
    m_estado = que;
    m_detalle = detalle;
    m_error = error;
}

bool NegocioEstado::confirma(GameSession& s) {
    const std::string nombre = s.currentBusinessName();
    const int precio = s.currentBusinessPrice();
    if (!m_pregunta.vacia()) {
        const bool si = m_pregunta.siElegido();
        m_pregunta = Confirmacion{};
        if (!si) return false;
        if (m_pendiente == AccionNegocio::Comprar) {
            if (s.buyCurrentBusiness())
                dice("Has comprado " + nombre + ".", "Te quedan " + deOro(s.gold()) + ".", false);
            else
                dice("No se ha podido comprar.", "Te faltan " + deOro(std::max(0, precio - s.gold())) + ".", true);
        } else if (m_pendiente == AccionNegocio::Revender) {
            const int recibes = s.sellCurrentBusiness(kReventaPct);
            if (recibes >= 0)
                dice("Has revendido " + nombre + ".", "Recibes " + deOro(recibes) + "; ahora tienes " + deOro(s.gold()) + ".",
                     false);
            else
                dice("No se ha podido revender.", "", true);
        }
        m_foco = 0;
        return false;
    }
    const std::vector<FilaNegocio> fs = filas(s);
    if (fs.empty()) return false;
    const FilaNegocio& f = fs[foco(fs.size())];
    if (f.apagada) {
        dice(f.texto + ": no se puede.", f.motivo, true);
        return false;
    }
    switch (f.accion) {
        case AccionNegocio::Comprar:
            m_pregunta = preguntaComprar(nombre, precio, s.gold(), s.currentBusinessIncome());
            m_pendiente = AccionNegocio::Comprar;
            return false;
        case AccionNegocio::Revender:
            m_pregunta = preguntaRevender(nombre, precio, reventa(precio));
            m_pendiente = AccionNegocio::Revender;
            return false;
        case AccionNegocio::SubirAlquiler:
        case AccionNegocio::BajarAlquiler:
            if (s.adjustCurrentRent(f.accion == AccionNegocio::SubirAlquiler ? +10 : -10))
                dice("Alquiler al " + std::to_string(alquilerDe(s)) + " %.",
                     "Te dará " + deOro(GameSession::incomeFor(s.currentBusinessIncome(), alquilerDe(s))) + " por ciclo.",
                     false);
            return false;
        case AccionNegocio::Salir:
            s.closeInteraction();
            *this = NegocioEstado{};
            return true;
    }
    return false;
}

bool NegocioEstado::cancela() {
    if (!m_pregunta.vacia()) {
        m_pregunta = Confirmacion{};
        return true;
    }
    *this = NegocioEstado{};
    return false;
}

}  // namespace Game
