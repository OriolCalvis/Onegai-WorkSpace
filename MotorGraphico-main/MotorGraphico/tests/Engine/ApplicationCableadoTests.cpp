// opencode 19/09 — Cableado de Application «que el jugador vea lo que ya
// existe» (Pendiente 34, ALCANCE_REAL §7). Comprueba las dos piezas toca
// motor y no contenido:
//   - G-12: el catalogo de equipo (assets/catalogs/equipment.json) se carga
//     y se pasa a la sesion via setInventoryRules; sin el, equipar responde
//     "no hay catalogo de equipo cargado" y los objetos quedan inertes.
//   - GDB-12.5: ESC en exploracion entra en pausa (requestPause) y
//     Reanudar/Cancelar la cierra (requestResume), ambos delegando en
//     GameSession. G-10 era inalcanzable: nadie llamaba a pause().
// Etiqueta gl (ventana real), igual que C-24. Solo llama a APIs commitadas.

#include "Engine/Application.h"
#include "Engine/Input/InputMap.h"
#include "Engine/Locale.h"
#include "Game/OptionsLocaleBridge.h"
#include "Game/GameSession.h"
#include "Game/SaveGame.h"
#include "Game/SaveSlotState.h"
#include "Level/ObjectCatalog.h"
#include "TestHarness.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

// Mismo patron de accesos que ApplicationTests (Application.h declara
// friend struct ApplicationTestAccess en el namespace global).
struct ApplicationTestAccess {
    static GameSession& session(Application& a) { return *a.m_session; }
    static void pauseRequest(Application& a) { a.requestPause(); }
    static void resumeRequest(Application& a) { a.requestResume(); }
    static void confirmPause(Application& a) { a.confirmPauseCommand(); }
    static bool hasPauseMenu(Application& a) { return a.m_pauseMenu != nullptr; }
    // H0-05 — accesos al servicio de ranuras cableado en Application.
    static Game::SaveSlots& saveSlots(Application& a) { return *a.m_saveSlots; }
    static void setProjectId(Application& a, const std::string& id) {
        a.m_projectId = id;
        a.setupSaveSlots();
    }
    static bool quickSave(Application& a) { return a.quickSave(); }
    static bool quickLoad(Application& a) { return a.quickLoad(); }
    static bool loadSlot(Application& a, int index) { return a.loadSlot(index); }
    static int newestLoadableSlot(Application& a) { return a.newestLoadableSlot(); }
    static const std::string& levelPath(Application& a) { return a.m_currentLevelPath; }
    // H0-07 — accesos al titulo desde manifiesto y a su cableado.
    static const Game::TitleMenuState& title(Application& a) { return *a.m_title; }
    static void chooseTitle(Application& a) { a.confirmTitleSelection(); }
    static bool pendingSceneReplace(Application& a) { return a.m_scenes.hasPendingTransition(); }
    static bool creandoPersonaje(Application& a) {
        return a.m_nuevaPartida != nullptr && a.m_nuevaPartida->activa();
    }
    static const std::string& projectId(Application& a) { return a.m_projectId; }
    // H0-01 — acceso al catalogo de objetos cableado en Application.
    static const ObjectCatalog& objectCatalog(Application& a) {
        return a.m_objectCatalog;
    }
    // H1-03 — acceso al submenu de opciones cableado en Application.
    static bool hasOptions(Application& a) { return a.m_options != nullptr; }
    static const Game::OptionsState& options(Application& a) { return *a.m_options; }
    static Game::OptionsState& optionState(Application& a) { return *a.m_options; }
    static void pauseNavigateDown(Application& a) { a.m_pauseMenu->moveDown(); }
    static void confirmOptions(Application& a) { a.confirmOptionsCommand(); }
    // H1-05 — acceso al Locale cableado en Application (null si sin tablas).
    static const Engine::Locale* locale(Application& a) { return a.m_locale.get(); }
    // H1-05 — comando de ajuste Izq/Der cableado (el mismo que usa el input).
    static void adjustOptionsRight(Application& a) { a.adjustOptionsRight(); }
    static void adjustOptionsLeft(Application& a) { a.adjustOptionsLeft(); }
};

namespace {

std::string writeObjectCatalog(const std::string& objectId, const std::string& name) {
    const std::string content = R"({"objects": [{"id": ")" + objectId + R"(", "name": ")" +
                                 name + R"(", "category": "pickup"}]})";
    const std::string path =
        "cableado_test_objects_" + std::to_string(objectId.size()) + ".json";
    std::ofstream out(path);
    out << content;
    out.close();
    return path;
}

}  // namespace

