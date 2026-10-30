// H0-08 (Fase H) — La capa de disparadores: del mapa y del jugador al
// intérprete.
//
// Lo que se defiende aqui, y por que en este orden. Los CINCO primeros casos
// son el criterio de cierre de H0-08 ("jugar assets/misiones/mision_llave.json:
// la puerta cerrada se abre con la llave") sobre EL ASSET PUBLICADO y conducido
// por CASILLAS, que es lo que un jugador puede hacer. `demo_mision_llave` ya
// demostraba la cadena entera, pero la conducia POR ID
// (`interactuar(mapa, "puerta", ...)`, examples/demo_mision_llave.cpp:236):
// entre eso y la partida faltaba justo lo que MapEventRunner hace.
//
// El resto son las reglas de disparo, cada una con el fallo concreto que
// impide: Touch que se rearma cada frame, Autorun que congela la partida,
// Parallel que bloquea sin deber, una escena interrumpida por otra, y una
// condicion que no se puede evaluar tratada como falsa — que es como una
// puerta que no se abre nunca y nadie sabe por que.

// La ultima seccion es H3-03 (dialogo ramificado por interruptores). El
// mecanismo ya estaba entero — CondicionesYPaginas.cpp evalua Flag, Variable,
// LocalSwitch e Item, y NarrativeCommandHost ejecuta interruptor.fijar y
// variable.sumar sobre el MISMO estado que se lee al elegir pagina —, pero
// NINGUN contenido publicado lo ejercitaba: mision_llave.json solo ramifica por
// banderas. Por eso la entrega es un mapa publicado nuevo
// (assets/misiones/conversacion_ramificada.json) y su prueba conducida por
// casillas, no codigo nuevo. Decidido por Oriol el 29/09: por la via de
// eventos, sin ampliar NarrativeCondition (que habria dejado dos vocabularios
// de condiciones haciendo lo mismo).

#include "RPG/MapEventRunner.h"

#include "TestHarness.h"

#include "Level/ObjectCatalog.h"
#include "Map/MapDocument.h"
#include "Project/ProjectSerializer.h"
#include "RPG/AdventureValidation.h"
#include "RPG/EventCommand.h"
#include "RPG/EventInterpreter.h"
#include "RPG/SessionCommandHost.h"
#include "Render/ICombatant.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {

using namespace RPG;

const char* const kMision = "assets/misiones/mision_llave.json";
const char* const kConversacion = "assets/misiones/conversacion_ramificada.json";

// --- Dobles mínimos, los mismos que usa SessionCommandHostTests ----------

class Cuerpo : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "jugador"; }
};

LevelDefinition nivelVacio() {
    LevelDefinition l;
    l.name = "Prueba";
    l.playerStart = GridCoord{0, 0};
    return l;
}

// --- Constructores de documentos ---------------------------------------

ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }
ids::LayerId lyr(const std::string& v) { return ids::LayerId::parse(v).value(); }

MapEventCommand cmd(const std::string& tipo, std::map<std::string, std::string> params = {}) {
    MapEventCommand c;
    c.type = tipo;
    c.params = std::move(params);
    return c;
}

MapEventPage pagina(const std::string& id, EventTrigger disparador,
                    std::vector<MapEventCommand> comandos,
                    std::vector<MapEventCondition> condiciones = {}) {
    MapEventPage p;
    p.id = evt(id);
    p.name = id;
    p.trigger = disparador;
    p.conditions = std::move(condiciones);
    p.commands = std::move(comandos);
    return p;
}

MapEventDocument evento(const std::string& id, GridCoord donde, EventTrigger disparador,
                        std::vector<MapEventCommand> comandos,
                        std::vector<MapEventCondition> condiciones = {}) {
    MapEventDocument e;
    e.id = evt(id);
    e.name = id;
    e.position = donde;
    e.pages = {pagina(id + "_p0", disparador, std::move(comandos), std::move(condiciones))};
    return e;
}

MapEventCondition bandera(const std::string& nombre) {
    MapEventCondition c;
    c.kind = EventConditionKind::Flag;
    c.reference = nombre;
    return c;
}

