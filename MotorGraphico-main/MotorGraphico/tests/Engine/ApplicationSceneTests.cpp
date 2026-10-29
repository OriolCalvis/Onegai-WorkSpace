#include "Engine/Application.h"
#include "Render/PergaminoGL.h"
#include "Render/Player.h"
#include "Project/ProjectSerializer.h"
#include "Game/SaveGame.h"
#include "TestHarness.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>

// Acceso de prueba al orquestador REAL; no sustituye escenas ni el driver GL.
struct ApplicationSceneTestAccess {
    static GameSession& session(Application& a) { return *a.m_session; }
    static Engine::SceneManager& scenes(Application& a) { return a.m_scenes; }
    static bool title(Application& a) { return a.isAtTitle(); }
    static void selectCampaign(Application& a) { while (a.m_title->selected() != 0) a.m_title->moveUp(); }
    static void draw(Application& a) { a.m_scenes.render(0.0f); }
    static void confirm(Application& a) { a.confirmTitleSelection(); }
    static void boundary(Application& a) { a.finishSceneFrame(); }
    static void door(Application& a) { a.applyPendingTransition(); }
    static void update(Application& a, float dt) { a.m_scenes.fixedUpdate(dt); }
    static const TileMap& map(Application& a) { return a.m_map; }
    static Player& player(Application& a) { return *a.m_player; }
    static std::vector<std::string>& inventory(Application& a) { return a.m_inventory; }
    static RPG::NarrativeState& narrative(Application& a) { return a.m_narrativeState; }
    static bool narrativeReady(Application& a) { return a.m_narrativaLista; }
    static bool campaign(Application& a) { return a.m_narrativeCampaignTransitions; }
    static std::string script(Application& a) { return a.m_prologo.id; }
    static std::vector<std::string> catalog(Application& a) { return a.m_objectCatalog.ids(); }
    static std::string level(Application& a) { return a.m_currentLevelPath; }
    static bool quickSave(Application& a) { return a.quickSave(); }
    static bool quickLoad(Application& a) { return a.quickLoad(); }
    static bool dialogueVisible(Application& a) { return a.m_dialogue->isVisible(); }
    static bool dialogueFrameVisible(Application& a) { return a.m_dialoguePanel->isVisible(); }
    static HudIconList& shopMenu(Application& a) { return *a.m_shopMenu; }
    static HudCommandMenu& businessMenu(Application& a) { return *a.m_businessMenu; }
    // UI-BG3-01 — El combate es la barra de cartas del pergamino.
    static Game::BarraCombate& battleBar(Application& a) { return a.m_barra; }
    static bool pergamino(Application& a) { return a.m_pergamino && a.m_pergamino->listo(); }
    // TIENDA-02: la tienda es Game::TiendaEstado (pestana, foco y resultado).
    static Game::TiendaEstado& tienda(Application& a) { return a.m_tienda; }
    static void confirmShop(Application& a) { a.m_tienda.confirma(*a.m_session); }
    static void confirmBusiness(Application& a) { a.confirmBusinessCommand(); }
    static void confirmBattle(Application& a, std::size_t fila) { a.confirmBattleCommand(fila); }
    static void timer(Application& a) {
        a.m_prologo = RPG::AdventureScript{};
        RPG::TimerDefinition timer;
        timer.id = "scene_clock";
        timer.duracion = 100.0f;
        timer.flagExpira = "scene_clock_expired";
        a.m_prologo.timers.push_back(timer);
        a.m_narrative.setAdventure(&a.m_prologo);
        a.m_narrativaLista = true;
        a.m_narrativeCampaignTransitions = false;
        a.m_session->setNarrative(&a.m_narrative, &a.m_narrativeState);
        a.m_narrative.startTimer("scene_clock", a.m_narrativeState);
    }
    // NUEVA-PARTIDA-01 — «Nueva partida» abre la creación de personaje, y la
    // escena de juego se pide cuando la creación termina (lo prueba
    // test_application_cableado). Estas pruebas miran el cambio de escena,
    // no la creación: sin ella, confirmar pide esa misma escena directamente
    // (la otra rama de Application::confirmTitleSelection).
    static void sinCreacion(Application& a) { a.m_nuevaPartida.reset(); }
    // El título es el de la campaña (H0-07) y ya no lista otros proyectos:
    // hoy nada del juego llama a openProject. Se pide como lo pedía aquel
    // título, con una escena que abre el proyecto en el borde del fotograma.
    static bool openProject(Application& a, const Editor::Project& p) {
        return a.m_scenes.requestReplace(a.makeScene(false, [&a, p] { return a.openProject(p); })).isOk();
    }
};

