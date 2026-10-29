// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-04  (fase C, 2 j) - Un enemic invocat per narrativa no trenca el guardat
//   C-08  (fase C, 1 j) - Un guardat antic no escriu fora de rang
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// RUN-10 — Guardado de partida: formato, escritura atomica y round-trip
// sobre una GameSession real.
//
// Unit y no demo: sin mapa (TileMap nullptr, "sin mapa nada bloquea") y
// sin assets en disco mas que el fichero temporal que el propio test
// escribe. Asi corre en CI en los tres sistemas sin fixtures ni cwd.

#include "TestHarness.h"

#include "Core/Json/JsonValue.h"
#include "Game/GameSession.h"
#include "Game/SaveGame.h"
#include "Game/Skill.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace {

class TestCombatant : public ICombatant {
public:
    explicit TestCombatant(int maxHealth) : m_health(maxHealth), m_maxHealth(maxHealth) {}

    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::min(m_maxHealth, m_health + amount); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }

    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }

private:
    int m_health;
    int m_maxHealth;
};

// GUARDAR-FICHA — Un cuerpo con ficha, como el Player con la suya.
class CuerpoConFicha : public TestCombatant {
public:
    explicit CuerpoConFicha(RPG::CharacterSheet* ficha) : TestCombatant(30), m_ficha(ficha) {}
    RPG::CharacterSheet* character_sheet() override { return m_ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return m_ficha; }

private:
    RPG::CharacterSheet* m_ficha;
};

ObjectCatalog makeCatalog() {
    ObjectCatalog catalog;
    auto result = catalog.loadFromString(R"({
        "objects": [
            {"id": "pocion", "name": "Pocion", "category": "pickup",
             "pickup": {"effect": "heal", "power": 15}},
            {"id": "antorcha", "name": "Antorcha", "category": "pickup",
             "pickup": {"effect": "heal", "power": 1}},
            {"id": "slime", "name": "Slime", "category": "enemy",
             "combat": {"maxHealth": 10, "maxMana": 0, "skills": []}}
        ]
    })");
    MOTOR_CHECK(result.isOk());
    return catalog;
}

// Nivel sin mapa: dos pickups y un enemigo, todos en fila.
LevelDefinition makeLevel() {
    LevelDefinition level;
    level.name = "Guardado";
    level.playerStart = GridCoord{0, 0};
    level.objects.push_back(
        ObjectSpawn{"pocion", GridCoord{1, 0}, GridCoord{1, 0}, GridCoord{1, 0}});
    level.objects.push_back(
        ObjectSpawn{"antorcha", GridCoord{2, 0}, GridCoord{2, 0}, GridCoord{2, 0}});
    level.objects.push_back(
        ObjectSpawn{"slime", GridCoord{5, 0}, GridCoord{5, 0}, GridCoord{5, 0}});
    return level;
}

// Ruta temporal propia del test: se borra al terminar cada caso que la
// usa. No se toca assets/ ni el directorio de build mas alla de esto.
std::string tempPath(const std::string& name) { return "run10_" + name + ".json"; }

SaveGameData makeFullData() {
    SaveGameData data;
    data.levelPath = "assets/levels/test_level.json";
    data.playerPosition = GridCoord{3, 4};
    data.playerHealth = 42;
    data.playerMaxHealth = 60;
    data.playerMilestones = 3;  // H2-04
    data.playerLevel = 47;      // NIVELES-01
    data.playerLevelProgress = 38;
    data.playerTier = 3;
    data.playerTierMilestones = 5;
    data.inventory = {"pocion", "llave_oxidada"};
    data.gold = 175;
    data.moralityValue = -30;
    data.stepsSinceRent = 7;

    OwnedBusiness posada;
    posada.objectId = "cartel_posada";
    posada.name = "Posada del Cuervo";
    posada.baseIncome = 20;
    posada.rentPercent = 130;
    data.businesses.push_back(posada);

    data.worldObjects.push_back(SavedWorldObject{"antorcha", GridCoord{2, 0}});
    data.enemies.push_back(SavedEnemy{2, "slime", GridCoord{5, 1}, 4});

    data.narrativeFlags = {"prologo_visto", "puerta_abierta"};
    data.narrativeBeatsFired = {"beat_intro"};

    RPG::TimerState timer;
    timer.id = "rescate";
    timer.restante = 12.5f;
    timer.activo = true;
    timer.expirado = false;
    timer.avisosDados = {true, false};
    data.narrativeTimers.push_back(timer);

    return data;
}

}  // namespace

// --- RUN-10.1 / 10.2: el DTO sobrevive al viaje de ida y vuelta ---

MOTOR_TEST_CASE(round_trip_preserves_every_field) {
    const SaveGameData original = makeFullData();

    SaveGameData restored;
    std::string error;
    MOTOR_CHECK(parseSaveGame(serializeSaveGame(original), &restored, &error));
    MOTOR_CHECK(error.empty());

    MOTOR_CHECK(restored.schemaVersion == SaveGameData::kSchemaVersion);
    MOTOR_CHECK(restored.levelPath == original.levelPath);
    MOTOR_CHECK(restored.playerPosition.x == 3 && restored.playerPosition.y == 4);
    MOTOR_CHECK(restored.playerHealth == 42);
    MOTOR_CHECK(restored.playerMaxHealth == 60);
    MOTOR_CHECK(restored.playerMilestones == 3);
    MOTOR_CHECK(restored.playerLevel == 47 && restored.playerLevelProgress == 38);
    MOTOR_CHECK(restored.playerTier == 3 && restored.playerTierMilestones == 5);
    MOTOR_CHECK(restored.inventory == original.inventory);
    MOTOR_CHECK(restored.gold == 175);
    MOTOR_CHECK(restored.moralityValue == -30);
    MOTOR_CHECK(restored.stepsSinceRent == 7);

    MOTOR_CHECK(restored.businesses.size() == 1);
    MOTOR_CHECK(restored.businesses[0].name == "Posada del Cuervo");
    MOTOR_CHECK(restored.businesses[0].rentPercent == 130);

    MOTOR_CHECK(restored.worldObjects.size() == 1);
    MOTOR_CHECK(restored.worldObjects[0].objectId == "antorcha");

    MOTOR_CHECK(restored.enemies.size() == 1);
    MOTOR_CHECK(restored.enemies[0].health == 4);
    MOTOR_CHECK(restored.enemies[0].position.y == 1);

    MOTOR_CHECK(restored.narrativeFlags == original.narrativeFlags);
    MOTOR_CHECK(restored.narrativeBeatsFired == original.narrativeBeatsFired);
    MOTOR_CHECK(restored.narrativeTimers.size() == 1);
    MOTOR_CHECK(restored.narrativeTimers[0].id == "rescate");
    MOTOR_CHECK(restored.narrativeTimers[0].activo);
    MOTOR_CHECK(!restored.narrativeTimers[0].expirado);
    MOTOR_CHECK(restored.narrativeTimers[0].avisosDados.size() == 2);
    MOTOR_CHECK(restored.narrativeTimers[0].avisosDados[0]);
    MOTOR_CHECK(!restored.narrativeTimers[0].avisosDados[1]);
    // Medio segundo de tolerancia sobra: el escritor guarda 6 decimales.
    const float delta = restored.narrativeTimers[0].restante - 12.5f;
    MOTOR_CHECK(delta > -0.001f && delta < 0.001f);
}

