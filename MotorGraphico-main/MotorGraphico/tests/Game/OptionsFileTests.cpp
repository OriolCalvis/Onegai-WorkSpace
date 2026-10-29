// OptionsFile / OptionsState — H6-03, los ajustes del jugador guardados.
//
// Lo que se defiende aquí no es que un JSON se lea y se escriba: eso lo hace
// JsonValue. Es la POLÍTICA de un fichero que el jugador puede tener a medias,
// que otra versión del motor pudo escribir, o que alguien puede haber tocado a
// mano. En los tres casos el juego tiene que arrancar, y si lo que se devolvió
// no es lo que el fichero decía, hay que decirlo — la misma ley que G-11 aplica
// a una ranura corrupta («un guardado roto no se lleva por delante la lista»).
//
// Los ficheros los escribe y borra el propio test en su directorio de trabajo
// (el build dir), así que no depende de assets ni de un cwd concreto.

#include "TestHarness.h"

#include "Game/OptionsFile.h"
#include "Game/OptionsState.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

using Game::OptionsFile;
using Game::OptionsRead;
using Game::OptionsState;
using Game::OptionsValues;

namespace {

std::string ruta(const std::string& nombre) { return "opf_" + nombre + ".json"; }

void escribir(const std::string& path, const std::string& texto) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << texto;
    file.close();
}

void borrar(const std::string& path) { std::remove(path.c_str()); }

bool menciona(const std::vector<std::string>& problems, const std::string& aguja) {
    for (const std::string& line : problems) {
        if (line.find(aguja) != std::string::npos) {
            return true;
        }
    }
    return false;
}

}  // namespace

MOTOR_TEST_CASE(escribir_y_leer_devuelve_lo_mismo) {
    const std::string path = ruta("ida_vuelta");
    OptionsValues values;
    values.music = 3;
    values.sfx = 10;
    values.ambience = 0;
    values.language = "ENGLISH";

    MOTOR_CHECK(OptionsFile::write(path, values).isOk());
    const OptionsRead read = OptionsFile::read(path);
    MOTOR_CHECK(read.found);
    MOTOR_CHECK(read.problems.empty());
    MOTOR_CHECK(read.values.music == 3);
    MOTOR_CHECK(read.values.sfx == 10);
    MOTOR_CHECK(read.values.ambience == 0);
    MOTOR_CHECK(read.values.language == "ENGLISH");
    borrar(path);
}

MOTOR_TEST_CASE(sin_fichero_se_arranca_con_los_defaults_y_no_es_un_problema) {
    const std::string path = ruta("no_existe_seguro");
    borrar(path);  // por si una pasada anterior lo dejó

    const OptionsRead read = OptionsFile::read(path);
    MOTOR_CHECK(!read.found);
    // La primera vez que se arranca el juego no es un problema: si lo fuera,
    // quien cablea aprendería a ignorar `problems` y entonces no serviría.
    MOTOR_CHECK(read.problems.empty());
    const OptionsValues defaults;
    MOTOR_CHECK(read.values.music == defaults.music);
    MOTOR_CHECK(read.values.sfx == defaults.sfx);
    MOTOR_CHECK(read.values.ambience == defaults.ambience);
    MOTOR_CHECK(read.values.language == defaults.language);
    // Y coinciden con los de la pantalla, que es lo que el jugador ve.
    const OptionsState state = OptionsState::create();
    MOTOR_CHECK(read.values.music == state.volume("music"));
    MOTOR_CHECK(read.values.language == state.language());
}

MOTOR_TEST_CASE(un_json_roto_no_deja_al_jugador_sin_ajustes) {
    const std::string path = ruta("roto");
    escribir(path, "{ esto no es json");
    const OptionsRead read = OptionsFile::read(path);
    MOTOR_CHECK(read.found);           // había fichero: no es la primera vez
    MOTOR_CHECK(!read.problems.empty());
    MOTOR_CHECK(menciona(read.problems, "JSON"));
    MOTOR_CHECK(read.values.music == OptionsValues().music);
    borrar(path);
}

