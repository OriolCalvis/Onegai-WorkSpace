// H3-06 (Gantt Fase H) — temporizadores de mision visibles en el HUD.
//
// Dos mitades, a proposito:
//
//   1. Casos con TimerState/TimerDefinition ESCRITOS A MANO: las reglas
//      de presentacion (contador M:SS, barra proporcional, secreto que no
//      se dibuja, inactivos/expirados que se saltan, id sin definicion,
//      barra sin total) sobre datos que decide el test.
//   2. Casos de INTEGRACION con el motor real: un AdventureScript con un
//      timer de verdad arranca por flag, avanza con
//      NarrativeEngine::tickTimers, SE CONGELA con dt=0 (combate) y expira
//      con su beat; el widget ensena los segundos que el motor descuenta.
//
// GL-free: sin ventana, sin assets, sin copiar_assets.
#include "TestHarness.h"

#include "HUD/MissionTimerWidget.h"
#include "RPG/NarrativeEngine.h"

#include <string>
#include <vector>

using Game::MissionTimerWidget;
using RPG::AdventureScript;
using RPG::NarrativeEngine;
using RPG::NarrativeState;
using RPG::TimerDefinition;
using RPG::TimerState;

namespace {

TimerDefinition defCon(const char* id, float duracion,
                       TimerDefinition::Presentacion modo) {
    TimerDefinition def;
    def.id = id;
    def.duracion = duracion;
    def.presentacion = modo;
    def.expiraPorTiempo = true;
    return def;
}

TimerState estado(const char* id, float restante, bool activo = true,
                  bool expirado = false) {
    TimerState state;
    state.id = id;
    state.restante = restante;
    state.activo = activo;
    state.expirado = expirado;
    return state;
}

bool contiene(const std::vector<std::string>& lineas, const std::string& trozo) {
    for (const std::string& l : lineas) {
        if (l.find(trozo) != std::string::npos) {
            return true;
        }
    }
    return false;
}

}  // namespace

MOTOR_TEST_CASE(el_contador_ensena_minutos_y_segundos) {
    const std::vector<TimerDefinition> defs = {
        defCon("huida", 120.0f, TimerDefinition::Presentacion::Contador)};
    MissionTimerWidget w;
    w.show({estado("huida", 90.0f)}, defs);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(w.visible());
    MOTOR_CHECK(w.shownCount() == 1);
    MOTOR_CHECK(lineas.size() == 1);
    MOTOR_CHECK(lineas[0] == "huida: 1:30");
}

MOTOR_TEST_CASE(los_segundos_llevan_cero_delante_y_trunca) {
    const std::vector<TimerDefinition> defs = {
        defCon("huida", 120.0f, TimerDefinition::Presentacion::Contador)};
    MissionTimerWidget w;
    w.show({estado("huida", 5.0f)}, defs);
    MOTOR_CHECK(contiene(w.displayLines(), "0:05"));
    // 89.9 s son "1:29": lo que se ve nunca promete mas de lo que queda.
    w.show({estado("huida", 89.9f)}, defs);
    MOTOR_CHECK(contiene(w.displayLines(), "1:29"));
}

MOTOR_TEST_CASE(la_barra_es_proporcional_al_total) {
    const std::vector<TimerDefinition> defs = {
        defCon("huida", 120.0f, TimerDefinition::Presentacion::Barra)};
    MissionTimerWidget w;
    // 45/120 = 0.375 -> 3 de 10.
    w.show({estado("huida", 45.0f)}, defs);
    MOTOR_CHECK(contiene(w.displayLines(), "[###-------]"));
    MOTOR_CHECK(contiene(w.displayLines(), "0:45"));
    w.show({estado("huida", 120.0f)}, defs);
    MOTOR_CHECK(contiene(w.displayLines(), "[##########]"));
    w.show({estado("huida", 0.0f)}, defs);
    MOTOR_CHECK(contiene(w.displayLines(), "[----------]"));
}

MOTOR_TEST_CASE(lo_secreto_no_se_dibuja) {
    const std::vector<TimerDefinition> defs = {
        defCon("ritual", 300.0f, TimerDefinition::Presentacion::Secreto)};
    MissionTimerWidget w;
    w.show({estado("ritual", 299.0f)}, defs);
    MOTOR_CHECK(w.visible());
    MOTOR_CHECK(w.shownCount() == 0);
    MOTOR_CHECK(w.displayLines().empty());
}

MOTOR_TEST_CASE(inactivos_y_expirados_se_saltan) {
    const std::vector<TimerDefinition> defs = {
        defCon("huida", 120.0f, TimerDefinition::Presentacion::Contador),
        defCon("viejo", 60.0f, TimerDefinition::Presentacion::Contador)};
    MissionTimerWidget w;
    w.show({estado("huida", 90.0f), estado("viejo", 60.0f, false),
            estado("fin", 0.0f, false, true)},
           defs);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 1);
    MOTOR_CHECK(contiene(lineas, "huida"));
}

