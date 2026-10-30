// OPCIONES-01 y VARIABLES-01 — Opciones en el dialogo y variables en las
// aventuras (fase 1 de «Los Perdidos de Boundington — revisión para jugarla
// entera»: «Opciones en el diálogo» y «Variables: puntos y reputación»).
//
// Se prueba: la lectura (lo que vale y lo que no), el motor (que opciones se
// ofrecen, que pasa al elegir, las variables con sus topes y sus rangos), la
// escritura de vuelta (idempotente), la validacion, la sesion (el dialogo no
// se cierra con una eleccion pendiente; con lineas se leen, sin lineas se
// cierra y la aventura sigue) y la hoja del dialogo (la elegida, dando la
// vuelta). GL-free.

#include "TestHarness.h"

#include "Game/GameSession.h"
#include "HUD/Pergamino/DialogoHoja.h"
#include "Level/ObjectCatalog.h"
#include "RPG/AdventureSerializer.h"
#include "RPG/AdventureValidation.h"
#include "RPG/NarrativeEngine.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

// La escena de Sandes, en pequeño: tres respuestas, una que solo sale con el
// afecto de Venides, y lo que viene detras de cada una (por sus flags).
const char* kAventura = R"({
  "id": "prueba_opciones",
  "beats": [
    {"id": "sandes", "trigger": {"type": "talk", "target": "sandes"}, "speaker": "Sandes",
     "lines": ["Dame a la chica y te dejo ir."],
     "effects": [{"type": "setFlag", "arg": "sandes_hablado"}],
     "opciones": [
       {"texto": "Entregarle a Verina", "lines": ["Sabia que eras listo."],
        "effects": [{"type": "setFlag", "arg": "verina_entregada"},
                    {"type": "addVariable", "arg": "puntos_gurkazaal", "value": 1}]},
       {"texto": "Negarte", "speaker": "",
        "effects": [{"type": "setFlag", "arg": "sandes_negado"},
                    {"type": "addVariable", "arg": "afecto_venides", "value": 5, "min": 0, "max": 3}]},
       {"texto": "Pedirle a Venides que hable",
        "requires": {"variables": {"afecto_venides": {"min": 1}}},
        "effects": [{"type": "setVariable", "arg": "afecto_venides", "value": 0}]}
     ]},
    {"id": "tras_negarte", "trigger": {"type": "auto"}, "requires": {"allFlags": ["sandes_negado"],
     "notFlags": ["tras_negarte_dicho"]},
     "lines": ["Sandes saca el cuchillo."], "effects": [{"type": "setFlag", "arg": "tras_negarte_dicho"}]},
    {"id": "con_afecto", "trigger": {"type": "talk", "target": "venides"},
     "requires": {"variables": {"afecto_venides": 3}},
     "lines": ["Gracias por no venderla."]}
  ]
})";

RPG::AdventureScript carga(const char* texto = kAventura) {
    auto r = RPG::AdventureScript::loadFromString(texto);
    MOTOR_CHECK(r.isOk());
    return r.isOk() ? r.value() : RPG::AdventureScript{};
}

std::string conBeat(const std::string& beat) {
    return R"({"id": "x", "beats": [)" + beat + "]}";
}

class Cuerpo : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }
};

}  // namespace

MOTOR_TEST_CASE(se_leen_las_opciones_y_las_variables) {
    const RPG::AdventureScript a = carga();
    MOTOR_CHECK(a.beats.size() == 3);
    const RPG::NarrativeBeat& b = a.beats[0];
    MOTOR_CHECK(b.opciones.size() == 3);
    MOTOR_CHECK(b.opciones[0].texto == "Entregarle a Verina");
    MOTOR_CHECK(b.opciones[0].speaker == "Sandes");  // sin "speaker": el del beat
    MOTOR_CHECK(b.opciones[1].speaker.empty());       // "" a proposito: el narrador
    MOTOR_CHECK(b.opciones[1].efectos.size() == 2 && b.opciones[1].efectos[1].max == 3);
    MOTOR_CHECK(b.opciones[2].requisitos.variables.size() == 1);
    MOTOR_CHECK(b.opciones[2].requisitos.variables[0].min == 1);
    MOTOR_CHECK(a.beats[2].requirements.variables[0].min == 3 && a.beats[2].requirements.variables[0].max == 3);
}

