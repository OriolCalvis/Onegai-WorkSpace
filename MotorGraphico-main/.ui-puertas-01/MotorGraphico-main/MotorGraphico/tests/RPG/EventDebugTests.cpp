#include "RPG/EventInterpreter.h"

#include "TestHarness.h"

#include <string>
#include <vector>

// E-08 — Depuracion paso a paso.
//
// Fichero aparte de EventInterpreterTests.cpp a proposito: ese lo esta
// ampliando otra sesion, y anadirle casos mientras tanto es como se
// pierde trabajo. Lo que se prueba aqui es el depurador, no el
// planificador.

namespace {

using namespace RPG;

ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }

MapEventCommand cmd(const std::string& tipo) {
    MapEventCommand c;
    c.type = tipo;
    return c;
}

MapEventDocument evento(const std::string& id, std::vector<MapEventCommand> comandos) {
    MapEventDocument e;
    e.id = evt(id);
    e.name = id;
    MapEventPage p;
    p.id = evt(id + "_p1");
    p.commands = std::move(comandos);
    e.pages.push_back(std::move(p));
    return e;
}

class Host : public EventHost {
public:
    std::vector<std::string> ejecutados;
    bool esperaResuelta = false;
    int rama = -1;

    CommandResult execute(const MapEventCommand& command) override {
        ejecutados.push_back(command.type);
        CommandResult r;
        if (command.type == "esperar") {
            r.outcome = CommandOutcome::Wait;
            r.waitMs = 100;
        } else if (command.type == "dialogo") {
            r.outcome = CommandOutcome::Wait;
            r.waitMs = 0;
        } else if (command.type == "si") {
            r.outcome = CommandOutcome::Branch;
            r.branch = rama;
        }
        return r;
    }
    bool waitResolved(const MapEventCommand&) override { return esperaResuelta; }
};

MOTOR_TEST_CASE(en_pausa_no_avanza_nadie_ni_corre_el_tiempo) {
    // Si las esperas siguieran corriendo con el depurador parado, el
    // autor volveria de mirar el estado y encontraria la escena en otro
    // sitio.
    Host host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.start(evento("charla", {cmd("texto"), cmd("esperar"), cmd("texto")}), 0).isOk());

    interprete.setMode(RunMode::Paused);
    for (int i = 0; i < 50; ++i) {
        interprete.tick(100);
    }
    MOTOR_CHECK(host.ejecutados.empty());
    MOTOR_CHECK(interprete.isRunning("charla"));

    interprete.setMode(RunMode::Continuous);
    interprete.tick(0);
    MOTOR_CHECK(host.ejecutados.size() == 2);   // texto y esperar
}

MOTOR_TEST_CASE(un_paso_ejecuta_exactamente_un_comando) {
    Host host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.start(evento("charla", {cmd("a"), cmd("b"), cmd("c")}), 0).isOk());
    interprete.setMode(RunMode::Paused);

    MOTOR_CHECK(interprete.stepOnce());
    MOTOR_CHECK(host.ejecutados.size() == 1);
    MOTOR_CHECK(host.ejecutados[0] == "a");

    MOTOR_CHECK(interprete.stepOnce());
    MOTOR_CHECK(host.ejecutados.size() == 2);
    MOTOR_CHECK(host.ejecutados[1] == "b");
}

MOTOR_TEST_CASE(un_paso_atraviesa_una_espera_pero_lo_anota) {
    // La especificacion §15.3 pide que «una espera no se salte sin
    // indicarlo». Se resuelve atravesandola Y anotandolo, no
    // prohibiendolo: en pausa el tiempo no corre, asi que negarse dejaria
    // el depurador trabado para siempre. Lo que no puede pasar es que se
    // salte en SILENCIO, porque entonces el autor veria un recorrido que
    // el juego nunca hace.
    Host host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.start(evento("charla", {cmd("esperar"), cmd("despues")}), 0).isOk());
    interprete.setMode(RunMode::Paused);

    MOTOR_CHECK(interprete.stepOnce());            // ejecuta "esperar"
    MOTOR_CHECK(host.ejecutados.size() == 1);
    MOTOR_CHECK(interprete.lastStepNote().empty());  // paso normal

    // Ahora esta esperando: el paso la atraviesa Y lo dice.
    MOTOR_CHECK(interprete.stepOnce());
    MOTOR_CHECK(host.ejecutados.size() == 1);      // no ha ejecutado nada nuevo
    MOTOR_CHECK(!interprete.lastStepNote().empty());
    MOTOR_CHECK(interprete.lastStepNote().find("espera") != std::string::npos);

    // Y el paso siguiente ya es normal.
    MOTOR_CHECK(interprete.stepOnce());
    MOTOR_CHECK(host.ejecutados.size() == 2);
    MOTOR_CHECK(host.ejecutados[1] == "despues");
    MOTOR_CHECK(interprete.lastStepNote().empty());
}

