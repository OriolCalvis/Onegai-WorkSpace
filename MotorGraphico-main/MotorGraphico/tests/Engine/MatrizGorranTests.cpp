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

// D0024 — La tregua con Gorran se pacta EN LA SUBASTA y le protege. Con el aval de Serra, la subasta tiene una pagina gemela
// (b2_subasta_aval) que cambia «Dejar que estalle» por «Proponerle una tregua» (4 opciones, el maximo) y, con la tregua puesta,
// b2_subasta_tregua ofrece pactos y UNA opcion explicita de traicion («Traicionar la tregua»), la unica que lleva a la baralla.
namespace {
struct Tregua {
    std::vector<std::string> jugado;
    std::vector<std::string> fallos;
};

// Juega el camino `tregua` con la decision `dia` y mira si Gorran sigue vivo y sin pelea (protegido) o si hubo baralla (traicion).
Tregua juegaLaTregua(const demo_bnd::Guiones& g, const Heroe& h, const demo_bnd::Jugador::DiaDeGorran& dia, bool traicion, bool mutada) {
    Tregua t;
    CaminoGorran c = caminoGorran("tregua");
    c.dia = dia;
    const SaveGameData d = gorran(g, h, c, &t.jugado, !mutada);
    Restaurada r = restaura(g, h, d, std::string("gorran_tregua_") + (traicion ? "traicion" : "pacto") + (mutada ? "_mut" : "") + ".json");
    const auto flag = [&](const char* f) { return r.j->s.hasFlag(f); };
    const auto visto = [&](const char* b) { return std::find(t.jugado.begin(), t.jugado.end(), b) != t.jugado.end(); };
    const auto falta = [&](const std::string& m) { t.fallos.push_back(m); };
    if (!flag("bnd2_gorran_tregua")) falta("falta bnd2_gorran_tregua");
    if (!visto("b2_subasta_aval_cojo_pobre") || !visto("b2_subasta_tregua")) {
        std::string vistos;
        for (const std::string& b : t.jugado)
            if (b.find("subasta") != std::string::npos || b.find("honor") != std::string::npos) vistos += " " + b;
        falta("la tregua no paso por la subasta (b2_subasta_aval* / b2_subasta_tregua); beats de la subasta vistos:" + vistos);
    }
    if (traicion) {
        if (!flag("bnd2_tregua_rota")) falta("falta bnd2_tregua_rota");
        if (!flag("bnd2_comedia_pelea") || !flag("bnd2_gorran_lanzado") || !flag("bnd2_comedia_pelea_lanzada")) falta("la traicion no lleva a la baralla");
        if (!flag("bnd2_gorran_muerto")) falta("tras la traicion se le puede matar (b2_gorran_victoria) y no pasa");
    } else {
        if (flag("bnd2_tregua_rota") || flag("bnd2_comedia_pelea") || flag("bnd2_gorran_lanzado") || flag("bnd2_comedia_pelea_lanzada")) falta("la tregua no impidio la baralla");
        if (flag("bnd2_gorran_muerto") || visto("b2_gorran_victoria")) falta("Gorran ha muerto con la tregua puesta");
        if (!flag("bnd2_tregua_cerrada")) falta("la tregua no se cerro");
    }
    return t;
}
}  // namespace

MOTOR_TEST_CASE(la_tregua_se_pacta_en_la_subasta_y_gorran_no_muere) {
    Fabrica f;
    const Tregua t = juegaLaTregua(f.g, f.heroe, caminoGorran("tregua").dia, false, false);
    for (const std::string& x : t.fallos) std::fprintf(stderr, "  [tregua] %s\n", x.c_str());
    MOTOR_CHECK(t.fallos.empty());
    // La pagina antigua (hablar con Gorran en el salon, escondido desde el cuerno) ya no esta: nada que no se pueda alcanzar.
    for (const RPG::NarrativeBeat& b : f.g.gorran.beats) MOTOR_CHECK(b.id != "b2_gorran_tregua");
}

MOTOR_TEST_CASE(traicionar_la_tregua_lleva_a_la_baralla_y_solo_entonces_se_puede_acabar_con_gorran) {
    Fabrica f;
    const auto dia = gd("BADC", true, true, {"Traicionar la tregua"}, "Acabar con él (no volverá a ayudarte)");
    const Tregua t = juegaLaTregua(f.g, f.heroe, dia, true, false);
    for (const std::string& x : t.fallos) std::fprintf(stderr, "  [traicion] %s\n", x.c_str());
    MOTOR_CHECK(t.fallos.empty());
}

// Mutaciones: sin el efecto de la traicion no hay baralla; y si un pacto lleva a la baralla, la tregua no protege.
MOTOR_TEST_CASE(sin_la_traicion_no_hay_baralla_y_si_el_pacto_pelea_la_tregua_no_protege) {
    Fabrica f;
    const auto traidor = gd("BADC", true, true, {"Traicionar la tregua"}, "Acabar con él (no volverá a ayudarte)");
    const demo_bnd::Guiones sinPelea = conMutacion(f.g, mg("b2_subasta_tregua", "Traicionar la tregua", "bnd2_comedia_pelea"), temporalDe("gorran_mut_", "traicion"));
    MOTOR_CHECK(!juegaLaTregua(sinPelea, f.heroe, traidor, true, true).fallos.empty());
    const demo_bnd::Guiones peleaSiempre =
        conMutacion(f.g, mg("b2_subasta_tregua", "Salvarle la cara a Gorran", "bnd2_honor_exito", "bnd2_comedia_pelea"), temporalDe("gorran_mut_", "protege"));
    MOTOR_CHECK(!juegaLaTregua(peleaSiempre, f.heroe, caminoGorran("tregua").dia, false, true).fallos.empty());
}

