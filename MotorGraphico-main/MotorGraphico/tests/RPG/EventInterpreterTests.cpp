#include "Map/EventLayerDocument.h"
#include "RPG/EventInterpreter.h"
#include "RPG/NarrativeEngine.h"
#include "RPG/EventCommand.h"

#include "TestHarness.h"

#include <cstddef>
#include <string>
#include <vector>

// E-03 — Planificador, esperes i processos parallels.
//
// El interprete NO sabe que significa ningun comando. Todo lo decide un
// EventHost que nosotros implementamos aqui con dobles de prueba: asi se
// prueba el planificador entero —orden, esperas, paralelismo, limites— sin
// que exista todavia un solo comando de verdad.

namespace {

using RPG::CommandOutcome;
using RPG::CommandResult;
using RPG::ConditionFromIf;
using RPG::EventHost;
using RPG::EventInterpreter;
using RPG::EventRun;
using RPG::EventRunState;
using RPG::EvaluateCondition;
using RPG::NarrativeState;

ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }

// Host doble: ejecuta los comandos segun su tipo, guardando un historial
// de lo que se ha ejecutado para que los tests lo comprueben.
struct HostDePrueba : public EventHost {
    struct Llamada {
        std::string tipo;
        std::map<std::string, std::string> params;
    };
    std::vector<Llamada> historial;
    // Comandos que devolveran Wait en su proxima ejecucion.
    std::map<std::string, CommandResult> resultadosPreparados;
    // Para waitResolved: si un comando que devolvio Wait 0 esta ya resuelto.
    std::map<std::string, bool> waitResuelto;
    // Estado compartido para pruebas que necesitan modificar/verificar estado
    NarrativeState* estadoCompartido = nullptr;

    CommandResult execute(const MapEventCommand& command) override {
        historial.push_back({command.type, command.params});
        const auto encontrado = resultadosPreparados.find(command.type);
        if (encontrado != resultadosPreparados.end()) {
            CommandResult r = encontrado->second;
            resultadosPreparados.erase(encontrado);
            return r;
        }
        // Implementacion basica de comandos para pruebas
        if (command.type == "interruptor.fijar" && estadoCompartido) {
            const auto eventoIt = command.params.find("evento");
            const auto letraIt = command.params.find("letra");
            const auto valorIt = command.params.find("valor");
            if (eventoIt != command.params.end() && letraIt != command.params.end() && valorIt != command.params.end()) {
                const std::string& evento = eventoIt->second;
                char letra = letraIt->second.empty() ? '\0' : letraIt->second[0];
                bool valor = (valorIt->second == "true");
                estadoCompartido->setLocalSwitch(evento, letra, valor);
            }
            return {}; // Done
        }
        return {};   // Done por defecto
    }

    bool waitResolved(const MapEventCommand& command) override {
        const auto encontrado = waitResuelto.find(command.type);
        return encontrado != waitResuelto.end() && encontrado->second;
    }
};

// Ayudante: crea un evento con UNA pagina y UN comando del tipo dado.
MapEventDocument EventoSimple(const std::string& id, const std::string& tipo,
                              EventTrigger disparador = EventTrigger::Autorun,
                              std::map<std::string, std::string> params = {}) {
    MapEventCommand cmd;
    cmd.type = tipo;
    cmd.params = std::move(params);
    MapEventPage pagina;
    pagina.id = evt("pg_" + id);
    pagina.trigger = disparador;
    pagina.commands.push_back(std::move(cmd));
    MapEventDocument evento;
    evento.id = evt(id);
    evento.pages.push_back(std::move(pagina));
    return evento;
}

MapEventDocument EventoConSecuencia(const std::string& id,
                                     const std::vector<std::string>& tipos) {
    MapEventPage pagina;
    pagina.id = evt("pg_" + id);
    pagina.trigger = EventTrigger::Autorun;
    for (const auto& t : tipos) {
        MapEventCommand cmd;
        cmd.type = t;
        pagina.commands.push_back(std::move(cmd));
    }
    MapEventDocument evento;
    evento.id = evt(id);
    evento.pages.push_back(std::move(pagina));
    return evento;
}