// Lo que sale del escritor tiene que poder entrar por el parser del motor:
// si algun dia el guardado se lee desde una herramienta externa, es JSON
// de verdad y no "casi JSON".
MOTOR_TEST_CASE(serialized_save_is_valid_json_for_the_engine_parser) {
    const std::string text = serializeSaveGame(makeFullData());
    Result<JsonValue> parsed = JsonValue::parse(text);
    MOTOR_CHECK(parsed.isOk());
    MOTOR_CHECK(parsed.value().isObject());
    MOTOR_CHECK(parsed.value()["narrative"]["flags"].size() == 2);
}

// Las cadenas con comillas, barras y saltos no pueden romper el fichero.
MOTOR_TEST_CASE(strings_with_quotes_and_newlines_survive) {
    SaveGameData data;
    data.levelPath = "assets/levels/\"raro\".json";
    data.inventory = {"nota:\n\"ve al norte\"", "barra\\invertida", "tabulador\there"};

    SaveGameData restored;
    std::string error;
    MOTOR_CHECK(parseSaveGame(serializeSaveGame(data), &restored, &error));
    MOTOR_CHECK(restored.levelPath == data.levelPath);
    MOTOR_CHECK(restored.inventory == data.inventory);
}

// --- RUN-10.4: version y documentos que no son un guardado ---

MOTOR_TEST_CASE(a_save_from_another_schema_version_is_rejected) {
    std::string text = serializeSaveGame(makeFullData());
    const std::string needle = "\"schemaVersion\": 1";
    const std::size_t at = text.find(needle);
    MOTOR_CHECK(at != std::string::npos);
    text.replace(at, needle.size(), "\"schemaVersion\": 99");

    SaveGameData destination = makeFullData();
    destination.gold = 999;  // centinela: no debe tocarse
    std::string error;
    MOTOR_CHECK(!parseSaveGame(text, &destination, &error));
    MOTOR_CHECK(!error.empty());
    MOTOR_CHECK(error.find("incompatible") != std::string::npos);
    MOTOR_CHECK(destination.gold == 999);
}

MOTOR_TEST_CASE(garbage_and_incomplete_documents_are_rejected) {
    SaveGameData out;
    std::string error;

    MOTOR_CHECK(!parseSaveGame("esto no es json", &out, &error));
    MOTOR_CHECK(!parseSaveGame("[1, 2, 3]", &out, &error));
    MOTOR_CHECK(!parseSaveGame("{\"gold\": 10}", &out, &error));  // sin schemaVersion
    MOTOR_CHECK(!parseSaveGame("{\"schemaVersion\": 1}", &out, &error));  // sin levelPath
    MOTOR_CHECK(error.find("levelPath") != std::string::npos);
}

namespace {

void rejectsSaveWithoutChangingDestination(const std::string& document,
                                          const std::string& fieldPath) {
    SaveGameData destination = makeFullData();
    destination.characterName = "Personaje que no debe perderse";
    destination.narrativeVariables["llaves"] = 3;
    const std::string before = serializeSaveGame(destination);
    std::string error;
    if (parseSaveGame(document, &destination, &error)) {
        MOTOR_CHECK_FAIL("guardado corrupto aceptado: " + document);
    }
    if (error.find("'" + fieldPath + "'") == std::string::npos) {
        MOTOR_CHECK_FAIL("falta la ruta " + fieldPath + " en el motivo: " + error);
    }
    MOTOR_CHECK(serializeSaveGame(destination) == before);
    // El error es opcional: la protección del destino no depende de él.
    MOTOR_CHECK(!parseSaveGame(document, &destination, nullptr));
    MOTOR_CHECK(serializeSaveGame(destination) == before);
}

std::string saveWithFields(const std::string& fields) {
    return R"({"schemaVersion":1,"levelPath":"nivel.json",)" + fields + "}";
}

}  // namespace

MOTOR_TEST_CASE(schema_version_must_be_an_exact_representable_integer) {
    // 1.5 se truncaba a v1; 1e300 y los infinitos llegaban a una conversión
    // double->int fuera de rango antes incluso de rechazar la versión.
    for (const char* version : {"1.5", "-1.5", "2147483648", "-2147483649",
                                "1e300", "-1e300", "1e999", "\"1\"", "true", "null"}) {
        rejectsSaveWithoutChangingDestination(
            std::string("{\"schemaVersion\":") + version + ",\"levelPath\":\"nivel.json\"}",
            "schemaVersion");
    }
    // La representación JSON no importa si el valor sí es exactamente 1.
    SaveGameData loaded;
    std::string error;
    MOTOR_CHECK(parseSaveGame(R"({"schemaVersion":1.0,"levelPath":"nivel.json"})",
                             &loaded, &error));
}

MOTOR_TEST_CASE(present_save_fields_do_not_silently_turn_into_defaults) {
    const struct { const char* fields; const char* path; } cases[] = {
        {R"("playerHealth":"42")", "playerHealth"},
        {R"("playerHealth":42.5)", "playerHealth"},
        {R"("playerMaxHealth":null)", "playerMaxHealth"},
        {R"("playerMilestones":"2")", "playerMilestones"},
        {R"("playerLevel":[])", "playerLevel"},
        {R"("gold":true)", "gold"},
        {R"("morality":[])", "morality"},
        {R"("stepsSinceRent":{})", "stepsSinceRent"},
        {R"("characterName":17)", "characterName"},
        {R"("savedAt":false)", "savedAt"},
        {R"("playerPosition":[])", "playerPosition"},
        {R"("playerPosition":{"x":"3"})", "playerPosition.x"},
        {R"("playerPosition":{"y":2147483648})", "playerPosition.y"},
        {R"("inventory":{"pocion":2})", "inventory"},
        {R"("inventory":["pocion",null])", "inventory[1]"},
        {R"("businesses":false)", "businesses"},
        {R"("worldObjects":{})", "worldObjects"},
        {R"("enemies":null)", "enemies"},
        {R"("narrative":[])", "narrative"},
    };
    for (const auto& c : cases) {
        rejectsSaveWithoutChangingDestination(saveWithFields(c.fields), c.path);
    }
}

MOTOR_TEST_CASE(nested_world_data_reports_the_exact_invalid_field) {
    const struct { const char* fields; const char* path; } cases[] = {
        {R"("businesses":[42])", "businesses[0]"},
        {R"("businesses":[{"objectId":false}])", "businesses[0].objectId"},
        {R"("businesses":[{"name":[]}])", "businesses[0].name"},
        {R"("businesses":[{"baseIncome":1.5}])", "businesses[0].baseIncome"},
        {R"("businesses":[{"rentPercent":1e300}])", "businesses[0].rentPercent"},
        {R"("worldObjects":["antorcha"])", "worldObjects[0]"},
        {R"("worldObjects":[{"objectId":{}}])", "worldObjects[0].objectId"},
        {R"("worldObjects":[{"position":null}])", "worldObjects[0].position"},
        {R"("worldObjects":[{"position":{"x":false}}])", "worldObjects[0].position.x"},
        {R"("enemies":[null])", "enemies[0]"},
        {R"("enemies":[{"objectId":[]}])", "enemies[0].objectId"},
        {R"("enemies":[{"spawnIndex":"2"}])", "enemies[0].spawnIndex"},
        {R"("enemies":[{"health":-1e999}])", "enemies[0].health"},
        {R"("enemies":[{"summoned":1}])", "enemies[0].summoned"},
        {R"("enemies":[{"position":{"y":1.5}}])", "enemies[0].position.y"},
    };
    for (const auto& c : cases) {
        rejectsSaveWithoutChangingDestination(saveWithFields(c.fields), c.path);
    }
}

