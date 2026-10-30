#pragma once

// Vector 4D basico, usado como color/tint RGBA por SpriteBatch (Fase 4:
// post-procesado e iluminacion). Mismo estilo minimalista que Vector2:
// POD con defaults, para que {1,1,1,1} (blanco opaco, sin cambio) sea el
// tint por defecto en submit()/Entity.
struct Vector4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vector4 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar, w * scalar};
    }
};
