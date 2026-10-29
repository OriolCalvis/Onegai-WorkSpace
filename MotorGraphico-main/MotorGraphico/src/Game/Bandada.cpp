// BLOQUES-02 — ver include/Game/Bandada.h.
#include "Game/Bandada.h"

#include <algorithm>
#include <cmath>

namespace Game {

std::uint32_t Bandada::siguiente() {
    // xorshift32: barato, determinista y suficiente para unos pajaros.
    std::uint32_t x = m_estado;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    m_estado = x;
    return x;
}

void Bandada::prepara(const std::vector<Azotea>& azoteas, std::uint32_t semilla, int cuantos, int especies) {
    m_azoteas = azoteas;
    m_pajaros.clear();
    m_estado = semilla == 0 ? 1u : semilla;
    if (m_azoteas.empty() || cuantos <= 0) return;
    for (int i = 0; i < cuantos; ++i) {
        Pajaro p;
        p.destino = static_cast<int>(siguiente() % m_azoteas.size());
        const Azotea& a = m_azoteas[static_cast<std::size_t>(p.destino)];
        p.x = static_cast<float>(a.celda.x) + a.dx;
        p.y = static_cast<float>(a.celda.y) + a.dy;
        p.altura = alturaAzotea(a);
        p.espera = 3.0f + 9.0f * azar();
        p.especie = especies > 1 ? static_cast<int>(siguiente() % static_cast<std::uint32_t>(especies)) : 0;
        p.mirandoDerecha = (siguiente() & 1u) != 0;
        m_pajaros.push_back(p);
    }
}

void Bandada::despega(Pajaro& p) {
    // Otra azotea, lejos: un vuelo de verdad, no un salto de tejado en tejado.
    int mejor = p.destino;
    float mejorDist = 0.0f;
    for (int intento = 0; intento < 6; ++intento) {
        const int k = static_cast<int>(siguiente() % m_azoteas.size());
        const Azotea& a = m_azoteas[static_cast<std::size_t>(k)];
        const float dx = static_cast<float>(a.celda.x) + a.dx - p.x, dy = static_cast<float>(a.celda.y) + a.dy - p.y;
        const float d = dx * dx + dy * dy;
        if (d > mejorDist) {
            mejorDist = d;
            mejor = k;
        }
    }
    p.destino = mejor;
    p.x0 = p.x;
    p.y0 = p.y;
    p.altura0 = p.altura;
    p.volando = true;
    p.reloj = 0.0f;
    const Azotea& a = m_azoteas[static_cast<std::size_t>(mejor)];
    // En pantalla, x crece hacia la derecha y y hacia la izquierda.
    p.mirandoDerecha = (static_cast<float>(a.celda.x) - p.x) - (static_cast<float>(a.celda.y) - p.y) >= 0.0f;
}

void Bandada::avanza(float dt, const GridCoord& jugador) {
    if (m_pajaros.empty() || dt <= 0.0f) return;
    for (Pajaro& p : m_pajaros) {
        p.reloj += dt;
        if (!p.volando) {
            const float dx = static_cast<float>(jugador.x) + 0.5f - p.x, dy = static_cast<float>(jugador.y) + 0.5f - p.y;
            if (p.reloj >= p.espera || dx * dx + dy * dy < kDistanciaSusto * kDistanciaSusto) despega(p);
            continue;
        }
        const Azotea& a = m_azoteas[static_cast<std::size_t>(p.destino)];
        const float tx = static_cast<float>(a.celda.x) + a.dx, ty = static_cast<float>(a.celda.y) + a.dy;
        const float total = std::max(0.001f, std::hypot(tx - p.x0, ty - p.y0));
        const float recorrido = std::min(total, kVelocidad * p.reloj);
        const float t = recorrido / total;
        p.x = p.x0 + (tx - p.x0) * t;
        p.y = p.y0 + (ty - p.y0) * t;
        // Sube en el primer cuarto, planea, y baja en el ultimo cuarto.
        const float alturaFinal = alturaAzotea(a);
        if (t < 0.25f) p.altura = p.altura0 + (kAlturaVuelo - p.altura0) * (t / 0.25f);
        else if (t > 0.75f) p.altura = kAlturaVuelo + (alturaFinal - kAlturaVuelo) * ((t - 0.75f) / 0.25f);
        else p.altura = kAlturaVuelo;
        if (t >= 1.0f) {
            p.volando = false;
            p.reloj = 0.0f;
            p.altura = alturaFinal;
            p.espera = 3.0f + 9.0f * azar();
        }
    }
}

int Bandada::frameDe(const Pajaro& p) {
    if (!p.volando) return 0;
    return (static_cast<int>(p.reloj / 0.12f) & 1) == 0 ? 1 : 2;
}

}  // namespace Game
