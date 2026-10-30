#pragma once

#include <array>
#include <string>

#include "Engine/Input/InputMap.h"

namespace Engine::Input {

// Snapshot inmutable de acciones de un frame. La distinción entre pressed,
// held y released evita que cada consumidor implemente su propio detector de
// flancos y permite que los tests simulen secuencias completas de entrada.
class InputSnapshot {
public:
    bool pressed(InputAction action) const;
    bool held(InputAction action) const;
    bool released(InputAction action) const;
    bool hasActivity() const;

    // UI-BG3-01 — El raton del frame, con sus flancos como las acciones.
    struct Mouse {
        bool inside = false;  // hay posicion (el cursor esta en la vista)
        float x = 0.0f, y = 0.0f;
        bool moved = false;   // cambio de posicion respecto al frame anterior
        bool leftPressed = false, leftHeld = false, leftReleased = false;
        bool rightPressed = false;
        int wheel = 0;        // pasos enteros de rueda (positivo = arriba)
    };
    const Mouse& mouse() const { return m_mouse; }
    // Lo tecleado este frame (UTF-8), para campos de texto.
    const std::string& text() const { return m_text; }

private:
    friend class InputSystem;

    struct ActionState {
        bool pressed = false;
        bool held = false;
        bool released = false;
    };
    std::array<ActionState, kInputActionCount> m_actions{};
    Mouse m_mouse;
    std::string m_text;
};

class InputSystem {
public:
    explicit InputSystem(InputMap map = InputMap::gameplayDefaults());

    const InputMap& map() const { return m_map; }
    void setMap(InputMap map);
    void reset();

    // Construye el snapshot de este frame y conserva el estado físico para
    // calcular las transiciones del siguiente. El argumento es por valor
    // para que quien captura eventos no pueda mutarlo después del muestreo.
    InputSnapshot update(RawInputState current);

    // Debe llamarse cuando la ventana pierde el foco. Equivale a un frame
    // vacío y por tanto genera released para las acciones que estaban held;
    // al recuperar foco no queda ninguna tecla "pegada".
    InputSnapshot releaseAll();

private:
    bool actionIsDown(InputAction action, const RawInputState& state) const;

    InputMap m_map;
    RawInputState m_previous;
    // Lo que sobra de la rueda (un panel tactil gira en fracciones): se
    // acumula hasta completar un paso.
    float m_wheelRest = 0.0f;
};

}  // namespace Engine::Input