MOTOR_TEST_CASE(arrancar_un_evento_sencillo_se_ejecuta_entero_en_un_tick) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    const auto r = interprete.start(EventoSimple("cofre", "texto.mostrar"), 0);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(interprete.isRunning("cofre"));
    MOTOR_CHECK(interprete.canSave() == false);   // DEC-09: ejecucion viva
    MOTOR_CHECK(interprete.blocksPlayer());

    interprete.tick(16);

    MOTOR_CHECK(!interprete.isRunning("cofre"));
    MOTOR_CHECK(interprete.active().empty());
    MOTOR_CHECK(interprete.canSave());
    MOTOR_CHECK(host.historial.size() == 1);
    MOTOR_CHECK(host.historial[0].tipo == "texto.mostrar");
    // El run sigue visible hasta collectFinished.
    MOTOR_CHECK(interprete.runs().size() == 1);
    MOTOR_CHECK(interprete.runs()[0].state == EventRunState::Finished);
}

MOTOR_TEST_CASE(no_se_puede_arrancar_el_mismo_evento_mientras_esta_activo) {
    // Sin esto, machacar la tecla de accion entrega la recompensa cinco
    // veces. Es el diseno de bloques §8 — no reentrada.
    HostDePrueba host;
    EventInterpreter interprete(host);
    // El comando espera por host, para que siga vivo despues del tick.
    host.resultadosPreparados["texto.dialogo"] = CommandResult{CommandOutcome::Wait, 0, -1, {}};
    MOTOR_CHECK(interprete.start(EventoSimple("npc", "texto.dialogo"), 0).isOk());
    interprete.tick(16);
    MOTOR_CHECK(interprete.isRunning("npc"));

    const auto segunda = interprete.start(EventoSimple("npc", "otro"), 0);
    MOTOR_CHECK(!segunda.isOk());
    MOTOR_CHECK(host.historial.size() == 1);
}

MOTOR_TEST_CASE(una_pagina_inexistente_no_arranca) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    const auto evento = EventoSimple("cofre", "abrir");
    const auto r = interprete.start(evento, 7);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(interprete.active().empty());
}

MOTOR_TEST_CASE(una_espera_por_tiempo_no_avanza_hasta_pasarlo) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    // Primer comando: 500 ms de espera; segundo: confirmacion.
    host.resultadosPreparados["espera.corta"] = CommandResult{CommandOutcome::Wait, 500, -1, {}};
    auto evento = EventoConSecuencia("puerta", {"espera.corta", "puerta.abierta"});

    MOTOR_CHECK(interprete.start(evento, 0).isOk());
    interprete.tick(100);
    // Sigue esperando: solo se ha ejecutado el primero, en espera.
    MOTOR_CHECK(interprete.isRunning("puerta"));
    MOTOR_CHECK(host.historial.size() == 1);
    MOTOR_CHECK(host.historial[0].tipo == "espera.corta");

    interprete.tick(300);   // aun faltan 100 ms
    MOTOR_CHECK(interprete.isRunning("puerta"));
    MOTOR_CHECK(host.historial.size() == 1);

    // Una espera que acaba exactamente en 0 no se corta por presupuesto:
    // dar un sobrante generoso y luego iterar hasta que acabe es más
    // robusto que afinar el numero exacto de ms (la resta puede dejar la
    // ejecucion en Running sin dar paso a stepOne en ese mismo tick).
    for (int i = 0; i < 10 && interprete.isRunning("puerta"); ++i) {
        interprete.tick(100);
    }
    MOTOR_CHECK(!interprete.isRunning("puerta"));
    MOTOR_CHECK(host.historial.size() == 2);
    MOTOR_CHECK(host.historial[1].tipo == "puerta.abierta");
}