// H1-05 — invariante vivo del cableado: tras cada ajuste Der/Izq en la fila
// IDIOMA, el Locale tiene que hablar el codigo del idioma que el menu muestra.
#define APPLICATION_CABLEADO_CHECK_IDIOMA(APP, NAV, LOC)                          \
    do {                                                                          \
        const std::string esperado =                                              \
            Game::OptionsLocaleBridge::localeCode((NAV).language());              \
        MOTOR_CHECK(!esperado.empty());                                           \
        if (!esperado.empty()) {                                                  \
            MOTOR_CHECK((LOC)->language() == esperado);                           \
        }                                                                         \
    } while (0)

// ========================================================================
// G-12 — el catalogo de equipo viaja hasta la sesion
// ========================================================================
MOTOR_TEST_CASE(cableado_eq_catalogo_pasado_a_sesion) {
    Application app;
    auto init =
        app.init(320, 240, "cableado: equipo", "assets/levels/test_level.json",
                 "assets/objects/test_objects.json");
    MOTOR_CHECK(init.isOk());

    auto& session = ApplicationTestAccess::session(app);
    // Without the wiring, inventoryRules().equipment queda nulo y todos los
    // objetos de equipo quedan inertes ("no hay catalogo de equipo cargado").
    MOTOR_CHECK(session.inventoryRules().equipment != nullptr);
}

// ========================================================================
// G-12 — con contenido real, una pieza de equipo se resuelve de verdad
// ========================================================================
MOTOR_TEST_CASE(cableado_eq_pieza_real_resuelve) {
    // item_aguja_del_firmamento -> "aguja_del_firmamento" (el puente quita
    // el prefijo item_) y esa pieza existe en equipment.json.
    const std::string extra = writeObjectCatalog("item_aguja_del_firmamento", "Aguja");
    Application app;
    auto init = app.init(320, 240, "cableado: pieza real", "assets/levels/test_level.json",
                         "assets/objects/test_objects.json", "",
                         std::vector<std::string>{extra});
    MOTOR_CHECK(init.isOk());

    auto& session = ApplicationTestAccess::session(app);
    // La pieza existe en el catalogo de equipo real (cargado por Application).
    MOTOR_CHECK(session.equipmentFor("item_aguja_del_firmamento") != nullptr);
    // Y con el catalogo en la sesion, el por-que-no-puedo-equipar deja de
    // ser el "sistema sin catalogo": empieza a hablar de reglas.
    MOTOR_CHECK(session.whyCannotEquip("item_aguja_del_firmamento") !=
                "no hay catalogo de equipo cargado");
    std::remove(extra.c_str());
}

// ========================================================================
// GDB-12.5 — ESC pausa; Reanudar (y Cancelar) reanudan
// ========================================================================
MOTOR_TEST_CASE(cableado_escape_pausa_y_reanudar) {
    // Flujo real del juego: con titulo (./juego sin --nivel) el menu de
    // pausa existe y Reanudar lo cierra. El titulo deja el nivel cargado
    // detras (Application::init lo crea primero).
    Application app;
    auto init = app.init(320, 240, "cableado: pausa", "assets/levels/test_level.json",
                         "assets/objects/test_objects.json", "", {}, /*startAtTitle=*/true);
    MOTOR_CHECK(init.isOk());
    MOTOR_CHECK(ApplicationTestAccess::hasPauseMenu(app));  // flujo real

    auto& session = ApplicationTestAccess::session(app);
    MOTOR_CHECK(session.mode() == GameMode::Exploration);

    // ESC -> requestPause(): la sesion congela (GameMode::Pause).
    ApplicationTestAccess::pauseRequest(app);
    MOTOR_CHECK(session.mode() == GameMode::Pause);

    // El menu de pausa arranca en "Reprender" (indice 0) y confirmar lo
    // ejecuta: cierra con requestResume() (antes closeInteraction(), un
    // no-op para Pause, dejando el juego clavado).
    ApplicationTestAccess::confirmPause(app);
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
}

MOTOR_TEST_CASE(cableado_cancelar_reanuda) {
    Application app;
    auto init =
        app.init(320, 240, "cableado: cancelar", "assets/levels/test_level.json",
                 "assets/objects/test_objects.json");
    MOTOR_CHECK(init.isOk());

    auto& session = ApplicationTestAccess::session(app);

    // El camino de Cancelar en el menu de pausa (ESC/E/TAB) es el mismo:
    // requestResume(), que es no-op salvo en Pause (igual que pause()).
    ApplicationTestAccess::pauseRequest(app);
    MOTOR_CHECK(session.mode() == GameMode::Pause);
    ApplicationTestAccess::resumeRequest(app);
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
}

