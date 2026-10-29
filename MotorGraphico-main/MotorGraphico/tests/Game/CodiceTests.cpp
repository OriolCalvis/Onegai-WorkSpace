// CODICE-01 — Lo conocido sale de la partida: beats disparados, nivel
// actual, ficha y flags codice_<id>. Lee los catalogos reales desde build/.
#include "Game/Codice.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/NarrativeState.h"
#include "TestHarness.h"

#include <algorithm>
#include <string>

namespace {

const Game::Codice& codice() {
    static Game::Codice c;
    static bool cargado = false;
    if (!cargado) {
        MOTOR_CHECK(c.cargarCatalogos("assets/catalogs").isOk());
        cargado = true;
    }
    return c;
}

RPG::AdventureScript aventura() {
    const char* json = R"({
      "id": "prueba", "name": "Aventura de prueba", "description": "Una noche larga.",
      "objectives": [],
      "beats": [
        {"id": "b_guardia", "trigger": {"type": "talk", "target": "guardia"}, "speaker": "Guardia", "lines": ["Alto."], "effects": []},
        {"id": "b_taberna", "trigger": {"type": "enter", "target": "taberna"}, "speaker": "", "lines": ["Huele a cerveza."], "effects": []},
        {"id": "b_nunca", "trigger": {"type": "talk", "target": "fantasma"}, "speaker": "Fantasma", "lines": ["Buh."], "effects": []}
      ]})";
    const auto r = RPG::AdventureScript::loadFromString(json);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

bool tiene(const Game::CodiceDatos& d, const std::string& tipo, const std::string& id) {
    return std::any_of(d.entradas.begin(), d.entradas.end(),
                       [&](const Game::CodiceEntrada& e) { return e.tipo == tipo && e.id == id; });
}

}  // namespace

MOTOR_TEST_CASE(codice_solo_conoce_lo_vivido) {
    const RPG::AdventureScript a = aventura();
    RPG::NarrativeState estado;
    Game::CodiceDatos d = codice().construir({&a}, estado, nullptr, "");
    MOTOR_CHECK(d.cuantas("personaje") == 0 && d.cuantas("lugar") == 0 && d.cuantas("lore") == 0);
    MOTOR_CHECK(d.cuantas("deidad") > 0);  // la religion es publica

    estado.markBeatFired("b_guardia");
    estado.markBeatFired("b_taberna");
    d = codice().construir({&a}, estado, nullptr, "assets/levels/ciudad_oeste.json");
    MOTOR_CHECK(d.cuantas("personaje") == 1);  // "Guardia" hablante = "guardia" target
    MOTOR_CHECK(!tiene(d, "personaje", "Fantasma") && !tiene(d, "personaje", "fantasma"));
    MOTOR_CHECK(tiene(d, "lugar", "taberna") && tiene(d, "lugar", "ciudad_oeste"));
    MOTOR_CHECK(tiene(d, "lore", "prueba"));
    const auto lore = std::find_if(d.entradas.begin(), d.entradas.end(),
                                   [](const Game::CodiceEntrada& e) { return e.tipo == "lore"; });
    MOTOR_CHECK(lore->texto == "Una noche larga.");
}

MOTOR_TEST_CASE(codice_usa_la_ficha_para_deidad_clase_y_raza) {
    RPG::CharacterSheet f;
    f.raceId = "race_human_marches";
    f.classId = "acechador_de_tejados";
    RPG::NarrativeState estado;
    Game::CodiceDatos d = codice().construir({}, estado, &f, "");
    MOTOR_CHECK(tiene(d, "lore", "acechador_de_tejados"));
    MOTOR_CHECK(d.cuantas("curiosidad") == 1 && tiene(d, "curiosidad", "race_human_marches"));
    const auto raza = std::find_if(d.entradas.begin(), d.entradas.end(),
                                   [](const Game::CodiceEntrada& e) { return e.tipo == "curiosidad"; });
    MOTOR_CHECK(raza->origen == "Tu raza" && !raza->texto.empty());
    // La deidad propia va la primera de las deidades.
    RPG::Catalogs::DeityCatalog deidades;
    MOTOR_CHECK(deidades.loadFromFile("assets/catalogs/deities.json").isOk());
    std::string ultima;
    deidades.forEach([&](const RPG::DeityDefinition& def) { if (def.name > ultima) ultima = def.name; });
    std::string ultimaId;
    deidades.forEach([&](const RPG::DeityDefinition& def) { if (def.name == ultima) ultimaId = def.id; });
    f.deityId = ultimaId;
    d = codice().construir({}, estado, &f, "");
    const auto primera = std::find_if(d.entradas.begin(), d.entradas.end(),
                                      [](const Game::CodiceEntrada& e) { return e.tipo == "deidad"; });
    MOTOR_CHECK(primera->id == ultimaId && primera->origen == "Tu deidad");
}

MOTOR_TEST_CASE(codice_flags_de_autor_y_texto) {
    RPG::NarrativeState estado;
    estado.setFlag("codice_nation_aegroum");
    estado.setFlag("codice_alguien_inventado");
    estado.setFlag("companero_el_apostata");  // CLASES-03: reclutado -> personaje
    const Game::CodiceDatos d = codice().construir({}, estado, nullptr, "");
    MOTOR_CHECK(tiene(d, "personaje", "companero_el_apostata"));
    MOTOR_CHECK(std::any_of(d.entradas.begin(), d.entradas.end(), [](const Game::CodiceEntrada& e) {
        return e.id == "companero_el_apostata" && e.origen == "Companero" && e.titulo == "Corvo el Apóstata"; }));
    MOTOR_CHECK(tiene(d, "lugar", "nation_aegroum"));
    MOTOR_CHECK(tiene(d, "personaje", "alguien_inventado"));
    const auto lineas = Game::Codice::lineas(d);
    MOTOR_CHECK(lineas.front() == "== Personajes (2)");
    MOTOR_CHECK(lineas[1] == "  Corvo el Apóstata  [Companero]");
    MOTOR_CHECK(lineas[2] == "  Alguien inventado  [Descubierto]");
    MOTOR_CHECK(std::find(lineas.begin(), lineas.end(), "== Lugares (1)") != lineas.end());
    MOTOR_CHECK(Game::Codice::lineas(Game::CodiceDatos{}).front() == "(nada conocido todavia)");
    MOTOR_CHECK(Game::Codice::idDeNivel("a/b/ciudad.json") == "ciudad");
    MOTOR_CHECK(Game::Codice::idDeNivel("plaza") == "plaza");
}

int main() { return motor_test::runAllTests(); }