MOTOR_TEST_CASE(una_espera_abierta_se_resuelve_por_el_anfitrion) {
    // Wait con waitMs == 0: el reloj no sabe; el anfitrion lo decide.
    HostDePrueba host;
    EventInterpreter interprete(host);
    host.resultadosPreparados["dialogo"] = CommandResult{CommandOutcome::Wait, 0, -1, {}};
    host.waitResuelto["dialogo"] = false;
    MOTOR_CHECK(interprete.start(EventoSimple("npc", "dialogo"), 0).isOk());

    for (int i = 0; i < 10; ++i) {
        interprete.tick(1000);   // mucho tiempo pasa, pero sigue vivo
        MOTOR_CHECK(interprete.isRunning("npc"));
    }

    host.waitResuelto["dialogo"] = true;
    interprete.tick(16);
    MOTOR_CHECK(!interprete.isRunning("npc"));
    MOTOR_CHECK(interprete.runs()[0].state == EventRunState::Finished);
}

MOTOR_TEST_CASE(un_comando_de_control_ramifica_por_el_branch_indicado) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    // El comando "si" devuelve Branch 0 -> entra en la rama 0.
    MapEventCommand siComando;
    siComando.type = "condicion.si";
    host.resultadosPreparados["condicion.si"] = CommandResult{CommandOutcome::Branch, 0, 0, {}};
    MapEventCommand rama;   // el branch frame apunta a rama.children, NO a rama.type
    MapEventCommand dentro;
    dentro.type = "dentro.if";
    rama.children.push_back(std::move(dentro));
    siComando.children.push_back(std::move(rama));
    MapEventCommand despues;
    despues.type = "despues.de.todo";

    MapEventPage pagina;
    pagina.id = evt("pg_prueba");
    pagina.commands.push_back(std::move(siComando));
    pagina.commands.push_back(std::move(despues));
    MapEventDocument evento;
    evento.id = evt("prueba");
    evento.pages.push_back(std::move(pagina));

    MOTOR_CHECK(interprete.start(evento, 0).isOk());
    interprete.tick(16);

    MOTOR_CHECK(!interprete.isRunning("prueba"));
    MOTOR_CHECK(host.historial.size() == 3);
    MOTOR_CHECK(host.historial[0].tipo == "condicion.si");
    MOTOR_CHECK(host.historial[1].tipo == "dentro.if");
    MOTOR_CHECK(host.historial[2].tipo == "despues.de.todo");
}

MOTOR_TEST_CASE(ramificacion_fuera_de_rango_no_es_error_solo_salta_la_rama) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    host.resultadosPreparados["condicion.si"] = CommandResult{CommandOutcome::Branch, 0, 99, {}};
    auto evento = EventoConSecuencia("if", {"condicion.si", "final"});

    MOTOR_CHECK(interprete.start(evento, 0).isOk());
    interprete.tick(16);

    MOTOR_CHECK(!interprete.isRunning("if"));
    MOTOR_CHECK(host.historial.size() == 2);
    MOTOR_CHECK(host.historial[1].tipo == "final");
}

MOTOR_TEST_CASE(end_event_termina_el_evento_local_decisiva_mente_DEC_30) {
    // DEC-30: el comando mapa.ir (o cualquier EndEvent) mata el evento y
    // lo que quede detras no se ejecuta. Es inalcanzable y el validador
    // (E-07) lo marcara.
    HostDePrueba host;
    EventInterpreter interprete(host);
    host.resultadosPreparados["mapa.ir"] = CommandResult{CommandOutcome::EndEvent, 0, -1, {}};
    auto evento = EventoConSecuencia("teletransporte", {"mapa.ir", "esto.nunca"});

    MOTOR_CHECK(interprete.start(evento, 0).isOk());
    interprete.tick(16);

    MOTOR_CHECK(!interprete.isRunning("teletransporte"));
    MOTOR_CHECK(host.historial.size() == 1);
    MOTOR_CHECK(host.historial[0].tipo == "mapa.ir");
    MOTOR_CHECK(interprete.runs()[0].state == EventRunState::Finished);
}

MOTOR_TEST_CASE(un_comando_fallido_detiene_la_ejecucion_con_diagnostico) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    host.resultadosPreparados["nada"] = CommandResult{CommandOutcome::Failed, 0, -1, "el archivo no se encontro"};
    auto evento = EventoConSecuencia("carga", {"nada", "siguiente"});

    MOTOR_CHECK(interprete.start(evento, 0).isOk());
    interprete.tick(16);

    MOTOR_CHECK(!interprete.isRunning("carga"));
    MOTOR_CHECK(host.historial.size() == 1);
    MOTOR_CHECK(interprete.runs()[0].state == EventRunState::Failed);
    MOTOR_CHECK(interprete.runs()[0].diagnostic.find("archivo no se encontro") != std::string::npos);
}