// ========================================================================
// H1-03 — «Opcions» del PauseMenu abre el submenu GL-free y se puede volver
// ========================================================================
MOTOR_TEST_CASE(cableado_opciones_se_abre_y_vuelve) {
    Application app;
    auto init = app.init(320, 240, "cableado: opciones", "assets/levels/test_level.json",
                         "assets/objects/test_objects.json", "", {},
                         /*startAtTitle=*/true);
    MOTOR_CHECK(init.isOk());
    MOTOR_CHECK(ApplicationTestAccess::hasPauseMenu(app));

    auto& session = ApplicationTestAccess::session(app);
    ApplicationTestAccess::pauseRequest(app);
    MOTOR_CHECK(session.mode() == GameMode::Pause);

    // «Opcions» es la fila 5 del PauseMenu (despues de Desar y Carregar);
    // todavia sin cablear, confirmarla no tocaba nada y no habia vuelta.
    for (int i = 0; i < 5; ++i) {
        ApplicationTestAccess::pauseNavigateDown(app);
    }
    ApplicationTestAccess::confirmPause(app);

    // La pausa sigue viva y el submenu existe, con sus 6 filas por defecto.
    MOTOR_CHECK(session.mode() == GameMode::Pause);
    MOTOR_CHECK(ApplicationTestAccess::hasOptions(app));
    const auto& opts = ApplicationTestAccess::options(app);
    MOTOR_CHECK(opts.entries().size() == 6);
    MOTOR_CHECK(opts.selectedEntry().key == "music");
    MOTOR_CHECK(opts.volume("music") == 8);

    // Desde el submenu, Cancelar (ESC) vuelve al menu de pausa sin salir.
    // Confirmar con el cursor en la fila 5 (VOLVER) hace lo mismo: Back.
    auto& nav = ApplicationTestAccess::optionState(app);
    for (int i = 0; i < 5; ++i) {
        nav.moveDown();
    }
    MOTOR_CHECK(nav.selectedEntry().key == "back");
    ApplicationTestAccess::confirmOptions(app);
    MOTOR_CHECK(!ApplicationTestAccess::hasOptions(app));
    MOTOR_CHECK(session.mode() == GameMode::Pause);

    // Y la pausa sigue cerrable: Reanudar vuelve a la exploracion.
    ApplicationTestAccess::resumeRequest(app);
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
}

// ========================================================================
// H1-05 — ajustar la fila IDIOMA aplica el idioma al Locale del motor
// ========================================================================
MOTOR_TEST_CASE(cableado_idioma_se_aplica_a_locale) {
    Application app;
    auto init = app.init(320, 240, "cableado: idioma", "assets/levels/test_level.json",
                         "assets/objects/test_objects.json", "", {},
                         /*startAtTitle=*/true);
    MOTOR_CHECK(init.isOk());

    // El Locale esta cableado y cargado desde assets/i18n (el test corre
    // desde build/, donde copiar_assets copia las tablas).
    const Engine::Locale* loc = ApplicationTestAccess::locale(app);
    MOTOR_CHECK(loc != nullptr);
    MOTOR_CHECK(loc->loaded());
    // El idioma activo no se sincroniza con la fila del menu al arrancar:
    // es el de referencia del manifiesto. Lo que cableado_idioma_* debe
    // garantizar es que CADA ajuste Der/Izq en la fila IDIOMA lleva el
    // Locale al codigo del idioma que el menu muestra ahora.

    // Pausa y abre Opciones (fila 5 del PauseMenu), como en H1-03.
    auto& session = ApplicationTestAccess::session(app);
    ApplicationTestAccess::pauseRequest(app);
    for (int i = 0; i < 5; ++i) {
        ApplicationTestAccess::pauseNavigateDown(app);
    }
    ApplicationTestAccess::confirmPause(app);
    MOTOR_CHECK(ApplicationTestAccess::hasOptions(app));

    // Navega hasta la fila IDIOMA (indice 3: music, sfx, ambience, language).
    auto& nav = ApplicationTestAccess::optionState(app);
    for (int i = 0; i < 3; ++i) {
        nav.moveDown();
    }
    MOTOR_CHECK(nav.selectedEntry().key == "language");

    // Der recorre todos los idiomas del ciclo; tras CADA ajuste el Locale
    // tiene que hablar el idioma que el menu esta mostrando. El estado
    // inicial no se comprueba: al abrir, el idioma activo es el de
    // referencia y no se ha sincronizado con la fila; el cableado promete
    // aplicar el idioma cuando se ajusta, no al arrancar.
    const int ciclos = static_cast<int>(nav.languages().size());
    for (int i = 0; i < ciclos; ++i) {
        ApplicationTestAccess::adjustOptionsRight(app);
        APPLICATION_CABLEADO_CHECK_IDIOMA(app, nav, loc);
    }

    // Izq da la vuelta entera y cierra el ciclo en CATALA (indice 0) de
    // nuevo. El Locale sigue el menu en cada paso.
    for (int i = 0; i < ciclos; ++i) {
        ApplicationTestAccess::adjustOptionsLeft(app);
        APPLICATION_CABLEADO_CHECK_IDIOMA(app, nav, loc);
    }
}