MOTOR_TEST_CASE(lo_que_no_vale_da_error_con_nombre) {
    const auto mal = [](const std::string& beat) { return !RPG::AdventureScript::loadFromString(conBeat(beat)).isOk(); };
    // Opciones sin pregunta, sin texto, con texto largo, o mas de cuatro.
    MOTOR_CHECK(mal(R"({"id": "a", "trigger": {"type": "auto"}, "opciones": [{"texto": "Si"}]})"));
    MOTOR_CHECK(mal(R"({"id": "a", "trigger": {"type": "auto"}, "lines": ["?"], "opciones": [{"texto": ""}]})"));
    MOTOR_CHECK(mal(R"({"id": "a", "trigger": {"type": "auto"}, "lines": ["?"], "opciones": [{"texto": ")" +
                    std::string(61, 'a') + R"("}]})"));
    MOTOR_CHECK(mal(R"({"id": "a", "trigger": {"type": "auto"}, "lines": ["?"], "opciones": [{"texto": "1"},
                    {"texto": "2"}, {"texto": "3"}, {"texto": "4"}, {"texto": "5"}]})"));
    // Sesenta letras con tildes caben (se cuentan letras, no bytes).
    std::string largo;
    for (int i = 0; i < 60; ++i) largo += "á";
    MOTOR_CHECK(!mal(R"({"id": "a", "trigger": {"type": "auto"}, "lines": ["?"], "opciones": [{"texto": ")" + largo +
                     R"("}]})"));
    // Variables: sin nombre, rangos al reves, efectos sin arg o con topes al reves.
    MOTOR_CHECK(mal(R"({"id": "a", "trigger": {"type": "auto"}, "requires": {"variables": {"x": {"min": 3, "max": 1}}}})"));
    MOTOR_CHECK(mal(R"({"id": "a", "trigger": {"type": "auto"}, "requires": {"variables": {"x": "mucho"}}})"));
    MOTOR_CHECK(mal(R"({"id": "a", "trigger": {"type": "auto"}, "effects": [{"type": "setVariable", "value": 1}]})"));
    MOTOR_CHECK(mal(R"({"id": "a", "trigger": {"type": "auto"}, "effects": [{"type": "addVariable", "arg": "x",
                    "value": 1, "min": 4, "max": 0}]})"));
    // Un error dentro de una opcion dice donde.
    const auto r = RPG::AdventureScript::loadFromString(conBeat(
        R"({"id": "a", "trigger": {"type": "auto"}, "lines": ["?"], "opciones": [{"texto": "Si",
            "effects": [{"type": "volar"}]}]})"));
    MOTOR_CHECK(!r.isOk() && r.errorMessage().find("opciones[0].effects[0]") != std::string::npos);
}

MOTOR_TEST_CASE(el_motor_ofrece_las_que_cumplen_y_aplica_la_elegida) {
    const RPG::AdventureScript a = carga();
    RPG::NarrativeEngine motor;
    motor.setAdventure(&a);
    RPG::NarrativeState estado;

    const RPG::NarrativeResult r = motor.talkTo("sandes", estado);
    MOTOR_CHECK(r.fired && r.lines.size() == 1);
    MOTOR_CHECK(estado.hasFlag("sandes_hablado"));  // los efectos del beat, antes de elegir
    // Sin afecto, la tercera no sale.
    MOTOR_CHECK(estado.eleccion().activa());
    MOTOR_CHECK(estado.eleccion().textos == (std::vector<std::string>{"Entregarle a Verina", "Negarte"}));
    // Una que no esta: nada cambia.
    MOTOR_CHECK(!motor.elige(2, estado).fired && estado.eleccion().activa());

    // Negarte: sin lineas; el afecto sube 5 pero se queda en el tope (3).
    const RPG::NarrativeResult negarte = motor.elige(1, estado);
    MOTOR_CHECK(negarte.fired && negarte.lines.empty());
    MOTOR_CHECK(!negarte.log.empty() && negarte.log[0] == "Eliges: Negarte.");
    MOTOR_CHECK(estado.hasFlag("sandes_negado") && !estado.hasFlag("verina_entregada"));
    MOTOR_CHECK(estado.variable("afecto_venides") == 3);
    MOTOR_CHECK(!estado.eleccion().activa());
    // Lo que viene detras sale por su flag; y la condicion por variable se cumple.
    MOTOR_CHECK(motor.tick(estado).beatId == "tras_negarte");
    MOTOR_CHECK(motor.talkTo("venides", estado).beatId == "con_afecto");

    // Otra vez Sandes, ya con afecto: salen las tres, y la tercera lo gasta.
    MOTOR_CHECK(motor.talkTo("sandes", estado).fired);
    MOTOR_CHECK(estado.eleccion().textos.size() == 3);
    MOTOR_CHECK(motor.elige(2, estado).fired);
    MOTOR_CHECK(estado.variable("afecto_venides") == 0);
    MOTOR_CHECK(!motor.talkTo("venides", estado).fired);

    // Entregarla: con lineas y quien las dice (el del beat).
    MOTOR_CHECK(motor.talkTo("sandes", estado).fired);
    const RPG::NarrativeResult entregar = motor.elige(0, estado);
    MOTOR_CHECK(entregar.speaker == "Sandes" && entregar.lines.size() == 1);
    MOTOR_CHECK(estado.variable("puntos_gurkazaal") == 1);
}

