// E-MATRIZ-01 — Un guardado y una prueba por camino del dia 1 (examples/fabricar_guardados.h, CaminoDia1).
//
// Cada camino juega el prologo y el dia 1 DE VERDAD con el arnes de las demos hasta cerrar el dia, fotografia la
// partida, la escribe y la lee con el lector del juego, la restaura en una sesion limpia sobre el nivel real y
// juega el inicio del dia siguiente (Gorran). Comprueba lo que la matriz (tools/matriz_decisiones.py) dice que la
// decision deja: sus flags y variables, que sigan en pie tras amanecer, y que los beats de los dias posteriores que
// las leen se vean (o no) con lo que trae ese guardado.
// Mutacion: la MISMA comprobacion sobre un guardado fabricado con una copia del JSON a la que se le quita el efecto
// de la opcion (temporal, el original no se toca) tiene que fallar.
// Sin GL ni ventana.
#include <algorithm>
#include <filesystem>
#include <string>

#include "TestHarness.h"
#include "fabricar_guardados.h"

namespace {

using namespace fabricar;
using demo_bnd::Suerte;  // (los caminos viven en fabricar_guardados.h: caminosDia1())

struct Fabrica {
    demo_bnd::Guiones g = demo_bnd::cargaTodo();
    Heroe heroe = creaHeroe();
};

std::vector<std::string> juegaYComprueba(const demo_bnd::Guiones& g, const Heroe& h, const CaminoDia1& c, std::vector<std::string>* jugado = nullptr,
                                         bool mutada = false) {
    const SaveGameData d = dia1(g, h, c, jugado, !mutada);
    Restaurada r = restaura(g, h, d, std::string(c.id) + ".json");
    return compruebaDia1(g, r, c);
}

}  // namespace

MOTOR_TEST_CASE(cada_camino_del_dia1_deja_lo_que_dice_la_matriz) {
    Fabrica f;
    for (const CaminoDia1& c : caminosDia1()) {
        std::vector<std::string> jugado;
        const std::vector<std::string> fallos = juegaYComprueba(f.g, f.heroe, c, &jugado);
        for (const std::string& x : fallos) std::fprintf(stderr, "  [%s] %s\n", c.id, x.c_str());
        MOTOR_CHECK(fallos.empty());
    }
}

MOTOR_TEST_CASE(sin_el_efecto_de_la_opcion_el_camino_falla) {
    Fabrica f;
    for (const CaminoDia1& c : caminosDia1()) {
        const std::string temporal = (std::filesystem::temp_directory_path() / (std::string("matriz_mut_") + c.id + ".json")).string();
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutacion, temporal);
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (fallos.empty()) std::fprintf(stderr, "  [%s] la mutacion (%s / %s / %s) no hizo fallar nada\n", c.id, c.mutacion.beat, c.mutacion.opcion, c.mutacion.cadena);
        else if (std::getenv("MATRIZ_VERBOSO")) std::fprintf(stderr, "  [%s] mutacion en rojo: %s%s\n", c.id, fallos.front().c_str(), fallos.size() > 1 ? " (+mas)" : "");
        MOTOR_CHECK(!fallos.empty());
    }
}

// E-MATRIZ-01b — El oro, la moral, el nivel y la mochila de cada partida. Los trae el guardado (el arnes los apunta y
// fabricar_guardados los vuelca por las piezas del juego) y los restaura restoreSaveGame; la tabla de caminos dice cuanto
// (el oro empieza en 150: la bolsa inicial). Mutacion: una copia del JSON a la que se le quita el cobro, o se le cambia
// el acto de la decision, tiene que dar otro oro u otra moral.
bool empiezaPor(const std::string& texto, const char* cabeza) { return texto.rfind(cabeza, 0) == 0; }

bool algunoEmpiezaPor(const std::vector<std::string>& fallos, const char* cabeza) {
    return std::any_of(fallos.begin(), fallos.end(), [&](const std::string& f) { return empiezaPor(f, cabeza); });
}

