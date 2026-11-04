// U3-02.E06b — dispatchRegionTriggers: de un trigger de region a la pagina del
// evento que la escucha, sin tocar MapEventRunner. Lo que se defiende:
//  - la pagina activa que escucha EXACTAMENTE esa region (capa + id) y ese
//    tipo de hecho arranca; ninguna otra;
//  - el orden Exit -> Enter -> Stay (D3) llega hasta el arranque;
//  - nunca dos escenas bloqueantes: ni con una viva, ni dos triggers en el
//    mismo despacho, ni un Autorun del mismo frame (se llama DESPUES de
//    MapEventRunner::update, ver la cabecera);
//  - una condicion no evaluable se reporta y no cuenta como falsa.

#include "RPG/RegionTriggerDispatch.h"
#include "RPG/RegionTriggerSupport.h"

#include "TestHarness.h"

#include "Level/ObjectCatalog.h"
#include "Map/MapDocument.h"
#include "RPG/EventCommand.h"
#include "RPG/EventInterpreter.h"
#include "RPG/MapEventRunner.h"
#include "RPG/SessionCommandHost.h"
#include "Render/ICombatant.h"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace region_test;

MOTOR_TEST_CASE(enter_arranca_la_pagina_que_escucha_esa_region) {
    const MapDocument mapa = mapaCon({eventoRegion("trampa", 1, EventTrigger::RegionEnter)});
    Escena e;
    const RegionDispatchResult r = e.despachar(mapa, {disp(Kind::Enter)});
    MOTOR_CHECK(r.started.size() == 1);
    MOTOR_CHECK(r.started[0].eventId == "trampa");
    MOTOR_CHECK(r.started[0].pageIndex == 0);
    MOTOR_CHECK(r.skipped.empty());
    MOTOR_CHECK(e.interprete.isRunning("trampa"));
}

MOTOR_TEST_CASE(solo_arranca_con_la_misma_region_capa_y_hecho) {
    const MapDocument mapa = mapaCon({
        eventoRegion("otra_region", 1, EventTrigger::RegionEnter, "otra"),
        eventoRegion("otra_capa", 2, EventTrigger::RegionEnter, "zona", "reg2"),
        eventoRegion("solo_exit", 3, EventTrigger::RegionExit),
        evento("interact", GridCoord{4, 0}, pagina("interact_p0", EventTrigger::Interact)),
    });
    Escena e;
    const RegionDispatchResult r = e.despachar(mapa, {disp(Kind::Enter)});
    MOTOR_CHECK(r.started.empty());
    MOTOR_CHECK(r.skipped.empty());
    MOTOR_CHECK(e.interprete.active().empty());
    // La misma id en la OTRA capa si es otra region: la escucha solo ese evento.
    const RegionDispatchResult r2 = e.despachar(mapa, {disp(Kind::Enter, "zona", "reg2")});
    MOTOR_CHECK(r2.started.size() == 1);
    MOTOR_CHECK(r2.started[0].eventId == "otra_capa");
}

MOTOR_TEST_CASE(la_pagina_se_elige_por_condiciones) {
    MapEventDocument ev = eventoRegion("puerta", 1, EventTrigger::RegionEnter);
    MapEventCondition c;
    c.kind = EventConditionKind::Flag;
    c.reference = "abierta";
    MapEventPage abierta = pagina("abierta_p", EventTrigger::RegionEnter, "reg", "zona");
    abierta.conditions = {c};
    // Orden: region(sin condicion) primero, la condicionada gana si se cumple.
    ev.pages.push_back(std::move(abierta));
    const MapDocument mapa = mapaCon({ev});
    Escena e;
    RegionDispatchResult r = e.despachar(mapa, {disp(Kind::Enter)});
    MOTOR_CHECK(r.started.size() == 1 && r.started[0].pageIndex == 0);
    e.interprete.cancelAll();
    e.interprete.collectFinished();
    e.estado.setFlag("abierta");
    r = e.despachar(mapa, {disp(Kind::Enter)});
    MOTOR_CHECK(r.started.size() == 1 && r.started[0].pageIndex == 1);
}

