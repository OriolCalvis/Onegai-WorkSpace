#include "Engine/ApplicationLoop.h"

#include <algorithm>
#include <cmath>

namespace Engine {
namespace {

constexpr float kDefaultFixedStepSeconds = 1.0f / 60.0f;
constexpr float kDefaultMaxFrameSeconds = 0.10f;
constexpr std::size_t kDefaultMaxFixedUpdates = 6;
constexpr double kClockEpsilon = 1.0e-9;

bool hasRequiredServices(const ApplicationLoopServices& services) {
    return static_cast<bool>(services.shouldKeepRunning) && static_cast<bool>(services.pollEvents) &&
           static_cast<bool>(services.processInput) && static_cast<bool>(services.fixedUpdate) &&
           static_cast<bool>(services.render);
}

}  // namespace

ApplicationLoop::ApplicationLoop(ApplicationLoopSettings settings) : m_settings(settings) {
    // No se permite construir un reloj que no pueda avanzar. Normalizar aquí
    // mantiene la llamada por frame sin ramas de configuración inválida.
    if (!std::isfinite(m_settings.fixedStepSeconds) || m_settings.fixedStepSeconds <= 0.0f) {
        m_settings.fixedStepSeconds = kDefaultFixedStepSeconds;
    }
    if (!std::isfinite(m_settings.maxFrameSeconds) || m_settings.maxFrameSeconds <= 0.0f) {
        m_settings.maxFrameSeconds = kDefaultMaxFrameSeconds;
    }
    if (m_settings.maxFixedUpdatesPerFrame == 0) {
        m_settings.maxFixedUpdatesPerFrame = kDefaultMaxFixedUpdates;
    }
}

void ApplicationLoop::reset() { m_accumulatorSeconds = 0.0; }

ApplicationLoopFrame ApplicationLoop::tick(float realFrameSeconds,
                                           const ApplicationLoopServices& services) {
    ApplicationLoopFrame frame;
    if (!hasRequiredServices(services) || !services.shouldKeepRunning()) {
        return frame;
    }

    services.pollEvents();
    if (!services.shouldKeepRunning()) {
        return frame;
    }

    services.processInput();
    frame.inputProcessed = true;
    if (!services.shouldKeepRunning()) {
        return frame;
    }

    // Un reloj de plataforma puede devolver valores negativos al reajustarse
    // y NaN/inf no pueden contaminar el acumulador. Ambos casos equivalen a
    // que no ha transcurrido tiempo de simulación en este frame.
    const float nonNegative = std::isfinite(realFrameSeconds)
                                  ? std::max(0.0f, realFrameSeconds)
                                  : 0.0f;
    frame.acceptedFrameSeconds = std::min(nonNegative, m_settings.maxFrameSeconds);
    m_accumulatorSeconds += static_cast<double>(frame.acceptedFrameSeconds);

    const double fixedStep = static_cast<double>(m_settings.fixedStepSeconds);
    while (m_accumulatorSeconds + kClockEpsilon >= fixedStep &&
           frame.fixedUpdates < m_settings.maxFixedUpdatesPerFrame &&
           services.shouldKeepRunning()) {
        services.fixedUpdate(m_settings.fixedStepSeconds);
        m_accumulatorSeconds -= fixedStep;
        if (m_accumulatorSeconds < 0.0) {
            m_accumulatorSeconds = 0.0;
        }
        ++frame.fixedUpdates;
    }

    // Si se alcanzó el presupuesto del frame se elimina el atraso entero,
    // salvo la fracción necesaria para interpolar. Simularlo en frames
    // posteriores haría que el juego persiguiera indefinidamente al reloj.
    if (frame.fixedUpdates == m_settings.maxFixedUpdatesPerFrame &&
        m_accumulatorSeconds + kClockEpsilon >= fixedStep) {
        const double retained = std::fmod(m_accumulatorSeconds, fixedStep);
        frame.droppedSimulationSeconds =
            static_cast<float>(std::max(0.0, m_accumulatorSeconds - retained));
        m_accumulatorSeconds = retained;
    }

    if (!services.shouldKeepRunning()) {
        return frame;
    }

    frame.interpolationAlpha = static_cast<float>(m_accumulatorSeconds / fixedStep);
    frame.interpolationAlpha = std::clamp(frame.interpolationAlpha, 0.0f, 1.0f);
    services.render(frame.interpolationAlpha);
    frame.rendered = true;
    return frame;
}

}  // namespace Engine
