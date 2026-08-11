#include "Render/DynamicLightMap.h"

#include "Core/Resources/Texture.h"
#include "Render/Camera.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <glad/glad.h>

namespace {

// Textura RGBA8 al tamano del viewport, inicializada a negro (sin luz),
// GL_LINEAR/GL_CLAMP_TO_EDGE: mismos parametros que la textura de
// FogOfWar (ver su constructor y el porque de cada uno).
Texture* createLightTexture(int width, int height) {
    unsigned int glID = 0;
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);
    std::vector<unsigned char> black(static_cast<std::size_t>(width) * height * 4, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                black.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return new Texture(glID, width, height);
}

}  // namespace

DynamicLightMap::DynamicLightMap(int viewportWidth, int viewportHeight)
    : m_viewportWidth(viewportWidth)
    , m_viewportHeight(viewportHeight)
    , m_texture(createLightTexture(viewportWidth, viewportHeight)) {}

DynamicLightMap::~DynamicLightMap() { delete m_texture; }

float DynamicLightMap::attenuationAt(float distance, float radius) {
    if (radius <= 0.0f) {
        return 0.0f;  // luz degenerada: apagada, no division por cero
    }
    return std::clamp(1.0f - distance / radius, 0.0f, 1.0f);
}

Vector4 DynamicLightMap::sampleLightAt(const std::vector<PointLight>& lights,
                                       const Vector2& worldPos) {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    for (const PointLight& light : lights) {
        float dx = worldPos.x - light.worldPos.x;
        float dy = worldPos.y - light.worldPos.y;
        float att = attenuationAt(std::sqrt(dx * dx + dy * dy), light.radius) * light.intensity;
        r += light.color.x * att;
        g += light.color.y * att;
        b += light.color.z * att;
    }
    return Vector4{std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f),
                   std::clamp(b, 0.0f, 1.0f), 1.0f};
}

void DynamicLightMap::updateTexture(const Camera& camera) {
    // Acumulador en float, no directamente bytes: dos luces solapadas se
    // SUMAN antes de clampear (mismo resultado que sampleLightAt() -- el
    // pintado por bounding-box es solo una optimizacion, no otro modelo).
    std::vector<float> accum(static_cast<std::size_t>(m_viewportWidth) * m_viewportHeight * 3,
                             0.0f);

    // Conversion mundo->pantalla, identica a FogOfWar::updateTexture()
    // (ver su comentario): screen = (world - camPos) * zoom + viewport/2.
    const float zoom = camera.zoom();
    const float halfVpX = static_cast<float>(m_viewportWidth) * 0.5f;
    const float halfVpY = static_cast<float>(m_viewportHeight) * 0.5f;

    for (const PointLight& light : m_lights) {
        float screenX = (light.worldPos.x - camera.position().x) * zoom + halfVpX;
        float screenY = (light.worldPos.y - camera.position().y) * zoom + halfVpY;
        float screenRadius = light.radius * zoom;
        if (screenRadius <= 0.0f) {
            continue;
        }

        // Bounding-box de la luz en pantalla, recortado al viewport: solo
        // se recorren los pixels que la luz puede tocar.
        int x0 = std::max(0, static_cast<int>(screenX - screenRadius));
        int x1 = std::min(m_viewportWidth, static_cast<int>(screenX + screenRadius) + 1);
        int yWin0 = std::max(0, static_cast<int>(screenY - screenRadius));
        int yWin1 = std::min(m_viewportHeight, static_cast<int>(screenY + screenRadius) + 1);

        for (int yWin = yWin0; yWin < yWin1; ++yWin) {
            // Flip Y ventana->textura GL: fila 0 del buffer = abajo (ver
            // el comentario largo en FogOfWar::updateTexture()).
            int row = m_viewportHeight - 1 - yWin;
            for (int px = x0; px < x1; ++px) {
                float dx = static_cast<float>(px) - screenX;
                float dy = static_cast<float>(yWin) - screenY;
                // attenuationAt() sobre distancias de PANTALLA con radio
                // de PANTALLA: al ser una caida por cociente d/r, es
                // identico a hacerlo en mundo (ambos escalan por zoom).
                float att = attenuationAt(std::sqrt(dx * dx + dy * dy), screenRadius) *
                            light.intensity;
                if (att <= 0.0f) {
                    continue;  // esquina del bounding-box fuera del circulo
                }
                std::size_t idx = (static_cast<std::size_t>(row) * m_viewportWidth + px) * 3;
                accum[idx + 0] += light.color.x * att;
                accum[idx + 1] += light.color.y * att;
                accum[idx + 2] += light.color.z * att;
            }
        }
    }

    // float acumulado -> RGBA8 clampado.
    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(m_viewportWidth) * m_viewportHeight * 4);
    for (std::size_t i = 0; i < accum.size() / 3; ++i) {
        pixels[i * 4 + 0] = static_cast<unsigned char>(std::clamp(accum[i * 3 + 0], 0.0f, 1.0f) *
                                                       255.0f);
        pixels[i * 4 + 1] = static_cast<unsigned char>(std::clamp(accum[i * 3 + 1], 0.0f, 1.0f) *
                                                       255.0f);
        pixels[i * 4 + 2] = static_cast<unsigned char>(std::clamp(accum[i * 3 + 2], 0.0f, 1.0f) *
                                                       255.0f);
        pixels[i * 4 + 3] = 255;
    }

    // Re-subida completa via el bind de la Texture propietaria (Texture no
    // expone su id GL y no hace falta: bind() deja GL_TEXTURE_2D apuntando
    // a ella). glTexImage2D y no SubImage: mismo criterio que FogOfWar.
    m_texture->bind(0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_viewportWidth, m_viewportHeight, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}
