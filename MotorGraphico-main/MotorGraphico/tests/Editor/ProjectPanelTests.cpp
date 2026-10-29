// H4-02 / E-11 — test_project_panel: los mapas de un proyecto, desde el panel
// Proyecto de motor_editor, sin ventana.
//
// Es la prueba que pide la ficha de E-11 con sus palabras: «dos projectes en
// carpetes amb espais, duplicacio sense col·lisions i cancel·lacio sense
// fitxers parcials. Recorregut UI: crear projecte, dos mapes, triar entrada,
// desar i reobrir». El recorrido va por ProjectPanelState, que es lo que el
// panel ImGui llama; el mapa se abre, se pinta, se guarda y se reabre con la
// misma EditorDocumentSession que usa motor_editor. Y el cierre de H4-02:
// «Nuevo mapa -> elegir tamano -> pintar -> guardar -> reabrir y esta».
//
// Todo contra un assetsRoot temporal CON ESPACIOS en la ruta: no toca assets/.
// Del repo solo se copian un TMX y su textura (via copiar_assets).
#include "Editor/EditorDocumentSession.h"
#include "Editor/ProjectMaps.h"
#include "Editor/ProjectPanelState.h"
#include "Core/Json/JsonValue.h"
#include "Project/ProjectIndex.h"
#include "Project/ProjectSerializer.h"
#include "Render/TileMap.h"

#include "TestHarness.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

namespace {

namespace fs = std::filesystem;
using Editor::EditorDocumentSession;
using Editor::Project;
using Editor::ProjectIndex;
using Editor::ProjectMaps;
using Editor::ProjectPanelState;
using Action = EditorDocumentSession::Action;
using Mode = ProjectPanelState::Mode;
using Kind = ProjectPanelState::Effect::Kind;

std::string readAll(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

void writeAll(const fs::path& path, const std::string& text) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << text;
}

// Un assetsRoot desechable con un proyecto «base» de un mapa (el TMX de
// ejemplo: 8x6, gid 1 suelo, gid 2 muro con collision).
struct Sandbox {
    fs::path top, root;
    Sandbox() {
        top = fs::temp_directory_path() /
              ("motor panel proyecto " +
               std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        root = top / "assets de prueba";
        for (const char* dir : {"proyectos", "levels", "maps", "textures", "objects", "adventures"})
            fs::create_directories(root / dir);
        fs::copy_file("assets/maps/ejemplo_mapa.tmx", root / "maps/base_mapa.tmx");
        fs::copy_file("assets/textures/test_checker.png", root / "textures/test_checker.png");
        writeAll(root / "levels/base_inicio.json",
                 "{\n  \"name\": \"Inicio de base\",\n  \"map\": \"" +
                     (root / "maps/base_mapa.tmx").generic_string() +
                     "\",\n  \"playerStart\": { \"x\": 1, \"y\": 1 },\n  \"objects\": []\n}\n");
        writeAll(root / "proyectos/index.json", R"({"proyectos": ["base"]})");
        writeAll(root / "proyectos/base.json",
                 manifest("base", "Base", "base_", "maps", "assets/levels/base_inicio.json",
                          R"(["base_inicio.json"])", R"(["base_mapa.tmx"])"));
    }
    ~Sandbox() {
        std::error_code ec;
        fs::remove_all(top, ec);
    }
    static std::string manifest(const std::string& id, const std::string& name,
                                const std::string& prefix, const std::string& mapsDir,
                                const std::string& entry, const std::string& levels,
                                const std::string& maps) {
        return R"({"schemaVersion": 1, "id": ")" + id + R"(", "name": ")" + name +
               R"(", "prefix": ")" + prefix +
               R"(", "description": "", "epoch": "", "author": "test", "settings": {"schemaVersion": 1, )" +
               R"("mapsDirectory": ")" + mapsDir + R"(", "levelsDirectory": "levels", )" +
               R"("adventuresDirectory": "adventures", "catalogsDirectory": "objects", "initialMap": ""}, )" +
               R"("entry": ")" + entry + R"(", "levels": )" + levels + R"(, "maps": )" + maps +
               R"(, "adventures": [], "catalogs": []})";
    }
    // Un segundo proyecto en el mismo assetsRoot.
    void addProject(const std::string& id, const std::string& prefix,
                    const std::string& mapsDir = "maps") {
        writeAll(root / ("proyectos/" + id + ".json"),
                 manifest(id, "Proyecto " + id, prefix, mapsDir, "", "[]", "[]"));
        auto index = JsonValue::parse(readAll(root / "proyectos/index.json")).value();
        std::string ids;
        for (std::size_t i = 0; i < index["proyectos"].size(); ++i)
            ids += "\"" + index["proyectos"][i].asString() + "\", ";
        writeAll(root / "proyectos/index.json", "{\"proyectos\": [" + ids + "\"" + id + "\"]}");
    }
    std::string str() const { return root.generic_string(); }
    bool has(const std::string& rel) const { return fs::exists(root / rel); }
    Project project(const std::string& id) const {
        auto p =
            ProjectSerializer::loadProjectFile((root / ("proyectos/" + id + ".json")).string());
        MOTOR_CHECK(p.isOk());
        return p.value();
    }
    // Foto de todo lo que hay en el assetsRoot (nombres y bytes).
    std::string snapshot() const {
        std::string all;
        std::set<std::string> files;
        for (const auto& entry : fs::recursive_directory_iterator(root))
            if (entry.is_regular_file())
                files.insert(entry.path().generic_string());
        for (const auto& file : files)
            all += file + "\n" + readAll(file) + "\n";
        return all;
    }
};

bool contains(const std::vector<std::string>& names, const std::string& name) {
    return std::find(names.begin(), names.end(), name) != names.end();
}

Editor::ProjectTileset firstTileset(const Sandbox& s, const std::string& projectId) {
    auto index = ProjectIndex::scan(s.str());
    MOTOR_CHECK(index.isOk());
    const auto tilesets = ProjectMaps::tilesets(s.str(), index.value().projects(), projectId);
    MOTOR_CHECK(!tilesets.empty());
    return tilesets.front();
}

}  // namespace

