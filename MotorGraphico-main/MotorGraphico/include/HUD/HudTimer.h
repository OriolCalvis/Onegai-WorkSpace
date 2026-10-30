// H1-02 (Gantt Fase H) — rescate de G-14: HudTimer vivía solo dentro de
// tests/Game/G14HudTimersAndCommerceTests.cpp. Aquí es producción: App
// enganxa este timer al widget GL del HUD; esta clase no sabe nada de
// ventanas ni de OpenGL.
//
// VER: planificacion/JUGABILIDAD_FASE_H.md — H1-02.
#pragma once

#include <functional>

namespace Game {

// Cómo se muestra y cuándo se dispara un temporizador de misión.
// REV-02: el timer se congela (dt=0) durante pausa/combate/diálogo;
// quien llama evita tickear en esas modalidades.
enum class HudTimerMode { Countdown, Bar, Secret };

// Temporizador de misión con callback de expiración.
//
// tick(dt) avanza el timer; devuelve true en el flanco de expiración.
// REV-02: durante pausa/combate/diálogo se pasa dt=0 o paused=true y
// quien llama evita tickear. onExpire() se dispara exactamente una vez
// por expiración (firedCount() lo confirma); después de expired()==true
// tick() no dispara de nuevo hasta reset().
class HudTimer {
public:
    HudTimer(float totalSeconds = 60.0f, HudTimerMode mode = HudTimerMode::Countdown);

    void setPaused(bool p);
    bool paused() const;

    // Devuelve true si EXPIRÓ esta llamada (flanco).
    bool tick(float dt);

    void reset();
    float remaining() const;
    float total() const;
    bool expired() const;
    int firedCount() const;

    // Registra un callback de expiración. Se dispara una vez por ciclo
    // completo (countdown → 0); el contador interno firedCount se
    // incrementa dentro del callback.
    void onExpire(std::function<void()> cb);

private:
    float m_total;
    float m_remain;
    HudTimerMode m_mode;
    bool m_paused = false;
    bool m_expired = false;
    int m_fired = 0;
    std::function<void()> m_onExpire;
};

}  // namespace Game