MapDocument mapaCon(std::vector<MapEventDocument> eventos) {
    MapDocument mapa(ids::MapId::parse("prueba").value(), "Prueba", 16, 12, 64, 32);
    MapLayerMeta meta;
    meta.id = lyr("eventos");
    meta.name = "Eventos";
    MOTOR_CHECK(mapa.addLayer(MapLayer::events(meta, EventLayerDocument())).isOk());
    for (MapEventDocument& e : eventos) {
        const Result<bool> alta = mapa.addEvent(meta.id, e);
        if (!alta.isOk()) {
            MOTOR_CHECK_FAIL(alta.errorMessage());
        }
    }
    return mapa;
}

MapDocument mapaPublicado(const char* ruta) {
    std::ifstream fh(ruta, std::ios::binary);
    if (!fh) {
        MOTOR_CHECK_FAIL(std::string("no esta ") + ruta +
                         " (falta copiar_assets o WORKING_DIRECTORY)");
    }
    std::ostringstream buf;
    buf << fh.rdbuf();
    const Result<MapDocument> vuelta = ProjectSerializer::deserializeMap(buf.str());
    if (!vuelta.isOk()) {
        MOTOR_CHECK_FAIL(std::string(ruta) + ": " + vuelta.errorMessage());
    }
    return vuelta.value();
}

MapDocument misionLlave() { return mapaPublicado(kMision); }
MapDocument conversacionRamificada() { return mapaPublicado(kConversacion); }

// --- La escena: partida + anfitriones + intérprete + runner -------------
//
// Los miembros se declaran en orden de dependencia porque cada uno referencia
// los anteriores: es la misma cadena que monta examples/demo_mision_llave.cpp
// (SessionCommandHost de fallback del NarrativeCommandHost, y el intérprete
// sobre ese anfitrión). El runner no construye nada de esto, y eso es parte de
// lo que se prueba: recibe el intérprete ya hecho.

struct Escena {
    ObjectCatalog catalogo;
    SkillCatalog habilidades;
    Cuerpo cuerpo;
    SkillSet skills{10};
    std::vector<std::string> inventario;
    GameSession sesion{nullptr, &catalogo, &habilidades, nivelVacio(), &cuerpo, &skills,
                       &inventario};
    NarrativeState estado;
    SessionCommandHost partida{sesion, inventario};
    NarrativeCommandHost host{estado, &partida};
    EventInterpreter interprete{host};
    MapEventRunner runner{estado, inventario, interprete};

    GridCoord casilla{0, 0};

    void montar(const MapDocument& mapa) { runner.setMap(&mapa); }

    // Un frame con el jugador en esta casilla. `accion` es un FLANCO: solo el
    // frame en que se pulsa.
    void frame(int x, int y, bool accion = false, int deltaMs = 16) {
        casilla = GridCoord{x, y};
        MapEventFrame f;
        f.playerCell = casilla;
        f.interactPressed = accion;
        f.deltaMs = deltaMs;
        runner.update(f);
    }

    // Un frame sin moverse. Es lo que hace el bucle del juego la mayoria de
    // las veces, y es el caso que un disparador mal escrito repite.
    void quedarse(int frames = 1, int deltaMs = 16) {
        for (int i = 0; i < frames; ++i) {
            frame(casilla.x, casilla.y, false, deltaMs);
        }
    }

    // Deja correr lo vivo hasta que termine, con tope: un fallo tiene que
    // dar un caso rojo, no un cuelgue.
    void correr(int maxFrames = 32, int deltaMs = 16) {
        for (int i = 0; i < maxFrames && !interprete.active().empty(); ++i) {
            quedarse(1, deltaMs);
        }
    }

    // Pulsar accion y dejar que la escena termine.
    void pulsar(int x, int y) {
        frame(x, y, true);
        correr();
    }

    std::string ultimaFrase() const {
        if (host.log().empty()) {
            return {};
        }
        return host.log().back();
    }
};

// =======================================================================
// 1. El criterio de cierre de H0-08, sobre el asset publicado y por casillas
// =======================================================================

MOTOR_TEST_CASE(mision_llave_la_puerta_no_se_abre_sin_la_llave) {
    const MapDocument mapa = misionLlave();
    Escena e;
    e.montar(mapa);

    // La puerta esta en (9,4); el jugador en (8,4), al oeste. Pulsar accion
    // tiene que encontrarla por casillas, sin que nadie diga "puerta".
    e.pulsar(8, 4);

    MOTOR_CHECK(e.ultimaFrase().find("Hace falta una llave") != std::string::npos);
    MOTOR_CHECK(!e.estado.hasFlag("puerta_abierta"));
    MOTOR_CHECK(e.inventario.empty());
}

