#include "Engine/Input/InputSystem.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace Engine::Input {
namespace {

std::size_t keyIndex(InputKey key) { return static_cast<std::size_t>(key); }
std::size_t actionIndex(InputAction action) { return static_cast<std::size_t>(action); }

}  // namespace

bool RawInputState::isDown(InputKey key) const { return m_down[keyIndex(key)]; }

void RawInputState::setDown(InputKey key, bool down) { m_down[keyIndex(key)] = down; }

void RawInputState::setMouse(float x, float y) {
    m_hasMouse = true;
    m_mouseX = x;
    m_mouseY = y;
}

bool RawInputState::isDown(MouseButton button) const {
    return m_buttons[static_cast<std::size_t>(button)];
}

void RawInputState::setDown(MouseButton button, bool down) {
    m_buttons[static_cast<std::size_t>(button)] = down;
}

InputMap InputMap::gameplayDefaults() {
    InputMap map;
    map.bind(InputAction::Quit, InputKey::Escape);
    // Oriol (25/10): «volver al wasd para moverse, el qwas es demasiado dificil». W A S D, cada una por un
    // eje de la rejilla isometrica (IsoMath), que en pantalla son las cuatro diagonales de los personajes
    // de PixelLab: W arriba-derecha (-y), S abajo-izquierda (+y), A arriba-izquierda (-x) y D
    // abajo-derecha (+x).
    map.bind(InputAction::MoveNorth, InputKey::W);
    map.bind(InputAction::MoveSouth, InputKey::S);
    map.bind(InputAction::MoveWest, InputKey::A);
    map.bind(InputAction::MoveEast, InputKey::D);
    // Los menus siguen en cruz: W/S arriba y abajo (MenuUp/MenuDown), A/D a los lados.
    map.bind(InputAction::SideLeft, InputKey::A);
    map.bind(InputAction::SideRight, InputKey::D);
    map.bind(InputAction::CursorUp, InputKey::UpArrow);
    map.bind(InputAction::CursorDown, InputKey::DownArrow);
    map.bind(InputAction::CursorLeft, InputKey::LeftArrow);
    map.bind(InputAction::CursorRight, InputKey::RightArrow);
    map.bind(InputAction::Interact, InputKey::E);
    map.bind(InputAction::ToggleInventory, InputKey::Tab);
    map.bind(InputAction::QuickSave, InputKey::F5);
    map.bind(InputAction::QuickLoad, InputKey::F9);
    map.bind(InputAction::MenuUp, InputKey::W);
    map.bind(InputAction::MenuDown, InputKey::S);
    // H0-02 — pestañas de inventario: ← y → pasan de pestaña (Equipo,
    // Estado, Acciones). La tecla era la única pieza que faltaba: el
    // consumidor (Application::processInput) ya existía desde GDB-12.
    map.bind(InputAction::MenuLeft, InputKey::LeftArrow);
    map.bind(InputAction::MenuRight, InputKey::RightArrow);
    map.bind(InputAction::Confirm, InputKey::Enter);
    map.bind(InputAction::Confirm, InputKey::Space);
    // Oriol (22/10): «la tecla E = Enter». La E confirma en menus, dialogos y tiendas
    // (antes cerraba); se sale con ESC o TAB.
    map.bind(InputAction::Confirm, InputKey::E);
    map.bind(InputAction::Cancel, InputKey::Tab);
    // UI-BG3-01 — Las flechas de arriba y abajo mueven el foco igual que W/S
    // (en las cuadriculas tambien se mueve con las cuatro).
    map.bind(InputAction::MenuUp, InputKey::UpArrow);
    map.bind(InputAction::MenuDown, InputKey::DownArrow);
    map.bind(InputAction::PrevTab, InputKey::Q);
    map.bind(InputAction::NextTab, InputKey::E);
    map.bind(InputAction::EndTurn, InputKey::F);
    map.bind(InputAction::ToggleLog, InputKey::L);
    map.bind(InputAction::Erase, InputKey::Backspace);
    map.bind(InputAction::ToggleMap, InputKey::M);  // MAPA-02
    const InputKey digitos[kSlotCount] = {InputKey::Digit1, InputKey::Digit2, InputKey::Digit3, InputKey::Digit4,
                                          InputKey::Digit5, InputKey::Digit6, InputKey::Digit7, InputKey::Digit8,
                                          InputKey::Digit9, InputKey::Digit0};
    for (std::size_t n = 0; n < kSlotCount; ++n) map.bind(slotAction(n), digitos[n]);
    return map;
}

