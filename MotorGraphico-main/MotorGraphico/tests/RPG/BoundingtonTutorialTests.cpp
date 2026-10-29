// Tutorial de Boundington en la zona principal — la cadena de datos que
// anade boundington_primer_dia.json: presentarse a los tres comerciantes de
// la plaza y, al volver, la emboscada de los Perdidos con su combate.
//
// El motor narrativo es GL-free y NO resuelve el combate: lo empaqueta en
// NarrativeResult.battles, y las flags de victoria/derrota las enciende
// GameSession cuando se cierra el BattleState (FORMATO_AVENTURAS.md, "la
// diferencia con skillCheck: el tiempo"). Aqui ese "despues" se simula
// encendiendo la flag a mano, igual que demo_campana_boundington, porque lo
// que se prueba es que la cadena ramifica y se cierra, no el combate.

#include "RPG/NarrativeEngine.h"
#include "Game/Cinematica.h"

#include "TestHarness.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using RPG::AdventureScript;
using RPG::BattleRequest;
using RPG::NarrativeEngine;
using RPG::NarrativeResult;
using RPG::NarrativeState;

const char* const kPlaza = "assets/levels/ciudad_centro.json";
const char* const kGuion = "assets/adventures/boundington_primer_dia.json";

AdventureScript carga() {
    const auto r = AdventureScript::loadFromFile(kGuion);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

NarrativeEngine motor(const AdventureScript& guion) {
    NarrativeEngine e;
    e.setAdventure(&guion);
    return e;
}

// Llegar a la plaza y dejar que el encargo del tutorial salte solo: los beats
// auto encadenan de uno en uno, asi que hace falta un tick despues del enter.
void llegaYRecibeElEncargo(const NarrativeEngine& e, NarrativeState& s) {
    const NarrativeResult llegada = e.enterLevel(kPlaza, s);
    MOTOR_CHECK(llegada.fired);
    MOTOR_CHECK(llegada.beatId == "beat_llegada");
    MOTOR_CHECK(s.hasFlag("bnd_llegada"));

    const NarrativeResult encargo = e.tick(s);
    MOTOR_CHECK(encargo.fired);
    MOTOR_CHECK(encargo.beatId == "tut_encargo");
    MOTOR_CHECK(s.hasFlag("bnd_tut_encargo_dado"));
}

void hablaCon(const NarrativeEngine& e, NarrativeState& s, const char* npc) {
    const NarrativeResult r = e.talkTo(npc, s);
    MOTOR_CHECK(r.fired);
}

// Cadena completa hasta la emboscada. Devuelve la peticion de combate para
// que cada caso decida como termina.
BattleRequest hastaLaEmboscada(const NarrativeEngine& e, NarrativeState& s) {
    llegaYRecibeElEncargo(e, s);
    hablaCon(e, s, "tendero_mercado");
    hablaCon(e, s, "joyero");
    hablaCon(e, s, "sastre");
    MOTOR_CHECK(s.hasFlag("bnd_tut_tendero"));
    MOTOR_CHECK(s.hasFlag("bnd_tut_joyero"));
    MOTOR_CHECK(s.hasFlag("bnd_tut_sastre"));

    const NarrativeResult emboscada = e.enterLevel(kPlaza, s);
    MOTOR_CHECK(emboscada.fired);
    MOTOR_CHECK(emboscada.beatId == "tut_emboscada");
    MOTOR_CHECK(emboscada.battles.size() == 1);
    return emboscada.battles[0];
}

MOTOR_TEST_CASE(los_tres_comerciantes_abren_la_emboscada_y_pide_un_solo_combate) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;

    const BattleRequest combate = hastaLaEmboscada(e, s);
    MOTOR_CHECK(combate.monsterId == "perdido_saqueador");
    // Las dos flags son obligatorias (FORMATO_AVENTURAS.md): un combate del
    // que no se puede saber como acabo es contenido que no ramifica.
    MOTOR_CHECK(!combate.flagVictory.empty());
    MOTOR_CHECK(!combate.flagDefeat.empty());
    MOTOR_CHECK(combate.flagVictory != combate.flagDefeat);
    // El primer objetivo del tutorial se cumple justo aqui.
    MOTOR_CHECK(s.hasFlag("bnd_tut_emboscada_lanzada"));
}

