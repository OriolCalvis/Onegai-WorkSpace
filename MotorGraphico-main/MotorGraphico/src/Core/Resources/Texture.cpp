#include "Core/Resources/Texture.h"

#include <glad/glad.h>

Texture::~Texture() {
    if (m_glID != 0) {
        glDeleteTextures(1, &m_glID);
    }
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_glID);
}
