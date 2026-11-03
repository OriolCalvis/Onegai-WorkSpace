// E-MATRIZ-02 — Un guardado y una prueba por camino del dia de Gorran (examples/fabricar_guardados.h, CaminoGorran).
//
// Cada camino juega el dia 1 por un camino conocido (el de unirse al culto o uno neutral) y el dia de Gorran DE VERDAD con el
// arnes de las demos (los lios de la aldea, la subasta, dormir), fotografia la partida al cerrar el dia, la escribe y la lee
// con el lector del juego, la restaura en una sesion limpia sobre el nivel real, deja que amanezca el tercer dia y juega su
// comienzo (Rosita con la espada). Comprueba lo que la matriz (tools/matriz_decisiones.py --dia dia_gorran) dice que la decision
// deja: flags, variables, beats, oro, moral, nivel, mochila, los beats del dia 3 y del Ocaso que la leen (se ven o no) y lo que
// se lee al empezar el dia 3.
// Mutacion: la MISMA comprobacion sobre un guardado fabricado con una copia temporal del JSON de Gorran a la que se le quita
// el efecto de la decision tiene que fallar (el original no se toca). Sin GL ni ventana.
#include <algorithm>
#include <filesystem>
#include <string>

#include "TestHarness.h"
#include "fabricar_guardados.h"

namespace {

using namespace fabricar;
using demo_bnd::Suerte;

struct Fabrica {
    demo_bnd::Guiones g = demo_bnd::cargaTodo();
    Heroe heroe = creaHeroe();
};

std::vector<std::string> juegaYComprueba(const demo_bnd::Guiones& g, const Heroe& h, const CaminoGorran& c, std::vector<std::string>* jugado = nullptr,
                                         bool mutada = false) {
    const SaveGameData d = gorran(g, h, c, jugado, !mutada);
    Restaurada r = restaura(g, h, d, std::string("gorran_") + c.id + ".json");
    return compruebaGorran(g, r, c);
}

bool empiezaPor(const std::string& texto, const char* cabeza) { return texto.rfind(cabeza, 0) == 0; }

bool algunoEmpiezaPor(const std::vector<std::string>& fallos, const char* cabeza) {
    return std::any_of(fallos.begin(), fallos.end(), [&](const std::string& f) { return empiezaPor(f, cabeza); });
}

std::string temporalDe(const char* prefijo, const char* id) {
    return (std::filesystem::temp_directory_path() / (std::string(prefijo) + id + ".json")).string();
}

}  // namespace

MOTOR_TEST_CASE(cada_camino_del_dia_de_gorran_deja_lo_que_dice_la_matriz) {
    Fabrica f;
    const char* solo = std::getenv("MATRIZ_SOLO");
    for (const CaminoGorran& c : caminosGorran()) {
        if (solo != nullptr && std::string(solo) != c.id) continue;
        std::vector<std::string> jugado;
        const std::vector<std::string> fallos = juegaYComprueba(f.g, f.heroe, c, &jugado);
        for (const std::string& x : fallos) std::fprintf(stderr, "  [%s] %s\n", c.id, x.c_str());
        MOTOR_CHECK(fallos.empty());
        // Los beats que el camino anuncia se vieron jugando (no solo en el estado).
        for (const std::string& b : c.beats)
            if (std::find(jugado.begin(), jugado.end(), b) == jugado.end()) {
                std::fprintf(stderr, "  [%s] el arnes no jugo el beat %s\n", c.id, b.c_str());
                MOTOR_CHECK(false);
            }
    }
}

MOTOR_TEST_CASE(sin_el_efecto_de_la_decision_el_camino_de_gorran_falla) {
    Fabrica f;
    const char* solo = std::getenv("MATRIZ_SOLO");
    for (const CaminoGorran& c : caminosGorran()) {
        if (solo != nullptr && std::string(solo) != c.id) continue;
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutacion, temporalDe("gorran_mut_", c.id));
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (fallos.empty()) std::fprintf(stderr, "  [%s] la mutacion (%s / %s / %s) no hizo fallar nada\n", c.id, c.mutacion.beat, c.mutacion.opcion, c.mutacion.cadena);
        else if (std::getenv("MATRIZ_VERBOSO")) std::fprintf(stderr, "  [%s] mutacion en rojo: %s%s\n", c.id, fallos.front().c_str(), fallos.size() > 1 ? " (+mas)" : "");
        MOTOR_CHECK(!fallos.empty());
    }
}

