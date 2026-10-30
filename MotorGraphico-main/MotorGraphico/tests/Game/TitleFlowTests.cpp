#include "Game/TitleMenuState.h"

#include "TestHarness.h"

#include "Game/SaveSlotState.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// G-08 — Título y arranque desde el manifiesto del proyecto.
//
// La prueba del plan (test_title_flow): dos proyectos con nombres y
// entradas distintos; un manifiesto inválido no cae en Boundington. Más
// lo que el criterio pide de «Continuar»: apagado con su motivo si no hay
// guardado válido, y apuntando al bueno si lo hay.

namespace {

namespace fs = std::filesystem;
using Game::TitleCommand;
using Game::TitleMenuState;

// Carpeta temporal CON ESPACIOS: la de un juego instalado en
// «Mis juegos» también los tiene.
struct Juego {
    fs::path base;
    Juego() {
        base = fs::temp_directory_path() /
               ("motor titulo " +
                std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(base / "assets/proyectos");
        fs::create_directories(base / "assets/levels");
    }
    ~Juego() {
        std::error_code ec;
        fs::remove_all(base, ec);
    }
    std::string manifiesto(const std::string& id) const {
        return (base / "assets/proyectos" / (id + ".json")).string();
    }
    std::string saves() const { return (base / "guardados").string(); }

    void escribir(const fs::path& ruta, const std::string& texto) const {
        fs::create_directories(ruta.parent_path());
        std::ofstream f(ruta, std::ios::binary);
        f << texto;
    }
    void proyecto(const std::string& id, const std::string& nombre, const std::string& nivel,
                  const std::string& titulo = "", bool crearNivel = true) const {
        std::string ajustes =
            R"("schemaVersion": 1, "mapsDirectory": "maps", "levelsDirectory": "levels",
               "adventuresDirectory": "adventures", "catalogsDirectory": "objects",
               "initialMap": "")";
        if (!titulo.empty()) ajustes += R"(, "title": ")" + titulo + "\"";
        escribir(manifiesto(id),
                 R"({"schemaVersion": 1, "id": ")" + id + R"(", "name": ")" + nombre +
                     R"(", "description": "Un proyecto de prueba", "epoch": "", "author": "",
                     "prefix": "", "settings": {)" + ajustes + R"(},
                     "entry": )" + (nivel.empty() ? std::string("\"\"") : "\"assets/levels/" + nivel + "\"") +
                     R"(, "levels": [)" + (nivel.empty() ? std::string() : "\"" + nivel + "\"") +
                     R"(], "maps": [], "adventures": [], "catalogs": []})");
        if (crearNivel && !nivel.empty()) {
            escribir(base / "assets/levels" / nivel, "{}");
        }
    }
};

SaveGameData partida(const std::string& quien, const std::string& cuando) {
    SaveGameData d;
    d.levelPath = "levels/x.json";
    d.characterName = quien;
    d.playerPosition = GridCoord{1, 1};
    d.playerHealth = 5;
    d.playerMaxHealth = 10;
    d.savedAt = cuando;
    return d;
}

std::vector<TitleCommand> comandos(const TitleMenuState& t) {
    std::vector<TitleCommand> v;
    for (const auto& i : t.items()) v.push_back(i.command);
    return v;
}

const Game::TitleMenuItem& item(const TitleMenuState& t, TitleCommand c) {
    for (const auto& i : t.items()) {
        if (i.command == c) return i;
    }
    throw std::runtime_error("no hay esa entrada");
}

bool contiene(const std::string& a, const std::string& b) { return a.find(b) != std::string::npos; }

}  // namespace

MOTOR_TEST_CASE(dos_proyectos_con_nombres_y_entradas_distintos) {
    Juego j;
    j.proyecto("la_llave", "La llave del faro", "faro_entrada.json");
    j.proyecto("cripta", "Cripta de Olm", "cripta_puerta.json", "Las Criptas de Olm");
    // Boundington también está instalado al lado; no debe colarse en nada.
    j.proyecto("boundington", "Boundington", "ciudad_sur.json");

    const auto a = TitleMenuState::fromManifest(j.manifiesto("la_llave"), j.base.string(), j.saves());
    const auto b = TitleMenuState::fromManifest(j.manifiesto("cripta"), j.base.string(), j.saves());
    MOTOR_CHECK(a.ok() && b.ok());
    MOTOR_CHECK(a.project().id == "la_llave");
    MOTOR_CHECK(b.project().id == "cripta");
    MOTOR_CHECK(a.title() == "La llave del faro");
    MOTOR_CHECK(b.title() == "Las Criptas de Olm");  // el título de E-11 manda sobre el nombre
    MOTOR_CHECK(a.subtitle() == "Un proyecto de prueba");
    MOTOR_CHECK(fs::path(a.entryPath()).filename() == "faro_entrada.json");
    MOTOR_CHECK(fs::path(b.entryPath()).filename() == "cripta_puerta.json");
    MOTOR_CHECK(fs::exists(a.entryPath()));

    const std::vector<TitleCommand> esperado{TitleCommand::Continue, TitleCommand::NewGame,
                                             TitleCommand::Options, TitleCommand::Quit};
    MOTOR_CHECK(comandos(a) == esperado);
    MOTOR_CHECK(item(a, TitleCommand::NewGame).enabled);
    // Sin guardados el cursor empieza en Nueva partida, no en la apagada.
    MOTOR_CHECK(a.selectedItem().command == TitleCommand::NewGame);
    TitleMenuState copia = a;
    MOTOR_CHECK(copia.confirm() == TitleCommand::NewGame);
}

