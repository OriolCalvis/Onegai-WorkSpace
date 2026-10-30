#include "Editor/EditorDocumentIO.h"
#include "Import/TmxMapImporter.h"
#include "Level/LevelLoader.h"
#include "Project/ProjectSerializer.h"
#include "TestHarness.h"

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <string>
#include <variant>

namespace {
namespace fs = std::filesystem;

struct Fixture {
    fs::path root;
    Fixture() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (unsigned suffix = 0;; ++suffix) {
            root = fs::temp_directory_path() /
                ("motor editor io " + std::to_string(stamp) + "-" + std::to_string(suffix));
            if (fs::create_directory(root)) break;
        }
    }
    ~Fixture() { std::error_code ignored; fs::remove_all(root, ignored); }
    std::string path(const std::string& name) const { return (root / name).string(); }
};

template<class Id>
Id parseId(const char* text) {
    const auto result = Id::parse(text);
    if (!result.isOk()) throw std::runtime_error(result.errorMessage());
    return result.value();
}

std::string readText(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("No se pudo leer " + path.string());
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void writeText(const fs::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary);
    output << text;
    output.close();
    if (!output) throw std::runtime_error("No se pudo escribir " + path.string());
}

bool sameCell(GridCoord a, GridCoord b) { return a.x == b.x && a.y == b.y; }

std::set<std::string> entries(const Fixture& fixture) {
    std::set<std::string> names;
    for (const auto& entry : fs::directory_iterator(fixture.root)) {
        names.insert(entry.path().filename().string());
    }
    return names;
}

MapDocument authoringDocument() {
    MapDocument document(parseId<ids::MapId>("io_map"), "Patio de Oriol", 3, 2, 64, 32);
    MapEntityDocument door;
    door.id = parseId<ids::EntityId>("puerta_unica");
    door.objectId = parseId<ids::AssetId>("puerta");
    door.position = {1, 0};
    door.patrolMin = {0, 0};
    door.patrolMax = {2, 1};
    door.targetLevel = "levels/posada.json";
    door.hasTargetPosition = true;
    door.targetPosition = {8, 9};
    door.loot = {"llave", "pocion"};
    door.lootTable = "tesoro";
    door.eventId = "abrir_posada";
    door.displayName = "Posada \"A&B\"";
    door.variant = "madera";
    door.scale = 1.5f;
    door.effectOverride = "cerrada";
    door.properties = {{"nota", "primera\nsegunda"}, {"llave", "llave"}};

    // Insercion desordenada: el documento preserva el order explicito,
    // incluidas las capas que el TMX de interoperabilidad no representa.
    MOTOR_CHECK(document.addLayer(MapLayer::tiles(
        {parseId<ids::LayerId>("tejados"), "Tejados & \"sombras\"", false, true, 0.5f, 30},
        TileLayerDocument(3, 2, {0, 0, 2, 0, 0, 0}))).isOk());
    MOTOR_CHECK(document.addLayer(MapLayer::entities(
        {parseId<ids::LayerId>("puertas"), "Puertas", true, true, 0.75f, 20},
        EntityLayerDocument({door}))).isOk());
    MOTOR_CHECK(document.addLayer(MapLayer::tiles(
        {parseId<ids::LayerId>("suelo"), "Suelo <plaza>", true, false, 1.0f, 10},
        TileLayerDocument(3, 2, {1, 1, 2, 1, 2, 1}))).isOk());
    MOTOR_CHECK(document.addLayer(MapLayer::regions(
        {parseId<ids::LayerId>("zonas"), "Zona de encuentro", false, false, 1.0f, 40},
        RegionLayerDocument({{parseId<ids::RegionId>("entrada"), "Entrada", {0, 1, 0, 1}}}))).isOk());
    MOTOR_CHECK(document.setCollisionGids({2}).isOk());
    return document;
}

EditorState editorFixture() {
    EditorState editor(1, 1);
    MOTOR_CHECK(editor.setDocument(authoringDocument()).isOk());
    editor.setPlayerStart({2, 1});
    return editor;
}

