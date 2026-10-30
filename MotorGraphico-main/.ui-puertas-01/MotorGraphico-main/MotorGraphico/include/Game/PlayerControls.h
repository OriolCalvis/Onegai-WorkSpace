// Convierte WASD en pasos: pulsacion inmediata y repeticion independiente de FPS.
// El consumidor decide si cada paso es legal; aqui no se mueve el mundo.
#pragma once

#include <cmath>
#include "Core/Math/GridCoord.h"
#include "Engine/Input/InputSystem.h"
#include "Game/PlayerControlsData.h"
#include "Game/SessionTipos.h"

enum class PlayerEscapeAction { Quit, CloseInventory, Pause, Resume, CloseInteraction, None };

class PlayerControls {
public:
    static PlayerEscapeAction escapeAction(GameMode mode, bool atTitle, bool inventoryOpen) {
        if (atTitle) return PlayerEscapeAction::Quit;
        if (inventoryOpen) return PlayerEscapeAction::CloseInventory;
        if (mode == GameMode::Exploration) return PlayerEscapeAction::Pause;
        if (mode == GameMode::Pause) return PlayerEscapeAction::Resume;
        if (mode == GameMode::Dialogue || mode == GameMode::Shop || mode == GameMode::Business)
            return PlayerEscapeAction::CloseInteraction;
        return PlayerEscapeAction::None;
    }

    static GridCoord poll(PlayerControlsData& data,
                          const Engine::Input::InputSnapshot& input,
                          double now, bool enabled) {
        using Engine::Input::InputAction;
        constexpr InputAction actions[] = {InputAction::MoveNorth, InputAction::MoveSouth,
                                          InputAction::MoveWest, InputAction::MoveEast};
        constexpr GridCoord steps[] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        if (!enabled || !std::isfinite(now) || now < data.lastTime) {
            data = {};
            data.waitingForRelease = true;
            return {};
        }
        data.lastTime = now;
        bool down[4]{};
        bool any = false;
        for (int i = 0; i < 4; ++i) {
            down[i] = input.held(actions[i]);
            any = any || down[i];
            if (input.pressed(actions[i])) data.pressOrder[i] = ++data.sequence;
        }
        if (data.waitingForRelease) {
            if (!any) data.waitingForRelease = false;
            return {};
        }
        // Los opuestos cancelan tambien el tap pendiente de ese eje.
        if (down[0] && down[1]) {
            down[0] = down[1] = false;
            if (data.pendingDirection == 0 || data.pendingDirection == 1)
                data.pendingDirection = -1;
        }
        if (down[2] && down[3]) {
            down[2] = down[3] = false;
            if (data.pendingDirection == 2 || data.pendingDirection == 3)
                data.pendingDirection = -1;
        }
        int selected = -1;
        for (int i = 0; i < 4; ++i) {
            if (down[i] && (selected < 0 || data.pressOrder[i] > data.pressOrder[selected]))
                selected = i;
        }
        if (data.pendingDirection >= 0 && data.pendingRequiresHold &&
            !down[data.pendingDirection]) data.pendingDirection = -1;
        if (selected >= 0 &&
            (input.pressed(actions[selected]) ||
             (selected != data.direction && data.pendingDirection < 0))) {
            data.pendingDirection = selected;
            data.pendingRequiresHold = !input.pressed(actions[selected]);
        }
        data.direction = selected;
        if (data.pendingDirection >= 0) {
            if (now < data.nextDirectionStep) return {};
            const int step = data.pendingDirection;
            // Soltar el tap no lo pierde. Tras consumirlo se retoma la tecla
            // que siga mantenida, respetando otro intervalo completo.
            data.pendingDirection = selected >= 0 && selected != step ? selected : -1;
            data.pendingRequiresHold = data.pendingDirection >= 0;
            // AV14-09: mantener la tecla anda seguido desde el primer paso (antes,
            // 0,24 s: el segundo paso llegaba tarde y el dibujo se paraba). Un toque
            // corto sigue siendo una casilla: el siguiente paso pide la tecla abajo.
            data.nextStep = now + 0.14;
            data.nextDirectionStep = now + 0.14;
            return steps[step];
        }
        if (selected < 0 || now < data.nextStep) return {};
        // Tras un frame lento no se recuperan pasos de golpe ni se cruza un muro.
        // La agenda sostenida compensa el muestreo del frame: no deriva a 30 FPS.
        data.nextStep += 0.14;
        if (data.nextStep <= now) data.nextStep = now + 0.14;
        data.nextDirectionStep = now + 0.14;
        return steps[selected];
    }
};