MOTOR_TEST_CASE(ruta_del_autor_proyecto_nuevo_dos_mapas_entrada_guardar_y_reabrir) {
    Sandbox s;
    ProjectPanelState panel(s.str());
    MOTOR_CHECK(panel.reload().isOk());
    MOTOR_CHECK(panel.projects().size() == 1);
    EditorDocumentSession session;

    MOTOR_CHECK(panel.begin(Mode::NewProject));
    panel.form().text = "La cueva del eco";
    MOTOR_CHECK(panel.previewFileId() == "la_cueva_del_eco");
    const auto created = panel.confirm(session);
    MOTOR_CHECK(created.isOk() && created.value().kind == Kind::None);
    const Project* cueva = panel.project("la_cueva_del_eco");
    MOTOR_CHECK(cueva != nullptr && cueva->prefix == "lac_" && cueva->name == "La cueva del eco");
    // Un proyecto sin mapas no se juega: el panel sigue con el primero, y los
    // tilesets salen de otro proyecto porque este todavia no tiene ninguno.
    MOTOR_CHECK(panel.form().mode == Mode::NewMap && panel.form().project == "la_cueva_del_eco");
    MOTOR_CHECK(panel.tilesets().size() == 1 && panel.tilesets()[0].name == "test_checker");
    panel.form().text = "Entrada";
    panel.form().width = 12;
    panel.form().height = 10;
    MOTOR_CHECK(panel.previewFileId() == "lac_entrada");
    const auto first = panel.confirm(session);
    MOTOR_CHECK(first.isOk() && first.value().kind == Kind::Open);
    MOTOR_CHECK(!panel.formOpen() && panel.focusProject() == "la_cueva_del_eco");
    MOTOR_CHECK(s.has("levels/lac_entrada.json") && s.has("maps/lac_entrada.tmx") &&
                s.has("maps/lac_entrada.map.json"));

    // Nuevo mapa -> pintar -> guardar -> reabrir y esta.
    const std::string entrada = first.value().path;
    MOTOR_CHECK(session.request(Action::Open, entrada).isOk());
    MOTOR_CHECK(session.state().width() == 12 && session.state().height() == 10);
    MOTOR_CHECK(session.name() == "Entrada");
    MOTOR_CHECK(session.state().tileAt(0, 0) == 1);  // suelo: la primera baldosa que no es muro
    MOTOR_CHECK(session.state().playerStart().x == 6 && session.state().playerStart().y == 5);
    session.paint(3, 3, 2);
    MOTOR_CHECK(session.dirty() && session.save().isOk());
    MOTOR_CHECK(session.request(Action::Open, entrada).isOk());
    MOTOR_CHECK(session.state().tileAt(3, 3) == 2 && session.state().tileAt(4, 3) == 1);

    MOTOR_CHECK(panel.begin(Mode::NewMap, "la_cueva_del_eco"));
    panel.form().text = "El bosque";
    MOTOR_CHECK(panel.confirm(session).isOk());
    MOTOR_CHECK(panel.maps("la_cueva_del_eco").size() == 2);

    // El juego empieza en el bosque.
    MOTOR_CHECK(panel.begin(Mode::Entry, "la_cueva_del_eco", "lac_el_bosque.json"));
    MOTOR_CHECK(panel.confirm(session).isOk());
    MOTOR_CHECK(!panel.maps("la_cueva_del_eco")[0].entry &&
                panel.maps("la_cueva_del_eco")[1].entry);
    auto index = ProjectIndex::scan(s.str());
    MOTOR_CHECK(index.isOk());
    const Project* scanned = index.value().find("la_cueva_del_eco");
    MOTOR_CHECK(scanned != nullptr && scanned->entryKind == Project::EntryKind::Level &&
                scanned->playable());

    MOTOR_CHECK(panel.begin(Mode::Title, "la_cueva_del_eco"));
    panel.form().text = "  El eco  ";
    MOTOR_CHECK(panel.confirm(session).isOk());
    MOTOR_CHECK(panel.project("la_cueva_del_eco")->settings.title == "El eco");

    // Otro panel que lee del disco ve lo mismo: el proyecto se ha guardado.
    ProjectPanelState again(s.str());
    MOTOR_CHECK(again.reload().isOk());
    const auto& maps = again.maps("la_cueva_del_eco");
    MOTOR_CHECK(maps.size() == 2 && maps[0].name == "Entrada" && maps[1].name == "El bosque" &&
                maps[1].entry);
}