// La gemela cambia SOLO la ultima opcion: 4 opciones en las tres variantes, sin «Dejar que estalle», con la tregua; las pacificas de
// b2_subasta_tregua no encienden la baralla y la traicion si.
MOTOR_TEST_CASE(las_gemelas_de_la_subasta_ofrecen_la_tregua_sin_pasar_de_cuatro_opciones) {
    Fabrica f;
    const auto beat = [&](const char* id) -> const RPG::NarrativeBeat& {
        for (const RPG::NarrativeBeat& b : f.g.gorran.beats)
            if (b.id == id) return b;
        MOTOR_CHECK(false);
        return f.g.gorran.beats.front();
    };
    const auto efectos = [](const RPG::OpcionDeDialogo& o, const char* flag) {
        return std::any_of(o.efectos.begin(), o.efectos.end(), [&](const RPG::NarrativeEffect& e) { return e.arg == flag; });
    };
    for (const char* id : {"b2_subasta_aval_cojo_pobre", "b2_subasta_aval", "b2_subasta_aval_sin_espada"}) {
        const RPG::NarrativeBeat& b = beat(id);
        MOTOR_CHECK(b.opciones.size() == 4);
        MOTOR_CHECK(b.opciones.size() <= RPG::NarrativeBeat::kMaxOpciones);
        MOTOR_CHECK(std::any_of(b.requirements.allFlags.begin(), b.requirements.allFlags.end(), [](const std::string& x) { return x == "bnd2_aval_serra"; }));
        int tregua = 0;
        for (const RPG::OpcionDeDialogo& o : b.opciones) {
            MOTOR_CHECK(o.texto != "Dejar que estalle");
            if (o.texto == "Proponerle una tregua") {
                ++tregua;
                MOTOR_CHECK(efectos(o, "bnd2_gorran_tregua"));
                MOTOR_CHECK(!efectos(o, "bnd2_comedia_pelea"));
            }
        }
        MOTOR_CHECK(tregua == 1);
    }
    for (const RPG::OpcionDeDialogo& o : beat("b2_subasta_tregua").opciones)
        MOTOR_CHECK(efectos(o, "bnd2_comedia_pelea") == (o.texto == "Traicionar la tregua"));
    MOTOR_CHECK(beat("b2_subasta_tregua").opciones.size() <= RPG::NarrativeBeat::kMaxOpciones);
}

// D0024.3 — El arnes respeta `ocultoSi` de cinematicas.json (Jugador::seVe: el mismo CatalogoCinematicas::visible que el juego): desde el
// cuerno de la subasta (bnd2_subasta_ya) `gorran` esta oculto y el que se ve es `gorran_subasta`. Hablar con quien se esconde hace fallar
// la prueba con un mensaje claro (require en Jugador::habla), asi que los caminos no pueden volver a tocar a un Gorran que no esta.
// ⚠ Hallazgo que sigue abierto: b2_gorran_trato (talk con `gorran`, bnd2_subasta_ya) NO se alcanza jugando (D0024 no lo toca); el trato
// se lee en el calabozo (b2_calabozo, camino `trato_cojo`).
MOTOR_TEST_CASE(el_arnes_no_habla_con_un_gorran_oculto_y_el_trato_en_su_salon_sigue_sin_alcanzarse) {
    Fabrica f;
    demo_bnd::Jugador limpio(f.g, Suerte::Buena);
    limpio.s.ponLugar("assets/levels/recinto_gorran.json");
    MOTOR_CHECK(limpio.seVe("gorran"));
    const SaveGameData d = gorranParada(f.g, f.heroe, caminoGorran("tregua"), "subasta");  // con el cuerno ya sonado
    Restaurada r = restaura(f.g, f.heroe, d, "gorran_parada_tregua.json");
    r.j->s.ponLugar("assets/levels/recinto_gorran.json");
    MOTOR_CHECK(r.j->s.hasFlag("bnd2_subasta_ya"));
    MOTOR_CHECK(!r.j->seVe("gorran"));
    MOTOR_CHECK(r.j->seVe("gorran_subasta"));
    const RPG::NarrativeBeat* trato = nullptr;
    for (const RPG::NarrativeBeat& x : f.g.gorran.beats)
        if (x.id == "b2_gorran_trato") trato = &x;
    MOTOR_CHECK(trato != nullptr);
    MOTOR_CHECK(std::find(trato->requirements.allFlags.begin(), trato->requirements.allFlags.end(), "bnd2_subasta_ya") != trato->requirements.allFlags.end());
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

int main() { return motor_test::runAllTests(); }