namespace {
using Access = ApplicationSceneTestAccess;
namespace fs = std::filesystem;

struct Fixture {
    fs::path root;
    Fixture() {
        const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        for (unsigned suffix = 0;; ++suffix) {
            root = fs::temp_directory_path() / ("motor-c16-" + std::to_string(stamp) + "-" + std::to_string(suffix));
            if (fs::create_directory(root)) break;
        }
    }
    ~Fixture() { std::error_code ignored; fs::remove_all(root, ignored); }
    std::string path(const std::string& file) const { return (root / file).generic_string(); }
    void write(const std::string& file, const std::string& content) {
        fs::create_directories((root / file).parent_path());
        std::ofstream output(root / file);
        output << content;
        if (!output) throw std::runtime_error("No se pudo crear fixture " + file);
    }
    std::string level(const std::string& file, const std::string& map,
                      const std::string& target = "") {
        std::string objects = "[]";
        if (!target.empty()) objects = "[{\"objectId\":\"c16_door\",\"position\":{\"x\":1,\"y\":0},\"targetLevel\":\"" + target + "\",\"targetPosition\":{\"x\":3,\"y\":2}}]";
        write(file, "{\"name\":\"C16 fixture\",\"map\":\"" + map + "\",\"playerStart\":{\"x\":0,\"y\":0},\"enemies\":[],\"objects\":" + objects + "}");
        return path(file);
    }
    std::string catalog() {
        write("objects.json", R"({"objects":[{"id":"c16_door","name":"Puerta","category":"prop","spriteId":1,"blocksMovement":false,"interactable":true}]})");
        return path("objects.json");
    }
    void interactions() {
        write("interactions.json", R"({"name":"Interacciones HUD","map":"assets/maps/test_map.tmx",
            "playerStart":{"x":0,"y":0},"enemies":[],"objects":[
                {"objectId":"hud_npc","position":{"x":0,"y":0}},
                {"objectId":"hud_shop","position":{"x":2,"y":0}},
                {"objectId":"hud_business","position":{"x":3,"y":2}},
                {"objectId":"hud_enemy","position":{"x":0,"y":2}}
            ]})");
        write("interactions_catalog.json", R"({"objects":[
            {"id":"hud_npc","name":"Guia","category":"npc","spriteId":1,
             "dialogue":["Puedes continuar explorando."]},
            {"id":"hud_shop","name":"Tienda","category":"npc","spriteId":1,
             "dialogue":["Elige un articulo."],
             "shop":{"items":[{"objectId":"hud_bread"},{"objectId":"hud_potion"}]}},
            {"id":"hud_bread","name":"Pan","category":"pickup","price":3},
            {"id":"hud_potion","name":"Pocion HUD","category":"pickup","price":7},
            {"id":"hud_business","name":"Local","category":"prop","spriteId":1,
             "business":{"price":50,"baseIncome":5}},
            {"id":"hud_enemy","name":"Rival","category":"enemy","spriteId":1,
             "combat":{"maxHealth":30}}
        ]})");
    }
};