TmxTilesetSettings tileset() {
    TmxTilesetSettings settings;
    settings.collisionGids = {2};
    settings.tilesetName = "Piedra & madera";
    settings.imageSource = "texturas/atlas & suelo.png";
    return settings;
}

MOTOR_TEST_CASE(native_bundle_reopens_all_authoring_layers_and_level_references) {
    Fixture fixture;
    EditorState editor = editorFixture();
    const auto saved = EditorDocumentIO::save(editor, "Nivel \"plaza\"", fixture.path("plaza.tmx"),
                                             fixture.path("nivel.json"), tileset());
    MOTOR_CHECK(saved.isOk());
    const auto level = LevelLoader::loadFromFile(fixture.path("nivel.json"));
    MOTOR_CHECK(level.isOk());
    MOTOR_CHECK(level.value().name == "Nivel \"plaza\"");
    MOTOR_CHECK(level.value().mapPath == fixture.path("plaza.tmx"));
    MOTOR_CHECK(level.value().mapDocumentPath == fixture.path("plaza.map.json"));
    MOTOR_CHECK(sameCell(level.value().playerStart, {2, 1}));

    const auto loaded = ProjectSerializer::loadMapFile(level.value().mapDocumentPath);
    MOTOR_CHECK(loaded.isOk());
    EditorState reopened(1, 1);
    MOTOR_CHECK(reopened.setDocument(loaded.value()).isOk());
    reopened.setPlayerStart(level.value().playerStart);
    const auto expected = ProjectSerializer::serializeMap(editor.document());
    const auto actual = ProjectSerializer::serializeMap(reopened.document());
    MOTOR_CHECK(expected.isOk() && actual.isOk());
    MOTOR_CHECK(actual.value() == expected.value());

    const auto& layers = reopened.document().layers();
    MOTOR_CHECK(layers.size() == 4);
    MOTOR_CHECK(layers[0].meta().name == "Suelo <plaza>");
    MOTOR_CHECK(layers[1].type() == MapLayerType::Entity);
    MOTOR_CHECK(layers[2].meta().id == parseId<ids::LayerId>("tejados"));
    MOTOR_CHECK(layers[2].meta().order == 30);
    MOTOR_CHECK(!layers[2].meta().visible && layers[2].meta().locked);
    MOTOR_CHECK(layers[2].meta().opacity == 0.5f);
    MOTOR_CHECK(layers[3].type() == MapLayerType::Region);
    const auto& entity = std::get<EntityLayerDocument>(layers[1].data()).entities().front();
    MOTOR_CHECK(entity.id == parseId<ids::EntityId>("puerta_unica"));
    MOTOR_CHECK(entity.properties.at("nota") == "primera\nsegunda");
    MOTOR_CHECK(level.value().objects.size() == 1);
    const auto& runtime = level.value().objects.front();
    MOTOR_CHECK(runtime.objectId == entity.objectId.value());
    MOTOR_CHECK(runtime.displayName == entity.displayName);
    MOTOR_CHECK(runtime.targetLevel == entity.targetLevel);
    MOTOR_CHECK(runtime.hasTargetPosition && sameCell(runtime.targetPosition, entity.targetPosition));
    MOTOR_CHECK(runtime.loot == entity.loot);
    MOTOR_CHECK(runtime.properties == entity.properties);
    MOTOR_CHECK((entries(fixture) == std::set<std::string>{"plaza.tmx", "plaza.map.json", "nivel.json"}));
}

