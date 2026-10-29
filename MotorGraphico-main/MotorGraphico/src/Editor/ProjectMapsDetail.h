// H4-02 — Lo que ProjectMaps.cpp necesita y no es su API: leer y escribir
// ficheros sin dejarlos a medias, el manifiesto, lo que dice un nivel, los ids
// ocupados y las rutas de imagen. Partido de ProjectMaps.cpp por la ley de los
// 500 lineas; no lo incluye nadie mas (vive en src/, no en include/).
#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Editor/EditorState.h"
#include "Project/Project.h"

namespace Editor::project_maps {

namespace fs = std::filesystem;

std::string trim(const std::string& text);
bool readText(const std::string& path, std::string& out);
// Escribe al lado y lo instala con rename; con mustNotExist no pisa nada.
Result<bool> writeFile(const fs::path& path, const std::string& text, bool mustNotExist);
void removeAll(const std::vector<fs::path>& paths);

std::string manifestPath(const std::string& root, const std::string& projectId);
Result<Project> loadProject(const std::string& root, const std::string& projectId);
Result<bool> saveProject(const std::string& root, const Project& project);
Result<bool> addToManifest(const std::string& root, const std::string& projectId,
                           const std::string& level, const std::string& map);
bool contains(const std::vector<std::string>& names, const std::string& name);

fs::path levelsDir(const std::string& root, const Project& project);
fs::path mapsDir(const std::string& root, const Project& project);

// Lo que dice un nivel de si mismo (nombre visible, TMX y documento).
struct LevelInfo {
    bool ok = false;
    std::string name, map, document;
};
LevelInfo readLevel(const std::string& path);
bool isEntry(const Project& project, const std::string& level);

std::string fileId(const std::string& fileName);
std::vector<std::string> takenIds(const std::string& root, const Project& project);
int floorGid(const TmxTilesetSettings& tileset);

std::string imageOf(const std::string& tmxPath, const std::string& source);
bool sameFolder(const fs::path& a, const fs::path& b);
std::string relativeTo(const std::string& target, const fs::path& from);

}  // namespace Editor::project_maps
