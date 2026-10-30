// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-15  (fase E, 3 j) - Problemes i cerca d'esdeveniments
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Editor/ProblemsState.h"

#include "TestHarness.h"

#include <map>
#include <string>
#include <variant>
#include <vector>

// E-15 — Lista de problemas y busqueda de eventos.
//
// Lo que defiende cada caso: que la lista siga senalando LO MISMO
// despues de que el autor edite el documento. La trampa no es calcular
// la lista —eso es E-07— sino usarla mas tarde: si «Ir ahi» guarda
// «pagina 2» y el autor borra la pagina 0, la pagina 2 ya es otra.
// El criterio de cierre lo dice asi: «una referencia borrada no provoca
// navegacion a una entidad diferente».

namespace {

using namespace Editor;

ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }
ids::LayerId lyr(const std::string& v) { return ids::LayerId::parse(v).value(); }

MapEventCommand cmd(const std::string& tipo,
                    std::map<std::string, std::string> params = {}) {
    MapEventCommand c;
    c.type = tipo;
    c.params = std::move(params);
    return c;
}

MapEventPage pagina(const std::string& id, std::vector<MapEventCommand> comandos) {
    MapEventPage p;
    p.id = evt(id);
    p.name = id;
    p.commands = std::move(comandos);
    return p;
}

MapEventDocument evento(const std::string& id, std::vector<MapEventPage> paginas, int x = 0) {
    MapEventDocument e;
    e.id = evt(id);
    e.name = id;
    e.position = GridCoord{x, 0};
    e.pages = std::move(paginas);
    return e;
}

MapDocument mapaCon(std::vector<MapEventDocument> eventos, const std::string& id = "mapa") {
    MapDocument doc(ids::MapId::parse(id).value(), id, 16, 16, 64, 32);
    MapLayerMeta meta;
    meta.id = lyr("eventos");
    meta.name = "eventos";
    doc.addLayer(MapLayer::events(meta, EventLayerDocument()));
    for (const MapEventDocument& e : eventos) {
        // Comprobado a proposito: addEvent revalida la capa entera y
        // rechaza, entre otras cosas, un comando sin tipo. Sin este
        // check, un fixture mal escrito produce un mapa VACIO y el test
        // falla mucho mas abajo, diciendo que no encuentra el problema
        // en vez de que el evento nunca llego a entrar.
        MOTOR_CHECK(doc.addEvent(lyr("eventos"), e).isOk());
    }
    return doc;
}

// Un comando que E-07 marca como error SOLO en su propio sitio: un
// interruptor de un evento que no existe (el error de un tipo desconocido
// se repetiria tambien en el «si» que lo contiene). Sirve para fabricar
// un problema con sitio conocido sin depender de que un mensaje concreto
// no cambie. (Hasta el cierre de E-04 era «comercio.abrir», declarado y
// sin cablear; ya esta cableado.)
MapEventCommand roto() {
    return cmd("interruptor.fijar", {{"evento", "evento_fantasma"}, {"letra", "A"}, {"valor", "true"}});
}

const ProblemEntry* primeroEn(const ProblemsState& estado, const std::string& eventId) {
    for (const ProblemEntry& p : estado.problems()) {
        if (p.where.eventId == eventId) return &p;
    }
    return nullptr;
}

// ---------------------------------------------------------------------
// La lista
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(los_errores_van_arriba_y_son_los_que_bloquean_probar) {
    // El autor mira la primera linea. Si ahi hay un aviso y el error que
    // le impide probar esta el septimo, la lista no le sirve.
    const MapDocument doc = mapaCon({
        // Aviso: un evento sin paginas no hace nada, pero se puede guardar.
        evento("farol", {}, 1),
        // Error: interruptor de un evento que no existe.
        evento("tienda", {pagina("tienda_p1", {roto()})}, 2),
    });
    ProblemsState estado;
    estado.refreshMap(doc);

    MOTOR_CHECK(estado.errorCount() >= 1);
    MOTOR_CHECK(estado.warningCount() >= 1);
    MOTOR_CHECK(!estado.canPlaytest());
    MOTOR_CHECK(estado.problems().front().isError());
    // Y el ultimo no lo es: el orden separa de verdad los dos grupos.
    MOTOR_CHECK(!estado.problems().back().isError());
}