MOTOR_TEST_CASE(sin_pausa_una_espera_se_respeta_de_verdad) {
    // El contrapunto del caso anterior: corriendo normal, la espera
    // cuenta su tiempo entero. Lo que se salta es cosa del depurador.
    Host host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.start(evento("charla", {cmd("esperar"), cmd("despues")}), 0).isOk());
    interprete.tick(0);
    MOTOR_CHECK(host.ejecutados.size() == 1);
    for (int i = 0; i < 9; ++i) {
        interprete.tick(10);                        // 90 ms de 100
    }
    MOTOR_CHECK(host.ejecutados.size() == 1);
    interprete.tick(10);                            // los 100
    MOTOR_CHECK(host.ejecutados.size() == 2);
}

MOTOR_TEST_CASE(un_punto_de_parada_para_antes_de_ejecutar_su_comando) {
    // El autor quiere ver el estado con el que el comando VA a correr, no
    // el que deja. Si parase despues, lo que ve ya no es lo que buscaba.
    Host host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.start(evento("charla", {cmd("a"), cmd("b"), cmd("c")}), 0).isOk());

    Breakpoint punto;
    punto.eventId = "charla";
    punto.pageIndex = 0;
    punto.commandPath = {1};        // el segundo comando
    interprete.addBreakpoint(punto);

    interprete.tick(0);
    MOTOR_CHECK(interprete.mode() == RunMode::Paused);
    MOTOR_CHECK(host.ejecutados.size() == 1);      // "b" NO se ha ejecutado
    MOTOR_CHECK(host.ejecutados[0] == "a");
    MOTOR_CHECK(interprete.lastHit() != nullptr);
    MOTOR_CHECK(interprete.lastHit()->commandPath == std::vector<std::size_t>{1});

    // Y el comando que va a ejecutarse es justo ese.
    const MapEventCommand* siguiente = interprete.nextCommand("charla");
    MOTOR_CHECK(siguiente != nullptr);
    MOTOR_CHECK(siguiente->type == "b");
    MOTOR_CHECK(interprete.nextCommandPath("charla") == std::vector<std::size_t>{1});
}

MOTOR_TEST_CASE(se_puede_seguir_desde_un_punto_de_parada_sin_quedarse_atascado) {
    // Si al reanudar volviera a pararse en el mismo sitio, no se
    // avanzaria nunca.
    Host host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.start(evento("charla", {cmd("a"), cmd("b"), cmd("c")}), 0).isOk());
    Breakpoint punto;
    punto.eventId = "charla";
    punto.commandPath = {1};
    interprete.addBreakpoint(punto);

    interprete.tick(0);
    MOTOR_CHECK(host.ejecutados.size() == 1);

    MOTOR_CHECK(interprete.stepOnce());
    MOTOR_CHECK(host.ejecutados.size() == 2);
    MOTOR_CHECK(host.ejecutados[1] == "b");

    interprete.setMode(RunMode::Continuous);
    interprete.tick(0);
    MOTOR_CHECK(host.ejecutados.size() == 3);
    MOTOR_CHECK(!interprete.isRunning("charla"));
}

MOTOR_TEST_CASE(poner_dos_veces_el_mismo_punto_no_lo_duplica_y_se_puede_quitar) {
    Host host;
    EventInterpreter interprete(host);
    Breakpoint punto;
    punto.eventId = "charla";
    punto.commandPath = {0};

    interprete.addBreakpoint(punto);
    interprete.addBreakpoint(punto);
    MOTOR_CHECK(interprete.breakpoints().size() == 1);

    MOTOR_CHECK(interprete.removeBreakpoint(punto));
    MOTOR_CHECK(interprete.breakpoints().empty());
    MOTOR_CHECK(!interprete.removeBreakpoint(punto));
}

MOTOR_TEST_CASE(la_traza_dice_por_donde_paso_en_orden) {
    // Sin log en orden temporal, depurar un evento que ramifica es
    // adivinar por donde paso.
    Host host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.start(evento("charla", {cmd("a"), cmd("b")}), 0).isOk());
    interprete.tick(0);

    MOTOR_CHECK(interprete.trace().size() == 2);
    MOTOR_CHECK(interprete.trace()[0].commandType == "a");
    MOTOR_CHECK(interprete.trace()[0].eventId == "charla");
    MOTOR_CHECK(interprete.trace()[0].commandPath == std::vector<std::size_t>{0});
    MOTOR_CHECK(interprete.trace()[1].commandType == "b");
    MOTOR_CHECK(interprete.trace()[1].commandPath == std::vector<std::size_t>{1});

    interprete.clearTrace();
    MOTOR_CHECK(interprete.trace().empty());
}

MOTOR_TEST_CASE(un_paso_sin_nada_vivo_dice_por_que_no_puede) {
    Host host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(!interprete.stepOnce());
    MOTOR_CHECK(!interprete.stepBlockedReason().empty());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
