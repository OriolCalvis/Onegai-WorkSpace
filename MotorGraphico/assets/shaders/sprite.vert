#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
// Tint RGBA por vertice (Fase 4): lo aporta SpriteBatch (blanco por
// defecto = sin cambio) y Entity::render() (m_tint).
layout(location = 2) in vec4 aColor;

uniform mat4 uViewProjection;

out vec2 vUV;
out vec4 vColor;

void main() {
    vUV = aUV;
    vColor = aColor;
    gl_Position = uViewProjection * vec4(aPos, 0.0, 1.0);
}