// ========================================================================
// H0-05 — F5 (quickSave) va a la ranura rapida de Game::SaveSlots
// ========================================================================
MOTOR_TEST_CASE(cableado_f5_usa_ranura_rapida) {
    Application app;
    auto init =
        app.init(320, 240, "cableado: ranura rapida", "assets/levels/test_level.json",
                 "assets/objects/test_objects.json");
    MOTOR_CHECK(init.isOk());

    auto& session = ApplicationTestAccess::session(app);
    auto& slots = ApplicationTestAccess::saveSlots(app);
    const int baseline = session.gold();
    session.addGold(17);
    MOTOR_CHECK(ApplicationTestAccess::quickSave(app));

    // El guardado NO es el viejo savegame.json suelto: vive en la ranura
    // rapida (kQuickSlot) y el servicio ya lo ve (fue quien lo escribio).
    MOTOR_CHECK(slots.quick().occupied());
    MOTOR_CHECK(slots.quick().path.find("saves/rapido.json") != std::string::npos);
    MOTOR_CHECK(slots.quick().gold == baseline + 17);

    // Cargar partida rapida encola una carga valida (el tramo de restore
    // es el mismo de loadGame, ya cubierto por C-24).
    MOTOR_CHECK(ApplicationTestAccess::quickLoad(app));

    std::filesystem::remove("saves/rapido.json");
}

// ========================================================================
// H0-05 — «Cargar partida» elige la ranura enumerada mas reciente
// ========================================================================
MOTOR_TEST_CASE(cableado_cargar_partida_elige_ranura_enumerada) {
    Application app;
    auto init =
        app.init(320, 240, "cableado: cargar partida", "assets/levels/test_level.json",
                 "assets/objects/test_objects.json");
    MOTOR_CHECK(init.isOk());

    auto& slots = ApplicationTestAccess::saveSlots(app);
    auto& session = ApplicationTestAccess::session(app);
    session.addGold(23);
    const SaveGameData data =
        session.captureSaveGame(ApplicationTestAccess::levelPath(app));
    auto r = slots.save(3, data);
    MOTOR_CHECK(r.isOk());

    // La ranura 3 es la unica ocupada: «Cargar partida» la detecta como la
    // mas reciente y encola su carga.
    MOTOR_CHECK(ApplicationTestAccess::newestLoadableSlot(app) == 3);
    MOTOR_CHECK(ApplicationTestAccess::loadSlot(app, 3));

    slots.remove(3);
}

// ========================================================================
// H0-05 — ranuras separadas por proyecto (saves/<proyecto>/)
// ========================================================================
MOTOR_TEST_CASE(cableado_ranuras_separadas_por_proyecto) {
    Application app;
    auto init =
        app.init(320, 240, "cableado: proyectos", "assets/levels/test_level.json",
                 "assets/objects/test_objects.json");
    MOTOR_CHECK(init.isOk());
    auto& session = ApplicationTestAccess::session(app);
    session.addGold(31);

    // Proyecto A: rapid guarda en saves/h005-proy-a/rapido.json.
    ApplicationTestAccess::setProjectId(app, "h005-proy-a");
    MOTOR_CHECK(ApplicationTestAccess::quickSave(app));
    MOTOR_CHECK(std::filesystem::exists("saves/h005-proy-a/rapido.json"));

    // Proyecto B: su ranura va a otra carpeta y NO pisa la de A.
    ApplicationTestAccess::setProjectId(app, "h005-proy-b");
    MOTOR_CHECK(ApplicationTestAccess::quickSave(app));
    MOTOR_CHECK(std::filesystem::exists("saves/h005-proy-b/rapido.json"));
    MOTOR_CHECK(std::filesystem::exists("saves/h005-proy-a/rapido.json"));

    std::filesystem::remove_all("saves/h005-proy-a");
    std::filesystem::remove_all("saves/h005-proy-b");
}