MOTOR_TEST_CASE(la_victoria_cierra_el_tutorial_y_marca_los_dos_objetivos) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;

    const BattleRequest combate = hastaLaEmboscada(e, s);
    s.setFlag(combate.flagVictory);          // lo que haria syncBattleOutcome

    const NarrativeResult cierre = e.tick(s);
    MOTOR_CHECK(cierre.fired);
    MOTOR_CHECK(cierre.beatId == "tut_victoria");
    MOTOR_CHECK(cierre.goldDelta == 0);
    MOTOR_CHECK(s.hasFlag("bnd_tut_cerrado"));
    MOTOR_CHECK(s.hasFlag("bnd_tut_emboscada_resuelta"));

    // MAPA-01 (14/10): los tres comerciantes son tres objetivos, cada uno con su tienda.
    int comerciantesHechos = 0;
    bool atraco = false;
    for (const auto& objetivo : e.objectives(s)) {
        if (objetivo.id == "obj_tut_tendero" || objetivo.id == "obj_tut_joyero" || objetivo.id == "obj_tut_sastre")
            comerciantesHechos += objetivo.done ? 1 : 0;
        if (objetivo.id == "obj_tut_emboscada") atraco = objetivo.done;
    }
    const bool comerciantes = comerciantesHechos == 3;
    MOTOR_CHECK(comerciantes);
    MOTOR_CHECK(atraco);
}

MOTOR_TEST_CASE(la_derrota_cierra_el_tutorial_y_cuesta_monedas) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;

    const BattleRequest combate = hastaLaEmboscada(e, s);
    s.setFlag(combate.flagDefeat);

    // Perder no puede dejar el tutorial a medias: cierra igual, con coste.
    const NarrativeResult cierre = e.tick(s);
    MOTOR_CHECK(cierre.fired);
    MOTOR_CHECK(cierre.beatId == "tut_derrota");
    MOTOR_CHECK(cierre.goldDelta == -15);
    MOTOR_CHECK(s.hasFlag("bnd_tut_cerrado"));
    MOTOR_CHECK(s.hasFlag("bnd_tut_emboscada_resuelta"));
}

MOTOR_TEST_CASE(sin_los_tres_comerciantes_no_hay_emboscada) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;

    llegaYRecibeElEncargo(e, s);
    hablaCon(e, s, "tendero_mercado");
    hablaCon(e, s, "joyero");

    const NarrativeResult vuelta = e.enterLevel(kPlaza, s);
    MOTOR_CHECK(vuelta.beatId != "tut_emboscada");
    MOTOR_CHECK(vuelta.battles.empty());
    MOTOR_CHECK(!s.hasFlag("bnd_tut_emboscada_lanzada"));
}

MOTOR_TEST_CASE(el_orden_en_que_se_visita_a_los_comerciantes_no_importa) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;

    llegaYRecibeElEncargo(e, s);
    hablaCon(e, s, "sastre");
    hablaCon(e, s, "tendero_mercado");
    hablaCon(e, s, "joyero");

    const NarrativeResult emboscada = e.enterLevel(kPlaza, s);
    MOTOR_CHECK(emboscada.beatId == "tut_emboscada");
    MOTOR_CHECK(emboscada.battles.size() == 1);
}

MOTOR_TEST_CASE(el_tutorial_no_se_repite_una_vez_cerrado) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s;

    const BattleRequest combate = hastaLaEmboscada(e, s);
    s.setFlag(combate.flagVictory);
    MOTOR_CHECK(e.tick(s).beatId == "tut_victoria");

    // Ni la emboscada ni el cierre vuelven a saltar: los notFlags los vetan.
    const NarrativeResult otraVez = e.enterLevel(kPlaza, s);
    MOTOR_CHECK(otraVez.beatId != "tut_emboscada");
    MOTOR_CHECK(otraVez.battles.empty());
    MOTOR_CHECK(e.tick(s).beatId != "tut_victoria");
}