MOTOR_TEST_CASE(los_problemas_de_otros_mapas_no_se_borran_al_revalidar_este) {
    // Un error en otro mapa tambien bloquea exportar. Si revalidar el
    // mapa abierto vaciara la lista, el autor exportaria roto.
    const MapDocument uno = mapaCon({evento("tienda", {pagina("t_p1", {roto()})}, 1)}, "pueblo");
    const MapDocument dos = mapaCon({evento("posada", {pagina("p_p1", {roto()})}, 1)}, "bosque");

    ProblemsState estado;
    estado.refreshMap(uno);
    estado.refreshMap(dos);
    MOTOR_CHECK(estado.errorCount() == 2);

    // Revalidar el primero sin cambios no toca al segundo.
    estado.refreshMap(uno);
    MOTOR_CHECK(estado.errorCount() == 2);

    estado.removeMap("bosque");
    MOTOR_CHECK(estado.errorCount() == 1);
    MOTOR_CHECK(estado.problems().front().where.mapId == "pueblo");
}

// ---------------------------------------------------------------------
// El foco
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(revalidar_conserva_el_foco_cuando_el_problema_sigue_ahi) {
    // Corregir un defecto revalida y rehace la lista entera. Si el foco
    // fuera un numero de fila, el autor acabaria mirando otro problema
    // cada vez que arregla uno.
    const MapDocument doc = mapaCon({
        evento("tienda", {pagina("t_p1", {roto()})}, 1),
        evento("posada", {pagina("p_p1", {roto()})}, 2),
    });
    ProblemsState estado;
    estado.refreshMap(doc);
    MOTOR_CHECK(estado.problems().size() >= 2);

    const ProblemEntry* posada = primeroEn(estado, "posada");
    MOTOR_CHECK(posada != nullptr);
    const std::string clave = posada->key();
    MOTOR_CHECK(estado.focusOn(clave));

    estado.refreshMap(doc);
    MOTOR_CHECK(estado.focused() != nullptr);
    MOTOR_CHECK(estado.focused()->key() == clave);
    MOTOR_CHECK(estado.lastFocusChange() != ProblemsState::FocusChange::Lost);
}

MOTOR_TEST_CASE(arreglar_lo_que_miras_deja_el_foco_en_el_siguiente_y_lo_dice) {
    const MapDocument antes = mapaCon({
        evento("tienda", {pagina("t_p1", {roto()})}, 1),
        evento("posada", {pagina("p_p1", {roto()})}, 2),
    });
    ProblemsState estado;
    estado.refreshMap(antes);
    // Por clave y no por indice: la lista va ordenada por id de evento,
    // asi que el 0 es «posada».
    MOTOR_CHECK(estado.focusOn(primeroEn(estado, "tienda")->key()));
    const std::string arreglado = estado.focused()->key();

    // El autor quita el comando roto de «tienda».
    MapDocument despues = antes;
    despues.setEventPages(lyr("eventos"), evt("tienda"),
                          {pagina("t_p1", {cmd("texto.mostrar", {{"texto", "Hola"}})})});
    estado.refreshMap(despues);

    MOTOR_CHECK(estado.focused() != nullptr);
    MOTOR_CHECK(estado.focused()->key() != arreglado);
    MOTOR_CHECK(estado.lastFocusChange() == ProblemsState::FocusChange::MovedToNext);
}

// ---------------------------------------------------------------------
// Ir ahi — el criterio de cierre
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(ir_ahi_lleva_al_comando_exacto_cuando_nada_ha_cambiado) {
    const MapDocument doc = mapaCon({
        evento("tienda", {pagina("t_p1", {cmd("texto.mostrar", {{"texto", "Hola"}}), roto()})}, 1),
    });
    ProblemsState estado;
    estado.refreshMap(doc);
    MOTOR_CHECK(estado.focusIndexAt(0));

    const NavigationRequest ir = estado.navigate(doc);
    MOTOR_CHECK(ir.ok());
    MOTOR_CHECK(ir.target.eventId == "tienda");
    MOTOR_CHECK(ir.target.hasPage && ir.target.pageIndex == 0);
    MOTOR_CHECK(ir.target.hasCommand);
    MOTOR_CHECK(ir.target.commandPath == std::vector<std::size_t>{1});
    MOTOR_CHECK(ir.target.commandType == "interruptor.fijar");
}