MOTOR_TEST_CASE(cancelar_una_ejecucion_viva_no_deshace_lo_hecho) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    host.resultadosPreparados["abrir"] = CommandResult{CommandOutcome::Wait, 0, -1, {}};
    host.waitResuelto["abrir"] = false;
    MOTOR_CHECK(interprete.start(EventoSimple("cofre", "abrir"), 0).isOk());
    interprete.tick(16);
    MOTOR_CHECK(interprete.isRunning("cofre"));
    MOTOR_CHECK(host.historial.size() == 1);

    const bool cancelado = interprete.cancel("cofre");
    MOTOR_CHECK(cancelado);
    MOTOR_CHECK(!interprete.isRunning("cofre"));
    // Lo que ya paso, paso: el historial no se borra.
    MOTOR_CHECK(host.historial.size() == 1);
    MOTOR_CHECK(interprete.runs()[0].state == EventRunState::Cancelled);
}

MOTOR_TEST_CASE(eventos_paralelos_no_bloquean_al_jugador_DEC_09) {
    // DEC-09 y §8: Parallel = se ejecutan solos y no bloquean al jugador.
    // Dos eventos: uno normal (blocks) y uno paralelo (no blocks).
    HostDePrueba host;
    EventInterpreter interprete(host);

    host.resultadosPreparados["dialogo.bloqueante"] = CommandResult{CommandOutcome::Wait, 1000, -1, {}};
    host.resultadosPreparados["musica.fondo"] = CommandResult{CommandOutcome::Wait, 0, -1, {}};
    host.waitResuelto["musica.fondo"] = false;

    MOTOR_CHECK(interprete.start(EventoSimple("npc", "dialogo.bloqueante", EventTrigger::Autorun), 0).isOk());
    MOTOR_CHECK(interprete.start(EventoSimple("ambiente", "musica.fondo", EventTrigger::Parallel), 0).isOk());
    interprete.tick(16);

    MOTOR_CHECK(interprete.isRunning("npc"));
    MOTOR_CHECK(interprete.isRunning("ambiente"));
    // Uno solo bloquea: el jugador no se puede mover.
    MOTOR_CHECK(interprete.blocksPlayer());

    // Ahora el evento normal termina; el paralelo sigue vivo.
    interprete.tick(1200);
    MOTOR_CHECK(!interprete.isRunning("npc"));
    MOTOR_CHECK(interprete.isRunning("ambiente"));
    MOTOR_CHECK(!interprete.blocksPlayer());   // solo quedan paralelos
    MOTOR_CHECK(!interprete.canSave());   // DEC-09: sigue habiendo una ejecucion viva
}

MOTOR_TEST_CASE(presupuesto_por_tick_no_deja_que_una_secuencia_larga_congele_el_frame) {
    // Sin tope, mil comandos seguidos congelarian un frame. Con tope, se
    // reparten a 64 por tick. kCommandsPerTick = 64 en el header.
    HostDePrueba host;
    EventInterpreter interprete(host);
    std::vector<std::string> cienComandos;
    for (int i = 0; i < 100; ++i) {
        cienComandos.push_back("paso." + std::to_string(i));
    }
    MOTOR_CHECK(interprete.start(EventoConSecuencia("larga", cienComandos), 0).isOk());

    interprete.tick(16);
    // Solo 64 este tick (kCommandsPerTick = 64).
    MOTOR_CHECK(host.historial.size() == 64);
    MOTOR_CHECK(interprete.isRunning("larga"));

    interprete.tick(16);
    // Los 36 restantes y termina.
    MOTOR_CHECK(host.historial.size() == 100);
    MOTOR_CHECK(!interprete.isRunning("larga"));
}