// G4-02: la noche del dia 0. «Subir a dormir» (bnd0_a_dormir) ya no duerme al
// instante: salta la cinematica taberna_rosita_puja, que deja al jugador en
// interior_taberna_p1 y enciende bnd0_rosita_te_sube; solo entonces b0_subir
// (o b0_subir_oido) cuenta la noche. Aqui se ejecuta la cinematica de verdad
// (catalogo real, sus pasos como hace el juego: beats y flags a la historia).
const char* const kTaberna = "assets/levels/interior_taberna.json";
const char* const kTabernaP1 = "assets/levels/interior_taberna_p1.json";

// Deja correr los beats automaticos; devuelve los ids que han saltado.
std::vector<std::string> drena(const NarrativeEngine& e, NarrativeState& s) {
    std::vector<std::string> ids;
    for (int i = 0; i < 100; ++i) {
        const NarrativeResult r = e.tick(s);
        if (!r.fired) break;
        ids.push_back(r.beatId);
    }
    return ids;
}

bool contiene(const std::vector<std::string>& v, const std::string& id) {
    for (const std::string& x : v) if (x == id) return true;
    return false;
}

// Estado exacto de la nit 0 justo despues de pagar y elegir «Subir a dormir».
NarrativeState nocheCero(bool oido) {
    NarrativeState s;
    s.setFlag("bnd0_noche");
    s.setFlag("bnd0_cama");
    s.setFlag("bnd0_a_dormir");
    if (oido) s.setFlag("bnd0_oido_por_devoto");
    return s;
}

Game::CatalogoCinematicas catalogo() {
    auto r = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
    MOTOR_CHECK(r.isOk());
    return r.value();
}

// Ejecuta la cinematica que empieza en la taberna; devuelve el nivel en que
// acaba el jugador ("" si no empieza ninguna).
std::string cinematicaDeRosita(const Game::CatalogoCinematicas& cat, const NarrativeEngine& e, NarrativeState& s,
                               std::vector<std::string>* beats) {
    const auto tiene = [&s](const std::string& f) { return s.hasFlag(f); };
    const Game::Cinematica* c = cat.laQueEmpieza(kTaberna, GridCoord{-1, -1}, tiene);
    if (c == nullptr) return "";
    MOTOR_CHECK(c->id == "taberna_rosita_puja");
    std::string nivel = kTaberna;
    for (const Game::PasoCinematica& p : c->pasos) {
        if (p.tipo == Game::PasoCinematica::Tipo::Nivel) nivel = p.texto;
        if (p.tipo == Game::PasoCinematica::Tipo::Beat || p.tipo == Game::PasoCinematica::Tipo::Flag) {
            s.setFlag(p.texto);
            const auto ids = drena(e, s);
            if (beats) beats->insert(beats->end(), ids.begin(), ids.end());
        }
    }
    s.setFlag(c->vista());
    for (const std::string& f : c->alTerminar) s.setFlag(f);
    return nivel;
}