MOTOR_TEST_CASE(el_orden_exit_enter_stay_llega_al_arranque) {
    const MapDocument mapa = mapaCon({
        eventoRegion("e_stay", 1, EventTrigger::RegionStay),
        eventoRegion("e_enter", 2, EventTrigger::RegionEnter),
        eventoRegion("e_exit", 3, EventTrigger::RegionExit),
    });
    Regiones::RegionDelta delta;
    const Regiones::RegionRef ref{lyr("reg"), reg("zona")};
    delta.entered = {ref};
    delta.stayed = {ref};
    delta.exited = {ref};
    Regiones::StayBatch stays;
    stays.fired = {ref};
    const std::vector<Regiones::RegionTrigger> orden =
        Regiones::orderRegionEvents(delta, stays, Regiones::RegionPolicy{});
    MOTOR_CHECK(orden.size() == 3);
    Escena e;
    const RegionDispatchResult r = e.despachar(mapa, orden);
    // Una sola escena bloqueante: gana la primera, Exit; Enter y Stay, en ese
    // orden, quedan omitidos.
    MOTOR_CHECK(r.started.size() == 1);
    MOTOR_CHECK(r.started[0].eventId == "e_exit");
    MOTOR_CHECK(r.skipped.size() == 2);
    MOTOR_CHECK(r.skipped[0].eventId == "e_enter");
    MOTOR_CHECK(r.skipped[1].eventId == "e_stay");
    MOTOR_CHECK(r.skipped[0].reason == RegionSkipReason::BlockedByScene);
}

MOTOR_TEST_CASE(con_una_escena_viva_el_trigger_no_arranca) {
    const MapDocument mapa = mapaCon({eventoRegion("trampa", 1, EventTrigger::RegionEnter)});
    Escena e;
    const MapEventDocument intro =
        evento("intro", GridCoord{5, 5}, pagina("intro_p0", EventTrigger::Interact));
    MOTOR_CHECK(e.interprete.start(intro, 0).isOk());
    MOTOR_CHECK(e.interprete.blocksPlayer());

    const RegionDispatchResult r = e.despachar(mapa, {disp(Kind::Enter)});

    MOTOR_CHECK(r.started.empty());
    MOTOR_CHECK(r.skipped.size() == 1);
    MOTOR_CHECK(r.skipped[0].reason == RegionSkipReason::BlockedByScene);
    MOTOR_CHECK(r.skipped[0].eventId == "trampa");
    MOTOR_CHECK(e.interprete.active().size() == 1);
    MOTOR_CHECK(!e.interprete.isRunning("trampa"));
}

MOTOR_TEST_CASE(autorun_y_trigger_en_el_mismo_frame_gana_el_autorun) {
    const MapDocument mapa = mapaCon({
        evento("intro", GridCoord{5, 5}, pagina("intro_p0", EventTrigger::Autorun)),
        eventoRegion("trampa", 1, EventTrigger::RegionEnter),
    });
    Escena e;
    e.runner.setMap(&mapa);
    MapEventFrame f;
    f.playerCell = GridCoord{1, 0};
    f.deltaMs = 16;
    // Orden documentado: primero update(), despues el despacho.
    e.runner.update(f);
    MOTOR_CHECK(e.runner.lastStartedEvent() == "intro");

    const RegionDispatchResult r = e.despachar(mapa, {disp(Kind::Enter)});

    MOTOR_CHECK(r.started.empty());
    MOTOR_CHECK(r.skipped.size() == 1 && r.skipped[0].reason == RegionSkipReason::BlockedByScene);
    MOTOR_CHECK(e.interprete.active().size() == 1);
    MOTOR_CHECK(e.interprete.isRunning("intro"));
}