MOTOR_TEST_CASE(mision_llave_el_cofre_da_una_sola_llave_aunque_se_insista) {
    const MapDocument mapa = misionLlave();
    Escena e;
    e.montar(mapa);

    // El cofre esta en (3,4): el jugador se pone ENCIMA, que es la primera
    // casilla que se prueba al pulsar accion.
    e.pulsar(3, 4);
    MOTOR_CHECK(e.inventario.size() == 1);
    MOTOR_CHECK(e.inventario[0] == "llave_oxidada");
    MOTOR_CHECK(e.estado.hasFlag("cofre_abierto"));

    // Insistir: gana la pagina siguiente (E-09) y la llave no se duplica. Es
    // la propiedad por la que existen las paginas, y aqui la elige el runner.
    e.pulsar(3, 4);
    MOTOR_CHECK(e.ultimaFrase().find("vacio") != std::string::npos ||
                e.ultimaFrase().find("vacío") != std::string::npos);
    MOTOR_CHECK(e.inventario.size() == 1);
}

MOTOR_TEST_CASE(mision_llave_la_puerta_se_abre_con_la_llave) {
    const MapDocument mapa = misionLlave();
    Escena e;
    e.montar(mapa);

    // El recorrido entero del demo, pero conducido por casillas: el anciano
    // (2,2), el cofre (3,4), la puerta (9,4) y el altar (13,4).
    e.pulsar(2, 3);  // anciano, al norte del jugador
    MOTOR_CHECK(e.ultimaFrase().find("guarda la llave") != std::string::npos);

    e.pulsar(8, 4);  // puerta, antes de la llave
    MOTOR_CHECK(!e.estado.hasFlag("puerta_abierta"));

    e.pulsar(3, 4);  // cofre
    MOTOR_CHECK(e.inventario.size() == 1);

    e.pulsar(2, 3);  // el anciano ya dice otra cosa: el mundo contesta
    MOTOR_CHECK(e.ultimaFrase().find("La puerta del fondo") != std::string::npos);

    e.pulsar(12, 4);  // el altar todavia no cierra la mision
    MOTOR_CHECK(!e.estado.hasFlag("mision_completada"));

    e.pulsar(8, 4);  // LA PUERTA, AHORA CON LA LLAVE
    MOTOR_CHECK(e.ultimaFrase().find("chirrido") != std::string::npos);
    MOTOR_CHECK(e.estado.hasFlag("puerta_abierta"));

    e.pulsar(8, 4);  // y a partir de aqui tiene otra pagina
    MOTOR_CHECK(e.ultimaFrase().find("ya esta abierta") != std::string::npos ||
                e.ultimaFrase().find("ya está abierta") != std::string::npos);

    e.pulsar(2, 3);
    MOTOR_CHECK(e.ultimaFrase().find("Has abierto el santuario") != std::string::npos);

    e.pulsar(12, 4);  // el altar cierra la mision
    MOTOR_CHECK(e.estado.hasFlag("mision_completada"));

    // Abrir no es consumir: la llave sigue ahi.
    MOTOR_CHECK(e.inventario.size() == 1);
}

MOTOR_TEST_CASE(mision_llave_pulsar_accion_lejos_no_hace_nada) {
    const MapDocument mapa = misionLlave();
    Escena e;
    e.montar(mapa);

    // (0,11) no tiene evento ni en la casilla propia ni en las cuatro
    // contiguas: la esquina mas cercana del mapa esta a tres casillas del
    // anciano.
    e.pulsar(0, 11);

    MOTOR_CHECK(e.host.log().empty());
    MOTOR_CHECK(e.runner.lastStartedEvent().empty());
    MOTOR_CHECK(e.interprete.active().empty());
}

MOTOR_TEST_CASE(mantener_accion_pulsada_no_repite_la_frase) {
    const MapDocument mapa = misionLlave();
    Escena e;
    e.montar(mapa);

    // Un solo flanco y cinco frames con la tecla "tenida abajo": si el runner
    // tratara interactPressed como un nivel, el anciano diria su frase seis
    // veces.
    e.frame(2, 3, /*accion=*/true);
    e.quedarse(5);

    MOTOR_CHECK(e.host.log().size() == 1);
}

// =======================================================================
// 2. Las reglas de disparo
// =======================================================================

