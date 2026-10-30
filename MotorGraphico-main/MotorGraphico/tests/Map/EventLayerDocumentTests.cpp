#include "Map/EventLayerDocument.h"
#include "Map/MapDocument.h"
#include "Project/ProjectSerializer.h"

#include "TestHarness.h"

#include <string>
#include <vector>

// E-01 — El evento es parte del documento de mapa.
//
// Lo que estas pruebas defienden, en una linea cada una: que una casilla
// no pueda tener dos eventos, que un comando de un tipo que este motor no
// conoce se guarde y se lea sin perder un campo, que un evento fuera del
// mapa lo rechace el documento y no la capa, y que una operacion que
// falla deje el documento exactamente como estaba.

namespace {

ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }
ids::LayerId lyr(const std::string& v) { return ids::LayerId::parse(v).value(); }

MapEventPage pagina(const std::string& id) {
    MapEventPage p;
    p.id = evt(id);
    p.name = "pagina " + id;
    return p;
}

MapEventDocument cofre(const std::string& id, int x, int y) {
    MapEventDocument e;
    e.id = evt(id);
    e.name = "Cofre";
    e.position = GridCoord{x, y};
    MapEventPage sinAbrir = pagina(id + "_p1");
    sinAbrir.trigger = EventTrigger::Interact;
    MapEventCondition cond;
    cond.kind = EventConditionKind::Flag;
    cond.reference = "cofre_abierto";
    cond.comparison = "==";
    cond.value = "false";
    sinAbrir.conditions.push_back(cond);
    MapEventCommand dar;
    dar.type = "recompensa.entregar";
    dar.params["recompensa"] = "llave_cueva";
    sinAbrir.commands.push_back(dar);
    e.pages.push_back(sinAbrir);
    return e;
}

MapDocument mapaCon(const MapEventDocument& evento, int ancho = 8, int alto = 8) {
    MapDocument doc(ids::MapId::parse("mapa").value(), "mapa", ancho, alto, 64, 32);
    MapLayerMeta meta;
    meta.id = lyr("eventos");
    meta.name = "eventos";
    meta.order = 0;
    doc.addLayer(MapLayer::events(meta, EventLayerDocument()));
    doc.addEvent(lyr("eventos"), evento);
    return doc;
}

MOTOR_TEST_CASE(una_casilla_no_admite_dos_eventos) {
    EventLayerDocument capa;
    MOTOR_CHECK(capa.addEvent(cofre("cofre_1", 3, 4)).isOk());

    // Mismo sitio, otro evento: se rechaza y la capa queda con uno solo.
    const Result<bool> segundo = capa.addEvent(cofre("cofre_2", 3, 4));
    MOTOR_CHECK(!segundo.isOk());
    MOTOR_CHECK(capa.events().size() == 1);

    // Y mover uno encima de otro tampoco cuela.
    MOTOR_CHECK(capa.addEvent(cofre("cofre_3", 5, 5)).isOk());
    MOTOR_CHECK(!capa.setEventPosition(evt("cofre_3"), GridCoord{3, 4}).isOk());
    MOTOR_CHECK(capa.find(evt("cofre_3"))->position.x == 5);

    // Moverlo a su propia casilla no es un choque consigo mismo.
    MOTOR_CHECK(capa.setEventPosition(evt("cofre_3"), GridCoord{5, 5}).isOk());
}