MOTOR_TEST_CASE(sin_escena_viva_tras_terminar_el_trigger_vuelve_a_arrancar) {
    const MapDocument mapa = mapaCon({eventoRegion("trampa", 1, EventTrigger::RegionEnter)});
    Escena e;
    MOTOR_CHECK(e.despachar(mapa, {disp(Kind::Enter)}).started.size() == 1);
    for (int i = 0; i < 100 && !e.interprete.active().empty(); ++i) {
        e.interprete.tick(100);
    }
    e.interprete.collectFinished();
    MOTOR_CHECK(e.interprete.active().empty());
    MOTOR_CHECK(e.despachar(mapa, {disp(Kind::Enter)}).started.size() == 1);
}

MOTOR_TEST_CASE(una_condicion_no_evaluable_se_reporta_y_no_arranca) {
    MapEventDocument ev = eventoRegion("rota", 1, EventTrigger::RegionEnter);
    MapEventCondition rota;
    rota.kind = EventConditionKind::Item;
    rota.reference = "llave";
    rota.comparison = "~=";
    ev.pages[0].conditions = {rota};
    const MapDocument mapa = mapaCon({ev});
    Escena e;
    const RegionDispatchResult r = e.despachar(mapa, {disp(Kind::Enter)});
    MOTOR_CHECK(r.started.empty());
    MOTOR_CHECK(r.skipped.size() == 1);
    MOTOR_CHECK(r.skipped[0].reason == RegionSkipReason::NotEvaluable);
    MOTOR_CHECK(e.interprete.active().empty());
}

MOTOR_TEST_CASE(sin_triggers_no_hace_nada) {
    const MapDocument mapa = mapaCon({eventoRegion("trampa", 1, EventTrigger::RegionEnter)});
    Escena e;
    const RegionDispatchResult r = e.despachar(mapa, {});
    MOTOR_CHECK(r.started.empty() && r.skipped.empty());
}

MOTOR_TEST_CASE(solo_reporta_no_evaluable_de_eventos_que_escuchan_ese_trigger) {
    MapEventCondition rota;
    rota.kind = EventConditionKind::Item;
    rota.reference = "llave";
    rota.comparison = "~=";
    MapEventDocument ajeno = evento("ajeno", GridCoord{2, 0}, pagina("ajeno_p0", EventTrigger::Interact));
    ajeno.pages[0].conditions = {rota};
    MapEventDocument otra = eventoRegion("otra_reg", 3, EventTrigger::RegionEnter, "otra");
    otra.pages[0].conditions = {rota};
    const MapDocument mapa = mapaCon({ajeno, otra});
    Escena e;
    const RegionDispatchResult r = e.despachar(mapa, {disp(Kind::Enter)});
    MOTOR_CHECK(r.started.empty());
    MOTOR_CHECK(r.skipped.empty());
}

MOTOR_TEST_CASE(already_running_con_un_parallel_vivo_cuya_pagina_pasa_a_ser_de_region) {
    MapEventDocument ev = evento("amb", GridCoord{1, 0}, pagina("amb_p0", EventTrigger::Parallel));
    MapEventCondition c;
    c.kind = EventConditionKind::Flag;
    c.reference = "cambia";
    MapEventPage reg = pagina("amb_p1", EventTrigger::RegionEnter, "reg", "zona");
    reg.conditions = {c};
    ev.pages.push_back(std::move(reg));
    const MapDocument mapa = mapaCon({ev});
    Escena e;
    MOTOR_CHECK(e.interprete.start(ev, 0).isOk());
    MOTOR_CHECK(!e.interprete.blocksPlayer());
    e.estado.setFlag("cambia");
    const RegionDispatchResult r = e.despachar(mapa, {disp(Kind::Enter)});
    MOTOR_CHECK(r.started.empty());
    MOTOR_CHECK(r.skipped.size() == 1);
    MOTOR_CHECK(r.skipped[0].reason == RegionSkipReason::AlreadyRunning);
    MOTOR_CHECK(e.interprete.active().size() == 1);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
