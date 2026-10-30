// PERGAMINO-01 — ver include/Game/PasarHoja.h.
#include "Game/PasarHoja.h"

#include <algorithm>
#include <cmath>

namespace Game {

namespace {
constexpr float kPi = 3.14159265358979f;
constexpr float kPerspectiva = 2800.0f;
constexpr float kOrigenY = 0.2f;  // perspective-origin: 50% 20%

float facilIn(float t) { return 0.25f * t + 0.75f * t * t; }
float facilOut(float t) {
    const float u = t - 1.0f;
    return 1.0f + 1.9f * u * u * u + 0.9f * u * u;
}
}  // namespace

void PasarHoja::empieza(int desde, int hacia, bool reducido) {
    if (m_activa) desde = m_hacia;  // la pulsacion nueva completa la anterior
    m_activa = desde != hacia;
    m_desde = desde;
    m_hacia = hacia;
    m_reducido = reducido;
    m_t = m_activa ? 0.0f : 1.0f;
}

void PasarHoja::avanza(float segundos) {
    if (!m_activa) return;
    const float duracion = m_reducido ? kFundido : kDuracion;
    m_t = std::min(1.0f, m_t + std::max(0.0f, segundos) / duracion);
    if (m_t >= 1.0f) m_activa = false;
}

float PasarHoja::anguloEn(bool haciaDelante, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return haciaDelante ? 180.0f * facilIn(t) : 180.0f * (1.0f - facilOut(t));
}

float PasarHoja::angulo() const {
    if (!m_activa || m_reducido) return 0.0f;
    return anguloEn(haciaDelante(), m_t);
}

float PasarHoja::brilloFrente() const {
    return 1.0f - 0.30f * std::sin(angulo() * kPi / 180.0f);
}

float PasarHoja::brilloDorso() const {
    return 0.80f - 0.22f * std::sin(angulo() * kPi / 180.0f);
}

float PasarHoja::sombraDebajo() const {
    const float a = angulo();
    const float s = std::sin(a * kPi / 180.0f);
    return std::max(0.0f, 0.9f * s * (a <= 90.0f ? 1.0f : (180.0f - a) / 90.0f));
}

std::vector<TiraHoja> PasarHoja::tiras(float x, float y, float ancho, float alto,
                                       float anguloGrados, int numTiras, float recorteY) {
    std::vector<TiraHoja> salida;
    if (numTiras <= 0 || ancho <= 0.0f || alto <= 0.0f) return salida;
    const float a = anguloGrados * kPi / 180.0f;
    const float bisagra = y + alto;
    const float ox = x + ancho * 0.5f;
    const float oy = y + alto * kOrigenY;
    // d = distancia a la bisagra sobre la hoja (0 en la bisagra, alto arriba).
    auto proyecta = [&](float px, float d) {
        const float y3 = bisagra - d * std::cos(a);
        const float z3 = d * std::sin(a);  // hacia el jugador
        const float s = kPerspectiva / std::max(1.0f, kPerspectiva - z3);
        return Vector2{ox + (px - ox) * s, oy + (y3 - oy) * s};
    };
    const bool dorso = anguloGrados > 90.0f;
    for (int k = 0; k < numTiras; ++k) {
        const float d0 = alto * static_cast<float>(k) / numTiras;       // lado de la bisagra
        const float d1 = alto * static_cast<float>(k + 1) / numTiras;   // lado lejano
        // Orden de TEXTURA: la cara de delante tiene su arriba lejos de la
        // bisagra; el dorso, girado 180 en su propio plano, junto a ella.
        const float dArriba = dorso ? d0 : d1;
        const float dAbajo = dorso ? d1 : d0;
        TiraHoja t;
        t.p[0] = proyecta(x, dArriba);
        t.p[1] = proyecta(x + ancho, dArriba);
        t.p[2] = proyecta(x + ancho, dAbajo);
        t.p[3] = proyecta(x, dAbajo);
        t.v0 = dorso ? dArriba / alto : 1.0f - dArriba / alto;
        t.v1 = dorso ? dAbajo / alto : 1.0f - dAbajo / alto;
        // Recorte contra el borde de las tapas: se interpola sobre cada lado.
        const float ymax = std::max(std::max(t.p[0].y, t.p[1].y), std::max(t.p[2].y, t.p[3].y));
        const float ymin = std::min(std::min(t.p[0].y, t.p[1].y), std::min(t.p[2].y, t.p[3].y));
        if (ymin >= recorteY) continue;
        if (ymax > recorteY) {
            // Los pares (0,3) y (1,2) son los lados; el que queda por debajo
            // del corte se lleva al corte, con su v interpolada.
            for (int lado = 0; lado < 2; ++lado) {
                Vector2& arriba = t.p[lado == 0 ? 0 : 1];
                Vector2& abajo = t.p[lado == 0 ? 3 : 2];
                const float va = t.v0, vb = t.v1;
                if (abajo.y > recorteY && arriba.y < recorteY) {
                    const float f = (recorteY - arriba.y) / (abajo.y - arriba.y);
                    abajo = Vector2{arriba.x + (abajo.x - arriba.x) * f, recorteY};
                    if (lado == 0) t.v1 = va + (vb - va) * f;
                } else if (arriba.y > recorteY && abajo.y < recorteY) {
                    const float f = (recorteY - abajo.y) / (arriba.y - abajo.y);
                    arriba = Vector2{abajo.x + (arriba.x - abajo.x) * f, recorteY};
                    if (lado == 0) t.v0 = vb + (va - vb) * f;
                }
            }
        }
        salida.push_back(t);
    }
    return salida;
}

}  // namespace Game