MOTOR_TEST_CASE(sin_te_sube_la_noche_cero_no_se_duerme_y_con_la_cinematica_si) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    const Game::CatalogoCinematicas cat = catalogo();
    NarrativeState s = nocheCero(false);

    // Antes de la cinematica: nada duerme, ni b0_subir ni b0_subir_oido.
    const auto antes = drena(e, s);
    MOTOR_CHECK(!contiene(antes, "b0_subir"));
    MOTOR_CHECK(!contiene(antes, "b0_subir_oido"));
    MOTOR_CHECK(!s.hasFlag("bnd0_dormido"));

    // La cinematica empieza, se ve a Rosita arriba y acaba en p1.
    const auto tiene = [&s](const std::string& f) { return s.hasFlag(f); };
    MOTOR_CHECK(!cat.visible("skilla", kTabernaP1, tiene));
    std::vector<std::string> durante;
    MOTOR_CHECK(cinematicaDeRosita(cat, e, s, &durante) == kTabernaP1);
    MOTOR_CHECK(contiene(durante, "b0_rosita_sube"));     // su linea, ya arriba
    MOTOR_CHECK(!contiene(durante, "b0_subir"));          // aun no duerme
    MOTOR_CHECK(cat.visible("skilla", kTabernaP1, tiene)); // Rosita esta arriba
    MOTOR_CHECK(s.hasFlag("bnd0_rosita_te_sube"));
    MOTOR_CHECK(!s.hasFlag("bnd0_dormido"));

    // Al acabar: la noche y el amanecer; Rosita ya no esta arriba.
    const auto despues = drena(e, s);
    MOTOR_CHECK(contiene(despues, "b0_subir"));
    MOTOR_CHECK(!contiene(despues, "b0_subir_oido"));
    MOTOR_CHECK(s.hasFlag("bnd0_dormido"));
    MOTOR_CHECK(s.variable("dia") == 1);
    MOTOR_CHECK(s.variable("franja") == 0);
    MOTOR_CHECK(!cat.visible("skilla", kTabernaP1, tiene));

    // No se repite la cinematica.
    MOTOR_CHECK(cinematicaDeRosita(cat, e, s, nullptr).empty());
}

// I-TXT-1: la mesa vacia y la escalera se dicen DURANTE la subida (antes de
// la fosa que cambia de nivel); la cama / el circulo y el «Duermes» arriba.
MOTOR_TEST_CASE(al_subir_la_mesa_vacia_sale_antes_del_cambio_de_nivel_y_la_cama_despues) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    const Game::CatalogoCinematicas cat = catalogo();
    for (bool oido : {false, true}) {
        NarrativeState s = nocheCero(oido);
        drena(e, s);  // lo que ya habia saltado antes de subir
        const auto tiene = [&s](const std::string& f) { return s.hasFlag(f); };
        const Game::Cinematica* c = cat.laQueEmpieza(kTaberna, GridCoord{-1, -1}, tiene);
        MOTOR_CHECK(c != nullptr);
        std::string nivel = kTaberna;
        std::string antes, despues;
        bool mesaAbajo = false, escaleraAbajo = false, camaAbajo = false;
        for (const Game::PasoCinematica& p : c->pasos) {
            if (p.tipo == Game::PasoCinematica::Tipo::Nivel) nivel = p.texto;
            if (p.tipo != Game::PasoCinematica::Tipo::Beat && p.tipo != Game::PasoCinematica::Tipo::Flag) continue;
            s.setFlag(p.texto);
            for (int i = 0; i < 20; ++i) {
                const NarrativeResult r = e.tick(s);
                if (!r.fired) break;
                std::string todo;
                for (const std::string& l : r.lines) todo += l + "\n";
                if (nivel == kTaberna) {
                    antes += todo;
                    MOTOR_CHECK(r.beatId == (oido ? "b0_subiendo_oido" : "b0_subiendo"));
                } else {
                    despues += todo;
                }
            }
        }
        mesaAbajo = antes.find("la mesa de la puerta está vacía") != std::string::npos;
        escaleraAbajo = antes.find("La escalera cruje en el tercer peldaño") != std::string::npos;
        camaAbajo = antes.find("La cama huele") != std::string::npos || antes.find("Duermes") != std::string::npos ||
                    antes.find("círculo") != std::string::npos;
        MOTOR_CHECK(mesaAbajo);
        MOTOR_CHECK(escaleraAbajo);
        MOTOR_CHECK(!camaAbajo);
        MOTOR_CHECK(antes.find("por si alguna noche te conviene bajar") != std::string::npos || oido);
        s.setFlag(c->vista());
        for (const std::string& f : c->alTerminar) s.setFlag(f);
        // arriba (tras la linea de Rosita): la cama / el circulo, sin repetir la mesa.
        for (const std::string& id : drena(e, s)) { (void)id; }
        MOTOR_CHECK(s.hasFlag("bnd0_dormido"));
    }
    // texto de arriba, con la cinematica ya hecha
    for (bool oido : {false, true}) {
        NarrativeState s = nocheCero(oido);
        s.setFlag("bnd0_subiendo");
        s.setFlag("bnd0_rosita_te_sube");
        std::string todo;
        for (int i = 0; i < 20; ++i) {
            const NarrativeResult r = e.tick(s);
            if (!r.fired) break;
            for (const std::string& l : r.lines) todo += l + "\n";
        }
        MOTOR_CHECK(todo.find("la mesa de la puerta está vacía") != std::string::npos);  // la dice b0_subiendo*
        MOTOR_CHECK(oido ? todo.find("En el polvo de tu puerta") != std::string::npos
                         : todo.find("La cama huele a lavanda") != std::string::npos);
        MOTOR_CHECK(oido ? todo.find("Duermes mal. Pero duermes.") != std::string::npos : true);
    }
}