// El oro, la moral, el nivel y la mochila de cada partida: los trae el guardado (el arnes los apunta, fabricar_guardados los
// vuelca por las piezas del juego) y los restaura restoreSaveGame. Mutacion: sin el cobro (o con otro acto) cambian.
MOTOR_TEST_CASE(cada_camino_de_gorran_trae_su_oro_su_moral_su_nivel_y_su_mochila) {
    Fabrica f;
    for (const CaminoGorran& c : caminosGorran()) {
        const SaveGameData d = gorran(f.g, f.heroe, c);
        MOTOR_CHECK(d.gold == c.cuenta.oro);
        MOTOR_CHECK(d.moralityValue == c.cuenta.moral);
        MOTOR_CHECK(d.playerLevel == c.cuenta.nivel && d.playerLevelProgress == c.cuenta.progreso);
        MOTOR_CHECK(d.moral.pendientes.empty());  // la noche del cierre asienta lo del dia
        Restaurada r = restaura(f.g, f.heroe, d, std::string("gorran_") + c.id + "_cuentas.json");
        MOTOR_CHECK(r.nivel->sesion->gold() == c.cuenta.oro);
        MOTOR_CHECK(r.nivel->sesion->morality().value() == c.cuenta.moral);
        MOTOR_CHECK(r.nivel->sesion->morality().tramo() == moral::tramoDe(c.cuenta.moral));
    }
}

MOTOR_TEST_CASE(sin_su_cobro_el_oro_del_camino_de_gorran_falla) {
    Fabrica f;
    for (const CaminoGorran& c : caminosGorran()) {
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutOro, temporalDe("gorran_oro_", c.id));
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (!algunoEmpiezaPor(fallos, "oro=")) std::fprintf(stderr, "  [%s] quitar el oro (%s / %s) no cambio el oro\n", c.id, c.mutOro.beat, c.mutOro.opcion);
        MOTOR_CHECK(algunoEmpiezaPor(fallos, "oro="));
    }
}

MOTOR_TEST_CASE(con_otro_acto_la_moral_del_camino_de_gorran_falla) {
    Fabrica f;
    int probados = 0;
    for (const CaminoGorran& c : caminosGorran()) {
        if (std::string(c.mutMoral.beat).empty()) {  // sin ningun acto en el camino (la comedia de la subasta no es ni bien ni mal): nada que mover
            MOTOR_CHECK(c.cuenta.moral == 0);
            continue;
        }
        ++probados;
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutMoral, temporalDe("gorran_moral_", c.id));
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (!algunoEmpiezaPor(fallos, "moral=")) std::fprintf(stderr, "  [%s] cambiar el acto (%s / %s / %s) no cambio la moral\n", c.id, c.mutMoral.beat, c.mutMoral.opcion, c.mutMoral.cadena);
        MOTOR_CHECK(algunoEmpiezaPor(fallos, "moral="));
    }
    MOTOR_CHECK(probados >= 8);
}

// Los caminos listados son los que la matriz pide (D0023): los 4 lios y solo 3, Mia perdida, la tregua, y la subasta con
// «Dejar que estalle» (perdonando y acabando con el).
MOTOR_TEST_CASE(estan_los_caminos_que_pide_la_matriz) {
    std::vector<std::string> ids;
    for (const CaminoGorran& c : caminosGorran()) ids.push_back(c.id);
    for (const char* quiero : {"cuatro_lios", "tres_lios", "mia_perdida", "tregua", "estalle_acaba", "culto_velas"})
        MOTOR_CHECK(std::find(ids.begin(), ids.end(), quiero) != ids.end());
    MOTOR_CHECK(ids.size() <= 10);
    // Dos puntos de partida: el que se une al culto y uno neutral.
    MOTOR_CHECK(std::string(caminoGorran("culto_velas").desde) == "edul_unirse");
    MOTOR_CHECK(std::string(caminoGorran("cuatro_lios").desde) == "pelea_separar");
}

