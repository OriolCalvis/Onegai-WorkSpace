// E-MATRIZ-04 — Un guardado y una prueba por final del Ocaso (examples/fabricar_guardados.h, CaminoOcaso).
//
// Cada camino juega el dia 1, el dia de Gorran, el dia 3 y el OCASO de verdad con el arnes de las demos (el sermon, correr a avisar a
// Venides o esperar la tarde, las campanas, la arenga, la salida, los combates, Verina, Sandes, Edul, Las Afueras, la huida y el epilogo),
// fotografia la partida al acabar (bnd3_fuera), la escribe y la lee con el lector del juego, la restaura en una sesion limpia sobre el nivel
// real y comprueba: el final (y solo ese), quien vive, el oro, la moral, el nivel, la mochila y el DIARIO (Game::construirDiario).
// Mutacion: la MISMA comprobacion sobre una copia temporal de boundington_ocaso.json a la que se le quita el efecto de la decision tiene que
// fallar (el original no se toca; la copia temporal se carga desde la prueba, no hace falta recompilar assets). Sin GL ni ventana.
//
// Diagnostico: MATRIZ_SOLO=<camino> limita a uno; MATRIZ_DIAG=1 describe lo que deja; MATRIZ_BEATS=1 lista los beats;
// MATRIZ_MUTAR=1 hace que la prueba verde use la copia mutada (sale en ROJO: es como se deja el log de cada mutacion).
#include <algorithm>
#include <filesystem>
#include <string>

#include "TestHarness.h"
#include "fabricar_guardados.h"

namespace {

using namespace fabricar;

struct Fabrica {
    demo_bnd::Guiones g = demo_bnd::cargaTodo();
    Heroe heroe = creaHeroe();
};

std::string temporalDe(const char* id) { return (std::filesystem::temp_directory_path() / (std::string("ocaso_mut_") + id + ".json")).string(); }

std::vector<std::string> juegaYComprueba(const demo_bnd::Guiones& g, const Heroe& h, const CaminoOcaso& c, std::vector<std::string>* jugado, bool mutada) {
    std::vector<std::string> beats;
    const SaveGameData d = ocaso(g, h, c, &beats, !mutada);
    if (jugado != nullptr) *jugado = beats;
    Restaurada r = restaura(g, h, d, std::string("ocaso_") + c.id + ".json");
    if (std::getenv("MATRIZ_DIAG") != nullptr && !mutada) describeOcaso(g, r, c, beats);
    return compruebaOcaso(g, r, c);
}

}  // namespace

MOTOR_TEST_CASE(cada_final_del_ocaso_deja_lo_que_dice_la_matriz) {
    Fabrica f;
    const char* solo = std::getenv("MATRIZ_SOLO");
    const bool mutar = std::getenv("MATRIZ_MUTAR") != nullptr;
    for (const CaminoOcaso& c : caminosOcaso()) {
        if (solo != nullptr && std::string(solo) != c.id) continue;
        const demo_bnd::Guiones mutada = mutar ? conMutacion(f.g, c.mutacion, temporalDe(c.id)) : f.g;
        if (mutar) std::fprintf(stderr, "  [%s] MUTADO: %s / %s / %s -> %s\n", c.id, c.mutacion.beat, c.mutacion.opcion, c.mutacion.cadena, c.mutacion.hacia);
        std::vector<std::string> jugado;
        const std::vector<std::string> fallos = juegaYComprueba(mutar ? mutada : f.g, f.heroe, c, &jugado, mutar);
        for (const std::string& x : fallos) std::fprintf(stderr, "  [%s] %s\n", c.id, x.c_str());
        if (std::getenv("MATRIZ_BEATS")) for (const std::string& b : jugado) std::fprintf(stderr, "  [%s] beat %s\n", c.id, b.c_str());
        MOTOR_CHECK(fallos.empty());
        // Los beats que el camino anuncia se vieron jugando (no solo en el estado), y los que no, no.
        for (const std::string& b : c.beats)
            if (std::find(jugado.begin(), jugado.end(), b) == jugado.end()) {
                std::fprintf(stderr, "  [%s] el arnes no jugo el beat %s\n", c.id, b.c_str());
                MOTOR_CHECK(false);
            }
        for (const std::string& b : c.noBeats)
            if (std::find(jugado.begin(), jugado.end(), b) != jugado.end()) {
                std::fprintf(stderr, "  [%s] el arnes jugo el beat %s, que el camino dice que no se ve\n", c.id, b.c_str());
                MOTOR_CHECK(false);
            }
        if (!mutar) std::fprintf(stderr, "  [%s] verde: final %s, %d beats jugados\n", c.id, c.final, static_cast<int>(jugado.size()));
    }
}

MOTOR_TEST_CASE(sin_el_efecto_de_la_decision_el_final_del_ocaso_falla) {
    Fabrica f;
    const char* solo = std::getenv("MATRIZ_SOLO");
    for (const CaminoOcaso& c : caminosOcaso()) {
        if (solo != nullptr && std::string(solo) != c.id) continue;
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutacion, temporalDe(c.id));
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (fallos.empty()) std::fprintf(stderr, "  [%s] la mutacion (%s / %s / %s) no hizo fallar nada\n", c.id, c.mutacion.beat, c.mutacion.opcion, c.mutacion.cadena);
        else std::fprintf(stderr, "  [%s] mutacion (%s / %s / %s) en rojo: %d fallos, el primero: %s\n", c.id, c.mutacion.beat, c.mutacion.opcion, c.mutacion.cadena,
                          static_cast<int>(fallos.size()), fallos.front().c_str());
        MOTOR_CHECK(!fallos.empty());
    }
}