// ========================================================================
// H0-07 — el titulo sale del manifiesto (TitleMenuState), no del indice
// ========================================================================
MOTOR_TEST_CASE(cableado_titulo_sale_del_manifiesto) {
    // «Continuar» depende de que la campana NO tenga partidas: limpiar la
    // carpeta de la campana antes de arrancar hace el caso deterministico.
    std::filesystem::remove_all("saves/boundington");

    Application app;
    auto init = app.init(320, 240, "cableado: titulo", "assets/levels/test_level.json",
                         "assets/objects/test_objects.json", "", {}, /*startAtTitle=*/true);
    MOTOR_CHECK(init.isOk());

    const auto& title = ApplicationTestAccess::title(app);
    // El manifiesto del primer proyecto del indice (boundington.json) se lee
    // de verdad: este es el titulo del JUEGO, no un rotulo del motor.
    MOTOR_CHECK(title.ok());
    MOTOR_CHECK(title.title() == "Los Perdidos de Boundington");
    MOTOR_CHECK(!title.entryPath().empty());
    MOTOR_CHECK(title.continueSlot() == -1);  // sin partidas de la campana

    // Menu completo: Continuar (apagado y CON su motivo), Nueva partida,
    // Opciones, Salir. Las apagadas no cierran huecos: se senalan y se leen.
    MOTOR_CHECK(title.items().size() == 4);
    MOTOR_CHECK(title.items()[0].command == Game::TitleCommand::Continue);
    MOTOR_CHECK(!title.items()[0].enabled);
    MOTOR_CHECK(!title.items()[0].reason.empty());

    // El cursor salta la entrada apagada: arranca en Nueva partida.
    MOTOR_CHECK(title.selected() == 1);
    MOTOR_CHECK(title.selectedItem().command == Game::TitleCommand::NewGame);

    // Y las ranuras de la campana apuntan a su proyecto (saves/boundington/),
    // el mismo directorio que escanea el titulo para «Continuar».
    MOTOR_CHECK(ApplicationTestAccess::projectId(app) == "boundington");

    // Cableado: confirmar Nueva partida abre la creacion de personaje
    // (NUEVA-PARTIDA-01, con el paso Apariencia de APARIENCIA-01). La
    // escena cambia al nivel ya cargado detras cuando la creacion termina.
    MOTOR_CHECK(!ApplicationTestAccess::pendingSceneReplace(app));
    ApplicationTestAccess::chooseTitle(app);
    MOTOR_CHECK(ApplicationTestAccess::creandoPersonaje(app));
    MOTOR_CHECK(!ApplicationTestAccess::pendingSceneReplace(app));
}

// ========================================================================
// H0-01 — la campana carga los catalogos del manifiesto, no solo el
// catalogo suelto que pasa juego.cpp (boundington_npcs.json). Con
// startAtTitle el indice se escanea ANTES de cargar el nivel y se funden en
// m_objectCatalog los ficheros que el manifest declara (los 15 de
// Boundington). En el flujo alterno (--nivel) esa fusion no ocurre.
// ========================================================================
MOTOR_TEST_CASE(cableado_campana_catalogo_desde_manifiesto) {
    Application app;
    auto init = app.init(320, 240, "cableado: manifiesto", "assets/levels/test_level.json",
                         "assets/objects/test_objects.json", "", {},
                         /*startAtTitle=*/true);
    MOTOR_CHECK(init.isOk());

    const auto& catalog = ApplicationTestAccess::objectCatalog(app);
    // IDs que SOLO declaran los catalogos del manifiesto (ciudad_objetos y
    // este_norte_objetos); ausentes de boundington_npcs.json y de los JSON
    // de test. Verificados contra los ficheros reales.
    MOTOR_CHECK(catalog.find("agua_bendita") != nullptr);
    MOTOR_CHECK(catalog.find("cerveza") != nullptr);

    // Sin titulo el catalogo es solo el que pasas: la fusion es cosa de la
    // ruta de campana (este es el hueco del 391 referencias inertes).
    Application direct;
    auto initDirect =
        direct.init(320, 240, "cableado: manifiesto", "assets/levels/test_level.json",
                    "assets/objects/test_objects.json");
    MOTOR_CHECK(initDirect.isOk());
    MOTOR_CHECK(ApplicationTestAccess::objectCatalog(direct).find("agua_bendita") == nullptr);
    MOTOR_CHECK(ApplicationTestAccess::objectCatalog(direct).find("cerveza") == nullptr);
}

int main() { return motor_test::runAllTests(); }