MOTOR_TEST_CASE(la_noche_cero_con_el_oido_por_devoto_usa_b0_subir_oido) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    const Game::CatalogoCinematicas cat = catalogo();
    NarrativeState s = nocheCero(true);

    MOTOR_CHECK(!contiene(drena(e, s), "b0_subir_oido"));
    MOTOR_CHECK(!s.hasFlag("bnd0_dormido"));
    MOTOR_CHECK(cinematicaDeRosita(cat, e, s, nullptr) == kTabernaP1);
    const auto despues = drena(e, s);
    MOTOR_CHECK(contiene(despues, "b0_subir_oido"));
    MOTOR_CHECK(s.hasFlag("bnd0_dormido"));
    MOTOR_CHECK(s.variable("dia") == 1);
}

MOTOR_TEST_CASE(el_desayuno_no_empieza_al_bajar_a_la_taberna_sino_al_llamarte_y_no_se_repite) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    NarrativeState s = nocheCero(false);
    s.setFlag("bnd0_rosita_te_sube");
    drena(e, s);
    MOTOR_CHECK(s.hasFlag("bnd0_dormido"));
    // Arriba (p1) no salta, y tampoco con un tick suelto.
    MOTOR_CHECK(!s.hasFlag("bnd1_desayuno_visto"));
    MOTOR_CHECK(e.enterLevel(kTabernaP1, s).beatId != "b1_desayuno");
    MOTOR_CHECK(!s.hasFlag("bnd1_desayuno_visto"));
    // H-DIA1-1: al bajar a la taberna tampoco (Venides te llama cuando pasas a su lado).
    const NarrativeResult baja = e.enterLevel(kTaberna, s);
    MOTOR_CHECK(baja.beatId != "b1_desayuno" && baja.beatId != "b1_desayuno_v0");
    MOTOR_CHECK(!e.tick(s).fired);
    MOTOR_CHECK(!s.hasFlag("bnd1_desayuno_visto"));
    // La cinematica venides_te_sienta enciende bnd1_desayuno_llamado: ahora si.
    s.setFlag("bnd1_desayuno_llamado");
    const NarrativeResult come = e.tick(s);
    MOTOR_CHECK(come.fired);
    MOTOR_CHECK(come.beatId == "b1_desayuno" || come.beatId == "b1_desayuno_v0");
    MOTOR_CHECK(s.hasFlag("bnd1_desayuno_visto"));
    // Volver a entrar no lo repite.
    const NarrativeResult otra = e.enterLevel(kTaberna, s);
    MOTOR_CHECK(otra.beatId != "b1_desayuno" && otra.beatId != "b1_desayuno_v0");
}

// G4-03: Venides se va despues de desayunar y no se le ve en la taberna hasta
// la noche del dia 1. Las apariciones solo miran flags, asi que la noche la
// avisa bnd1_venides_nit, que encienden a la vez que pasarFranja noche los
// beats b1_skilla_hora y b1_posadero_hora. Catalogo y guion reales.
bool venidesSeVe(const Game::CatalogoCinematicas& cat, const NarrativeState& s) {
    return cat.visible("venides", kTaberna, [&s](const std::string& f) { return s.hasFlag(f); });
}

