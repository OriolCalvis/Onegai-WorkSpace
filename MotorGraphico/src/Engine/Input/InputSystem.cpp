#include "Engine/Input/InputSystem.h"

#include <algorithm>
#include <utility>

namespace Engine::Input {
namespace {

std::size_t keyIndex(InputKey key) { return static_cast<std::size_t>(key); }
std::size_t actionIndex(InputAction action) { return static_cast<std::size_t>(action); }

}  // namespace

bool RawInputState::isDown(InputKey key) const { return m_down[keyIndex(key)]; }

void RawInputState::setDown(InputKey key, bool down) { m_down[keyIndex(key)] = down; }

InputMap InputMap::gameplayDefaults() {
    InputMap map;
    map.bind(InputAction::Quit, InputKey::Escape);
    map.bind(InputAction::MoveNorth, InputKey::W);
    map.bind(InputAction::MoveSouth, InputKey::S);
    map.bind(InputAction::MoveWest, InputKey::A);
    map.bind(InputAction::MoveEast, InputKey::D);
    map.bind(InputAction::Interact, InputKey::E);
    map.bind(InputAction::ToggleInventory, InputKey::Tab);
    map.bind(InputAction::QuickSave, InputKey::F5);
    map.bind(InputAction::QuickLoad, InputKey::F9);
    map.bind(InputAction::MenuUp, InputKey::W);
    map.bind(InputAction::MenuDown, InputKey::S);
    map.bind(InputAction::Confirm, InputKey::Enter);
    map.bind(InputAction::Confirm, InputKey::Space);
    map.bind(InputAction::Cancel, InputKey::E);
    map.bind(InputAction::Cancel, InputKey::Tab);
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
    return false;
}

InputSystem::InputSystem(InputMap map) : m_map(std::move(map)) {}

void InputSystem::setMap(InputMap map) {
    m_map = std::move(map);
    reset();
}

void InputSystem::reset() { m_previous = RawInputState{}; }

InputSnapshot InputSystem::update(RawInputState current) {
    InputSnapshot snapshot;
    for (std::size_t index = 0; index < kInputActionCount; ++index) {
        const InputAction action = static_cast<InputAction>(index);
        const bool wasDown = actionIsDown(action, m_previous);
        const bool isDown = actionIsDown(action, current);
        snapshot.m_actions[index] = InputSnapshot::ActionState{isDown && !wasDown, isDown,
                                                                 wasDown && !isDown};
    }
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