std::string baseMap() { return "assets/maps/test_map.tmx"; }
void init(Application& app, const std::string& level, const std::string& catalog, bool title = false,
          const std::string& adventure = "") {
    const auto result = app.init(640, 480, "C-16 escenas", level, catalog, adventure, {}, title);
    if (!result.isOk()) throw std::runtime_error(result.errorMessage());
}
void enterDoor(Application& app) {
    MOTOR_CHECK(Access::session(app).tryMovePlayer(1, 0));
    MOTOR_CHECK(Access::session(app).interact());
    MOTOR_CHECK(Access::session(app).pendingTransition().pending);
    Access::door(app);
    MOTOR_CHECK(Access::scenes(app).hasPendingTransition());
}
bool sameCell(GridCoord a, GridCoord b) { return a.x == b.x && a.y == b.y; }

void checkInteractionHud(Application& app, GameMode mode) {
    MOTOR_CHECK(Access::session(app).mode() == mode);
    Access::update(app, 0.0f);
    // UI-BG3-01: en combate el registro lo lleva la barra de cartas; el
    // cuadro de siempre solo sale si el tema pergamino no ha cargado. Lo mismo
    // el dialogo (PERGAMINO-01, la hoja), la tienda (TIENDA-02) y el negocio.
    const bool pergamino = Access::pergamino(app);
    const bool dialogue = (mode == GameMode::Dialogue || mode == GameMode::Shop || mode == GameMode::Battle ||
                           mode == GameMode::Business) &&
                          !pergamino;  // AV15-05a: el negocio tambien lleva su hoja
    MOTOR_CHECK(Access::dialogueVisible(app) == dialogue);
    MOTOR_CHECK(Access::dialogueFrameVisible(app) == dialogue);
    MOTOR_CHECK(Access::shopMenu(app).isVisible() == (mode == GameMode::Shop && !pergamino));
    MOTOR_CHECK(Access::businessMenu(app).isVisible() == (mode == GameMode::Business && !pergamino));  // AV15-05a
    Access::draw(app);
}

MOTOR_TEST_CASE(dialogue_hud_closes_and_reopens_after_title) {
    Fixture fixture;
    fixture.interactions();
    Application app;
    init(app, fixture.path("interactions.json"), fixture.path("interactions_catalog.json"), true);
    MOTOR_CHECK(Access::title(app));
    MOTOR_CHECK(!Access::dialogueVisible(app));
    MOTOR_CHECK(!Access::dialogueFrameVisible(app));
    Access::sinCreacion(app);
    Access::confirm(app);
    Access::boundary(app);
    MOTOR_CHECK(!Access::title(app));
    checkInteractionHud(app, GameMode::Exploration);

    auto& session = Access::session(app);
    MOTOR_CHECK(session.interact());
    checkInteractionHud(app, GameMode::Dialogue);
    MOTOR_CHECK(!session.tryMovePlayer(1, 0));
    MOTOR_CHECK(!session.dialogueLines().empty());
    session.closeInteraction();
    MOTOR_CHECK(session.dialogueLines().empty());
    MOTOR_CHECK(!session.log().empty());
    checkInteractionHud(app, GameMode::Exploration);

    // El historial sigue existiendo y el cuadro vuelve a abrirse al hablar.
    MOTOR_CHECK(session.interact());
    checkInteractionHud(app, GameMode::Dialogue);
    session.closeInteraction();
    checkInteractionHud(app, GameMode::Exploration);
    MOTOR_CHECK(session.tryMovePlayer(1, 0));
}