MOTOR_TEST_CASE(saved_tmx_imports_tiles_order_names_visibility_and_collision) {
    Fixture fixture;
    EditorState editor = editorFixture();
    MOTOR_CHECK(EditorDocumentIO::save(editor, "Plaza", fixture.path("plaza.tmx"),
                                      fixture.path("nivel.json"), tileset()).isOk());
    const auto imported = TmxMapImporter().importFile(fixture.path("plaza.tmx"),
                                                     parseId<ids::MapId>("imported"));
    MOTOR_CHECK(!imported.hasErrors());
    const auto& document = imported.document;
    MOTOR_CHECK(document.width() == 3 && document.height() == 2);
    MOTOR_CHECK(document.tileWidth() == 64 && document.tileHeight() == 32);
    MOTOR_CHECK(document.collisionGids() == std::vector<int>{2});
    MOTOR_CHECK(document.layers().size() == 2);
    MOTOR_CHECK(document.layers()[0].meta().name == "Suelo <plaza>");
    MOTOR_CHECK(document.layers()[1].meta().name == "Tejados & \"sombras\"");
    MOTOR_CHECK(!document.layers()[1].meta().visible);
    MOTOR_CHECK(document.layers()[1].meta().opacity == 0.5f);
    MOTOR_CHECK((std::get<TileLayerDocument>(document.layers()[0].data()).gids() ==
                 std::vector<int>{1, 1, 2, 1, 2, 1}));
    MOTOR_CHECK((std::get<TileLayerDocument>(document.layers()[1].data()).gids() ==
                 std::vector<int>{0, 0, 2, 0, 0, 0}));
}

MOTOR_TEST_CASE(overwriting_a_bundle_updates_every_output_without_backup_debris) {
    Fixture fixture;
    EditorState editor = editorFixture();
    const std::array<std::string, 3> names = {"plaza.map.json", "plaza.tmx", "nivel.json"};
    for (const auto& name : names) writeText(fixture.path(name), "contenido anterior");
    MOTOR_CHECK(EditorDocumentIO::save(editor, "Nuevo", fixture.path("plaza.tmx"),
                                      fixture.path("nivel.json"), tileset()).isOk());
    for (const auto& name : names) MOTOR_CHECK(readText(fixture.path(name)) != "contenido anterior");
    const auto level = LevelLoader::loadFromFile(fixture.path("nivel.json"));
    MOTOR_CHECK(level.isOk() && level.value().name == "Nuevo");
    MOTOR_CHECK(ProjectSerializer::loadMapFile(fixture.path("plaza.map.json")).isOk());
    MOTOR_CHECK((entries(fixture) == std::set<std::string>{names.begin(), names.end()}));
}

MOTOR_TEST_CASE(missing_parent_during_staging_preserves_the_existing_bundle) {
    Fixture fixture;
    EditorState editor = editorFixture();
    writeText(fixture.path("plaza.map.json"), "documento anterior");
    writeText(fixture.path("plaza.tmx"), "tmx anterior");
    writeText(fixture.path("otro.json"), "otro nivel");
    const auto before = entries(fixture);
    const auto result = EditorDocumentIO::save(editor, "Plaza", fixture.path("plaza.tmx"),
        fixture.path("carpeta_ausente/nivel.json"), tileset());
    MOTOR_CHECK(!result.isOk());
    MOTOR_CHECK(!result.errorMessage().empty());
    MOTOR_CHECK(readText(fixture.path("plaza.map.json")) == "documento anterior");
    MOTOR_CHECK(readText(fixture.path("plaza.tmx")) == "tmx anterior");
    MOTOR_CHECK(readText(fixture.path("otro.json")) == "otro nivel");
    MOTOR_CHECK(entries(fixture) == before);
}

MOTOR_TEST_CASE(directory_destination_is_rejected_before_overwriting_other_outputs) {
    for (const std::string directory : {"plaza.map.json", "plaza.tmx", "nivel.json"}) {
        Fixture fixture;
        EditorState editor = editorFixture();
        for (const std::string name : {"plaza.map.json", "plaza.tmx", "nivel.json"}) {
            if (name == directory) fs::create_directory(fixture.root / name);
            else writeText(fixture.path(name), "original " + name);
        }
        writeText(fixture.root / directory / "conservar.txt", "contenido del directorio");
        const auto before = entries(fixture);
        const auto result = EditorDocumentIO::save(editor, "Plaza", fixture.path("plaza.tmx"),
                                                  fixture.path("nivel.json"), tileset());
        MOTOR_CHECK(!result.isOk());
        MOTOR_CHECK(fs::is_directory(fixture.root / directory));
        MOTOR_CHECK(readText(fixture.root / directory / "conservar.txt") == "contenido del directorio");
        for (const std::string name : {"plaza.map.json", "plaza.tmx", "nivel.json"}) {
            if (name != directory) MOTOR_CHECK(readText(fixture.path(name)) == "original " + name);
        }
        MOTOR_CHECK(entries(fixture) == before);
    }
}