MOTOR_TEST_CASE(narrative_types_are_validated_before_restoring_any_state) {
    const struct { const char* fields; const char* path; } cases[] = {
        {R"("flags":{})", "narrative.flags"},
        {R"("flags":[false])", "narrative.flags[0]"},
        {R"("beatsFired":null)", "narrative.beatsFired"},
        {R"("beatsFired":["intro",17])", "narrative.beatsFired[1]"},
        {R"("variables":[])", "narrative.variables"},
        {R"("variables":{"llaves":"3"})", "narrative.variables.llaves"},
        {R"("variables":{"llaves":3.5})", "narrative.variables.llaves"},
        {R"("variables":{"llaves":-1e300})", "narrative.variables.llaves"},
        {R"("timers":{})", "narrative.timers"},
        {R"("timers":[false])", "narrative.timers[0]"},
        {R"("timers":[{"id":null}])", "narrative.timers[0].id"},
        {R"("timers":[{"restante":"12.5"}])", "narrative.timers[0].restante"},
        {R"("timers":[{"restante":1e300}])", "narrative.timers[0].restante"},
        {R"("timers":[{"restante":1e999}])", "narrative.timers[0].restante"},
        {R"("timers":[{"activo":1}])", "narrative.timers[0].activo"},
        {R"("timers":[{"expirado":"false"}])", "narrative.timers[0].expirado"},
        {R"("timers":[{"avisosDados":{}}])", "narrative.timers[0].avisosDados"},
        {R"("timers":[{"avisosDados":[true,0]}])", "narrative.timers[0].avisosDados[1]"},
        {R"("localSwitches":false)", "narrative.localSwitches"},
        {R"("localSwitches":[[]])", "narrative.localSwitches[0]"},
        {R"("localSwitches":[{"event":0}])", "narrative.localSwitches[0].event"},
        {R"("localSwitches":[{"event":"cofre","on":null}])", "narrative.localSwitches[0].on"},
        {R"("localSwitches":[{"event":"cofre","on":[true]}])", "narrative.localSwitches[0].on[0]"},
    };
    for (const auto& c : cases) {
        rejectsSaveWithoutChangingDestination(
            saveWithFields(std::string("\"narrative\":{") + c.fields + "}"), c.path);
    }
}

// GUARDAR-FICHA — El heroe va y vuelve entero: lo elegido al crearlo, lo
// puesto y su aspecto. Sin clase no se escribe: un guardado sin heroe (o de
// antes) da el mismo texto y carga sin el.
MOTOR_TEST_CASE(the_hero_survives_the_round_trip) {
    SaveGameData data = makeFullData();
    data.characterName = "Ilse";
    SavedHero& h = data.hero;
    h.raceId = "race_human_marches";
    h.classId = "acechador_de_tejados";
    h.backgroundId = "mes_de_la_agonia";
    h.bondId = "el_laud";
    h.virtueId = "persistente";
    h.equipped = {"", "armadura_de_cuero", "", "", "arco_largo", "arco_largo", "", "", "", "", ""};
    h.raza = "humano";
    h.piel = "clara";
    h.peloEstilo = "corto";
    h.ropa = {{"torso", "gambeson"}, {"pies", "botas"}};
    h.quitarCapas = {"capa"};

    SaveGameData restored;
    std::string error;
    MOTOR_CHECK(parseSaveGame(serializeSaveGame(data), &restored, &error));
    const SavedHero& r = restored.hero;
    MOTOR_CHECK(restored.characterName == "Ilse");
    MOTOR_CHECK(r.raceId == h.raceId && r.classId == h.classId && r.backgroundId == h.backgroundId);
    MOTOR_CHECK(r.deityId.empty() && r.bondId == "el_laud" && r.virtueId == "persistente" && r.fearId.empty());
    MOTOR_CHECK(r.equipped == h.equipped);
    MOTOR_CHECK(r.raza == "humano" && r.piel == "clara" && r.peloEstilo == "corto" && r.barbaEstilo.empty());
    MOTOR_CHECK(r.ropa == h.ropa && r.quitarCapas == h.quitarCapas);

    const std::string sinHeroe = serializeSaveGame(makeFullData());
    MOTOR_CHECK(sinHeroe.find("\"hero\"") == std::string::npos);
    SaveGameData leido;
    MOTOR_CHECK(parseSaveGame(sinHeroe, &leido, &error) && leido.hero.empty());
}

MOTOR_TEST_CASE(the_hero_types_are_validated) {
    const struct { const char* fields; const char* path; } cases[] = {
        {R"("hero":[])", "hero"},
        {R"("hero":{"classId":3})", "hero.classId"},
        {R"("hero":{"virtueId":null})", "hero.virtueId"},
        {R"("hero":{"equipped":"arco"})", "hero.equipped"},
        {R"("hero":{"equipped":["arco",1]})", "hero.equipped[1]"},
        {R"("hero":{"appearance":[]})", "hero.appearance"},
        {R"("hero":{"appearance":{"piel":false}})", "hero.appearance.piel"},
        {R"("hero":{"appearance":{"ropa":{"torso":5}}})", "hero.appearance.ropa.torso"},
        {R"("hero":{"appearance":{"quitarCapas":[null]}})", "hero.appearance.quitarCapas[0]"},
    };
    for (const auto& c : cases) {
        rejectsSaveWithoutChangingDestination(saveWithFields(c.fields), c.path);
    }
}