MOTOR_TEST_CASE(un_manifiesto_invalido_no_cae_en_boundington) {
    Juego j;
    j.proyecto("boundington", "Boundington", "ciudad_sur.json");
    j.escribir(j.manifiesto("roto"), "{ esto no es json");
    j.escribir(j.manifiesto("mal_id"),
               R"({"schemaVersion": 1, "id": "Mi Juego", "name": "Mi juego", "settings": {"schemaVersion": 1},
                   "entry": "", "levels": [], "maps": [], "adventures": [], "catalogs": []})");

    const struct {
        std::string ruta;
        const char* pista;
    } casos[] = {
        {j.manifiesto("no_existe"), "no_existe.json"},
        {j.manifiesto("roto"), "roto.json"},
        {j.manifiesto("mal_id"), "mal_id.json"},
        {"", "ningún proyecto"},
    };
    for (const auto& c : casos) {
        TitleMenuState t = TitleMenuState::fromManifest(c.ruta, j.base.string(), j.saves());
        if (t.ok()) MOTOR_CHECK_FAIL("se dio por bueno: " + c.ruta);
        if (!contiene(t.error(), c.pista)) MOTOR_CHECK_FAIL("error sin su pista: " + t.error());
        MOTOR_CHECK(t.project().id.empty());
        MOTOR_CHECK(t.entryPath().empty());
        MOTOR_CHECK(t.continueSlot() == -1);
        MOTOR_CHECK(!contiene(t.title(), "Boundington"));
        MOTOR_CHECK(t.subtitle() == t.error());   // el motivo se enseña, no se esconde
        MOTOR_CHECK(comandos(t) == std::vector<TitleCommand>{TitleCommand::Quit});
        MOTOR_CHECK(t.confirm() == TitleCommand::Quit);
    }
}

MOTOR_TEST_CASE(continuar_apagado_dice_por_que_y_no_se_ejecuta) {
    Juego j;
    j.proyecto("la_llave", "La llave", "entrada.json");

    auto t = TitleMenuState::fromManifest(j.manifiesto("la_llave"), j.base.string(), j.saves());
    const auto& seguir = item(t, TitleCommand::Continue);
    MOTOR_CHECK(!seguir.enabled);
    MOTOR_CHECK(seguir.reason == "No hay ninguna partida guardada.");
    MOTOR_CHECK(t.continueSlot() == -1);
    // Se puede señalar (para leer el motivo), pero no se ejecuta.
    t.moveUp();
    MOTOR_CHECK(t.selectedItem().command == TitleCommand::Continue);
    MOTOR_CHECK(t.confirm() == TitleCommand::None);
    MOTOR_CHECK(t.message() == seguir.reason);
    t.moveDown();
    MOTOR_CHECK(t.message().empty());
    // Las flechas dan la vuelta.
    t.moveUp();
    t.moveUp();
    MOTOR_CHECK(t.selectedItem().command == TitleCommand::Quit);
    t.moveDown();
    MOTOR_CHECK(t.selectedItem().command == TitleCommand::Continue);

    // Un juego sin carpeta de guardados, igual pero con otro motivo.
    const auto sin = TitleMenuState::fromManifest(j.manifiesto("la_llave"), j.base.string(), "");
    MOTOR_CHECK(item(sin, TitleCommand::Continue).reason == "Este juego no guarda partidas.");

    // Los guardados de OTRO proyecto no cuentan.
    Game::SaveSlots otro(j.saves(), "boundington");
    MOTOR_CHECK(otro.save(1, partida("Mira", "2026-09-17 10:00")).isOk());
    const auto ajeno = TitleMenuState::fromManifest(j.manifiesto("la_llave"), j.base.string(), j.saves());
    MOTOR_CHECK(!item(ajeno, TitleCommand::Continue).enabled);
}