MOTOR_TEST_CASE(touch_dispara_al_entrar_y_no_cada_frame) {
    const MapDocument mapa = mapaCon({evento("placa", GridCoord{5, 5}, EventTrigger::Touch,
                                             {cmd("bandera.encender", {{"bandera", "pisada"}})})});
    Escena e;
    e.montar(mapa);

    e.frame(4, 5);
    MOTOR_CHECK(!e.estado.hasFlag("pisada"));

    e.frame(5, 5);  // ENTRA en la casilla
    e.correr();
    MOTOR_CHECK(e.estado.hasFlag("pisada"));
    MOTOR_CHECK(e.runner.lastStartedEvent() == "placa");

    // Diez frames de pie encima: no se rearma. Sin esto, una placa de
    // presion volveria a arrancar su evento 60 veces por segundo.
    e.estado.clearFlag("pisada");
    e.quedarse(10);
    MOTOR_CHECK(!e.estado.hasFlag("pisada"));

    // Salir y volver a entrar si: es lo que hace que una placa funcione dos
    // veces.
    e.frame(4, 5);
    e.frame(5, 5);
    e.correr();
    MOTOR_CHECK(e.estado.hasFlag("pisada"));
}

MOTOR_TEST_CASE(autorun_bloquea_al_jugador_y_no_se_rearma) {
    const MapDocument mapa =
        mapaCon({evento("intro", GridCoord{1, 1}, EventTrigger::Autorun,
                        {cmd("control.esperar", {{"ms", "1000"}})})});
    Escena e;
    e.montar(mapa);

    // El jugador esta en (7,7), a seis casillas del evento: un Autorun no
    // necesita que nadie vaya ni pulse nada.
    e.frame(7, 7);
    MOTOR_CHECK(e.runner.blocksPlayer());
    MOTOR_CHECK(e.runner.lastStartedEvent() == "intro");

    // Y mientras corre, bloquea cada frame — que es lo que Application lee
    // para no mover al jugador.
    e.quedarse(5);
    MOTOR_CHECK(e.runner.blocksPlayer());

    // Al terminar deja de bloquear, y NO vuelve a arrancar: la pagina activa
    // no ha cambiado. La alternativa (rearmar siempre que las condiciones se
    // cumplan) convertiria esta escena en un bucle que congela la partida.
    e.quedarse(80);
    MOTOR_CHECK(!e.runner.blocksPlayer());
    MOTOR_CHECK(e.interprete.active().empty());
    MOTOR_CHECK(e.host.log().empty());
}

MOTOR_TEST_CASE(parallel_no_bloquea_al_jugador) {
    const MapDocument mapa =
        mapaCon({evento("ambiente", GridCoord{1, 1}, EventTrigger::Parallel,
                        {cmd("control.esperar", {{"ms", "1000"}})})});
    Escena e;
    e.montar(mapa);

    e.frame(7, 7);
    MOTOR_CHECK(e.interprete.isRunning("ambiente"));
    // Esa es toda la diferencia entre paralelo y automatico.
    MOTOR_CHECK(!e.runner.blocksPlayer());
}

MOTOR_TEST_CASE(un_evento_que_bloquea_impide_arrancar_otro) {
    const MapDocument mapa = mapaCon({
        evento("intro", GridCoord{1, 1}, EventTrigger::Autorun,
               {cmd("control.esperar", {{"ms", "1000"}})}),
        evento("anciano", GridCoord{5, 5}, EventTrigger::Interact,
               {cmd("texto.mostrar", {{"texto", "hola"}})}),
    });
    Escena e;
    e.montar(mapa);

    // Mismo frame: arranca el Autorun y el jugador pulsa accion encima del
    // PNJ. Una escena no se interrumpe con otra encima.
    e.frame(5, 5, /*accion=*/true);

    MOTOR_CHECK(e.runner.lastStartedEvent() == "intro");
    MOTOR_CHECK(e.host.log().empty());
    MOTOR_CHECK(e.runner.blocksPlayer());
}

