#pragma once

#include <vector>

#include "Core/Math/Vector4.h"

// Paletizado / LUT de color (Fase 4, tercer efecto de pulido visual):
// reduce la escena a una paleta de "levels" tonos por canal RGB, estilo
// pixel art de paleta limitada (ver motor_grafico_dafo.md: "el estilo
// pixel art con paleta limitada reduce drasticamente el coste de
// produccion artistica"). Post-procesado por lookup, mismo patron
// FBO+fullscreen-triangle que lightmap (iluminacion) y fog (niebla):
// IsometricRenderer::applyPostProcessing() dibuja la escena a un FBO y un
// pase con el shader "lut" remapea cada pixel a traves de esta tabla.
//
// Tecnica: LUT de color 3D estandar (color grading), codificada como
// textura 2D (cubo RGB de lado N desenrollado en una fila de N tiles de
// NxN -- ver el constructor). levels=N es a la vez la resolucion del cubo
// Y el numero de tonos por canal tras la cuantizacion (ver
// quantizeChannel()): con levels=4 cada canal solo puede tomar 4 valores
// (paleta muy limitada, pixel art marcado); con levels=16 el resultado es
// casi identidad (perdida minima).
//
// La cuantizacion (quantizeChannel/quantizeColor) es pura, sin GL: se
// puede testear sin contexto OpenGL (mismo criterio que
// FogOfWar::stateAt), y es la misma funcion que rellena la textura del
// cubo identidad en el constructor.
class ColorLUT {
public:
    // Cuantiza un canal en [0,1] a "levels" escalones equiespaciados
    // (levels < 2 se trata como 2). Ej. levels=4: 0.5 -> 2/3 (escalon mas
    // cercano de {0, 1/3, 2/3, 1}).
    static float quantizeChannel(float value, int levels);
    static Vector4 quantizeColor(const Vector4& color, int levels);

    // levels: resolucion del cubo y tonos por canal (ver arriba, minimo
    // 2). Crea la textura GL (2D, levels*levels de ancho x levels de
    // alto, RGBA8): GL_NEAREST para bordes nitidos entre escalones
    // (coherente con el look de paleta limitada; GL_LINEAR suavizaria el
    // posterizado, que es justo el punto del efecto) y GL_CLAMP_TO_EDGE.
    explicit ColorLUT(int levels);
    ~ColorLUT();

    ColorLUT(const ColorLUT&) = delete;
    ColorLUT& operator=(const ColorLUT&) = delete;
    ColorLUT(ColorLUT&&) = delete;
    ColorLUT& operator=(ColorLUT&&) = delete;

    // Bindea la textura de la LUT en "slot" para que el shader "lut" la
    // samplee (igual que Texture::bind).
    void bind(unsigned int slot) const;

    int levels() const { return m_levels; }

    // Solo lectura, para tests sin necesidad de leer la textura GL: color
    // cuantizado que esta LUT asigna a la celda identidad (r,g,b) del
    // cubo, en [0, levels). Fuera de rango se clampa (mismo criterio
    // permisivo que FogOfWar::stateAt: no lanza).
    Vector4 cellColor(int r, int g, int b) const;

private:
    int m_levels;
    // Cubo identidad cuantizado, [b*levels*levels + g*levels + r]: mismo
    // contenido que se sube a m_lutTexture, guardado en CPU para poder
    // testear cellColor() sin leer la textura GL.
    std::vector<Vector4> m_cells;
    unsigned int m_lutTexture = 0;
};
