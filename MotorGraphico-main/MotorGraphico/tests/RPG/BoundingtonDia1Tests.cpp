// Dia 1 de Boundington: el encargo de Venides en el desayuno y el objetivo de explorar
// la plaza (E-G4-05). Guion, diario y fichas reales (assets del directorio de build).

#include "RPG/NarrativeEngine.h"
#include "Game/DiarioMisiones.h"
#include "Game/Cinematica.h"
#include "Level/LevelLoader.h"

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
    s.setFlag("bnd1_desayuno_llamado");  // H-DIA1-1: el desayuno empieza al llamarte Venides
    e.tick(s);
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
        MOTOR_CHECK(todo.find("Venides: «Empieza por las tiendas del tendero, el joyero y el sastre. Después, la plaza principal. Habla con la gente y escucha.»") != std::string::npos);  // recordatorio de Oriol
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


// ---- H-DIA1-1: el desayuno empieza cuando pasas junto a Venides, intentas salir o le hablas ----

const char* const kTabernaP1 = "assets/levels/interior_taberna_p1.json";

Game::CatalogoCinematicas catalogo() {
    auto r = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
    MOTOR_CHECK(r.isOk());
    return r.value();
}

bool contiene(const std::vector<std::string>& v, const std::string& id) { return std::find(v.begin(), v.end(), id) != v.end(); }

// Amanece el dia 1 arriba, tras la noche 0 (estado de despues de b0_subir).
NarrativeState amanece(const NarrativeEngine& e) {
    NarrativeState s;
    for (const char* f : {"bnd0_noche", "bnd0_cama", "bnd0_a_dormir", "bnd0_rosita_te_sube"}) s.setFlag(f);
    drena(e, s);
    MOTOR_CHECK(s.hasFlag("bnd0_dormido"));
    return s;
}

// Hace lo que el juego: si empieza una cinematica con el jugador en `donde`, da sus beats y
// flags a la historia (y deja que salten los automaticos). Devuelve su id ("" si ninguna).
std::string cine(const Game::CatalogoCinematicas& cat, const NarrativeEngine& e, NarrativeState& s,
                 const GridCoord& donde, std::vector<std::string>* beats = nullptr) {
    const Game::Cinematica* c = cat.laQueEmpieza(kTaberna, donde, [&s](const std::string& f) { return s.hasFlag(f); });
    if (c == nullptr) return "";
    for (const Game::PasoCinematica& p : c->pasos) {
        if (p.tipo != Game::PasoCinematica::Tipo::Beat && p.tipo != Game::PasoCinematica::Tipo::Flag) continue;
        s.setFlag(p.texto);
        const auto ids = drena(e, s);
        if (beats) beats->insert(beats->end(), ids.begin(), ids.end());
    }
    s.setFlag(c->vista());
    for (const std::string& f : c->alTerminar) s.setFlag(f);
    return c->id;
}

bool esDesayuno(const std::string& id) { return id == "b1_desayuno" || id == "b1_desayuno_v0"; }

// Tras la cinematica que te sienta, salta el desayuno (auto) y se queda hecho una sola vez.
void siguenLosPasosDelDesayuno(const Game::CatalogoCinematicas& cat, const NarrativeEngine& e, NarrativeState& s) {
    MOTOR_CHECK(!s.hasFlag("bnd1_desayuno_visto"));
    MOTOR_CHECK(cine(cat, e, s, GridCoord{1, 9}) == "venides_te_sienta");  // venga de donde venga
    MOTOR_CHECK(s.hasFlag("bnd1_desayuno_llamado"));
    MOTOR_CHECK(esDesayuno(e.tick(s).beatId));
    MOTOR_CHECK(s.hasFlag("bnd1_desayuno_visto"));
    // Una sola vez: ni cinematicas, ni entrar, ni acercarse, ni hablarle lo repiten.
    for (const GridCoord& c : {GridCoord{5, 7}, GridCoord{12, 2}, GridCoord{1, 9}}) MOTOR_CHECK(cine(cat, e, s, c).empty());
    MOTOR_CHECK(!esDesayuno(e.enterLevel(kTaberna, s).beatId));
    MOTOR_CHECK(!esDesayuno(e.talkTo("venides", s).beatId));
    MOTOR_CHECK(!esDesayuno(e.tick(s).beatId));
}

MOTOR_TEST_CASE(bajar_a_la_taberna_no_empieza_el_desayuno_ni_pasando_lejos_de_venides) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    const Game::CatalogoCinematicas cat = catalogo();
    NarrativeState s = amanece(e);
    s.ponLugar(kTabernaP1);
    MOTOR_CHECK(!esDesayuno(e.enterLevel(kTabernaP1, s).beatId));
    s.ponLugar(kTaberna);
    MOTOR_CHECK(!e.enterLevel(kTaberna, s).fired);
    // Baja por la escalera (12, 2), cruza la sala, llega a la barra, a 3 o mas celdas de el.
    for (const GridCoord& c : {GridCoord{12, 2}, GridCoord{12, 5}, GridCoord{8, 6}, GridCoord{7, 7}, GridCoord{3, 4},
                               GridCoord{2, 9}, GridCoord{4, 10}, GridCoord{1, 10}, GridCoord{4, 4}})
        MOTOR_CHECK(cine(cat, e, s, c).empty());
    MOTOR_CHECK(!e.tick(s).fired);
    MOTOR_CHECK(!s.hasFlag("bnd1_desayuno_visto") && !s.hasFlag("bnd1_desayuno_pide"));
}