MOTOR_TEST_CASE(cada_camino_trae_su_oro_su_moral_su_nivel_y_su_mochila) {
    Fabrica f;
    for (const CaminoDia1& c : caminosDia1()) {
        const SaveGameData d = dia1(f.g, f.heroe, c);
        // Lo que se escribe en el fichero (no solo lo que sale de restaurar).
        MOTOR_CHECK(d.gold == c.cuenta.oro);
        MOTOR_CHECK(d.moralityValue == c.cuenta.moral);
        MOTOR_CHECK(d.playerLevel == c.cuenta.nivel && d.playerLevelProgress == c.cuenta.progreso);
        MOTOR_CHECK(d.moral.pendientes.empty());  // la noche del cierre asienta lo del dia
        Restaurada r = restaura(f.g, f.heroe, d, std::string(c.id) + "_cuentas.json");
        MOTOR_CHECK(r.nivel->sesion->gold() == c.cuenta.oro);
        MOTOR_CHECK(r.nivel->sesion->morality().value() == c.cuenta.moral);
        MOTOR_CHECK(r.nivel->sesion->morality().tramo() == moral::tramoDe(c.cuenta.moral));
    }
}

MOTOR_TEST_CASE(sin_su_cobro_el_oro_del_camino_falla) {
    Fabrica f;
    for (const CaminoDia1& c : caminosDia1()) {
        const std::string temporal = (std::filesystem::temp_directory_path() / (std::string("matriz_oro_") + c.id + ".json")).string();
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutOro, temporal);
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (!algunoEmpiezaPor(fallos, "oro=")) std::fprintf(stderr, "  [%s] quitar el oro (%s / %s) no cambio el oro\n", c.id, c.mutOro.beat, c.mutOro.opcion);
        MOTOR_CHECK(algunoEmpiezaPor(fallos, "oro="));
    }
}

MOTOR_TEST_CASE(con_otro_acto_la_moral_del_camino_falla) {
    Fabrica f;
    int probados = 0;
    for (const CaminoDia1& c : caminosDia1()) {
        if (std::string(c.mutMoral.beat).empty()) {  // sin ningun acto en el camino (la pelea no corrompe): nada que mover
            MOTOR_CHECK(c.cuenta.moral == 0);
            continue;
        }
        ++probados;
        const std::string temporal = (std::filesystem::temp_directory_path() / (std::string("matriz_moral_") + c.id + ".json")).string();
        const demo_bnd::Guiones mutada = conMutacion(f.g, c.mutMoral, temporal);
        const std::vector<std::string> fallos = juegaYComprueba(mutada, f.heroe, c, nullptr, true);
        if (!algunoEmpiezaPor(fallos, "moral=")) std::fprintf(stderr, "  [%s] cambiar el acto (%s / %s / %s) no cambio la moral\n", c.id, c.mutMoral.beat, c.mutMoral.opcion, c.mutMoral.cadena);
        MOTOR_CHECK(algunoEmpiezaPor(fallos, "moral="));
    }
    MOTOR_CHECK(probados >= 14);  // todos menos los de moral 0 sin acto (la pelea, la pifia de Aigren)
}

// ===========================================================================================================
// E-DIA1-CONSEC — Cada decision del dia 1 que no dejaba rastro (MATRIZ_DIA1.md, «Hallazgos») tiene ahora su eco. Para cada
// una: se carga el guardado del camino (el mismo de caminosDia1()), se juega lo que sigue como la ruta del heroe hasta donde
// sale (sigueLaCampana) y se mira lo que el jugador LEE (Jugador::dicho): sale en su camino y no sale en el contrario.
// Mutacion: una copia temporal del JSON (el del dia 1 o el del dia posterior) sin la flag nueva o sin la condicion de la
// variante, y el mismo camino tiene que dejar de ver su eco.
// ===========================================================================================================
struct Eco {
    const char* id;
    const char* camino;
    Hasta hasta;
    std::vector<std::string> dice;   // respuestas que van por delante de las de la ruta
    std::vector<std::string> sale;   // trozos de lo leido que tienen que salir
    std::vector<std::string> noSale; // ... y que no
    int rival = 0;                   // dado rival de la tirada de Persuasion del capitan (0 = no se mira)
    bool cajas = false;              // el cargamento gris llega a las Afueras
    Mutacion muta{"", "", ""};       // beat vacio = sin mutacion (el contrario de otra fila)
    bool edulAfueras = false;        // Edul no se encuentra en el Casco: contra el abrevadero de las Afueras
};