MOTOR_TEST_CASE(un_id_sin_definicion_sale_como_contador) {
    const std::vector<TimerDefinition> defs;  // vacio a proposito
    MissionTimerWidget w;
    w.show({estado("misterio", 60.0f)}, defs);
    MOTOR_CHECK(contiene(w.displayLines(), "misterio: 1:00"));
}

MOTOR_TEST_CASE(barra_sin_total_cae_a_contador) {
    std::vector<TimerDefinition> defs = {
        defCon("zona", 0.0f, TimerDefinition::Presentacion::Barra)};
    MissionTimerWidget w;
    // Sin total no hay fraccion que pintar: mejor el numero que nada.
    w.show({estado("zona", 0.0f)}, defs);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 1);
    MOTOR_CHECK(!contiene(lineas, "["));
    MOTOR_CHECK(contiene(lineas, "0:00"));
}

MOTOR_TEST_CASE(el_orden_es_el_del_estado_y_mostrar_sustituye) {
    const std::vector<TimerDefinition> defs = {
        defCon("b_dos", 60.0f, TimerDefinition::Presentacion::Contador),
        defCon("a_uno", 60.0f, TimerDefinition::Presentacion::Contador)};
    MissionTimerWidget w;
    w.show({estado("b_dos", 60.0f), estado("a_uno", 59.0f)}, defs);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 2);
    MOTOR_CHECK(lineas[0].find("b_dos") != std::string::npos);
    w.show({estado("a_uno", 10.0f)}, defs);
    MOTOR_CHECK(w.displayLines().size() == 1);
    MOTOR_CHECK(!contiene(w.displayLines(), "b_dos"));
}

MOTOR_TEST_CASE(ocultar_apaga_aunque_haya_tiempo) {
    const std::vector<TimerDefinition> defs = {
        defCon("huida", 120.0f, TimerDefinition::Presentacion::Contador)};
    MissionTimerWidget w;
    MOTOR_CHECK(!w.visible());
    MOTOR_CHECK(w.displayLines().empty());
    w.show({estado("huida", 90.0f)}, defs);
    w.hide();
    MOTOR_CHECK(!w.visible());
    MOTOR_CHECK(w.shownCount() == 0);
    MOTOR_CHECK(w.displayLines().empty());
}

MOTOR_TEST_CASE(el_tiempo_que_ensena_lo_descuenta_el_motor) {
    // Integracion: timer de verdad (120 s, Contador, arranca por flag).
    AdventureScript script;
    script.id = "mision_huida";
    TimerDefinition def = defCon("huida", 120.0f, TimerDefinition::Presentacion::Contador);
    def.arrancaPor = "empieza_huida";
    def.flagExpira = "huida_acabada";
    script.timers.push_back(def);
    NarrativeEngine engine;
    engine.setAdventure(&script);
    NarrativeState state;
    state.setFlag("empieza_huida");

    MissionTimerWidget w;
    engine.tickTimers(state, 30.0f, "nivel_x");
    w.show(state.timers(), script.timers);
    MOTOR_CHECK(contiene(w.displayLines(), "huida: 1:30"));

    // Congelacion: con dt=0 (combate/dialogo/pausa/AFK) no baja.
    engine.tickTimers(state, 0.0f, "nivel_x");
    w.show(state.timers(), script.timers);
    MOTOR_CHECK(contiene(w.displayLines(), "huida: 1:30"));

    // Expiracion: agota los 90 que quedan y la fila desaparece (el beat
    // Auto con flagExpira es quien pone las consecuencias, no el widget).
    engine.tickTimers(state, 90.0f, "nivel_x");
    w.show(state.timers(), script.timers);
    MOTOR_CHECK(!contiene(w.displayLines(), "huida"));
    MOTOR_CHECK(state.hasFlag("huida_acabada"));
}

MOTOR_TEST_CASE(la_barra_del_motor_baja_sola) {
    // Integracion en modo Barra: 45 s gastados de 120 -> 3 de 10.
    AdventureScript script;
    script.id = "mision_huida";
    TimerDefinition def = defCon("huida", 120.0f, TimerDefinition::Presentacion::Barra);
    def.arrancaPor = "empieza_huida";
    script.timers.push_back(def);
    NarrativeEngine engine;
    engine.setAdventure(&script);
    NarrativeState state;
    state.setFlag("empieza_huida");

    MissionTimerWidget w;
    engine.tickTimers(state, 75.0f, "nivel_x");
    w.show(state.timers(), script.timers);
    MOTOR_CHECK(contiene(w.displayLines(), "[###-------]"));
    MOTOR_CHECK(contiene(w.displayLines(), "0:45"));
}

