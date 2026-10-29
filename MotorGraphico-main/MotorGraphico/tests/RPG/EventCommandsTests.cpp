#include "RPG/EventCommand.h"
#include "Project/ProjectSerializer.h"

#include "TestHarness.h"

#include <string>
#include <vector>

// E-04 — El juego de comandas: catalogo, validacion, evaluacion de
// condiciones y el anfitrion de estado narrativo.

namespace {

using namespace RPG;

ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }

MapEventCommand cmd(const std::string& tipo,
                    std::map<std::string, std::string> params = {}) {
    MapEventCommand c;
    c.type = tipo;
    c.params = std::move(params);
    return c;
}

MOTOR_TEST_CASE(el_catalogo_distingue_declarado_de_implementado) {
    // Ofrecer al autor un comando que no hace nada es peor que no
    // ofrecerlo. El editor necesita saber que existe y que no esta listo.
    MOTOR_CHECK(!CommandCatalog().empty());
    const CommandSpec* texto = FindCommand("texto.mostrar");
    MOTOR_CHECK(texto != nullptr);
    MOTOR_CHECK(texto->implemented);

    // Combate se cableo en la tercera rebanada (SessionCommandHost):
    // arranca el encuentro y espera el desenlace. El catalogo ya no lo
    // declara como pendiente. [Rescate 13/09: esta comprobacion quedo
    // desfasada respecto a su propia implementacion.]
    const CommandSpec* combate = FindCommand("combate.iniciar");
    MOTOR_CHECK(combate != nullptr);
    MOTOR_CHECK(combate->implemented);
    // Y las familias de partida SI lo estan desde la segunda rebanada.
    MOTOR_CHECK(FindCommand("economia.oro")->implemented);
    MOTOR_CHECK(FindCommand("mapa.ir")->implemented);

    // Un tipo que no existe no esta en el catalogo, y eso no es un error:
    // puede venir de un proyecto hecho con una version mas nueva.
    MOTOR_CHECK(FindCommand("clima.lluvia") == nullptr);

    // Ningun id ni etiqueta repetidos: el editor los lista por id.
    for (std::size_t i = 0; i < CommandCatalog().size(); ++i) {
        for (std::size_t j = i + 1; j < CommandCatalog().size(); ++j) {
            MOTOR_CHECK(CommandCatalog()[i].type != CommandCatalog()[j].type);
        }
    }
}

MOTOR_TEST_CASE(la_validacion_dice_que_parametro_falla_y_por_que) {
    // Un diagnostico que no nombra el parametro obliga al autor a
    // adivinar cual de los cinco esta mal.
    MOTOR_CHECK(ValidateCommand(cmd("bandera.encender", {{"bandera", "llave"}})).isOk());

    const Result<bool> falta = ValidateCommand(cmd("bandera.encender"));
    MOTOR_CHECK(!falta.isOk());
    MOTOR_CHECK(falta.errorMessage().find("bandera") != std::string::npos);

    const Result<bool> vacio = ValidateCommand(cmd("bandera.encender", {{"bandera", ""}}));
    MOTOR_CHECK(!vacio.isOk());

    // Tipos: una duracion negativa, un entero que no lo es, una letra
    // fuera de A-D y una comparacion inventada.
    MOTOR_CHECK(!ValidateCommand(cmd("control.esperar", {{"ms", "-5"}})).isOk());
    MOTOR_CHECK(ValidateCommand(cmd("control.esperar", {{"ms", "400"}})).isOk());
    MOTOR_CHECK(!ValidateCommand(cmd("variable.sumar",
                                     {{"variable", "hierbas"}, {"valor", "dos"}})).isOk());
    MOTOR_CHECK(ValidateCommand(cmd("variable.sumar",
                                    {{"variable", "hierbas"}, {"valor", "-2"}})).isOk());
    MOTOR_CHECK(!ValidateCommand(cmd("interruptor.fijar",
                                     {{"evento", "cofre"}, {"letra", "Z"}, {"valor", "true"}})).isOk());
    MOTOR_CHECK(ValidateCommand(cmd("interruptor.fijar",
                                    {{"evento", "cofre"}, {"letra", "b"}, {"valor", "true"}})).isOk());

    // Un tipo desconocido se reporta como no disponible, no como roto.
    const Result<bool> desconocido = ValidateCommand(cmd("clima.lluvia"));
    MOTOR_CHECK(!desconocido.isOk());
    MOTOR_CHECK(desconocido.errorMessage().find("no disponible") != std::string::npos);
}

