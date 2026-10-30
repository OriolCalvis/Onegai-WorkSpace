// AV14-02 — ver include/Game/EfectosDeGolpe.h.
#include "Game/EfectosDeGolpe.h"

#include <algorithm>
#include <cmath>

namespace Game {

namespace {

using Grado = GolpeCombate::Grado;

// Cuanto dura y cuanto se mueve cada grado (pixeles del mundo).
float vidaDe(Grado g) {
    switch (g) {
        case Grado::Critico: return 0.45f;
        case Grado::Cura: return 0.5f;
        case Grado::Fallo:
        case Grado::Pifia: return 0.28f;
        default: return 0.3f;
    }
}

float empujeDe(Grado g) {
    switch (g) {
        case Grado::Critico: return 11.0f;
        case Grado::Acierto: return 6.0f;
        case Grado::Roce: return 3.0f;
        case Grado::Periodico: return 2.0f;
        case Grado::Fallo:
        case Grado::Pifia: return 7.0f;  // de lado: se aparta
        case Grado::Cura: return 0.0f;
    }
    return 0.0f;
}

bool parpadea(Grado g) { return g != Grado::Fallo && g != Grado::Pifia && g != Grado::Cura; }

}  // namespace

void EfectosDeGolpe::golpe(const GolpeCombate& g, Vector2 direccion, float fuerzaHoja) {
    const float largo = std::sqrt(direccion.x * direccion.x + direccion.y * direccion.y);
    direccion = largo > 0.001f ? Vector2{direccion.x / largo, direccion.y / largo} : Vector2{0.0f, 1.0f};
    // El que esquiva se aparta de lado (perpendicular al golpe), no hacia atras.
    if (g.grado == Grado::Fallo || g.grado == Grado::Pifia) direccion = Vector2{-direccion.y, direccion.x};
    // Un golpe nuevo sobre el mismo sustituye al anterior (no se suman los empujes).
    m_efectos.erase(std::remove_if(m_efectos.begin(), m_efectos.end(),
                                   [&](const Efecto& e) { return e.aliado == g.aAliado && e.i == g.i; }),
                    m_efectos.end());
    m_efectos.push_back(Efecto{g.aAliado, g.i, g.grado, direccion, 0.0f, vidaDe(g.grado)});
    if (g.grado == Grado::Critico) m_pausa = std::max(m_pausa, kPausaCritico);
    if (g.aAliado && g.i == 0 && g.cambio < 0) m_hoja = std::max(m_hoja, std::clamp(fuerzaHoja, 0.35f, 1.0f));
}

void EfectosDeGolpe::avanza(float segundos) {
    m_reloj += segundos;
    for (Efecto& e : m_efectos) e.edad += segundos;
    m_efectos.erase(std::remove_if(m_efectos.begin(), m_efectos.end(), [](const Efecto& e) { return e.edad >= e.vida; }),
                    m_efectos.end());
    m_hoja = std::max(0.0f, m_hoja - segundos / kHoja);
}

void EfectosDeGolpe::olvida() {
    m_efectos.clear();
    m_pausa = m_hoja = 0.0f;
}

bool EfectosDeGolpe::pausaMundo(float segundos) {
    if (m_pausa <= 0.0f) return false;
    m_pausa = std::max(0.0f, m_pausa - segundos);
    return true;
}

const EfectosDeGolpe::Efecto* EfectosDeGolpe::de(bool aliado, std::size_t i) const {
    for (const Efecto& e : m_efectos)
        if (e.aliado == aliado && e.i == i) return &e;
    return nullptr;
}

bool EfectosDeGolpe::activo(bool aliado, std::size_t i) const { return de(aliado, i) != nullptr; }

Vector4 EfectosDeGolpe::tinte(bool aliado, std::size_t i) const {
    const Efecto* e = de(aliado, i);
    if (e == nullptr) return Vector4{1.0f, 1.0f, 1.0f, 1.0f};
    const float t = e->edad / e->vida;
    if (e->grado == Grado::Cura) {  // un pulso verde que se va
        const float v = std::sin(t * 3.14159f);
        return Vector4{1.0f - 0.35f * v, 1.0f, 1.0f - 0.35f * v, 1.0f};
    }
    if (!parpadea(e->grado)) return Vector4{1.0f, 1.0f, 1.0f, 1.0f};
    // Parpadeo carmin: encendido y apagado cada 60 ms, cada vez mas flojo.
    const bool encendido = static_cast<int>(e->edad / 0.06f) % 2 == 0;
    const float f = encendido ? 1.0f - t * 0.5f : 0.0f;
    return Vector4{1.0f, 1.0f - 0.68f * f, 1.0f - 0.72f * f, 1.0f};
}

Vector2 EfectosDeGolpe::desplazamiento(bool aliado, std::size_t i) const {
    const Efecto* e = de(aliado, i);
    if (e == nullptr) return Vector2{};
    // Sale deprisa (el primer cuarto) y vuelve despacio.
    const float t = std::clamp(e->edad / e->vida, 0.0f, 1.0f);
    const float curva = t < 0.25f ? t / 0.25f : 1.0f - (t - 0.25f) / 0.75f;
    const float d = empujeDe(e->grado) * curva * curva * (3.0f - 2.0f * curva);
    return Vector2{std::round(e->direccion.x * d), std::round(e->direccion.y * d * 0.6f)};
}

Vector2 EfectosDeGolpe::temblorHoja() const {
    if (m_hoja <= 0.0f) return Vector2{};
    // Un temblor que se apaga con la hoja: rapido al principio, nada al final.
    const float a = 6.0f * m_hoja * m_hoja;
    return Vector2{std::round(std::sin(m_reloj * 61.0f) * a), std::round(std::cos(m_reloj * 47.0f) * a * 0.5f)};
}

}  // namespace Game