MOTOR_TEST_CASE(un_comando_desconocido_no_pierde_ni_un_campo) {
    // El requisito del diseno de blocs §9: un tipo que este motor no
    // conoce conserva sus datos y no se elimina al abrir/guardar. Si esto
    // falla, abrir una aventura con una version mas nueva del editor y
    // volver a guardarla la mutila en silencio.
    MapEventDocument e = cofre("puerta", 1, 1);
    MapEventCommand futuro;
    futuro.type = "clima.lluvia";          // no existe en este motor
    futuro.version = 7;
    futuro.params["intensidad"] = "0.8";
    futuro.params["duracion_ms"] = "4000";
    MapEventCommand rama;
    rama.type = "texto.mostrar";
    rama.params["texto"] = "Empieza a llover";
    futuro.children.push_back(rama);
    e.pages[0].commands.push_back(futuro);

    EventLayerDocument capa;
    MOTOR_CHECK(capa.addEvent(e).isOk());

    const MapEventCommand& guardado = capa.find(evt("puerta"))->pages[0].commands[1];
    MOTOR_CHECK(guardado.type == "clima.lluvia");
    MOTOR_CHECK(guardado.version == 7);
    MOTOR_CHECK(guardado.params.size() == 2);
    MOTOR_CHECK(guardado.params.at("intensidad") == "0.8");
    MOTOR_CHECK(guardado.params.at("duracion_ms") == "4000");
    MOTOR_CHECK(guardado.children.size() == 1);
    MOTOR_CHECK(guardado.children[0].params.at("texto") == "Empieza a llover");
}

MOTOR_TEST_CASE(el_documento_rechaza_un_evento_fuera_del_mapa) {
    // La capa por si sola NO sabe en que mapa vive: acepta la casilla 99.
    EventLayerDocument capa;
    MOTOR_CHECK(capa.addEvent(cofre("lejos", 99, 99)).isOk());
    MOTOR_CHECK(capa.validate().isOk());

    // Es el documento quien la rechaza, igual que hace con las entidades.
    MapDocument doc(ids::MapId::parse("mapa").value(), "mapa", 8, 8, 64, 32);
    MapLayerMeta meta;
    meta.id = lyr("eventos");
    meta.name = "eventos";
    const Result<bool> anadida = doc.addLayer(MapLayer::events(meta, capa));
    MOTOR_CHECK(!anadida.isOk());
    MOTOR_CHECK(doc.layers().empty());
}

MOTOR_TEST_CASE(una_operacion_que_falla_no_cambia_el_documento) {
    MapDocument doc = mapaCon(cofre("cofre_1", 2, 2));
    MOTOR_CHECK(doc.layers().size() == 1);

    // Fuera del mapa: mutateLayer revalida y tira la copia.
    MOTOR_CHECK(!doc.moveEvent(lyr("eventos"), evt("cofre_1"), GridCoord{50, 50}).isOk());
    // Capa que no es de eventos: mensaje claro, sin tocar nada.
    MOTOR_CHECK(!doc.addEvent(lyr("no_existe"), cofre("x", 1, 1)).isOk());

    const MapLayer* capa = doc.findLayer(lyr("eventos"));
    const EventLayerDocument& eventos = std::get<EventLayerDocument>(capa->data());
    MOTOR_CHECK(eventos.events().size() == 1);
    MOTOR_CHECK(eventos.find(evt("cofre_1"))->position.x == 2);
    MOTOR_CHECK(eventos.find(evt("cofre_1"))->position.y == 2);
    MOTOR_CHECK(doc.validate().isOk());
}

MOTOR_TEST_CASE(las_paginas_se_cambian_en_un_solo_paso_y_conservan_su_orden) {
    // Reordenar o borrar una pagina es UN cambio del autor (E-05): si se
    // pudiera quedar a medias, el evento acabaria con paginas de dos
    // versiones distintas.
    MapDocument doc = mapaCon(cofre("cofre_1", 2, 2));

    std::vector<MapEventPage> nuevas;
    nuevas.push_back(pagina("cofre_1_p2"));
    nuevas.push_back(pagina("cofre_1_p1"));
    nuevas.push_back(pagina("cofre_1_p3"));
    MOTOR_CHECK(doc.setEventPages(lyr("eventos"), evt("cofre_1"), nuevas).isOk());

    const EventLayerDocument& eventos =
        std::get<EventLayerDocument>(doc.findLayer(lyr("eventos"))->data());
    const MapEventDocument* e = eventos.find(evt("cofre_1"));
    MOTOR_CHECK(e->pages.size() == 3);
    // El orden se conserva tal cual: quien decide que pagina gana es el
    // interprete (DEC-10, sin responder), no este documento.
    MOTOR_CHECK(e->pages[0].id == evt("cofre_1_p2"));
    MOTOR_CHECK(e->pages[1].id == evt("cofre_1_p1"));
    MOTOR_CHECK(e->pages[2].id == evt("cofre_1_p3"));

    // Dos paginas con el mismo id se rechazan y las de antes siguen ahi.
    std::vector<MapEventPage> repetidas;
    repetidas.push_back(pagina("igual"));
    repetidas.push_back(pagina("igual"));
    MOTOR_CHECK(!doc.setEventPages(lyr("eventos"), evt("cofre_1"), repetidas).isOk());
    const EventLayerDocument& despues =
        std::get<EventLayerDocument>(doc.findLayer(lyr("eventos"))->data());
    MOTOR_CHECK(despues.find(evt("cofre_1"))->pages.size() == 3);
}