// H3-06 — La regla que pinta el pergamino es la misma que la de las lineas
// de texto: relojesVisibles() deja fuera lo inactivo, lo expirado y lo
// secreto, y dice lo que queda, el total y si va en barra.
MOTOR_TEST_CASE(relojes_visibles_es_la_misma_regla_que_las_lineas) {
    const std::vector<TimerDefinition> defs = {
        defCon("huida", 120.0f, TimerDefinition::Presentacion::Barra),
        defCon("ritual", 60.0f, TimerDefinition::Presentacion::Contador),
        defCon("trampa", 30.0f, TimerDefinition::Presentacion::Secreto),
        defCon("sin_total", 0.0f, TimerDefinition::Presentacion::Barra)};
    const std::vector<TimerState> estados = {estado("huida", 45.0f),       estado("ritual", 10.0f),
                                             estado("trampa", 5.0f),       estado("sin_total", 9.0f),
                                             estado("parado", 50.0f, false), estado("acabado", 0.0f, true, true),
                                             estado("suelto", 70.0f)};
    const std::vector<Game::RelojVisible> r = Game::relojesVisibles(estados, defs);
    MOTOR_CHECK(r.size() == 4);  // huida, ritual, sin_total y suelto
    MOTOR_CHECK(r[0].id == "huida" && r[0].barra && r[0].restante == 45.0f && r[0].total == 120.0f);
    MOTOR_CHECK(r[1].id == "ritual" && !r[1].barra && r[1].total == 60.0f);
    MOTOR_CHECK(r[2].id == "sin_total" && !r[2].barra);
    MOTOR_CHECK(r[3].id == "suelto" && !r[3].barra && r[3].total == 0.0f);
    MOTOR_CHECK(Game::formatoReloj(70.9f) == "1:10" && Game::formatoReloj(-3.0f) == "0:00");
    MissionTimerWidget w;
    w.show(estados, defs);
    MOTOR_CHECK(w.shownCount() == r.size());
}

// Oriol (05/10): el campo «nombre» opcional. Con nombre, la linea y el
// pergamino ensenan el nombre; sin el, como siempre (el id en las lineas,
// «TIEMPO» en el pergamino).
MOTOR_TEST_CASE(el_nombre_sustituye_al_id_cuando_lo_hay) {
    std::vector<TimerDefinition> defs = {
        defCon("incendio", 120.0f, TimerDefinition::Presentacion::Barra),
        defCon("ritual", 60.0f, TimerDefinition::Presentacion::Contador)};
    defs[0].nombre = "El incendio";
    MissionTimerWidget w;
    w.show({estado("incendio", 45.0f), estado("ritual", 30.0f)}, defs);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 2);
    MOTOR_CHECK(lineas[0] == "El incendio: [###-------] 0:45");
    MOTOR_CHECK(lineas[1] == "ritual: 0:30");
    const std::vector<Game::RelojVisible> r = Game::relojesVisibles({estado("incendio", 45.0f)}, defs);
    MOTOR_CHECK(r.size() == 1 && r[0].nombre == "El incendio" && r[0].id == "incendio");
}

MOTOR_TEST_CASE(el_nombre_se_lee_y_solo_se_escribe_si_lo_hay) {
    const char* json = R"({
      "id": "mision_incendio",
      "beats": [],
      "timers": [
        {"id": "incendio", "nombre": "El incendio", "duracion": 90, "arrancaPor": "fuego",
         "flagExpira": "fuego_fin"},
        {"id": "ritual", "duracion": 60, "arrancaPor": "velas", "flagExpira": "ritual_fin"}
      ]
    })";
    const auto leida = AdventureScript::loadFromString(json);
    MOTOR_CHECK(leida.isOk());
    if (!leida.isOk()) return;
    const AdventureScript& a = leida.value();
    MOTOR_CHECK(a.timers.size() == 2);
    MOTOR_CHECK(a.timers[0].nombre == "El incendio");
    MOTOR_CHECK(a.timers[1].nombre.empty());
    // De ida y vuelta, igual; y la clave solo sale en el que la tiene.
    const std::string texto = RPG::serializeAdventure(a);
    std::size_t veces = 0;
    for (std::size_t pos = texto.find("\"nombre\""); pos != std::string::npos; pos = texto.find("\"nombre\"", pos + 1))
        ++veces;
    MOTOR_CHECK(veces == 1);
    const auto otra = AdventureScript::loadFromString(texto);
    MOTOR_CHECK(otra.isOk() && otra.value().timers[0].nombre == "El incendio");
    MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == texto);
}

int main() { return motor_test::runAllTests(); }
