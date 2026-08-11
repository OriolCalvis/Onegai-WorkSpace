#include "Game/Morality.h"

#include <algorithm>
#include <cstddef>

void Morality::record(int delta, const std::string& reason) {
    if (delta == 0) {
        return;  // accion moralmente neutra: no ensucia el historial
    }
    m_value = std::clamp(m_value + delta, kMin, kMax);

    // El signo va EN el texto: el HUD lo muestra tal cual y asi se lee
    // "por que" y "cuanto" de un vistazo, sin que quien dibuja tenga que
    // formatear numeros.
    const std::string signo = delta > 0 ? "+" : "";
    m_history.insert(m_history.begin(), signo + std::to_string(delta) + "  " + reason);
    if (m_history.size() > kHistoryLimit) {
        m_history.resize(kHistoryLimit);
    }
}

void Morality::setValue(int value) { m_value = std::clamp(value, kMin, kMax); }

float Morality::fraction() const {
    return static_cast<float>(m_value - kMin) / static_cast<float>(kMax - kMin);
}

const char* Morality::label() const {
    // Tramos asimetricos a proposito: hace falta MENOS maldad para
    // dejar de ser neutral que bondad para ser un heroe. Es la misma
    // intuicion que tiene la gente sobre la reputacion -- se pierde mas
    // rapido de lo que se gana.
    if (m_value <= -60) {
        return "VILLANO";
    }
    if (m_value <= -20) {
        return "CANALLA";
    }
    if (m_value < 40) {
        return "NEUTRAL";
    }
    if (m_value < 75) {
        return "JUSTO";
    }
    return "HEROE";
}