MOTOR_TEST_CASE(borrar_una_pagina_no_manda_el_foco_a_la_pagina_de_al_lado) {
    // ESTE es el caso del criterio. El problema estaba en la pagina de
    // indice 2; el autor borra la 0; el indice 2 ya no existe y el 1 es
    // OTRA pagina. Navegar por indice abriria la equivocada sin avisar.
    const MapDocument antes = mapaCon({
        evento("tienda",
               {pagina("t_p1", {}), pagina("t_p2", {}), pagina("t_p3", {roto()})}, 1),
    });
    ProblemsState estado;
    estado.refreshMap(antes);
    const ProblemEntry* problema = primeroEn(estado, "tienda");
    MOTOR_CHECK(problema != nullptr);
    MOTOR_CHECK(problema->where.pageIndex == 2);
    MOTOR_CHECK(problema->where.pageId == "t_p3");
    MOTOR_CHECK(estado.focusOn(problema->key()));

    // Se borra la primera pagina: t_p3 pasa a ser la de indice 1.
    MapDocument despues = antes;
    despues.setEventPages(lyr("eventos"), evt("tienda"),
                          {pagina("t_p2", {}), pagina("t_p3", {roto()})});

    const NavigationRequest ir = estado.navigate(despues);
    MOTOR_CHECK(ir.ok());
    // Sigue siendo t_p3, no «la que hay en el indice 2».
    MOTOR_CHECK(ir.target.pageId == "t_p3");
    MOTOR_CHECK(ir.target.pageIndex == 1);
}

MOTOR_TEST_CASE(si_la_pagina_desaparece_se_abre_el_evento_y_se_dice_que_no_esta) {
    const MapDocument antes = mapaCon({
        evento("tienda", {pagina("t_p1", {}), pagina("t_p2", {roto()})}, 1),
    });
    ProblemsState estado;
    estado.refreshMap(antes);
    MOTOR_CHECK(estado.focusOn(primeroEn(estado, "tienda")->key()));

    MapDocument despues = antes;
    despues.setEventPages(lyr("eventos"), evt("tienda"), {pagina("t_p1", {})});

    const NavigationRequest ir = estado.navigate(despues);
    MOTOR_CHECK(ir.outcome == NavigationOutcome::PageGone);
    MOTOR_CHECK(!ir.ok());
    // Se puede llevar al autor al evento: eso si existe.
    MOTOR_CHECK(ir.navigable());
    MOTOR_CHECK(ir.target.eventId == "tienda");
    MOTOR_CHECK(!ir.target.hasPage);
    MOTOR_CHECK(!ir.note.empty());
}

MOTOR_TEST_CASE(borrar_un_comando_no_selecciona_al_vecino_que_ocupa_su_hueco) {
    // El diagnostico apuntaba al comando 1. El autor borra el 0. En el
    // indice 1 hay ahora otro comando, de otro tipo: seleccionarlo seria
    // senalar al inocente.
    const MapDocument antes = mapaCon({
        evento("tienda",
               {pagina("t_p1", {cmd("texto.mostrar", {{"texto", "Hola"}}), roto(),
                                cmd("texto.mostrar", {{"texto", "Adios"}})})},
               1),
    });
    ProblemsState estado;
    estado.refreshMap(antes);
    const ProblemEntry* problema = primeroEn(estado, "tienda");
    MOTOR_CHECK(problema != nullptr);
    MOTOR_CHECK(problema->where.commandType == "interruptor.fijar");
    MOTOR_CHECK(estado.focusOn(problema->key()));

    MapDocument despues = antes;
    despues.setEventPages(lyr("eventos"), evt("tienda"),
                          {pagina("t_p1", {cmd("texto.mostrar", {{"texto", "Hola"}}),
                                           cmd("texto.mostrar", {{"texto", "Adios"}})})});

    const NavigationRequest ir = estado.navigate(despues);
    MOTOR_CHECK(ir.outcome == NavigationOutcome::CommandGone);
    MOTOR_CHECK(!ir.target.hasCommand);
    // Pero la pagina si se abre: el autor quiere ver donde estaba.
    MOTOR_CHECK(ir.target.hasPage && ir.target.pageId == "t_p1");
}