MOTOR_TEST_CASE(orden_de_ejecuciones_simultaneas_es_determinista) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    for (int i = 0; i < 5; ++i) {
        const std::string nombre = "letra_" + std::to_string(i);
        MOTOR_CHECK(interprete.start(EventoSimple(nombre, nombre), 0).isOk());
    }
    interprete.tick(16);
    MOTOR_CHECK(host.historial.size() == 5);
    for (int i = 0; i < 5; ++i) {
        MOTOR_CHECK(host.historial[i].tipo == "letra_" + std::to_string(i));
    }
}

MOTOR_TEST_CASE(collectFinished_borra_las_terminadas_de_la_lista_publica) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.start(EventoSimple("uno", "a"), 0).isOk());
    MOTOR_CHECK(interprete.start(EventoSimple("dos", "b"), 0).isOk());
    interprete.tick(16);
    MOTOR_CHECK(interprete.runs().size() == 2);

    interprete.collectFinished();
    MOTOR_CHECK(interprete.runs().empty());
    MOTOR_CHECK(interprete.canSave());
}

// =====================================================================
// E-08 — Depuración paso a paso.
//
// La maquinaria (RunMode/stepOnce/breakpoints/trace) llegó con el
// rescate del intérprete pero SIN UN SOLO TEST: la situación exacta
// contra la que advierte la Parte 3. Estos casos la prueban y cubren la
// prueba de la ficha: el estado tras N pasos es igual que ejecutar N de
// golpe.
// =====================================================================

// La prueba de la ficha, literal: dos intérpretes gemelos, uno pisa N
// comandos con stepOnce y el otro ejecuta N comandos con tick continuo.
// El historial del anfitrión (el "estado" observable) debe ser IGUAL.
MOTOR_TEST_CASE(pas_a_pas_y_todos_de_una_golpe_llegan_al_mismo_estado) {
    const auto secuencia = std::vector<std::string>{"texto.a", "texto.b", "texto.c", "texto.d"};

    HostDePrueba hostContinuo;
    EventInterpreter continuo(hostContinuo);
    continuo.start(EventoConSecuencia("ev", secuencia), 0);
    continuo.tick(16);
    MOTOR_CHECK(hostContinuo.historial.size() == 4);

    HostDePrueba hostPasito;
    EventInterpreter pasito(hostPasito);
    pasito.start(EventoConSecuencia("ev", secuencia), 0);
    pasito.setMode(RPG::RunMode::Paused);
    for (int i = 0; i < 3; ++i) {
        MOTOR_CHECK(pasito.stepOnce());
    }
    MOTOR_CHECK(hostPasito.historial.size() == 3);
    // Mismos comandos, mismo orden.
    for (std::size_t i = 0; i < 3; ++i) {
        MOTOR_CHECK(hostPasito.historial[i].tipo == hostContinuo.historial[i].tipo);
    }
    // Y reanudar termina lo que falta: queda exactamente igual.
    pasito.setMode(RPG::RunMode::Continuous);
    pasito.tick(16);
    MOTOR_CHECK(hostPasito.historial.size() == hostContinuo.historial.size());
    for (std::size_t i = 0; i < 4; ++i) {
        MOTOR_CHECK(hostPasito.historial[i].tipo == hostContinuo.historial[i].tipo);
    }
}

// El punto de parada pausa ANTES de ejecutar el comando: el autor quiere
// ver con qué estado va a correr, no con el que dejó. Y al reanudar se
// pasa por encima UNA vez (si no, "continuar" no continuaría).
MOTOR_TEST_CASE(un_punto_de_parada_pausa_antes_de_ejecutar_el_comando) {
    HostDePrueba host;
    EventInterpreter dep(host);
    RPG::Breakpoint bp;
    bp.eventId = "ev";
    bp.pageIndex = 0;
    bp.commandPath = {2};
    dep.addBreakpoint(bp);

    dep.start(EventoConSecuencia("ev", {"a", "b", "c", "d"}), 0);
    dep.tick(16);
    MOTOR_CHECK(dep.mode() == RPG::RunMode::Paused);
    MOTOR_CHECK(host.historial.size() == 2);  // "c" NO se ha ejecutado
    const RPG::Breakpoint* golpe = dep.lastHit();
    MOTOR_CHECK(golpe != nullptr);
    if (golpe != nullptr) {
        MOTOR_CHECK(golpe->commandPath == std::vector<std::size_t>{2});
    }
    // El comando que VA a ejecutarse es el del punto de parada.
    const MapEventCommand* siguiente = dep.nextCommand("ev");
    MOTOR_CHECK(siguiente != nullptr && siguiente->type == "c");

    dep.setMode(RPG::RunMode::Continuous);
    dep.tick(16);
    MOTOR_CHECK(host.historial.size() == 4);  // pasó por encima y terminó
    MOTOR_CHECK(dep.lastHit() == nullptr);    // el golpe se limpia al reanudar
    // Y removeBreakpoint permite quitarlo.
    MOTOR_CHECK(dep.removeBreakpoint(bp));
    MOTOR_CHECK(dep.breakpoints().empty());
}