std::vector<std::string> juegaElEco(const demo_bnd::Guiones& g, const Heroe& h, const Eco& e, bool mutada) {
    const CaminoDia1& c = caminoDia1(e.camino);
    const SaveGameData d = dia1(g, h, c, nullptr, !mutada);
    Restaurada r = restaura(g, h, d, std::string(c.id) + "_eco.json");
    demo_bnd::Jugador& j = *r.j;
    sigueLaCampana(j, e.dice, e.hasta, e.cajas, e.edulAfueras);
    std::vector<std::string> fallos;
    for (const std::string& s : e.sale)
        if (!j.dijo(s)) fallos.push_back("no sale «" + s + "»");
    for (const std::string& s : e.noSale)
        if (j.dijo(s)) fallos.push_back("sobra «" + s + "»");
    if (e.rival != 0) {
        int vistas = 0;
        for (const auto& d2 : j.dados)
            if (d2.first.rfind("b2_capitan", 0) == 0 && d2.first.size() > 11 && d2.first.compare(d2.first.size() - 11, 11, "/persuasion") == 0) {
                ++vistas;
                if (d2.second != e.rival) fallos.push_back("la tirada del capitan usa " + std::to_string(d2.second) + " dados, no " + std::to_string(e.rival));
            }
        if (vistas == 0) fallos.push_back("no hubo tirada de Persuasion con el capitan");
    }
    return fallos;
}