MOTOR_TEST_CASE(absent_v1_fields_and_unknown_extensions_still_load) {
    SaveGameData loaded = makeFullData();
    std::string error;
    MOTOR_CHECK(parseSaveGame(R"({"schemaVersion":1,"levelPath":"nivel.json"})",
                             &loaded, &error));
    MOTOR_CHECK(loaded.playerPosition.x == 0 && loaded.playerPosition.y == 0);
    MOTOR_CHECK(loaded.playerHealth == 0 && loaded.playerMaxHealth == 0);
    MOTOR_CHECK(loaded.inventory.empty() && loaded.gold == 0);
    MOTOR_CHECK(loaded.businesses.empty() && loaded.worldObjects.empty() && loaded.enemies.empty());
    MOTOR_CHECK(loaded.narrativeFlags.empty() && loaded.narrativeTimers.empty());
    MOTOR_CHECK(loaded.narrativeVariables.empty() && loaded.narrativeLocalSwitches.empty());

    // Tampoco se convierten en obligatorios los miembros de objetos anidados.
    MOTOR_CHECK(parseSaveGame(saveWithFields(R"(
        "playerPosition":{}, "businesses":[{}], "worldObjects":[{}], "enemies":[{}],
        "narrative":{"timers":[{}],"localSwitches":[{}],"extension":null},
        "extension":{"dato":[true,null,17]})"), &loaded, &error));
    MOTOR_CHECK(loaded.businesses[0].rentPercent == 100);
    MOTOR_CHECK(loaded.enemies[0].spawnIndex == -1 && !loaded.enemies[0].summoned);
    MOTOR_CHECK(loaded.narrativeTimers[0].restante == 0.0f);
    MOTOR_CHECK(loaded.narrativeLocalSwitches.empty());
}

MOTOR_TEST_CASE(representable_numbers_are_not_rejected_by_balance_rules) {
    SaveGameData loaded;
    std::string error;
    MOTOR_CHECK(parseSaveGame(saveWithFields(R"(
        "gold":-2147483648, "morality":2147483647, "playerHealth":-7,
        "playerMaxHealth":0, "stepsSinceRent":-1, "playerPosition":{"x":-4,"y":99},
        "narrative":{"timers":[{"restante":-12.5},{"restante":1e38}],
                     "variables":{"contador":2147483647}})"), &loaded, &error));
    MOTOR_CHECK(loaded.gold == std::numeric_limits<int>::min());
    MOTOR_CHECK(loaded.moralityValue == std::numeric_limits<int>::max());
    MOTOR_CHECK(loaded.playerHealth == -7 && loaded.playerMaxHealth == 0);
    MOTOR_CHECK(loaded.stepsSinceRent == -1 && loaded.playerPosition.x == -4);
    MOTOR_CHECK(loaded.narrativeTimers[0].restante == -12.5f);
    MOTOR_CHECK(loaded.narrativeTimers[1].restante > 1e37f);
    MOTOR_CHECK(loaded.narrativeVariables.at("contador") == std::numeric_limits<int>::max());
}

MOTOR_TEST_CASE(unknown_local_switch_letters_keep_the_historical_recovery_policy) {
    SaveGameData loaded;
    std::string error;
    MOTOR_CHECK(parseSaveGame(saveWithFields(R"("narrative":{"localSwitches":[
        {"event":"cofre","on":["Z","","AB","A"]}, {"on":["A"]}]})"), &loaded, &error));
    MOTOR_CHECK(loaded.narrativeLocalSwitches.size() == 1);
    MOTOR_CHECK((loaded.narrativeLocalSwitches[0].on == std::vector<char>{'Z', 'A'}));
    RPG::NarrativeState state;
    applyNarrativeState(loaded, state);
    MOTOR_CHECK(state.localSwitch("cofre", 'A'));
    MOTOR_CHECK(!state.localSwitch("cofre", 'Z'));
}

// --- RUN-10.3: fichero y escritura atomica ---

namespace {

struct SaveWriteSandbox {
    std::filesystem::path root = std::filesystem::temp_directory_path() /
        ("motor-save-write-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    SaveWriteSandbox() { std::filesystem::create_directory(root); }
    ~SaveWriteSandbox() {
        std::error_code ec;
        std::filesystem::remove_all(root, ec);
    }
    std::vector<std::string> entries() const {
        std::vector<std::string> names;
        for (const auto& entry : std::filesystem::directory_iterator(root)) {
            names.push_back(entry.path().filename().u8string());
        }
        std::sort(names.begin(), names.end());
        return names;
    }
};

std::string saveFileBytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    MOTOR_CHECK(file.is_open());
    return std::string(std::istreambuf_iterator<char>(file), {});
}

}  // namespace

MOTOR_TEST_CASE(a_failed_save_never_deletes_a_destination_directory) {
    SaveWriteSandbox sandbox;
    const auto destination = sandbox.root / "partida.json";
    std::filesystem::create_directory(destination);
    const auto before = sandbox.entries();
    std::string error;
    MOTOR_CHECK(!writeSaveGameFile(destination.u8string(), makeFullData(), &error));
    MOTOR_CHECK(!error.empty());
    MOTOR_CHECK(std::filesystem::is_directory(destination));
    MOTOR_CHECK(std::filesystem::is_empty(destination));
    MOTOR_CHECK(sandbox.entries() == before);
}

MOTOR_TEST_CASE(a_failed_replacement_keeps_existing_contents_and_cleans_its_staging) {
    SaveWriteSandbox sandbox;
    const auto destination = sandbox.root / "partida.json";
    std::filesystem::create_directory(destination);
    { std::ofstream marker(destination / "original"); marker << "no tocar"; }
    const auto before = sandbox.entries();
    std::string error;
    MOTOR_CHECK(!writeSaveGameFile(destination.u8string(), makeFullData(), &error));
    MOTOR_CHECK(!error.empty());
    MOTOR_CHECK(saveFileBytes(destination / "original") == "no tocar");
    MOTOR_CHECK(sandbox.entries() == before);
}

MOTOR_TEST_CASE(saving_does_not_reuse_an_existing_temporary_file_or_directory) {
    SaveWriteSandbox sandbox;
    for (bool directory : {false, true}) {
        const auto destination = sandbox.root / (directory ? "dos.json" : "uno.json");
        const auto foreign = std::filesystem::path(destination.u8string() + ".tmp");
        if (directory) std::filesystem::create_directory(foreign);
        else { std::ofstream file(foreign); file << "trabajo ajeno"; }
        std::string error;
        MOTOR_CHECK(writeSaveGameFile(destination.u8string(), makeFullData(), &error));
        if (directory) MOTOR_CHECK(std::filesystem::is_directory(foreign));
        else MOTOR_CHECK(saveFileBytes(foreign) == "trabajo ajeno");
        SaveGameData saved;
        MOTOR_CHECK(readSaveGameFile(destination.u8string(), &saved, &error));
        MOTOR_CHECK(saved.gold == makeFullData().gold);
    }
    MOTOR_CHECK(sandbox.entries().size() == 4);
}

MOTOR_TEST_CASE(an_old_temporary_link_cannot_truncate_another_save) {
    SaveWriteSandbox sandbox;
    const auto archive = sandbox.root / "archivo.json";
    const auto destination = sandbox.root / "partida.json";
    const auto foreign = sandbox.root / "partida.json.tmp";
    std::string error;
    MOTOR_CHECK(writeSaveGameFile(archive.u8string(), makeFullData(), &error));
    const auto original = saveFileBytes(archive);
    std::filesystem::create_hard_link(archive, foreign);
    SaveGameData next = makeFullData();
    next.gold = 999;
    MOTOR_CHECK(writeSaveGameFile(destination.u8string(), next, &error));
    MOTOR_CHECK(saveFileBytes(archive) == original);
    MOTOR_CHECK(saveFileBytes(foreign) == original);
    SaveGameData saved;
    MOTOR_CHECK(readSaveGameFile(destination.u8string(), &saved, &error));
    MOTOR_CHECK(saved.gold == 999);
    MOTOR_CHECK(sandbox.entries().size() == 3);
}

MOTOR_TEST_CASE(saves_with_spaces_and_accents_replace_the_previous_file_without_residue) {
    SaveWriteSandbox sandbox;
    const auto destination = sandbox.root / std::filesystem::u8path("partida de Núria.json");
    std::string error;
    MOTOR_CHECK(writeSaveGameFile(destination.u8string(), makeFullData(), &error));
    SaveGameData next = makeFullData();
    next.gold = 321;
    MOTOR_CHECK(writeSaveGameFile(destination.u8string(), next, &error));
    SaveGameData saved;
    MOTOR_CHECK(readSaveGameFile(destination.u8string(), &saved, &error));
    MOTOR_CHECK(saved.gold == 321);
    MOTOR_CHECK(sandbox.entries().size() == 1);
}

MOTOR_TEST_CASE(an_invalid_parent_fails_without_touching_existing_files) {
    SaveWriteSandbox sandbox;
    const auto original = sandbox.root / "partida.json";
    std::string error;
    MOTOR_CHECK(writeSaveGameFile(original.u8string(), makeFullData(), &error));
    const auto bytes = saveFileBytes(original);
    MOTOR_CHECK(!writeSaveGameFile((original / "hija.json").u8string(), makeFullData(), &error));
    MOTOR_CHECK(!error.empty());
    MOTOR_CHECK(saveFileBytes(original) == bytes);
    MOTOR_CHECK(sandbox.entries().size() == 1);
}

MOTOR_TEST_CASE(writing_a_save_leaves_no_temporary_behind) {
    const std::string path = tempPath("atomico");
    std::string error;
    MOTOR_CHECK(writeSaveGameFile(path, makeFullData(), &error));
    MOTOR_CHECK(error.empty());

    // El .tmp intermedio no puede sobrevivir a un guardado correcto.
    std::ifstream leftover(path + ".tmp", std::ios::binary);
    MOTOR_CHECK(!leftover.good());

    SaveGameData reread;
    MOTOR_CHECK(readSaveGameFile(path, &reread, &error));
    MOTOR_CHECK(reread.gold == 175);

    std::remove(path.c_str());
}

MOTOR_TEST_CASE(saving_twice_replaces_the_previous_save) {
    const std::string path = tempPath("sobrescribe");
    SaveGameData first = makeFullData();
    first.gold = 10;
    SaveGameData second = makeFullData();
    second.gold = 20;

    std::string error;
    MOTOR_CHECK(writeSaveGameFile(path, first, &error));
    MOTOR_CHECK(writeSaveGameFile(path, second, &error));

    SaveGameData reread;
    MOTOR_CHECK(readSaveGameFile(path, &reread, &error));
    MOTOR_CHECK(reread.gold == 20);

    std::remove(path.c_str());
}

MOTOR_TEST_CASE(reading_a_missing_file_fails_with_a_reason) {
    SaveGameData out;
    std::string error;
    MOTOR_CHECK(!readSaveGameFile("run10_no_existe_seguro.json", &out, &error));
    MOTOR_CHECK(!error.empty());
}

// --- RUN-10.2: captura y restauracion sobre la sesion ---

MOTOR_TEST_CASE(a_session_survives_being_saved_and_loaded) {
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;
    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;

    GameSession session(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills, &inventory);

    // Se juega un poco: recoger la pocion de (1,0) y encajar dano.
    MOTOR_CHECK(session.tryMovePlayer(1, 0));
    MOTOR_CHECK(session.lastPickupId() == "pocion");
    MOTOR_CHECK(inventory.size() == 1);
    body.takeDamage(25);
    session.addGold(90);
    session.morality().record(-12, "Subiste el alquiler");

    const SaveGameData saved = session.captureSaveGame("assets/levels/guardado.json");
    MOTOR_CHECK(saved.levelPath == "assets/levels/guardado.json");
    MOTOR_CHECK(saved.playerHealth == 35);
    MOTOR_CHECK(saved.inventory.size() == 1);
    // La pocion recogida ya no esta en el mundo: quedan antorcha y slime.
    MOTOR_CHECK(saved.worldObjects.size() == 1);
    MOTOR_CHECK(saved.worldObjects[0].objectId == "antorcha");
    MOTOR_CHECK(saved.enemies.size() == 1);

    // Partida nueva desde el mismo nivel: todo vuelve a estar entero.
    ObjectCatalog catalog2 = makeCatalog();
    SkillCatalog skills2;
    TestCombatant body2(60);
    SkillSet playerSkills2(10);
    std::vector<std::string> inventory2;
    GameSession loaded(nullptr, &catalog2, &skills2, makeLevel(), &body2, &playerSkills2,
                       &inventory2);
    MOTOR_CHECK(loaded.worldObjects().size() == 2);

    std::string error;
    MOTOR_CHECK(loaded.restoreSaveGame(saved, &error));
    MOTOR_CHECK(error.empty());

    MOTOR_CHECK(loaded.playerPosition().x == 1 && loaded.playerPosition().y == 0);
    MOTOR_CHECK(body2.health() == 35);
    MOTOR_CHECK(inventory2 == inventory);
    MOTOR_CHECK(loaded.gold() == 90);
    MOTOR_CHECK(loaded.morality().value() == -12);
    MOTOR_CHECK(loaded.worldObjects().size() == 1);
    MOTOR_CHECK(loaded.worldObjects()[0].objectId == "antorcha");
    MOTOR_CHECK(loaded.enemies().size() == 1);
    MOTOR_CHECK(loaded.mode() == GameMode::Exploration);
}

// PUERTAS-01 — Guardar dentro de un interior apunta por donde se entro, y al
// cargar la puerta en la que se aparecio sigue llevando a esa calle (Oriol,
// 08/10: «se sale por donde se entra»). Un guardado de antes no la trae y
// carga igual, sin vuelta.
MOTOR_TEST_CASE(guardar_en_un_interior_recuerda_por_que_puerta_se_entro) {
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;
    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills, &inventory);
    session.ponVuelta("assets/levels/ciudad_centro.json", GridCoord{41, 23});
    const SaveGameData saved = session.captureSaveGame("assets/levels/interior_sastreria.json");
    MOTOR_CHECK(saved.returnLevel == "assets/levels/ciudad_centro.json");
    MOTOR_CHECK(saved.returnDoor.x == 41 && saved.returnDoor.y == 23);

    SaveGameData leido;
    std::string error;
    MOTOR_CHECK(parseSaveGame(serializeSaveGame(saved), &leido, &error));
    MOTOR_CHECK(leido.returnLevel == saved.returnLevel);
    MOTOR_CHECK(leido.returnDoor.x == 41 && leido.returnDoor.y == 23);

    ObjectCatalog catalog2 = makeCatalog();
    SkillCatalog skills2;
    TestCombatant body2(60);
    SkillSet playerSkills2(10);
    std::vector<std::string> inventory2;
    GameSession loaded(nullptr, &catalog2, &skills2, makeLevel(), &body2, &playerSkills2, &inventory2);
    MOTOR_CHECK(loaded.restoreSaveGame(leido, &error));
    MOTOR_CHECK(loaded.pendingTransition().vueltaNivel == "assets/levels/ciudad_centro.json");
    MOTOR_CHECK(loaded.pendingTransition().vueltaPuerta.x == 41 && loaded.pendingTransition().vueltaPuerta.y == 23);
    MOTOR_CHECK(!loaded.pendingTransition().pending);

    SaveGameData deAntes = saved;
    deAntes.returnLevel.clear();
    const std::string texto = serializeSaveGame(deAntes);
    MOTOR_CHECK(texto.find("returnLevel") == std::string::npos);
    MOTOR_CHECK(parseSaveGame(texto, &leido, &error) && leido.returnLevel.empty());
    std::string malo = serializeSaveGame(saved);  // la puerta tiene que ser {x, y}
    const std::size_t i = malo.find("\"returnDoor\"");
    MOTOR_CHECK(i != std::string::npos);
    const std::size_t dosPuntos = malo.find(':', i), cierre = malo.find('}', i);
    malo.replace(dosPuntos, cierre - dosPuntos + 1, ": \"la calle\"");
    error.clear();
    MOTOR_CHECK(!parseSaveGame(malo, &leido, &error));
    MOTOR_CHECK(error.find("returnDoor") != std::string::npos);
}

// GUARDAR-FICHA — La sesion apunta quien es el heroe: su nombre, lo elegido
// al crearlo y lo que lleva puesto (el aspecto lo pone la creacion). Un cuerpo
// sin ficha no tiene heroe que guardar.
MOTOR_TEST_CASE(a_session_saves_who_the_hero_is) {
    RPG::CharacterSheet ficha;
    ficha.displayName = "Ilse";
    ficha.raceId = "race_human_marches";
    ficha.classId = "acechador_de_tejados";
    ficha.backgroundId = "mes_de_la_agonia";
    ficha.bondId = "el_laud";
    ficha.virtueId = "persistente";
    ficha.equippedIds[static_cast<std::size_t>(RPG::EquipSlot::Torso)] = "armadura_de_cuero";
    CuerpoConFicha cuerpo(&ficha);
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, makeLevel(), &cuerpo, &playerSkills, &inventory);
    const SaveGameData data = session.captureSaveGame("assets/levels/ciudad_centro.json");
    MOTOR_CHECK(data.characterName == "Ilse");
    MOTOR_CHECK(data.hero.raceId == "race_human_marches" && data.hero.classId == "acechador_de_tejados");
    MOTOR_CHECK(data.hero.backgroundId == "mes_de_la_agonia" && data.hero.bondId == "el_laud");
    MOTOR_CHECK(data.hero.virtueId == "persistente" && data.hero.deityId.empty());
    MOTOR_CHECK(data.hero.equipped.size() == RPG::kRanurasDeEquipo);
    MOTOR_CHECK(data.hero.equipped[static_cast<std::size_t>(RPG::EquipSlot::Torso)] == "armadura_de_cuero");

    TestCombatant otro(30);
    GameSession sinFicha(nullptr, &catalog, &skills, makeLevel(), &otro, &playerSkills, &inventory);
    MOTOR_CHECK(sinFicha.captureSaveGame("assets/levels/ciudad_centro.json").hero.empty());
}

MOTOR_TEST_CASE(a_defeated_enemy_stays_defeated_after_loading) {
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;
    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills, &inventory);

    SaveGameData saved = session.captureSaveGame("assets/levels/guardado.json");
    saved.enemies.clear();  // el slime murio

    ObjectCatalog catalog2 = makeCatalog();
    SkillCatalog skills2;
    TestCombatant body2(60);
    SkillSet playerSkills2(10);
    std::vector<std::string> inventory2;
    GameSession loaded(nullptr, &catalog2, &skills2, makeLevel(), &body2, &playerSkills2,
                       &inventory2);
    MOTOR_CHECK(loaded.enemies().size() == 1);

    std::string error;
    MOTOR_CHECK(loaded.restoreSaveGame(saved, &error));
    MOTOR_CHECK(loaded.enemies().empty());
}

// El guardado guarda la vida que le quede al enemigo: huir de un combate
// a medias y cargar despues no le regala la vida ya perdida.
MOTOR_TEST_CASE(a_wounded_enemy_keeps_its_wounds) {
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;
    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills, &inventory);

    SaveGameData saved = session.captureSaveGame("assets/levels/guardado.json");
    MOTOR_CHECK(saved.enemies.size() == 1);
    saved.enemies[0].health = 3;
    saved.enemies[0].position = GridCoord{6, 2};

    ObjectCatalog catalog2 = makeCatalog();
    SkillCatalog skills2;
    TestCombatant body2(60);
    SkillSet playerSkills2(10);
    std::vector<std::string> inventory2;
    GameSession loaded(nullptr, &catalog2, &skills2, makeLevel(), &body2, &playerSkills2,
                       &inventory2);
    std::string error;
    MOTOR_CHECK(loaded.restoreSaveGame(saved, &error));

    MOTOR_CHECK(loaded.enemies().size() == 1);
    MOTOR_CHECK(loaded.enemies()[0].brain->health() == 3);
    MOTOR_CHECK(loaded.enemies()[0].position.x == 6);
    MOTOR_CHECK(loaded.enemies()[0].brain->position().y == 2);
}

