// E-MATRIZ-03 — Un guardado y una prueba por camino del dia 3 (examples/fabricar_guardados.h, CaminoDia3).
//
// Cada camino juega el dia 1, el dia de Gorran (la tregua o la pelea) y el dia 3 DE VERDAD con el arnes de las demos (la espada de Rosita,
// el castillo, la Barriada, la fiesta del Barrio Alto, el informe a Venides, dormir), fotografia la partida al cerrar el tercer dia, la
// escribe y la lee con el lector del juego, la restaura en una sesion limpia sobre el nivel real, EMPIEZA EL OCASO y comprueba lo que la
// matriz (tools/matriz_decisiones.py --dia segundo_dia) dice que la decision deja: flags, variables, oro, moral, nivel, mochila, los beats
// del Ocaso que la leen (se ven o no) y lo que el Ocaso cuenta al empezar.
// Mutacion: la MISMA comprobacion sobre un guardado fabricado con una copia temporal del JSON a la que se le quita el efecto de la
// decision tiene que fallar (el original no se toca). Sin GL ni ventana.
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

std::vector<std::string> juegaYComprueba(const demo_bnd::Guiones& g, const Heroe& h, const CaminoDia3& c, std::vector<std::string>* jugado = nullptr,
                                         bool mutada = false) {
    const SaveGameData d = dia3(g, h, c, jugado, !mutada);
    Restaurada r = restaura(g, h, d, std::string("dia3_") + c.id + ".json");
    if (std::getenv("MATRIZ_DIAG") != nullptr && !mutada) describeDia3(g, r, c);
    Restaurada r2 = restaura(g, h, d, std::string("dia3_") + c.id + "_b.json");
    return compruebaDia3(g, r2, c);
}

bool empiezaPor(const std::string& texto, const char* cabeza) { return texto.rfind(cabeza, 0) == 0; }

bool algunoEmpiezaPor(const std::vector<std::string>& fallos, const char* cabeza) {
    return std::any_of(fallos.begin(), fallos.end(), [&](const std::string& f) { return empiezaPor(f, cabeza); });
}

std::string temporalDe(const char* prefijo, const char* id) {
    return (std::filesystem::temp_directory_path() / (std::string(prefijo) + id + ".json")).string();
}

}  // namespace

MOTOR_TEST_CASE(cada_camino_del_dia_3_deja_lo_que_dice_la_matriz) {
    Fabrica f;
    const char* solo = std::getenv("MATRIZ_SOLO");
    for (const CaminoDia3& c : caminosDia3()) {
        if (solo != nullptr && std::string(solo) != c.id) continue;
        std::vector<std::string> jugado;
        const std::vector<std::string> fallos = juegaYComprueba(f.g, f.heroe, c, &jugado);
        for (const std::string& x : fallos) std::fprintf(stderr, "  [%s] %s\n", c.id, x.c_str());
        if (std::getenv("MATRIZ_BEATS")) for (const std::string& b : jugado) std::fprintf(stderr, "  [%s] beat %s\n", c.id, b.c_str());
        MOTOR_CHECK(fallos.empty());
        // Los beats que el camino anuncia se vieron jugando (no solo en el estado).
        for (const std::string& b : c.beats)
            if (std::find(jugado.begin(), jugado.end(), b) == jugado.end()) {
                std::fprintf(stderr, "  [%s] el arnes no jugo el beat %s\n", c.id, b.c_str());
                MOTOR_CHECK(false);
            }
    }
}

MOTOR_TEST_CASE(sin_el_efecto_de_la_decision_el_camino_del_dia_3_falla) {
    Fabrica f;
    const char* solo = std::getenv("MATRIZ_SOLO");
    for (const CaminoDia3& c : caminosDia3()) {
        if (solo != nullptr && std::string(solo) != c.id) continue;
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutacion, temporalDe("dia3_mut_", c.id));
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (fallos.empty()) std::fprintf(stderr, "  [%s] la mutacion (%s / %s / %s) no hizo fallar nada\n", c.id, c.mutacion.beat, c.mutacion.opcion, c.mutacion.cadena);
        else if (std::getenv("MATRIZ_VERBOSO")) std::fprintf(stderr, "  [%s] mutacion en rojo: %s%s\n", c.id, fallos.front().c_str(), fallos.size() > 1 ? " (+mas)" : "");
        MOTOR_CHECK(!fallos.empty());
    }
}

