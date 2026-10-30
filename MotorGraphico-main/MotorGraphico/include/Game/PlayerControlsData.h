// Estado de repeticion direccional. No conoce ventanas ni reglas de colision.
#pragma once

#include <array>

struct PlayerControlsData {
    std::array<unsigned long long, 4> pressOrder{};
    unsigned long long sequence = 0;
    int direction = -1;
    int pendingDirection = -1; // Un solo tap pendiente; una pulsacion nueva lo sustituye.
    double nextStep = 0.0;
    double nextDirectionStep = 0.0; // Cambiar/re-pulsar nunca adelanta el paso anterior.
    double lastTime = 0.0;
    bool waitingForRelease = false;
    bool pendingRequiresHold = false;
};