// Si el nivel cambio bajo la partida, restaurar se niega ENTERO en vez de
// colocar el estado en los objetos equivocados.
MOTOR_TEST_CASE(restoring_over_a_changed_level_is_refused_without_side_effects) {
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;
    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills, &inventory);

    SaveGameData saved = session.captureSaveGame("assets/levels/guardado.json");
    saved.gold = 500;
    saved.worldObjects[0].objectId = "cofre_que_ya_no_existe";

    std::string error;
    MOTOR_CHECK(!session.restoreSaveGame(saved, &error));
    MOTOR_CHECK(error.find("ha cambiado") != std::string::npos);
    // Nada aplicado: ni el oro ni el mundo.
    MOTOR_CHECK(session.gold() == 0);
    MOTOR_CHECK(session.worldObjects().size() == 2);

    SaveGameData outOfRange = session.captureSaveGame("assets/levels/guardado.json");
    outOfRange.enemies[0].spawnIndex = 77;
    MOTOR_CHECK(!session.restoreSaveGame(outOfRange, &error));
    MOTOR_CHECK(error.find("fuera de rango") != std::string::npos);
    MOTOR_CHECK(session.enemies().size() == 1);
}

// OBJETOS-01 — El cofre de El Vacío se llamaba «cofre» (el mismo id que el
// mueble de los interiores) y ahora es «cofre_del_vacio». Un guardado de antes
// lo trae con el nombre viejo en su casilla: eso no es que el nivel haya
// cambiado, y la partida carga. Otro objeto en esa casilla sigue sin valer.
MOTOR_TEST_CASE(un_guardado_de_antes_trae_el_cofre_de_el_vacio_con_su_nombre_viejo) {
    ObjectCatalog catalog;
    MOTOR_CHECK(catalog.loadFromString(R"({"objects": [
        {"id": "cofre_del_vacio", "name": "Cofre", "category": "prop", "blocksMovement": true}]})").isOk());
    LevelDefinition level;
    level.name = "El Vacío";
    level.playerStart = GridCoord{0, 0};
    ObjectSpawn cofre;
    cofre.objectId = "cofre_del_vacio";
    cofre.position = GridCoord{1, 0};
    level.objects.push_back(cofre);
    SkillCatalog skills;
    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, level, &body, &playerSkills, &inventory);

    SaveGameData viejo = session.captureSaveGame("assets/levels/interior_vacio.json");
    MOTOR_CHECK(viejo.worldObjects.size() == 1);
    if (viejo.worldObjects.size() != 1) return;
    viejo.worldObjects[0].objectId = "cofre";
    std::string error;
    MOTOR_CHECK(session.restoreSaveGame(viejo, &error));
    MOTOR_CHECK(error.empty());

    viejo.worldObjects[0].objectId = "barril";
    MOTOR_CHECK(!session.restoreSaveGame(viejo, &error));
    MOTOR_CHECK(error.find("ha cambiado") != std::string::npos);
}