MOTOR_TEST_CASE(coincident_paths_are_rejected_including_lexical_aliases) {
    Fixture fixture;
    EditorState editor = editorFixture();
    writeText(fixture.path("plaza.map.json"), "documento anterior");
    writeText(fixture.path("plaza.tmx"), "tmx anterior");
    const auto before = entries(fixture);
    for (const std::string level : {"plaza.tmx", "./plaza.tmx", "plaza.map.json"}) {
        const auto result = EditorDocumentIO::save(editor, "Plaza", fixture.path("plaza.tmx"),
                                                  fixture.path(level), tileset());
        MOTOR_CHECK(!result.isOk());
        MOTOR_CHECK(readText(fixture.path("plaza.map.json")) == "documento anterior");
        MOTOR_CHECK(readText(fixture.path("plaza.tmx")) == "tmx anterior");
        MOTOR_CHECK(entries(fixture) == before);
    }
}

MOTOR_TEST_CASE(legacy_enemies_keep_instance_skills_alongside_document_objects) {
    Fixture fixture;
    EditorState editor = editorFixture();
    const std::vector<EnemySpawn> enemies = {
        {"slime", {0, 1}, {0, 0}, {2, 1}, {"ataque_especial", "curacion"}},
        {"slime", {2, 0}, {2, 0}, {2, 0}, {"otra_habilidad"}},
    };
    MOTOR_CHECK(EditorDocumentIO::save(editor, "Plaza", fixture.path("plaza.tmx"),
        fixture.path("nivel.json"), tileset(), enemies).isOk());
    const auto loaded = LevelLoader::loadFromFile(fixture.path("nivel.json"));
    MOTOR_CHECK(loaded.isOk());
    MOTOR_CHECK(loaded.value().objects.size() == 1);
    MOTOR_CHECK(loaded.value().enemies.size() == enemies.size());
    for (std::size_t i = 0; i < enemies.size(); ++i) {
        const auto& actual = loaded.value().enemies[i];
        MOTOR_CHECK(actual.type == enemies[i].type);
        MOTOR_CHECK(sameCell(actual.position, enemies[i].position));
        MOTOR_CHECK(sameCell(actual.patrolMin, enemies[i].patrolMin));
        MOTOR_CHECK(sameCell(actual.patrolMax, enemies[i].patrolMax));
        MOTOR_CHECK(actual.skillIds == enemies[i].skillIds);
    }
}

MOTOR_TEST_CASE(map_document_reference_is_optional_but_must_be_a_nonempty_string) {
    const auto legacy = LevelLoader::loadFromString(R"({"map":"plaza.tmx"})");
    MOTOR_CHECK(legacy.isOk() && legacy.value().mapDocumentPath.empty());
    const auto native = LevelLoader::loadFromString(
        R"({"map":"plaza.tmx","mapDocument":"maps/plaza.map.json"})");
    MOTOR_CHECK(native.isOk());
    MOTOR_CHECK(native.value().mapDocumentPath == "maps/plaza.map.json");
    for (const std::string value : {"null", "false", "42", "[]", "{}", "\"\""}) {
        const auto invalid = LevelLoader::loadFromString(
            "{\"map\":\"plaza.tmx\",\"mapDocument\":" + value + "}");
        MOTOR_CHECK(!invalid.isOk());
        MOTOR_CHECK(invalid.errorMessage().find("mapDocument") != std::string::npos);
    }
}

}  // namespace

int main() { return motor_test::runAllTests(); }
