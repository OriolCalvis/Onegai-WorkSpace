#pragma once

// Vector 2D basico, usado por Camera, Entity y el sistema de coordenadas
// pantalla<->mundo<->grid isometrico (ver motor_grafico_clases.puml).
struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    Vector2 operator+(const Vector2& other) const { return {x + other.x, y + other.y}; }
    Vector2 operator-(const Vector2& other) const { return {x - other.x, y - other.y}; }
    Vector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
};
