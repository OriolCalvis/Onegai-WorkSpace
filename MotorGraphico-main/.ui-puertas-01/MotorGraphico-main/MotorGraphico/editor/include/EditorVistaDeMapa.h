// EST-26 (relevo A) — LA VISTA DEL MAPA DEL EDITOR.
//
// El unico sitio del editor que dibuja el mundo: camara, tileset, atlas,
// renderer isometrico y el framebuffer al que va todo antes de entrar en
// un panel de ImGui. Y la captura a PPM, que es lo mismo leido al reves.
//
// LO QUE MANDA AQUI ES LA REVISION DEL DOCUMENTO. sync() no reconstruye
// nada si la revision no ha cambiado, y solo rehace tileset y catalogo
// cuando cambia el NIVEL: reconstruir el atlas en cada fotograma se nota
// en la mano de quien pinta.

#pragma once

#include <algorithm>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Editor/EditorDocumentSession.h"
#include "Level/ObjectCatalog.h"
#include "Project/Project.h"
#include "Project/ProjectCatalog.h"
#include "Render/Camera.h"
#include "Render/Framebuffer.h"
#include "Render/IsometricRenderer.h"
#include "Runtime/RuntimeTileMapBuilder.h"

#include <glad/glad.h>

#include "imgui.h"

namespace Editor {

void capture(const std::string& path, int width, int height);

struct MapView {
    Camera camera{640, 480};
    RuntimeTileMap runtime;
    TextureManager textures;
    std::unique_ptr<TextureAtlas> atlas;
    std::unique_ptr<IsometricRenderer> renderer;
    std::unique_ptr<Framebuffer> target;
    ObjectCatalog catalog;
    std::vector<std::string> objectIds;
    GLuint textureId = 0;
    unsigned long revision = ~0UL;
    std::string path;
    // La peticion debe sobrevivir hasta render(), que conoce el visor real.
    bool framePending = false;

    void sync(const EditorDocumentSession& session, Shader* shader,
              const std::vector<Project>& projects) {
        if (revision == session.revision()) return;
        const auto built = RuntimeTileMapBuilder{}.build(session.state().document());
        if (!built.isOk()) throw std::runtime_error(built.errorMessage());
        runtime = built.value();
        if (path != session.levelPath()) {
            const auto& source = session.importedMap();
            auto texture = textures.load(source.tilesetImagePath(), source.tilesetImagePath());
            if (!texture.isOk()) throw std::runtime_error(texture.errorMessage());
            texture.value()->bind();
            GLint textureName = 0; glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureName);
            textureId = static_cast<GLuint>(textureName);
            atlas = std::make_unique<TextureAtlas>(texture.value(), source.getTilesetTileWidth(), source.getTilesetTileHeight());
            const int columns = source.getTilesetColumns();
            const int rows = source.getTilesetImageHeight() / source.getTilesetTileHeight();
            if (columns <= 0 || rows <= 0) throw std::runtime_error("Tileset sin dimensiones válidas");
            for (int i = 0; i < source.getTilesetTileCount(); ++i)
                atlas->defineRegion(source.getTilesetFirstGid() + i, i % columns, rows - 1 - i / columns);
            renderer = std::make_unique<IsometricRenderer>(&camera, &runtime, atlas.get(), shader);
            catalog = {}; objectIds.clear();
            for (const auto& project : projects) {
                const auto found = std::find_if(project.levels.begin(), project.levels.end(), [&](const auto& file) {
                    return std::filesystem::weakly_canonical(std::filesystem::path("assets") / project.settings.levelsDirectory / file) ==
                           std::filesystem::weakly_canonical(session.levelPath());
                });
                if (found != project.levels.end()) {
                    auto loaded = ProjectCatalog::load(project);
                    if (!loaded.isOk()) throw std::runtime_error(loaded.errorMessage());
                    catalog = std::move(loaded.value().objects);
                    objectIds = catalog.ids();
                    break;
                }
            }
            center();
            path = session.levelPath();
        }
        revision = session.revision();
    }
    void center();
    void render(int width, int height);
    ImVec2 screen(GridCoord cell, ImVec2 origin, ImVec2 scale) const {
        auto world = runtime.gridToScreen(cell);
        world.x += runtime.getTileWidth() * 0.5f;
        world.y += runtime.getTileHeight() * 0.5f;
        return {origin.x + ((world.x - camera.position().x) * camera.zoom() + camera.viewportWidth() * 0.5f) / scale.x,
                origin.y + ((world.y - camera.position().y) * camera.zoom() + camera.viewportHeight() * 0.5f) / scale.y};
    }
};

}  // namespace Editor