MOTOR_TEST_CASE(un_volumen_fuera_de_rango_se_recorta_y_se_dice) {
    const std::string path = ruta("fuera_de_rango");
    escribir(path, R"({"schemaVersion":1,"language":"CATALA",)"
                   R"("audio":{"music":99,"sfx":-3,"ambience":5}})");
    const OptionsRead read = OptionsFile::read(path);
    MOTOR_CHECK(read.values.music == OptionsState::kVolumeMax);
    MOTOR_CHECK(read.values.sfx == OptionsState::kVolumeMin);
    MOTOR_CHECK(read.values.ambience == 5);   // el sano no se toca
    MOTOR_CHECK(menciona(read.problems, "music"));
    MOTOR_CHECK(menciona(read.problems, "sfx"));
    MOTOR_CHECK(!menciona(read.problems, "ambience"));
    borrar(path);
}

MOTOR_TEST_CASE(un_campo_del_tipo_equivocado_se_descarta_y_se_dice) {
    const std::string path = ruta("tipo_equivocado");
    escribir(path, R"({"audio":{"music":"alto","sfx":4}})");
    const OptionsRead read = OptionsFile::read(path);
    MOTOR_CHECK(read.values.music == OptionsValues().music);
    MOTOR_CHECK(read.values.sfx == 4);
    MOTOR_CHECK(menciona(read.problems, "no es un numero"));
    borrar(path);
}

MOTOR_TEST_CASE(un_idioma_desconocido_se_rechaza) {
    const std::string path = ruta("idioma_raro");
    escribir(path, R"({"language":"KLINGON","audio":{"music":6}})");
    const OptionsRead read = OptionsFile::read(path);
    // Se rechaza en vez de conservarse: un idioma fuera de la lista dejaría la
    // interfaz sin textos y el jugador no sabría por qué.
    MOTOR_CHECK(read.values.language == OptionsValues().language);
    MOTOR_CHECK(menciona(read.problems, "KLINGON"));
    MOTOR_CHECK(read.values.music == 6);   // el resto del fichero sí se aprovecha
    borrar(path);
}

MOTOR_TEST_CASE(una_version_que_este_motor_no_conoce_no_se_interpreta) {
    const std::string path = ruta("version_futura");
    escribir(path, R"({"schemaVersion":2,"language":"ENGLISH","audio":{"music":1}})");
    const OptionsRead read = OptionsFile::read(path);
    MOTOR_CHECK(menciona(read.problems, "schemaVersion"));
    MOTOR_CHECK(read.values.music == OptionsValues().music);
    MOTOR_CHECK(read.values.language == OptionsValues().language);
    borrar(path);
}

MOTOR_TEST_CASE(un_fichero_parcial_conserva_los_defaults_que_faltan) {
    const std::string path = ruta("parcial");
    // Política de SaveGameValidation: la ausencia conserva el default y no es
    // un problema; solo un campo presente con otro tipo lo es.
    escribir(path, R"({"audio":{"music":2}})");
    const OptionsRead read = OptionsFile::read(path);
    MOTOR_CHECK(read.values.music == 2);
    MOTOR_CHECK(read.values.sfx == OptionsValues().sfx);
    MOTOR_CHECK(read.values.language == OptionsValues().language);
    MOTOR_CHECK(read.problems.empty());
    borrar(path);
}

MOTOR_TEST_CASE(escribir_recorta_antes_de_guardar) {
    const std::string path = ruta("recorte_al_escribir");
    OptionsValues values;
    values.music = 99;
    values.sfx = -5;
    MOTOR_CHECK(OptionsFile::write(path, values).isOk());
    const OptionsRead read = OptionsFile::read(path);
    MOTOR_CHECK(read.values.music == OptionsState::kVolumeMax);
    MOTOR_CHECK(read.values.sfx == OptionsState::kVolumeMin);
    // Lo importante: el fichero que el motor acaba de escribir no necesita
    // cura, o sea que no avisa de nada.
    MOTOR_CHECK(read.problems.empty());
    borrar(path);
}

