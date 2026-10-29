// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   D-05  (fase D, 3 j) - Objectes, equipament i consumibles
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Project/ProjectCatalog.h"
#include "Project/ProjectIndex.h"
#include "Game/GameSession.h"
#include "Level/LevelLoader.h"
#include "Render/ICombatant.h"
#include "Render/TileMap.h"
#include "TestHarness.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>

namespace {
namespace fs = std::filesystem;

struct Fixture {
    fs::path root = fs::temp_directory_path() / ("motor-project-catalog-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Fixture() { fs::create_directories(root / "custom/objects"); }
    ~Fixture() { std::error_code ec; fs::remove_all(root, ec); }
    void write(const std::string& name, const std::string& data) {
        std::ofstream output(root / "custom/objects" / name);
        output << data;
        MOTOR_CHECK(output.good());
    }
    Editor::Project project() const {
        Editor::Project p;
        p.id = "custom";
        p.name = "Custom";
        p.settings.catalogsDirectory = "custom/objects";
        return p;
    }
};

Editor::Project realProject(const std::string& id) {
    const auto index = Editor::ProjectIndex::scan();
    MOTOR_CHECK(index.isOk());
    MOTOR_CHECK(index.value().find(id));
    MOTOR_CHECK(index.value().check(id).ok());
    return *index.value().find(id);
}

class Body : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 60; }
    int maxHealth() const override { return 60; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "catalog_test_player"; }
};

MOTOR_TEST_CASE(empty_project_does_not_inherit_test_objects) {
    Fixture f;
    const auto loaded = Editor::ProjectCatalog::load(f.project(), f.root.string());
    MOTOR_CHECK(loaded.isOk());
    MOTOR_CHECK(loaded.value().objects.size() == 0);
    MOTOR_CHECK(loaded.value().paths.empty());
    ObjectCatalog empty;
    MOTOR_CHECK(empty.loadFromFile("assets/objects/empty_objects.json").isOk());
    MOTOR_CHECK(empty.size() == 0);
}

MOTOR_TEST_CASE(custom_directory_and_declared_override_order_reach_playtest_paths) {
    Fixture f;
    f.write("base.json", R"({"objects":[{"id":"item","category":"pickup","price":10}]})");
    f.write("override.json", R"({"objects":[{"id":"item","category":"pickup","price":42}]})");
    auto project = f.project();
    project.catalogs = {"base.json", "override.json"};
    const auto loaded = Editor::ProjectCatalog::load(project, f.root.string());
    MOTOR_CHECK(loaded.isOk());
    MOTOR_CHECK(loaded.value().objects.find("item")->pickup.price == 42);
    MOTOR_CHECK(loaded.value().objects.size() == 1);
    MOTOR_CHECK(loaded.value().paths.size() == 2);
    ObjectCatalog playtest;
    for (const auto& path : loaded.value().paths) MOTOR_CHECK(playtest.loadFromFile(path).isOk());
    MOTOR_CHECK(playtest.ids() == loaded.value().objects.ids());
    MOTOR_CHECK(playtest.find("item")->pickup.price == 42);
    MOTOR_CHECK(loaded.value().paths.front() == (f.root / "custom/objects/base.json").generic_string());
}

MOTOR_TEST_CASE(missing_or_malformed_catalog_reports_project_and_file) {
    Fixture f;
    f.write("base.json", R"({"objects":[{"id":"partial","category":"prop"}]})");
    auto project = f.project();
    project.catalogs = {"base.json", "broken.json"};
    for (bool malformed : {false, true}) {
        if (malformed) f.write("broken.json", "{broken");
        const auto loaded = Editor::ProjectCatalog::load(project, f.root.string());
        MOTOR_CHECK(!loaded.isOk());
        MOTOR_CHECK(loaded.errorMessage().find("custom") != std::string::npos);
        MOTOR_CHECK(loaded.errorMessage().find("broken.json") != std::string::npos);
    }
}

MOTOR_TEST_CASE(invalid_project_paths_are_rejected_before_loading) {
    Fixture f;
    auto project = f.project();
    project.catalogs = {"../outside.json"};
    MOTOR_CHECK(!Editor::ProjectCatalog::load(project, f.root.string()).isOk());
    project.catalogs.clear();
    project.settings.catalogsDirectory = "../outside";
    MOTOR_CHECK(!Editor::ProjectCatalog::load(project, f.root.string()).isOk());
}