MOTOR_TEST_CASE(un_comando_no_puede_tener_mas_ramas_de_las_que_admite) {
    MapEventCommand texto = cmd("texto.mostrar", {{"texto", "hola"}});
    texto.children.push_back(cmd("bandera.encender", {{"bandera", "x"}}));
    const Result<bool> r = ValidateCommand(texto);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("rama") != std::string::npos);
}

MOTOR_TEST_CASE(una_condicion_de_inventario_da_error_y_no_falso) {
    // "No tiene la llave" y "no se mirar el inventario desde aqui" son
    // cosas distintas. Confundirlas convierte una pieza que falta en una
    // puerta que nunca se abre y que nadie sabe por que.
    NarrativeState estado;
    MapEventCondition item;
    item.kind = EventConditionKind::Item;
    item.reference = "llave_cueva";
    const Result<bool> r = EvaluateCondition(item, estado);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("inventario") != std::string::npos);
}

MOTOR_TEST_CASE(las_condiciones_de_bandera_y_variable_se_evaluan) {
    NarrativeState estado;
    estado.setFlag("prologo");
    estado.setVariable("hierbas", 3);

    MapEventCondition bandera;
    bandera.kind = EventConditionKind::Flag;
    bandera.reference = "prologo";
    // Sin valor, "esta encendida" es lo que el autor quiere decir el 90%
    // de las veces.
    MOTOR_CHECK(EvaluateCondition(bandera, estado).value());
    bandera.negated = true;
    MOTOR_CHECK(!EvaluateCondition(bandera, estado).value());

    MapEventCondition ausente = bandera;
    ausente.negated = false;
    ausente.reference = "epilogo";
    MOTOR_CHECK(!EvaluateCondition(ausente, estado).value());

    MapEventCondition variable;
    variable.kind = EventConditionKind::Variable;
    variable.reference = "hierbas";
    variable.value = "5";
    for (const char* op : {">=", ">", "=="}) {
        variable.comparison = op;
        MOTOR_CHECK(!EvaluateCondition(variable, estado).value());
    }
    for (const char* op : {"<", "<=", "!="}) {
        variable.comparison = op;
        MOTOR_CHECK(EvaluateCondition(variable, estado).value());
    }

    // Comparar una variable con algo que no es un numero da error, no falso.
    variable.value = "muchas";
    MOTOR_CHECK(!EvaluateCondition(variable, estado).isOk());
    // Y una bandera no admite mayor/menor.
    bandera.comparison = ">";
    MOTOR_CHECK(!EvaluateCondition(bandera, estado).isOk());
}

MOTOR_TEST_CASE(una_pagina_combina_sus_condiciones_con_su_and_o_or) {
    NarrativeState estado;
    estado.setFlag("a");

    MapEventPage pagina;
    pagina.id = evt("p1");
    MapEventCondition tieneA;
    tieneA.kind = EventConditionKind::Flag;
    tieneA.reference = "a";
    MapEventCondition tieneB = tieneA;
    tieneB.reference = "b";
    pagina.conditions = {tieneA, tieneB};

    pagina.requiresAllConditions = true;
    MOTOR_CHECK(!EvaluatePage(pagina, estado).value());
    pagina.requiresAllConditions = false;
    MOTOR_CHECK(EvaluatePage(pagina, estado).value());

    // Sin condiciones se cumple: es lo que espera quien pone una sola
    // pagina sin condicion ninguna.
    MapEventPage libre;
    libre.id = evt("p2");
    MOTOR_CHECK(EvaluatePage(libre, estado).value());
}

