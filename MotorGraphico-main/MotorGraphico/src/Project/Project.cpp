#include "Project/Project.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <string>
#include <vector>

namespace Editor {
namespace {

bool isProjectId(const std::string& id) {
    if (id.empty()) {
        return false;
    }
    for (char character : id) {
        if (!((character >= 'a' && character <= 'z') ||
              (character >= '0' && character <= '9') || character == '_')) {
            return false;
        }
    }
    return true;
}

bool isRelativePath(const std::string& path) {
    if (path.empty() || path.front() == '/' || path.front() == '\\' ||
        (path.size() > 1 && std::isalpha(static_cast<unsigned char>(path[0])) &&
         path[1] == ':') ||
        path.find('\\') != std::string::npos) {
        return false;
    }

    std::size_t begin = 0;
    while (begin <= path.size()) {
        const std::size_t end = path.find('/', begin);
        const std::string segment = path.substr(begin, end - begin);
        if (segment.empty() || segment == "." || segment == "..") {
            return false;
        }
        if (end == std::string::npos) {
            break;
        }
        begin = end + 1;
    }
    return true;
}

bool isDocumentName(const std::string& name) {
    return !name.empty() && name.find('/') == std::string::npos &&
           name.find('\\') == std::string::npos && name != "." && name != "..";
}

Result<bool> validateNames(const std::vector<std::string>& names, const char* kind) {
    std::set<std::string> unique;
    for (const std::string& name : names) {
        if (!isDocumentName(name)) {
            return Result<bool>::Error(std::string(kind) + " contiene una referencia no válida: '" +
                                       name + "'");
        }
        if (!unique.insert(name).second) {
            return Result<bool>::Error(std::string(kind) + " contiene una referencia duplicada: '" +
                                       name + "'");
        }
    }
    return Result<bool>::Ok(true);
}

}  // namespace

Result<bool> ProjectSettings::validate() const {
    if (schemaVersion != kCurrentSchemaVersion) {
        return Result<bool>::Error("schemaVersion no soportada: " +
                                   std::to_string(schemaVersion));
    }

    const std::string* directories[] = {&mapsDirectory, &levelsDirectory,
                                        &adventuresDirectory, &catalogsDirectory};
    std::set<std::string> unique;
    for (const std::string* directory : directories) {
        if (!isRelativePath(*directory)) {
            return Result<bool>::Error("directorio lógico no válido: '" + *directory + "'");
        }
        if (!unique.insert(*directory).second) {
            return Result<bool>::Error("los directorios lógicos deben ser distintos: '" +
                                       *directory + "'");
        }
    }
    if (!initialMap.empty() && !isDocumentName(initialMap)) {
        return Result<bool>::Error("initialMap no es un nombre de documento válido: '" + initialMap +
                                   "'");
    }
    return Result<bool>::Ok(true);
}

Result<bool> Project::validate() const {
    if (!isProjectId(id)) {
        return Result<bool>::Error("id de proyecto inválido: solo [a-z0-9_]");
    }
    if (name.empty()) {
        return Result<bool>::Error("el nombre del proyecto no puede estar vacío");
    }
    if (!prefix.empty() && prefix.back() != '_') {
        return Result<bool>::Error("el prefijo de proyecto debe acabar en '_'");
    }
    const Result<bool> settingsResult = settings.validate();
    if (!settingsResult.isOk()) {
        return settingsResult;
    }
    if (!entry.empty() && !isRelativePath(entry)) {
        return Result<bool>::Error("entrada no es una ruta lógica relativa: '" + entry + "'");
    }

    const std::pair<const std::vector<std::string>*, const char*> groups[] = {
        {&levels, "levels"}, {&maps, "maps"}, {&adventures, "adventures"}, {&catalogs, "catalogs"}};
    for (const auto& group : groups) {
        const Result<bool> groupResult = validateNames(*group.first, group.second);
        if (!groupResult.isOk()) {
            return groupResult;
        }
    }
    if (!settings.initialMap.empty() &&
        std::find(maps.begin(), maps.end(), settings.initialMap) == maps.end()) {
        return Result<bool>::Error("initialMap no pertenece a maps: '" + settings.initialMap + "'");
    }
    return Result<bool>::Ok(true);
}

}  // namespace Editor
