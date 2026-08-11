#pragma once

#include "Core/Resources/ResourceManager.h"
#include "Core/Resources/Shader.h"

// El "path" que recibe load() es un prefijo sin extension: se resuelve a
// "<path>.vert" y "<path>.frag" (convencion simple, ver .cpp).
class ShaderManager : public ResourceManager<Shader> {
protected:
    std::unique_ptr<Shader> loadFromDisk(const std::string& path) override;
};
