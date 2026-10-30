// U3-02.E06b — updateEventsAndRegions: orquestacion ENTRE FRAMES de
// MapEventRunner::update y el despacho de regiones. Lo que se defiende: nunca
// dos escenas bloqueantes vivas, Autorun y Parallel diferidos (no perdidos)
// mientras hay escena viva, el mapa del runner restaurado aunque update lance,
// y sin inanicion de automaticos frente a un Stay de intervalo corto.

#include "RPG/RegionTriggerDispatch.h"
#include "RPG/RegionTriggerSupport.h"

#include "TestHarness.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace region_test;

// Escena de region viva en F1; en F2 se activa la bandera de un Autorun. Con
// update() a secas arrancaria encima (dos bloqueantes); con la orquestacion
// espera a que la region termine.
MOTOR_TEST_CASE(autorun_activado_con_una_region_viva_espera_a_que_termine) {
    MapEventCondition c;
    c.kind = EventConditionKind::Flag;
    c.reference = "listo";
    MapEventDocument aut = evento("aut", GridCoord{9, 9}, pagina("aut_p0", EventTrigger::Autorun));
    aut.pages[0].conditions = {c};
    const MapDocument mapa = mapaCon({aut, eventoRegion("trampa", 1, EventTrigger::RegionEnter)});
    Escena e;
    e.runner.setMap(&mapa);
    MapEventFrame f;
    f.playerCell = GridCoord{1, 0};
    f.deltaMs = 16;
    const auto bloqueantes = [&]() {
        int n = 0;
        for (const EventRun& run : e.interprete.active()) {
            n += run.parallel ? 0 : 1;
        }
        return n;
    };
    // F1: arranca la region.
    Result<RegionDispatchResult> r = updateEventsAndRegions(
        e.runner, e.interprete, e.estado, e.inventario, f, {disp(Kind::Enter)});
    MOTOR_CHECK(r.isOk() && r.value().started.size() == 1);
    // F2..: la bandera se activa con la region viva.
    e.estado.setFlag("listo");
    for (int i = 0; i < 10 && e.interprete.isRunning("trampa"); ++i) {
        r = updateEventsAndRegions(e.runner, e.interprete, e.estado, e.inventario, f, {});
        MOTOR_CHECK(bloqueantes() <= 1);
        if (e.interprete.isRunning("trampa")) {
            MOTOR_CHECK(!e.interprete.isRunning("aut"));
        }
        f.deltaMs = 400;
    }
    MOTOR_CHECK(!e.interprete.isRunning("trampa"));
    // Termino la region: el siguiente frame si arranca el Autorun.
    r = updateEventsAndRegions(e.runner, e.interprete, e.estado, e.inventario, f, {});
    MOTOR_CHECK(e.interprete.isRunning("aut"));
    MOTOR_CHECK(bloqueantes() == 1);
    MOTOR_CHECK(e.runner.map() == &mapa);
}

MOTOR_TEST_CASE(un_parallel_activado_con_escena_viva_arranca_al_terminar_y_no_se_pierde) {
    MapEventCondition c;
    c.kind = EventConditionKind::Flag;
    c.reference = "ambiente";
    MapEventDocument par = evento("par", GridCoord{9, 9}, pagina("par_p0", EventTrigger::Parallel));
    par.pages[0].conditions = {c};
    const MapDocument mapa = mapaCon({par, eventoRegion("trampa", 1, EventTrigger::RegionEnter)});
    Escena e;
    e.runner.setMap(&mapa);
    MapEventFrame f;
    f.playerCell = GridCoord{1, 0};
    f.deltaMs = 16;
    MOTOR_CHECK(updateEventsAndRegions(e.runner, e.interprete, e.estado, e.inventario, f,
                                       {disp(Kind::Enter)}).isOk());
    MOTOR_CHECK(e.interprete.isRunning("trampa"));
    e.estado.setFlag("ambiente");
    for (int i = 0; i < 10 && e.interprete.isRunning("trampa"); ++i) {
        MOTOR_CHECK(updateEventsAndRegions(e.runner, e.interprete, e.estado, e.inventario, f,
                                           {}).isOk());
        if (e.interprete.isRunning("trampa")) {
            MOTOR_CHECK(!e.interprete.isRunning("par"));
        }
        f.deltaMs = 400;
    }
    MOTOR_CHECK(!e.interprete.isRunning("trampa"));
    MOTOR_CHECK(updateEventsAndRegions(e.runner, e.interprete, e.estado, e.inventario, f,
                                       {}).isOk());
    MOTOR_CHECK(e.interprete.isRunning("par"));
}

