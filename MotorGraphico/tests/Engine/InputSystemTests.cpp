#include "Engine/Input/InputSystem.h"
#include "TestHarness.h"

namespace {

using Engine::Input::InputAction;
using Engine::Input::InputKey;
using Engine::Input::InputMap;
using Engine::Input::InputSystem;
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

}  // namespace

int main() { return motor_test::runAllTests(); }