// La narrativa se restaura aparte del resto (applyNarrativeState) porque
// quien carga una partida tiene que dejarla lista ANTES de cargar el
// nivel: si no, el beat de "entrar al nivel" repetiria el prologo.
MOTOR_TEST_CASE(narrative_state_is_restored_and_applying_twice_changes_nothing) {
    SaveGameData data = makeFullData();

    RPG::NarrativeState state;
    applyNarrativeState(data, state);

    MOTOR_CHECK(state.hasFlag("prologo_visto"));
    MOTOR_CHECK(state.hasFlag("puerta_abierta"));
    MOTOR_CHECK(!state.hasFlag("final_visto"));
    MOTOR_CHECK(state.beatFired("beat_intro"));
    MOTOR_CHECK(state.flagCount() == 2);
    MOTOR_CHECK(state.timers().size() == 1);
    MOTOR_CHECK(state.timers()[0].activo);

    // Idempotente: restoreSaveGame la vuelve a llamar despues.
    applyNarrativeState(data, state);
    MOTOR_CHECK(state.flagCount() == 2);
    MOTOR_CHECK(state.timers().size() == 1);
    MOTOR_CHECK(state.beatsFired().size() == 1);
}

// Y el camino de ida: una sesion con narrativa enchufada la fotografia.
MOTOR_TEST_CASE(a_session_with_an_adventure_saves_its_flags) {
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;
    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills, &inventory);

    RPG::NarrativeEngine engine;
    RPG::NarrativeState state;
    state.setFlag("llave_encontrada");
    state.markBeatFired("beat_puerta");
    session.setNarrative(&engine, &state);

    const SaveGameData saved = session.captureSaveGame("assets/levels/guardado.json");
    MOTOR_CHECK(saved.narrativeFlags.size() == 1);
    MOTOR_CHECK(saved.narrativeFlags[0] == "llave_encontrada");
    MOTOR_CHECK(saved.narrativeBeatsFired.size() == 1);

    // Y vuelve entera tras pasar por el fichero.
    const std::string path = tempPath("narrativa");
    std::string error;
    MOTOR_CHECK(writeSaveGameFile(path, saved, &error));
    SaveGameData reread;
    MOTOR_CHECK(readSaveGameFile(path, &reread, &error));
    MOTOR_CHECK(reread.narrativeFlags == saved.narrativeFlags);
    MOTOR_CHECK(reread.narrativeBeatsFired == saved.narrativeBeatsFired);
    std::remove(path.c_str());
}