// El oro, la moral, el nivel y la mochila de cada partida: los trae el guardado y los restaura restoreSaveGame. Mutacion: sin el cobro
// (o con otro acto) cambian.
MOTOR_TEST_CASE(cada_camino_del_dia_3_trae_su_oro_su_moral_su_nivel_y_su_mochila) {
    Fabrica f;
    const char* solo = std::getenv("MATRIZ_SOLO");
    for (const CaminoDia3& c : caminosDia3()) {
        if (solo != nullptr && std::string(solo) != c.id) continue;
        const SaveGameData d = dia3(f.g, f.heroe, c);
        MOTOR_CHECK(d.gold == c.cuenta.oro);
        MOTOR_CHECK(d.moralityValue == c.cuenta.moral);
        MOTOR_CHECK(d.playerLevel == c.cuenta.nivel && d.playerLevelProgress == c.cuenta.progreso);
        MOTOR_CHECK(d.moral.pendientes.empty());  // la noche del cierre asienta lo del dia
        Restaurada r = restaura(f.g, f.heroe, d, std::string("dia3_") + c.id + "_cuentas.json");
        MOTOR_CHECK(r.nivel->sesion->gold() == c.cuenta.oro);
        MOTOR_CHECK(r.nivel->sesion->morality().value() == c.cuenta.moral);
    }
}

MOTOR_TEST_CASE(sin_su_cobro_el_oro_del_camino_del_dia_3_falla) {
    Fabrica f;
    const char* solo = std::getenv("MATRIZ_SOLO");
    for (const CaminoDia3& c : caminosDia3()) {
        if (solo != nullptr && std::string(solo) != c.id) continue;
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutOro, temporalDe("dia3_oro_", c.id));
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (std::getenv("MATRIZ_VERBOSO")) for (const std::string& x : fallos) std::fprintf(stderr, "  [%s] (oro) %s\n", c.id, x.c_str());
        if (!algunoEmpiezaPor(fallos, "oro=")) std::fprintf(stderr, "  [%s] quitar el oro (%s / %s) no cambio el oro\n", c.id, c.mutOro.beat, c.mutOro.opcion);
        MOTOR_CHECK(algunoEmpiezaPor(fallos, "oro="));
    }
}

MOTOR_TEST_CASE(con_otro_acto_la_moral_del_camino_del_dia_3_falla) {
    Fabrica f;
    const char* solo = std::getenv("MATRIZ_SOLO");
    for (const CaminoDia3& c : caminosDia3()) {
        if (solo != nullptr && std::string(solo) != c.id) continue;
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutMoral, temporalDe("dia3_moral_", c.id));
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (!algunoEmpiezaPor(fallos, "moral=")) std::fprintf(stderr, "  [%s] cambiar el acto (%s / %s / %s) no cambio la moral\n", c.id, c.mutMoral.beat, c.mutMoral.opcion, c.mutMoral.cadena);
        MOTOR_CHECK(algunoEmpiezaPor(fallos, "moral="));
    }
}

// Los caminos listados son los que la matriz pide: la fiesta (entrar / no entrar), Verina, el aviso de Xila y el informe a Venides, desde
// dos finales de Gorran (la tregua y la pelea).
MOTOR_TEST_CASE(estan_los_caminos_que_pide_la_matriz_del_dia_3) {
    std::vector<std::string> ids;
    for (const CaminoDia3& c : caminosDia3()) ids.push_back(c.id);
    for (const char* quiero : {"fiesta_dentro", "sin_fiesta", "verina_arriba", "verina_arriba_2", "xila_avisa", "xila_sin_aviso", "xila_resaca", "xila_corre", "informe_nada"})
        MOTOR_CHECK(std::find(ids.begin(), ids.end(), quiero) != ids.end());
    MOTOR_CHECK(ids.size() <= 13);
    std::set<std::string> gorran;
    for (const CaminoDia3& c : caminosDia3()) gorran.insert(c.gorran);
    MOTOR_CHECK(gorran.count("tregua") == 1 && gorran.count("cuatro_lios") == 1);  // la tregua y la pelea (estalla la subasta y se perdona)
}