MOTOR_TEST_CASE(el_fichero_guardado_es_json_con_los_campos_documentados) {
    const std::string path = ruta("formato");
    OptionsValues values;
    values.music = 7;
    MOTOR_CHECK(OptionsFile::write(path, values).isOk());
    std::ifstream file(path, std::ios::binary);
    std::string text;
    std::getline(file, text, static_cast<char>(EOF));
    file.close();
    MOTOR_CHECK(text.find("\"schemaVersion\"") != std::string::npos);
    MOTOR_CHECK(text.find("\"audio\"") != std::string::npos);
    MOTOR_CHECK(text.find("\"music\"") != std::string::npos);
    MOTOR_CHECK(text.find("\"language\"") != std::string::npos);
    borrar(path);
}

MOTOR_TEST_CASE(escribir_en_una_ruta_imposible_devuelve_error) {
    OptionsValues values;
    const Result<void> written =
        OptionsFile::write("directorio_que_no_existe_seguro/opciones.json", values);
    MOTOR_CHECK(!written.isOk());
    MOTOR_CHECK(!written.errorMessage().empty());
}

MOTOR_TEST_CASE(el_puente_con_la_pantalla_ida_y_vuelta) {
    const std::string path = ruta("puente");
    OptionsState pantalla = OptionsState::create();
    pantalla.setVolume("music", 2);
    pantalla.setVolume("sfx", 4);
    pantalla.setVolume("ambience", 6);
    MOTOR_CHECK(pantalla.setLanguage("ENGLISH"));

    MOTOR_CHECK(OptionsFile::write(path, OptionsFile::fromState(pantalla)).isOk());
    const OptionsRead read = OptionsFile::read(path);
    MOTOR_CHECK(read.problems.empty());

    // Una pantalla NUEVA, como la que crearía el juego al arrancar otra vez.
    OptionsState siguiente = OptionsState::create();
    MOTOR_CHECK(siguiente.volume("music") == 8);   // antes de aplicar: el default
    MOTOR_CHECK(OptionsFile::applyTo(siguiente, read.values));
    MOTOR_CHECK(siguiente.volume("music") == 2);
    MOTOR_CHECK(siguiente.volume("sfx") == 4);
    MOTOR_CHECK(siguiente.volume("ambience") == 6);
    MOTOR_CHECK(siguiente.language() == "ENGLISH");

    // Y lo que el HUD pinta refleja el valor cargado, no el default.
    bool visto = false;
    for (const std::string& line : siguiente.displayLines()) {
        if (line.find("MUSICA") != std::string::npos && line.find('2') != std::string::npos) {
            visto = true;
        }
    }
    MOTOR_CHECK(visto);
    borrar(path);
}

MOTOR_TEST_CASE(los_setters_de_la_pantalla_recortan_y_rechazan) {
    OptionsState state = OptionsState::create();
    state.setVolume("music", 99);
    MOTOR_CHECK(state.volume("music") == OptionsState::kVolumeMax);
    state.setVolume("music", -1);
    MOTOR_CHECK(state.volume("music") == OptionsState::kVolumeMin);
    // Una clave que no es de volumen no inventa un campo nuevo.
    state.setVolume("master", 5);
    MOTOR_CHECK(state.volume("master") == 0);
    // Un idioma fuera de la lista no se aplica y se dice.
    const std::string antes = state.language();
    MOTOR_CHECK(!state.setLanguage("KLINGON"));
    MOTOR_CHECK(state.language() == antes);
    MOTOR_CHECK(state.setLanguage("CASTELLA"));
    MOTOR_CHECK(state.language() == "CASTELLA");
}

int main() { return motor_test::runAllTests(); }