MOTOR_TEST_CASE(una_condicion_no_evaluable_se_dice_y_no_se_arranca) {
    // Comparacion que la validacion del documento acepta (solo rechaza la
    // vacia) y que el evaluador no conoce. Es el caso que E-07 caza en
    // estatico, pero el runtime no puede tratarlo como "falso": seria una
    // puerta que no se abre nunca y nadie sabe por que.
    MapEventCondition rota;
    rota.kind = EventConditionKind::Item;
    rota.reference = "llave_oxidada";
    rota.comparison = "~=";

    const MapDocument mapa = mapaCon({evento("puerta", GridCoord{5, 5}, EventTrigger::Interact,
                                             {cmd("texto.mostrar", {{"texto", "abierta"}})},
                                             {rota})});
    Escena e;
    e.montar(mapa);

    e.pulsar(5, 5);

    MOTOR_CHECK(e.host.log().empty());
    MOTOR_CHECK(e.interprete.active().empty());
    MOTOR_CHECK(e.runner.diagnostic().find("no evaluable") != std::string::npos);
    MOTOR_CHECK(e.runner.diagnostic().find("puerta") != std::string::npos);
}

MOTOR_TEST_CASE(interact_prueba_los_pies_antes_que_las_vecinas) {
    // Dos eventos de Interact a tiro: uno bajo los pies y uno al norte. Gana
    // el de los pies, que es el orden de GameSession::interact() — una puerta
    // se pisa y a un PNJ se le habla de frente.
    const MapDocument mapa = mapaCon({
        evento("suelo", GridCoord{5, 5}, EventTrigger::Interact,
               {cmd("texto.mostrar", {{"texto", "bajo los pies"}})}),
        evento("vecina", GridCoord{5, 4}, EventTrigger::Interact,
               {cmd("texto.mostrar", {{"texto", "en la vecina"}})}),
    });
    Escena e;
    e.montar(mapa);

    e.pulsar(5, 5);

    MOTOR_CHECK(e.runner.lastStartedEvent() == "suelo");
    MOTOR_CHECK(e.ultimaFrase() == "bajo los pies");
}

MOTOR_TEST_CASE(sin_mapa_no_hay_disparos_pero_el_interprete_avanza) {
    const MapDocument mapa = mapaCon({evento("intro", GridCoord{1, 1}, EventTrigger::Autorun,
                                             {cmd("control.esperar", {{"ms", "100"}})})});
    Escena e;
    e.montar(mapa);
    e.frame(7, 7);
    MOTOR_CHECK(e.runner.blocksPlayer());

    // Se carga otro nivel: el mapa desaparece de debajo del runner. Lo que
    // ya estaba vivo tiene que seguir su curso en vez de quedarse colgado
    // para siempre bloqueando al jugador.
    e.runner.setMap(nullptr);
    e.quedarse(20);

    MOTOR_CHECK(!e.runner.blocksPlayer());
    MOTOR_CHECK(e.interprete.active().empty());
    // Y sin mapa no se dispara nada nuevo, por muy dentro que este el jugador
    // de una casilla con evento.
    e.frame(1, 1, /*accion=*/true);
    e.correr();
    MOTOR_CHECK(e.runner.lastStartedEvent().empty());
}

MOTOR_TEST_CASE(autorun_vuelve_a_sonar_cuando_cambia_su_pagina_activa) {
    // Un Autorun de dos paginas: la segunda condicionada a una bandera. Cuando
    // la bandera se enciende cambia la pagina activa, y eso es un automatico
    // distinto: tiene que sonar. Es el patron del jefe que habla entre fases.
    MapEventDocument jefe;
    jefe.id = evt("jefe");
    jefe.name = "jefe";
    jefe.position = GridCoord{2, 2};
    jefe.pages = {
        pagina("fase1", EventTrigger::Autorun,
               {cmd("texto.mostrar", {{"texto", "primera fase"}}),
                cmd("bandera.encender", {{"bandera", "fase1_vista"}})}),
        pagina("fase2", EventTrigger::Autorun, {cmd("texto.mostrar", {{"texto", "segunda fase"}})},
               {bandera("fase1_vista")}),
    };
    const MapDocument mapa = mapaCon({jefe});
    Escena e;
    e.montar(mapa);

    e.frame(9, 9);
    e.correr();
    MOTOR_CHECK(e.host.log().size() == 1);
    MOTOR_CHECK(e.estado.hasFlag("fase1_vista"));

    e.quedarse(10);
    MOTOR_CHECK(e.host.log().size() == 2);
    MOTOR_CHECK(e.ultimaFrase() == "segunda fase");

    // Y no hay tercera: la pagina activa ya no vuelve a cambiar.
    e.quedarse(20);
    MOTOR_CHECK(e.host.log().size() == 2);
}