MOTOR_TEST_CASE(duplicar_dos_veces_no_choca_no_toca_el_original_y_cambia_el_id_del_documento) {
    Sandbox s;
    const std::string levelBefore = readAll(s.root / "levels/base_inicio.json");
    const std::string tmxBefore = readAll(s.root / "maps/base_mapa.tmx");
    const auto a = ProjectMaps::duplicate(s.str(), "base", "base_inicio.json", "Copia");
    const auto b = ProjectMaps::duplicate(s.str(), "base", "base_inicio.json", "Copia");
    MOTOR_CHECK(a.isOk() && b.isOk());
    MOTOR_CHECK(a.value().level == "base_copia.json" && b.value().level == "base_copia_2.json");
    MOTOR_CHECK(readAll(s.root / "levels/base_inicio.json") == levelBefore);
    MOTOR_CHECK(readAll(s.root / "maps/base_mapa.tmx") == tmxBefore);
    MOTOR_CHECK(readAll(s.root / "maps/base_copia.tmx") ==
                tmxBefore);  // misma carpeta: bytes iguales
    const auto copy = JsonValue::parse(readAll(s.root / "levels/base_copia.json")).value();
    MOTOR_CHECK(copy["name"].asString() == "Copia");
    MOTOR_CHECK(copy["map"].asString() == (s.root / "maps/base_copia.tmx").generic_string());
    const Project project = s.project("base");
    MOTOR_CHECK(project.levels.size() == 3 && contains(project.maps, "base_copia_2.tmx"));

    // Con documento de autoria: guardar el original desde el editor lo crea;
    // la copia lleva el suyo, con SU id, y se abre con lo pintado.
    EditorDocumentSession session;
    MOTOR_CHECK(session.request(Action::Open, (s.root / "levels/base_inicio.json").generic_string())
                    .isOk());
    session.paint(2, 2, 2);
    MOTOR_CHECK(session.save().isOk() && s.has("maps/base_mapa.map.json"));
    const auto c = ProjectMaps::duplicate(s.str(), "base", "base_inicio.json", "Con documento");
    MOTOR_CHECK(c.isOk() && s.has("maps/base_con_documento.map.json"));
    const auto doc =
        ProjectSerializer::loadMapFile((s.root / "maps/base_con_documento.map.json").string());
    MOTOR_CHECK(doc.isOk() && doc.value().id().value() == "base_con_documento");
    const auto withDoc =
        JsonValue::parse(readAll(s.root / "levels/base_con_documento.json")).value();
    MOTOR_CHECK(withDoc["mapDocument"].asString() ==
                (s.root / "maps/base_con_documento.map.json").generic_string());
    EditorDocumentSession copySession;
    MOTOR_CHECK(copySession.request(Action::Open, c.value().path).isOk());
    MOTOR_CHECK(copySession.state().tileAt(2, 2) == 2 && copySession.name() == "Con documento");
}