const std::vector<Eco>& ecos() {
    static const std::vector<Eco> v = {
        // 1. Venides: «Confiar en él» -> el informe de la noche del dia 3
        {"venides_confia", "informe_confia", Hasta::Dia3, {"Contarselo todo"}, {"Hace dos noches bebiste conmigo"}, {}, 0, false,
         {"b2_noche_rosita_informa_confianza", "", "bnd1_venides_confianza", "zz_mutada", 1, 0, "segundo_dia"}},
        {"venides_cuenta_todo", "informe_cuenta_todo", Hasta::Dia3, {"Contarselo todo"}, {"Se lo cuentas todo: el ataque"}, {"bebiste conmigo"}},
        // 2. La pelea: protegido / mirando -> Rosita en Gorran
        {"rosita_protegido", "pelea_proteger", Hasta::Gorran, {}, {"Hoy el guiso lo pagas con eso"}, {"mirandome las manos"},
         0, false, {"b1_pelea", "Proteger al borracho dormido", "bnd1_pelea_protegido"}},
        {"rosita_protegido_luisarda", "pelea_proteger_luisarda", Hasta::Gorran, {}, {"Hoy el guiso lo pagas con eso"}, {"mirandome las manos"},
         0, false, {"b1_pelea", "Proteger al borracho dormido", "bnd1_pelea_protegido", "zz_mutada", 2}},
        {"rosita_mirando", "pelea_mirar", Hasta::Gorran, {}, {"Mira menos y ayuda mas"}, {"guiso"},
         0, false, {"bg_rosita_pelea_mirando", "", "bnd1_pelea_rosita", "zz_mutada", 1, 0, "dia_gorran"}},
        {"rosita_separar_no_dice_nada", "pelea_separar", Hasta::Gorran, {}, {}, {"guiso", "mirandome las manos"}},
        // 3. La pelea: separarlos ganada / perdida / huida -> Venides en el castillo
        {"venides_pelea_ganada", "pelea_separar", Hasta::Dia3, {"¿Anoche escribiste lo de la pelea?"}, {"lo tumbaste tu solo"}, {"a rastras"}, 0, false,
         {"b2_venides_castillo_s1_a_pelea_ganada", "", "bnd1_pelea_ganada", "zz_mutada", 1, 0, "segundo_dia"}},
        {"venides_pelea_perdida", "pelea_perdida", Hasta::Dia3, {"¿Anoche escribiste lo de la pelea?"}, {"te sacaron a rastras"}, {"lo tumbaste tu solo"}, 0, false,
         {"b2_venides_castillo_s1_a_pelea_perdida", "", "bnd1_pelea_perdida", "zz_mutada", 1, 0, "segundo_dia"}},
        {"venides_pelea_huida", "pelea_huida", Hasta::Dia3, {"¿Anoche escribiste lo de la pelea?"}, {"te sacaron a rastras"}, {"lo tumbaste tu solo"}, 0, false,
         {"b2_venides_castillo_s1_a_pelea_perdida", "", "bnd1_pelea_huida", "zz_mutada", 1, 0, "segundo_dia"}},
        {"venides_pelea_rosita", "pelea_proteger", Hasta::Dia3, {"¿Anoche escribiste lo de la pelea?"}, {"Pues eso no lo pongo"}, {"a rastras", "lo tumbaste tu solo"}, 0, false,
         {"b2_venides_castillo_s1_a", "¿Anoche escribiste lo de la pelea?", "bnd1_pelea_lanzada", "zz_mutada", 1, 0, "segundo_dia"}},
        // 4. El camorrista: muerto -> el capitan (un dado rival mas); echado -> el fanatico del ultimo barril
        {"capitan_muerto", "camorrista_muerto", Hasta::Dia3, {"Pedirle que prepare a la guardia [Persuasion]"}, {"Anteayer me trajeron un muerto"}, {}, 4, false,
         {"b2_capitan_muerto", "", "bnd1_camorrista_muerto", "zz_mutada", 1, 0, "segundo_dia"}},
        {"capitan_sin_muerto", "pelea_separar", Hasta::Dia3, {"Pedirle que prepare a la guardia [Persuasion]"}, {}, {"un muerto"}, 3},
        {"barril_echado", "pelea_separar", Hasta::Ocaso, {}, {"Te dije que te acordarias de mi"}, {}, 0, true,
         {"b3_barricada_gris_echado", "", "bnd1_camorrista_echado", "zz_mutada", 1, 0, "ocaso"}},
        {"barril_sin_echar", "camorrista_muerto", Hasta::Ocaso, {}, {"barril con el sello de cera gris"}, {"Te dije que te acordarias de mi"}, 0, true},
        // 5. Sandes: «¿Y tú quién eres para mandar?» / «Solo miraba.» / el resto -> el estrado
        {"estrado_desafiado", "sandes_mandar", Hasta::Sermon, {"Acercarte al estrado"}, {"preguntaba quien soy yo para mandar"}, {"solo miraba", "Recuerdo todas las caras"}, 0, false,
         {"b1_charla_sandes", "¿Y tú quién eres para mandar?", "bnd1_sandes_desafiado"}},
        {"estrado_solo_miraba", "sandes_solo", Hasta::Sermon, {"Acercarte al estrado"}, {"El que solo miraba"}, {"preguntaba quien soy yo", "Recuerdo todas las caras"}, 0, false,
         {"b1_charla_sandes", "Solo miraba.", "bnd1_sandes_enfadado"}},
        {"estrado_de_siempre", "sandes_luisarda", Hasta::Sermon, {"Acercarte al estrado"}, {"Recuerdo todas las caras"}, {"El que solo miraba", "preguntaba quien soy yo"}, 0, false,
         {"b3_sermon_s3_a", "Acercarte al estrado", "bnd1_sandes_te_conoce", "zz_mutada", 1, 0, "ocaso"}},
        // 6. Sandes: «Tu padre te escribe desde Killville» -> Edul contra el abrevadero
        {"edul_carta", "camara_carta", Hasta::Ocaso, {}, {"carta de Killville"}, {},
         0, false, {"b1_camara_sandes", "«Tu padre te escribe desde Killville.»", "bnd1_sandes_carta_dicha"}, true},
        {"edul_sin_carta", "camara_perdido", Hasta::Ocaso, {}, {"Sandes dice que arda con ello"}, {"carta de Killville"}, 0, false, {"", "", ""}, true},
        // 7. La guarida: «Plantarles cara» -> el capitan y la ronda de la iglesia vieja
        {"capitan_ronda", "edul_cara", Hasta::Dia3, {"Pedirle que prepare a la guardia [Persuasion]"}, {"oliendo a incienso"}, {}, 0, false,
         {"b1_edul", "Plantarles cara", "bnd1_guardias_avisados"}},
        {"capitan_sin_ronda", "edul_unirse", Hasta::Dia3, {"Pedirle que prepare a la guardia [Persuasion]"}, {}, {"oliendo a incienso"}},
        // 8. Los Verdaderos: vencidos / ganan / huido -> el reparto de pan
        {"pan_rufo", "verdaderos_cara", Hasta::Sermon, {}, {"Rufo, con la nariz torcida"}, {"cabecilla de la puerta azul con capa gris nueva"}, 0, false,
         {"b3_rufo_pan", "", "bnd1_verdaderos_vencidos", "zz_mutada", 1, 0, "ocaso"}},
        {"pan_cabecilla_ganan", "verdaderos_ganan", Hasta::Sermon, {}, {"cabecilla de la puerta azul con capa gris nueva"}, {"Rufo, con la nariz torcida"}, 0, false,
         {"b3_cabecilla_pan", "", "bnd1_verdaderos_ganan", "zz_mutada", 1, 0, "ocaso"}},
        {"pan_cabecilla_huido", "verdaderos_huido", Hasta::Sermon, {}, {"cabecilla de la puerta azul con capa gris nueva"}, {"Rufo, con la nariz torcida"}, 0, false,
         {"b3_cabecilla_pan", "", "bnd1_verdaderos_huido", "zz_mutada", 1, 0, "ocaso"}},
        {"pan_sin_rastro", "verdaderos_sumarse", Hasta::Sermon, {}, {}, {"Rufo, con la nariz torcida", "cabecilla de la puerta azul con capa gris nueva"}},
    };
    return v;
}

