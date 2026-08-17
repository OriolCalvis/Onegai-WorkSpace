#pragma once

// Region de coordenadas UV (0..1) dentro de un atlas de texturas, usada
// por TextureAtlas::getUV() para recortar un tile concreto (ver Fase 2
// del Gantt: "Carga de atlas de tiles").
struct UVRect {
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
};
