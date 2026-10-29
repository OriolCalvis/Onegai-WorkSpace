// H4-02 / E-11 — ver include/Editor/ProjectMaps.h.
#include "Editor/ProjectMaps.h"

#include "ProjectMapsDetail.h"

#include <algorithm>
#include <filesystem>
#include <set>
#include <sstream>

#include "Core/Json/JsonValue.h"
#include "Editor/EditorDocumentIO.h"
#include "Level/LevelDocumentBridge.h"
#include "Project/ProjectIndex.h"
#include "Project/ProjectSerializer.h"
#include "Render/TileMap.h"

namespace Editor {

using namespace project_maps;

std::vector<ProjectMap> ProjectMaps::list(const std::string& assetsRoot, const Project& project) {
    std::vector<ProjectMap> maps;
    for (const std::string& level : project.levels) {
        ProjectMap map;
        map.level = level;
        map.path = (levelsDir(assetsRoot, project) / level).generic_string();
        const LevelInfo info = readLevel(map.path);
        map.readable = info.ok;
        map.name = info.ok && !trim(info.name).empty() ? info.name : fileId(level);
        map.map = info.map;
        map.entry = isEntry(project, level);
        map.canonical = canonicalPath(map.path);
        maps.push_back(std::move(map));
    }
    return maps;
}

std::vector<ProjectTileset> ProjectMaps::tilesets(const std::string& assetsRoot,
                                                  const std::vector<Project>& projects,
                                                  const std::string& projectId) {
    std::vector<ProjectTileset> found;
    std::set<std::string> seen;
    auto collect = [&](const Project& project) {
        for (const std::string& name : project.maps) {
            if (fs::path(name).extension() != ".tmx")
                continue;
            const std::string path = (mapsDir(assetsRoot, project) / name).generic_string();
            TileMap map;
            if (!map.loadFromFile(path).isOk() || map.getTilesetFirstGid() != 1 ||
                map.getTilesetTileCount() < 1)
                continue;
            ProjectTileset tileset;
            auto& s = tileset.settings;
            s.tileWidth = map.getTileWidth();
            s.tileHeight = map.getTileHeight();
            s.tilesetName = map.getTilesetName();
            s.imageSource = map.getTilesetImageSource();
            s.imageWidth = map.getTilesetImageWidth();
            s.imageHeight = map.getTilesetImageHeight();
            s.tilesetTileWidth = map.getTilesetTileWidth();
            s.tilesetTileHeight = map.getTilesetTileHeight();
            s.tileCount = map.getTilesetTileCount();
            s.columns = map.getTilesetColumns();
            s.collisionGids = map.getCollisionGids();
            tileset.image = imageOf(path, s.imageSource);
            tileset.source = path;
            tileset.name =
                !s.tilesetName.empty() ? s.tilesetName : fs::path(tileset.image).stem().string();
            std::ostringstream key;
            key << relativeTo(tileset.image, ".") << '|' << s.tileWidth << 'x' << s.tileHeight
                << '|' << s.tilesetTileWidth << 'x' << s.tilesetTileHeight << '|' << s.tileCount
                << '|';
            for (int gid : s.collisionGids)
                key << gid << ',';
            if (seen.insert(key.str()).second)
                found.push_back(std::move(tileset));
        }
    };
    for (const Project& project : projects)
        if (project.id == projectId)
            collect(project);
    if (found.empty())
        for (const Project& project : projects)
            collect(project);
    return found;
}

Result<std::string> ProjectMaps::suggestFileId(const std::string& assetsRoot,
                                               const Project& project,
                                               const std::string& visibleName) {
    const auto base = ProjectIndex::suggestId(trim(visibleName));
    if (!base.isOk())
        return Result<std::string>::Error(
            "ese nombre no sirve para un mapa: escribe al menos una letra o un numero");
    std::string id = base.value();
    if (!project.prefix.empty() && id.rfind(project.prefix, 0) != 0)
        id = project.prefix + id;
    return Result<std::string>::Ok(ProjectIndex::uniqueId(id, takenIds(assetsRoot, project)));
}

Result<ProjectMap> ProjectMaps::create(const std::string& assetsRoot, const std::string& projectId,
                                       const std::string& visibleName, int width, int height,
                                       const ProjectTileset& tileset) {
    using R = Result<ProjectMap>;
    const std::string name = trim(visibleName);
    if (name.empty())
        return R::Error("escribe un nombre para el mapa");
    if (width < kMinSide || width > kMaxSide || height < kMinSide || height > kMaxSide)
        return R::Error("el mapa tiene que medir entre " + std::to_string(kMinSide) + " y " +
                        std::to_string(kMaxSide) + " celdas de lado");
    TmxTilesetSettings settings = tileset.settings;
    if (settings.tileCount < 1 || settings.tileWidth <= 0 || settings.tileHeight <= 0)
        return R::Error("elige un tileset");
    auto loaded = loadProject(assetsRoot, projectId);
    if (!loaded.isOk())
        return R::Error(loaded.errorMessage());
    const Project& project = loaded.value();
    const auto id = suggestFileId(assetsRoot, project, name);
    if (!id.isOk())
        return R::Error(id.errorMessage());

    std::error_code ec;
    fs::create_directories(levelsDir(assetsRoot, project), ec);
    fs::create_directories(mapsDir(assetsRoot, project), ec);
    const std::string levelPath =
        (levelsDir(assetsRoot, project) / (id.value() + ".json")).generic_string();
    const std::string tmxPath =
        (mapsDir(assetsRoot, project) / (id.value() + ".tmx")).generic_string();
    const std::string documentPath = EditorDocumentIO::documentPathFor(tmxPath);
    for (const auto& path : {levelPath, tmxPath, documentPath})
        if (fs::exists(path, ec))
            return R::Error("ya existe " + path + ": no se sobreescribe");
    if (!tileset.image.empty())
        settings.imageSource = relativeTo(tileset.image, mapsDir(assetsRoot, project));

    MapDocument document(ids::MapId::parse(id.value()).value(), name, width, height,
                         settings.tileWidth, settings.tileHeight);
    MapLayerMeta floor;
    floor.id = ids::LayerId::parse("suelo").value();
    floor.name = "suelo";
    const auto gids =
        std::vector<int>(static_cast<std::size_t>(width) * height, floorGid(settings));
    auto added = document.addLayer(MapLayer::tiles(floor, TileLayerDocument(width, height, gids)));
    auto objects = level_document::ToEntityLayer({});
    if (added.isOk() && objects.isOk())
        added = document.addLayer(MapLayer::entities(
            {ids::LayerId::parse("level_objects").value(), "Objetos", true, false, 1.0f, 1},
            std::move(objects.value())));
    if (!added.isOk())
        return R::Error("no se pudo preparar el mapa: " + added.errorMessage());
    EditorState state(1, 1);
    const auto set = state.setDocument(std::move(document));
    if (!set.isOk())
        return R::Error("no se pudo preparar el mapa: " + set.errorMessage());
    state.setPlayerStart({width / 2, height / 2});

    const auto saved = EditorDocumentIO::save(state, name, tmxPath, levelPath, settings, {});
    if (!saved.isOk())
        return R::Error("no se pudo crear el mapa: " + saved.errorMessage());
    const auto registered =
        addToManifest(assetsRoot, projectId, id.value() + ".json", id.value() + ".tmx");
    if (!registered.isOk()) {
        removeAll({levelPath, tmxPath, documentPath});
        return R::Error("no se pudo anadir al proyecto (no queda nada a medias): " +
                        registered.errorMessage());
    }
    return R::Ok(ProjectMap{id.value() + ".json", levelPath, name, tmxPath, false, true});
}

Result<ProjectMap> ProjectMaps::duplicate(const std::string& assetsRoot,
                                          const std::string& projectId, const std::string& level,
                                          const std::string& newVisibleName) {
    using R = Result<ProjectMap>;
    const std::string name = trim(newVisibleName);
    if (name.empty())
        return R::Error("escribe un nombre para la copia");
    auto loaded = loadProject(assetsRoot, projectId);
    if (!loaded.isOk())
        return R::Error(loaded.errorMessage());
    const Project& project = loaded.value();
    if (!contains(project.levels, level))
        return R::Error("el mapa no es de este proyecto: " + level);
    const std::string sourceLevel = (levelsDir(assetsRoot, project) / level).generic_string();
    std::string levelText, tmxText, documentText;
    const LevelInfo info = readLevel(sourceLevel);
    if (!info.ok || info.map.empty() || !readText(sourceLevel, levelText))
        return R::Error("no se pudo leer el nivel " + sourceLevel);
    if (!readText(info.map, tmxText))
        return R::Error("no se pudo leer el mapa " + info.map);
    if (!info.document.empty() && !readText(info.document, documentText))
        return R::Error("no se pudo leer el documento " + info.document);

    const auto id = suggestFileId(assetsRoot, project, name);
    if (!id.isOk())
        return R::Error(id.errorMessage());
    std::error_code ec;
    fs::create_directories(mapsDir(assetsRoot, project), ec);
    const std::string levelPath =
        (levelsDir(assetsRoot, project) / (id.value() + ".json")).generic_string();
    const std::string tmxPath =
        (mapsDir(assetsRoot, project) / (id.value() + ".tmx")).generic_string();
    const std::string documentPath = EditorDocumentIO::documentPathFor(tmxPath);

    // El TMX guarda su imagen relativa a si mismo: si la copia cambia de
    // carpeta, la ruta se rehace para que siga apuntando a la misma imagen.
    TileMap map;
    if (!sameFolder(fs::path(info.map).parent_path(), mapsDir(assetsRoot, project)) &&
        map.loadFromFile(info.map).isOk() && !map.getTilesetImageSource().empty()) {
        const std::string raw = map.getTilesetImageSource();
        const std::string before = "source=\"" + raw + "\"";
        const std::string after =
            "source=\"" + relativeTo(imageOf(info.map, raw), mapsDir(assetsRoot, project)) + "\"";
        const auto at = tmxText.find(before);
        if (at != std::string::npos)
            tmxText.replace(at, before.size(), after);
    }
    auto text = replaceTopLevelString(levelText, "name", name, true);
    if (text.isOk())
        text = replaceTopLevelString(text.value(), "map", tmxPath, true);
    if (text.isOk() && !documentText.empty())
        text = replaceTopLevelString(text.value(), "mapDocument", documentPath, true);
    auto document = documentText.empty()
                        ? Result<std::string>::Ok("")
                        : replaceTopLevelString(documentText, "id", id.value(), false);
    if (document.isOk() && !documentText.empty())
        document = replaceTopLevelString(document.value(), "name", name, true);
    if (!text.isOk() || !document.isOk())
        return R::Error("no se pudo preparar la copia: " +
                        (text.isOk() ? document.errorMessage() : text.errorMessage()));
    if (!documentText.empty() && !ProjectSerializer::deserializeMap(document.value()).isOk())
        return R::Error("la copia del documento no es valida; no se ha escrito nada");

    std::vector<fs::path> written;
    std::vector<std::pair<std::string, std::string>> files = {{tmxPath, tmxText}};
    if (!documentText.empty())
        files.emplace_back(documentPath, document.value());
    files.emplace_back(levelPath, text.value());
    for (const auto& file : files) {
        const auto result = writeFile(file.first, file.second, true);
        if (!result.isOk()) {
            removeAll(written);
            return R::Error(result.errorMessage());
        }
        written.push_back(file.first);
    }
    const auto registered =
        addToManifest(assetsRoot, projectId, id.value() + ".json", id.value() + ".tmx");
    if (!registered.isOk()) {
        removeAll(written);
        return R::Error("no se pudo anadir al proyecto (no queda nada a medias): " +
                        registered.errorMessage());
    }
    return R::Ok(ProjectMap{id.value() + ".json", levelPath, name, tmxPath, false, true});
}

Result<bool> ProjectMaps::rename(const std::string& assetsRoot, const std::string& projectId,
                                 const std::string& level, const std::string& newVisibleName) {
    const std::string name = trim(newVisibleName);
    if (name.empty())
        return Result<bool>::Error("escribe el nombre nuevo del mapa");
    auto loaded = loadProject(assetsRoot, projectId);
    if (!loaded.isOk())
        return Result<bool>::Error(loaded.errorMessage());
    if (!contains(loaded.value().levels, level))
        return Result<bool>::Error("el mapa no es de este proyecto: " + level);
    const fs::path path = levelsDir(assetsRoot, loaded.value()) / level;
    std::string text;
    if (!readText(path.string(), text))
        return Result<bool>::Error("no se pudo leer " + path.generic_string());
    const auto renamed = replaceTopLevelString(text, "name", name, true);
    if (!renamed.isOk())
        return Result<bool>::Error(renamed.errorMessage());
    return writeFile(path, renamed.value(), false);
}

Result<bool> ProjectMaps::remove(const std::string& assetsRoot, const std::string& projectId,
                                 const std::string& level) {
    auto loaded = loadProject(assetsRoot, projectId);
    if (!loaded.isOk())
        return Result<bool>::Error(loaded.errorMessage());
    Project project = std::move(loaded.value());
    const auto it = std::find(project.levels.begin(), project.levels.end(), level);
    if (it == project.levels.end())
        return Result<bool>::Error("el mapa no es de este proyecto: " + level);
    if (isEntry(project, level))
        return Result<bool>::Error(
            "es el mapa donde empieza el juego: elige otro mapa de inicio antes de quitarlo");
    const LevelInfo info = readLevel((levelsDir(assetsRoot, project) / level).generic_string());
    project.levels.erase(it);
    // Su TMX sale tambien, salvo que otro nivel del proyecto lo use.
    const std::string tmx = info.ok ? fs::path(info.map).filename().string() : std::string();
    bool shared = tmx.empty();
    for (const std::string& other : project.levels) {
        const LevelInfo otherInfo =
            readLevel((levelsDir(assetsRoot, project) / other).generic_string());
        shared = shared || (otherInfo.ok && fs::path(otherInfo.map).filename().string() == tmx);
    }
    if (!shared) {
        project.maps.erase(std::remove(project.maps.begin(), project.maps.end(), tmx),
                           project.maps.end());
        if (project.settings.initialMap == tmx)
            project.settings.initialMap.clear();
    }
    return saveProject(assetsRoot, project);
}

Result<bool> ProjectMaps::makeEntry(const std::string& assetsRoot, const std::string& projectId,
                                    const std::string& level) {
    auto loaded = loadProject(assetsRoot, projectId);
    if (!loaded.isOk())
        return Result<bool>::Error(loaded.errorMessage());
    Project project = std::move(loaded.value());
    if (!contains(project.levels, level))
        return Result<bool>::Error("el mapa no es de este proyecto: " + level);
    project.entry = "assets/" + project.settings.levelsDirectory + "/" + level;
    return saveProject(assetsRoot, project);
}

Result<bool> ProjectMaps::setTitle(const std::string& assetsRoot, const std::string& projectId,
                                   const std::string& title) {
    auto loaded = loadProject(assetsRoot, projectId);
    if (!loaded.isOk())
        return Result<bool>::Error(loaded.errorMessage());
    Project project = std::move(loaded.value());
    project.settings.title = trim(title);
    return saveProject(assetsRoot, project);
}

}  // namespace Editor