MOTOR_TEST_CASE(cada_decision_del_dia1_con_eco_lo_tiene_en_su_camino_y_no_en_el_contrario) {
    Fabrica f;
    const char* solo = std::getenv("CONSEC_SOLO");  // para diagnosticar: una sola fila (con BND_RUIDO_CAMPANA=1 cuenta la campana paso a paso)
    for (const Eco& e : ecos()) {
        if (solo != nullptr && std::string(solo) != e.id) continue;
        if (std::getenv("CONSEC_TRAZA") != nullptr) std::fprintf(stderr, "  ... %s\n", e.id);
        const std::vector<std::string> fallos = juegaElEco(f.g, f.heroe, e, false);
        for (const std::string& x : fallos) std::fprintf(stderr, "  [%s / %s] %s\n", e.id, e.camino, x.c_str());
        MOTOR_CHECK(fallos.empty());
    }
}

MOTOR_TEST_CASE(sin_la_flag_o_la_condicion_el_eco_desaparece) {
    Fabrica f;
    int probadas = 0;
    for (const Eco& e : ecos()) {
        if (std::string(e.muta.beat).empty()) continue;  // la fila del camino contrario
        if (std::getenv("CONSEC_TRAZA") != nullptr) std::fprintf(stderr, "  ... mutando %s\n", e.id);
        ++probadas;
        const std::string temporal = (std::filesystem::temp_directory_path() / (std::string("consec_mut_") + e.id + ".json")).string();
        const demo_bnd::Guiones mutada = conMutacion(f.g, e.muta, temporal);
        const std::vector<std::string> fallos = juegaElEco(mutada, f.heroe, e, true);
        if (fallos.empty()) std::fprintf(stderr, "  [%s] la mutacion (%s / %s / %s) no hizo fallar nada\n", e.id, e.muta.beat, e.muta.opcion, e.muta.cadena);
        else if (std::getenv("MATRIZ_VERBOSO")) std::fprintf(stderr, "  [%s] mutacion en rojo: %s\n", e.id, fallos.front().c_str());
        MOTOR_CHECK(!fallos.empty());
    }
    MOTOR_CHECK(probadas >= 18);
}