MOTOR_TEST_CASE(dos_proyectos_en_carpetas_con_espacios_y_cada_uno_con_sus_mapas) {
    Sandbox s;
    s.addProject("otro", "otr_");
    const auto tileset = firstTileset(s, "otro");  // otro no tiene mapas: sale el de base
    MOTOR_CHECK(tileset.source.find("base_mapa.tmx") != std::string::npos);
    const auto mine = ProjectMaps::create(s.str(), "otro", "Entrada", 8, 8, tileset);
    const auto theirs = ProjectMaps::create(s.str(), "base", "Entrada", 8, 8, tileset);
    MOTOR_CHECK(mine.isOk() && theirs.isOk());
    MOTOR_CHECK(mine.value().level == "otr_entrada.json" &&
                theirs.value().level == "base_entrada.json");
    MOTOR_CHECK(s.project("otro").levels.size() == 1 && s.project("base").levels.size() == 2);
    // La imagen del tileset nuevo apunta a la misma textura que el original.
    TileMap created, original;
    MOTOR_CHECK(created.loadFromFile((s.root / "maps/otr_entrada.tmx").string()).isOk());
    MOTOR_CHECK(original.loadFromFile((s.root / "maps/base_mapa.tmx").string()).isOk());
    MOTOR_CHECK(fs::equivalent(created.tilesetImagePath(), original.tilesetImagePath()));
    MOTOR_CHECK(created.getCollisionGids() == original.getCollisionGids());
    // Un nombre que ya esta cogido en disco (aunque no en el manifiesto) no se pisa.
    writeAll(s.root / "levels/otr_suelto.json", "{}");
    const auto loose = ProjectMaps::create(s.str(), "otro", "Suelto", 5, 5, tileset);
    MOTOR_CHECK(loose.isOk() && loose.value().level == "otr_suelto_2.json");
    MOTOR_CHECK(readAll(s.root / "levels/otr_suelto.json") == "{}");
}

MOTOR_TEST_CASE(cancelar_o_fallar_no_deja_ficheros_ni_manifiestos_a_medias) {
    Sandbox s;
    s.addProject("roto", "rot_", "mapas_roto");
    writeAll(s.root / "mapas_roto", "esto es un fichero, no una carpeta");
    writeAll(s.root / "levels/base_sin_mapa.json",
             R"({"name": "Sin mapa", "map": "no/existe.tmx"})");
    MOTOR_CHECK(ProjectIndex::addLevelToProject(s.str(), "base", "base_sin_mapa.json").isOk());
    const auto tileset = firstTileset(s, "base");
    const std::string before = s.snapshot();

    ProjectPanelState panel(s.str());
    EditorDocumentSession session;
    MOTOR_CHECK(panel.reload().isOk() && panel.begin(Mode::NewMap, "base"));
    panel.form().text = "Nunca";
    panel.cancel();
    MOTOR_CHECK(!panel.formOpen());
    MOTOR_CHECK(panel.begin(Mode::NewMap, "base"));
    panel.form().text = "¿?";
    MOTOR_CHECK(!panel.confirm(session).isOk() && panel.formOpen() && !panel.form().error.empty());
    panel.form().text = "Diminuto";
    panel.form().width = 2;
    MOTOR_CHECK(!panel.confirm(session).isOk() && panel.formOpen());

    Editor::ProjectTileset empty = tileset;
    empty.settings.tileCount = 0;
    MOTOR_CHECK(
        !ProjectMaps::create(s.str(), "base", "Grande", ProjectMaps::kMaxSide + 1, 8, tileset)
             .isOk());
    MOTOR_CHECK(!ProjectMaps::create(s.str(), "base", "Sin tileset", 8, 8, empty).isOk());
    MOTOR_CHECK(!ProjectMaps::create(s.str(), "no_existe", "Nada", 8, 8, tileset).isOk());
    MOTOR_CHECK(!ProjectMaps::create(s.str(), "roto", "Nada", 8, 8, tileset)
                     .isOk());  // no se puede escribir
    MOTOR_CHECK(!ProjectMaps::duplicate(s.str(), "base", "no_esta.json", "Copia").isOk());
    MOTOR_CHECK(!ProjectMaps::duplicate(s.str(), "base", "base_sin_mapa.json", "Copia").isOk());
    MOTOR_CHECK(!ProjectMaps::rename(s.str(), "base", "base_inicio.json", "   ").isOk());
    MOTOR_CHECK(s.snapshot() == before);
}