MOTOR_TEST_CASE(se_escriben_de_vuelta_igual) {
    const RPG::AdventureScript a = carga();
    const std::string una = RPG::serializeAdventure(a);
    auto otra = RPG::AdventureScript::loadFromString(una);
    MOTOR_CHECK(otra.isOk());
    if (!otra.isOk()) return;
    MOTOR_CHECK(RPG::serializeAdventure(otra.value()) == una);
    MOTOR_CHECK(otra.value().beats[0].opciones.size() == 3);
    MOTOR_CHECK(otra.value().beats[0].opciones[1].efectos[1].min == 0);
    MOTOR_CHECK(otra.value().beats[2].requirements.variables[0].max == 3);
}

MOTOR_TEST_CASE(la_validacion_mira_dentro_de_las_opciones) {
    const RPG::ValidationReport bien = RPG::ValidateAdventure(carga());
    // sandes_negado solo lo pone una opcion: no es una bandera huerfana.
    for (const RPG::Diagnostic& d : bien.diagnostics)
        MOTOR_CHECK(d.message.find("sandes_negado") == std::string::npos);
    // Una variable que se pide y nadie escribe: aviso.
    const RPG::ValidationReport mal = RPG::ValidateAdventure(carga(R"({"id": "y", "beats": [
        {"id": "a", "trigger": {"type": "auto"}, "requires": {"variables": {"reputacion": {"min": 2}}},
         "lines": ["Hola."]}]})"));
    bool aviso = false;
    for (const RPG::Diagnostic& d : mal.diagnostics) aviso = aviso || d.message.find("'reputacion'") != std::string::npos;
    MOTOR_CHECK(aviso);
}

MOTOR_TEST_CASE(la_sesion_no_cierra_hasta_elegir_y_sigue_la_aventura) {
    const RPG::AdventureScript a = carga();
    RPG::NarrativeEngine motor;
    motor.setAdventure(&a);
    RPG::NarrativeState estado;
    ObjectCatalog catalogo;
    SkillCatalog cartas;
    SkillSet habilidades{10};
    std::vector<std::string> mochila;
    Cuerpo cuerpo;
    LevelDefinition nivel;
    nivel.name = "Callejon";
    GameSession sesion(nullptr, &catalogo, &cartas, nivel, &cuerpo, &habilidades, &mochila);
    sesion.setNarrative(&motor, &estado);

    MOTOR_CHECK(!sesion.chooseOption(0));  // sin dialogo, nada que elegir
    MOTOR_CHECK(sesion.talkNarrative("sandes"));
    MOTOR_CHECK(sesion.mode() == GameMode::Dialogue);
    sesion.closeInteraction();  // con la eleccion pendiente no se cierra
    MOTOR_CHECK(sesion.mode() == GameMode::Dialogue);

    // Entregarla: trae lineas, se leen; al cerrar, la aventura no tiene nada mas.
    MOTOR_CHECK(sesion.chooseOption(0));
    MOTOR_CHECK(sesion.mode() == GameMode::Dialogue);
    MOTOR_CHECK(sesion.dialogueSpeaker() == "Sandes");
    MOTOR_CHECK(sesion.dialogueLines() == std::vector<std::string>{"Sabia que eras listo."});
    MOTOR_CHECK(std::find(sesion.log().begin(), sesion.log().end(), "Eliges: Entregarle a Verina.") != sesion.log().end());
    sesion.closeInteraction();
    MOTOR_CHECK(sesion.mode() == GameMode::Exploration);

    // Negarse: sin lineas, el dialogo se cierra solo y salta lo que viene detras.
    MOTOR_CHECK(sesion.talkNarrative("sandes"));
    MOTOR_CHECK(sesion.chooseOption(1));
    MOTOR_CHECK(sesion.mode() == GameMode::Dialogue);
    MOTOR_CHECK(sesion.dialogueLines() == std::vector<std::string>{"Sandes saca el cuchillo."});
    MOTOR_CHECK(!estado.eleccion().activa());
}

MOTOR_TEST_CASE(la_hoja_del_dialogo_lleva_la_elegida_y_da_la_vuelta) {
    Pergamino::DialogoHoja hoja;
    MOTOR_CHECK(!hoja.conOpciones());
    hoja.miraOpciones({"Entregarle a Verina", "Negarte", "Pedirle a Venides que hable"});
    MOTOR_CHECK(hoja.conOpciones() && hoja.opcion() == 0);
    hoja.mueveOpcion(-1);
    MOTOR_CHECK(hoja.opcion() == 2);  // de la primera, arriba, a la ultima
    hoja.mueveOpcion(+1);
    MOTOR_CHECK(hoja.opcion() == 0);
    hoja.mueveOpcion(+1);
    // Las mismas opciones otro fotograma: la elegida se queda.
    hoja.miraOpciones({"Entregarle a Verina", "Negarte", "Pedirle a Venides que hable"});
    MOTOR_CHECK(hoja.opcion() == 1);
    // Otras: vuelve a la primera. Y cerrar las quita.
    hoja.miraOpciones({"Si", "No"});
    MOTOR_CHECK(hoja.opcion() == 0);
    hoja.cierra();
    MOTOR_CHECK(!hoja.conOpciones());
}

int main() { return motor_test::runAllTests(); }
