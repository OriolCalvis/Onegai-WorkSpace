// FICHA-01 — ver include/Render/HudIconos.h.
#include "Render/HudIconos.h"

#include <algorithm>
#include <utility>

#include "Core/Resources/TextureAtlas.h"
#include "Render/SpriteBatch.h"

namespace {
const Vector4 kBlanco{1.0f, 1.0f, 1.0f, 1.0f};
}

HudIconos::HudIconos(Texture* iconos, Hud::Iconos manifiesto) : m_manifiesto(std::move(manifiesto)) {
    if (iconos == nullptr || !m_manifiesto.cargado()) return;
    m_atlas = std::make_unique<TextureAtlas>(iconos, m_manifiesto.lado(), m_manifiesto.lado());
    const int columnas = std::max(1, m_manifiesto.columnas());
    const int filas = std::max(1, m_manifiesto.filas());
    for (int f = 1; f <= columnas * filas; ++f) {
        m_atlas->defineRegion(f, (f - 1) % columnas, filas - 1 - (f - 1) / columnas);
    }
}

HudIconos::~HudIconos() = default;
HudIconos::HudIconos(HudIconos&&) noexcept = default;
HudIconos& HudIconos::operator=(HudIconos&&) noexcept = default;

void HudIconos::dibujar(SpriteBatch& batch, int frame, const Vector2& topLeft) const {
    if (m_atlas == nullptr || frame <= 0) return;
    batch.submit(topLeft, Vector2{lado(), lado()}, m_atlas->getUV(frame), m_atlas->texture(), kBlanco);
}