void eligeTexto(const NarrativeEngine& e, NarrativeState& s, const std::string& texto) {
    const std::vector<std::string> t = s.eleccion().textos;
    for (std::size_t i = 0; i < t.size(); ++i) {
        if (t[i] != texto) continue;
        e.elige(static_cast<int>(i), s);
        return;
    }
    MOTOR_CHECK(false && "no esta esa opcion");
}

// De la cama al «Ponerse en marcha» del desayuno (la mañana del dia 1).
void desayunaYSalePorLaCalle(const NarrativeEngine& e, NarrativeState& s) {
    s = nocheCero(false);
    s.setFlag("bnd0_rosita_te_sube");
    drena(e, s);  // b0_subir: dia 1, mañana, bnd0_dormido
    MOTOR_CHECK(s.hasFlag("bnd0_dormido"));
    s.setFlag("bnd1_desayuno_llamado");  // venides_llama + venides_te_sienta (H-DIA1-1)
    MOTOR_CHECK(e.tick(s).fired);
    eligeTexto(e, s, "Ponerse en marcha");
    MOTOR_CHECK(s.hasFlag("bnd1_desayuno_hecho"));
}

// Ejecuta venides_se_va; "antes" recoge lo que se ve de Venides en cada paso.
// Devuelve cuantos pasos se vieron con Venides visible y cuantos no.
struct Marcha { bool empezo = false; int conVenides = 0; int sinVenides = 0; bool visibleAntesDeIrse = false; };
Marcha marcha(const Game::CatalogoCinematicas& cat, NarrativeState& s) {
    Marcha m;
    const auto tiene = [&s](const std::string& f) { return s.hasFlag(f); };
    const Game::Cinematica* c = cat.laQueEmpieza(kTaberna, GridCoord{-1, -1}, tiene);
    if (c == nullptr) return m;
    MOTOR_CHECK(c->id == "venides_se_va");
    m.empezo = true;
    m.visibleAntesDeIrse = venidesSeVe(cat, s);
    for (const Game::PasoCinematica& p : c->pasos) {
        if (p.tipo == Game::PasoCinematica::Tipo::Flag) s.setFlag(p.texto);
        (venidesSeVe(cat, s) ? m.conVenides : m.sinVenides)++;
    }
    s.setFlag(c->vista());
    for (const std::string& f : c->alTerminar) s.setFlag(f);
    return m;
}

MOTOR_TEST_CASE(tras_el_desayuno_venides_cruza_la_taberna_y_ya_no_esta) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    const Game::CatalogoCinematicas cat = catalogo();
    NarrativeState s;
    desayunaYSalePorLaCalle(e, s);
    drena(e, s);

    // Hasta que se va, sigue en su mesa; la cinematica lo ve salir y despues no esta.
    const Marcha m = marcha(cat, s);
    MOTOR_CHECK(m.empezo);
    MOTOR_CHECK(m.visibleAntesDeIrse);
    MOTOR_CHECK(m.conVenides > 0);
    MOTOR_CHECK(m.sinVenides > 0);
    MOTOR_CHECK(!venidesSeVe(cat, s));
    // No se repite, y de dia el resto del dia no vuelve.
    MOTOR_CHECK(marcha(cat, s).empezo == false);
    MOTOR_CHECK(!venidesSeVe(cat, s));
    // Solo se esconde en la taberna: los demas niveles no cambian.
    MOTOR_CHECK(cat.visible("venides", kTabernaP1, [&s](const std::string& f) { return s.hasFlag(f); }));
}