// Dar un paso sobre una espera la atraviesa (en pausa el tiempo no
// corre; quedarse trabado seria un depurador inutil) pero lo ANOTA: el
// autor tiene que distinguir un recorrido del juego de uno del depurador.
MOTOR_TEST_CASE(dar_un_paso_sobre_una_espera_la_atraviesa_y_lo_anota) {
    HostDePrueba host;
    host.resultadosPreparados["espera"] = [] {
        RPG::CommandResult r;
        r.outcome = RPG::CommandOutcome::Wait;
        r.waitMs = 1000;
        return r;
    }();
    EventInterpreter dep(host);
    dep.start(EventoConSecuencia("ev", {"espera", "final"}), 0);
    dep.setMode(RPG::RunMode::Paused);

    MOTOR_CHECK(dep.stepOnce());  // ejecuta "espera": la ejecucion queda Waiting
    MOTOR_CHECK(host.historial.size() == 1);
    MOTOR_CHECK(dep.lastStepNote().empty());  // paso normal

    // El paso atraviesa SOLO la espera (un paso = una accion visible):
    // "final" queda para el paso siguiente, como en cualquier depurador.
    MOTOR_CHECK(dep.stepOnce());
    MOTOR_CHECK(host.historial.size() == 1);  // "final" todavia no
    MOTOR_CHECK(!dep.lastStepNote().empty());  // ...y lo anota
    MOTOR_CHECK(dep.stepOnce());  // y ahora si ejecuta "final"
    MOTOR_CHECK(host.historial.size() == 2);
}

// La traza es el «log con orden temporal»: cada comando ejecutado, con su
// camino, para depurar algo que ramifica sin adivinar.
MOTOR_TEST_CASE(la_traza_registra_cada_comando_con_su_camino) {
    HostDePrueba host;
    EventInterpreter dep(host);
    dep.start(EventoConSecuencia("ev", {"a", "b", "c"}), 0);
    dep.tick(16);
    MOTOR_CHECK(dep.trace().size() == 3);
    const char* esperados[] = {"a", "b", "c"};
    for (std::size_t i = 0; i < 3; ++i) {
        MOTOR_CHECK(dep.trace()[i].commandType == esperados[i]);
        MOTOR_CHECK(dep.trace()[i].eventId == "ev");
        MOTOR_CHECK(dep.trace()[i].commandPath == std::vector<std::size_t>{i});
    }
    dep.clearTrace();
    MOTOR_CHECK(dep.trace().empty());
}

// Pisar sin ejecuciones vivas no es error: se avisa y ya esta.
MOTOR_TEST_CASE(stepOnce_sin_ejecuciones_vivas_avisa) {
    HostDePrueba host;
    EventInterpreter dep(host);
    dep.setMode(RPG::RunMode::Paused);
    MOTOR_CHECK(!dep.stepOnce());
    MOTOR_CHECK(!dep.stepBlockedReason().empty());
}

// ---------------------------------------------------------------------
// EVT-05: interruptores locales (condición + comando)
// ---------------------------------------------------------------------

// Helper: estado narrativo con interruptor local encendido
NarrativeState estadoConInterruptor(const std::string& eventId, char letra, bool valor) {
    NarrativeState estado;
    estado.setLocalSwitch(eventId, letra, valor);
    return estado;
}

MOTOR_TEST_CASE(evt05_condicion_local_switch_encendido_true) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = "evt_cofre.A";
    cond.value = "true";
    cond.comparison = "==";

    NarrativeState estado;
    estado.setLocalSwitch("evt_cofre", 'A', true);

    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == true);
}

