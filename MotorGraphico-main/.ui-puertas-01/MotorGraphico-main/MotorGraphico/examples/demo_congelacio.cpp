// G-07: demo GL-free que verifica la congercio de esquemes (serializacio
// d'aventura i nivell) en assets reals, no nomes en fixtures de test.
//
// El flux que comprova: asset -> struct -> JSON (serialitzador) -> struct
// (parser) -> JSON (serialitzador) == JSON. Si el segon JSON difereix del
// primer, el lector i l'escritor no estan d'acord i la fixture no es
// un contrato de veritat. El demo carrega dues aventures/levels reals,
// verifica idempotencia, i també comprova la fixture congelada de
// test_schema_fixtures.
//
// GL-free: només usa motor_json (JsonValue), motor_rpg (AdventureScript +
// NarrativeEngine) i motor_level (LevelLoader). Sense OpenGL/GLFW ni
// assets binaris. S'executa en CI com la resta de demos GL-free.
#include "RPG/AdventureSerializer.h"
#include "RPG/NarrativeEngine.h"
#include "Level/LevelLoader.h"
#include "Level/LevelSerializer.h"

#include "Core/Check.h"

#include <iostream>
#include <string>

namespace {

// Carrega un AdventureScript, el serialitza, el recarrega i verifica que
// el text serialitzat es idempotent (serializar dos vegades dona el mateix).
bool verificarAventura(const std::string& path) {
    std::cout << "\n--- Aventura: " << path << " ---\n";

    auto loaded = RPG::AdventureScript::loadFromFile(path);
    if (!loaded.isOk()) {
        std::cout << "  AVIS: no s'ha pogut carregar " << path
                  << " (" << loaded.errorMessage() << ")\n";
        return false;
    }
    const RPG::AdventureScript& a = loaded.value();
    std::cout << "  carregada: \"" << a.name << "\", "
              << a.beats.size() << " beats, "
              << a.objectives.size() << " objectius, "
              << a.timers.size() << " temporadors\n";

    // Serialitza -> recarrega -> serialitza torna
    const std::string json1 = RPG::serializeAdventure(a);
    auto reparsed = RPG::AdventureScript::loadFromString(json1);
    if (!reparsed.isOk()) {
        std::cout << "  FALLA: la serialitzacio no es pot reparar ("
                  << reparsed.errorMessage() << ")\n";
        return false;
    }
    const std::string json2 = RPG::serializeAdventure(reparsed.value());

    if (json1 != json2) {
        std::cout << "  FALLA: idempotencia trencada a " << path << "\n";
        std::cout << "  json1 (" << json1.size() << " bytes) != json2 ("
                  << json2.size() << " bytes)\n";
        return false;
    }
    std::cout << "  idempotencia OK (" << json1.size() << " bytes estables)\n";

    // Verifica que les dades principals sobreviuen el round-trip
    const RPG::AdventureScript& a2 = reparsed.value();
    require(a2.beats.size() == a.beats.size());
    require(a2.objectives.size() == a.objectives.size());
    if (a.timers.size() != a2.timers.size()) {
        std::cout << "  FALLA: timers perdsos en round-trip\n";
        return false;
    }
    std::cout << "  dades principals preservades (beats/objectius/timers)\n";
    return true;
}

bool verificarNivell(const std::string& path) {
    std::cout << "\n--- Nivell: " << path << " ---\n";

    auto loaded = LevelLoader::loadFromFile(path);
    if (!loaded.isOk()) {
        std::cout << "  AVIS: no s'ha pogut carregar " << path
                  << " (" << loaded.errorMessage() << ")\n";
        return false;
    }
    const LevelDefinition& l = loaded.value();
    std::cout << "  carregat: \"" << l.name << "\", "
              << l.enemies.size() << " enemics, "
              << l.objects.size() << " objectes\n";

    const std::string json1 = serializeLevelDefinition(l);
    auto reparsed = LevelLoader::loadFromString(json1);
    if (!reparsed.isOk()) {
        std::cout << "  FALLA: la serialitzacio no es pot reparar ("
                  << reparsed.errorMessage() << ")\n";
        return false;
    }
    const std::string json2 = serializeLevelDefinition(reparsed.value());

    if (json1 != json2) {
        std::cout << "  FALLA: idempotencia trencada a " << path << "\n";
        return false;
    }
    std::cout << "  idempotencia OK (" << json1.size() << " bytes estables)\n";

    const LevelDefinition& l2 = reparsed.value();
    require(l2.enemies.size() == l.enemies.size());
    require(l2.objects.size() == l.objects.size());
    require(l2.playerStart.x == l.playerStart.x && l2.playerStart.y == l.playerStart.y);
    if (l.fogEnabled != l2.fogEnabled || l.lightingEnabled != l2.lightingEnabled) {
        std::cout << "  FALLA: fog/lighting perdut en round-trip\n";
        return false;
    }
    std::cout << "  dades principals preservades (enemics/objectes/playerStart/fog/lighting)\n";
    return true;
}

}  // namespace

int main() {
    std::cout << "[CONGELACION G-07] verificacio de serializacio en assets reals\n";

    bool ok = true;

    // Aventura real del repo
    ok = verificarAventura("assets/adventures/aventura_silbido_medianoche.json") && ok;

    // Nivells reals del repo (2 exemples tipus)
    ok = verificarNivell("assets/levels/test_level.json") && ok;
    ok = verificarNivell("assets/levels/ciudad_centro.json") && ok;

    // Fixtures congelades de SchemaFixtureTests (G-07)
    std::cout << "\n--- Fixture congelada d'aventura (G-07) ---\n";
    {
        auto adv = RPG::AdventureScript::loadFromFile(
            "tests/fixtures/esquemas/aventura-v1.json");
        require(adv.isOk());
        const std::string idem = RPG::serializeAdventure(adv.value());
        auto reparsed = RPG::AdventureScript::loadFromString(idem);
        require(reparsed.isOk());
        require(RPG::serializeAdventure(reparsed.value()) == idem);
        std::cout << "  fixture aventura-v1.json idempotent (" << idem.size() << " bytes)\n";
    }

    std::cout << "\n--- Fixture congelada de nivell (G-07) ---\n";
    {
        auto lvl = LevelLoader::loadFromFile("tests/fixtures/esquemas/nivel-v1.json");
        require(lvl.isOk());
        const std::string idem = serializeLevelDefinition(lvl.value());
        auto reparsed = LevelLoader::loadFromString(idem);
        require(reparsed.isOk());
        require(serializeLevelDefinition(reparsed.value()) == idem);
        std::cout << "  fixture nivel-v1.json idempotent (" << idem.size() << " bytes)\n";
    }

    std::cout << "\n[CONGELACION G-07] " << (ok ? "TOTES les comprobacions han passat." : "Hi ha errors (veure adreces alerta).")
              << "\n";
    return ok ? 0 : 1;
}
