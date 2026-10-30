#include "Game/SaveGame.h"
#include "RPG/NarrativeEngine.h"

#include "TestHarness.h"

#include <limits>
#include <string>

// E-02 — Variables numericas con nombre e interruptores locales A-D.
//
// Son el otro medio contador del estado narrativo, junto a las banderas.
// Una bandera sabe decir si algo paso; no sabe decir "3 de 5 hierbas" ni
// "este cofre concreto ya se abrio, pero el de al lado no".

namespace {

using RPG::NarrativeState;


MOTOR_TEST_CASE(una_variable_que_nadie_ha_tocado_vale_cero_y_no_existe) {
    // Leer no crea estado. Si lo creara, el guardado engordaria con cada
    // condicion consultada y un nombre mal escrito se convertiria en una
    // variable nueva y silenciosa en vez de un cero visible.
    NarrativeState estado;
    MOTOR_CHECK(estado.variable("hierbas") == 0);
    MOTOR_CHECK(!estado.hasVariable("hierbas"));
    MOTOR_CHECK(estado.variables().empty());

    estado.setVariable("hierbas", 3);
    MOTOR_CHECK(estado.variable("hierbas") == 3);
    MOTOR_CHECK(estado.hasVariable("hierbas"));

    estado.clearVariable("hierbas");
    MOTOR_CHECK(!estado.hasVariable("hierbas"));
    MOTOR_CHECK(estado.variable("hierbas") == 0);
}

MOTOR_TEST_CASE(sumar_devuelve_el_valor_nuevo_y_satura_en_vez_de_desbordar) {
    NarrativeState estado;
    MOTOR_CHECK(estado.addToVariable("hierbas", 1) == 1);
    MOTOR_CHECK(estado.addToVariable("hierbas", 4) == 5);
    MOTOR_CHECK(estado.addToVariable("hierbas", -2) == 3);

    // Un contador que da la vuelta a negativo por un evento que se repite
    // mas veces de las previstas es un bug que solo se ve en la partida
    // de alguien y no hay forma de reproducir.
    estado.setVariable("tope", std::numeric_limits<int>::max());
    MOTOR_CHECK(estado.addToVariable("tope", 100) == std::numeric_limits<int>::max());
    estado.setVariable("suelo", std::numeric_limits<int>::min());
    MOTOR_CHECK(estado.addToVariable("suelo", -100) == std::numeric_limits<int>::min());
}

MOTOR_TEST_CASE(un_nombre_vacio_se_ignora_porque_no_hay_forma_de_releerlo) {
    NarrativeState estado;
    estado.setVariable("", 7);
    MOTOR_CHECK(estado.variables().empty());
    MOTOR_CHECK(estado.addToVariable("", 7) == 0);
    MOTOR_CHECK(estado.variables().empty());
}

MOTOR_TEST_CASE(dos_eventos_iguales_llevan_su_propia_cuenta) {
    // Es la razon de ser de los interruptores locales: sin ellos, el
    // autor tiene que inventar cofre_17_abierto para cada instancia.
    NarrativeState estado;
    MOTOR_CHECK(estado.setLocalSwitch("cofre_norte", 'A', true).isOk());

    MOTOR_CHECK(estado.localSwitch("cofre_norte", 'A'));
    MOTOR_CHECK(!estado.localSwitch("cofre_sur", 'A'));
    MOTOR_CHECK(!estado.localSwitch("cofre_norte", 'B'));

    // Minuscula vale: el autor escribe 'a' tan a menudo como 'A'.
    MOTOR_CHECK(estado.localSwitch("cofre_norte", 'a'));
    MOTOR_CHECK(estado.setLocalSwitch("cofre_sur", 'd', true).isOk());
    MOTOR_CHECK(estado.localSwitch("cofre_sur", 'D'));
}

MOTOR_TEST_CASE(una_letra_que_no_es_A_D_da_error_en_vez_de_no_hacer_nada) {
    // Un no-op silencioso se manifiesta como una puerta que no se abre
    // tres horas despues. El error tiene que verse en el editor.
    NarrativeState estado;
    for (char letra : {'E', 'Z', '1', '\0'}) {
        const Result<bool> r = estado.setLocalSwitch("cofre", letra, true);
        MOTOR_CHECK(!r.isOk());
    }
    MOTOR_CHECK(!estado.setLocalSwitch("", 'A', true).isOk());
    MOTOR_CHECK(estado.localSwitches().empty());
}

MOTOR_TEST_CASE(un_evento_sin_interruptores_encendidos_no_ocupa_sitio) {
    NarrativeState estado;
    MOTOR_CHECK(estado.setLocalSwitch("cofre", 'A', true).isOk());
    MOTOR_CHECK(estado.setLocalSwitch("cofre", 'C', true).isOk());
    MOTOR_CHECK(estado.localSwitches().size() == 1);
    MOTOR_CHECK(estado.localSwitches()[0].on.size() == 2);
    MOTOR_CHECK(estado.localSwitches()[0].on[0] == 'A');
    MOTOR_CHECK(estado.localSwitches()[0].on[1] == 'C');

    MOTOR_CHECK(estado.setLocalSwitch("cofre", 'A', false).isOk());
    MOTOR_CHECK(estado.setLocalSwitch("cofre", 'C', false).isOk());
    MOTOR_CHECK(estado.localSwitches().empty());

    // Apagar uno que ya estaba apagado no es un error.
    MOTOR_CHECK(estado.setLocalSwitch("cofre", 'B', false).isOk());
    MOTOR_CHECK(estado.localSwitches().empty());
}

MOTOR_TEST_CASE(reset_borra_tambien_lo_nuevo) {
    // Si esto se olvida, empezar partida nueva hereda los contadores de
    // la anterior y el bug aparece dos horas despues.
    NarrativeState estado;
    estado.setFlag("prologo");
    estado.setVariable("hierbas", 4);
    MOTOR_CHECK(estado.setLocalSwitch("cofre", 'A', true).isOk());

    estado.reset();
    MOTOR_CHECK(!estado.hasFlag("prologo"));
    MOTOR_CHECK(estado.variables().empty());
    MOTOR_CHECK(estado.localSwitches().empty());
}

// H2-01 — La ultima prueba de un beat, para la hoja del dialogo: el estado
// la numera (lo que pone quien la tira no cuenta) y reset() se la lleva sin
// volver a contar desde cero, para que la primera tirada de la partida nueva
// no se confunda con una vieja ya vista.
MOTOR_TEST_CASE(la_tirada_del_beat_se_numera_y_reset_no_vuelve_a_cero) {
    NarrativeState estado;
    MOTOR_CHECK(estado.ultimaTirada().numero == 0);
    RPG::TiradaVisible t;
    t.que = "Percepcion";
    t.numero = 99;
    estado.ponTirada(t);
    MOTOR_CHECK(estado.ultimaTirada().numero == 1 && estado.ultimaTirada().que == "Percepcion");
    estado.ponTirada(t);
    MOTOR_CHECK(estado.ultimaTirada().numero == 2);
    estado.reset();
    MOTOR_CHECK(estado.ultimaTirada().numero == 2 && estado.ultimaTirada().que.empty());
    estado.ponTirada(t);
    MOTOR_CHECK(estado.ultimaTirada().numero == 3);
}

MOTOR_TEST_CASE(variables_e_interruptores_sobreviven_a_guardar_y_cargar) {
    // "Tot va al guardat" es el criterio de cierre de E-02. Sin esta
    // prueba es una frase: el guardado podria llevarlos y la carga
    // tirarlos, y el contador de hierbas volveria a cero al continuar.
    SaveGameData datos;
    datos.levelPath = "assets/levels/plaza.json";
    datos.narrativeFlags = {"prologo_visto"};
    datos.narrativeVariables = {{"hierbas", 3}, {"reputacion", -12}};
    RPG::NarrativeState::LocalSwitches cofre;
    cofre.eventId = "cofre_norte";
    cofre.on = {'A', 'C'};
    datos.narrativeLocalSwitches.push_back(cofre);

    const std::string json = serializeSaveGame(datos);
    SaveGameData vuelta;
    std::string error;
    MOTOR_CHECK(parseSaveGame(json, &vuelta, &error));
    MOTOR_CHECK(error.empty());

    MOTOR_CHECK(vuelta.narrativeVariables.size() == 2);
    MOTOR_CHECK(vuelta.narrativeVariables.at("hierbas") == 3);
    MOTOR_CHECK(vuelta.narrativeVariables.at("reputacion") == -12);
    MOTOR_CHECK(vuelta.narrativeLocalSwitches.size() == 1);
    MOTOR_CHECK(vuelta.narrativeLocalSwitches[0].eventId == "cofre_norte");
    MOTOR_CHECK(vuelta.narrativeLocalSwitches[0].on.size() == 2);
    MOTOR_CHECK(vuelta.narrativeLocalSwitches[0].on[0] == 'A');
    MOTOR_CHECK(vuelta.narrativeLocalSwitches[0].on[1] == 'C');

    // Ida y vuelta ESTABLE: guardar dos veces da el mismo fichero. Si no,
    // cada guardado ensucia el diff y no se puede comparar dos partidas.
    MOTOR_CHECK(serializeSaveGame(vuelta) == json);
}

MOTOR_TEST_CASE(un_guardado_anterior_a_E_02_se_sigue_pudiendo_cargar) {
    // No se sube schemaVersion: el lector rechaza cualquier version que
    // no sea la suya, asi que subirla dejaria ilegibles de golpe todas
    // las partidas guardadas. Las dos claves nuevas son OPCIONALES al
    // leer, y esto es un fichero tal y como lo escribia el motor ANTES
    // de E-02 — sin "variables" y sin "localSwitches".
    const std::string antiguo = R"({
  "schemaVersion": 1,
  "levelPath": "assets/levels/plaza.json",
  "playerPosition": { "x": 2, "y": 5 },
  "playerHealth": 30,
  "playerMaxHealth": 60,
  "inventory": ["pocion"],
  "gold": 40,
  "morality": -3,
  "stepsSinceRent": 7,
  "businesses": [],
  "worldObjects": [],
  "enemies": [],
  "narrative": {
    "flags": ["prologo_visto"],
    "beatsFired": [],
    "timers": []
  }
})";

    SaveGameData datos;
    std::string error;
    MOTOR_CHECK(parseSaveGame(antiguo, &datos, &error));
    MOTOR_CHECK(error.empty());
    // Entra exactamente como antes...
    MOTOR_CHECK(datos.narrativeFlags.size() == 1);
    MOTOR_CHECK(datos.gold == 40);
    MOTOR_CHECK(datos.playerHealth == 30);
    // ...y lo nuevo llega vacio, no a medias ni con basura.
    MOTOR_CHECK(datos.narrativeVariables.empty());
    MOTOR_CHECK(datos.narrativeLocalSwitches.empty());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
