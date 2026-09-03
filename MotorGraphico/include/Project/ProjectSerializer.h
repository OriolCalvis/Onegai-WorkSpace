#pragma once

#include <string>

#include "Core/Errors/Result.h"
#include "Map/MapDocument.h"
#include "Project/Project.h"

// Persistencia canónica v1 de los documentos de autoría. Es deliberadamente
// independiente de RuntimeTileMap: guarda lo que edita el autor y el builder
// lo compila después. Las salidas usan orden de claves fijo para que dos
// guardados del mismo documento produzcan los mismos bytes.
class ProjectSerializer {
public:
    static Result<std::string> serializeProject(const Editor::Project& project);
    static Result<Editor::Project> deserializeProject(const std::string& text);

    static Result<std::string> serializeMap(const MapDocument& map);
    static Result<MapDocument> deserializeMap(const std::string& text);

    static Result<bool> saveProjectAtomic(const std::string& path, const Editor::Project& project);
    static Result<bool> saveMapAtomic(const std::string& path, const MapDocument& map);
    static Result<Editor::Project> loadProjectFile(const std::string& path);
    static Result<MapDocument> loadMapFile(const std::string& path);
};