MOTOR_TEST_CASE(modal_hud_preserves_shop_business_and_battle_selection) {
    Fixture fixture;
    fixture.interactions();
    Application app;
    init(app, fixture.path("interactions.json"), fixture.path("interactions_catalog.json"));
    auto& session = Access::session(app);
    PlayerCarryOver carry;
    carry.gold = 100;
    session.applyCarryOver(carry);

    MOTOR_CHECK(session.tryMovePlayer(1, 0));
    MOTOR_CHECK(session.tryMovePlayer(1, 0));
    MOTOR_CHECK(session.interact());
    // TIENDA-02: el tendero saluda antes («Elige un articulo.») y, al cerrar, la tienda.
    checkInteractionHud(app, GameMode::Dialogue);
    session.closeInteraction();
    checkInteractionHud(app, GameMode::Shop);
    MOTOR_CHECK(!session.tryMovePlayer(1, 0));
    Access::tienda(app).ponFoco(1, 2);
    checkInteractionHud(app, GameMode::Shop);
    MOTOR_CHECK(Access::tienda(app).foco(2) == 1);  // TIENDA-02: el foco es del modelo
    const int gold = session.gold();
    Access::confirmShop(app);
    MOTOR_CHECK(session.gold() == gold - 7);
    MOTOR_CHECK(Access::inventory(app).back() == "hud_potion");
    checkInteractionHud(app, GameMode::Shop);
    MOTOR_CHECK(Access::tienda(app).foco(2) == 1);  // TIENDA-02: el foco es del modelo
    session.closeInteraction();
    checkInteractionHud(app, GameMode::Exploration);

    MOTOR_CHECK(session.tryMovePlayer(1, 0));
    MOTOR_CHECK(session.tryMovePlayer(0, 1));
    MOTOR_CHECK(session.tryMovePlayer(0, 1));
    MOTOR_CHECK(session.interact());
    checkInteractionHud(app, GameMode::Business);
    MOTOR_CHECK(!session.tryMovePlayer(-1, 0));
    Access::businessMenu(app).setSelectedIndex(1);
    checkInteractionHud(app, GameMode::Business);
    MOTOR_CHECK(Access::businessMenu(app).selectedIndex() == 1);
    Access::confirmBusiness(app); // SALIR, no debe comprar por perder la selección.
    MOTOR_CHECK(!session.ownsBusiness("hud_business"));
    checkInteractionHud(app, GameMode::Exploration);

    MOTOR_CHECK(session.tryMovePlayer(-1, 0));
    MOTOR_CHECK(session.tryMovePlayer(-1, 0));
    MOTOR_CHECK(!session.tryMovePlayer(-1, 0)); // El enemigo abre el combate.
    checkInteractionHud(app, GameMode::Battle);
    MOTOR_CHECK(!session.tryMovePlayer(1, 0));
    MOTOR_CHECK(session.battle() != nullptr);
    Access::battleBar(app).mueve(Game::vistaDeBatalla(*session.battle()), 4);  // se queda en HUIR
    checkInteractionHud(app, GameMode::Battle);
    MOTOR_CHECK(Access::battleBar(app).foco() == 2);
    Access::confirmBattle(app, 2); // HUIR devuelve a exploración y cierra el log.
    MOTOR_CHECK(session.enemies().size() == 1);
    checkInteractionHud(app, GameMode::Exploration);
}

MOTOR_TEST_CASE(title_freezes_world_and_campaign_waits_until_frame_boundary) {
    Fixture fixture;
    Application app;
    init(app, fixture.level("start.json", baseMap()), fixture.catalog(), true);
    Access::timer(app);
    auto* session = &Access::session(app);
    auto* scene = Access::scenes(app).activeScene();
    const auto remaining = Access::narrative(app).timers().front().restante;
    Access::update(app, 5.0f);
    MOTOR_CHECK(Access::narrative(app).timers().front().restante == remaining);
    Access::sinCreacion(app);
    Access::confirm(app);
    MOTOR_CHECK(Access::title(app));
    MOTOR_CHECK(Access::scenes(app).activeScene() == scene);
    MOTOR_CHECK(Access::scenes(app).hasPendingTransition());
    Access::update(app, 5.0f);
    MOTOR_CHECK(Access::narrative(app).timers().front().restante == remaining);
    Access::boundary(app);
    MOTOR_CHECK(!Access::title(app));
    MOTOR_CHECK(&Access::session(app) == session);
    Access::update(app, 1.0f);
    MOTOR_CHECK(Access::narrative(app).timers().front().restante < remaining);
}

