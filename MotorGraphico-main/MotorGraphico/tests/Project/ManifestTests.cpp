#include "Project/ProjectIndex.h"
#include "Project/ProjectSerializer.h"
#include "Core/Json/JsonValue.h"

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
void require(bool ok, const std::string& message) {
    if (!ok) throw std::runtime_error(message);
}
std::string read(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    require(file.good(), "cannot open " + path);
    return {(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
}

void index_lifecycle() {
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / ("motor-project-index-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    struct Cleanup {
        fs::path path;
        ~Cleanup() { std::error_code ec; fs::remove_all(path, ec); }
    } cleanup{root};
    const auto folder = root / "proyectos";
    fs::create_directories(folder);
    { std::ofstream f(folder / "index.json"); f << R"({"proyectos":["broken","missing"]})"; }
    { std::ofstream f(folder / "broken.json"); f << R"({"id":"broken","nombre":"Viejo"})"; }
    const std::string name = "Proyecto \"nuevo\"\nNiña\t";
    const auto created = Editor::ProjectIndex::create(root.string(), "new_project", name, "np_", "test", "2000");
    require(created.isOk(), created.errorMessage());
    const auto serialized = ProjectSerializer::loadProjectFile((folder / "new_project.json").string());
    require(serialized.isOk() && serialized.value().name == name, "create bypassed canonical serializer");
    fs::create_directories(root / "levels");
    { std::ofstream f(root / "levels/start.json"); f << "{}"; }
    { std::ofstream f(root / "root-level.json"); f << "{}"; }
    for (const auto& entry : {"start.json", "levels/start.json", "assets/levels/start.json",
                              "assets/root-level.json"}) {
        auto project = serialized.value();
        project.entry = entry;
        require(ProjectSerializer::saveProjectAtomic((folder / "new_project.json").string(), project).isOk(),
                "cannot save entry fixture");
        const auto entryScan = Editor::ProjectIndex::scan(root.string());
        require(entryScan.isOk() && entryScan.value().find("new_project") != nullptr &&
                    entryScan.value().find("new_project")->entryKind == Editor::Project::EntryKind::Level,
                std::string("entry did not resolve against supplied assets root: ") + entry);
    }
    const auto scan = Editor::ProjectIndex::scan(root.string());
    require(scan.isOk() && scan.value().size() == 1 && scan.value().orphans().size() == 2,
            "create discarded unreadable/missing manifests from the index");
    require(Editor::ProjectIndex::remove(root.string(), "new_project").isOk(), "cannot remove created project");
    const auto after = Editor::ProjectIndex::scan(root.string());
    require(after.isOk() && after.value().orphans().size() == 2, "remove discarded unrelated manifest ids");
    require(Editor::ProjectIndex::remove(root.string(), "broken").isOk(), "cannot remove invalid manifest");
    require(Editor::ProjectIndex::remove(root.string(), "missing").isOk(), "cannot remove missing manifest reference");
    const auto empty = Editor::ProjectIndex::scan(root.string());
    require(empty.isOk() && empty.value().size() == 0, "empty index cannot bootstrap a project");
    require(Editor::ProjectIndex::create(root.string(), "first", "First", "", "", "").isOk(),
            "cannot create first project from empty index");
    require(!Editor::ProjectIndex::remove(root.string(), "../outside").isOk(), "unsafe project id accepted");
    std::cout << "[PASS] canonical create/remove, invalid manifests preserved, empty index\n";
}
}

int main(int argc, char** argv) {
    try {
        const std::string root = argc > 1 ? argv[1] : "assets";
        const auto index = JsonValue::parse(read(root + "/proyectos/index.json"));
        require(index.isOk(), index.errorMessage());
        const auto scan = Editor::ProjectIndex::scan(root);
        require(scan.isOk(), scan.errorMessage());
        const auto& ids = index.value()["proyectos"];
        require(ids.isArray() && ids.size() > 0, "no real manifests tested");
        require(scan.value().size() == ids.size(), "ProjectIndex skipped manifests");

        // H0-07 — Un proyecto publicado en el indice es un proyecto que el
        // titulo ofrece al jugador. `TitleScreen::fromProjects` descarta los
        // que no cumplen `playable() && entryKind != None && != Missing` y
        // solo los anota en `skipped()`: nadie los reclama. Hasta aqui la
        // comprobacion de abajo vivia dentro de un `if (!entry.empty())`, o
        // sea que una entrada VACIA pasaba en silencio y el proyecto dejaba
        // de ser jugable sin que ninguna prueba lo dijera. `mundo` y
        // `este_sur` estuvieron asi desde el 16/08 (mapamundi al 42%).
        //
        // `ProjectIndex::check()` documenta que un pack PUEDE ser solo
        // escenarios y que eso no es un error de datos. No se contradice
        // aqui: se exige que sea una decision explicita. Un pack publicado
        // que no se puede jugar se lista abajo con su motivo; si no esta,
        // se publica para jugarse.
        const std::vector<std::string> kSoloEscenarios = {};

        for (std::size_t i = 0; i < ids.size(); ++i) {
            const auto id = ids[i].asString();
            const auto bytes = read(root + "/proyectos/" + id + ".json");
            const auto loaded = ProjectSerializer::deserializeProject(bytes);
            require(loaded.isOk(), id + ": " + loaded.errorMessage());
            const auto written = ProjectSerializer::serializeProject(loaded.value());
            require(written.isOk(), written.errorMessage());
            require(written.value() == bytes, id + ": canonical byte round-trip changed manifest");
            auto unknown = bytes;
            unknown.insert(1, "\"unexpected\":1,");
            require(!ProjectSerializer::deserializeProject(unknown).isOk(),
                    "unknown authoring field would be silently discarded");
            const auto* indexed = scan.value().find(id);
            require(indexed != nullptr, id + ": absent from index");
            const auto indexedBytes = ProjectSerializer::serializeProject(*indexed);
            require(indexedBytes.isOk() && indexedBytes.value() == bytes,
                    id + ": ProjectIndex and ProjectSerializer disagree");
            const bool soloEscenarios =
                std::find(kSoloEscenarios.begin(), kSoloEscenarios.end(), id) !=
                kSoloEscenarios.end();
            const bool jugable = indexed->playable() &&
                                 indexed->entryKind != Editor::Project::EntryKind::None &&
                                 indexed->entryKind != Editor::Project::EntryKind::Missing;
            if (!soloEscenarios) {
                require(!indexed->entry.empty(),
                        id + ": published in the index with an empty 'entry', so the "
                              "title silently drops it (TitleScreen::skipped). Give it "
                              "an entry or list it in kSoloEscenarios with a reason");
                require(indexed->entryKind != Editor::Project::EntryKind::Missing,
                        id + ": entry lost during migration");
                require(jugable, id + ": entry resolves to neither an adventure nor a level");
            }
            std::cout << "[PASS] " << id << " canonical round-trip + index + entry"
                      << (jugable ? " (playable: " + indexed->entryPath + ")"
                                  : " (scenarios only)")
                      << "\n";
        }
        const auto legacy = ProjectSerializer::deserializeProject(
            R"({"id":"legacy","nombre":"Legacy","niveles":[]})");
        require(!legacy.isOk() && legacy.errorMessage().find("migrar_proyectos.py") != std::string::npos,
                "legacy format must explain how to migrate");
        std::cout << "[PASS] legacy format rejected with migration command\n";
        const auto mixed = ProjectSerializer::deserializeProject(
            R"({"schemaVersion":1,"name":"New","nombre":null})");
        require(!mixed.isOk() && mixed.errorMessage().find("migrar_proyectos.py") != std::string::npos,
                "mixed format accepted or missing migration advice");
        index_lifecycle();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] " << e.what() << '\n';
        return 1;
    }
}