// Anfitrion que lanza en execute(): simula un fallo del host durante el tick.
class HostQueLanza : public EventHost {
public:
    bool lanza = true;
    CommandResult execute(const MapEventCommand&) override {
        if (lanza) {
            throw std::runtime_error("fallo del anfitrion");
        }
        return CommandResult{};
    }
};

MOTOR_TEST_CASE(el_mapa_del_runner_se_restaura_aunque_update_lance) {
    MapEventCondition c;
    c.kind = EventConditionKind::Flag;
    c.reference = "listo";
    MapEventDocument aut = evento("aut", GridCoord{9, 9}, pagina("aut_p0", EventTrigger::Autorun));
    aut.pages[0].conditions = {c};
    const MapDocument mapa = mapaCon({aut});
    HostQueLanza host;
    EventInterpreter interprete(host);
    NarrativeState estado;
    std::vector<std::string> inventario;
    MapEventRunner runner(estado, inventario, interprete);
    runner.setMap(&mapa);
    // Escena viva => el wrapper suspende el mapa; el tick lanza.
    const MapEventDocument viva =
        evento("viva", GridCoord{5, 5}, pagina("viva_p0", EventTrigger::Interact));
    MOTOR_CHECK(interprete.start(viva, 0).isOk());
    MapEventFrame f;
    f.deltaMs = 16;
    bool lanzo = false;
    try {
        (void)updateEventsAndRegions(runner, interprete, estado, inventario, f, {});
    } catch (const std::runtime_error&) {
        lanzo = true;
    }
    MOTOR_CHECK(lanzo);
    MOTOR_CHECK(runner.map() == &mapa);
    // El frame siguiente dispara con normalidad.
    host.lanza = false;
    interprete.cancelAll();
    estado.setFlag("listo");
    MOTOR_CHECK(updateEventsAndRegions(runner, interprete, estado, inventario, f, {}).isOk());
    MOTOR_CHECK(runner.lastStartedEvent() == "aut");
}

// Stay cada frame (16 ms) sobre una escena de 1000 ms: sin la regla de
// diferir, el frame en que la escena termina arrancaria otro Stay y el
// automatico pendiente no arrancaria nunca.
void sinInanicion(EventTrigger tipo, bool pendiente) {
    MapEventCondition c;
    c.kind = EventConditionKind::Flag;
    c.reference = "listo";
    MapEventDocument aut = evento("aut", GridCoord{9, 9}, pagina("aut_p0", tipo));
    aut.pages[0].conditions = {c};
    const MapDocument mapa = mapaCon({aut, eventoRegion("stay", 1, EventTrigger::RegionStay)});
    Escena e;
    e.runner.setMap(&mapa);
    MapEventFrame f;
    f.playerCell = GridCoord{1, 0};
    f.deltaMs = 16;
    const std::vector<Regiones::RegionTrigger> cada = {disp(Kind::Stay)};
    const auto frame = [&]() {
        const Result<RegionDispatchResult> r =
            updateEventsAndRegions(e.runner, e.interprete, e.estado, e.inventario, f, cada);
        MOTOR_CHECK(r.isOk());
        return r.value();
    };
    MOTOR_CHECK(frame().started.size() == 1);
    if (pendiente) {
        e.estado.setFlag("listo");
    }
    int n = 0;
    bool diferido = false;
    while (e.interprete.isRunning("stay") && n++ < 200) {
        const RegionDispatchResult r = frame();
        for (const RegionSkipped& s : r.skipped) {
            diferido = diferido || s.reason == RegionSkipReason::DeferredToAutomatics;
        }
        MOTOR_CHECK(r.started.empty());
    }
    MOTOR_CHECK(n < 200);
    MOTOR_CHECK(diferido);
    // Frame siguiente al fin de la escena.
    const RegionDispatchResult r = frame();
    if (pendiente) {
        MOTOR_CHECK(e.interprete.isRunning("aut"));
        if (tipo == EventTrigger::Autorun) {
            MOTOR_CHECK(!e.interprete.isRunning("stay"));
            MOTOR_CHECK(r.started.empty());
        }
    } else {
        MOTOR_CHECK(r.started.size() == 1 && r.started[0].eventId == "stay");
    }
}

MOTOR_TEST_CASE(un_autorun_pendiente_gana_al_stay_en_el_frame_en_que_termina_la_escena) {
    sinInanicion(EventTrigger::Autorun, true);
}

MOTOR_TEST_CASE(un_parallel_pendiente_arranca_al_terminar_la_escena_de_stay) {
    sinInanicion(EventTrigger::Parallel, true);
}

MOTOR_TEST_CASE(sin_automaticos_pendientes_el_siguiente_stay_si_arranca) {
    sinInanicion(EventTrigger::Autorun, false);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