MOTOR_TEST_CASE(el_anfitrion_aplica_los_comandos_de_estado_narrativo) {
    NarrativeState estado;
    NarrativeCommandHost host(estado);

    host.execute(cmd("bandera.encender", {{"bandera", "llave_entregada"}}));
    MOTOR_CHECK(estado.hasFlag("llave_entregada"));
    host.execute(cmd("bandera.apagar", {{"bandera", "llave_entregada"}}));
    MOTOR_CHECK(!estado.hasFlag("llave_entregada"));

    host.execute(cmd("variable.fijar", {{"variable", "hierbas"}, {"valor", "2"}}));
    host.execute(cmd("variable.sumar", {{"variable", "hierbas"}, {"valor", "3"}}));
    MOTOR_CHECK(estado.variable("hierbas") == 5);

    host.execute(cmd("interruptor.fijar",
                     {{"evento", "cofre_norte"}, {"letra", "A"}, {"valor", "true"}}));
    MOTOR_CHECK(estado.localSwitch("cofre_norte", 'A'));
    MOTOR_CHECK(!estado.localSwitch("cofre_sur", 'A'));

    host.execute(cmd("texto.mostrar", {{"texto", "Está vacío"}, {"hablante", "Cofre"}}));
    MOTOR_CHECK(host.log().size() == 1);
    MOTOR_CHECK(host.log()[0] == "Cofre: Está vacío");
}

MOTOR_TEST_CASE(el_condicional_elige_rama_0_si_se_cumple_y_1_si_no) {
    NarrativeState estado;
    estado.setVariable("hierbas", 5);
    NarrativeCommandHost host(estado);

    const CommandResult si = host.execute(cmd("control.si", {{"clase", "variable"},
                                                             {"referencia", "hierbas"},
                                                             {"comparacion", ">="},
                                                             {"valor", "5"}}));
    MOTOR_CHECK(si.outcome == CommandOutcome::Branch);
    MOTOR_CHECK(si.branch == 0);

    const CommandResult no = host.execute(cmd("control.si", {{"clase", "variable"},
                                                             {"referencia", "hierbas"},
                                                             {"comparacion", ">"},
                                                             {"valor", "5"}}));
    MOTOR_CHECK(no.branch == 1);

    // Una clase inventada falla nombrandola.
    const CommandResult malo = host.execute(cmd("control.si", {{"clase", "tiempo"},
                                                               {"referencia", "lluvia"}}));
    MOTOR_CHECK(malo.outcome == CommandOutcome::Failed);
    MOTOR_CHECK(malo.diagnostic.find("tiempo") != std::string::npos);
}

MOTOR_TEST_CASE(un_comando_de_partida_sin_sesion_falla_diciendolo) {
    // No se ejecuta a medias ni se ignora en silencio. (Hasta el cierre de
    // E-04 este caso usaba comercio.abrir como «declarado y no cableado»;
    // ya esta cableado y ningun comando queda en ese estado.)
    NarrativeState estado;
    NarrativeCommandHost host(estado);
    const CommandResult r = host.execute(cmd("comercio.abrir", {{"tienda", "general"}}));
    MOTOR_CHECK(r.outcome == CommandOutcome::Failed);
    MOTOR_CHECK(r.diagnostic.find("comercio.abrir") != std::string::npos);
}

MOTOR_TEST_CASE(un_comando_a_medio_rellenar_falla_antes_de_ejecutarse) {
    // Sin esto se ejecutaria con un valor inventado: encender la bandera
    // "" en vez de decir que falta el parametro.
    NarrativeState estado;
    NarrativeCommandHost host(estado);
    const CommandResult r = host.execute(cmd("bandera.encender"));
    MOTOR_CHECK(r.outcome == CommandOutcome::Failed);
    MOTOR_CHECK(estado.flags().empty());
}