MOTOR_TEST_CASE(renombrar_cambia_solo_el_nombre_y_quitar_saca_del_proyecto_sin_borrar) {
    Sandbox s;
    const std::string before = readAll(s.root / "levels/base_inicio.json");
    MOTOR_CHECK(ProjectMaps::rename(s.str(), "base", "base_inicio.json", "Plaza \"mayor\"").isOk());
    std::string expected = before;
    expected.replace(expected.find("Inicio de base"), std::string("Inicio de base").size(),
                     "Plaza \\\"mayor\\\"");
    MOTOR_CHECK(readAll(s.root / "levels/base_inicio.json") == expected);  // diff de una linea
    MOTOR_CHECK(JsonValue::parse(expected).value()["name"].asString() == "Plaza \"mayor\"");

    // La entrada no se quita; un mapa cualquiera si, y sus ficheros se quedan.
    MOTOR_CHECK(!ProjectMaps::remove(s.str(), "base", "base_inicio.json").isOk());
    const auto extra = ProjectMaps::create(s.str(), "base", "Extra", 6, 6, firstTileset(s, "base"));
    MOTOR_CHECK(extra.isOk());
    Project project = s.project("base");
    project.settings.initialMap = "base_extra.tmx";
    MOTOR_CHECK(
        ProjectSerializer::saveProjectAtomic((s.root / "proyectos/base.json").string(), project)
            .isOk());
    MOTOR_CHECK(ProjectMaps::remove(s.str(), "base", "base_extra.json").isOk());
    project = s.project("base");
    MOTOR_CHECK(!contains(project.levels, "base_extra.json") &&
                !contains(project.maps, "base_extra.tmx"));
    MOTOR_CHECK(project.settings.initialMap.empty());
    MOTOR_CHECK(s.has("levels/base_extra.json") && s.has("maps/base_extra.tmx"));

    // Un TMX que usa otro nivel del proyecto se queda en maps.
    writeAll(s.root / "levels/base_otra_vista.json",
             "{\"name\": \"Otra vista\", \"map\": \"" +
                 (s.root / "maps/base_mapa.tmx").generic_string() + "\"}");
    MOTOR_CHECK(ProjectIndex::addLevelToProject(s.str(), "base", "base_otra_vista.json").isOk());
    MOTOR_CHECK(ProjectMaps::remove(s.str(), "base", "base_otra_vista.json").isOk());
    MOTOR_CHECK(contains(s.project("base").maps, "base_mapa.tmx"));
}