MOTOR_TEST_CASE(un_evento_borrado_no_navega_a_la_casilla_que_ocupa_su_sitio) {
    const MapDocument antes = mapaCon({evento("tienda", {pagina("t_p1", {roto()})}, 1)});
    ProblemsState estado;
    estado.refreshMap(antes);
    MOTOR_CHECK(estado.focusOn(primeroEn(estado, "tienda")->key()));

    // Se borra el evento y se pone OTRO en la misma casilla.
    MapDocument despues = antes;
    despues.removeEvent(lyr("eventos"), evt("tienda"));
    despues.addEvent(lyr("eventos"), evento("posada", {pagina("p_p1", {})}, 1));

    const NavigationRequest ir = estado.navigate(despues);
    MOTOR_CHECK(ir.outcome == NavigationOutcome::EventGone);
    MOTOR_CHECK(!ir.navigable());
    MOTOR_CHECK(ir.target.eventId.empty());
}

MOTOR_TEST_CASE(un_problema_de_otro_mapa_no_navega_dentro_del_abierto) {
    const MapDocument bosque = mapaCon({evento("posada", {pagina("p_p1", {roto()})}, 1)}, "bosque");
    const MapDocument pueblo = mapaCon({evento("posada", {pagina("p_p1", {})}, 1)}, "pueblo");
    ProblemsState estado;
    estado.refreshMap(bosque);
    MOTOR_CHECK(estado.focusIndexAt(0));

    // Mismo id de evento en los dos mapas: sin comprobar el mapa,
    // navegaria al de al lado tan tranquilo.
    const NavigationRequest ir = estado.navigate(pueblo);
    MOTOR_CHECK(ir.outcome == NavigationOutcome::MapGone);
    MOTOR_CHECK(ir.note.find("bosque") != std::string::npos);
}

MOTOR_TEST_CASE(la_ruta_de_una_rama_se_resuelve_hasta_el_comando_de_dentro) {
    // Las rutas alternan indice de comando e indice de rama. Si se
    // leyeran como indices planos, «Ir ahi» sobre un comando dentro de
    // un «si» apuntaria a cualquier cosa.
    MapEventCommand si = cmd("control.si", {{"clase", "flag"}, {"referencia", "tiene_llave"}});
    MapEventCommand ramaSi;
    ramaSi.type = "rama.si";
    ramaSi.children.push_back(roto());
    MapEventCommand ramaNo;
    ramaNo.type = "rama.no";
    si.children.push_back(ramaSi);
    si.children.push_back(ramaNo);

    const MapDocument doc = mapaCon({
        evento("puerta", {pagina("pu_p1", {cmd("bandera.encender", {{"bandera", "tiene_llave"}}),
                                           si})}, 1),
    });
    ProblemsState estado;
    estado.refreshMap(doc);
    const ProblemEntry* problema = primeroEn(estado, "puerta");
    MOTOR_CHECK(problema != nullptr);
    MOTOR_CHECK(problema->where.commandPath == (std::vector<std::size_t>{1, 0, 0}));
    MOTOR_CHECK(problema->where.commandType == "interruptor.fijar");

    MOTOR_CHECK(estado.focusOn(problema->key()));
    const NavigationRequest ir = estado.navigate(doc);
    MOTOR_CHECK(ir.ok());
    MOTOR_CHECK(ir.target.commandPath == (std::vector<std::size_t>{1, 0, 0}));
}