MOTOR_TEST_CASE(door_commits_at_boundary_and_preserves_player_state_and_minimap) {
    Fixture fixture;
    const auto destination = fixture.level("destination.json", baseMap());
    Application app;
    init(app, fixture.level("start.json", baseMap(), destination), fixture.catalog());
    PlayerCarryOver carry;
    carry.gold = 137; carry.moralityValue = 17; carry.stepsSinceRent = 3;
    carry.businesses.push_back({"c16_business", "Tienda", 25, 120});
    Access::session(app).applyCarryOver(carry);
    Access::player(app).takeDamage(7);
    Access::inventory(app).push_back("c16_token");
    Access::narrative(app).setFlag("c16_persistent");
    const int health = Access::player(app).health();
    auto* original = &Access::session(app);
    enterDoor(app);
    const auto expected = Access::session(app).captureCarryOver();
    MOTOR_CHECK(&Access::session(app) == original);
    MOTOR_CHECK(Access::level(app) != destination);
    Access::boundary(app);
    MOTOR_CHECK(Access::level(app) == destination);
    MOTOR_CHECK(sameCell(Access::session(app).playerPosition(), {3, 2}));
    MOTOR_CHECK(Access::player(app).health() == health);
    MOTOR_CHECK(Access::inventory(app).back() == "c16_token");
    MOTOR_CHECK(Access::narrative(app).hasFlag("c16_persistent"));
    const auto actual = Access::session(app).captureCarryOver();
    MOTOR_CHECK(actual.gold == expected.gold);
    MOTOR_CHECK(actual.moralityValue == expected.moralityValue);
    MOTOR_CHECK(actual.stepsSinceRent == expected.stepsSinceRent);
    MOTOR_CHECK(actual.businesses.size() == 1);
    MOTOR_CHECK(actual.businesses.front().rentPercent == 120);
    // C-02: el minimapa viejo (HudMinimap) guardaba la sesion y habia que
    // comprobar que se reenganchaba. Su sustituto (PERGAMINO-01,
    // Pergamino::MinimapaTinta) la recibe en cada fotograma y no la guarda.
    MOTOR_CHECK(!Access::scenes(app).hasPendingTransition());
}

void failedDoor(const std::string& failure) {
    Fixture fixture;
    const auto destination = fixture.path("bad.json");
    if (failure == "level") fixture.write("bad.json", "{invalid");
    if (failure == "map") fixture.level("bad.json", fixture.path("missing.tmx"));
    if (failure == "tileset") {
        fixture.write("bad.tmx", R"(<map width="2" height="2" tilewidth="64" tileheight="32"><tileset firstgid="1" name="missing" tilewidth="8" tileheight="8" tilecount="1" columns="1"><image source="missing.png" width="8" height="8"/></tileset><layer name="floor" width="2" height="2"><data encoding="csv">1,1,1,1</data></layer></map>)");
        fixture.level("bad.json", fixture.path("bad.tmx"));
    }
    Application app;
    const auto source = fixture.level("start.json", baseMap(), destination);
    init(app, source, fixture.catalog());
    auto* session = &Access::session(app);
    auto* scene = Access::scenes(app).activeScene();
    const auto tileset = Access::map(app).tilesetImagePath();
    enterDoor(app);
    const auto position = Access::session(app).playerPosition();
    Access::boundary(app);
    MOTOR_CHECK(&Access::session(app) == session);
    MOTOR_CHECK(Access::scenes(app).activeScene() == scene);
    MOTOR_CHECK(Access::level(app) == source);
    MOTOR_CHECK(sameCell(Access::session(app).playerPosition(), position));
    MOTOR_CHECK(Access::map(app).getWidth() == 4);
    MOTOR_CHECK(Access::map(app).getHeight() == 3);
    MOTOR_CHECK(Access::map(app).tilesetImagePath() == tileset);
    MOTOR_CHECK(!Access::session(app).pendingTransition().pending);
    MOTOR_CHECK(!Access::scenes(app).hasPendingTransition());
    const auto logSize = Access::session(app).log().size();
    // Repara el fichero: sin otra interacción NO debe reintentar la petición.
    fixture.level("bad.json", baseMap());
    Access::update(app, 0.01f);
    Access::boundary(app);
    MOTOR_CHECK(Access::level(app) == source);
    MOTOR_CHECK(Access::session(app).log().size() == logSize);
}
MOTOR_TEST_CASE(invalid_level_preserves_scene_without_retry) { failedDoor("level"); }
MOTOR_TEST_CASE(invalid_map_preserves_scene_without_retry) { failedDoor("map"); }
MOTOR_TEST_CASE(invalid_tileset_preserves_scene_without_retry) { failedDoor("tileset"); }


