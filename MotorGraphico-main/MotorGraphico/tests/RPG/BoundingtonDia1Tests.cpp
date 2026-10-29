// Dia 1 de Boundington: el encargo de Venides en el desayuno y el objetivo de explorar
// la plaza (E-G4-05). Guion, diario y fichas reales (assets del directorio de build).

#include "RPG/NarrativeEngine.h"
#include "Game/DiarioMisiones.h"

#include "TestHarness.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

using RPG::AdventureScript;
using RPG::NarrativeEngine;
using RPG::NarrativeResult;
using RPG::NarrativeState;

const char* const kPlaza = "assets/levels/ciudad_centro.json";
const char* const kTaberna = "assets/levels/interior_taberna.json";
const char* const kOtro = "assets/levels/interior_herreria.json";

AdventureScript carga() {
    const auto r = AdventureScript::loadFromFile("assets/adventures/boundington_primer_dia.json");
    MOTOR_CHECK(r.isOk());
    return r.value();
}

NarrativeEngine motor(const AdventureScript& guion) {
    NarrativeEngine e;
    e.setAdventure(&guion);
    return e;
}

std::vector<std::string> drena(const NarrativeEngine& e, NarrativeState& s) {
    std::vector<std::string> ids;
    for (int i = 0; i < 100; ++i) {
        const NarrativeResult r = e.tick(s);
        if (!r.fired) break;
        ids.push_back(r.beatId);
    }
    return ids;
}

// 0 oculto, 1 pendiente, 2 hecho (segun el diario).
int estadoPlaza(const AdventureScript& g, const NarrativeState& s) {
    const Game::DiarioDatos d = Game::construirDiario({&g}, &g, s);
    for (const Game::DiarioMision& m : d.misiones)
        for (const Game::DiarioObjetivo& o : m.objetivos)
            if (o.id == "obj_plaza") return o.hecho ? 2 : 1;
    return 0;
}

// Estado de un objetivo cualquiera en el diario: 0 oculto, 1 pendiente, 2 hecho.
int estadoObjetivo(const AdventureScript& g, const NarrativeState& s, const std::string& id) {
    const Game::DiarioDatos d = Game::construirDiario({&g}, &g, s);
    for (const Game::DiarioMision& m : d.misiones)
        for (const Game::DiarioObjetivo& o : m.objetivos)
            if (o.id == id) return o.hecho ? 2 : 1;
    return 0;
}

// Llega, recibe el encargo, saluda a los tres y pelea (gana). El tutorial se cierra con el
// jugador en `dondeCierra`.
void cierraElTutorial(const NarrativeEngine& e, const AdventureScript& g, NarrativeState& s, const char* dondeCierra) {
    s.ponLugar(kPlaza);
    MOTOR_CHECK(e.enterLevel(kPlaza, s).beatId == "beat_llegada");
    MOTOR_CHECK(e.tick(s).beatId == "tut_encargo");
    for (const char* npc : {"tendero_mercado", "joyero"}) {
        MOTOR_CHECK(e.talkTo(npc, s).fired);
        MOTOR_CHECK(estadoPlaza(g, s) == 0);
    }
    MOTOR_CHECK(e.talkTo("sastre", s).fired);
    MOTOR_CHECK(estadoPlaza(g, s) == 0);  // los tres vistos: aun falta la emboscada
    s.setFlag(e.enterLevel(kPlaza, s).battles.at(0).flagVictory);
    MOTOR_CHECK(estadoPlaza(g, s) == 0);
    s.ponLugar(dondeCierra);
    MOTOR_CHECK(e.tick(s).beatId == "tut_victoria");
    MOTOR_CHECK(estadoPlaza(g, s) == 1);
}

MOTOR_TEST_CASE(si_el_tutorial_se_cierra_en_la_plaza_el_objetivo_se_cumple_sin_salir) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;
    cierraElTutorial(e, guion, s, kPlaza);
    MOTOR_CHECK(e.tick(s).beatId == "tut_plaza_ya");
    MOTOR_CHECK(s.hasFlag("bnd1_plaza_explorada") && estadoPlaza(guion, s) == 2);
    MOTOR_CHECK(e.enterLevel(kPlaza, s).beatId != "tut_plaza");  // una sola vez
}

MOTOR_TEST_CASE(si_el_tutorial_se_cierra_fuera_el_objetivo_se_cumple_al_entrar_en_la_plaza) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;
    cierraElTutorial(e, guion, s, kOtro);
    MOTOR_CHECK(e.tick(s).beatId != "tut_plaza_ya");  // fuera de la plaza no salta solo
    MOTOR_CHECK(estadoPlaza(guion, s) == 1);
    s.ponLugar(kPlaza);
    MOTOR_CHECK(e.enterLevel(kPlaza, s).beatId == "tut_plaza");
    MOTOR_CHECK(estadoPlaza(guion, s) == 2);
    MOTOR_CHECK(e.enterLevel(kPlaza, s).beatId != "tut_plaza");
}

