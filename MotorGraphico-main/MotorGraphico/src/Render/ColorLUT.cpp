#include "Render/ColorLUT.h"

#include <algorithm>
#include <cmath>

#include <glad/glad.h>

float ColorLUT::quantizeChannel(float value, int levels) {
    if (levels < 2) {
        levels = 2;
    }
    value = std::clamp(value, 0.0f, 1.0f);
    float step = std::round(value * static_cast<float>(levels - 1));
    return step / static_cast<float>(levels - 1);
}

Vector4 ColorLUT::quantizeColor(const Vector4& color, int levels) {
    return Vector4{quantizeChannel(color.x, levels), quantizeChannel(color.y, levels),
                   quantizeChannel(color.z, levels), color.w};
}

ColorLUT::ColorLUT(int levels) : m_levels(levels < 2 ? 2 : levels) {
    const int n = m_levels;
    m_cells.resize(static_cast<std::size_t>(n) * n * n);

    // Cubo identidad cuantizado: para cada celda (r,g,b) del cubo RGB de
    // lado n, su color identidad normalizado es (r,g,b)/(n-1) -- ya cae
    // exactamente en la rejilla de n escalones, asi que quantizeColor()
    // aqui es un no-op matematico, pero se llama igualmente para que la
    // logica de generacion y la de cuantizacion sean una unica fuente de
    // verdad (si algun dia se genera el cubo a partir de otra cosa que no
    // sea la identidad, sigue cuantizando correctamente).
    std::vector<unsigned char> pixels(static_cast<std::size_t>(n) * n * n * 4);
    for (int b = 0; b < n; ++b) {
        for (int g = 0; g < n; ++g) {
            for (int r = 0; r < n; ++r) {
                Vector4 identity{static_cast<float>(r) / static_cast<float>(n - 1),
                                 static_cast<float>(g) / static_cast<float>(n - 1),
                                 static_cast<float>(b) / static_cast<float>(n - 1), 1.0f};
                Vector4 quantized = quantizeColor(identity, n);
                m_cells[static_cast<std::size_t>(b) * n * n + static_cast<std::size_t>(g) * n + r] =
                    quantized;

                // Layout 2D de la textura: el slice "b" (canal azul) ocupa
                // el tile de columnas [b*n, b*n+n); dentro del tile, x=r,
                // y=g. El shader "lut" samplea con este mismo layout (ver
                // assets/shaders/lut.frag).
                std::size_t px = static_cast<std::size_t>(b) * n + static_cast<std::size_t>(r);
                std::size_t py = static_cast<std::size_t>(g);
                std::size_t idx = (py * static_cast<std::size_t>(n) * n + px) * 4;
                pixels[idx + 0] = static_cast<unsigned char>(quantized.x * 255.0f + 0.5f);
                pixels[idx + 1] = static_cast<unsigned char>(quantized.y * 255.0f + 0.5f);
                pixels[idx + 2] = static_cast<unsigned char>(quantized.z * 255.0f + 0.5f);
                pixels[idx + 3] = 255;
            }
        }
    }

    glGenTextures(1, &m_lutTexture);
    glBindTexture(GL_TEXTURE_2D, m_lutTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, n * n, n, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

ColorLUT::~ColorLUT() {
    if (m_lutTexture != 0) {
        glDeleteTextures(1, &m_lutTexture);
    }
}

void ColorLUT::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_lutTexture);
}

Vector4 ColorLUT::cellColor(int r, int g, int b) const {
    r = std::clamp(r, 0, m_levels - 1);
    g = std::clamp(g, 0, m_levels - 1);
    b = std::clamp(b, 0, m_levels - 1);
    return m_cells[static_cast<std::size_t>(b) * m_levels * m_levels +
                   static_cast<std::size_t>(g) * m_levels + static_cast<std::size_t>(r)];
}
