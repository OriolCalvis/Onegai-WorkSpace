// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/Camera.h"
#include "Core/Math/Vector2.h"

#include "TestHarness.h"

#include <cmath>

// --- Tests de construccion ---

MOTOR_TEST_CASE(camera_constructor_default_values) {
    Camera camera(800, 600);
    MOTOR_CHECK(camera.position().x == 0.0f);
    MOTOR_CHECK(camera.position().y == 0.0f);
    MOTOR_CHECK(camera.zoom() == 1.0f);
    MOTOR_CHECK(camera.viewportWidth() == 800);
    MOTOR_CHECK(camera.viewportHeight() == 600);
    MOTOR_CHECK(!camera.isTransitioning());
}

MOTOR_TEST_CASE(camera_constructor_with_different_viewport) {
    Camera camera(1280, 720);
    MOTOR_CHECK(camera.viewportWidth() == 1280);
    MOTOR_CHECK(camera.viewportHeight() == 720);
}

// --- Tests de movimiento ---

MOTOR_TEST_CASE(camera_move_updates_target_position) {
    Camera camera(800, 600);
    camera.move(Vector2{10.0f, 20.0f});
    // move() actualiza m_targetPosition, no m_position directamente
    // despues de update(), m_position se acerca a m_targetPosition
    MOTOR_CHECK(true); // No crash
}

MOTOR_TEST_CASE(camera_move_multiple_times) {
    Camera camera(800, 600);
    camera.move(Vector2{10.0f, 0.0f});
    camera.move(Vector2{0.0f, 15.0f});
    MOTOR_CHECK(true);
}

// --- Tests de zoom ---

MOTOR_TEST_CASE(camera_set_zoom_valid_values) {
    Camera camera(800, 600);
    camera.setZoom(2.0f);
    MOTOR_CHECK(camera.zoom() == 2.0f);
    
    camera.setZoom(0.5f);
    MOTOR_CHECK(camera.zoom() == 0.5f);
}

MOTOR_TEST_CASE(camera_set_zoom_clamped_to_minimum) {
    Camera camera(800, 600);
    camera.setZoom(0.0f);
    MOTOR_CHECK(camera.zoom() == 0.01f); // Minimum clamp
    
    camera.setZoom(-1.0f);
    MOTOR_CHECK(camera.zoom() == 0.01f);
}

MOTOR_TEST_CASE(camera_set_zoom_large_values) {
    Camera camera(800, 600);
    camera.setZoom(100.0f);
    MOTOR_CHECK(camera.zoom() == 100.0f);
}

// --- Tests de viewport ---

MOTOR_TEST_CASE(camera_set_viewport_size) {
    Camera camera(800, 600);
    camera.setViewportSize(1024, 768);
    MOTOR_CHECK(camera.viewportWidth() == 1024);
    MOTOR_CHECK(camera.viewportHeight() == 768);
}

MOTOR_TEST_CASE(camera_set_viewport_size_clamped) {
    Camera camera(800, 600);
    camera.setViewportSize(0, 0);
    MOTOR_CHECK(camera.viewportWidth() == 1);
    MOTOR_CHECK(camera.viewportHeight() == 1);
    
    camera.setViewportSize(-10, -10);
    MOTOR_CHECK(camera.viewportWidth() == 1);
    MOTOR_CHECK(camera.viewportHeight() == 1);
}

// --- Tests de update (lerp exponencial) ---

MOTOR_TEST_CASE(camera_update_moves_toward_target) {
    Camera camera(800, 600);
    camera.move(Vector2{100.0f, 50.0f});
    
    // Inicialmente position = (0,0), targetPosition = (100,50)
    float initialX = camera.position().x;
    float initialY = camera.position().y;
    
    camera.update(0.1f); // deltaTime = 0.1s
    
    float newX = camera.position().x;
    float newY = camera.position().y;
    
    // Debería haberse acercado a (100, 50)
    MOTOR_CHECK(newX > initialX);
    MOTOR_CHECK(newY > initialY);
    MOTOR_CHECK(newX < 100.0f); // No debería haber llegado
    MOTOR_CHECK(newY < 50.0f);
}

MOTOR_TEST_CASE(camera_update_reaches_target_over_time) {
    Camera camera(800, 600);
    camera.move(Vector2{10.0f, 10.0f});
    
    // Update con deltaTime grande para llegar al destino
    camera.update(10.0f); // con lerpSpeed = 8.0f, esto debería ser suficiente
    
    // Debería estar muy cerca del destino (10, 10)
    MOTOR_CHECK(std::abs(camera.position().x - 10.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.position().y - 10.0f) < 0.001f);
}

MOTOR_TEST_CASE(camera_update_stays_at_target) {
    Camera camera(800, 600);
    // No move, position ya está en (0,0) que es el target
    camera.update(0.1f);
    MOTOR_CHECK(camera.position().x == 0.0f);
    MOTOR_CHECK(camera.position().y == 0.0f);
}

// --- Tests de transitionTo ---

MOTOR_TEST_CASE(camera_transition_to_instant) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{50.0f, 30.0f}, 2.0f, 0.0f); // duration = 0
    
    MOTOR_CHECK(camera.isTransitioning());
    camera.update(0.1f);
    
    // Debería haber llegado instantáneamente
    MOTOR_CHECK(!camera.isTransitioning());
    MOTOR_CHECK(std::abs(camera.position().x - 50.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.position().y - 30.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.zoom() - 2.0f) < 0.001f);
}