// ⚠ Hallazgos comprobados jugando (MATRIZ_GORRAN.md): combinaciones que la historia de hoy PERMITE aunque se contradigan. Si Oriol
// decide cerrarlas (D0023: tregua que impide matar a Gorran), estas expectativas cambian con la decision.
MOTOR_TEST_CASE(hallazgo_se_puede_acabar_con_gorran_despues_de_su_tregua) {
    Fabrica f;
    CaminoGorran c = caminoGorran("tregua");
    c.dia = gd("BADC", true, true, {"Dejar que estalle"}, "Acabar con él (no volverá a ayudarte)");  // tregua, y luego la subasta estalla y se le mata
    const SaveGameData d = gorran(f.g, f.heroe, c);
    Restaurada r = restaura(f.g, f.heroe, d, "gorran_tregua_y_muerte.json");
    MOTOR_CHECK(r.j->s.hasFlag("bnd2_gorran_tregua"));
    MOTOR_CHECK(r.j->s.hasFlag("bnd2_gorran_muerto"));
    MOTOR_CHECK(!r.j->s.hasFlag("bnd2_gorran_perdonado"));
}

MOTOR_TEST_CASE(hallazgo_acabar_con_gorran_deja_a_iris_llevada) {
    Fabrica f;
    CaminoGorran c = caminoGorran("iris_llevada");
    c.dia = gd("BADC", false, false, {"Dejar que estalle"}, "Acabar con él (no volverá a ayudarte)", {"Dejar que se la lleven"}, false, {{"bga_garsa_pifia", Suerte::Mala}});
    const SaveGameData d = gorran(f.g, f.heroe, c);
    Restaurada r = restaura(f.g, f.heroe, d, "gorran_iris_y_muerte.json");
    MOTOR_CHECK(r.j->s.hasFlag("bga_iris_llevada"));   // sin Gorran redimido nadie la suelta (b2_gorran_suelta_iris)
    MOTOR_CHECK(!r.j->s.hasFlag("bga_iris_salvada"));
    MOTOR_CHECK(!lectorSeVe(f.g, Lector{"ocaso", "b3_epi_iris", "", true}, r.j->s, true));
    MOTOR_CHECK(!lectorSeVe(f.g, Lector{"ocaso", "b3_rev_casa", "", true}, r.j->s, true));
}

// ⚠ La tregua (b2_gorran_tregua) y el trato (b2_gorran_trato) se ofrecen hablando con `gorran`, y el juego lo esconde justo entonces:
// cinematicas.json («apariciones») pone `gorran` oculto con bnd2_subasta_ya, que enciende b2_subasta_aviso (auto) en cuanto hay
// gorran_misiones >= 3 (el mismo umbral de la tregua). Los dos beats se cumplen con el heroe delante... de un Gorran que no esta.
// El arnes (Jugador::habla -> talkTo) no mira la escena, por eso los caminos `tregua` y `trato_cojo` pasan; el jugador no llega.
MOTOR_TEST_CASE(hallazgo_la_tregua_y_el_trato_se_ofrecen_a_un_gorran_que_el_juego_esconde) {
    Fabrica f;
    auto catalogo = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
    MOTOR_CHECK(catalogo.isOk());
    for (const char* camino : {"tregua", "trato_cojo"}) {
        const SaveGameData d = gorranParada(f.g, f.heroe, caminoGorran(camino), "tregua");  // justo antes de hablar con Gorran
        Restaurada r = restaura(f.g, f.heroe, d, std::string("gorran_parada_") + camino + ".json");
        const auto flag = [&](const std::string& n) { return r.j->s.hasFlag(n); };
        const char* beat = std::string(camino) == "tregua" ? "b2_gorran_tregua" : "b2_gorran_trato";
        const RPG::NarrativeBeat* b = nullptr;
        for (const RPG::NarrativeBeat& x : f.g.gorran.beats)
            if (x.id == beat) b = &x;
        MOTOR_CHECK(b != nullptr);
        MOTOR_CHECK(b->requirements.isSatisfiedBy(r.j->s));  // el beat se dispararia al hablar...
        MOTOR_CHECK(r.j->s.hasFlag("bnd2_subasta_ya"));
        MOTOR_CHECK(!catalogo.value().visible("gorran", d.levelPath, flag));  // ... pero Gorran no esta (oculto: ni se ve ni se le habla)
        MOTOR_CHECK(catalogo.value().visible("gorran_subasta", d.levelPath, flag));  // el que se ve es el del salon, que no tiene esos beats
    }
}

int main() { return motor_test::runAllTests(); }
