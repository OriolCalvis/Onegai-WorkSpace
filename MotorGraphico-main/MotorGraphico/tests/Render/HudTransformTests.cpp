// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/HudElement.h"
#include "Core/Math/Vector2.h"

#include "TestHarness.h"

#include <cmath>

// --- Tests de HudTransform::resolveTopLeft ---

MOTOR_TEST_CASE(hud_transform_default_values) {
    HudTransform transform;
    MOTOR_CHECK(transform.anchor == HudAnchor::TopLeft);
    MOTOR_CHECK(transform.offset.x == 0.0f);
    MOTOR_CHECK(transform.offset.y == 0.0f);
    MOTOR_CHECK(transform.size.x == 100.0f);
    MOTOR_CHECK(transform.size.y == 20.0f);
}

MOTOR_TEST_CASE(hud_transform_resolve_top_left_default_anchor) {
    HudTransform transform;
    // anchor = TopLeft, offset = (0,0), viewport = 800x600
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    MOTOR_CHECK(std::abs(topLeft.x) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y) < 0.001f);
}

MOTOR_TEST_CASE(hud_transform_resolve_top_left_with_offset) {
    HudTransform transform;
    transform.offset = Vector2{10.0f, 20.0f};
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    MOTOR_CHECK(std::abs(topLeft.x - 10.0f) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y - 20.0f) < 0.001f);
}

MOTOR_TEST_CASE(hud_transform_resolve_top_right) {
    HudTransform transform;
    transform.anchor = HudAnchor::TopRight;
    transform.offset = Vector2{10.0f, 20.0f};
    
    // TopRight: la esquina superior-derecha del viewport menos el offset
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    // viewportWidth - size.x - offset.x = 800 - 100 - 10 = 690
    // offset.y = 20
    MOTOR_CHECK(std::abs(topLeft.x - (800.0f - 100.0f - 10.0f)) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y - 20.0f) < 0.001f);
}

MOTOR_TEST_CASE(hud_transform_resolve_bottom_left) {
    HudTransform transform;
    transform.anchor = HudAnchor::BottomLeft;
    transform.offset = Vector2{10.0f, 20.0f};
    
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    // offset.x = 10
    // viewportHeight - size.y - offset.y = 600 - 20 - 20 = 560
    MOTOR_CHECK(std::abs(topLeft.x - 10.0f) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y - (600.0f - 20.0f - 20.0f)) < 0.001f);
}

MOTOR_TEST_CASE(hud_transform_resolve_bottom_right) {
    HudTransform transform;
    transform.anchor = HudAnchor::BottomRight;
    transform.offset = Vector2{10.0f, 20.0f};
    
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    // viewportWidth - size.x - offset.x = 800 - 100 - 10 = 690
    // viewportHeight - size.y - offset.y = 600 - 20 - 20 = 560
    MOTOR_CHECK(std::abs(topLeft.x - (800.0f - 100.0f - 10.0f)) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y - (600.0f - 20.0f - 20.0f)) < 0.001f);
}

// EST-33 — LAS CINCO EXPECTATIVAS QUE ESTABAN MAL, Y POR QUE NADIE LO VIO.
//
// Esta bateria se registro en CTest con C-24 y NUNCA se llego a construir:
// el objetivo no enlazaba ninguna libreria, asi que CTest la listaba como
// "Not Run" y parecia cobertura. Al cablearla, cinco casos fallaron. Los
// cinco por lo mismo: el signo del offset en un eje CENTRADO.
//
// Manda la implementacion, y esta documentada en HudElement.cpp: el offset
// empuja HACIA DENTRO de la pantalla en los bordes (sumar arriba/izquierda,
// restar abajo/derecha), y en un eje centrado NO hay una "direccion de
// fuera", asi que se suma tal cual, como ajuste fino. La prueba asumia que
// tambien se restaba. Con esa regla, un offset de +10 en un elemento
// centrado lo movia a la IZQUIERDA, que no es lo que nadie espera.
//
// La prueba se escribio a partir de una formula supuesta y no se ejecuto
// nunca. Es exactamente el fallo que una prueba registrada pero no
// construida garantiza: parece que vigila, y no vigila nada.
MOTOR_TEST_CASE(hud_transform_resolve_center) {
    HudTransform transform;
    transform.anchor = HudAnchor::Center;
    transform.offset = Vector2{10.0f, 20.0f};
    transform.size = Vector2{200.0f, 50.0f};
    
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    // Center: los dos ejes estan centrados, asi que el offset se SUMA
    // (ver la nota de arriba): (400 - 100 + 10, 300 - 25 + 20) = (310, 295)
    float expectedX = 800.0f * 0.5f - 200.0f * 0.5f + 10.0f;
    float expectedY = 600.0f * 0.5f - 50.0f * 0.5f + 20.0f;
    MOTOR_CHECK(std::abs(topLeft.x - expectedX) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y - expectedY) < 0.001f);
}

