#include "Engine/Input/InputSystem.h"
#include "TestHarness.h"

namespace {

using Engine::Input::InputAction;
using Engine::Input::InputKey;
using Engine::Input::InputMap;
using Engine::Input::InputSystem;
using Engine::Input::MouseButton;
using Engine::Input::RawInputState;

MOTOR_TEST_CASE(default_map_resolves_move_and_interact_from_simulated_keys) {
    InputSystem input;
    RawInputState raw;
    raw.setDown(InputKey::W);
    raw.setDown(InputKey::E);

    const auto snapshot = input.update(raw);
    MOTOR_CHECK(snapshot.pressed(InputAction::MoveNorth));
    MOTOR_CHECK(snapshot.held(InputAction::MoveNorth));
    MOTOR_CHECK(snapshot.pressed(InputAction::MenuUp));
    MOTOR_CHECK(snapshot.pressed(InputAction::Interact));
    MOTOR_CHECK(snapshot.pressed(InputAction::Cancel));
    MOTOR_CHECK(snapshot.hasActivity());
}

MOTOR_TEST_CASE(snapshot_tracks_pressed_held_and_released_transitions) {
    InputSystem input;
    RawInputState down;
    down.setDown(InputKey::Space);

    const auto pressed = input.update(down);
    MOTOR_CHECK(pressed.pressed(InputAction::Confirm));
    MOTOR_CHECK(pressed.held(InputAction::Confirm));
    MOTOR_CHECK(!pressed.released(InputAction::Confirm));

    const auto held = input.update(down);
    MOTOR_CHECK(!held.pressed(InputAction::Confirm));
    MOTOR_CHECK(held.held(InputAction::Confirm));
    MOTOR_CHECK(!held.released(InputAction::Confirm));

    const auto released = input.update(RawInputState{});
    MOTOR_CHECK(!released.pressed(InputAction::Confirm));
    MOTOR_CHECK(!released.held(InputAction::Confirm));
    MOTOR_CHECK(released.released(InputAction::Confirm));
}

MOTOR_TEST_CASE(bindings_are_configurable_without_glfw) {
    InputMap map;
    map.bind(InputAction::Interact, InputKey::Space);
    map.bind(InputAction::Interact, InputKey::Space);  // no duplica el binding
    map.bind(InputAction::MoveEast, InputKey::D);
    InputSystem input(map);

    RawInputState raw;
    raw.setDown(InputKey::Space);
    raw.setDown(InputKey::D);
    const auto snapshot = input.update(raw);
    MOTOR_CHECK(snapshot.pressed(InputAction::Interact));
    MOTOR_CHECK(snapshot.pressed(InputAction::MoveEast));
    MOTOR_CHECK(map.bindings(InputAction::Interact).size() == 1);

    map.clear(InputAction::Interact);
    input.setMap(map);
    MOTOR_CHECK(!input.update(raw).held(InputAction::Interact));
}

MOTOR_TEST_CASE(releasing_focus_clears_held_actions_before_the_next_frame) {
    InputSystem input;
    RawInputState raw;
    raw.setDown(InputKey::Tab);
    input.update(raw);

    const auto focusLost = input.releaseAll();
    MOTOR_CHECK(focusLost.released(InputAction::ToggleInventory));
    MOTOR_CHECK(!focusLost.held(InputAction::ToggleInventory));

    const auto idle = input.update(RawInputState{});
    MOTOR_CHECK(!idle.pressed(InputAction::ToggleInventory));
    MOTOR_CHECK(!idle.released(InputAction::ToggleInventory));
    MOTOR_CHECK(!idle.hasActivity());
}

// UI-BG3-01 — Las teclas nuevas: Q/E pasan de hoja, F pasa el turno, L
// esconde el registro, retroceso borra y 1-0 son las casillas de la barra.
MOTOR_TEST_CASE(new_keys_resolve_tabs_turn_log_erase_and_slots) {
    InputSystem input;
    RawInputState raw;
    raw.setDown(InputKey::Q);
    raw.setDown(InputKey::F);
    raw.setDown(InputKey::L);
    raw.setDown(InputKey::Backspace);
    raw.setDown(InputKey::Digit1);
    raw.setDown(InputKey::Digit0);
    raw.setDown(InputKey::DownArrow);
    const auto s = input.update(raw);
    MOTOR_CHECK(s.pressed(InputAction::PrevTab));
    MOTOR_CHECK(s.pressed(InputAction::EndTurn));
    MOTOR_CHECK(s.pressed(InputAction::ToggleLog));
    MOTOR_CHECK(s.pressed(InputAction::Erase));
    MOTOR_CHECK(s.pressed(Engine::Input::slotAction(0)));
    MOTOR_CHECK(s.pressed(Engine::Input::slotAction(9)));
    MOTOR_CHECK(!s.pressed(Engine::Input::slotAction(4)));
    MOTOR_CHECK(s.pressed(InputAction::MenuDown));
    MOTOR_CHECK(!s.pressed(InputAction::MoveSouth));  // la flecha no camina

    RawInputState e;
    e.setDown(InputKey::E);
    const auto s2 = input.update(e);
    MOTOR_CHECK(s2.pressed(InputAction::NextTab));
    MOTOR_CHECK(s2.pressed(InputAction::Interact));
}

MOTOR_TEST_CASE(mouse_buttons_have_edges_and_movement_counts_as_activity) {
    InputSystem input;
    RawInputState raw;
    MOTOR_CHECK(!input.update(raw).mouse().inside);  // sin posicion: fuera

    raw.setMouse(100.0f, 50.0f);
    const auto entra = input.update(raw);
    MOTOR_CHECK(entra.mouse().inside);
    MOTOR_CHECK(entra.mouse().moved);
    MOTOR_CHECK(entra.hasActivity());
    MOTOR_CHECK(entra.mouse().x == 100.0f && entra.mouse().y == 50.0f);

    const auto quieto = input.update(raw);
    MOTOR_CHECK(!quieto.mouse().moved);
    MOTOR_CHECK(!quieto.hasActivity());

    raw.setDown(MouseButton::Left);
    const auto baja = input.update(raw);
    MOTOR_CHECK(baja.mouse().leftPressed);
    MOTOR_CHECK(baja.mouse().leftHeld);
    const auto sigue = input.update(raw);
    MOTOR_CHECK(!sigue.mouse().leftPressed);
    MOTOR_CHECK(sigue.mouse().leftHeld);
    raw.setDown(MouseButton::Left, false);
    raw.setDown(MouseButton::Right);
    const auto suelta = input.update(raw);
    MOTOR_CHECK(suelta.mouse().leftReleased);
    MOTOR_CHECK(suelta.mouse().rightPressed);
}

MOTOR_TEST_CASE(wheel_accumulates_fractions_until_a_whole_step) {
    InputSystem input;
    RawInputState medio;
    medio.addWheel(0.5f);
    MOTOR_CHECK(input.update(medio).mouse().wheel == 0);
    MOTOR_CHECK(input.update(medio).mouse().wheel == 1);  // 0,5 + 0,5
    RawInputState abajo;
    abajo.addWheel(-2.25f);
    MOTOR_CHECK(input.update(abajo).mouse().wheel == -2);
    MOTOR_CHECK(input.update(RawInputState{}).mouse().wheel == 0);  // sobra -0,25
}

MOTOR_TEST_CASE(typed_text_reaches_the_snapshot_once) {
    InputSystem input;
    RawInputState raw;
    raw.addText("Ner");
    raw.addText("ea");
    const auto s = input.update(raw);
    MOTOR_CHECK(s.text() == "Nerea");
    MOTOR_CHECK(s.hasActivity());
    MOTOR_CHECK(input.update(RawInputState{}).text().empty());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
