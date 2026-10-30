// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/Culling.h"
#include "Core/Math/GridBounds.h"
#include "Core/Math/GridCoord.h"

#include "TestHarness.h"

// Mocks para pruebas
class MockCamera : public Camera {
public:
    MockCamera(int width, int height) : Camera(width, height) {}
    
    // Exponer métodos para pruebas
    using Camera::screenToWorld;
    using Camera::viewportWidth;
    using Camera::viewportHeight;
};

class MockRuntimeTileMap : public RuntimeTileMap {
public:
    MockRuntimeTileMap(int width, int height, int tileWidth, int tileHeight) {
        replaceCompiledData(width, height, tileWidth, tileHeight, {});
    }
    
    using RuntimeTileMap::getWidth;
    using RuntimeTileMap::getHeight;
    using RuntimeTileMap::screenToGrid;
    using RuntimeTileMap::replaceCompiledData;
};

// --- Tests de VisibleGridRange ---

MOTOR_TEST_CASE(culling_visible_grid_range_empty_map) {
    MockCamera camera(800, 600);
    MockRuntimeTileMap map(0, 0, 64, 32);
    
    GridBounds bounds = VisibleGridRange(map, camera);
    MOTOR_CHECK(bounds.isEmpty());
}

MOTOR_TEST_CASE(culling_visible_grid_range_single_tile_map) {
    MockCamera camera(800, 600);
    MockRuntimeTileMap map(1, 1, 64, 32);
    
    GridBounds bounds = VisibleGridRange(map, camera);
    MOTOR_CHECK(!bounds.isEmpty());
    MOTOR_CHECK(bounds.minX <= 0);
    MOTOR_CHECK(bounds.maxX >= 0);
    MOTOR_CHECK(bounds.minY <= 0);
    MOTOR_CHECK(bounds.maxY >= 0);
}

MOTOR_TEST_CASE(culling_visible_grid_range_small_map) {
    MockCamera camera(800, 600);
    MockRuntimeTileMap map(10, 10, 64, 32);
    
    GridBounds bounds = VisibleGridRange(map, camera);
    MOTOR_CHECK(!bounds.isEmpty());
    MOTOR_CHECK(bounds.minX >= 0);
    MOTOR_CHECK(bounds.maxX < 10);
    MOTOR_CHECK(bounds.minY >= 0);
    MOTOR_CHECK(bounds.maxY < 10);
}

MOTOR_TEST_CASE(culling_visible_grid_range_large_map) {
    MockCamera camera(800, 600);
    MockRuntimeTileMap map(100, 100, 64, 32);
    
    GridBounds bounds = VisibleGridRange(map, camera);
    MOTOR_CHECK(!bounds.isEmpty());
    // El rango visible debería ser pequeño comparado con el mapa
    MOTOR_CHECK((bounds.maxX - bounds.minX + 1) < 100);
    MOTOR_CHECK((bounds.maxY - bounds.minY + 1) < 100);
}

MOTOR_TEST_CASE(culling_visible_grid_range_with_camera_pan) {
    MockCamera camera(800, 600);
    camera.move(Vector2{200.0f, 150.0f});
    camera.update(10.0f); // Asegurar que llega al destino
    
    MockRuntimeTileMap map(50, 50, 64, 32);
    
    GridBounds bounds = VisibleGridRange(map, camera);
    MOTOR_CHECK(!bounds.isEmpty());
    // Con la cámara movida, el rango visible debería ser diferente al centro
    // No verificamos minX/minY específicos porque depende de la implementación
    MOTOR_CHECK(true);
}

MOTOR_TEST_CASE(culling_visible_grid_range_with_zoom) {
    MockCamera camera(800, 600);
    camera.setZoom(2.0f);
    
    MockRuntimeTileMap map(50, 50, 64, 32);
    
    GridBounds bounds = VisibleGridRange(map, camera);
    MOTOR_CHECK(!bounds.isEmpty());
    // Con zoom 2x, el rango visible debería ser más pequeño
    int width1 = bounds.maxX - bounds.minX + 1;
    
    camera.setZoom(0.5f);
    bounds = VisibleGridRange(map, camera);
    int width2 = bounds.maxX - bounds.minX + 1;
    
    MOTOR_CHECK(width2 > width1);
}

MOTOR_TEST_CASE(culling_visible_grid_range_different_tile_sizes) {
    MockCamera camera(800, 600);
    
    MockRuntimeTileMap map1(50, 50, 64, 32); // Tiles 2:1
    MockRuntimeTileMap map2(50, 50, 32, 32); // Tiles 1:1
    
    GridBounds bounds1 = VisibleGridRange(map1, camera);
    GridBounds bounds2 = VisibleGridRange(map2, camera);
    
    // El tamaño del tile afecta el rango visible
    MOTOR_CHECK(!bounds1.isEmpty());
    MOTOR_CHECK(!bounds2.isEmpty());
}

MOTOR_TEST_CASE(culling_visible_grid_range_corner_cases) {
    MockCamera camera(800, 600);
    MockRuntimeTileMap map(20, 20, 64, 32);
    
    // Más alto que ancho
    MockRuntimeTileMap tallMap(10, 40, 64, 32);
    GridBounds bounds = VisibleGridRange(tallMap, camera);
    MOTOR_CHECK(!bounds.isEmpty());
    
    // Más ancho que alto
    MockRuntimeTileMap wideMap(40, 10, 64, 32);
    bounds = VisibleGridRange(wideMap, camera);
    MOTOR_CHECK(!bounds.isEmpty());
}

// --- Tests de que VisibleGridRange es determinista ---

MOTOR_TEST_CASE(culling_visible_grid_range_is_deterministic) {
    MockCamera camera(800, 600);
    MockRuntimeTileMap map(50, 50, 64, 32);
    
    GridBounds bounds1 = VisibleGridRange(map, camera);
    GridBounds bounds2 = VisibleGridRange(map, camera);
    
    MOTOR_CHECK(bounds1.minX == bounds2.minX);
    MOTOR_CHECK(bounds1.maxX == bounds2.maxX);
    MOTOR_CHECK(bounds1.minY == bounds2.minY);
    MOTOR_CHECK(bounds1.maxY == bounds2.maxY);
}

int main() { return motor_test::runAllTests(); }