MOTOR_TEST_CASE(hud_transform_resolve_top_center) {
    HudTransform transform;
    transform.anchor = HudAnchor::TopCenter;
    transform.offset = Vector2{10.0f, 20.0f};
    transform.size = Vector2{200.0f, 50.0f};
    
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    // TopCenter: eje X centrado (offset sumado), eje Y pegado arriba
    // (offset empuja hacia dentro, o sea tambien sumado).
    float expectedX = 800.0f * 0.5f - 200.0f * 0.5f + 10.0f;
    float expectedY = 20.0f;
    MOTOR_CHECK(std::abs(topLeft.x - expectedX) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y - expectedY) < 0.001f);
}

MOTOR_TEST_CASE(hud_transform_resolve_center_left) {
    HudTransform transform;
    transform.anchor = HudAnchor::CenterLeft;
    transform.offset = Vector2{10.0f, 20.0f};
    transform.size = Vector2{200.0f, 50.0f};
    
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    // CenterLeft: eje X pegado a la izquierda, eje Y centrado (sumado).
    float expectedX = 10.0f;
    float expectedY = 600.0f * 0.5f - 50.0f * 0.5f + 20.0f;
    MOTOR_CHECK(std::abs(topLeft.x - expectedX) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y - expectedY) < 0.001f);
}

MOTOR_TEST_CASE(hud_transform_resolve_center_right) {
    HudTransform transform;
    transform.anchor = HudAnchor::CenterRight;
    transform.offset = Vector2{10.0f, 20.0f};
    transform.size = Vector2{200.0f, 50.0f};
    
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    // CenterRight: eje X pegado a la derecha (offset RESTADO, empuja hacia
    // dentro), eje Y centrado (sumado).
    float expectedX = 800.0f - 200.0f - 10.0f;
    float expectedY = 600.0f * 0.5f - 50.0f * 0.5f + 20.0f;
    MOTOR_CHECK(std::abs(topLeft.x - expectedX) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y - expectedY) < 0.001f);
}

MOTOR_TEST_CASE(hud_transform_resolve_bottom_center) {
    HudTransform transform;
    transform.anchor = HudAnchor::BottomCenter;
    transform.offset = Vector2{10.0f, 20.0f};
    transform.size = Vector2{200.0f, 50.0f};
    
    Vector2 topLeft = transform.resolveTopLeft(800, 600);
    // BottomCenter: eje X centrado (sumado), eje Y pegado abajo (restado).
    float expectedX = 800.0f * 0.5f - 200.0f * 0.5f + 10.0f;
    float expectedY = 600.0f - 50.0f - 20.0f;
    MOTOR_CHECK(std::abs(topLeft.x - expectedX) < 0.001f);
    MOTOR_CHECK(std::abs(topLeft.y - expectedY) < 0.001f);
}

// --- Tests de todos los anclajes ---

MOTOR_TEST_CASE(hud_transform_all_anchors_are_distinct) {
    const int viewportW = 800;
    const int viewportH = 600;
    const Vector2 offset{10.0f, 20.0f};
    const Vector2 size{100.0f, 50.0f};
    
    // Probar todos los anclajes
    HudAnchor anchors[] = {
        HudAnchor::TopLeft, HudAnchor::TopCenter, HudAnchor::TopRight,
        HudAnchor::CenterLeft, HudAnchor::Center, HudAnchor::CenterRight,
        HudAnchor::BottomLeft, HudAnchor::BottomCenter, HudAnchor::BottomRight
    };
    
    std::vector<Vector2> positions;
    for (HudAnchor anchor : anchors) {
        HudTransform transform;
        transform.anchor = anchor;
        transform.offset = offset;
        transform.size = size;
        positions.push_back(transform.resolveTopLeft(viewportW, viewportH));
    }
    
    // Verificar que todas las posiciones son distintas
    for (std::size_t i = 0; i < positions.size(); ++i) {
        for (std::size_t j = i + 1; j < positions.size(); ++j) {
            MOTOR_CHECK(positions[i].x != positions[j].x || positions[i].y != positions[j].y);
        }
    }
}

// --- Tests de determinismo ---

MOTOR_TEST_CASE(hud_transform_resolve_is_deterministic) {
    HudTransform transform;
    transform.anchor = HudAnchor::Center;
    transform.offset = Vector2{5.0f, 10.0f};
    transform.size = Vector2{200.0f, 100.0f};
    
    Vector2 pos1 = transform.resolveTopLeft(1024, 768);
    Vector2 pos2 = transform.resolveTopLeft(1024, 768);
    
    MOTOR_CHECK(std::abs(pos1.x - pos2.x) < 0.001f);
    MOTOR_CHECK(std::abs(pos1.y - pos2.y) < 0.001f);
}

int main() { return motor_test::runAllTests(); }