MOTOR_TEST_CASE(un_anidamiento_desbocado_se_rechaza_sin_reventar_la_pila) {
    // Un documento corrupto o generado por un bucle no debe tumbar al
    // editor al validarlo: tiene que salir por un error.
    MapEventDocument e = cofre("hondo", 0, 0);
    MapEventCommand raiz;
    raiz.type = "control.si";
    MapEventCommand* actual = &raiz;
    for (std::size_t i = 0; i < EventLayerDocument::kMaxCommandDepth + 5; ++i) {
        MapEventCommand hijo;
        hijo.type = "control.si";
        actual->children.push_back(hijo);
        actual = &actual->children.back();
    }
    e.pages[0].commands.push_back(raiz);

    EventLayerDocument capa;
    const Result<bool> anadido = capa.addEvent(e);
    MOTOR_CHECK(!anadido.isOk());
    MOTOR_CHECK(capa.events().empty());

    // Y un anidamiento normal, justo en el limite, si entra.
    MapEventDocument ok = cofre("justo", 1, 0);
    MapEventCommand cadena;
    cadena.type = "control.si";
    MapEventCommand* punta = &cadena;
    for (std::size_t i = 1; i < EventLayerDocument::kMaxCommandDepth; ++i) {
        MapEventCommand hijo;
        hijo.type = "control.si";
        punta->children.push_back(hijo);
        punta = &punta->children.back();
    }
    ok.pages[0].commands.push_back(cadena);
    MOTOR_CHECK(capa.addEvent(ok).isOk());
}

MOTOR_TEST_CASE(una_condicion_sin_nombre_se_rechaza_al_anadir) {
    // Una condicion que no nombra nada se evaluaria siempre igual y el
    // autor no tendria forma de saber por que su cofre no se abre.
    MapEventDocument e = cofre("mudo", 0, 0);
    MapEventCondition vacia;
    vacia.reference = "";
    e.pages[0].conditions.push_back(vacia);

    EventLayerDocument capa;
    MOTOR_CHECK(!capa.addEvent(e).isOk());
    MOTOR_CHECK(capa.events().empty());
}