// Las cuatro opciones del guion que pasan a la noche el dia 1.
void pasaALaNoche(const NarrativeEngine& e, NarrativeState& s, int via) {
    const bool skilla = via < 2;
    if (skilla) {
        s.setFlag("bnd_skilla");
        s.setFlag("bnd_skilla_confianza");
    } else {
        s.setFlag("bnd_llegada");
    }
    MOTOR_CHECK(e.talkTo(skilla ? "skilla" : "posadero", s).fired);
    if (via % 2 == 0) {
        eligeTexto(e, s, "Esperar a que anochezca");
    } else {
        s.setVariable("franja", 1);  // «Subir a dormir» pide al menos la tarde
        MOTOR_CHECK(e.talkTo(skilla ? "skilla" : "posadero", s).fired);
        eligeTexto(e, s, "Subir a dormir");
    }
}

MOTOR_TEST_CASE(venides_vuelve_a_la_taberna_cuando_pasa_a_la_noche_por_cualquier_via) {
    const AdventureScript guion = carga();
    const NarrativeEngine e = motor(guion);
    const Game::CatalogoCinematicas cat = catalogo();
    for (int via = 0; via < 4; ++via) {
        NarrativeState s;
        desayunaYSalePorLaCalle(e, s);
        drena(e, s);
        MOTOR_CHECK(marcha(cat, s).empezo);
        MOTOR_CHECK(!venidesSeVe(cat, s));
        pasaALaNoche(e, s, via);
        MOTOR_CHECK(s.variable("franja") == 2);
        MOTOR_CHECK(s.hasFlag("bnd1_venides_nit"));
        MOTOR_CHECK(venidesSeVe(cat, s));
        // Y se le puede hablar: la charla de la noche sigue siendo abastable.
        const NarrativeResult r = e.talkTo("venides", s);
        MOTOR_CHECK(r.fired);
        MOTOR_CHECK(r.beatId == "b1_borracho");
    }
    // De dia no basta con esperar la tarde: sigue fuera.
    NarrativeState t;
    desayunaYSalePorLaCalle(e, t);
    drena(e, t);
    MOTOR_CHECK(marcha(cat, t).empezo);
    t.setFlag("bnd_llegada");
    MOTOR_CHECK(e.talkTo("posadero", t).fired);
    eligeTexto(e, t, "Esperar a la tarde");
    MOTOR_CHECK(t.variable("franja") == 1);
    MOTOR_CHECK(!venidesSeVe(cat, t));
    // Si es de noche, la cinematica de marcha no salta.
    NarrativeState n;
    desayunaYSalePorLaCalle(e, n);
    n.setFlag("bnd1_venides_nit");
    MOTOR_CHECK(!marcha(cat, n).empezo);
    MOTOR_CHECK(venidesSeVe(cat, n));
}

MOTOR_TEST_CASE(durante_la_marcha_solo_hay_un_venides) {
    const Game::CatalogoCinematicas cat = catalogo();
    const Game::Cinematica* c = cat.busca("venides_se_va");
    MOTOR_CHECK(c != nullptr);
    if (c == nullptr) return;
    // La cinematica mueve al propio venides: nada de un doble que salga mientras el otro se funde.
    int pasosDeVenides = 0;
    for (const Game::PasoCinematica& p : c->pasos) {
        MOTOR_CHECK(p.tipo != Game::PasoCinematica::Tipo::Coloca);
        MOTOR_CHECK(p.tipo != Game::PasoCinematica::Tipo::Nivel);
        if (!p.actor.empty()) {
            MOTOR_CHECK(p.actor == "venides");
            ++pasosDeVenides;
        }
    }
    MOTOR_CHECK(pasosDeVenides > 0);
    // En la taberna hay un unico objeto «venides» (y ninguna otra variante).
    std::ifstream f("assets/levels/interior_taberna.json");
    std::stringstream ss;
    ss << f.rdbuf();
    const std::string nivel = ss.str();
    int cuantos = 0;
    for (std::size_t i = nivel.find("\"objectId\": \"venides"); i != std::string::npos;
         i = nivel.find("\"objectId\": \"venides", i + 1))
        ++cuantos;
    MOTOR_CHECK(cuantos == 1);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
