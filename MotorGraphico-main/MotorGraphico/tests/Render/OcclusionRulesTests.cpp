// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/OcclusionRules.h"
#include "Core/Math/GridCoord.h"

#include "TestHarness.h"

// --- Tests de OcclusionRules::shouldFadeTile ---

MOTOR_TEST_CASE(occlusion_should_not_fade_non_blocking_tile) {
    GridCoord focus{5, 5};
    GridCoord tile{6, 5};
    bool blocksMovement = false;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result);
}

MOTOR_TEST_CASE(occlusion_should_not_fade_when_depth_zero) {
    GridCoord focus{5, 5};
    // depth = (tile.x + tile.y) - (focus.x + focus.y)
    // Para depth = 0: tile.x + tile.y = focus.x + focus.y
    GridCoord tile{6, 4}; // 6+4 = 10, focus 5+5 = 10, depth = 0
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result);
}

MOTOR_TEST_CASE(occlusion_should_not_fade_when_depth_negative) {
    GridCoord focus{5, 5};
    // depth negativo = detrás del foco
    GridCoord tile{4, 4}; // 4+4 = 8, focus 5+5 = 10, depth = -2
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result);
}

MOTOR_TEST_CASE(occlusion_should_fade_when_depth_one_and_lateral_zero) {
    GridCoord focus{5, 5};
    GridCoord tile{6, 5}; // 6+5 = 11, focus 5+5 = 10, depth = 1
    // lateral = (6-5) - (5-5) = 1 - 0 = 1, lateralDistance = 1
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(result);
}

// AV16-13 — A dos casillas de lado (depth 2, lateral 2) ya no tapa al jugador: no se funde.
MOTOR_TEST_CASE(occlusion_should_not_fade_when_depth_two_and_lateral_two) {
    GridCoord focus{5, 5};
    GridCoord tile{7, 5}; // 7+5 = 12, focus 5+5 = 10, depth = 2
    // lateral = (7-5) - (5-5) = 2 - 0 = 2, lateralDistance = 2 > kLateralMaximo
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result);
    MOTOR_CHECK(!OcclusionRules::shouldFadeTile(focus, GridCoord{5, 7}, true));  // el otro lado, igual
}

MOTOR_TEST_CASE(occlusion_should_not_fade_when_lateral_distance_exceeds_two_with_depth_one) {
    GridCoord focus{5, 5};
    GridCoord tile{7, 4}; // 7+4 = 11, focus 5+5 = 10, depth = 1
    // lateral = (7-4) - (5-5) = 3 - 0 = 3, lateralDistance = 3
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result); // lateralDistance = 3 > 2, no debería fadear
}

MOTOR_TEST_CASE(occlusion_should_not_fade_when_depth_three) {
    GridCoord focus{5, 5};
    GridCoord tile{8, 5}; // 8+5 = 13, focus 5+5 = 10, depth = 3
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result); // depth > 2
}

MOTOR_TEST_CASE(occlusion_should_not_fade_when_lateral_distance_three) {
    GridCoord focus{5, 5};
    GridCoord tile{5, 8}; // 5+8 = 13, focus 5+5 = 10, depth = 3
    // Pero lateral: (5-8) - (5-5) = -3 - 0 = -3, lateralDistance = 3
    // depth = 3 que ya es > 2
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result);
}

MOTOR_TEST_CASE(occlusion_should_fade_when_depth_two_and_lateral_two) {
    GridCoord focus{5, 5};
    GridCoord tile{7, 4}; // 7+4 = 11, focus 5+5 = 10, depth = 1
    // lateral = (7-4) - (5-5) = 3 - 0 = 3, lateralDistance = 3
    // depth = 1, lateralDistance = 3 -> !result
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result);
}

MOTOR_TEST_CASE(occlusion_should_fade_when_depth_two_lateral_one) {
    GridCoord focus{5, 5};
    GridCoord tile{6, 6}; // 6+6 = 12, focus 5+5 = 10, depth = 2
    // lateral = (6-6) - (5-5) = 0 - 0 = 0, lateralDistance = 0
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(result);
}

MOTOR_TEST_CASE(occlusion_should_not_fade_when_depth_exceeds_two) {
    GridCoord focus{5, 5};
    GridCoord tile{8, 5}; // 8+5 = 13, focus 5+5 = 10, depth = 3
    // depth = 3 > 2, así que no aplica
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result);
}

MOTOR_TEST_CASE(occlusion_edge_case_focus_at_origin) {
    GridCoord focus{0, 0};
    GridCoord tile{1, 0}; // depth = 1, lateral = 1
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(result);
}

MOTOR_TEST_CASE(occlusion_edge_case_tile_at_origin) {
    GridCoord focus{1, 1};
    GridCoord tile{0, 0}; // depth = -2 (detrás)
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(!result);
}

