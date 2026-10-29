// C2-04c — ver include/Render/FuegoNivel.h.
#include "Render/FuegoNivel.h"

#include <climits>
#include <cmath>

#include "Core/Math/IsoMath.h"
#include "Game/FuegoCeldas.h"
#include "Render/IsometricRenderer.h"
#include "Render/PintaDestellos.h"

namespace Render {

void FuegoNivel::monta(IsometricRenderer& renderer, Texture* blanco, int tileW, int tileH) {
    if (m_renderer != nullptr) desmonta(*m_renderer);
    m_blanco = blanco;
    m_tileW = tileW;
    m_tileH = tileH;
    auto mapa = Game::ParticulasDeCartas::cargar();
    if (mapa.isOk()) m_mapa = mapa.value();
    m_color = m_mapa.colorDe("llamas");
    m_renderer = &renderer;
    renderer.addToQueue(this);
}

void FuegoNivel::desmonta(IsometricRenderer& renderer) {
    renderer.removeFromQueue(this);
    m_renderer = nullptr;
    m_celdas.clear();
}

int FuegoNivel::getSortKey() const { return INT_MAX - 2048; }

void FuegoNivel::render(SpriteBatch& batch) {
    if (m_blanco == nullptr || m_celdas.empty()) return;
    const double reloj = std::chrono::duration<double>(std::chrono::steady_clock::now() - m_inicio).count();
    const float tw = static_cast<float>(m_tileW), th = static_cast<float>(m_tileH);
    const float lado = 64.0f;  // el frame de 64x64 de los destellos
    for (const GridCoord& c : m_celdas) {
        const std::uint32_t semilla = Game::semillaDe(c);
        // Bucle continuo; cada casilla con su fase para que no respiren a la vez.
        const double fase = static_cast<double>(semilla % 1000u) / 1000.0 * Game::kDuracionDeCarta;
        const double t = std::fmod(reloj + fase, Game::kDuracionDeCarta);
        m_destellos.clear();
        Game::destellosDeCarta("llamas", m_color, semilla, t, m_destellos);
        // Los pies del frame en el punto de apoyo de la casilla (como el aura del jugador).
        const Vector2 base = IsoMath::gridToScreen(c, tw, th);
        const Vector2 esquina{base.x + tw * 0.5f - lado * 0.5f, base.y + th * 0.9f - lado * 0.94f};
        pintaDestellos(batch, m_destellos, esquina, Vector2{lado, lado}, m_blanco);
    }
}

}  // namespace Render