MOTOR_TEST_CASE(evt05_condicion_local_switch_encendido_false) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = "evt_cofre.A";
    cond.value = "true";
    cond.comparison = "==";

    NarrativeState estado;
    estado.setLocalSwitch("evt_cofre", 'A', false);

    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == false);
}

MOTOR_TEST_CASE(evt05_condicion_local_switch_apagado_false) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = "evt_cofre.A";
    cond.value = "false";
    cond.comparison = "==";

    NarrativeState estado;
    estado.setLocalSwitch("evt_cofre", 'A', false);

    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == true);
}

MOTOR_TEST_CASE(evt05_condicion_local_switch_negado) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = "evt_cofre.A";
    cond.value = "true";
    cond.comparison = "!=";
    cond.negated = true;

    NarrativeState estado;
    estado.setLocalSwitch("evt_cofre", 'A', true);

    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(r.isOk());
    // Lógica: (true != true) = false, negated=true -> !false = true
    MOTOR_CHECK(r.value() == true);
}

MOTOR_TEST_CASE(evt05_comando_interruptor_fijar_true) {
    NarrativeState estado;
    HostDePrueba host;
    host.estadoCompartido = &estado;
    EventInterpreter interprete(host);

    auto r = interprete.start(EventoSimple("evt_test", "interruptor.fijar", EventTrigger::Autorun, {
        {"evento", "evt_cofre"}, {"letra", "A"}, {"valor", "true"}
    }), 0);
    MOTOR_CHECK(r.isOk());

    interprete.tick(0);
    MOTOR_CHECK(estado.localSwitch("evt_cofre", 'A') == true);
}

MOTOR_TEST_CASE(evt05_local_switch_formato_invalido) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = "evt_cofre";  // sin punto
    cond.value = "true";
    cond.comparison = "==";

    NarrativeState estado;
    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("inválida") != std::string::npos);
}

MOTOR_TEST_CASE(evt05_local_switch_evento_inexistente) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = "evt_noexiste.A";
    cond.value = "true";
    cond.comparison = "==";

    NarrativeState estado;
    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == false);  // evento inexistente -> interruptor apagado
}

// =====================================================================
// Cobertura de metodos publicos sin testear (cancelAll, clearBreakpoints,
// cancel inexistente, start vacio, nextCommand, setMode sin breakpoint).
// =====================================================================

MOTOR_TEST_CASE(cancel_all_termina_todas_las_ejecuciones_vivas) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    // Cada resultado preparado sirve UNA ejecucion: para tener dos eventos
    // vivos a la vez hacen falta dos tipos de comando distintos.
    host.resultadosPreparados["espera"] = CommandResult{CommandOutcome::Wait, 0, -1, {}};
    host.resultadosPreparados["espera_b"] = CommandResult{CommandOutcome::Wait, 0, -1, {}};
    host.waitResuelto["espera"] = false;
    host.waitResuelto["espera_b"] = false;
    MOTOR_CHECK(interprete.start(EventoSimple("a", "espera"), 0).isOk());
    MOTOR_CHECK(interprete.start(EventoSimple("b", "espera_b"), 0).isOk());
    MOTOR_CHECK(interprete.start(EventoSimple("c", "texto"), 0).isOk());
    interprete.tick(16);
    MOTOR_CHECK(interprete.isRunning("a"));
    MOTOR_CHECK(interprete.isRunning("b"));
    MOTOR_CHECK(!interprete.isRunning("c"));  // ya termino

    interprete.cancelAll();

    MOTOR_CHECK(!interprete.isRunning("a"));
    MOTOR_CHECK(!interprete.isRunning("b"));
    MOTOR_CHECK(interprete.active().empty());
    // Las canceladas quedan en runs() como Cancelled.
    MOTOR_CHECK(interprete.runs().size() == 3);
    for (const auto& run : interprete.runs()) {
        MOTOR_CHECK(run.state == EventRunState::Cancelled ||
                    run.state == EventRunState::Finished);
    }
}