MOTOR_TEST_CASE(esperar_cede_el_turno_con_su_duracion) {
    NarrativeState estado;
    NarrativeCommandHost host(estado);
    const CommandResult r = host.execute(cmd("control.esperar", {{"ms", "400"}}));
    MOTOR_CHECK(r.outcome == CommandOutcome::Wait);
    MOTOR_CHECK(r.waitMs == 400);
}

MOTOR_TEST_CASE(el_cofre_de_verdad_se_abre_una_sola_vez) {
    // La prueba que une la cadena entera: E-01 guarda el evento con sus
    // paginas, E-02 el estado, E-03 lo ejecuta y E-04 dice que significa
    // cada comando. Es el recorrido del cofre del piloto de DEC-07.
    //
    // Pagina 1 (cofre sin abrir): da la llave, sube el contador, enciende
    // el interruptor local A y lo dice.
    MapEventPage sinAbrir;
    sinAbrir.id = evt("cofre_p1");
    sinAbrir.trigger = EventTrigger::Interact;
    MapEventCommand dar = cmd("variable.sumar", {{"variable", "llaves"}, {"valor", "1"}});
    MapEventCommand marcar = cmd("interruptor.fijar",
                                 {{"evento", "cofre_norte"}, {"letra", "A"}, {"valor", "true"}});
    MapEventCommand decir = cmd("texto.mostrar", {{"texto", "Dentro hay una llave."}});
    sinAbrir.commands = {dar, marcar, decir};

    MapEventDocument cofre;
    cofre.id = evt("cofre_norte");
    cofre.name = "Cofre";
    cofre.position = GridCoord{3, 4};
    cofre.pages.push_back(sinAbrir);

    // Pagina 2 (ya abierto): solo lo dice. Su condicion mira el contador.
    MapEventPage abierto;
    abierto.id = evt("cofre_p2");
    abierto.trigger = EventTrigger::Interact;
    MapEventCondition yaTiene;
    yaTiene.kind = EventConditionKind::Variable;
    yaTiene.reference = "llaves";
    yaTiene.comparison = ">=";
    yaTiene.value = "1";
    abierto.conditions.push_back(yaTiene);
    abierto.commands = {cmd("texto.mostrar", {{"texto", "Está vacío."}})};
    cofre.pages.push_back(abierto);

    NarrativeState estado;
    NarrativeCommandHost host(estado);
    EventInterpreter interprete(host);

    // Primera interaccion: la pagina 2 todavia no se cumple, asi que va
    // la 1. (Quien ELIGE la pagina es DEC-10, sin responder: aqui se
    // elige explicitamente, que es lo que hace el interprete hoy.)
    MOTOR_CHECK(!EvaluatePage(cofre.pages[1], estado).value());
    MOTOR_CHECK(interprete.start(cofre, 0).isOk());
    interprete.tick(0);
    MOTOR_CHECK(!interprete.isRunning("cofre_norte"));
    interprete.collectFinished();

    MOTOR_CHECK(estado.variable("llaves") == 1);
    MOTOR_CHECK(estado.localSwitch("cofre_norte", 'A'));
    MOTOR_CHECK(host.log().size() == 1);

    // Segunda interaccion: ahora la pagina 2 SI se cumple, y el contador
    // no vuelve a subir. Es lo que impide que un cofre pague dos veces.
    MOTOR_CHECK(EvaluatePage(cofre.pages[1], estado).value());
    MOTOR_CHECK(interprete.start(cofre, 1).isOk());
    interprete.tick(0);
    interprete.collectFinished();

    MOTOR_CHECK(estado.variable("llaves") == 1);
    MOTOR_CHECK(host.log().size() == 2);
    MOTOR_CHECK(host.log()[1] == "Está vacío.");

    // Y todo esto es un punto seguro para guardar (DEC-09).
    MOTOR_CHECK(interprete.canSave());
}

