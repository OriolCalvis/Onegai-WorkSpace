#include "TextAssetManager.h"

#include "Core/Errors/EngineException.h"

#include <fstream>
#include <sstream>

std::unique_ptr<TextAsset> TextAssetManager::loadFromDisk(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        // Excepcion de dominio: ResourceManager<T>::load() la capturara
        // y la convertira en Result<T*>::Error.
        throw ResourceLoadException(path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return std::make_unique<TextAsset>(ss.str());
}
