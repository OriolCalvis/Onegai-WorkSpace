// H1-05 / G-17 — proves del bridge OptionsState ↔ Locale.
//
// Verifica que canviar l'idioma a Opcions (CATALA/CASTELLA/ENGLISH)
// propagate al Locale ("ca"/"es"/"en") i que el text canvia efectivament
// amb les taules reals d'assets/i18n/.
//
// Locale.cpp, OptionsState.cpp i OptionsLocaleBridge.cpp es compilen directament
// (patró G-14): no s'afegeix a motor_level per evitar conflicts sobre
// CMakeLists.txt. Precisa copiar_assets per a assets/i18n/.

#include "Game/OptionsState.h"
#include "Game/OptionsLocaleBridge.h"

#include <string>

#include "Engine/Locale.h"
#include "TestHarness.h"

namespace {

const char* kTestKey = "combate.ataca";

// Torna una Locale carregada de veritat des d'assets/i18n/.
Engine::Locale loadRealLocale() {
    Engine::Locale loc;
    // loadFromDirectory requereix el WORKING_DIRECTORY del build.
    loc.loadFromDirectory("assets/i18n");
    return loc;
}

}  // namespace

// 1. Mapatge de noms d'OptionsState → codis de Locale.
MOTOR_TEST_CASE(locale_mapping_catala_ca) {
    MOTOR_CHECK(Game::OptionsLocaleBridge::localeCode("CATALA") == "ca");
}

// 2. Inversa: codi → nom d'OptionsState.
MOTOR_TEST_CASE(locale_mapping_castella_es) {
    MOTOR_CHECK(Game::OptionsLocaleBridge::localeCode("CASTELLA") == "es");
    MOTOR_CHECK(Game::OptionsLocaleBridge::optionsLanguage("es") == "CASTELLA");
}

// 3. Tots els mapejos són compatibles amb el manifest d'idiomes.
MOTOR_TEST_CASE(locale_mapping_english_en) {
    MOTOR_CHECK(Game::OptionsLocaleBridge::localeCode("ENGLISH") == "en");
    MOTOR_CHECK(Game::OptionsLocaleBridge::optionsLanguage("en") == "ENGLISH");
}

// 4. Noms/codis desconeguts retornen cadena buida.
MOTOR_TEST_CASE(locale_mapping_unknown_returns_empty) {
    MOTOR_CHECK(Game::OptionsLocaleBridge::localeCode("FRANCES").empty());
    MOTOR_CHECK(Game::OptionsLocaleBridge::localeCode("").empty());
    MOTOR_CHECK(Game::OptionsLocaleBridge::optionsLanguage("fr").empty());
}

// 5. applyLanguage canvia efectivament l'idioma del Locale.
MOTOR_TEST_CASE(bridge_apply_language_changes_locale) {
    auto loc = loadRealLocale();
    MOTOR_CHECK(loc.loaded());

    // OptionsState comença en CATALA → code "ca".
    Game::OptionsState opt = Game::OptionsState::create();
    MOTOR_CHECK(opt.language() == "CATALA");

    // Aplica'l: el Locale ha d'entrar en "ca".
    MOTOR_CHECK(loc.language() == "es");  // el manifiesto diu referència "es"
    MOTOR_CHECK(Game::OptionsLocaleBridge::applyLanguage(opt, loc));
    MOTOR_CHECK(loc.language() == "ca");

    // Cicla a CASTELLA: adjustRight quan la fila seleccionada és IDIOMA.
    // La fila IDIOMA és l'índex 3; moure-ho allà necessita moveDown 3 vegades.
    opt.moveDown(); opt.moveDown(); opt.moveDown();
    MOTOR_CHECK(opt.selectedEntry().key == "language");
    opt.adjustRight();
    MOTOR_CHECK(opt.language() == "CASTELLA");

    MOTOR_CHECK(Game::OptionsLocaleBridge::applyLanguage(opt, loc));
    MOTOR_CHECK(loc.language() == "es");
}

// 6. El text canvia entre idiomes amb les taules reals.
MOTOR_TEST_CASE(bridge_text_changes_between_languages) {
    auto loc = loadRealLocale();
    MOTOR_CHECK(loc.loaded());

    // Català: "ataca {objetivo}" (sense preposició "a").
    Game::OptionsState opt = Game::OptionsState::create();
    MOTOR_CHECK(Game::OptionsLocaleBridge::applyLanguage(opt, loc));
    MOTOR_CHECK(loc.language() == "ca");
    const std::string caText = loc.text(kTestKey).text;
    MOTOR_CHECK(!caText.empty());
    MOTOR_CHECK(caText.find("dany") != std::string::npos);  // ca: "de dany"

    // Castellano: "ataca a {objetivo}" (amb preposició "a").
    opt.moveDown(); opt.moveDown(); opt.moveDown();
    opt.adjustRight();  // → CASTELLA
    MOTOR_CHECK(opt.language() == "CASTELLA");
    MOTOR_CHECK(Game::OptionsLocaleBridge::applyLanguage(opt, loc));
    MOTOR_CHECK(loc.language() == "es");
    const std::string esText = loc.text(kTestKey).text;
    MOTOR_CHECK(!esText.empty());
    MOTOR_CHECK(esText.find("de daño") != std::string::npos);  // es: "de daño"

    // Els textos són efectivament diferents.
    MOTOR_CHECK(caText != esText);
}

int main() { return motor_test::runAllTests(); }
