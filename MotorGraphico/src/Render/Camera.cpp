#include "Render/Camera.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Core/Math/IsoMath.h"

namespace {

// Curvas de easing: t en [0,1] (progreso lineal de tiempo) -> progreso
// "curvado" en [0,1]. EaseInOutQuad y EaseOutCubic son las formulas
// estandar (ver easings.net); Linear es la identidad (equivalente a no
// tener easing, para transiciones mecanicas/instantaneas en vez de
// "suaves").
float applyEasing(Camera::Easing easing, float t) {
    switch (easing) {
        case Camera::Easing::Linear:
            return t;
        case Camera::Easing::EaseInOutQuad:
            return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
        case Camera::Easing::EaseOutCubic:
            return 1.0f - std::pow(1.0f - t, 3.0f);
    }
    return t;  // inalcanzable, pero algunos compiladores lo exigen
}

}  // namespace

Camera::Camera(int viewportWidth, int viewportHeight)
    : m_viewportWidth(viewportWidth), m_viewportHeight(viewportHeight) {}

void Camera::update(float deltaTime) {
    if (m_transitionActive) {
        m_transitionElapsed += deltaTime;
        // duration <= 0 (corte instantaneo, ver transitionTo()): t=1 de
        // golpe en la primera update(), sin dividir por cero.
        float t = m_transitionDuration > 0.0f
                      ? std::clamp(m_transitionElapsed / m_transitionDuration, 0.0f, 1.0f)
                      : 1.0f;
        float eased = applyEasing(m_transitionEasing, t);
        m_position = m_transitionStartPosition +
                    (m_transitionTargetPosition - m_transitionStartPosition) * eased;
        m_zoom = m_transitionStartZoom + (m_transitionTargetZoom - m_transitionStartZoom) * eased;
        if (t >= 1.0f) {
            // Fija el destino exacto (evita que quede a 0.9999... por
            // redondeo de la curva) y desactiva la transicion: update()
            // vuelve al lerp exponencial normal a partir del siguiente
            // frame (m_targetPosition ya apunta aqui, ver transitionTo()).
            m_position = m_transitionTargetPosition;
            m_zoom = m_transitionTargetZoom;
            m_transitionActive = false;
        }
        return;
    }

    float t = std::clamp(m_lerpSpeed * deltaTime, 0.0f, 1.0f);
    m_position = m_position + (m_targetPosition - m_position) * t;
}

void Camera::move(const Vector2& delta) { m_targetPosition = m_targetPosition + delta; }

void Camera::setZoom(float zoom) { m_zoom = std::max(zoom, 0.01f); }

void Camera::transitionTo(const Vector2& targetPosition, float targetZoom, float duration,
                          Easing easing) {
    // Reinicia SIEMPRE desde la posicion/zoom actuales (ya interpolados
    // si habia una transicion en curso), nunca desde el destino de una
    // transicion previa: encadenar llamadas no produce saltos.
    m_transitionActive = true;
    m_transitionElapsed = 0.0f;
    m_transitionDuration = std::max(duration, 0.0f);
    m_transitionEasing = easing;
    m_transitionStartPosition = m_position;
    m_transitionStartZoom = m_zoom;
    m_transitionTargetPosition = targetPosition;
    m_transitionTargetZoom = std::max(targetZoom, 0.01f);  // mismo minimo que setZoom()
    // El paneo continuo (move()/update() normal) retoma desde aqui en
    // cuanto la transicion termine, sin salto.
    m_targetPosition = targetPosition;
}

void Camera::setViewportSize(int width, int height) {
    m_viewportWidth = std::max(width, 1);
    m_viewportHeight = std::max(height, 1);
}

glm::mat4 Camera::getViewProjectionMatrix() const {
    float halfW = static_cast<float>(m_viewportWidth) / (2.0f * m_zoom);
    float halfH = static_cast<float>(m_viewportHeight) / (2.0f * m_zoom);

    // bottom=halfH, top=-halfH: ver comentario de Y invertida en el .h.
    glm::mat4 projection = glm::ortho(-halfW, halfW, halfH, -halfH, -1.0f, 1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-m_position.x, -m_position.y, 0.0f));
    return projection * view;
}

Vector2 Camera::screenToWorld(const Vector2& screenPos) const {
    Vector2 centered{
        screenPos.x - static_cast<float>(m_viewportWidth) * 0.5f,
        screenPos.y - static_cast<float>(m_viewportHeight) * 0.5f,
    };
    return m_position + centered * (1.0f / m_zoom);
}

GridCoord Camera::worldToGrid(const Vector2& worldPos, float tileWidth, float tileHeight) const {
    return IsoMath::screenToGrid(worldPos, tileWidth, tileHeight);
}