// Los caminos listados son los que pide la matriz: los tres finales (oeste, solos, caido), el aviso de Xila (si / no / tras correr), Venides sin
// aviso, la Barriada delatada, Sandes aliado, el norte y los sacrificios; y a lo sumo 10.
MOTOR_TEST_CASE(estan_los_finales_y_caminos_que_pide_la_matriz_del_ocaso) {
    std::set<std::string> ids, finales;
    for (const CaminoOcaso& c : caminosOcaso()) {
        ids.insert(c.id);
        finales.insert(c.final);
    }
    for (const char* quiero : {"oeste_todos", "oeste_xila_avisa", "oeste_xila_sin_aviso", "oeste_sin_venides", "solos_delatada", "sandes_aliado", "oeste_norte", "caido", "sacrificios"})
        MOTOR_CHECK(ids.count(quiero) == 1);
    MOTOR_CHECK(caminosOcaso().size() <= 10);
    MOTOR_CHECK(finales.count("bnd3_final_oeste") == 1 && finales.count("bnd3_final_solos") == 1 && finales.count("bnd3_final_caido") == 1);
    // Cada camino parte de un dia 3 que existe, y ese dia 3 es uno de los de la matriz del dia 3.
    for (const CaminoOcaso& c : caminosOcaso()) MOTOR_CHECK(std::string(caminoDia3(c.dia3).id) == c.dia3);
}

// ⚠ Hallazgo: «FINAL: con los Perdidos» (b3_aliado_fuera) nunca salta: b3_huida_gitanos y b3_huida_sola van antes en el fichero, no excluyen
// a bnd3_aliado_sandes, y el primer beat que coincide gana. Con Sandes aliado se sale «con los gitanos» (si hay Barriada) o «sin ayuda».
MOTOR_TEST_CASE(hallazgo_el_final_con_los_perdidos_queda_tapado_por_la_huida_de_los_gitanos_y_la_huida_sola) {
    Fabrica f;
    for (const bool delatada : {false, true}) {
        demo_bnd::Jugador j(f.g, demo_bnd::Suerte::Buena);
        fase(j, "ocaso");
        for (const char* fl : {"bnd2_dia_cerrado", "bnd3_aliado_sandes", "bnd3_edul_cerrado", "bnd3_sandes_cerrado", "bnd3_esc06_ciudad_atras"}) j.s.setFlag(fl);
        if (delatada) j.s.setFlag("bnd2_barriada_delatada");
        else j.s.setFlag("bnd2_aliado_gitanos");
        j.entra("huida_boundington");
        MOTOR_CHECK(j.s.hasFlag("bnd3_fuera") && j.s.hasFlag("bnd3_final_solos") == !j.s.hasFlag("bnd3_final_oeste"));
        MOTOR_CHECK(!j.visto("b3_aliado_fuera"));
        MOTOR_CHECK(j.visto(delatada ? "b3_huida_sola" : "b3_huida_gitanos"));
    }
    int posGitanos = -1, posSola = -1, posAliado = -1, i = 0;
    for (const RPG::NarrativeBeat& b : f.g.ocaso.beats) {
        if (b.id == "b3_huida_gitanos") posGitanos = i;
        if (b.id == "b3_huida_sola") posSola = i;
        if (b.id == "b3_aliado_fuera") posAliado = i;
        ++i;
    }
    MOTOR_CHECK(posGitanos >= 0 && posGitanos < posSola && posSola < posAliado);
}

// ⚠ Hallazgo: el diario marca «Sal de Boundington con vida» (obj3_salir, doneFlag bnd3_fuera) como hecho aunque el heroe CAIGA (b3_caida pone
// bnd3_fuera y bnd3_final_caido), y el final (oeste / solos / caido) no sale en ninguna decision del diario.
MOTOR_TEST_CASE(hallazgo_el_diario_da_por_hecha_la_salida_aunque_el_heroe_caiga) {
    Fabrica f;
    RPG::NarrativeState s;
    s.setFlag("bnd_ocaso_empezado");
    s.setFlag("bnd3_fuera");
    s.setFlag("bnd3_final_caido");
    const std::vector<const RPG::AdventureScript*> campana = {&f.g.prologo, &f.g.dia1, &f.g.gorran, &f.g.dia2, &f.g.ocaso};
    const Game::DiarioDatos d = Game::construirDiario(campana, &f.g.ocaso, s);
    const Game::DiarioMision* m = nullptr;
    for (const Game::DiarioMision& x : d.misiones)
        if (x.id == "adv_boundington_ocaso") m = &x;
    MOTOR_CHECK(m != nullptr);
    if (m == nullptr) return;
    bool salirHecho = false;
    for (const Game::DiarioObjetivo& o : m->objetivos)
        if (o.id == "obj3_salir") salirHecho = o.hecho;
    MOTOR_CHECK(salirHecho);          // el hallazgo: caido y «salir con vida» hecho
    MOTOR_CHECK(m->decisiones.empty());  // ninguna decision de la ficha nombra el final caido / oeste / solos
}

int main() { return motor_test::runAllTests(); }