MOTOR_TEST_CASE(el_estado_de_disparo_se_reinicia_al_cargar_nivel) {
    const MapDocument mapa = mapaCon({evento("intro", GridCoord{1, 1}, EventTrigger::Autorun,
                                             {cmd("texto.mostrar", {{"texto", "hola"}})})});
    Escena e;
    e.montar(mapa);

    e.frame(7, 7);
    e.correr();
    MOTOR_CHECK(e.host.log().size() == 1);

    // Mismo mapa otra vez (el juego recarga el nivel al volver de una
    // transicion): los automaticos tienen que poder sonar de nuevo, aunque el
    // id sea el mismo.
    e.runner.resetTriggerState();
    e.quedarse(5);
    MOTOR_CHECK(e.host.log().size() == 2);
}

MOTOR_TEST_CASE(las_ejecuciones_terminadas_se_limpian) {
    const MapDocument mapa = misionLlave();
    Escena e;
    e.montar(mapa);

    // La mision entera son diez interacciones, o sea diez ejecuciones. Si el
    // runner no limpiara las terminadas, una partida larga acumularia una
    // ejecucion —con su copia del evento dentro— por cada vez que el jugador
    // pulsa accion, desde el principio de la sesion.
    for (const GridCoord& celda : {GridCoord{2, 3}, GridCoord{8, 4}, GridCoord{3, 4},
                                   GridCoord{2, 3}, GridCoord{12, 4}, GridCoord{8, 4},
                                   GridCoord{8, 4}, GridCoord{2, 3}, GridCoord{12, 4}}) {
        e.pulsar(celda.x, celda.y);
    }
    MOTOR_CHECK(e.host.log().size() >= 9);

    // Un frame mas, y de las ejecuciones no queda ninguna: todas terminaron y
    // el runner las recoge al empezar el frame siguiente, que es cuando quien
    // llama ya ha tenido el suyo para leer el desenlace en runs().
    e.quedarse(1);
    MOTOR_CHECK(e.interprete.runs().empty());
    MOTOR_CHECK(e.interprete.active().empty());
}

MOTOR_TEST_CASE(las_candidatas_de_interaccion_son_los_pies_y_las_cuatro_vecinas) {
    const std::vector<GridCoord> c = MapEventRunner::interactCandidates(GridCoord{5, 5});
    MOTOR_CHECK(c.size() == 5);
    MOTOR_CHECK(c[0].x == 5 && c[0].y == 5);  // primero los pies
    MOTOR_CHECK(c[1].x == 5 && c[1].y == 4);
    MOTOR_CHECK(c[2].x == 5 && c[2].y == 6);
    MOTOR_CHECK(c[3].x == 4 && c[3].y == 5);
    MOTOR_CHECK(c[4].x == 6 && c[4].y == 5);
}

// =======================================================================
// 4. H3-03 — el dialogo ramifica por interruptores, variables, objetos e
//    hitos. Sobre el asset PUBLICADO y conducido por casillas, igual que la
//    seccion 1: lo que un jugador puede hacer, no lo que un id puede pedir.
// =======================================================================

MOTOR_TEST_CASE(h303_el_mapa_publicado_pasa_la_validacion_estatica) {
    // E-07 existe justo para esto: una bandera que se consulta y nadie
    // enciende, un interruptor local de un evento que no existe, un parametro
    // a medio rellenar. Se comprueba ANTES de jugarlo, porque un contenido de
    // ejemplo roto ensena al autor a escribir mal.
    const MapDocument mapa = conversacionRamificada();
    const ValidationReport informe = ValidateEvents(mapa);
    for (const Diagnostic& d : informe.diagnostics) {
        if (d.isError()) {
            MOTOR_CHECK_FAIL("error de validacion en " + d.eventId + ": " + d.message);
        }
    }
    MOTOR_CHECK(informe.errorCount() == 0);
    MOTOR_CHECK(informe.canPlaytest());
}

MOTOR_TEST_CASE(h303_la_primera_charla_enciende_el_interruptor_local) {
    const MapDocument mapa = conversacionRamificada();
    Escena e;
    e.montar(mapa);

    // La tabernera esta en (4,3): se le habla de frente, desde (4,4). Nadie
    // dice "tabernera" ni "tabernera_desconocida": el runner la encuentra por
    // casillas y elige la pagina.
    e.pulsar(4, 4);

    MOTOR_CHECK(e.ultimaFrase().find("No te había visto") != std::string::npos);
    // El interruptor local es lo que hace que la proxima charla sea otra. Es
    // la pieza que mision_llave.json no usaba: alli todo son banderas.
    MOTOR_CHECK(e.estado.localSwitch("tabernera", 'A'));
    MOTOR_CHECK(e.estado.variable("charlas") == 1);
}

