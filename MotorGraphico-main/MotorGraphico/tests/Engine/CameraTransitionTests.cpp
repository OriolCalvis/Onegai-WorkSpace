// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/Camera.h"
#include "Core/Math/Vector2.h"

#include "TestHarness.h"

#include <cmath>

MOTOR_TEST_CASE(camera_constructor_sets_initial_values) {
    Camera camera(800, 600);
    MOTOR_CHECK(camera.position().x == 0.0f);
    MOTOR_CHECK(camera.position().y == 0.0f);
    MOTOR_CHECK(camera.zoom() == 1.0f);
    MOTOR_CHECK(camera.viewportWidth() == 800);
    MOTOR_CHECK(camera.viewportHeight() == 600);
}

MOTOR_TEST_CASE(camera_set_zoom_changes_zoom) {
    Camera camera(800, 600);
    camera.setZoom(2.0f);
    MOTOR_CHECK(camera.zoom() == 2.0f);
}

MOTOR_TEST_CASE(camera_transition_to_instant_when_duration_zero) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{100.0f, 200.0f}, 2.0f, 0.0f);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.0f);
    MOTOR_CHECK(std::abs(camera.position().x - 100.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.position().y - 200.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.zoom() - 2.0f) < 0.001f);
    MOTOR_CHECK(!camera.isTransitioning());
}

MOTOR_TEST_CASE(camera_transition_to_linear_interpolation) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{100.0f, 100.0f}, 1.0f, 1.0f, Camera::Easing::Linear);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.5f);
    MOTOR_CHECK(std::abs(camera.position().x - 50.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.position().y - 50.0f) < 0.001f);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.5f);
    MOTOR_CHECK(std::abs(camera.position().x - 100.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.position().y - 100.0f) < 0.001f);
    MOTOR_CHECK(!camera.isTransitioning());
}

MOTOR_TEST_CASE(camera_transition_to_linear_zoom) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{0.0f, 0.0f}, 2.0f, 1.0f, Camera::Easing::Linear);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.5f);
    MOTOR_CHECK(std::abs(camera.zoom() - 1.5f) < 0.001f);
    camera.update(0.5f);
    MOTOR_CHECK(std::abs(camera.zoom() - 2.0f) < 0.001f);
    MOTOR_CHECK(!camera.isTransitioning());
}

MOTOR_TEST_CASE(camera_transition_updates_is_transitioning_flag) {
    Camera camera(800, 600);
    MOTOR_CHECK(!camera.isTransitioning());
    camera.transitionTo(Vector2{100.0f, 100.0f}, 1.0f, 0.5f);
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.5f);
    MOTOR_CHECK(!camera.isTransitioning());
}

MOTOR_TEST_CASE(camera_easing_enum_values_exist) {
    // Solo verificamos que los enum valores compilan
    MOTOR_CHECK(static_cast<int>(Camera::Easing::Linear) >= 0);
    MOTOR_CHECK(static_cast<int>(Camera::Easing::EaseInOutQuad) >= 0);
    MOTOR_CHECK(static_cast<int>(Camera::Easing::EaseOutCubic) >= 0);
}

int main() { return motor_test::runAllTests(); }