// ---------------------------------------------------------------------
// Busqueda
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(buscar_por_nombre_ignora_mayusculas_y_acentos) {
    // DEC-15: el texto visible lleva ortografia correcta. El autor
    // escribe «mision» en la caja. Una busqueda que no lo encuentre no
    // sirve para nada.
    MapEventDocument e = evento("cofre", {pagina("c_p1", {})}, 1);
    e.name = "Misión de la Llave";
    MapEventDocument otro = evento("farol", {pagina("f_p1", {})}, 2);
    otro.name = "Farol del camí";
    const MapDocument doc = mapaCon({e, otro});

    MOTOR_CHECK(SearchEventsByName(doc, "mision").size() == 1);
    MOTOR_CHECK(SearchEventsByName(doc, "MISIÓN").size() == 1);
    MOTOR_CHECK(SearchEventsByName(doc, "cami").size() == 1);
    MOTOR_CHECK(SearchEventsByName(doc, "llave").front().where.eventId == "cofre");
    // Tambien por id, que es lo que el autor recuerda a veces.
    MOTOR_CHECK(SearchEventsByName(doc, "farol").size() == 1);
    // Caja vacia = la lista entera, no la lista vacia.
    MOTOR_CHECK(SearchEventsByName(doc, "").size() == 2);
    MOTOR_CHECK(SearchEventsByName(doc, "dragon").empty());
}

MOTOR_TEST_CASE(los_usos_de_una_bandera_salen_de_condiciones_y_de_comandos) {
    MapEventPage p = pagina("c_p1", {cmd("bandera.encender", {{"bandera", "tiene_llave"}}),
                                     cmd("bandera.apagar", {{"bandera", "otra"}})});
    MapEventCondition cond;
    cond.kind = EventConditionKind::Flag;
    cond.reference = "tiene_llave";
    p.conditions.push_back(cond);

    const MapDocument doc = mapaCon({
        evento("cofre", {p}, 1),
        evento("puerta",
               {pagina("pu_p1", {cmd("control.si", {{"clase", "flag"},
                                                    {"referencia", "tiene_llave"}})})}, 2),
    });

    const std::vector<SearchHit> usos = FindReferences(doc, ReferenceKind::Flag, "tiene_llave");
    // Tres: la condicion de pagina, el comando que la enciende y el
    // condicional que la lee.
    MOTOR_CHECK(usos.size() == 3);
    MOTOR_CHECK(FindReferences(doc, ReferenceKind::Flag, "otra").size() == 1);
    MOTOR_CHECK(FindReferences(doc, ReferenceKind::Flag, "no_existe").empty());
    // Nombre vacio no es «todas»: es «no has pedido nada».
    MOTOR_CHECK(FindReferences(doc, ReferenceKind::Flag, "").empty());
}

MOTOR_TEST_CASE(el_condicional_cuenta_como_bandera_o_como_variable_segun_su_clase) {
    // `control.si` declara `referencia` como texto libre porque su tipo
    // real depende de `clase`. Sin mirar la clase, buscar la variable
    // «monedas» encontraria tambien la bandera «monedas», que es otra
    // cosa distinta con el mismo nombre.
    const MapDocument doc = mapaCon({
        evento("a", {pagina("a_p1", {cmd("control.si", {{"clase", "flag"},
                                                        {"referencia", "monedas"}})})}, 1),
        evento("b", {pagina("b_p1", {cmd("control.si", {{"clase", "variable"},
                                                        {"referencia", "monedas"},
                                                        {"comparacion", ">="},
                                                        {"valor", "3"}})})}, 2),
    });

    const std::vector<SearchHit> comoBandera = FindReferences(doc, ReferenceKind::Flag, "monedas");
    const std::vector<SearchHit> comoVariable =
        FindReferences(doc, ReferenceKind::Variable, "monedas");
    MOTOR_CHECK(comoBandera.size() == 1);
    MOTOR_CHECK(comoBandera.front().where.eventId == "a");
    MOTOR_CHECK(comoVariable.size() == 1);
    MOTOR_CHECK(comoVariable.front().where.eventId == "b");
}