MOTOR_TEST_CASE(h303_volver_a_hablar_no_repite_la_misma_rama) {
    const MapDocument mapa = conversacionRamificada();
    Escena e;
    e.montar(mapa);

    e.pulsar(4, 4);
    const std::string primera = e.ultimaFrase();
    e.pulsar(4, 4);
    const std::string segunda = e.ultimaFrase();
    e.pulsar(4, 4);
    const std::string tercera = e.ultimaFrase();

    MOTOR_CHECK(!primera.empty());
    MOTOR_CHECK(primera != segunda);
    MOTOR_CHECK(segunda != tercera);
    MOTOR_CHECK(segunda.find("Otra vez tú") != std::string::npos);
    MOTOR_CHECK(tercera.find("Eres de la casa") != std::string::npos);
    // La rama la decide la variable, no el numero de pulsaciones: sin ella el
    // autor no podria escribir "a la tercera vez". La tercera charla ya no
    // suma, porque la pagina que gana no lleva variable.sumar.
    MOTOR_CHECK(e.estado.variable("charlas") == 2);
    MOTOR_CHECK(e.estado.hasFlag("habitual_de_la_casa"));
}

MOTOR_TEST_CASE(h303_un_objeto_en_el_inventario_cambia_la_conversacion) {
    const MapDocument mapa = conversacionRamificada();
    Escena e;
    e.montar(mapa);

    // El forastero esta en (8,3). Sin carta, se cierra.
    e.pulsar(8, 4);
    MOTOR_CHECK(e.ultimaFrase().find("menos sin una carta") != std::string::npos);
    MOTOR_CHECK(!e.estado.hasFlag("carta_vista"));

    // La carta esta en el mostrador (6,3): se pisa.
    e.pulsar(6, 3);
    MOTOR_CHECK(e.inventario.size() == 1);
    MOTOR_CHECK(e.inventario[0] == "carta_sellada");

    // Y la conversacion es otra: la condicion de objeto la evalua
    // ActivePageWithInventory contra el inventario real de la partida.
    e.pulsar(8, 4);
    MOTOR_CHECK(e.ultimaFrase().find("No la abras aquí dentro") != std::string::npos);
    MOTOR_CHECK(e.estado.hasFlag("carta_vista"));

    // Se deja la carta en el buzon (6,6) y la rama VUELVE atras: una
    // condicion de estado no es un hito, y si el autor quiere que no tenga
    // vuelta lo que debe usar es una bandera.
    e.pulsar(6, 6);
    MOTOR_CHECK(e.inventario.empty());
    e.pulsar(8, 4);
    MOTOR_CHECK(e.ultimaFrase().find("menos sin una carta") != std::string::npos);
}

MOTOR_TEST_CASE(h303_un_hito_jugado_cambia_la_conversacion) {
    const MapDocument mapa = conversacionRamificada();
    Escena e;
    e.montar(mapa);

    // El veterano esta en (8,6).
    e.pulsar(8, 7);
    MOTOR_CHECK(e.ultimaFrase().find("Grandes Cruzadas") != std::string::npos);
    MOTOR_CHECK(!e.estado.hasFlag("veterano_enterado"));

    // El hito lo enciende SU mision: el altar de mision_llave.json termina con
    // bandera.encender "mision_completada". Aqui se pone a mano porque la
    // prueba conduce un solo mapa, y es el mismo estado que escribe el altar.
    e.estado.setFlag("mision_completada");
    e.pulsar(8, 7);
    MOTOR_CHECK(e.ultimaFrase().find("tú lo abriste") != std::string::npos);
    MOTOR_CHECK(e.estado.hasFlag("veterano_enterado"));

    // Y no se repite: la pagina del hito no lleva nada que la desactive, asi
    // que el veterano sigue sabiendolo. Eso es un hito y no una condicion.
    e.pulsar(8, 7);
    MOTOR_CHECK(e.ultimaFrase().find("tú lo abriste") != std::string::npos);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
