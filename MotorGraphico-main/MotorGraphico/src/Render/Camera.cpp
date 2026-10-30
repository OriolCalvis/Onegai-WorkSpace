#include "Render/Camera.h"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Core/Math/IsoMath.h"
#include "Core/Math/Interpolacion.h"

namespace {

// Adaptador del enum publico Camera::Easing a la curva compartida
// (Core/Math/Interpolacion.h, U3-07). Las formulas viven alli.
Interpolacion::TipoCurva tipoDeEasing(Camera::Easing easing) {
    switch (easing) {
        case Camera::Easing::Linear:
            return Interpolacion::TipoCurva::Lineal;
        case Camera::Easing::EaseInOutQuad:
            return Interpolacion::TipoCurva::EntradaSalidaCuadratica;
        case Camera::Easing::EaseOutCubic:
            return Interpolacion::TipoCurva::SalidaCubica;
    }
    return Interpolacion::TipoCurva::Lineal;  // inalcanzable
}

}  // namespace

Camera::Camera(int viewportWidth, int viewportHeight)
    : m_viewportWidth(viewportWidth), m_viewportHeight(viewportHeight) {}

void Camera::update(float deltaTime) {
    if (m_transitionActive) {
        m_transitionElapsed += deltaTime;
        // duration <= 0 (corte instantaneo, ver transitionTo(), que ya acota
        // a >= 0): t=1 de golpe en la primera update(), sin dividir por cero.
        // El reloj sigue siendo float (m_transitionElapsed); progreso() solo
        // normaliza. Si devolviera Error (no ocurre con duracion finita >= 0),
        // se trata como fin de transicion (t=1). Cambio aceptado en U3-07.E08:
        // duracion +inf o deltaTime NaN ahora terminan la transicion en la
        // primera update (antes no terminaba nunca).
        float t = 1.0f;
        if (m_transitionDuration > 0.0f) {
            auto progreso = Interpolacion::progreso(static_cast<double>(m_transitionElapsed),
                                                    static_cast<double>(m_transitionDuration));
            if (progreso.isOk()) t = static_cast<float>(progreso.value());
        }
        double eased = Interpolacion::curva(tipoDeEasing(m_transitionEasing), t);
        m_position = Interpolacion::tweenVec2(m_transitionStartPosition,
                                              m_transitionTargetPosition, eased);
        m_zoom = static_cast<float>(
            Interpolacion::tweenEscalar(m_transitionStartZoom, m_transitionTargetZoom, eased));
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
