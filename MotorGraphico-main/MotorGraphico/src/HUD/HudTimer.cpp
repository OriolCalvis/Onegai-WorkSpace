#include "HUD/HudTimer.h"

namespace Game {

HudTimer::HudTimer(float totalSeconds, HudTimerMode mode)
    : m_total(totalSeconds), m_remain(totalSeconds), m_mode(mode) {}

void HudTimer::setPaused(bool p) { m_paused = p; }
bool HudTimer::paused() const { return m_paused; }

bool HudTimer::tick(float dt) {
    // REV-02: durante pausa/combate/diálogo se pasa dt=0 (o paused=true)
    // y quien llama evita tickear.
    if (m_expired) return false;
    if (m_paused) return false;
    if (dt <= 0.0f) return false;
    m_remain -= dt;
    if (m_remain <= 0.0f) {
        m_remain = 0.0f;
        m_expired = true;
        if (m_onExpire) m_onExpire();
        return true;
    }
    return false;
}

void HudTimer::reset() {
    m_remain = m_total;
    m_expired = false;
    m_fired = 0;
}

float HudTimer::remaining() const { return m_remain; }
float HudTimer::total() const { return m_total; }
bool HudTimer::expired() const { return m_expired; }
int HudTimer::firedCount() const { return m_fired; }

void HudTimer::onExpire(std::function<void()> cb) {
    m_onExpire = [this, cb = std::move(cb)]() {
        ++m_fired;
        cb();
    };
}

}  // namespace Game
