// E10-ENCUADRAR-MAPA: comprobar lo que se ve, usando la proyeccion del
// renderer como oraculo; no repetir la formula de encuadre del producto.
#include "Editor/MapFraming.h"
#include "Core/Math/IsoMath.h"
#include "TestHarness.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace {
constexpr float tolerance = 0.002f;

void near(float actual, float expected) {
    MOTOR_CHECK(std::fabs(actual - expected) < tolerance);
}

Vector2 project(Vector2 world, const Editor::MapFraming& framing,
                int viewportWidth, int viewportHeight) {
    return {(world.x - framing.center.x) * framing.zoom + viewportWidth * 0.5f,
            (world.y - framing.center.y) * framing.zoom + viewportHeight * 0.5f};
}

// Cada baldosa se entrega a SpriteBatch como un rectangulo, no como un
// punto. Revisamos sus cuatro vertices, incluidos los bordes exteriores.
void checkVisibleTiles(int columns, int rows, int tileWidth, int tileHeight,
                       int viewportWidth, int viewportHeight) {
    const auto result = Editor::fitMap(columns, rows, tileWidth, tileHeight,
                                       viewportWidth, viewportHeight);
    MOTOR_CHECK(result.isOk());
    const auto& framing = result.value();
    MOTOR_CHECK(framing.zoom > 0.15f && framing.zoom <= 1.0f);
    float left = std::numeric_limits<float>::max();
    float top = std::numeric_limits<float>::max();
    float right = std::numeric_limits<float>::lowest();
    float bottom = std::numeric_limits<float>::lowest();
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            const auto tile = IsoMath::gridToScreen({x, y}, tileWidth, tileHeight);
            for (const int dx : {0, tileWidth}) {
                for (const int dy : {0, tileHeight}) {
                    const auto point = project({tile.x + dx, tile.y + dy}, framing,
                                               viewportWidth, viewportHeight);
                    MOTOR_CHECK(point.x >= viewportWidth * 0.05f - tolerance);
                    MOTOR_CHECK(point.x <= viewportWidth * 0.95f + tolerance);
                    MOTOR_CHECK(point.y >= viewportHeight * 0.05f - tolerance);
                    MOTOR_CHECK(point.y <= viewportHeight * 0.95f + tolerance);
                    left = std::min(left, point.x);
                    right = std::max(right, point.x);
                    top = std::min(top, point.y);
                    bottom = std::max(bottom, point.y);
                }
            }
        }
    }
    near(left, viewportWidth - right);
    near(top, viewportHeight - bottom);
    // Sin saturacion, debe aprovechar el espacio disponible en un eje.
    // Una implementacion que reduzca siempre el mapa de mas no pasa.
    if (framing.zoom < 1.0f) {
        const bool fillsWidth = std::fabs(left - viewportWidth * 0.05f) < tolerance;
        const bool fillsHeight = std::fabs(top - viewportHeight * 0.05f) < tolerance;
        MOTOR_CHECK(fillsWidth || fillsHeight);
    }
}

MOTOR_TEST_CASE(one_tile_centers_its_rectangle_without_magnifying_it) {
    const auto result = Editor::fitMap(1, 1, 64, 32, 800, 600);
    MOTOR_CHECK(result.isOk());
    near(result.value().center.x, 32.0f);
    near(result.value().center.y, 16.0f);
    near(result.value().zoom, 1.0f);
    checkVisibleTiles(1, 1, 64, 32, 800, 600);
}

MOTOR_TEST_CASE(rectangular_maps_and_single_tile_strips_keep_every_border_visible) {
    for (const auto dimensions : {std::array<int, 2>{1, 24}, {24, 1}, {8, 31}, {31, 8}})
        checkVisibleTiles(dimensions[0], dimensions[1], 64, 32, 700, 400);
    checkVisibleTiles(32, 15, 128, 80, 1280, 720);
}

MOTOR_TEST_CASE(large_square_fits_with_margin_on_all_four_sides) {
    checkVisibleTiles(64, 64, 64, 32, 960, 720);
}

MOTOR_TEST_CASE(narrow_viewport_uses_available_width_instead_of_clipping_sides) {
    checkVisibleTiles(60, 12, 64, 32, 480, 720);
}

MOTOR_TEST_CASE(double_density_preserves_logical_projection_when_fit_is_not_saturated) {
    const auto normal = Editor::fitMap(64, 64, 64, 32, 960, 720);
    const auto retina = Editor::fitMap(64, 64, 64, 32, 1920, 1440);
    MOTOR_CHECK(normal.isOk() && retina.isOk());
    MOTOR_CHECK(normal.value().zoom > 0.15f && retina.value().zoom < 1.0f);
    near(normal.value().center.x, retina.value().center.x);
    near(normal.value().center.y, retina.value().center.y);
    near(retina.value().zoom, 2 * normal.value().zoom);
    for (const auto cell : {GridCoord{0, 0}, {63, 0}, {0, 63}, {63, 63}, {17, 42}}) {
        const auto world = IsoMath::gridToScreen(cell, 64, 32);
        const auto logical = project(world, normal.value(), 960, 720);
        const auto physical = project(world, retina.value(), 1920, 1440);
        near(logical.x, physical.x / 2);
        near(logical.y, physical.y / 2);
    }
    checkVisibleTiles(64, 64, 64, 32, 1920, 1440);
}

MOTOR_TEST_CASE(small_map_keeps_native_zoom_even_when_the_viewport_has_space) {
    const auto result = Editor::fitMap(3, 2, 64, 32, 1920, 1080);
    MOTOR_CHECK(result.isOk());
    near(result.value().zoom, 1.0f);
    checkVisibleTiles(3, 2, 64, 32, 1920, 1080);
}

MOTOR_TEST_CASE(huge_map_keeps_the_existing_wheel_minimum_and_a_finite_center) {
    const auto result = Editor::fitMap(100000, 80000, 64, 32, 800, 600);
    MOTOR_CHECK(result.isOk());
    near(result.value().zoom, 0.15f);
    MOTOR_CHECK(std::isfinite(result.value().center.x));
    MOTOR_CHECK(std::isfinite(result.value().center.y));
    // Con el minimo alcanzado el mapa puede no caber; no se promete un
    // encuadre total a costa de producir un salto al usar la rueda.
    const auto farCorner = IsoMath::gridToScreen({99999, 79999}, 64, 32);
    const auto point = project({farCorner.x + 64, farCorner.y + 32}, result.value(), 800, 600);
    MOTOR_CHECK(point.y > 600);
}

MOTOR_TEST_CASE(nonpositive_dimensions_return_a_diagnostic_instead_of_a_framing) {
    const std::array<int, 6> valid{64, 64, 64, 32, 960, 720};
    for (std::size_t index = 0; index < valid.size(); ++index) {
        for (const int invalid : {0, -1, std::numeric_limits<int>::min()}) {
            auto input = valid;
            input[index] = invalid;
            const auto result = Editor::fitMap(input[0], input[1], input[2], input[3],
                                               input[4], input[5]);
            MOTOR_CHECK(!result.isOk());
            MOTOR_CHECK(!result.errorMessage().empty());
        }
    }
}
} // namespace

int main() { return motor_test::runAllTests(); }
