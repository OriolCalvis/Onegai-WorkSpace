#pragma once

#include <array>
#include <cstddef>
#include <string>
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
    LeftArrow,
    RightArrow,
    // UI-BG3-01 — Las pantallas en cuadricula (Oriol, 05/10): Q/E pasan de
    // hoja, F pasa el turno, L esconde el registro, 1-0 juegan cartas y las
    // flechas de arriba y abajo tambien mueven el foco.
    Q,
    F,
    L,
    Backspace,
    UpArrow,
    DownArrow,
    Digit1,
    Digit2,
    Digit3,
    Digit4,
    Digit5,
    Digit6,
    Digit7,
    Digit8,
    Digit9,
    Digit0,
    M,  // MAPA-02: el mapa grande
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
    MenuLeft,
    MenuRight,
    Confirm,
    Cancel,
    // UI-BG3-01
    PrevTab,    // Q: hoja anterior del cuaderno de creacion
    NextTab,    // E: hoja siguiente
    EndTurn,    // F: pasar el turno en combate
    ToggleLog,  // L: esconder o ensenar el registro del combate
    Erase,      // retroceso en un campo de texto
    Slot1,      // 1..0: las diez casillas de la barra de cartas
    Slot2,
    Slot3,
    Slot4,
    Slot5,
    Slot6,
    Slot7,
    Slot8,
    Slot9,
    Slot10,
    ToggleMap,  // MAPA-02 — M: abrir y cerrar el mapa grande
    // Andar va con W A S D (Oriol, 25/10; del 23/10 al 25/10 fue Q W A S); los menus, en cruz.
    SideLeft,     // A: a la izquierda en los menus (pestanas, barra de cartas, cuaderno)
    SideRight,    // D: a la derecha en los menus
    CursorUp,     // flechas: el cursor del mapa grande (solo flechas, para no chocar con W A S D)
    CursorDown,
    CursorLeft,
    CursorRight,
    Count,
};

constexpr std::size_t kInputKeyCount = static_cast<std::size_t>(InputKey::Count);
constexpr std::size_t kInputActionCount = static_cast<std::size_t>(InputAction::Count);
constexpr std::size_t kSlotCount = 10;

// La casilla n (0..9) de la barra de cartas: Slot1..Slot10.
constexpr InputAction slotAction(std::size_t n) {
    return static_cast<InputAction>(static_cast<std::size_t>(InputAction::Slot1) + n);
}

// UI-BG3-01 — Botones del raton que el juego entiende.
enum class MouseButton : std::size_t { Left, Right, Count };
constexpr std::size_t kMouseButtonCount = static_cast<std::size_t>(MouseButton::Count);

// Estado físico de las teclas recogido durante un frame. Se puede construir
// en tests, GLFW o cualquier otra plataforma sin introducir sus cabeceras en
// el núcleo del motor.
//
// UI-BG3-01 — Tambien el raton (posicion en pixeles de la vista, botones y
// la rueda girada este frame) y el texto tecleado este frame (UTF-8): las
// pantallas en cuadricula se manejan con raton o con teclado, y el nombre
// del personaje se escribe.
class RawInputState {
public:
    bool isDown(InputKey key) const;
    void setDown(InputKey key, bool down = true);

    // Sin setMouse() el raton no esta en la vista (hasMouse() == false).
    void setMouse(float x, float y);
    bool hasMouse() const { return m_hasMouse; }
    float mouseX() const { return m_mouseX; }
    float mouseY() const { return m_mouseY; }
    bool isDown(MouseButton button) const;
    void setDown(MouseButton button, bool down = true);
    // Pasos de rueda girados este frame (positivo = hacia arriba).
    void addWheel(float steps) { m_wheel += steps; }
    float wheel() const { return m_wheel; }
    void addText(const std::string& utf8) { m_text += utf8; }
    const std::string& text() const { return m_text; }

private:
    std::array<bool, kInputKeyCount> m_down{};
    std::array<bool, kMouseButtonCount> m_buttons{};
    bool m_hasMouse = false;
    float m_mouseX = 0.0f;
    float m_mouseY = 0.0f;
    float m_wheel = 0.0f;
    std::string m_text;
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