MOTOR_TEST_CASE(una_rama_del_condicional_se_ejecuta_de_verdad_en_el_interprete) {
    // El condicional devuelve Branch; que la rama correcta se EJECUTE es
    // cosa del interprete. Probarlo junto es lo unico que demuestra que
    // los dos hablan el mismo idioma.
    MapEventCommand si = cmd("control.si", {{"clase", "flag"}, {"referencia", "tiene_llave"}});
    MapEventCommand ramaSi;
    ramaSi.type = "rama.si";
    ramaSi.children.push_back(cmd("texto.mostrar", {{"texto", "La puerta se abre."}}));
    MapEventCommand ramaNo;
    ramaNo.type = "rama.no";
    ramaNo.children.push_back(cmd("texto.mostrar", {{"texto", "Está cerrada."}}));
    si.children = {ramaSi, ramaNo};

    MapEventDocument puerta;
    puerta.id = evt("puerta");
    puerta.position = GridCoord{1, 1};
    MapEventPage p;
    p.id = evt("puerta_p1");
    p.commands = {si};
    puerta.pages.push_back(p);

    // Sin la llave: rama 1.
    {
        NarrativeState estado;
        NarrativeCommandHost host(estado);
        EventInterpreter interprete(host);
        MOTOR_CHECK(interprete.start(puerta, 0).isOk());
        interprete.tick(0);
        MOTOR_CHECK(host.log().size() == 1);
        MOTOR_CHECK(host.log()[0] == "Está cerrada.");
    }
    // Con la llave: rama 0.
    {
        NarrativeState estado;
        estado.setFlag("tiene_llave");
        NarrativeCommandHost host(estado);
        EventInterpreter interprete(host);
        MOTOR_CHECK(interprete.start(puerta, 0).isOk());
        interprete.tick(0);
        MOTOR_CHECK(host.log().size() == 1);
        MOTOR_CHECK(host.log()[0] == "La puerta se abre.");
    }
}

}  // namespace

// ---------------------------------------------------------------------
// E-09 — Que pagina de un evento esta activa
// ---------------------------------------------------------------------

namespace {

MapEventPage paginaCon(const std::string& id, std::vector<MapEventCondition> condiciones) {
    MapEventPage p;
    p.id = evt(id);
    p.conditions = std::move(condiciones);
    p.commands = {cmd("texto.mostrar", {{"texto", id}})};
    return p;
}

MapEventCondition banderaCond(const std::string& nombre) {
    MapEventCondition c;
    c.kind = EventConditionKind::Flag;
    c.reference = nombre;
    return c;
}

}  // namespace

MOTOR_TEST_CASE(gana_la_ultima_pagina_cuyas_condiciones_se_cumplen) {
    // Las paginas se leen como estados sucesivos y el autor añade el
    // nuevo debajo. Si ganara la primera, añadir una pagina no cambiaria
    // nada y el cofre no se abriria nunca.
    //
    // PROVISIONAL, pendiente de DEC-10: es el borrador que la propia
    // ficha propone, no una decision tomada.
    MapEventDocument cofre;
    cofre.id = evt("cofre");
    cofre.position = GridCoord{1, 1};
    cofre.pages = {paginaCon("p_cerrado", {}),
                   paginaCon("p_abierto", {banderaCond("abierto")}),
                   paginaCon("p_vacio", {banderaCond("vaciado")})};

    NarrativeState estado;
    MOTOR_CHECK(ActivePage(cofre, estado).index == 0);

    estado.setFlag("abierto");
    MOTOR_CHECK(ActivePage(cofre, estado).index == 1);

    // Se cumplen la 1 y la 2 a la vez: gana la ultima.
    estado.setFlag("vaciado");
    const ActivePageResult activa = ActivePage(cofre, estado);
    MOTOR_CHECK(activa.index == 2);
    MOTOR_CHECK(activa.page != nullptr);
    MOTOR_CHECK(activa.page->id.value() == "p_vacio");
    MOTOR_CHECK(activa.trustworthy());
}

