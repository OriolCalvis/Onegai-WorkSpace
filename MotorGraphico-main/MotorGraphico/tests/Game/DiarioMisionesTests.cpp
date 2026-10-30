// MISIONES-01 — El diario es una vista de las flags: nada que guardar,
// nada que pueda desincronizarse.
#include "Game/DiarioMisiones.h"
#include "RPG/Narrative/AdventureScript.h"
#include "RPG/Narrative/NarrativeState.h"
#include "TestHarness.h"

#include <string>

namespace {

RPG::AdventureScript aventura(const std::string& id, const std::string& nombre,
                              std::initializer_list<std::pair<const char*, const char*>> objetivos) {
    std::string json = "{\"id\":\"" + id + "\",\"name\":\"" + nombre + "\",\"objectives\":[";
    bool primero = true;
    for (const auto& [texto, flag] : objetivos) {
        if (!primero) json += ",";
        primero = false;
        json += std::string("{\"id\":\"obj_") + flag + "\",\"text\":\"" + texto + "\",\"doneFlag\":\"" + flag + "\"}";
    }
    json += "],\"beats\":[]}";
    const auto r = RPG::AdventureScript::loadFromString(json);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

}  // namespace

MOTOR_TEST_CASE(diario_resuelve_objetivos_contra_las_flags_y_pone_la_activa_primero) {
    const RPG::AdventureScript prologo = aventura("prologo", "Prologo", {{"Hablar con la posadera", "posadera"}, {"Encontrar la llave", "llave"}});
    const RPG::AdventureScript dia1 = aventura("dia1", "Primer dia", {{"Ir al mercado", "mercado"}});
    RPG::NarrativeState estado;
    estado.setFlag("posadera");

    Game::DiarioDatos d = Game::construirDiario({&prologo, &dia1}, &dia1, estado);
    MOTOR_CHECK(d.misiones.size() == 2);
    MOTOR_CHECK(d.misiones[0].id == "dia1" && d.misiones[0].activa);
    MOTOR_CHECK(std::string(d.misiones[0].estado()) == "activa");
    MOTOR_CHECK(d.misiones[1].id == "prologo" && !d.misiones[1].activa);
    MOTOR_CHECK(d.misiones[1].hechos == 1 && d.misiones[1].total == 2);
    MOTOR_CHECK(d.misiones[1].objetivos[0].hecho && !d.misiones[1].objetivos[1].hecho);
    MOTOR_CHECK(std::string(d.misiones[1].estado()) == "pendiente");
    MOTOR_CHECK(d.activas() == 1 && d.completas() == 0);

    estado.setFlag("llave");
    d = Game::construirDiario({&prologo, &dia1}, &dia1, estado);
    MOTOR_CHECK(d.misiones[1].completa() && std::string(d.misiones[1].estado()) == "hecha");
    MOTOR_CHECK(d.completas() == 1);

    // La activa puede ser la completada: entonces no cuenta como activa.
    // Sin esto, activas() daria 1 con el diario entero fulfilling.
    Game::DiarioDatos c = Game::construirDiario({&prologo, &dia1}, &prologo, estado);
    MOTOR_CHECK(c.misiones[0].activa && c.misiones[0].completa());
    MOTOR_CHECK(c.activas() == 0);
    MOTOR_CHECK(c.completas() == 1);
    MOTOR_CHECK(std::string(c.misiones[1].estado()) == "pendiente");
}

MOTOR_TEST_CASE(diario_no_duplica_la_activa_ni_se_cae_con_nulos) {
    const RPG::AdventureScript sola = aventura("sola", "Sola", {{"Algo", "algo"}});
    RPG::NarrativeState estado;
    const Game::DiarioDatos d = Game::construirDiario({nullptr, &sola, nullptr}, &sola, estado);
    MOTOR_CHECK(d.misiones.size() == 1 && d.misiones[0].activa);
    const Game::DiarioDatos vacio = Game::construirDiario({}, nullptr, estado);
    MOTOR_CHECK(vacio.misiones.empty());
    MOTOR_CHECK(Game::lineasDiario(vacio).front() == "(sin misiones)");
    const RPG::AdventureScript sinObjetivos = aventura("plaza", "Plaza", {});
    const Game::DiarioDatos p = Game::construirDiario({}, &sinObjetivos, estado);
    MOTOR_CHECK(p.misiones[0].total == 0 && !p.misiones[0].completa());

    // La campana puede traer OTRO objeto con el mismo id que la activa (dos
    // cargadores, una aventura repetida): el dedup es por id, no solo por
    // puntero, o el diario repite la mision.
    const RPG::AdventureScript misma = aventura("sola", "Sola", {{"Algo", "algo"}});
    const Game::DiarioDatos d2 = Game::construirDiario({&misma}, &sola, estado);
    MOTOR_CHECK(d2.misiones.size() == 1 && d2.misiones[0].id == "sola");

    // Sin name, la linea tiene que llevar el id: una cadena vacia en el diario
    // es indistinguible de una mision sin titulo.
    const RPG::AdventureScript sinNombre = aventura("sin_titulo", "", {{"Algo", "algo"}});
    const Game::DiarioDatos n = Game::construirDiario({&sinNombre}, nullptr, estado);
    MOTOR_CHECK(n.misiones[0].nombre == "sin_titulo");
    MOTOR_CHECK(Game::lineasDiario(n).front() == "  sin_titulo 0/1");
}

MOTOR_TEST_CASE(diario_en_texto_marca_activa_hecha_y_casillas) {
    const RPG::AdventureScript a = aventura("a", "Aventura A", {{"Uno", "f1"}, {"Dos", "f2"}});
    const RPG::AdventureScript b = aventura("b", "Aventura B", {{"Tres", "f3"}});
    RPG::NarrativeState estado;
    estado.setFlag("f1");
    estado.setFlag("f3");
    const auto lineas = Game::lineasDiario(Game::construirDiario({&a, &b}, &a, estado));
    MOTOR_CHECK(lineas.size() == 5);
    MOTOR_CHECK(lineas[0] == "> Aventura A 1/2");
    MOTOR_CHECK(lineas[1] == "  [x] Uno");
    MOTOR_CHECK(lineas[2] == "  [ ] Dos");
    MOTOR_CHECK(lineas[3] == "* Aventura B 1/1");
}

// GUARDAR-FICHA — El capitulo de una partida guardada sale de las flags de cada
// salto de la campana: sin ninguna, el prologo; con varias, el ultimo.
MOTOR_TEST_CASE(el_capitulo_de_la_campana_sale_de_sus_flags) {
    RPG::NarrativeState estado;
    MOTOR_CHECK(Game::capituloDeLaCampana(estado) == 0);
    estado.setFlag("bnd_prologo_terminado");
    MOTOR_CHECK(Game::capituloDeLaCampana(estado) == 0);  // terminado, pero aun no ha saltado
    estado.setFlag("bnd_dia1_empezado");
    MOTOR_CHECK(Game::capituloDeLaCampana(estado) == 1);
    estado.setFlag("bnd_dia2_empezado");
    MOTOR_CHECK(Game::capituloDeLaCampana(estado) == 2);
    estado.setFlag("bnd_ocaso_empezado");
    MOTOR_CHECK(Game::capituloDeLaCampana(estado) == 3);
}

int main() { return motor_test::runAllTests(); }