MOTOR_TEST_CASE(occlusion_diagonal_movement) {
    GridCoord focus{5, 5};
    GridCoord tile{6, 6}; // depth = 2, lateral = 0
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(result);
}

MOTOR_TEST_CASE(occlusion_negative_coordinates) {
    GridCoord focus{-5, -5};
    GridCoord tile{-4, -5}; // depth = 1, lateral = 1
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(result);
}

MOTOR_TEST_CASE(occlusion_large_coordinates) {
    GridCoord focus{100, 100};
    GridCoord tile{101, 100}; // depth = 1, lateral = 1
    bool blocksMovement = true;
    
    bool result = OcclusionRules::shouldFadeTile(focus, tile, blocksMovement);
    MOTOR_CHECK(result);
}

// --- AV15-03: los objetos de decorado que bloquean tambien se funden ---

MOTOR_TEST_CASE(occlusion_prop_in_front_of_player_fades) {
    const GridCoord focus{10, 10};
    MOTOR_CHECK(OcclusionRules::shouldFadeProp(focus, GridCoord{11, 10}, true, false));  // depth 1, lateral 1
    MOTOR_CHECK(OcclusionRules::shouldFadeProp(focus, GridCoord{11, 11}, true, false));  // depth 2, lateral 0
    MOTOR_CHECK(OcclusionRules::shouldFadeProp(focus, GridCoord{12, 11}, true, false));  // depth 3, lateral 1
}

MOTOR_TEST_CASE(occlusion_prop_window_is_one_row_deeper_than_tiles) {
    const GridCoord focus{10, 10};
    const GridCoord tres{12, 11};  // depth 3
    MOTOR_CHECK(!OcclusionRules::shouldFadeTile(focus, tres, true));
    MOTOR_CHECK(OcclusionRules::shouldFadeProp(focus, tres, true, false));
    MOTOR_CHECK(!OcclusionRules::shouldFadeProp(focus, GridCoord{12, 12}, true, false));  // depth 4: ya no tapa
}

MOTOR_TEST_CASE(occlusion_prop_behind_or_beside_never_fades) {
    const GridCoord focus{10, 10};
    MOTOR_CHECK(!OcclusionRules::shouldFadeProp(focus, GridCoord{10, 10}, true, false));  // su propia celda
    MOTOR_CHECK(!OcclusionRules::shouldFadeProp(focus, GridCoord{9, 10}, true, false));   // detras
    MOTOR_CHECK(!OcclusionRules::shouldFadeProp(focus, GridCoord{11, 9}, true, false));   // depth 0
    MOTOR_CHECK(!OcclusionRules::shouldFadeProp(focus, GridCoord{14, 8}, true, false));   // depth 2 pero muy a un lado
    MOTOR_CHECK(!OcclusionRules::shouldFadeProp(focus, GridCoord{12, 10}, true, false));  // depth 2, lateral 2: al lado, no tapa
    MOTOR_CHECK(!OcclusionRules::shouldFadeProp(focus, GridCoord{10, 12}, true, false));
}

MOTOR_TEST_CASE(occlusion_prop_only_fades_if_it_blocks_and_is_not_an_actor) {
    const GridCoord focus{10, 10};
    const GridCoord delante{11, 11};
    MOTOR_CHECK(!OcclusionRules::shouldFadeProp(focus, delante, false, false));  // flores, tendederos: no bloquean
    MOTOR_CHECK(!OcclusionRules::shouldFadeProp(focus, delante, true, true));    // un PNJ no se funde
    MOTOR_CHECK(OcclusionRules::shouldFadeProp(focus, delante, true, false));
    MOTOR_CHECK(OcclusionRules::kAlfaProp > 0.0f && OcclusionRules::kAlfaProp < 1.0f);
}

MOTOR_TEST_CASE(occlusion_prop_window_matches_blocks_window) {
    // BloquesNivel::atenua recorre profundidad 1..3 y lateral -kLateralMaximo..kLateralMaximo (con la paridad de la rejilla): mismas celdas.
    const GridCoord focus{20, 20};
    int celdas = 0;
    for (int y = 0; y < 40; ++y)
        for (int x = 0; x < 40; ++x)
            if (OcclusionRules::propInFadeWindow(focus, GridCoord{x, y})) ++celdas;
    int esperadas = 0;
    for (int profundidad = 1; profundidad <= 3; ++profundidad)
        for (int lateral = -OcclusionRules::kLateralMaximo; lateral <= OcclusionRules::kLateralMaximo; ++lateral)
            if (((profundidad + lateral) & 1) == 0) ++esperadas;
    MOTOR_CHECK(celdas == esperadas);
}

int main() { return motor_test::runAllTests(); }