MOTOR_TEST_CASE(un_evento_sin_ninguna_pagina_valida_no_tiene_pagina_activa) {
    // No es un error: un evento puede quedarse sin nada que hacer. Lo que
    // no vale es devolver la pagina 0 como si se cumpliera.
    MapEventDocument piedra;
    piedra.id = evt("piedra");
    piedra.position = GridCoord{1, 1};
    piedra.pages = {paginaCon("p1", {banderaCond("nunca")})};

    NarrativeState estado;
    const ActivePageResult activa = ActivePage(piedra, estado);
    MOTOR_CHECK(activa.page == nullptr);
    MOTOR_CHECK(activa.trustworthy());
}

MOTOR_TEST_CASE(una_pagina_que_no_se_puede_evaluar_no_cuenta_como_falsa_y_se_dice) {
    // Una condicion de inventario sin inventario que mirar no es "no lo
    // lleva": es "no lo se". Tratarlo como falso convierte una pieza que
    // falta en una puerta que no se abre y que nadie sabe por que.
    MapEventCondition llave;
    llave.kind = EventConditionKind::Item;
    llave.reference = "llave";

    MapEventDocument puerta;
    puerta.id = evt("puerta");
    puerta.position = GridCoord{1, 1};
    puerta.pages = {paginaCon("p_cerrada", {}), paginaCon("p_abierta", {llave})};

    NarrativeState estado;
    const ActivePageResult activa = ActivePage(puerta, estado);
    // Elige la 0 porque es la unica que sabe evaluar, PERO avisa de que
    // la eleccion no es de fiar.
    MOTOR_CHECK(activa.index == 0);
    MOTOR_CHECK(!activa.trustworthy());
    MOTOR_CHECK(activa.unevaluable == std::vector<std::size_t>{1});
    MOTOR_CHECK(!activa.diagnostic.empty());
}

MOTOR_TEST_CASE(un_si_sin_comparacion_escrita_deja_que_cada_clase_ponga_la_suya) {
    // Rellenar "==" por defecto convertiria «lleva la llave» en «lleva
    // exactamente una llave», y la puerta no se abriria con dos.
    const Result<MapEventCondition> c =
        ConditionFromIf(cmd("control.si", {{"clase", "item"}, {"referencia", "llave"}}));
    MOTOR_CHECK(c.isOk());
    MOTOR_CHECK(c.value().comparison.empty());
    MOTOR_CHECK(c.value().kind == EventConditionKind::Item);

    const Result<MapEventCondition> mala =
        ConditionFromIf(cmd("control.si", {{"clase", "tiempo"}, {"referencia", "lluvia"}}));
    MOTOR_CHECK(!mala.isOk());
}

// ---------------------------------------------------------------------
// EVT-05: interruptores locales (condición + comando)
// ---------------------------------------------------------------------

