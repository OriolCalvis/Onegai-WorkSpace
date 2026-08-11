#include "Render/HudWidgets.h"

#include "Core/Math/UVRect.h"
#include "Render/SpriteBatch.h"

#include <algorithm>
#include <cmath>

namespace {

// UV completa (0,0)-(1,1): whiteTexture es 1x1, asi que cualquier UV
// devuelve el mismo pixel blanco; se deja explicito en vez de confiar en
// el default de UVRect para que quede claro que es intencional, no un
// descuido.
constexpr UVRect kFullUV{0.0f, 0.0f, 1.0f, 1.0f};

}  // namespace

HudBar::HudBar(const HudTransform& transform, Texture* whiteTexture)
    : m_transform(transform), m_whiteTexture(whiteTexture) {}

void HudBar::setValue(float value) {
    m_value = std::clamp(value, 0.0f, m_maxValue);
    if (m_animationSpeed <= 0.0f) {
        m_displayedValue = m_value;  // sin animacion: siempre en sincronia
    }
}

void HudBar::setMaxValue(float maxValue) {
    m_maxValue = std::max(maxValue, 0.0f);
    m_value = std::clamp(m_value, 0.0f, m_maxValue);
    m_displayedValue = std::clamp(m_displayedValue, 0.0f, m_maxValue);
    if (m_animationSpeed <= 0.0f) {
        m_displayedValue = m_value;
    }
}

void HudBar::setColors(const Vector4& background, const Vector4& fill) {
    m_backgroundColor = background;
    m_fillColor = fill;
}

void HudBar::setBorder(const Vector4& color, float thickness) {
    m_borderColor = color;
    m_borderThickness = thickness;
}

void HudBar::setAnimationSpeed(float speed) {
    m_animationSpeed = speed;
    if (m_animationSpeed <= 0.0f) {
        m_displayedValue = m_value;  // sin animacion: nada "a medio camino"
    }
}

void HudBar::update(float deltaTime) {
    if (m_animationSpeed <= 0.0f) {
        m_displayedValue = m_value;
        return;
    }
    // Suavizado exponencial independiente del framerate: en cada segundo
    // se recorre una fraccion fija de la distancia restante. El clamp del
    // factor a [0,1] evita sobrepasar el objetivo con deltas grandes
    // (una parada en el debugger no debe hacer rebotar la barra).
    float t = std::clamp(m_animationSpeed * deltaTime, 0.0f, 1.0f);
    m_displayedValue += (m_value - m_displayedValue) * t;
    // Corte de convergencia: por debajo de medio pixel de barra no se
    // aprecia, y asi displayedValue() termina EXACTAMENTE en value().
    if (std::abs(m_displayedValue - m_value) < 0.001f * std::max(m_maxValue, 1.0f)) {
        m_displayedValue = m_value;
    }
}

void HudBar::setThresholdColors(const Vector4& high, const Vector4& mid, const Vector4& low) {
    m_thresholdHigh = high;
    m_thresholdMid = mid;
    m_thresholdLow = low;
    m_useThresholds = true;
}

const Vector4& HudBar::currentFillColor() const {
    if (!m_useThresholds) {
        return m_fillColor;
    }
    float ratio = m_maxValue > 0.0f ? (m_value / m_maxValue) : 0.0f;
    if (ratio > 0.5f) {
        return m_thresholdHigh;
    }
    return ratio > 0.25f ? m_thresholdMid : m_thresholdLow;
}

void HudBar::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_transform.resolveTopLeft(viewportWidth, viewportHeight);

    // Borde: un quad mayor DETRAS del fondo (2 quads en total, no 4
    // tiras: mas simple y el resultado visual es identico porque el
    // fondo tapa el centro).
    if (m_borderThickness > 0.0f) {
        Vector2 borderTopLeft{topLeft.x - m_borderThickness, topLeft.y - m_borderThickness};
        Vector2 borderSize{m_transform.size.x + 2.0f * m_borderThickness,
                           m_transform.size.y + 2.0f * m_borderThickness};
        batch.submit(borderTopLeft, borderSize, kFullUV, m_whiteTexture, m_borderColor);
    }

    // Fondo: quad completo, tint = m_backgroundColor.
    batch.submit(topLeft, m_transform.size, kFullUV, m_whiteTexture, m_backgroundColor);

    // maxValue<=0 se trata como "sin relleno visible" en vez de dividir
    // por cero.
    float ratio = m_maxValue > 0.0f ? (m_value / m_maxValue) : 0.0f;
    float displayedRatio = m_maxValue > 0.0f ? (m_displayedValue / m_maxValue) : 0.0f;

    // Franja "fantasma": entre el valor real y el dibujado (solo cuando
    // el dibujado va POR DETRAS al bajar, es decir, tras recibir dano).
    // Se pinta a lo ancho del valor dibujado, y el relleno real encima la
    // recorta a la franja visible.
    if (displayedRatio > ratio) {
        Vector2 ghostSize{m_transform.size.x * displayedRatio, m_transform.size.y};
        batch.submit(topLeft, ghostSize, kFullUV, m_whiteTexture, m_ghostColor);
    }

    // Relleno: mismo quad pero recortado en X segun value/maxValue (0 =
    // sin relleno, 1 = barra completa).
    Vector2 fillSize{m_transform.size.x * ratio, m_transform.size.y};
    if (fillSize.x > 0.0f) {
        batch.submit(topLeft, fillSize, kFullUV, m_whiteTexture, currentFillColor());
    }
}

HudPanel::HudPanel(const HudTransform& transform, Texture* whiteTexture, const Vector4& color)
    : m_transform(transform), m_whiteTexture(whiteTexture), m_color(color) {}

void HudPanel::setBorder(const Vector4& color, float thickness) {
    m_borderColor = color;
    m_borderThickness = thickness;
}

void HudPanel::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_transform.resolveTopLeft(viewportWidth, viewportHeight);
    if (m_borderThickness > 0.0f) {
        Vector2 borderTopLeft{topLeft.x - m_borderThickness, topLeft.y - m_borderThickness};
        Vector2 borderSize{m_transform.size.x + 2.0f * m_borderThickness,
                           m_transform.size.y + 2.0f * m_borderThickness};
        batch.submit(borderTopLeft, borderSize, kFullUV, m_whiteTexture, m_borderColor);
    }
    batch.submit(topLeft, m_transform.size, kFullUV, m_whiteTexture, m_color);
}
