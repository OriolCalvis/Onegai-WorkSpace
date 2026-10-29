#include "Render/BlobShadow.h"

#include "Core/Resources/Texture.h"

#include <algorithm>
#include <cmath>

#include <glad/glad.h>

void FillBlobShadowPixels(std::vector<unsigned char>& pixels, int size) {
    pixels.clear();
    if (size <= 0) {
        return;
    }
    pixels.resize(static_cast<std::size_t>(size) * size * 4);

    const float center = (static_cast<float>(size) - 1.0f) * 0.5f;
    const float maxDist = static_cast<float>(size) * 0.5f;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float dx = static_cast<float>(x) - center;
            float dy = static_cast<float>(y) - center;
            float t = std::clamp(1.0f - std::sqrt(dx * dx + dy * dy) / maxDist, 0.0f, 1.0f);
            // t*t: caida cuadratica -- centro denso, borde que se funde
            // (ver el comentario del header).
            unsigned char alpha = static_cast<unsigned char>(t * t * 255.0f);
            std::size_t idx = (static_cast<std::size_t>(y) * size + x) * 4;
            pixels[idx + 0] = 255;
            pixels[idx + 1] = 255;
            pixels[idx + 2] = 255;
            pixels[idx + 3] = alpha;
        }
    }
}

Texture* CreateBlobShadowTexture(int size) {
    std::vector<unsigned char> pixels;
    FillBlobShadowPixels(pixels, size);

    unsigned int glID = 0;
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return new Texture(glID, size, size);
}
