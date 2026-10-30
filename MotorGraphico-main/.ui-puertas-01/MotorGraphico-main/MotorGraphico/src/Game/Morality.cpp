#include "Game/Morality.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <map>
#include <utility>

namespace moral {

Tramo tramoDe(int valor) {
    // D-MOR-03 (Neutral de −19 a +19, Respetado hasta +59, Héroe desde +75) y
    // los que Oriol puso el 08/10: Desgraciado en −60 y Villano en −75.
    if (valor <= -75) return Tramo::Villano;
    if (valor <= -60) return Tramo::Desgraciado;
    if (valor <= -20) return Tramo::Canalla;
    if (valor < 20) return Tramo::Neutral;
    if (valor < 60) return Tramo::Respetado;
    if (valor < 75) return Tramo::Salvador;
    return Tramo::Heroe;
}

const char* nombreTramo(Tramo t) {
    switch (t) {
        case Tramo::Villano: return "Villano";
        case Tramo::Desgraciado: return "Desgraciado";
        case Tramo::Canalla: return "Canalla";
        case Tramo::Neutral: return "Neutral";
        case Tramo::Respetado: return "Respetado";
        case Tramo::Salvador: return "Salvador";
        case Tramo::Heroe: return "Héroe";
    }
    return "Neutral";
}

}  // namespace moral

void Morality::anota(const std::string& linea) {
    m_history.insert(m_history.begin(), linea);
    if (m_history.size() > kHistoryLimit) {
        m_history.resize(kHistoryLimit);
    }
}

void Morality::record(int delta, const std::string& reason) {
    if (delta == 0) {
        return;  // accion moralmente neutra: no ensucia el historial
    }
    m_value = std::clamp(m_value + delta, static_cast<double>(kMin), static_cast<double>(kMax));

    // El signo va EN el texto: el HUD lo muestra tal cual y asi se lee
    // "por que" y "cuanto" de un vistazo, sin que quien dibuja tenga que
    // formatear numeros.
    const std::string signo = delta > 0 ? "+" : "";
    anota(signo + std::to_string(delta) + "  " + reason);
}

int Morality::value() const { return static_cast<int>(std::lround(m_value)); }

void Morality::setValue(int value) { m_value = std::clamp(value, kMin, kMax); }

float Morality::fraction() const {
    return static_cast<float>((m_value - kMin) / static_cast<double>(kMax - kMin));
}

const char* Morality::label() const {
    // Simétricos desde MORAL-01: antes hacía falta menos maldad para dejar de
    // ser neutral que bondad para ser un héroe, y Oriol lo quiere al revés de
    // rápido («si nos corrompemos muy rápido no tendría gracia»).
    switch (tramo()) {
        case moral::Tramo::Villano: return "VILLANO";
        case moral::Tramo::Desgraciado: return "DESGRACIADO";
        case moral::Tramo::Canalla: return "CANALLA";
        case moral::Tramo::Neutral: return "NEUTRAL";
        case moral::Tramo::Respetado: return "RESPETADO";
        case moral::Tramo::Salvador: return "SALVADOR";
        case moral::Tramo::Heroe: return "HÉROE";
    }
    return "NEUTRAL";
}

// ---------------------------------------------------------------------
// MORAL-01 — El libro del día
// ---------------------------------------------------------------------

void Morality::apunta(const moral::ActoApuntado& acto) {
    if (acto.signo == 0) return;  // un acto que no pesa en la moral (solo reputación o pureza)
    m_estado.pendientes.push_back(acto);
    anota(std::string(acto.signo > 0 ? "hoy, te honra: " : "hoy, te pesa: ") + acto.motivo);
}

void Morality::aplica(double v, const moral::Parametros& p) {
    if (v == 0.0) return;
    // La resistencia: alejarse del 0 cuesta más cuanto más lejos estás; volver
    // hacia el centro cuesta lo normal.
    const bool seAleja = m_value == 0.0 || (v > 0.0) == (m_value > 0.0);
    if (seAleja && p.resistencia > 0) v *= std::max(0.0, 1.0 - std::fabs(m_value) / p.resistencia);
    m_value = std::clamp(m_value + v, static_cast<double>(kMin), static_cast<double>(kMax));
}

moral::Asiento Morality::asienta(const moral::Parametros& p, int dia) {
    moral::Asiento a;
    a.antes = value();
    a.actos = static_cast<int>(m_estado.pendientes.size());
    const double antes = m_value;

    // Los menores y los medios: los del mismo tipo cuentan menos cada vez
    // (repetición) y la noche entera no mueve más que el tope.
    std::map<std::string, std::size_t> vistos;
    double cotidiano = 0.0;
    std::vector<double> mayores;
    for (const moral::ActoApuntado& acto : m_estado.pendientes) {
        const int g = std::clamp(static_cast<int>(acto.gravedad), 0, 2);
        const double peso = static_cast<double>(p.peso[g]) * (acto.signo > 0 ? 1.0 : -1.0);
        if (acto.gravedad == moral::Gravedad::Mayor) {
            mayores.push_back(peso);
            if (acto.signo < 0 && acto.contraInocente) {
                m_estado.marcas.push_back(moral::Marca{acto.id, acto.motivo, dia});
                a.marcasNuevas.push_back(acto.motivo);
            }
            continue;
        }
        std::size_t& veces = vistos[acto.tipo.empty() ? acto.id : acto.tipo];
        const double rep = p.repeticion.empty() ? 1.0 : p.repeticion[std::min(veces, p.repeticion.size() - 1)];
        ++veces;
        cotidiano += peso * rep;
    }
    const double tope = static_cast<double>(std::max(p.tope, 0));
    aplica(std::clamp(cotidiano, -tope, tope), p);
    // Los mayores, sin tope: son decisiones explícitas, y cada uno con la
    // resistencia de cómo te ha dejado el anterior.
    for (const double v : mayores) aplica(v, p);

    m_estado.pendientes.clear();
    a.delta = m_value - antes;
    a.despues = value();
    if (a.actos > 0) {
        const int redondo = static_cast<int>(std::lround(a.delta));
        anota((redondo > 0 ? "+" : "") + std::to_string(redondo) + "  la noche del día " + std::to_string(dia));
    }
    return a;
}

void Morality::cambiaPureza(int delta, const std::string& reason) {
    if (delta == 0) return;
    m_estado.pureza = std::clamp(m_estado.pureza + delta, 0, kPurezaMax);
    anota("pureza " + std::string(delta > 0 ? "+" : "") + std::to_string(delta) + "  " + reason);
}

void Morality::restauraEstado(moral::Estado e) {
    e.pureza = std::clamp(e.pureza, 0, kPurezaMax);
    m_estado = std::move(e);
}