MOTOR_TEST_CASE(el_evento_va_y_vuelve_del_json_del_mapa_sin_perder_nada) {
    // "Es desa amb el mapa" es el criterio de cierre de E-01. Sin esta
    // prueba es una frase: el evento podria guardarse a medias y el mapa
    // seguiria abriendo, solo que la puerta dejaria de pedir la llave.
    MapEventDocument e = cofre("cofre_1", 3, 4);
    e.pages[0].requiresAllConditions = false;
    e.pages[0].appearanceObjectId = "cofre_cerrado";
    e.pages[0].conditions[0].negated = true;
    e.pages[0].trigger = EventTrigger::Touch;

    MapEventPage vacia = pagina("cofre_1_p2");
    vacia.trigger = EventTrigger::Autorun;
    MapEventCommand texto;
    texto.type = "texto.mostrar";
    texto.params["hablante"] = "Cofre";
    texto.params["texto"] = "Está vacío";
    MapEventCommand desconocido;              // un tipo que este motor no conoce
    desconocido.type = "clima.lluvia";
    desconocido.version = 7;
    desconocido.params["intensidad"] = "0.8";
    desconocido.children.push_back(texto);
    vacia.commands.push_back(texto);
    vacia.commands.push_back(desconocido);
    e.pages.push_back(vacia);

    MapDocument original = mapaCon(e);
    const Result<std::string> json = ProjectSerializer::serializeMap(original);
    MOTOR_CHECK(json.isOk());
    const Result<MapDocument> vuelta = ProjectSerializer::deserializeMap(json.value());
    MOTOR_CHECK(vuelta.isOk());

    const EventLayerDocument& capa =
        std::get<EventLayerDocument>(vuelta.value().findLayer(lyr("eventos"))->data());
    const MapEventDocument* leido = capa.find(evt("cofre_1"));
    MOTOR_CHECK(leido != nullptr);
    MOTOR_CHECK(leido->name == "Cofre");
    MOTOR_CHECK(leido->position.x == 3 && leido->position.y == 4);
    MOTOR_CHECK(leido->pages.size() == 2);

    // Pagina 1: disparador, AND/OR, apariencia y la condicion negada.
    const MapEventPage& p1 = leido->pages[0];
    MOTOR_CHECK(p1.trigger == EventTrigger::Touch);
    MOTOR_CHECK(p1.requiresAllConditions == false);
    MOTOR_CHECK(p1.appearanceObjectId == "cofre_cerrado");
    MOTOR_CHECK(p1.conditions.size() == 1);
    MOTOR_CHECK(p1.conditions[0].kind == EventConditionKind::Flag);
    MOTOR_CHECK(p1.conditions[0].reference == "cofre_abierto");
    MOTOR_CHECK(p1.conditions[0].negated == true);
    MOTOR_CHECK(p1.commands.size() == 1);
    MOTOR_CHECK(p1.commands[0].params.at("recompensa") == "llave_cueva");

    // Pagina 2: y el comando de tipo desconocido, con su version, su
    // parametro y su rama. Es el requisito del diseno de bloques §9.
    const MapEventPage& p2 = leido->pages[1];
    MOTOR_CHECK(p2.trigger == EventTrigger::Autorun);
    MOTOR_CHECK(p2.commands.size() == 2);
    MOTOR_CHECK(p2.commands[0].params.at("texto") == "Está vacío");
    MOTOR_CHECK(p2.commands[1].type == "clima.lluvia");
    MOTOR_CHECK(p2.commands[1].version == 7);
    MOTOR_CHECK(p2.commands[1].params.at("intensidad") == "0.8");
    MOTOR_CHECK(p2.commands[1].children.size() == 1);
    MOTOR_CHECK(p2.commands[1].children[0].type == "texto.mostrar");

    // Y el texto vuelve a salir igual: ida y vuelta estable, no solo
    // equivalente. Si esto falla, guardar dos veces da dos ficheros.
    const Result<std::string> segunda = ProjectSerializer::serializeMap(vuelta.value());
    MOTOR_CHECK(segunda.isOk());
    MOTOR_CHECK(segunda.value() == json.value());
}

MOTOR_TEST_CASE(un_disparador_que_no_se_entiende_falla_en_vez_de_caer_a_uno_por_defecto) {
    // Caer a "interact" convertiria una pagina que arranca sola en una que
    // espera al jugador, o al reves, sin un solo error a la vista.
    MapDocument doc = mapaCon(cofre("cofre_1", 1, 1));
    const Result<std::string> json = ProjectSerializer::serializeMap(doc);
    MOTOR_CHECK(json.isOk());

    std::string roto = json.value();
    const std::size_t donde = roto.find("\"interact\"");
    MOTOR_CHECK(donde != std::string::npos);
    roto.replace(donde, std::string("\"interact\"").size(), "\"cuando_llueve\"");

    const Result<MapDocument> vuelta = ProjectSerializer::deserializeMap(roto);
    MOTOR_CHECK(!vuelta.isOk());
    MOTOR_CHECK(vuelta.errorMessage().find("cuando_llueve") != std::string::npos);
}


}  // namespace

int main() { return motor_test::runAllTests(); }
