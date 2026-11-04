#include "Core/Resources/ShaderManager.h"

#include "Core/Errors/EngineException.h"

#include <fstream>
#include <sstream>

namespace {
std::string readFileOrThrow(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw ResourceLoadException(path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
}  // namespace

std::unique_ptr<Shader> ShaderManager::loadFromDisk(const std::string& path) {
    std::string vertexSource = readFileOrThrow(path + ".vert");
    std::string fragmentSource = readFileOrThrow(path + ".frag");
    // Shader lanza ShaderCompileException si falla el compile/link;
    // ResourceManager<Shader>::load() la capturara y la convertira en
    // Result<Shader*>::Error con el log incluido en el mensaje.
    return std::make_unique<Shader>(vertexSource, fragmentSource);
}
