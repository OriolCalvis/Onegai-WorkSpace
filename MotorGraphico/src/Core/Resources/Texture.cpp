#include "Core/Resources/Texture.h"

#include <glad/glad.h>

Texture::~Texture() {
    if (m_glID != 0) {
        glDeleteTextures(1, &m_glID);
    }
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        if (m_glID != 0) {
            glDeleteTextures(1, &m_glID);
        }
        m_glID = other.m_glID;
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_glID = 0;
    }
    return *this;
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_glID);
}
