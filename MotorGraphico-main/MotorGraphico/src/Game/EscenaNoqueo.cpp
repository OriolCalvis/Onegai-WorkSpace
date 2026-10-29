// AV13-33 — La escena de noqueo o rescate antes de «Has caído» (Game/EscenaNoqueo.h).
#include "Game/EscenaNoqueo.h"

#include <algorithm>
#include <cmath>

namespace Game {

EscenaNoqueo EscenaNoqueo::para(const std::string& rescatador) {
    EscenaNoqueo e;
    e.m_rescate = !rescatador.empty();
    e.m_lineas.push_back("El golpe te tumba. El suelo sube a tu encuentro.");
    if (e.m_rescate) {
        e.m_lineas.push_back(rescatador + " se pone delante y te arrastra lejos de la pelea.");
        e.m_lineas.push_back("Su voz te llama desde muy lejos. Después, nada.");
    } else {
        e.m_lineas.push_back("Oyes pasos que se alejan. Después, nada.");
    }
    return e;
}

float EscenaNoqueo::duracion() const {
    return kCaida + kLinea * static_cast<float>(m_lineas.size()) + kFundido;
}

int EscenaNoqueo::indiceLinea() const {
    if (m_t < kCaida) return -1;
    return std::min(static_cast<int>(m_lineas.size()), static_cast<int>((m_t - kCaida) / kLinea));
}

void EscenaNoqueo::salta() {
    const int i = indiceLinea();
    if (i >= static_cast<int>(m_lineas.size())) {
        m_t = duracion();
        return;
    }
    m_t = kCaida + kLinea * static_cast<float>(i + 1);  // durante la caida (i = -1), a la primera
}

float EscenaNoqueo::caida() const { return std::clamp(m_t / kCaida, 0.0f, 1.0f); }

float EscenaNoqueo::velo() const {
    if (m_t < kCaida) return 0.55f * m_t / kCaida;
    const float lineas = kLinea * static_cast<float>(m_lineas.size());
    if (m_t < kCaida + lineas) return 0.55f + 0.25f * (m_t - kCaida) / lineas;
    return std::min(1.0f, 0.8f + 0.2f * (m_t - kCaida - lineas) / kFundido);
}

std::string EscenaNoqueo::linea() const {
    const int i = indiceLinea();
    return i >= 0 && i < static_cast<int>(m_lineas.size()) ? m_lineas[static_cast<std::size_t>(i)] : std::string();
}

float EscenaNoqueo::alfaLinea() const {
    const int i = indiceLinea();
    if (i < 0 || i >= static_cast<int>(m_lineas.size())) return 0.0f;
    const float dentro = m_t - kCaida - kLinea * static_cast<float>(i);
    return std::clamp(std::min(dentro, kLinea - dentro) / 0.3f, 0.0f, 1.0f);
}

}  // namespace Game