// «Acercarte al estrado» (b3_sermon_s3_a): para cada combinacion de lo que el dia 1 puede dejar con `bnd1_sandes_te_conoce`, el motor
// (NarrativeEngine::tick, el primer beat que encaja) enseña la pagina que toca y en ella UNA sola salida con ese texto.
std::string paginaDelEstrado(const demo_bnd::Guiones& g, bool conocido, bool enfadado, bool desafiado, int* cuantas) {
    demo_bnd::Jugador j(g, demo_bnd::Suerte::Buena);
    j.fase("ocaso");
    for (const char* f : {"bnd3_sermon_abierta", "bnd3_sermon_visto", "bnd3_sermon_en_charla", "bnd1_sandes_te_conoce"}) j.s.setFlag(f);
    j.s.setVariable("bnd3_sermon_pag", 3);
    j.s.ponLugar("assets/levels/ciudad_centro.json");
    if (conocido) j.s.setFlag("bnd1_sandes_conocido");
    if (enfadado) j.s.setFlag("bnd1_sandes_enfadado");
    if (desafiado) j.s.setFlag("bnd1_sandes_desafiado");
    const RPG::NarrativeResult r = j.e.tick(j.s);
    *cuantas = static_cast<int>(std::count(j.s.eleccion().textos.begin(), j.s.eleccion().textos.end(), "Acercarte al estrado"));
    return r.fired ? r.beatId : "(nada)";
}

bool todasLasCombinacionesDelEstradoTienenUnaSalida(const demo_bnd::Guiones& g) {
    bool bien = true;
    for (int conocido = 0; conocido < 2; ++conocido)
        for (int enfadado = 0; enfadado < 2; ++enfadado)
            for (int desafiado = 0; desafiado < 2; ++desafiado) {
                if (desafiado && !(conocido && enfadado)) continue;  // «¿Y tu quien eres para mandar?» pone las tres
                const std::string quiero = desafiado ? "b3_sermon_s3_a_desafiado" : (conocido && enfadado) ? "b3_sermon_s3_a_miraba" : "b3_sermon_s3_a";
                int cuantas = 0;
                const std::string sale = paginaDelEstrado(g, conocido != 0, enfadado != 0, desafiado != 0, &cuantas);
                if (sale != quiero || cuantas != 1) {
                    std::fprintf(stderr, "  estrado: conocido=%d enfadado=%d desafiado=%d -> %s con %d salidas (queria %s con 1)\n", conocido, enfadado, desafiado,
                                 sale.c_str(), cuantas, quiero.c_str());
                    bien = false;
                }
            }
    return bien;
}

MOTOR_TEST_CASE(el_estrado_tiene_una_sola_salida_para_cada_combinacion_del_dia1) {
    Fabrica f;
    MOTOR_CHECK(todasLasCombinacionesDelEstradoTienenUnaSalida(f.g));
    // Mutacion: sin la condicion de la pagina de «solo miraba» (enfadado) esa combinacion cae en la pagina de siempre.
    const std::string temporal = (std::filesystem::temp_directory_path() / "consec_mut_estrado.json").string();
    const Mutacion m{"b3_sermon_s3_a_miraba", "", "bnd1_sandes_enfadado", "zz_mutada", 1, 0, "ocaso"};
    const demo_bnd::Guiones mutada = conMutacion(f.g, m, temporal);
    MOTOR_CHECK(!todasLasCombinacionesDelEstradoTienenUnaSalida(mutada));
}

int main() { return motor_test::runAllTests(); }