// --- C-08: un guardado antiguo no escribe fuera de rango ---
//
// El bug: TimerState::avisosDados se restaura con el tamano que tenia el
// dia en que se guardo, y tickTimers lo indexa contra el tamano de la
// DEFINICION actual. Anadir un aviso a una aventura ya publicada
// convertia todas las partidas guardadas en una escritura fuera de rango
// -- silenciosa, y por tanto peor que un crash.

namespace {

// Aventura con UN temporizador y "avisos" umbrales. El test la construye
// con dos tamanos distintos para simular "la aventura ha crecido desde
// que se guardo".
RPG::AdventureScript makeAdventureWithWarnings(int warningCount) {
    RPG::TimerDefinition timer;
    timer.id = "incendio";
    timer.duracion = 100.0f;
    timer.expiraPorTiempo = true;
    timer.flagExpira = "casa_ardida";
    for (int i = 0; i < warningCount; ++i) {
        RPG::TimerDefinition::Aviso aviso;
        // Umbrales decrecientes: 0.9, 0.7, 0.5, 0.3...  Con restante=50
        // (la mitad), los dos primeros ya estan cruzados.
        aviso.fraccion = 0.9f - 0.2f * static_cast<float>(i);
        aviso.flag = "aviso_" + std::to_string(i);
        timer.avisos.push_back(aviso);
    }

    RPG::AdventureScript script;
    script.id = "incendio_test";
    script.name = "Incendio";
    script.timers.push_back(timer);
    return script;
}

}  // namespace

MOTOR_TEST_CASE(a_save_with_fewer_warnings_than_the_adventure_does_not_write_out_of_range) {
    // Se guardo con DOS avisos...
    const RPG::AdventureScript vieja = makeAdventureWithWarnings(2);
    RPG::NarrativeEngine engineViejo;
    engineViejo.setAdventure(&vieja);

    RPG::NarrativeState guardando;
    engineViejo.startTimer("incendio", guardando);
    RPG::TimerState& enJuego = guardando.timerRef("incendio");
    enJuego.restante = 50.0f;
    MOTOR_CHECK(enJuego.avisosDados.size() == 2);

    SaveGameData data;
    data.levelPath = "assets/levels/casa.json";
    data.narrativeTimers = guardando.timers();
    MOTOR_CHECK(data.narrativeTimers.size() == 1);
    MOTOR_CHECK(data.narrativeTimers[0].avisosDados.size() == 2);

    // ...y se carga sobre una aventura que ya tiene CUATRO.
    const RPG::AdventureScript nueva = makeAdventureWithWarnings(4);
    RPG::NarrativeEngine engineNuevo;
    engineNuevo.setAdventure(&nueva);

    RPG::NarrativeState cargada;
    applyNarrativeState(data, cargada);
    MOTOR_CHECK(cargada.timerRef("incendio").avisosDados.size() == 2);  // aun el tamano viejo

    // Un tick indexa los cuatro avisos. Antes de C-08, los dos ultimos
    // escribian fuera del vector (ASan lo pilla; sin ASan, corrompia
    // memoria en silencio).
    engineNuevo.tickTimers(cargada, 1.0f, "");

    const RPG::TimerState& ts = cargada.timerRef("incendio");
    MOTOR_CHECK(ts.avisosDados.size() == 4);
    MOTOR_CHECK(ts.restante == 49.0f);
    // Los dos umbrales ya cruzados (0.9 y 0.7 de 100 -> 90 y 70, con 49
    // restante) se han dado; los dos nuevos, 0.5 y 0.3 -> 50 y 30: el de
    // 50 tambien (49 <= 50), el de 30 todavia no.
    MOTOR_CHECK(ts.avisosDados[0]);
    MOTOR_CHECK(ts.avisosDados[1]);
    MOTOR_CHECK(ts.avisosDados[2]);
    MOTOR_CHECK(!ts.avisosDados[3]);
}

// Y al reves: quitar un aviso de la aventura tampoco descuadra nada (el
// vector guardado sobra y se recorta), conservando los ya dados.
MOTOR_TEST_CASE(a_save_with_more_warnings_than_the_adventure_is_trimmed) {
    const RPG::AdventureScript vieja = makeAdventureWithWarnings(4);
    RPG::NarrativeEngine engineViejo;
    engineViejo.setAdventure(&vieja);

    RPG::NarrativeState guardando;
    engineViejo.startTimer("incendio", guardando);
    guardando.timerRef("incendio").restante = 50.0f;

    SaveGameData data;
    data.levelPath = "assets/levels/casa.json";
    data.narrativeTimers = guardando.timers();

    const RPG::AdventureScript nueva = makeAdventureWithWarnings(1);
    RPG::NarrativeEngine engineNuevo;
    engineNuevo.setAdventure(&nueva);

    RPG::NarrativeState cargada;
    applyNarrativeState(data, cargada);
    engineNuevo.tickTimers(cargada, 1.0f, "");

    MOTOR_CHECK(cargada.timerRef("incendio").avisosDados.size() == 1);
    MOTOR_CHECK(cargada.timerRef("incendio").avisosDados[0]);
}

// --- C-04: un enemigo invocado por la narrativa no rompe el guardado ---
//
// El bug: resolveBattles materializaba al enemigo de la emboscada sin
// darle identidad -- spawnIndex se quedaba en -1, que es el valor de
// "ninguno". Al guardar, ese -1 iba al fichero; al cargar,
// restoreSaveGame buscaba un enemigo DEL NIVEL con indice -1, no lo
// encontraba y rechazaba el guardado entero. Huir de una emboscada y
// guardar dejaba la partida inabrible para siempre.

namespace {

ObjectCatalog makeAmbushCatalog() {
    ObjectCatalog catalog;
    auto result = catalog.loadFromString(R"({
        "objects": [
            {"id": "perdido", "name": "Perdido", "category": "enemy",
             "combat": {"maxHealth": 30, "maxMana": 0, "skills": []}}
        ]
    })");
    MOTOR_CHECK(result.isOk());
    return catalog;
}