// A0 — El arnes no habla con nadie que no se ve (ya no hay lista de pendientes) y, si no se entra en la fiesta, el noble no esta.
MOTOR_TEST_CASE(el_arnes_no_habla_con_el_noble_de_una_fiesta_a_la_que_no_se_entra_ni_con_un_venides_que_no_esta) {
    Fabrica f;
    demo_bnd::Jugador limpio(f.g, Suerte::Buena);
    limpio.s.ponLugar("assets/levels/mansion_barrio_alto.json");
    MOTOR_CHECK(!limpio.seVe("seguidor_brinda"));  // sin bnd2_fiesta_dentro
    limpio.s.setFlag("bnd2_fiesta_dentro");
    MOTOR_CHECK(limpio.seVe("seguidor_brinda"));
    limpio.s.clearFlag("bnd2_fiesta_dentro");
    // Venides en el cuartel del Ocaso: esta con el dia cerrado, y no cuando callaste el informe.
    limpio.s.ponLugar("assets/levels/interior_cuartel.json");
    limpio.s.setFlag("bnd2_dia_cerrado");
    MOTOR_CHECK(limpio.seVe("venides_base"));
    limpio.s.setFlag("bnd2_venides_sin_aviso");
    MOTOR_CHECK(!limpio.seVe("venides_base"));
    // El camino que no entra en la fiesta cierra el dia sin hablar con el noble, y el que calla el informe empieza el Ocaso sin hablar con Venides.
    MOTOR_CHECK(!dia3(f.g, f.heroe, caminoDia3("sin_fiesta")).narrativeFlags.empty());
    MOTOR_CHECK(!dia3(f.g, f.heroe, caminoDia3("informe_nada")).narrativeFlags.empty());
}

// Como un jugador real llega a la fiesta: de noche (franja 2), sin haberse acostado, a ciudad_este (b2_fiesta_puerta). Si ya se acosto
// (bnd2_a_dormir), la puerta no salta.
MOTOR_TEST_CASE(a_la_fiesta_se_llega_de_noche_y_sin_acostarse) {
    Fabrica f;
    const SaveGameData d = dia3(f.g, f.heroe, caminoDia3("fiesta_dentro"), nullptr, true, "fiesta");
    Restaurada r = restaura(f.g, f.heroe, d, "dia3_parada_fiesta.json");
    MOTOR_CHECK(r.j->s.variable("franja") == 2);
    MOTOR_CHECK(r.j->s.hasFlag("bnd2_encargo") && !r.j->s.hasFlag("bnd2_a_dormir") && !r.j->s.hasFlag("bnd2_fiesta_dentro"));
    for (const RPG::NarrativeBeat& b : f.g.dia2.beats)
        if (b.id == "b2_fiesta_puerta") {
            MOTOR_CHECK(std::find(b.requirements.notFlags.begin(), b.requirements.notFlags.end(), "bnd2_a_dormir") != b.requirements.notFlags.end());
            MOTOR_CHECK(b.opciones.size() <= RPG::NarrativeBeat::kMaxOpciones);
        }
}