MOTOR_TEST_CASE(acercarse_a_venides_le_hace_llamarte_y_empieza_el_desayuno_una_vez) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    const Game::CatalogoCinematicas cat = catalogo();
    // A <= 2 celdas de Venides (4, 7) por delante de la mesa: las esquinas de la zona.
    for (const GridCoord& c : {GridCoord{5, 7}, GridCoord{4, 5}, GridCoord{6, 9}, GridCoord{4, 9}, GridCoord{6, 6}}) {
        NarrativeState s = amanece(e);
        std::vector<std::string> beats;
        MOTOR_CHECK(cine(cat, e, s, c, &beats) == "venides_llama");
        MOTOR_CHECK(contiene(beats, "b1_venides_llama"));  // la linea con que te llama
        MOTOR_CHECK(s.hasFlag("bnd1_desayuno_pide") && !s.hasFlag("bnd1_desayuno_visto"));
        MOTOR_CHECK(!e.tick(s).fired);  // aun no: primero anda hasta su silla
        MOTOR_CHECK(cine(cat, e, s, c) == "venides_te_sienta");
        MOTOR_CHECK(esDesayuno(e.tick(s).beatId));
        MOTOR_CHECK(cine(cat, e, s, c).empty());
    }
    // Una sola vez: despues de llamarte, la zona ya no lo vuelve a llamar.
    NarrativeState s = amanece(e);
    MOTOR_CHECK(cine(cat, e, s, GridCoord{5, 7}) == "venides_llama");
    siguenLosPasosDelDesayuno(cat, e, s);
    // Fuera de su zona (a 3 celdas o por detras de la mesa) no salta.
    NarrativeState t = amanece(e);
    for (const GridCoord& c : {GridCoord{7, 7}, GridCoord{3, 5}, GridCoord{2, 7}, GridCoord{4, 4}, GridCoord{5, 10}})
        MOTOR_CHECK(cine(cat, e, t, c).empty());
}

MOTOR_TEST_CASE(intentar_salir_por_la_puerta_no_sale_y_empieza_el_desayuno) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    const Game::CatalogoCinematicas cat = catalogo();
    const auto nivel = LevelLoader::loadFromFile(kTaberna);
    MOTOR_CHECK(nivel.isOk());
    const ObjectSpawn* puerta = nullptr;
    for (const ObjectSpawn& o : nivel.value().objects)
        if (o.objectId == "salida_interior_taberna") puerta = &o;
    MOTOR_CHECK(puerta != nullptr && puerta->beatCerrada == "b0_puerta_cerrada");

    NarrativeState s = amanece(e);
    s.ponLugar(kTaberna);
    const auto tiene = [&s](const std::string& f) { return s.hasFlag(f); };
    MOTOR_CHECK(puerta->estaCerrada(tiene));  // el jugador no sale: GameSession habla el beat en vez de cruzar
    const NarrativeResult r = e.talkTo(puerta->beatCerrada, s);
    MOTOR_CHECK(r.beatId == "b0_puerta_cerrada_venides");
    MOTOR_CHECK(r.lines.size() == 1 && r.lines[0].find("Siéntate, que tengo un trabajo para ti.") != std::string::npos);
    MOTOR_CHECK(s.hasFlag("bnd1_desayuno_pide"));
    MOTOR_CHECK(puerta->estaCerrada(tiene));
    MOTOR_CHECK(cine(cat, e, s, GridCoord{2, 10}) == "venides_te_sienta");  // sin pasar por venides_llama
    MOTOR_CHECK(esDesayuno(e.tick(s).beatId));
    MOTOR_CHECK(puerta->estaCerrada(tiene));  // hasta elegir «Ponerse en marcha»
    s.setFlag("bnd1_desayuno_hecho");
    MOTOR_CHECK(!puerta->estaCerrada(tiene));
}

MOTOR_TEST_CASE(hablar_con_venides_antes_de_tiempo_empieza_el_desayuno_sin_taparlo_otro_beat) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    const Game::CatalogoCinematicas cat = catalogo();
    NarrativeState s = amanece(e);
    const NarrativeResult r = e.talkTo("venides", s);
    MOTOR_CHECK(r.beatId == "b1_desayuno_habla");
    MOTOR_CHECK(s.hasFlag("bnd1_desayuno_pide") && !s.hasFlag("bnd1_desayuno_visto"));
    // Ya no vuelve a llamarte: la zona no empieza y se pasa a sentarte.
    MOTOR_CHECK(cine(cat, e, s, GridCoord{5, 7}) == "venides_te_sienta");
    MOTOR_CHECK(esDesayuno(e.tick(s).beatId));
    // Una sola vez: hablarle de nuevo ya no es b1_desayuno_habla.
    MOTOR_CHECK(e.talkTo("venides", s).beatId != "b1_desayuno_habla");
    // Si la cinematica no llegase a sentarte (no se pide dos veces): sin llamado, no hay desayuno.
    NarrativeState u = amanece(e);
    MOTOR_CHECK(e.talkTo("venides", u).beatId == "b1_desayuno_habla");
    MOTOR_CHECK(!e.tick(u).fired);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