// Aventura minima: hablar con el vigia dispara una emboscada.
RPG::AdventureScript makeAmbushAdventure() {
    auto loaded = RPG::AdventureScript::loadFromString(R"({
        "id": "emboscada",
        "name": "Emboscada",
        "beats": [
            {
                "id": "emboscada_vigia",
                "trigger": {"type": "talk", "target": "vigia"},
                "speaker": "Vigia",
                "lines": ["Detras de ti."],
                "effects": [
                    {"type": "startBattle",
                     "startBattle": {"monsterId": "perdido",
                                     "flagVictory": "emboscada_vencida",
                                     "flagDefeat": "emboscada_perdida"}}
                ]
            }
        ]
    })");
    MOTOR_CHECK(loaded.isOk());
    return loaded.value();
}

// Nivel sin enemigos propios: todo lo que aparezca en m_enemies lo habra
// puesto la narrativa.
LevelDefinition makeEmptyLevel() {
    LevelDefinition level;
    level.name = "Cuartel";
    level.playerStart = GridCoord{2, 2};
    return level;
}

}  // namespace

MOTOR_TEST_CASE(a_summoned_enemy_survives_saving_and_loading) {
    ObjectCatalog catalog = makeAmbushCatalog();
    SkillCatalog skills;
    const RPG::AdventureScript script = makeAmbushAdventure();
    RPG::NarrativeEngine engine;
    engine.setAdventure(&script);
    RPG::NarrativeState state;

    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, makeEmptyLevel(), &body, &playerSkills,
                        &inventory);
    session.setNarrative(&engine, &state);

    // 1) Hablar con el vigia: la narrativa materializa al Perdido y
    //    arranca el combate.
    MOTOR_CHECK(session.talkNarrative("vigia"));
    MOTOR_CHECK(session.enemies().size() == 1);
    MOTOR_CHECK(session.enemies()[0].objectId == "perdido");
    MOTOR_CHECK(session.enemies()[0].summoned);
    MOTOR_CHECK(session.enemies()[0].spawnIndex >= 0);  // identidad propia, ya no -1
    MOTOR_CHECK(session.battle() != nullptr);

    // 2) Le quitamos 12 PV y huimos: 18 es lo que tiene que sobrevivir al
    //    guardado.
    session.battle()->enemies()[0].combatant->takeDamage(12);
    BattleAction huir;
    huir.type = BattleActionType::Flee;
    session.battle()->resolveAllyAction(0, huir);
    MOTOR_CHECK(session.syncBattleOutcome());
    MOTOR_CHECK(session.battle() == nullptr);
    MOTOR_CHECK(session.enemies().size() == 1);  // sigue en el mundo: se ha huido, no vencido

    // 3) Guardar y volver a leer del fichero (la clave "summoned" es
    //    nueva: tiene que ir y volver).
    const SaveGameData saved = session.captureSaveGame("assets/levels/cuartel.json");
    MOTOR_CHECK(saved.enemies.size() == 1);
    MOTOR_CHECK(saved.enemies[0].summoned);
    MOTOR_CHECK(saved.enemies[0].health == 18);

    const std::string path = tempPath("emboscada");
    std::string error;
    MOTOR_CHECK(writeSaveGameFile(path, saved, &error));
    SaveGameData reread;
    MOTOR_CHECK(readSaveGameFile(path, &reread, &error));
    std::remove(path.c_str());
    MOTOR_CHECK(reread.enemies.size() == 1);
    MOTOR_CHECK(reread.enemies[0].summoned);

    // 4) Cargar sobre una sesion recien construida del mismo nivel. Antes
    //    de C-04 esto devolvia false ("enemigo fuera de rango (-1)") y la
    //    partida no se abria nunca mas.
    TestCombatant body2(60);
    SkillSet playerSkills2(10);
    std::vector<std::string> inventory2;
    SkillCatalog skills2;
    GameSession loaded(nullptr, &catalog, &skills2, makeEmptyLevel(), &body2, &playerSkills2,
                       &inventory2);
    MOTOR_CHECK(loaded.enemies().empty());  // el nivel no trae ninguno

    MOTOR_CHECK(loaded.restoreSaveGame(reread, &error));
    MOTOR_CHECK(loaded.enemies().size() == 1);
    MOTOR_CHECK(loaded.enemies()[0].objectId == "perdido");
    MOTOR_CHECK(loaded.enemies()[0].summoned);
    MOTOR_CHECK(loaded.enemies()[0].brain != nullptr);
    MOTOR_CHECK(loaded.enemies()[0].brain->health() == 18);
    // TUTORIAL-01 — La emboscada sale a la casilla libre de al lado del jugador (2,2),
    // no encima de el: la primera que se mira es la de la derecha.
    MOTOR_CHECK(loaded.enemies()[0].position.x == 3);
    MOTOR_CHECK(loaded.enemies()[0].position.y == 2);
}

// Un enemigo invocado y VENCIDO no vuelve: no aparece en el guardado, y
// por tanto tampoco al cargar (misma regla que los del nivel).
MOTOR_TEST_CASE(a_defeated_summoned_enemy_does_not_come_back) {
    ObjectCatalog catalog = makeAmbushCatalog();
    SkillCatalog skills;
    const RPG::AdventureScript script = makeAmbushAdventure();
    RPG::NarrativeEngine engine;
    engine.setAdventure(&script);
    RPG::NarrativeState state;

    TestCombatant body(60);
    SkillSet playerSkills(10);
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills, makeEmptyLevel(), &body, &playerSkills,
                        &inventory);
    session.setNarrative(&engine, &state);

    MOTOR_CHECK(session.talkNarrative("vigia"));

    // Se le deja 1 PV y se golpea hasta que cae. El bucle es acotado y
    // no un "atacar una vez": el dano depende del grado de la tirada y
    // una pifia hace 0 -- un solo golpe no garantiza nada. Matarlo a
    // mano con takeDamage(999) tampoco vale: BattleState ignora una
    // accion contra un objetivo ya muerto y el combate no terminaria.
    session.battle()->enemies()[0].combatant->takeDamage(29);
    BattleAction golpe;
    golpe.type = BattleActionType::Attack;
    golpe.targetIndex = 0;
    for (int intento = 0; intento < 50 && session.battle() != nullptr &&
                          session.battle()->outcome() == BattleOutcome::InProgress;
         ++intento) {
        session.battle()->resolveAllyAction(0, golpe);
    }
    MOTOR_CHECK(session.battle() != nullptr);
    MOTOR_CHECK(session.battle()->outcome() == BattleOutcome::Victory);
    MOTOR_CHECK(session.syncBattleOutcome());
    MOTOR_CHECK(session.enemies().empty());

    const SaveGameData saved = session.captureSaveGame("assets/levels/cuartel.json");
    MOTOR_CHECK(saved.enemies.empty());
}

// Compatibilidad: un guardado anterior a C-04 no trae la clave
// "summoned". Se lee como false -- "del nivel" -- que es lo unico que
// habia entonces, y sigue cargando igual.
MOTOR_TEST_CASE(a_save_without_the_summoned_key_still_loads) {
    const std::string legacy = R"({
        "schemaVersion": 1,
        "levelPath": "assets/levels/guardado.json",
        "player": {"position": {"x": 0, "y": 0}, "health": 60, "maxHealth": 60},
        "enemies": [
            {"spawnIndex": 2, "objectId": "slime",
             "position": {"x": 5, "y": 0}, "health": 7}
        ]
    })";

    SaveGameData out;
    std::string error;
    MOTOR_CHECK(parseSaveGame(legacy, &out, &error));
    MOTOR_CHECK(out.enemies.size() == 1);
    MOTOR_CHECK(!out.enemies[0].summoned);
    MOTOR_CHECK(out.enemies[0].spawnIndex == 2);
}

int main() { return motor_test::runAllTests(); }
