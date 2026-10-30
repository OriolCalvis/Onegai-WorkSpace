// H4-02 — Las piezas de ProjectMaps que no son su API: ficheros, manifiesto,
// niveles y rutas. Ver src/Editor/ProjectMapsDetail.h.
#include "ProjectMapsDetail.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"
#include "Project/ProjectSerializer.h"

namespace Editor::project_maps {

std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

bool readText(const std::string& path, std::string& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;
    std::ostringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

// Escribe al lado y lo instala con rename: quien lea el fichero ve el viejo
// entero o el nuevo entero, nunca medio. Con mustNotExist no pisa nada.
Result<bool> writeFile(const fs::path& path, const std::string& text, bool mustNotExist) {
    static std::atomic<unsigned long> sequence{0};
    std::error_code ec;
    if (mustNotExist && fs::exists(path, ec))
        return Result<bool>::Error("ya existe " + path.generic_string() + ": no se sobreescribe");
    const fs::path staging =
        path.string() + ".editor-new-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" +
        std::to_string(sequence++);
    {
        std::ofstream file(staging, std::ios::binary | std::ios::trunc);
        file << text;
        file.close();
        if (!file) {
            fs::remove(staging, ec);
            return Result<bool>::Error("no se pudo escribir " + path.generic_string());
        }
    }
    fs::rename(staging, path, ec);
    if (ec) {
        fs::remove(staging, ec);
        return Result<bool>::Error("no se pudo instalar " + path.generic_string());
    }
    return Result<bool>::Ok(true);
}

void removeAll(const std::vector<fs::path>& paths) {
    std::error_code ec;
    for (const auto& path : paths)
        fs::remove(path, ec);
}

std::string manifestPath(const std::string& root, const std::string& projectId) {
    return root + "/proyectos/" + projectId + ".json";
}

Result<Project> loadProject(const std::string& root, const std::string& projectId) {
    if (projectId.empty() ||
        projectId.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_") != std::string::npos)
        return Result<Project>::Error("id de proyecto invalido: '" + projectId + "'");
    auto project = ProjectSerializer::loadProjectFile(manifestPath(root, projectId));
    if (!project.isOk())
        return Result<Project>::Error("no se pudo leer el proyecto '" + projectId +
                                      "': " + project.errorMessage());
    return project;
}

Result<bool> saveProject(const std::string& root, const Project& project) {
    const auto valid = project.validate();
    if (!valid.isOk())
        return Result<bool>::Error("el proyecto quedaria invalido: " + valid.errorMessage());
    return ProjectSerializer::saveProjectAtomic(manifestPath(root, project.id), project);
}

bool contains(const std::vector<std::string>& names, const std::string& name) {
    return std::find(names.begin(), names.end(), name) != names.end();
}

fs::path levelsDir(const std::string& root, const Project& project) {
    return fs::path(root) / project.settings.levelsDirectory;
}
fs::path mapsDir(const std::string& root, const Project& project) {
    return fs::path(root) / project.settings.mapsDirectory;
}

LevelInfo readLevel(const std::string& path) {
    LevelInfo info;
    std::string text;
    if (!readText(path, text))
        return info;
    const auto parsed = JsonValue::parse(text);
    if (!parsed.isOk() || !parsed.value().isObject())
        return info;
    const JsonValue& root = parsed.value();
    info.ok = true;
    info.name = root["name"].asString("");
    info.map = root["map"].asString("");
    info.document = root["mapDocument"].asString("");
    return info;
}

// entry admite "assets/levels/x.json", "levels/x.json" y "x.json"
// (ver resuelveEntrada en ProjectIndex.cpp).
bool isEntry(const Project& project, const std::string& level) {
    std::string entry = project.entry;
    if (entry.rfind("assets/", 0) == 0)
        entry.erase(0, 7);
    if (entry.find('/') == std::string::npos)
        return entry == level;
    return entry == project.settings.levelsDirectory + "/" + level;
}

std::string fileId(const std::string& fileName) {
    for (const char* suffix : {".map.json", ".json", ".tmx"}) {
        const std::string s = suffix;
        if (fileName.size() > s.size() &&
            fileName.compare(fileName.size() - s.size(), s.size(), s) == 0)
            return fileName.substr(0, fileName.size() - s.size());
    }
    return fs::path(fileName).stem().string();
}

// Lo que ya esta cogido en las dos carpetas, haya o no manifiesto que lo
// reclame: el nombre de un fichero suelto tambien esta ocupado.
std::vector<std::string> takenIds(const std::string& root, const Project& project) {
    std::vector<std::string> taken;
    for (const fs::path& dir : {levelsDir(root, project), mapsDir(root, project)}) {
        std::error_code ec;
        if (!fs::is_directory(dir, ec))
            continue;
        for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec))
            taken.push_back(fileId(it->path().filename().string()));
    }
    for (const auto* group : {&project.levels, &project.maps})
        for (const auto& name : *group)
            taken.push_back(fileId(name));
    return taken;
}

int floorGid(const TmxTilesetSettings& tileset) {
    for (int gid = 1; gid <= tileset.tileCount; ++gid)
        if (!std::count(tileset.collisionGids.begin(), tileset.collisionGids.end(), gid))
            return gid;
    return 1;
}

// La imagen de un TMX tal como la guarda (relativa a el) llevada a una ruta
// que se puede abrir desde el cwd.
std::string imageOf(const std::string& tmxPath, const std::string& source) {
    if (source.empty() || fs::path(source).is_absolute())
        return source;
    return (fs::path(tmxPath).parent_path() / source).lexically_normal().generic_string();
}

bool sameFolder(const fs::path& a, const fs::path& b) {
    std::error_code ec;
    return fs::weakly_canonical(fs::absolute(a, ec), ec) ==
           fs::weakly_canonical(fs::absolute(b, ec), ec);
}

std::string relativeTo(const std::string& target, const fs::path& from) {
    std::error_code ec;
    const fs::path a = fs::weakly_canonical(fs::absolute(target, ec), ec);
    const fs::path b = fs::weakly_canonical(fs::absolute(from, ec), ec);
    const fs::path relative = a.lexically_relative(b);
    return relative.empty() ? target : relative.generic_string();
}

Result<bool> addToManifest(const std::string& root, const std::string& projectId,
                           const std::string& level, const std::string& map) {
    auto project = loadProject(root, projectId);
    if (!project.isOk())
        return Result<bool>::Error(project.errorMessage());
    Project p = std::move(project.value());
    if (!contains(p.levels, level))
        p.levels.push_back(level);
    if (!contains(p.maps, map))
        p.maps.push_back(map);
    return saveProject(root, p);
}

}  // namespace Editor::project_maps

namespace Editor {

std::string canonicalPath(const std::string& path) {
    std::error_code ec;
    const auto canonical =
        std::filesystem::weakly_canonical(std::filesystem::absolute(path, ec), ec);
    return ec ? path : canonical.generic_string();
}

}  // namespace Editor