MOTOR_TEST_CASE(el_panel_no_pisa_el_mapa_abierto_con_cambios_y_luego_lo_recarga_o_lo_cierra) {
    Sandbox s;
    ProjectPanelState panel(s.str());
    MOTOR_CHECK(panel.reload().isOk());
    EditorDocumentSession session;
    const std::string inicio = panel.maps("base")[0].path;
    MOTOR_CHECK(session.request(Action::Open, inicio).isOk());
    session.paint(2, 2, 2);
    MOTOR_CHECK(panel.begin(Mode::RenameMap, "base", "base_inicio.json"));
    MOTOR_CHECK(panel.form().text == "Inicio de base");
    panel.form().text = "Renombrado";
    MOTOR_CHECK(!panel.confirm(session).isOk() && panel.formOpen());
    MOTOR_CHECK(JsonValue::parse(readAll(inicio)).value()["name"].asString() == "Inicio de base");
    MOTOR_CHECK(session.save().isOk());
    const auto renamed = panel.confirm(session);
    MOTOR_CHECK(renamed.isOk() && renamed.value().kind == Kind::Reload &&
                renamed.value().path == inicio);
    MOTOR_CHECK(session.request(Action::Open, inicio).isOk() && session.name() == "Renombrado");

    // El mismo fichero escrito de otra forma (como llega con --load) sigue
    // siendo el mapa abierto: se recarga con la ruta con la que se abrio.
    const std::string otherForm = (s.root / "levels/../levels/./base_inicio.json").generic_string();
    MOTOR_CHECK(session.request(Action::Open, otherForm).isOk());
    MOTOR_CHECK(panel.begin(Mode::RenameMap, "base", "base_inicio.json"));
    panel.form().text = "Otra vez";
    const auto again = panel.confirm(session);
    MOTOR_CHECK(again.isOk() && again.value().kind == Kind::Reload &&
                again.value().path == otherForm);

    MOTOR_CHECK(panel.begin(Mode::DuplicateMap, "base", "base_inicio.json"));
    MOTOR_CHECK(panel.form().text == "Copia de Otra vez");
    const auto copy = panel.confirm(session);
    MOTOR_CHECK(copy.isOk() && copy.value().kind == Kind::Open);
    MOTOR_CHECK(session.request(Action::Open, copy.value().path).isOk());
    MOTOR_CHECK(panel.begin(Mode::RemoveMap, "base", "base_copia_de_otra_vez.json"));
    const auto removed = panel.confirm(session);
    MOTOR_CHECK(removed.isOk() && removed.value().kind == Kind::Close);
    MOTOR_CHECK(panel.maps("base").size() == 1);
    MOTOR_CHECK(!panel.begin(Mode::RenameMap, "base", "no_esta.json") &&
                !panel.begin(Mode::NewMap, "nadie"));
}

MOTOR_TEST_CASE(cambiar_una_cadena_de_primer_nivel_no_toca_nada_mas) {
    const std::string nested =
        R"({"a": {"name": "dentro"}, "t": "a\"name\": \"x\"", "name": "fuera", "n": [1, {"name": 2}]})";
    const auto replaced = Editor::replaceTopLevelString(nested, "name", "nuevo", false);
    MOTOR_CHECK(replaced.isOk());
    MOTOR_CHECK(
        replaced.value() ==
        R"({"a": {"name": "dentro"}, "t": "a\"name\": \"x\"", "name": "nuevo", "n": [1, {"name": 2}]})");
    const auto escaped =
        Editor::replaceTopLevelString(nested, "t", "comillas \" y \\ barra\n", false);
    MOTOR_CHECK(escaped.isOk() && JsonValue::parse(escaped.value()).value()["t"].asString() ==
                                      "comillas \" y \\ barra\n");
    const auto inserted = Editor::replaceTopLevelString("{\n  \"a\": 1\n}", "name", "x", true);
    MOTOR_CHECK(inserted.isOk());
    const auto parsed = JsonValue::parse(inserted.value());
    MOTOR_CHECK(parsed.isOk() && parsed.value()["name"].asString() == "x" &&
                parsed.value()["a"].asInt() == 1);
    const auto intoEmpty = Editor::replaceTopLevelString(" {} ", "name", "x", true);
    MOTOR_CHECK(intoEmpty.isOk() &&
                JsonValue::parse(intoEmpty.value()).value()["name"].asString() == "x");
    MOTOR_CHECK(!Editor::replaceTopLevelString(R"({"name": 3})", "name", "x", true).isOk());
    MOTOR_CHECK(!Editor::replaceTopLevelString("[1, 2]", "name", "x", true).isOk());
    MOTOR_CHECK(!Editor::replaceTopLevelString(R"({"a": "b"})", "name", "x", false).isOk());
    MOTOR_CHECK(
        !Editor::replaceTopLevelString(R"({"name": "sin cerrar)", "name", "x", false).isOk());
}

int main() { return motor_test::runAllTests(); }