namespace {

// Helper: estado narrativo con interruptor local encendido
NarrativeState estadoConInterruptor(const std::string& eventId, char letra, bool valor) {
    NarrativeState estado;
    estado.setLocalSwitch(eventId, letra, valor);
    return estado;
}

// Doble de pruebas: host que devuelve un resultado preconfigurado por tipo.
struct HostDePrueba : EventHost {
    std::map<std::string, CommandResult> resultadosPreparados;
    CommandResult execute(const MapEventCommand& command) override {
        auto it = resultadosPreparados.find(command.type);
        if (it != resultadosPreparados.end()) return it->second;
        return CommandResult{CommandOutcome::Done};
    }
};

// Helper: crea un MapEventDocument amb una sola pàgina i un sol comando.
MapEventDocument EventoSimple(const std::string& id, const std::string& cmdType,
                               EventTrigger trigger,
                               std::map<std::string, std::string> params = {}) {
    MapEventDocument e;
    e.id = ids::EventId::parse(id).value();
    e.name = id;
    e.position = GridCoord{0, 0};
    MapEventPage p;
    p.id = ids::EventId::parse(id + "_p1").value();
    p.name = "pagina";
    p.trigger = trigger;
    MapEventCommand cmd;
    cmd.type = cmdType;
    cmd.params = std::move(params);
    p.commands.push_back(cmd);
    e.pages.push_back(p);
    return e;
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
    MOTOR_CHECK(r.value() == true);  // != "true" -> false, negado -> !false = true
}

MOTOR_TEST_CASE(evt05_comando_interruptor_fijar_true) {
    NarrativeState estado;
    NarrativeCommandHost host(estado);
    EventInterpreter interprete(host);

    MapEventDocument evento = EventoSimple("evt_test", "interruptor.fijar",
        EventTrigger::Autorun,
        {{"evento", "evt_cofre"}, {"letra", "A"}, {"valor", "true"}});

    auto r = interprete.start(evento, 0);
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

// E-05 — evt05_local_switch_roundtrip: una condició LocalSwitch
// sobreviu la serialització JSON ida i tornada.
MOTOR_TEST_CASE(evt05_local_switch_roundtrip) {
    using namespace RPG;

    // 1. Crear un MapDocument amb un event que té condició LocalSwitch.
    ids::MapId mapId = ids::MapId::parse("mapa_test").value();
    ids::LayerId lyrId = ids::LayerId::parse("eventos").value();
    ids::EventId evtId = ids::EventId::parse("puerta_magica").value();

    MapDocument original(mapId, "mapa_test", 8, 8, 64, 32);
    MapLayerMeta meta;
    meta.id = lyrId;
    meta.name = "eventos";
    meta.order = 0;
    original.addLayer(MapLayer::events(meta, EventLayerDocument()));

    MapEventDocument evento;
    evento.id = evtId;
    evento.name = "Puerta magica";
    evento.position = GridCoord{2, 3};

    MapEventPage pagina;
    pagina.id = ids::EventId::parse("puerta_magica_p1").value();
    pagina.name = "pagina principal";
    pagina.trigger = EventTrigger::Interact;

    // Condició: si l'interruptor local A de "puerta_magica" està activat.
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = "puerta_magica.A";
    cond.comparison = "==";
    cond.value = "true";
    cond.negated = false;
    pagina.conditions.push_back(cond);

    // Comando: texte.mostrar
    MapEventCommand cmd;
    cmd.type = "texto.mostrar";
    cmd.params["texto"] = "La puerta se abre";
    pagina.commands.push_back(cmd);

    evento.pages.push_back(pagina);
    original.addEvent(lyrId, evento);

    // 2. Serialitzar.
    const Result<std::string> json = ProjectSerializer::serializeMap(original);
    MOTOR_CHECK(json.isOk());

    // 3. Deserialitzar.
    const Result<MapDocument> vuelta = ProjectSerializer::deserializeMap(json.value());
    MOTOR_CHECK(vuelta.isOk());

    // 4. Verificar que la condició LocalSwitch va sobreviure.
    const EventLayerDocument& capa =
        std::get<EventLayerDocument>(vuelta.value().findLayer(lyrId)->data());
    const MapEventDocument* leido = capa.find(evtId);
    MOTOR_CHECK(leido != nullptr);
    MOTOR_CHECK(leido->pages.size() == 1);

    const MapEventPage& p = leido->pages[0];
    MOTOR_CHECK(p.conditions.size() == 1);
    MOTOR_CHECK(p.conditions[0].kind == EventConditionKind::LocalSwitch);
    MOTOR_CHECK(p.conditions[0].reference == "puerta_magica.A");
    MOTOR_CHECK(p.conditions[0].comparison == "==");
    MOTOR_CHECK(p.conditions[0].value == "true");
    MOTOR_CHECK(p.conditions[0].negated == false);

    // 5. Serialitzar de nou i comparar: ida i tornada estable.
    const Result<std::string> segunda = ProjectSerializer::serializeMap(vuelta.value());
    MOTOR_CHECK(segunda.isOk());
    MOTOR_CHECK(segunda.value() == json.value());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