MOTOR_TEST_CASE(continuar_apunta_a_la_partida_mas_reciente_del_proyecto) {
    Juego j;
    j.proyecto("la_llave", "La llave", "entrada.json");
    Game::SaveSlots ranuras(j.saves(), "la_llave");
    MOTOR_CHECK(ranuras.save(1, partida("Kael", "2026-09-10 10:00")).isOk());
    MOTOR_CHECK(ranuras.save(3, partida("Kael", "2026-09-15 09:30")).isOk());
    MOTOR_CHECK(ranuras.save(Game::SaveSlots::kQuickSlot, partida("Kael", "2026-09-12 18:00")).isOk());
    // Una rota más nueva no gana: no se puede cargar.
    j.escribir(fs::path(ranuras.pathFor(2)), "{ basura");
    // Y una de otro proyecto, aún más nueva, tampoco.
    Game::SaveSlots otro(j.saves(), "cripta");
    MOTOR_CHECK(otro.save(1, partida("Mira", "2026-09-17 10:00")).isOk());

    auto t = TitleMenuState::fromManifest(j.manifiesto("la_llave"), j.base.string(), j.saves());
    MOTOR_CHECK(item(t, TitleCommand::Continue).enabled);
    MOTOR_CHECK(t.continueSlot() == 3);
    MOTOR_CHECK(t.continueSummary() == "Kael · 2026-09-15 09:30");
    // Con guardado, el cursor empieza en Continuar.
    MOTOR_CHECK(t.selectedItem().command == TitleCommand::Continue);
    MOTOR_CHECK(t.confirm() == TitleCommand::Continue);

    // El guardado rápido, si es el último, es el que se continúa.
    MOTOR_CHECK(ranuras.save(Game::SaveSlots::kQuickSlot, partida("Kael", "2026-09-16 08:00")).isOk());
    const auto u = TitleMenuState::fromManifest(j.manifiesto("la_llave"), j.base.string(), j.saves());
    MOTOR_CHECK(u.continueSlot() == Game::SaveSlots::kQuickSlot);
}

MOTOR_TEST_CASE(solo_guardados_rotos_o_de_otra_version) {
    Juego j;
    j.proyecto("la_llave", "La llave", "entrada.json");
    Game::SaveSlots ranuras(j.saves(), "la_llave");
    j.escribir(fs::path(ranuras.pathFor(1)), "{ basura");
    auto t = TitleMenuState::fromManifest(j.manifiesto("la_llave"), j.base.string(), j.saves());
    MOTOR_CHECK(item(t, TitleCommand::Continue).reason == "La partida guardada está dañada.");

    j.escribir(fs::path(ranuras.pathFor(1)), R"({"schemaVersion": 999})");
    t = TitleMenuState::fromManifest(j.manifiesto("la_llave"), j.base.string(), j.saves());
    MOTOR_CHECK(item(t, TitleCommand::Continue).reason ==
                "La partida guardada es de otra versión del juego.");

    j.escribir(fs::path(ranuras.pathFor(4)), "{ basura");
    t = TitleMenuState::fromManifest(j.manifiesto("la_llave"), j.base.string(), j.saves());
    const auto& seguir = item(t, TitleCommand::Continue);
    MOTOR_CHECK(!seguir.enabled);
    MOTOR_CHECK(contiene(seguir.reason, "Las partidas guardadas no se pueden cargar"));
    MOTOR_CHECK(t.continueSlot() == -1);
    // Los ficheros no se tocan: el titulo solo mira.
    MOTOR_CHECK(fs::exists(ranuras.pathFor(1)) && fs::exists(ranuras.pathFor(4)));
}

MOTOR_TEST_CASE(nueva_partida_apagada_si_falta_la_entrada) {
    Juego j;
    j.proyecto("sin_nivel", "Sin nivel", "no_esta.json", "", /*crearNivel=*/false);
    j.proyecto("vacio", "Vacío", "");
    const auto a = TitleMenuState::fromManifest(j.manifiesto("sin_nivel"), j.base.string(), j.saves());
    MOTOR_CHECK(a.ok());
    MOTOR_CHECK(!item(a, TitleCommand::NewGame).enabled);
    MOTOR_CHECK(contiene(item(a, TitleCommand::NewGame).reason, "assets/levels/no_esta.json"));
    // Ni Continuar ni Nueva: el cursor cae en Opciones.
    MOTOR_CHECK(a.selectedItem().command == TitleCommand::Options);
    const auto b = TitleMenuState::fromManifest(j.manifiesto("vacio"), j.base.string(), j.saves());
    MOTOR_CHECK(item(b, TitleCommand::NewGame).reason == "El proyecto no tiene nivel de entrada.");
    MOTOR_CHECK(b.entryPath().empty());
}

// Con los manifiestos publicados del repositorio: los dos que se juegan
// se abren con su propio nombre.
MOTOR_TEST_CASE(los_proyectos_publicados_abren_con_su_nombre) {
    const auto e = TitleMenuState::fromManifest("assets/proyectos/ejemplos.json", ".", "");
    if (!e.ok()) MOTOR_CHECK_FAIL("ejemplos: " + e.error());
    MOTOR_CHECK(e.title() == "Ejemplos del motor");
    MOTOR_CHECK(item(e, TitleCommand::NewGame).enabled);
    const auto b = TitleMenuState::fromManifest("assets/proyectos/boundington.json", ".", "");
    if (!b.ok()) MOTOR_CHECK_FAIL("boundington: " + b.error());
    MOTOR_CHECK(b.project().id == "boundington");
    MOTOR_CHECK(b.title() != e.title());
}

int main() { return motor_test::runAllTests(); }
