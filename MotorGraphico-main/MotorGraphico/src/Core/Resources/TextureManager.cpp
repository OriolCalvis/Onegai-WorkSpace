#include "Core/Resources/TextureManager.h"

#include <glad/glad.h>
#include <stb_image.h>

std::unique_ptr<Texture> TextureManager::loadFromDisk(const std::string& path) {
    int width = 0, height = 0, channels = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        // No lanzamos aqui: devolver nullptr basta, ResourceManager<T>::load()
        // lo traduce en un Result<T*>::Error legible.
        return nullptr;
    }

    unsigned int glID = 0;
    glGenTextures(1, &glID);
    glBindTexture(GL_TEXTURE_2D, glID);

    // GL_NEAREST en min/mag: requisito de la guia de estilo pixel art,
    // sin mipmaps (ver GDD seccion 4).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    return std::make_unique<Texture>(glID, width, height);
}
