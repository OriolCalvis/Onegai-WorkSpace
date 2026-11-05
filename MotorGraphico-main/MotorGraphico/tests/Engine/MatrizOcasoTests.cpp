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
#include <fstream>
#include <iterator>
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

// Los caminos listados son los que pide la matriz: los tres finales (oeste, solos, huido), el aviso de Xila (si / no / tras correr), Venides sin
// aviso, la Barriada delatada, Sandes aliado, el norte y los sacrificios; y a lo sumo 10.
MOTOR_TEST_CASE(estan_los_finales_y_caminos_que_pide_la_matriz_del_ocaso) {
    std::set<std::string> ids, finales;
    for (const CaminoOcaso& c : caminosOcaso()) {
        ids.insert(c.id);
        finales.insert(c.final);
    }
    for (const char* quiero : {"oeste_todos", "oeste_xila_avisa", "oeste_xila_sin_aviso", "oeste_sin_venides", "solos_delatada", "sandes_aliado", "oeste_norte", "huido", "sacrificios"})
        MOTOR_CHECK(ids.count(quiero) == 1);
    MOTOR_CHECK(caminosOcaso().size() <= 10);
    MOTOR_CHECK(finales.count("bnd3_final_oeste") == 1 && finales.count("bnd3_final_solos") == 1 && finales.count("bnd3_final_huido") == 1);
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

// E-HALLAZGOS-01 y D0028: el diario del Ocaso solo da por hecho «Sal de Boundington con vida» (obj3_salir) si el heroe sale (bnd3_salio_vivo,
// que ponen todas las salidas, tambien la huida a la carrera) y cuenta como acabo la noche (decision bnd3_final).
namespace {
struct DiarioDelOcaso {
    bool salirHecho = false, hayMision = false;
    std::string final, ruta;  // lo que ensenan las decisiones bnd3_final / bnd3_ruta ("" = ninguna)
};
DiarioDelOcaso diarioDelOcaso(const demo_bnd::Guiones& g, std::initializer_list<const char*> flags) {
    RPG::NarrativeState s;
    s.setFlag("bnd_ocaso_empezado");
    for (const char* f : flags) s.setFlag(f);
    const std::vector<const RPG::AdventureScript*> campana = {&g.prologo, &g.dia1, &g.gorran, &g.dia2, &g.ocaso};
    const Game::DiarioDatos d = Game::construirDiario(campana, &g.ocaso, s);
    DiarioDelOcaso r;
    for (const Game::DiarioMision& m : d.misiones) {
        if (m.id != "adv_boundington_ocaso") continue;
        r.hayMision = true;
        for (const Game::DiarioObjetivo& o : m.objetivos)
            if (o.id == "obj3_salir") r.salirHecho = o.hecho;
        for (const Game::DiarioDecision& x : m.decisiones) {
            if (x.id == "bnd3_final") r.final = x.eleccion;
            if (x.id == "bnd3_ruta") r.ruta = x.eleccion;
        }
    }
    return r;
}
}  // namespace

MOTOR_TEST_CASE(el_diario_da_por_hecha_la_salida_solo_al_salir_y_cuenta_como_acabo_la_noche) {
    Fabrica f;
    const DiarioDelOcaso huido = diarioDelOcaso(f.g, {"bnd3_fuera", "bnd3_salio_vivo", "bnd3_final_huido"});
    MOTOR_CHECK(huido.hayMision);
    MOTOR_CHECK(huido.salirHecho);                                // huir tambien es «salir con vida» (D0028)
    MOTOR_CHECK(huido.final.find("Huiste") != std::string::npos);
    const DiarioDelOcaso sinSalir = diarioDelOcaso(f.g, {"bnd3_fuera"});  // estar fuera de la historia no basta
    MOTOR_CHECK(!sinSalir.salirHecho);
    const DiarioDelOcaso oeste = diarioDelOcaso(f.g, {"bnd3_fuera", "bnd3_salio_vivo", "bnd3_final_oeste"});
    MOTOR_CHECK(oeste.salirHecho && oeste.final.find("con los gitanos") != std::string::npos);
    const DiarioDelOcaso solos = diarioDelOcaso(f.g, {"bnd3_fuera", "bnd3_salio_vivo", "bnd3_final_solos"});
    MOTOR_CHECK(solos.salirHecho && solos.final.find("sin los gitanos") != std::string::npos);
    const DiarioDelOcaso antes = diarioDelOcaso(f.g, {});         // antes del final no se anuncia nada
    MOTOR_CHECK(!antes.salirHecho && antes.final.empty());
    // Todas las salidas ponen bnd3_salio_vivo: nadie muere en el final (D0027, D0028).
    for (const char* b : {"b3_huida_gitanos", "b3_huida_sola", "b3_aliado_fuera", "b3_huida_perdida", "b3_rodeo_huida"}) {
        bool pone = false;
        for (const RPG::NarrativeBeat& beat : f.g.ocaso.beats)
            if (beat.id == b)
                for (const RPG::NarrativeEffect& e : beat.effects) pone = pone || (e.type == RPG::EffectType::SetFlag && e.arg == "bnd3_salio_vivo");
        MOTOR_CHECK(pone);
    }
}

// D0029: a estas ramas solo se llega huyendo (D0027: morir reinicia el combate), asi que su texto cuenta una huida y no una caida.
MOTOR_TEST_CASE(las_ramas_a_las_que_se_llega_huyendo_cuentan_una_huida) {
    Fabrica f;
    const struct {
        const RPG::AdventureScript* guion;
        const char* beat;
        const char* dice;
    } ramas[] = {
        {&f.g.gorran, "bgd_perdida", "Echas a correr"},   {&f.g.gorran, "bgb_detras_perdida", "Echas a correr"},
        {&f.g.dia2, "b2_saqueadores_ganan", "corres"},    {&f.g.dia1, "b1_secta_ganan", "Te escapas"},
        {&f.g.ocaso, "b3_sandes_gana_casco", "corres"},
        {&f.g.ocaso, "b3_sandes_sigue", "Corres"},
    };
    for (const auto& rama : ramas) {
        bool visto = false;
        for (const RPG::NarrativeBeat& beat : rama.guion->beats) {
            if (beat.id != rama.beat) continue;
            visto = true;
            std::string texto;
            for (const std::string& l : beat.lines) texto += l + "\n";
            MOTOR_CHECK(texto.find(rama.dice) != std::string::npos);
            for (const char* caida : {"Caes", "Despiertas", "levantas", "levantarte"}) MOTOR_CHECK(texto.find(caida) == std::string::npos);
        }
        MOTOR_CHECK(visto);
    }
    // El diario cuenta la huida con la flag que de verdad se enciende.
    std::ifstream quests("assets/catalogs/quests.json");
    const std::string diario((std::istreambuf_iterator<char>(quests)), std::istreambuf_iterator<char>());
    MOTOR_CHECK(diario.find("\"bnd1_guarida_huida\"") != std::string::npos);
    // D0031: de los saqueadores de la Barriada ya no se huye, asi que no hay entrada de huir ni de perder.
    for (const char* muerta : {"\"bnd1_guarida_perdida\"", "\"bnd2_saqueadores_ganan\"", "\"bnd2_saqueadores_huido\""}) MOTOR_CHECK(diario.find(muerta) == std::string::npos);
}

// E-HALLAZGOS-01 (hallazgo 4): quedarse a luchar en la base pone bnd3_ruta_quedarse y, al caer la base (b3_base_cae), tambien bnd3_ruta_oeste. El
// diario cuenta lo que elegiste («te quedaste»), no solo hacia donde acabaste huyendo.
MOTOR_TEST_CASE(quedarse_en_la_base_se_ve_en_el_diario_como_quedarse_y_no_como_ir_al_oeste) {
    Fabrica f;
    const DiarioDelOcaso quedarse = diarioDelOcaso(f.g, {"bnd3_ruta", "bnd3_ruta_quedarse", "bnd3_ruta_oeste", "bnd3_base_caida"});
    MOTOR_CHECK(quedarse.ruta.find("quedaste") != std::string::npos);
    MOTOR_CHECK(quedarse.ruta.find("oeste") == std::string::npos);
    const DiarioDelOcaso oeste = diarioDelOcaso(f.g, {"bnd3_ruta", "bnd3_ruta_oeste"});
    MOTOR_CHECK(oeste.ruta.find("oeste") != std::string::npos);
    const DiarioDelOcaso norte = diarioDelOcaso(f.g, {"bnd3_ruta", "bnd3_ruta_norte"});
    MOTOR_CHECK(norte.ruta.find("norte") != std::string::npos);
    // El juego pone las dos flags al caer la base tras quedarse (b3_base_cae), como dice la ficha.
    bool poneOeste = false;
    for (const RPG::NarrativeBeat& b : f.g.ocaso.beats)
        if (b.id == "b3_base_cae")
            for (const RPG::NarrativeEffect& e : b.effects) poneOeste = poneOeste || (e.type == RPG::EffectType::SetFlag && e.arg == "bnd3_ruta_oeste");
    MOTOR_CHECK(poneOeste);
}

// E-HALLAZGOS-01 (hallazgo 6): con Venides sin aviso (bnd2_venides_sin_aviso: salio solo a Las Afueras) nadie puede avisarle; al sonar las campanas
// el evento venides_retenido pone bnd3_aviso_resuelto y el diario da el objetivo por cerrado (antes se quedaba en 2/3 para siempre). Avisarle de
// verdad (b3_avisar_venides) tambien lo pone.
MOTOR_TEST_CASE(con_venides_sin_aviso_el_diario_cierra_el_objetivo_de_avisar_al_sonar_las_campanas) {
    Fabrica f;
    demo_bnd::Jugador j(f.g, demo_bnd::Suerte::Buena);
    fase(j, "ocaso");
    j.s.setFlag("bnd2_dia_cerrado");
    j.s.setFlag("bnd2_venides_sin_aviso");
    j.s.setVariable("dia", 4);
    j.s.setVariable("franja", 2);
    MOTOR_CHECK(!j.s.hasFlag("bnd3_aviso_resuelto"));
    for (int i = 0; i < 4 && j.evaluaEventos(); ++i) {}
    MOTOR_CHECK(j.s.hasFlag("bnd3_campanas") && j.s.hasFlag("bnd3_sin_venides"));
    MOTOR_CHECK(j.s.hasFlag("bnd3_aviso_resuelto") && !j.s.hasFlag("bnd3_avisado"));
    const std::vector<const RPG::AdventureScript*> campana = {&f.g.prologo, &f.g.dia1, &f.g.gorran, &f.g.dia2, &f.g.ocaso};
    const Game::DiarioDatos d = Game::construirDiario(campana, &f.g.ocaso, j.s);
    bool avisarHecho = false;
    for (const Game::DiarioMision& m : d.misiones)
        if (m.id == "adv_boundington_ocaso")
            for (const Game::DiarioObjetivo& o : m.objetivos)
                if (o.id == "obj3_avisar") avisarHecho = o.hecho;
    MOTOR_CHECK(avisarHecho);
    // Con Venides en su sitio y sin avisar, el objetivo sigue pendiente hasta que se le avisa.
    demo_bnd::Jugador k(f.g, demo_bnd::Suerte::Buena);
    fase(k, "ocaso");
    k.s.setFlag("bnd2_dia_cerrado");
    k.s.setVariable("dia", 4);
    k.s.setVariable("franja", 1);
    for (int i = 0; i < 4 && k.evaluaEventos(); ++i) {}
    MOTOR_CHECK(!k.s.hasFlag("bnd3_aviso_resuelto") && !k.s.hasFlag("bnd3_sin_venides"));
}

// E-DISPARADORES-3 (decision de Oriol, 04/11: «Portar-hi per dialeg»): la salida del Ocaso se elige en el DIALOGO de la base
// (b3_por_donde) y el juego lleva al jugador a la casilla de su cinematica base_ruta_* (disparador.lleva) y la arranca. Sin elegir, no
// salta ninguna; elegida una, solo salta esa; el jugador acaba en la casilla (camino real de GameSession::rutaAndando) y la cinematica pone
// su ruta. El norte no tiene casilla transitable en su zona: lleva.a es una casilla de la calle junto a su paso.
MOTOR_TEST_CASE(elegir_la_salida_en_el_dialogo_lleva_al_jugador_y_arranca_la_cinematica) {
    Fabrica f;
    auto cat = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
    MOTOR_CHECK(cat.isOk());
    if (!cat.isOk()) return;
    const std::string nivel = "assets/levels/ciudad_oeste.json";
    const struct { const char* cine; const char* ruta; const char* flag; GridCoord casilla; } rutas[] = {
        {"base_ruta_oeste", "oeste", "bnd3_elige_oeste", {3, 12}},
        {"base_ruta_norte", "norte", "bnd3_elige_norte", {28, 13}},
        {"base_ruta_quedarse", "quedarse", "bnd3_elige_quedarse", {12, 12}}};
    for (const auto& r : rutas) {
        demo_bnd::Jugador j(f.g, demo_bnd::Suerte::Buena);
        fase(j, "ocaso");
        for (const char* fl : {"bnd2_dia_cerrado", "bnd3_arenga"}) j.s.setFlag(fl);
        j.rutaOcaso = r.cine;
        j.entra("interior_cuartel");
        j.drena();  // b3_por_donde: el menu de las tres salidas; contesta la de rutaOcaso
        MOTOR_CHECK(j.visto("b3_por_donde"));
        MOTOR_CHECK(j.s.hasFlag("bnd3_salidas_explicadas") && !j.s.hasFlag("bnd3_ruta"));
        for (const auto& otra : rutas) MOTOR_CHECK(j.s.hasFlag(otra.flag) == (otra.flag == std::string(r.flag)));
        // En ciudad_oeste (desde la puerta de la base, (8, 11)): solo salta la elegida, con el radio y sin vision.
        j.entra("ciudad_oeste");
        const auto tiene = [&j](const std::string& fl) { return j.s.hasFlag(fl); };
        const Game::VistaDelNivel vista;  // exterior, nada tapa
        const Game::Cinematica* c = cat.value().laQueEmpieza(nivel, GridCoord{8, 11}, tiene, &vista);
        MOTOR_CHECK(c != nullptr && c->id == r.cine);
        if (c == nullptr || c->id != r.cine) continue;
        MOTOR_CHECK(c->puntoDeEntrada().x == r.casilla.x && c->puntoDeEntrada().y == r.casilla.y);
        // El camino: el de la sesion real sobre el mapa real; acaba EN la casilla y no es un salto.
        const RPG::CharacterSheet ficha = creaHeroe().ficha;
        auto montado = montaNivel("ciudad_oeste", ficha);
        const GridCoord desde = montado->nivel.playerStart;
        Game::LlevaDelJugador lleva;
        lleva.jugador = desde;
        lleva.ruta = [&](const GridCoord& d) { return montado->sesion->rutaAndando(d); };
        const std::vector<Game::PasoCinematica> pasos = cat.value().pasosEfectivos(*c, &lleva);
        MOTOR_CHECK(pasos.size() >= 3);
        if (pasos.size() < 3) continue;
        MOTOR_CHECK(pasos.front().tipo == Game::PasoCinematica::Tipo::Anda && pasos.front().actor == "jugador");
        MOTOR_CHECK(!pasos.front().camino.empty() && pasos.front().camino.back().x == r.casilla.x && pasos.front().camino.back().y == r.casilla.y);
        MOTOR_CHECK(pasos.back().tipo == Game::PasoCinematica::Tipo::Beat);
        // La cinematica pone su ruta (el beat «Vamos») y nada mas se decide: bnd3_ruta_<x>.
        MOTOR_CHECK(j.cinematica(c->puntoDeEntrada()));
        MOTOR_CHECK(j.s.hasFlag("bnd3_ruta") && j.s.hasFlag(std::string("bnd3_ruta_") + r.ruta));
        MOTOR_CHECK(j.s.hasFlag(c->vista()));
        std::fprintf(stderr, "  [%s] elige «%s», %d pasos hasta (%d,%d), ruta %s\n", r.cine, r.flag, static_cast<int>(pasos.front().camino.size()), r.casilla.x, r.casilla.y, r.ruta);
    }
    // Sin elegir (b3_por_donde aun no ha preguntado), ninguna de las tres salta, ni en su casilla ni en la puerta.
    std::set<std::string> flags = {"bnd3_salidas_explicadas", "bnd3_arenga"};
    const auto sin = [&flags](const std::string& fl) { return flags.count(fl) != 0; };
    const Game::VistaDelNivel vista;
    for (const GridCoord donde : {GridCoord{8, 11}, GridCoord{3, 12}, GridCoord{28, 13}, GridCoord{12, 12}, GridCoord{4, 10}, GridCoord{7, 7}}) {
        const Game::Cinematica* c = cat.value().laQueEmpieza(nivel, donde, sin, &vista);
        MOTOR_CHECK(c == nullptr || c->id.rfind("base_ruta_", 0) != 0);
    }
}

int main() { return motor_test::runAllTests(); }
