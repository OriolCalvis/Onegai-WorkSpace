#pragma once

#include "Core/Resources/ResourceManager.h"
#include "Core/Resources/Texture.h"

// ResourceManager<Texture>: unica pieza especifica es como leer un PNG del
// disco y subirlo a la GPU con GL_NEAREST (requisito del pixel art nitido).
class TextureManager : public ResourceManager<Texture> {
protected:
    std::unique_ptr<Texture> loadFromDisk(const std::string& path) override;
};
