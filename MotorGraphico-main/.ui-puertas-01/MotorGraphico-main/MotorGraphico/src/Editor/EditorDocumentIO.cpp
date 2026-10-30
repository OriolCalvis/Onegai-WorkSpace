#include "Editor/EditorDocumentIO.h"

#include "Core/Json/JsonWriter.h"
#include "Project/ProjectSerializer.h"

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>

namespace fs = std::filesystem;
namespace {
struct Output {
    fs::path path, staging, backup;
    std::string text;
    bool backedUp = false;
    bool installed = false;
};

Result<bool> writeBundle(std::array<Output, 3> outputs) {
    static std::atomic<unsigned long> sequence{0};
    const auto nonce = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) +
                       "-" + std::to_string(sequence++);
    std::set<fs::path> paths;
    std::error_code ec;
    for (auto& output : outputs) {
        if (output.path.empty()) return Result<bool>::Error("Ruta de guardado vacía");
        auto absolute = fs::weakly_canonical(fs::absolute(output.path, ec), ec);
        if (ec || !paths.insert(absolute).second)
            return Result<bool>::Error("Rutas de guardado inválidas o coincidentes");
        if (fs::exists(output.path, ec) &&
            (fs::is_symlink(output.path, ec) || !fs::is_regular_file(output.path, ec)))
            return Result<bool>::Error("El destino no es un archivo regular: " + output.path.string());
        output.staging = output.path.string() + ".editor-new-" + nonce;
        output.backup = output.path.string() + ".editor-old-" + nonce;
    }
    const auto fail = [&](const std::string& reason) {
        std::string error = reason;
        for (auto it = outputs.rbegin(); it != outputs.rend(); ++it) {
            ec.clear();
            if (it->installed) fs::remove(it->path, ec);
            if (it->backedUp) {
                ec.clear();
                fs::rename(it->backup, it->path, ec);
                if (ec) error += "; copia de recuperación conservada en " + it->backup.string();
            }
            ec.clear();
            fs::remove(it->staging, ec);
        }
        return Result<bool>::Error(error);
    };
    for (auto& output : outputs) {
        std::ofstream file(output.staging, std::ios::binary | std::ios::trunc);
        file << output.text;
        file.close();
        if (!file) return fail("No se pudo preparar " + output.path.string());
    }
    for (auto& output : outputs) {
        ec.clear();
        if (fs::exists(output.path, ec)) {
            fs::rename(output.path, output.backup, ec);
            if (ec) return fail("No se pudo respaldar " + output.path.string());
            output.backedUp = true;
        }
        fs::rename(output.staging, output.path, ec);
        if (ec) return fail("No se pudo instalar " + output.path.string());
        output.installed = true;
    }
    for (auto& output : outputs) {
        if (output.backedUp) { ec.clear(); fs::remove(output.backup, ec); }
    }
    return Result<bool>::Ok(true);
}
}

std::string EditorDocumentIO::documentPathFor(const std::string& tmxPath) {
    auto path = fs::path(tmxPath);
    path.replace_extension(".map.json");
    return path.string();
}

Result<bool> EditorDocumentIO::save(const EditorState& editor, const std::string& levelName,
                                  const std::string& tmxPath, const std::string& levelPath,
                                  const TmxTilesetSettings& tileset,
                                  const std::vector<EnemySpawn>& legacyEnemies) {
    const auto nativePath = documentPathFor(tmxPath);
    auto document = editor.document();
    const auto collisions = document.setCollisionGids(tileset.collisionGids);
    if (!collisions.isOk()) return collisions;
    const auto native = ProjectSerializer::serializeMap(document);
    if (!native.isOk()) return Result<bool>::Error(native.errorMessage());
    std::string level = editor.exportLevelJson(levelName, tmxPath, nativePath);
    // Los niveles antiguos pueden guardar enemigos fuera de objects. No
    // convertirlos: perderíamos sus habilidades particulares por instancia.
    if (!legacyEnemies.empty()) {
        JsonWriter writer;
        writer.beginArray();
        for (const auto& enemy : legacyEnemies) {
            writer.beginObject(); writer.member("type", enemy.type);
            for (const auto& entry : {std::make_pair("position", enemy.position),
                                      std::make_pair("patrolMin", enemy.patrolMin),
                                      std::make_pair("patrolMax", enemy.patrolMax)}) {
                writer.key(entry.first); writer.beginObject();
                writer.member("x", entry.second.x); writer.member("y", entry.second.y);
                writer.endObject();
            }
            writer.key("skills"); writer.beginArray();
            for (const auto& skill : enemy.skillIds) writer.value(skill);
            writer.endArray(); writer.endObject();
        }
        writer.endArray();
        const auto end = level.find_last_of('}');
        if (end == std::string::npos) return Result<bool>::Error("Exportación de nivel inválida");
        level.insert(end, ",\n\"enemies\":" + writer.str() + "\n");
    }
    return writeBundle({{{nativePath, {}, {}, native.value()},
                         {tmxPath, {}, {}, editor.exportTmx(tileset)},
                         {levelPath, {}, {}, std::move(level)}}});
}