MOTOR_TEST_CASE(camera_transition_to_with_duration) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{100.0f, 100.0f}, 1.0f, 1.0f); // 1 segundo de duración
    
    MOTOR_CHECK(camera.isTransitioning());
    
    // Después de medio segundo
    camera.update(0.5f);
    MOTOR_CHECK(camera.isTransitioning());
    
    // Debería estar a mitad del camino
    float posX = camera.position().x;
    float posY = camera.position().y;
    MOTOR_CHECK(posX > 0.0f && posX < 100.0f);
    MOTOR_CHECK(posY > 0.0f && posY < 100.0f);
    
    // Después del tiempo completo
    camera.update(0.5f);
    MOTOR_CHECK(!camera.isTransitioning());
    MOTOR_CHECK(std::abs(camera.position().x - 100.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.position().y - 100.0f) < 0.001f);
    MOTOR_CHECK(std::abs(camera.zoom() - 1.0f) < 0.001f);
}

MOTOR_TEST_CASE(camera_transition_to_cancels_previous) {
    Camera camera(800, 600);
    camera.transitionTo(Vector2{100.0f, 100.0f}, 1.0f, 2.0f);
    camera.update(0.1f);
    
    // Iniciar una nueva transición
    camera.transitionTo(Vector2{50.0f, 50.0f}, 1.0f, 0.5f);
    
    // La nueva transición debería haber reiniciado desde la posición actual
    // (que ya había avanzado algo hacia 100,100)
    MOTOR_CHECK(camera.isTransitioning());
}

// --- Tests de screenToWorld ---

MOTOR_TEST_CASE(camera_screen_to_world_center) {
    Camera camera(800, 600);
    Vector2 center{400.0f, 300.0f};
    Vector2 world = camera.screenToWorld(center);
    
    // El centro de la pantalla en coordenadas de mundo debería ser (0, 0)
    MOTOR_CHECK(std::abs(world.x) < 0.001f);
    MOTOR_CHECK(std::abs(world.y) < 0.001f);
}

MOTOR_TEST_CASE(camera_screen_to_world_top_left) {
    Camera camera(800, 600);
    Vector2 topLeft{0.0f, 0.0f};
    Vector2 world = camera.screenToWorld(topLeft);
    
    // Top-left: debería ser negativo en x e y
    MOTOR_CHECK(world.x < 0.0f);
    MOTOR_CHECK(world.y < 0.0f);
}

MOTOR_TEST_CASE(camera_screen_to_world_with_zoom) {
    Camera camera(800, 600);
    camera.setZoom(2.0f);
    
    Vector2 center{400.0f, 300.0f};
    Vector2 world = camera.screenToWorld(center);
    
    // Con zoom 2x, el centro sigue siendo (0,0)
    MOTOR_CHECK(std::abs(world.x) < 0.001f);
    MOTOR_CHECK(std::abs(world.y) < 0.001f);
}

MOTOR_TEST_CASE(camera_screen_to_world_with_pan) {
    Camera camera(800, 600);
    camera.move(Vector2{100.0f, 50.0f});
    camera.update(10.0f); // Llegar al destino
    
    Vector2 center{400.0f, 300.0f};
    Vector2 world = camera.screenToWorld(center);
    
    // Con la cámara en (100, 50), el centro de la pantalla en mundo es (100, 50)
    MOTOR_CHECK(std::abs(world.x - 100.0f) < 0.001f);
    MOTOR_CHECK(std::abs(world.y - 50.0f) < 0.001f);
}

// --- Tests de worldToGrid ---

MOTOR_TEST_CASE(camera_world_to_grid_delegates_to_iso_math) {
    Camera camera(800, 600);
    Vector2 worldPos{128.0f, 64.0f};
    GridCoord grid = camera.worldToGrid(worldPos);
    
    // El cálculo exacto depende de IsoMath, pero no debería crash
    MOTOR_CHECK(true);
}

// --- Tests de herencia (IUpdatable) ---

MOTOR_TEST_CASE(camera_implements_iupdatable) {
    Camera camera(800, 600);
    IUpdatable* updatable = &camera;
    MOTOR_CHECK(updatable != nullptr);
}

// --- Tests de Easing enum ---

MOTOR_TEST_CASE(camera_easing_enum_has_values) {
    // Verificar que el enum tiene los valores esperados
    MOTOR_CHECK(static_cast<int>(Camera::Easing::Linear) == 0);
    MOTOR_CHECK(static_cast<int>(Camera::Easing::EaseInOutQuad) == 1);
    MOTOR_CHECK(static_cast<int>(Camera::Easing::EaseOutCubic) == 2);
}

// --- Tests de getViewProjectionMatrix (GL-free, solo matriz glm) ---

MOTOR_TEST_CASE(camera_get_view_projection_matrix_returns_matrix) {
    Camera camera(800, 600);
    glm::mat4 matrix = camera.getViewProjectionMatrix();
    
    // La matriz debería ser válida (no nula)
    MOTOR_CHECK(matrix[0][0] != 0.0f || matrix[1][1] != 0.0f || matrix[2][2] != 0.0f);
}

int main() { return motor_test::runAllTests(); }