// D0025.1 (antes hallazgo): Xila borracho del dia 1 (bnd1_xila_borracho) se ve el dia 3 hasta que se le habla (bnd2_xila_resaca_visto), y
// b2_xila_resaca se alcanza. Sobrio (o ya recuperado) se esconde en cuanto hay bnd2_encargo, como siempre; con el Ocaso vuelve.
MOTOR_TEST_CASE(xila_borracho_se_ve_el_dia_3_hasta_que_se_le_habla) {
    Fabrica f;
    demo_bnd::Jugador j(f.g, Suerte::Buena);
    j.s.ponLugar("assets/levels/ciudad_oeste.json");
    MOTOR_CHECK(j.seVe("xila"));  // antes del encargo (dia 1) se ve
    j.s.setFlag("bnd2_encargo");
    MOTOR_CHECK(!j.seVe("xila"));  // sobrio, el dia 3 no
    j.s.setFlag("bnd1_xila_borracho");
    MOTOR_CHECK(j.seVe("xila"));  // borracho, si
    j.s.setFlag("bnd2_xila_resaca_visto");
    MOTOR_CHECK(!j.seVe("xila"));  // ya se le hablo: se esconde
    j.s.clearFlag("bnd2_xila_resaca_visto");
    j.s.clearFlag("bnd1_xila_borracho");
    j.s.setFlag("bnd1_xila_honor");  // recuperado
    MOTOR_CHECK(!j.seVe("xila"));
    j.s.setFlag("bnd1_xila_borracho");
    j.s.setFlag("bnd2_xila_resaca_visto");
    j.s.setFlag("bnd_ocaso_empezado");
    MOTOR_CHECK(j.seVe("xila"));  // con el Ocaso vuelve
    const RPG::NarrativeBeat* resaca = nullptr;
    for (const RPG::NarrativeBeat& b : f.g.dia2.beats)
        if (b.id == "b2_xila_resaca") resaca = &b;
    MOTOR_CHECK(resaca != nullptr && resaca->trigger.target == "xila");
}

// D0025.3: el evento del aviso de Xila vale con Xila recuperado (honor sin borracho), con el heroe que corrio a avisar a Venides (bnd3_avisado:
// las campanas ya suenan) y sigue sin valer si Venides salio solo (bnd2_venides_sin_aviso) o si Xila sigue borracho.
MOTOR_TEST_CASE(el_aviso_de_xila_vale_recuperado_y_tras_correr_a_avisar_pero_no_sin_venides) {
    const auto avisa = [](std::initializer_list<const char*> flags, bool tarde) {
        auto r = RPG::eventos::cargaCatalogo("assets/eventos/boundington.json");
        require(r);
        RPG::NarrativeState s;
        for (const char* f : flags) s.setFlag(f);
        s.setVariable("dia", 4);
        s.setVariable("franja", tarde ? 1 : 0);
        RPG::eventos::evalua(r.value(), s);
        return s.hasFlag("bnd3_xila_avisa");
    };
    MOTOR_CHECK(avisa({"bnd1_xila_honor"}, true));                                   // el de siempre
    MOTOR_CHECK(avisa({"bnd1_xila_honor", "bnd1_aliado_xila", "bnd2_xila_resaca_visto"}, true));  // recuperado: honor, sin borracho
    MOTOR_CHECK(avisa({"bnd1_xila_honor", "bnd3_avisado", "bnd3_campanas"}, true));  // corrio a avisar a Venides
    MOTOR_CHECK(!avisa({"bnd1_xila_honor", "bnd3_campanas"}, true));                 // campanas sin haber avisado (puesta, el evento no vuelve)
    MOTOR_CHECK(!avisa({"bnd1_xila_honor", "bnd2_venides_sin_aviso"}, true));        // Venides ya salio solo
    MOTOR_CHECK(!avisa({"bnd1_xila_honor", "bnd2_venides_sin_aviso", "bnd3_avisado", "bnd3_campanas"}, true));
    MOTOR_CHECK(!avisa({"bnd1_xila_borracho"}, true));                               // borracho sin recuperar (sin honor)
    MOTOR_CHECK(!avisa({"bnd1_xila_honor", "bnd1_xila_borracho"}, true));
    MOTOR_CHECK(!avisa({"bnd1_xila_honor"}, false));                                 // sin cruzar la tarde
}

// ⚠ Hallazgo: bnd2_rosita_verina solo la lee el diario (quests.json:746): Verina arriba no cambia nada en el Ocaso.
MOTOR_TEST_CASE(hallazgo_verina_arriba_solo_la_lee_el_diario) {
    Fabrica f;
    for (const RPG::NarrativeBeat& b : f.g.ocaso.beats) {
        const auto lee = [](const std::vector<std::string>& v) { return std::find(v.begin(), v.end(), "bnd2_rosita_verina") != v.end(); };
        MOTOR_CHECK(!lee(b.requirements.allFlags) && !lee(b.requirements.notFlags) && !lee(b.requirements.anyFlags));
    }
}

int main() { return motor_test::runAllTests(); }