void InputMap::bind(InputAction action, InputKey key) {
    std::vector<InputKey>& keys = m_bindings[actionIndex(action)];
    if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
        keys.push_back(key);
    }
}

void InputMap::clear(InputAction action) { m_bindings[actionIndex(action)].clear(); }

bool InputMap::isBound(InputAction action, InputKey key) const {
    const std::vector<InputKey>& keys = bindings(action);
    return std::find(keys.begin(), keys.end(), key) != keys.end();
}

const std::vector<InputKey>& InputMap::bindings(InputAction action) const {
    return m_bindings[actionIndex(action)];
}

bool InputSnapshot::pressed(InputAction action) const {
    return m_actions[actionIndex(action)].pressed;
}

bool InputSnapshot::held(InputAction action) const { return m_actions[actionIndex(action)].held; }

bool InputSnapshot::released(InputAction action) const {
    return m_actions[actionIndex(action)].released;
}

bool InputSnapshot::hasActivity() const {
    for (const ActionState& action : m_actions) {
        if (action.pressed || action.held || action.released) {
            return true;
        }
    }
    // Mover el raton, pulsar o girar la rueda tambien es estar jugando.
    return m_mouse.moved || m_mouse.leftPressed || m_mouse.leftHeld || m_mouse.leftReleased ||
           m_mouse.rightPressed || m_mouse.wheel != 0 || !m_text.empty();
}

InputSystem::InputSystem(InputMap map) : m_map(std::move(map)) {}

void InputSystem::setMap(InputMap map) {
    m_map = std::move(map);
    reset();
}

void InputSystem::reset() {
    m_previous = RawInputState{};
    m_wheelRest = 0.0f;
}

InputSnapshot InputSystem::update(RawInputState current) {
    InputSnapshot snapshot;
    for (std::size_t index = 0; index < kInputActionCount; ++index) {
        const InputAction action = static_cast<InputAction>(index);
        const bool wasDown = actionIsDown(action, m_previous);
        const bool isDown = actionIsDown(action, current);
        snapshot.m_actions[index] = InputSnapshot::ActionState{isDown && !wasDown, isDown,
                                                                 wasDown && !isDown};
    }
    // UI-BG3-01 — El raton: flancos de los botones, si se ha movido y los
    // pasos enteros de rueda (lo que sobra espera al frame siguiente).
    InputSnapshot::Mouse& raton = snapshot.m_mouse;
    raton.inside = current.hasMouse();
    raton.x = current.mouseX();
    raton.y = current.mouseY();
    raton.moved = current.hasMouse() &&
                  (!m_previous.hasMouse() || current.mouseX() != m_previous.mouseX() ||
                   current.mouseY() != m_previous.mouseY());
    const bool izquierdo = current.isDown(MouseButton::Left);
    const bool izquierdoAntes = m_previous.isDown(MouseButton::Left);
    raton.leftPressed = izquierdo && !izquierdoAntes;
    raton.leftHeld = izquierdo;
    raton.leftReleased = izquierdoAntes && !izquierdo;
    raton.rightPressed = current.isDown(MouseButton::Right) && !m_previous.isDown(MouseButton::Right);
    m_wheelRest += current.wheel();
    const float pasos = std::trunc(m_wheelRest);
    raton.wheel = static_cast<int>(pasos);
    m_wheelRest -= pasos;
    snapshot.m_text = current.text();
    m_previous = current;
    return snapshot;
}

InputSnapshot InputSystem::releaseAll() { return update(RawInputState{}); }

bool InputSystem::actionIsDown(InputAction action, const RawInputState& state) const {
    for (InputKey key : m_map.bindings(action)) {
        if (state.isDown(key)) {
            return true;
        }
    }
    return false;
}

}  // namespace Engine::Input