MOTOR_TEST_CASE(failed_project_keeps_campaign_and_can_retry_successfully) {
    Fixture fixture;
    Application app;
    init(app, fixture.level("original.json", baseMap()), fixture.catalog(), true);
    fixture.write("proyectos/index.json", R"({"proyectos":["c16_project"]})");
    fixture.level("levels/entry.json", fixture.path("missing_map.tmx"));
    Editor::Project project;
    project.id = "c16_project";
    project.name = "Proyecto C16";
    project.entry = "entry.json";
    project.levels = {"entry.json"};
    MOTOR_CHECK(ProjectSerializer::saveProjectAtomic(fixture.path("proyectos/c16_project.json"), project).isOk());
    auto index = Editor::ProjectIndex::scan(fixture.root.generic_string());
    MOTOR_CHECK(index.isOk());
    MOTOR_CHECK(index.value().projects().size() == 1);
    const Editor::Project proyecto = index.value().projects().front();  // con su entrada resuelta
    auto* session = &Access::session(app);
    auto* scene = Access::scenes(app).activeScene();
    const auto oldCatalog = Access::catalog(app);
    const auto oldScript = Access::script(app);
    const auto ready = Access::narrativeReady(app);
    const auto campaign = Access::campaign(app);
    Access::narrative(app).setFlag("c16_previous_story");
    MOTOR_CHECK(Access::openProject(app, proyecto));
    MOTOR_CHECK(Access::scenes(app).hasPendingTransition());
    Access::boundary(app);
    MOTOR_CHECK(Access::title(app));
    MOTOR_CHECK(Access::scenes(app).activeScene() == scene);
    MOTOR_CHECK(&Access::session(app) == session);
    MOTOR_CHECK(Access::catalog(app) == oldCatalog);
    MOTOR_CHECK(Access::script(app) == oldScript);
    MOTOR_CHECK(Access::narrativeReady(app) == ready);
    MOTOR_CHECK(Access::campaign(app) == campaign);
    MOTOR_CHECK(Access::narrative(app).hasFlag("c16_previous_story"));
    Access::draw(app);
    // El mismo proyecto reparado puede abrirse: el fallo no deja una petición viva.
    fixture.level("levels/entry.json", baseMap());
    MOTOR_CHECK(Access::openProject(app, proyecto));
    Access::boundary(app);
    MOTOR_CHECK(!Access::title(app));
    MOTOR_CHECK(Access::level(app) == fixture.path("levels/entry.json"));
    MOTOR_CHECK(!Access::narrativeReady(app));
    MOTOR_CHECK(!Access::campaign(app));
    Access::draw(app);
}