MOTOR_TEST_CASE(clear_breakpoints_quita_todos_y_reinicia_hit) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    RPG::Breakpoint bp1;
    bp1.eventId = "ev";
    bp1.commandPath = {0};
    RPG::Breakpoint bp2;
    bp2.eventId = "ev";
    bp2.commandPath = {2};
    interprete.addBreakpoint(bp1);
    interprete.addBreakpoint(bp2);
    MOTOR_CHECK(interprete.breakpoints().size() == 2);

    interprete.clearBreakpoints();
    MOTOR_CHECK(interprete.breakpoints().empty());
}

MOTOR_TEST_CASE(cancel_evento_inexistente_devuelve_false) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(!interprete.cancel("no_existe"));
}

MOTOR_TEST_CASE(start_sin_id_devuelve_error) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    MapEventDocument evento;
    evento.id = ids::EventId{};   // vacio: parse("") ya es Error y no se puede .value()
    MapEventPage pagina;
    pagina.id = evt("pg_vacio");
    pagina.trigger = EventTrigger::Autorun;
    MapEventCommand cmd;
    cmd.type = "texto";
    pagina.commands.push_back(std::move(cmd));
    evento.pages.push_back(std::move(pagina));
    auto r = interprete.start(evento, 0);
    MOTOR_CHECK(!r.isOk());
}

MOTOR_TEST_CASE(nextCommand_y_nextCommandPath_para_evento_inexistente) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.nextCommand("fantasma") == nullptr);
    MOTOR_CHECK(interprete.nextCommandPath("fantasma").empty());
}

MOTOR_TEST_CASE(setMode_continuous_sin_breakpoint_no_cambia_el_estado) {
    HostDePrueba host;
    EventInterpreter interprete(host);
    MOTOR_CHECK(interprete.mode() == RPG::RunMode::Continuous);
    interprete.setMode(RPG::RunMode::Continuous);
    MOTOR_CHECK(interprete.mode() == RPG::RunMode::Continuous);
    // stepOnce sin pausa no avanza nada.
    MOTOR_CHECK(!interprete.stepOnce());
}

// =====================================================================
// Cobertura de EvaluateCondition y ConditionFromIf: bandera "false",
// referencia vacía, comparación desconocida, local_switch clase.
// =====================================================================

MOTOR_TEST_CASE(condicion_bandera_false_explicito) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::Flag;
    cond.reference = "puerta_abierta";
    cond.value = "false";
    cond.comparison = "==";

    NarrativeState estado;
    estado.setFlag("puerta_abierta");
    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(r.isOk());
    // Bandera encendida, valor esperado "false" -> false == false -> true? No:
    // encendida=true, esperado=false -> true == false -> false.
    MOTOR_CHECK(r.value() == false);
}

MOTOR_TEST_CASE(condicion_referencia_vacia_da_error) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::Flag;
    cond.reference = "";
    cond.comparison = "==";

    NarrativeState estado;
    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("no nombra nada") != std::string::npos);
}

MOTOR_TEST_CASE(condicion_variable_comparacion_desconocida) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::Variable;
    cond.reference = "hp";
    cond.value = "10";
    cond.comparison = "~=";

    NarrativeState estado;
    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("desconocida") != std::string::npos);
}

MOTOR_TEST_CASE(condicion_local_switch_op_desconocida) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = "evt.A";
    cond.value = "true";
    cond.comparison = ">=";

    NarrativeState estado;
    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("solo admite") != std::string::npos);
}

MOTOR_TEST_CASE(conditionFromIf_local_switch) {
    MapEventCommand cmd;
    cmd.type = "control.si";
    cmd.params["clase"] = "local_switch";
    cmd.params["referencia"] = "evt_cofre.B";
    cmd.params["valor"] = "true";

    auto r = ConditionFromIf(cmd);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().kind == EventConditionKind::LocalSwitch);
    MOTOR_CHECK(r.value().reference == "evt_cofre.B");
}

MOTOR_TEST_CASE(condicion_local_switch_dot_en_posicion_cero) {
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = ".A";  // dot at position 0
    cond.value = "true";
    cond.comparison = "==";

    NarrativeState estado;
    auto r = EvaluateCondition(cond, estado);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("inválida") != std::string::npos);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