// Venides manda a la plaza en el desayuno, lo pregunte el jugador o no (si no, en la despedida).
std::string charlaDeDesayuno(const NarrativeEngine& e, NarrativeState& s, bool pregunta) {
    s.setFlag("bnd0_noche");
    s.setFlag("bnd0_cama");
    s.setFlag("bnd0_a_dormir");
    s.setFlag("bnd0_rosita_te_sube");
    drena(e, s);
    e.enterLevel(kTaberna, s);
    std::string todo;
    for (int vuelta = 0; vuelta < 6 && !s.hasFlag("bnd1_desayuno_hecho"); ++vuelta) {
        const std::vector<std::string> t = s.eleccion().textos;
        for (const char* op : {"¿Qué trabajo?", "Dejarlo ahí", "Ponerse en marcha"}) {
            if (!pregunta && std::string(op) == "¿Qué trabajo?") continue;
            const auto it = std::find(t.begin(), t.end(), op);
            if (it == t.end()) continue;
            for (const std::string& l : e.elige(static_cast<int>(it - t.begin()), s).lines) todo += l + "\n";
            break;
        }
        e.sigue(s);
        drena(e, s);
    }
    return todo;
}

MOTOR_TEST_CASE(venides_envia_a_explorar_la_plaza_en_el_desayuno_se_pregunte_o_no) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    for (bool pregunta : {false, true}) {
        NarrativeState s;
        const std::string todo = charlaDeDesayuno(e, s, pregunta);
        MOTOR_CHECK(s.hasFlag("bnd1_desayuno_hecho"));
        const std::size_t a = todo.find("plaza principal");
        MOTOR_CHECK(a != std::string::npos);
        MOTOR_CHECK(!pregunta || todo.find("plaza principal", a + 1) != std::string::npos);  // respuesta + despedida
        MOTOR_CHECK(todo.find("el tendero, el joyero y el sastre") != std::string::npos);
    }
}

// Habla con Xila por primera vez (de dia, sin rol): elige la primera opcion.
void primeraCharlaConXila(const NarrativeEngine& e, NarrativeState& s) {
    MOTOR_CHECK(e.talkTo("xila", s).beatId == "b1_xila_desconocido");
    MOTOR_CHECK(!s.eleccion().textos.empty());
    e.elige(0, s);
}

MOTOR_TEST_CASE(la_vecina_de_la_plaza_no_da_la_pista_de_xila_antes_de_explorar_la_plaza) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;
    const NarrativeResult r = e.talkTo("vecino_de_plaza", s);
    MOTOR_CHECK(r.fired && r.beatId == "b1_vecina_plaza");
    MOTOR_CHECK(!s.hasFlag("bnd1_xila_pista_plaza"));
    MOTOR_CHECK(estadoObjetivo(guion, s, "obj_xila") == 0);
    MOTOR_CHECK(e.talkTo("vecino_de_plaza", s).beatId == "b1_vecina_plaza");  // sigue sin pista
    MOTOR_CHECK(!s.hasFlag("bnd1_xila_pista_plaza"));
}

MOTOR_TEST_CASE(la_vecina_de_la_plaza_envia_al_borracho_de_pico_dragon_y_apunta_el_objetivo) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;
    s.setFlag("bnd1_plaza_explorada");
    MOTOR_CHECK(estadoObjetivo(guion, s, "obj_xila") == 0);
    const NarrativeResult r = e.talkTo("vecino_de_plaza", s);
    MOTOR_CHECK(r.beatId == "b1_vecina_plaza_pista");
    MOTOR_CHECK(s.hasFlag("bnd1_xila_pista_plaza"));
    std::string todo;
    for (const std::string& l : r.lines) todo += l + "\n";
    MOTOR_CHECK(todo.find("Pico Dragón") != std::string::npos);
    MOTOR_CHECK(todo.find("Taberna del Gremio de Ladrones") != std::string::npos);
    MOTOR_CHECK(todo.find("Xila") == std::string::npos);  // el jugador aun no sabe como se llama
    MOTOR_CHECK(estadoObjetivo(guion, s, "obj_xila") == 1);
    MOTOR_CHECK(e.talkTo("vecino_de_plaza", s).beatId == "b1_vecina_plaza_ya");  // recordatorio, sin repetir
    MOTOR_CHECK(estadoObjetivo(guion, s, "obj_xila") == 1);
}

MOTOR_TEST_CASE(hablar_con_xila_cumple_el_objetivo_y_se_le_puede_encontrar_sin_la_pista) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    {
        NarrativeState s;
        s.setFlag("bnd1_plaza_explorada");
        MOTOR_CHECK(e.talkTo("vecino_de_plaza", s).beatId == "b1_vecina_plaza_pista");
        MOTOR_CHECK(estadoObjetivo(guion, s, "obj_xila") == 1);
        primeraCharlaConXila(e, s);
        MOTOR_CHECK(s.hasFlag("bnd1_xila_conocido"));
        MOTOR_CHECK(estadoObjetivo(guion, s, "obj_xila") == 2);
    }
    {
        NarrativeState s;  // sin pista: Xila habla igual y el objetivo nunca llega a verse
        primeraCharlaConXila(e, s);
        MOTOR_CHECK(s.hasFlag("bnd1_xila_conocido") && !s.hasFlag("bnd1_xila_pista_plaza"));
        MOTOR_CHECK(estadoObjetivo(guion, s, "obj_xila") == 0);
    }
}

}  // namespace

int main() { return motor_test::runAllTests(); }
