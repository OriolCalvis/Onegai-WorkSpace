#pragma once

#include <array>

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

private:
    friend class InputSystem;

    struct ActionState {
        bool pressed = false;
        bool held = false;
        bool released = false;
    };
    std::array<ActionState, kInputActionCount> m_actions{};
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
};

}  // namespace Engine::Input
