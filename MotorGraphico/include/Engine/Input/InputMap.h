#pragma once

#include <array>
#include <cstddef>
#include <vector>

// RUN-02 — Vocabulario de entrada independiente de la biblioteca de ventana.
// GLFW (o un gamepad futuro) se adapta a InputKey; el juego solo consulta
// InputAction y no necesita conocer códigos físicos de teclas.
namespace Engine::Input {

enum class InputKey : std::size_t {
    Escape,
    W,
    A,
    S,
    D,
    E,
    Tab,
    F5,
    F9,
    Enter,
    Space,
    Count,
};

enum class InputAction : std::size_t {
    Quit,
    MoveNorth,
    MoveSouth,
    MoveWest,
    MoveEast,
    Interact,
    ToggleInventory,
    QuickSave,
    QuickLoad,
    MenuUp,
    MenuDown,
    Confirm,
    Cancel,
    Count,
};

constexpr std::size_t kInputKeyCount = static_cast<std::size_t>(InputKey::Count);
constexpr std::size_t kInputActionCount = static_cast<std::size_t>(InputAction::Count);

// Estado físico de las teclas recogido durante un frame. Se puede construir
// en tests, GLFW o cualquier otra plataforma sin introducir sus cabeceras en
// el núcleo del motor.
class RawInputState {
public:
    bool isDown(InputKey key) const;
    void setDown(InputKey key, bool down = true);

private:
    std::array<bool, kInputKeyCount> m_down{};
};

class InputMap {
public:
    // Mapa de teclado del juego actual. Se concentra aquí para que
    // Application deje de contener decisiones tecla -> acción.
    static InputMap gameplayDefaults();

    void bind(InputAction action, InputKey key);
    void clear(InputAction action);
    bool isBound(InputAction action, InputKey key) const;
    const std::vector<InputKey>& bindings(InputAction action) const;

private:
    std::array<std::vector<InputKey>, kInputActionCount> m_bindings;
};

}  // namespace Engine::Input