MOTOR_TEST_CASE(boundington_market_opens_with_three_real_prices) {
    const auto project = realProject("boundington");
    MOTOR_CHECK(std::find(project.catalogs.begin(), project.catalogs.end(), "test_objects.json") == project.catalogs.end());
    MOTOR_CHECK(std::find(project.levels.begin(), project.levels.end(), "mazmorra_64x64.json") == project.levels.end());
    MOTOR_CHECK(std::find(project.maps.begin(), project.maps.end(), "mazmorra_64x64.tmx") == project.maps.end());
    auto catalog = Editor::ProjectCatalog::load(project);
    MOTOR_CHECK(catalog.isOk());
    MOTOR_CHECK(!catalog.value().objects.has("slime"));
    MOTOR_CHECK(!catalog.value().objects.has("llave_cueva"));
    const auto loaded = LevelLoader::loadFromFile("assets/levels/interior_mercado.json");
    MOTOR_CHECK(loaded.isOk());
    auto level = loaded.value();
    level.playerStart = {6, 5}; // Delante del tendero real del mercado.
    TileMap map;
    MOTOR_CHECK(map.loadFromFile(level.mapPath).isOk());
    Body body;
    SkillCatalog skills;
    SkillSet learned{8};
    std::vector<std::string> inventory;
    GameSession session(&map, &catalog.value().objects, &skills, level, &body, &learned, &inventory);
    MOTOR_CHECK(session.interact());
    // TIENDA-02: el tendero saluda («¡Bienvenido al mercado!») y, al cerrar, la tienda.
    MOTOR_CHECK(session.mode() == GameMode::Dialogue);
    MOTOR_CHECK(!session.dialogueLines().empty() && session.dialogueLines()[0] == "¡Bienvenido al mercado!");
    session.closeInteraction();
    MOTOR_CHECK(session.mode() == GameMode::Shop);
    const auto& offers = session.shopOffers();
    MOTOR_CHECK(offers.size() == 3);
    MOTOR_CHECK(offers[0].objectId == "pocion" && offers[0].price == 30);
    MOTOR_CHECK(offers[1].objectId == "eter" && offers[1].price == 45);
    MOTOR_CHECK(offers[2].objectId == "pan" && offers[2].price == 8);
    session.addGold(100);
    MOTOR_CHECK(session.buy(0));
    MOTOR_CHECK(session.gold() == 70);
    MOTOR_CHECK(inventory == std::vector<std::string>{"pocion"});
    MOTOR_CHECK(!session.buy(offers.size()));
    session.closeInteraction();
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
    MOTOR_CHECK(session.gold() == 70 && inventory.size() == 1);
}

MOTOR_TEST_CASE(examples_keep_fixtures_and_all_common_categories_without_duplicate_ids) {
    const auto project = realProject("ejemplos");
    MOTOR_CHECK(project.entryKind == Editor::Project::EntryKind::Level);
    MOTOR_CHECK(project.entryPath == "assets/levels/ejemplos_objetos.json");
    auto loaded = Editor::ProjectCatalog::load(project);
    MOTOR_CHECK(loaded.isOk());
    std::set<std::string> ids;
    for (const auto& path : loaded.value().paths) {
        ObjectCatalog file;
        MOTOR_CHECK(file.loadFromFile(path).isOk());
        for (const auto& id : file.ids()) MOTOR_CHECK(ids.insert(id).second);
    }
    std::set<ObjectCategory> categories;
    for (const auto& filename : project.levels) {
        const auto level = LevelLoader::loadFromFile("assets/levels/" + filename);
        MOTOR_CHECK(level.isOk());
        TileMap map;
        MOTOR_CHECK(map.loadFromFile(level.value().mapPath).isOk());
        for (const auto& spawn : level.value().objects) {
            const auto* def = loaded.value().objects.find(spawn.objectId);
            MOTOR_CHECK(def);
            categories.insert(def->category);
            const auto& tile = map.getTile(0, spawn.position.x, spawn.position.y);
            MOTOR_CHECK(!tile.isEmpty() && !tile.hasCollision());
        }
    }
    MOTOR_CHECK(categories.size() == 4);
    MOTOR_CHECK(loaded.value().objects.find("ejemplos_guia")->dialogue.lines.size() == 3);
    MOTOR_CHECK(loaded.value().objects.find("ejemplos_tendero")->shop.items.size() == 2);
    MOTOR_CHECK(loaded.value().objects.find("ejemplos_negocio")->business.price == 100);
    MOTOR_CHECK(!loaded.value().objects.has("tendero_mercado"));
}

MOTOR_TEST_CASE(example_gallery_exit_resolves_inside_its_project) {
    const auto project = realProject("ejemplos");
    const auto gallery = LevelLoader::loadFromFile(project.entryPath);
    MOTOR_CHECK(gallery.isOk());
    const auto& exit = gallery.value().objects.back();
    MOTOR_CHECK(exit.objectId == "ejemplos_puerta");
    MOTOR_CHECK(exit.targetLevel == "assets/levels/mazmorra_64x64.json");
    const auto target = LevelLoader::loadFromFile(exit.targetLevel);
    MOTOR_CHECK(target.isOk());
    TileMap map;
    MOTOR_CHECK(map.loadFromFile(target.value().mapPath).isOk());
    MOTOR_CHECK(!map.getTile(0, exit.targetPosition.x, exit.targetPosition.y).hasCollision());
}
}  // namespace

int main() { return motor_test::runAllTests(); }