MOTOR_TEST_CASE(save_restore_is_deferred_and_rejects_incompatible_state_before_commit) {
    Fixture fixture;
    Application app;
    const auto destination = fixture.level("other.json", baseMap());
    const auto original = fixture.level("original.json", baseMap(), destination);
    init(app, original, fixture.catalog());
    Access::narrative(app).setFlag("c16_saved");
    MOTOR_CHECK(Access::quickSave(app));
    enterDoor(app);
    Access::boundary(app);
    MOTOR_CHECK(Access::level(app) == destination);
    Access::narrative(app).setFlag("c16_later");
    MOTOR_CHECK(Access::quickLoad(app));
    MOTOR_CHECK(Access::level(app) == destination);
    Access::boundary(app);
    MOTOR_CHECK(Access::level(app) == original);
    MOTOR_CHECK(Access::narrative(app).hasFlag("c16_saved"));
    MOTOR_CHECK(!Access::narrative(app).hasFlag("c16_later"));
    MOTOR_CHECK(Access::scenes(app).activeMapId().value() == "test_map");
    Access::draw(app);
    // Elimina la puerta del nivel después de guardar: el guardado ya no encaja.
    fixture.level("original.json", baseMap());
    auto* session = &Access::session(app);
    Access::narrative(app).setFlag("c16_keep_after_failure");
    MOTOR_CHECK(Access::quickLoad(app));
    Access::boundary(app);
    MOTOR_CHECK(&Access::session(app) == session);
    MOTOR_CHECK(Access::narrative(app).hasFlag("c16_keep_after_failure"));
    MOTOR_CHECK(!Access::scenes(app).hasPendingTransition());
}

MOTOR_TEST_CASE(campaign_level_change_waits_for_scene_boundary) {
    Fixture fixture;
    Application app;
    init(app, fixture.level("original.json", baseMap()), fixture.catalog());
    MOTOR_CHECK(Access::campaign(app));
    Access::narrative(app).setFlag("bnd_prologo_terminado");
    const auto previous = Access::level(app);
    Access::update(app, 0.01f);
    MOTOR_CHECK(Access::level(app) == previous);
    MOTOR_CHECK(!Access::narrative(app).hasFlag("bnd_dia1_empezado"));
    MOTOR_CHECK(Access::scenes(app).hasPendingTransition());
    Access::boundary(app);
    MOTOR_CHECK(Access::level(app) == "assets/levels/interior_taberna.json");
    MOTOR_CHECK(Access::narrative(app).hasFlag("bnd_dia1_empezado"));
    Access::draw(app);
}


MOTOR_TEST_CASE(standalone_adventure_enters_level_without_campaign_transition) {
    Fixture fixture;
    Application app;
    const auto original = fixture.level("standalone.json", baseMap());
    fixture.write("adventure.json", "{\"id\":\"c16_story\",\"name\":\"Historia\",\"beats\":[{\"id\":\"intro\",\"trigger\":{\"type\":\"enter\",\"target\":\"" + original + "\"},\"lines\":[\"Comienza la historia.\"],\"effects\":[{\"type\":\"setFlag\",\"arg\":\"c16_intro\"}]}]}");
    init(app, original, fixture.catalog(), false, fixture.path("adventure.json"));
    MOTOR_CHECK(Access::narrativeReady(app));
    MOTOR_CHECK(!Access::campaign(app));
    MOTOR_CHECK(Access::narrative(app).hasFlag("c16_intro"));
    MOTOR_CHECK(Access::session(app).mode() == GameMode::Dialogue);
    Access::narrative(app).setFlag("bnd_prologo_terminado");
    Access::update(app, 0.01f);
    Access::boundary(app);
    MOTOR_CHECK(Access::level(app) == original);
    MOTOR_CHECK(!Access::narrative(app).hasFlag("bnd_dia1_empezado"));
    Access::draw(app);
}

MOTOR_TEST_CASE(shutdown_cancels_pending_scene_transition) {
    Fixture fixture;
    Application app;
    init(app, fixture.level("start.json", baseMap()), fixture.catalog(), true);
    Access::sinCreacion(app);
    Access::confirm(app);
    MOTOR_CHECK(Access::scenes(app).hasPendingTransition());
    app.shutdown();
    MOTOR_CHECK(!Access::scenes(app).hasPendingTransition());
    MOTOR_CHECK(Access::scenes(app).activeScene() == nullptr);
    app.shutdown();
}

} // namespace
int main() { return motor_test::runAllTests(); }
