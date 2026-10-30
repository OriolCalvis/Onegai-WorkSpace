// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-14  (fase G, 3 j) - HUD, diàlegs i comerç amb accions visibles
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

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
    : m_whiteTexture(whiteTexture), m_data(transform) {}

void HudBar::setValue(float value) {
    m_data.value = std::clamp(value, 0.0f, m_data.maxValue);
    if (m_data.animationSpeed <= 0.0f) {
        m_data.displayedValue = m_data.value;  // sin animacion: siempre en sincronia
    }
}

void HudBar::setMaxValue(float maxValue) {
    m_data.maxValue = std::max(maxValue, 0.0f);
    m_data.value = std::clamp(m_data.value, 0.0f, m_data.maxValue);
    m_data.displayedValue = std::clamp(m_data.displayedValue, 0.0f, m_data.maxValue);
    if (m_data.animationSpeed <= 0.0f) {
        m_data.displayedValue = m_data.value;
    }
}

void HudBar::setColors(const Vector4& background, const Vector4& fill) {
    m_data.backgroundColor = background;
    m_data.fillColor = fill;
}

void HudBar::setBorder(const Vector4& color, float thickness) {
    m_data.borderColor = color;
    m_data.borderThickness = thickness;
}

void HudBar::setAnimationSpeed(float speed) {
    m_data.animationSpeed = speed;
    if (m_data.animationSpeed <= 0.0f) {
        m_data.displayedValue = m_data.value;  // sin animacion: nada "a medio camino"
    }
}

void HudBar::update(float deltaTime) {
    if (m_data.animationSpeed <= 0.0f) {
        m_data.displayedValue = m_data.value;
        return;
    }
    // Suavizado exponencial independiente del framerate: en cada segundo
    // se recorre una fraccion fija de la distancia restante. El clamp del
    // factor a [0,1] evita sobrepasar el objetivo con deltas grandes
    // (una parada en el debugger no debe hacer rebotar la barra).
    float t = std::clamp(m_data.animationSpeed * deltaTime, 0.0f, 1.0f);
    m_data.displayedValue += (m_data.value - m_data.displayedValue) * t;
    // Corte de convergencia: por debajo de medio pixel de barra no se
    // aprecia, y asi displayedValue() termina EXACTAMENTE en value().
    if (std::abs(m_data.displayedValue - m_data.value) < 0.001f * std::max(m_data.maxValue, 1.0f)) {
        m_data.displayedValue = m_data.value;
    }
}

void HudBar::setThresholdColors(const Vector4& high, const Vector4& mid, const Vector4& low) {
    m_data.thresholdHigh = high;
    m_data.thresholdMid = mid;
    m_data.thresholdLow = low;
    m_data.useThresholds = true;
}

const Vector4& HudBar::currentFillColor() const {
    if (!m_data.useThresholds) {
        return m_data.fillColor;
    }
    float ratio = m_data.maxValue > 0.0f ? (m_data.value / m_data.maxValue) : 0.0f;
    if (ratio > 0.5f) {
        return m_data.thresholdHigh;
    }
    return ratio > 0.25f ? m_data.thresholdMid : m_data.thresholdLow;
}

void HudBar::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_data.transform.resolveTopLeft(viewportWidth, viewportHeight);

    // Borde: un quad mayor DETRAS del fondo (2 quads en total, no 4
    // tiras: mas simple y el resultado visual es identico porque el
    // fondo tapa el centro).
    if (m_data.borderThickness > 0.0f) {
        Vector2 borderTopLeft{topLeft.x - m_data.borderThickness,
                              topLeft.y - m_data.borderThickness};
        Vector2 borderSize{m_data.transform.size.x + 2.0f * m_data.borderThickness,
                           m_data.transform.size.y + 2.0f * m_data.borderThickness};
        batch.submit(borderTopLeft, borderSize, kFullUV, m_whiteTexture, m_data.borderColor);
    }

    // Fondo: quad completo, tint = HudBarData::backgroundColor.
    batch.submit(topLeft, m_data.transform.size, kFullUV, m_whiteTexture, m_data.backgroundColor);

    // maxValue<=0 se trata como "sin relleno visible" en vez de dividir
    // por cero.
    float ratio = m_data.maxValue > 0.0f ? (m_data.value / m_data.maxValue) : 0.0f;
    float displayedRatio =
        m_data.maxValue > 0.0f ? (m_data.displayedValue / m_data.maxValue) : 0.0f;

    // Franja "fantasma": entre el valor real y el dibujado (solo cuando
    // el dibujado va POR DETRAS al bajar, es decir, tras recibir dano).
    // Se pinta a lo ancho del valor dibujado, y el relleno real encima la
    // recorta a la franja visible.
    if (displayedRatio > ratio) {
        Vector2 ghostSize{m_data.transform.size.x * displayedRatio, m_data.transform.size.y};
        batch.submit(topLeft, ghostSize, kFullUV, m_whiteTexture, m_data.ghostColor);
    }

    // Relleno: mismo quad pero recortado en X segun value/maxValue (0 =
    // sin relleno, 1 = barra completa).
    Vector2 fillSize{m_data.transform.size.x * ratio, m_data.transform.size.y};
    if (fillSize.x > 0.0f) {
        batch.submit(topLeft, fillSize, kFullUV, m_whiteTexture, currentFillColor());
    }
}

HudPanel::HudPanel(const HudTransform& transform, Texture* whiteTexture, const Vector4& color)
    : m_whiteTexture(whiteTexture), m_data(transform, color) {}

void HudPanel::setBorder(const Vector4& color, float thickness) {
    m_data.borderColor = color;
    m_data.borderThickness = thickness;
}

void HudPanel::render(SpriteBatch& batch, int viewportWidth, int viewportHeight) const {
    Vector2 topLeft = m_data.transform.resolveTopLeft(viewportWidth, viewportHeight);
    if (m_data.borderThickness > 0.0f) {
        Vector2 borderTopLeft{topLeft.x - m_data.borderThickness,
                              topLeft.y - m_data.borderThickness};
        Vector2 borderSize{m_data.transform.size.x + 2.0f * m_data.borderThickness,
                           m_data.transform.size.y + 2.0f * m_data.borderThickness};
        batch.submit(borderTopLeft, borderSize, kFullUV, m_whiteTexture, m_data.borderColor);
    }
    batch.submit(topLeft, m_data.transform.size, kFullUV, m_whiteTexture, m_data.color);
}