MOTOR_TEST_CASE(la_condicion_de_interruptor_local_no_se_confunde_con_una_bandera) {
    // Regresion del caso que faltaba en `ClaseDeCondicion`: sin el, toda
    // condicion de interruptor local se clasificaba como bandera, asi que
    // buscar el interruptor «A» no la encontraba y buscar la bandera «A»
    // encontraba una cosa que no es una bandera.
    MapEventPage p = pagina("c_p1", {});
    MapEventCondition cond;
    cond.kind = EventConditionKind::LocalSwitch;
    cond.reference = "A";
    p.conditions.push_back(cond);

    const MapDocument doc = mapaCon({evento("cofre", {p}, 1)});

    const std::vector<SearchHit> interruptores =
        FindReferences(doc, ReferenceKind::LocalSwitch, "A");
    MOTOR_CHECK(interruptores.size() == 1);
    MOTOR_CHECK(interruptores.front().where.eventId == "cofre");
    MOTOR_CHECK(FindReferences(doc, ReferenceKind::Flag, "A").empty());
    // Y el desplegable de nombres conocidos lo ofrece como interruptor.
    const std::vector<std::string> nombres =
        CollectReferenceNames(doc, ReferenceKind::LocalSwitch);
    MOTOR_CHECK(nombres.size() == 1);
    MOTOR_CHECK(nombres.front() == "A");
}

MOTOR_TEST_CASE(los_usos_saben_su_sitio_y_se_puede_ir_ahi) {
    // Un hallazgo de busqueda que no supiera donde esta obligaria al
    // autor a buscarlo otra vez a mano.
    const MapDocument doc = mapaCon({
        evento("cofre", {pagina("c_p1", {cmd("texto.mostrar", {{"texto", "Hola"}}),
                                         cmd("variable.sumar", {{"variable", "hierbas"},
                                                                {"valor", "1"}})})}, 1),
    });
    const std::vector<SearchHit> usos = FindReferences(doc, ReferenceKind::Variable, "hierbas");
    MOTOR_CHECK(usos.size() == 1);
    MOTOR_CHECK(usos.front().where.commandPath == std::vector<std::size_t>{1});
    MOTOR_CHECK(usos.front().eventName == "cofre");

    const NavigationRequest ir = ResolveNavigation(doc, usos.front().where);
    MOTOR_CHECK(ir.ok());
    MOTOR_CHECK(ir.target.commandType == "variable.sumar");
}

MOTOR_TEST_CASE(los_interruptores_y_los_eventos_referidos_tambien_se_buscan) {
    const MapDocument doc = mapaCon({
        evento("cofre", {pagina("c_p1", {cmd("interruptor.fijar", {{"evento", "puerta"},
                                                                   {"letra", "A"},
                                                                   {"valor", "true"}})})}, 1),
        evento("puerta", {pagina("pu_p1", {cmd("mapa.ir", {{"mapa", "bosque"}})})}, 2),
    });
    MOTOR_CHECK(FindReferences(doc, ReferenceKind::Event, "puerta").size() == 1);
    MOTOR_CHECK(FindReferences(doc, ReferenceKind::LocalSwitch, "A").size() == 1);
    MOTOR_CHECK(FindReferences(doc, ReferenceKind::Map, "bosque").size() == 1);
}

MOTOR_TEST_CASE(la_lista_de_nombres_usados_va_ordenada_y_sin_repetir) {
    // Es lo que el panel ofrece en el desplegable: sin esto el autor
    // escribe el nombre de memoria y se equivoca de bandera.
    const MapDocument doc = mapaCon({
        evento("a", {pagina("a_p1", {cmd("bandera.encender", {{"bandera", "zeta"}}),
                                     cmd("bandera.apagar", {{"bandera", "alfa"}})})}, 1),
        evento("b", {pagina("b_p1", {cmd("bandera.encender", {{"bandera", "zeta"}})})}, 2),
    });
    const std::vector<std::string> nombres =
        CollectReferenceNames(doc, ReferenceKind::Flag);
    MOTOR_CHECK(nombres.size() == 2);
    MOTOR_CHECK(nombres[0] == "alfa");
    MOTOR_CHECK(nombres[1] == "zeta");
}

}  // namespace

int main() { return motor_test::runAllTests(); }
