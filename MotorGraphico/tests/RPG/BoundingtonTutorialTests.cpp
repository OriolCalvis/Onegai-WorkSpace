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

#include "TestHarness.h"

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

    bool comerciantes = false;
    bool atraco = false;
    for (const auto& objetivo : e.objectives(s)) {
        if (objetivo.id == "obj_tut_comerciantes") comerciantes = objetivo.done;
        if (objetivo.id == "obj_tut_emboscada") atraco = objetivo.done;
    }
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

}  // namespace

int main() { return motor_test::runAllTests(); }
