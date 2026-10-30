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

// ---- U3-02.E06a (D1 = b): la pagina dice a que region escucha ----

MapDocument mapaConRegion(const MapEventPage& pag) {
    MapDocument doc(ids::MapId::parse("mapa").value(), "mapa", 8, 8, 64, 32);
    MapLayerMeta rm;
    rm.id = lyr("regiones");
    rm.name = "regiones";
    rm.order = 0;
    MapRegionDocument zona;
    zona.id = ids::RegionId::parse("zona_norte").value();
    zona.name = "Zona";
    zona.bounds = GridBounds{0, 3, 0, 3};
    doc.addLayer(MapLayer::regions(rm, RegionLayerDocument({zona})));
    MapLayerMeta em;
    em.id = lyr("eventos");
    em.name = "eventos";
    em.order = 1;
    doc.addLayer(MapLayer::events(em, EventLayerDocument()));
    MapEventDocument e;
    e.id = evt("vigia");
    e.name = "Vigia";
    e.position = GridCoord{5, 5};
    e.pages.push_back(pag);
    doc.addEvent(lyr("eventos"), e);
    return doc;
}

MapEventPage paginaDeRegion(EventTrigger trigger, const std::string& capa,
                            const std::string& region) {
    MapEventPage p = pagina("vigia_p1");
    p.trigger = trigger;
    if (!capa.empty()) {
        p.regionLayerId = lyr(capa);
    }
    if (!region.empty()) {
        p.regionId = ids::RegionId::parse(region).value();
    }
    return p;
}

MOTOR_TEST_CASE(los_tres_triggers_de_region_van_y_vuelven_con_su_region) {
    const EventTrigger todos[] = {EventTrigger::RegionEnter, EventTrigger::RegionStay,
                                  EventTrigger::RegionExit};
    const char* textos[] = {"region_enter", "region_stay", "region_exit"};
    for (int i = 0; i < 3; ++i) {
        MapDocument doc = mapaConRegion(paginaDeRegion(todos[i], "regiones", "zona_norte"));
        MOTOR_CHECK(doc.validate().isOk());
        const Result<std::string> json = ProjectSerializer::serializeMap(doc);
        MOTOR_CHECK(json.isOk());
        MOTOR_CHECK(json.value().find(std::string("\"") + textos[i] + "\"") != std::string::npos);
        const Result<MapDocument> vuelta = ProjectSerializer::deserializeMap(json.value());
        MOTOR_CHECK(vuelta.isOk());
        const auto& capa =
            std::get<EventLayerDocument>(vuelta.value().findLayer(lyr("eventos"))->data());
        const MapEventPage& leida = capa.find(evt("vigia"))->pages[0];
        MOTOR_CHECK(leida.trigger == todos[i]);
        MOTOR_CHECK(leida.regionLayerId.value() == "regiones");
        MOTOR_CHECK(leida.regionId.value() == "zona_norte");
        const Result<std::string> segunda = ProjectSerializer::serializeMap(vuelta.value());
        MOTOR_CHECK(segunda.isOk() && segunda.value() == json.value());
    }
}

MOTOR_TEST_CASE(un_trigger_de_region_sin_referencia_se_rechaza_en_todas_las_vias) {
    // Documento de eventos: capa o region vacias.
    EventLayerDocument capa;
    MapEventDocument e;
    e.id = evt("vigia");
    e.position = GridCoord{1, 1};
    e.pages.push_back(paginaDeRegion(EventTrigger::RegionEnter, "", ""));
    MOTOR_CHECK(!capa.addEvent(e).isOk());
    e.pages[0] = paginaDeRegion(EventTrigger::RegionStay, "regiones", "");
    MOTOR_CHECK(!capa.addEvent(e).isOk());
    e.pages[0] = paginaDeRegion(EventTrigger::RegionExit, "", "zona_norte");
    MOTOR_CHECK(!capa.addEvent(e).isOk());
    // Y un trigger que NO es de region no admite referencia.
    e.pages[0] = paginaDeRegion(EventTrigger::Touch, "regiones", "zona_norte");
    MOTOR_CHECK(!capa.addEvent(e).isOk());
    MOTOR_CHECK(capa.events().empty());

    // JSON escrito a mano: region_enter sin la clave "region" no carga.
    MapDocument ok =
        mapaConRegion(paginaDeRegion(EventTrigger::RegionEnter, "regiones", "zona_norte"));
    std::string json = ProjectSerializer::serializeMap(ok).value();
    const std::size_t a = json.find("\"region\": {");
    MOTOR_CHECK(a != std::string::npos);
    const std::size_t b = json.find('}', a);
    json.erase(a, b - a + 1);
    MOTOR_CHECK(!ProjectSerializer::deserializeMap(json).isOk());
}

MOTOR_TEST_CASE(el_mapa_rechaza_un_trigger_que_escucha_una_region_inexistente) {
    // Desde r1 las ediciones tambien lo rechazan: addEvent con region inexistente o en una
    // capa que no es de regiones no entra en el documento.
    const auto sinEvento = [](const MapDocument& d) {
        return std::get<EventLayerDocument>(d.findLayer(lyr("eventos"))->data()).events().empty();
    };
    MOTOR_CHECK(sinEvento(
        mapaConRegion(paginaDeRegion(EventTrigger::RegionEnter, "regiones", "no_existe"))));
    MOTOR_CHECK(sinEvento(
        mapaConRegion(paginaDeRegion(EventTrigger::RegionEnter, "eventos", "zona_norte"))));
    // Un JSON roto a mano (el unico camino que queda) lo rechaza el deserializador.
    std::string json = ProjectSerializer::serializeMap(
        mapaConRegion(paginaDeRegion(EventTrigger::RegionEnter, "regiones", "zona_norte")))
                           .value();
    const std::size_t ref = json.find("\"region\": {");
    MOTOR_CHECK(ref != std::string::npos);
    const std::size_t nombre = json.find("zona_norte", ref);
    MOTOR_CHECK(nombre != std::string::npos);
    json.replace(nombre, std::string("zona_norte").size(), "no_existe");
    const Result<MapDocument> vuelta = ProjectSerializer::deserializeMap(json);
    MOTOR_CHECK(!vuelta.isOk());
    MOTOR_CHECK(vuelta.errorMessage().find("no_existe") != std::string::npos);
}

MOTOR_TEST_CASE(un_json_antiguo_sin_clave_region_sigue_cargando_igual) {
    // Compatibilidad hacia atras: ningun mapa sin trigger de region escribe "region".
    MapDocument doc = mapaCon(cofre("cofre_1", 1, 1));
    const Result<std::string> json = ProjectSerializer::serializeMap(doc);
    MOTOR_CHECK(json.isOk());
    MOTOR_CHECK(json.value().find("\"region\"") == std::string::npos);
    const Result<MapDocument> vuelta = ProjectSerializer::deserializeMap(json.value());
    MOTOR_CHECK(vuelta.isOk());
    const auto& capa =
        std::get<EventLayerDocument>(vuelta.value().findLayer(lyr("eventos"))->data());
    const MapEventPage& p = capa.find(evt("cofre_1"))->pages[0];
    MOTOR_CHECK(p.regionLayerId.empty() && p.regionId.empty());
}

MapDocument mapaConDosRegiones() {
    MapDocument doc =
        mapaConRegion(paginaDeRegion(EventTrigger::RegionEnter, "regiones", "zona_norte"));
    MapRegionDocument libre;
    libre.id = ids::RegionId::parse("zona_libre").value();
    libre.name = "Libre";
    libre.bounds = GridBounds{4, 5, 4, 5};
    doc.addRegion(lyr("regiones"), libre);
    return doc;
}

// Rechazado, con el mapa intacto (valido y serializa igual) y el mensaje nombrando la pagina.
bool rechazadaSinTocar(const MapDocument& doc, const std::string& antes, const Result<bool>& r) {
    return !r.isOk() && r.errorMessage().find("vigia_p1") != std::string::npos &&
           doc.validate().isOk() && ProjectSerializer::serializeMap(doc).value() == antes;
}

MOTOR_TEST_CASE(las_ediciones_no_dejan_una_pagina_escuchando_una_region_que_ya_no_existe) {
    MapDocument doc = mapaConDosRegiones();
    const std::string antes = ProjectSerializer::serializeMap(doc).value();

    MOTOR_CHECK(rechazadaSinTocar(
        doc, antes, doc.removeRegion(lyr("regiones"), ids::RegionId::parse("zona_norte").value())));
    MOTOR_CHECK(rechazadaSinTocar(doc, antes, doc.removeLayer(lyr("regiones"))));

    MapLayerMeta renombrada = doc.findLayer(lyr("regiones"))->meta();
    renombrada.id = lyr("otras");
    MOTOR_CHECK(rechazadaSinTocar(doc, antes, doc.setLayerMeta(lyr("regiones"), renombrada)));

    std::vector<MapEventPage> paginas = {
        paginaDeRegion(EventTrigger::RegionStay, "regiones", "no_existe")};
    MOTOR_CHECK(rechazadaSinTocar(
        doc, antes, doc.setEventPages(lyr("eventos"), evt("vigia"), paginas)));

    // Positivo: lo que nadie escucha se puede borrar, y cambiar el orden de capa tambien.
    MOTOR_CHECK(
        doc.removeRegion(lyr("regiones"), ids::RegionId::parse("zona_libre").value()).isOk());
    MapLayerMeta reordenada = doc.findLayer(lyr("regiones"))->meta();
    reordenada.order = 5;
    MOTOR_CHECK(doc.setLayerMeta(lyr("regiones"), reordenada).isOk());
    MOTOR_CHECK(doc.validate().isOk());
    // Y repuntar la pagina a una region que si existe funciona.
    paginas = {paginaDeRegion(EventTrigger::RegionStay, "regiones", "zona_norte")};
    paginas[0].id = evt("vigia_p1");
    MOTOR_CHECK(doc.setEventPages(lyr("eventos"), evt("vigia"), paginas).isOk());
}

MOTOR_TEST_CASE(una_pagina_normal_con_region_vacia_da_el_mensaje_de_solo_region) {
    MapDocument doc = mapaCon(cofre("cofre_1", 1, 1));
    std::string json = ProjectSerializer::serializeMap(doc).value();
    const std::size_t t = json.find("\"trigger\": \"interact\",");
    MOTOR_CHECK(t != std::string::npos);
    json.insert(t, "\"region\": {}, ");
    const Result<MapDocument> r = ProjectSerializer::deserializeMap(json);
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(r.errorMessage().find("solo un trigger de region") != std::string::npos);
}

MapLayerMeta metaCapa(const std::string& id, int order) {
    MapLayerMeta m;
    m.id = lyr(id);
    m.name = id;
    m.order = order;
    return m;
}

MapEventDocument vigiaQueEscucha(const std::string& capa, const std::string& region) {
    MapEventDocument e;
    e.id = evt("vigia");
    e.position = GridCoord{5, 5};
    e.pages.push_back(paginaDeRegion(EventTrigger::RegionEnter, capa, region));
    return e;
}

MOTOR_TEST_CASE(addLayer_comprueba_las_regiones_que_escuchan_las_paginas_y_es_atomico) {
    MapDocument doc(ids::MapId::parse("mapa").value(), "mapa", 8, 8, 64, 32);
    MapRegionDocument zona;
    zona.id = ids::RegionId::parse("zona_norte").value();
    zona.name = "Zona";
    zona.bounds = GridBounds{0, 3, 0, 3};
    MOTOR_CHECK(
        doc.addLayer(MapLayer::regions(metaCapa("regiones", 0), RegionLayerDocument({zona}))).isOk());
    MOTOR_CHECK(doc.addLayer(MapLayer::tiles(metaCapa("suelo", 1),
                                             TileLayerDocument(8, 8, std::vector<int>(64, 1))))
                    .isOk());
    const std::size_t antes = doc.layers().size();
    const std::string json = ProjectSerializer::serializeMap(doc).value();

    // Region inexistente en una capa que si es de regiones.
    MOTOR_CHECK(!doc.addLayer(MapLayer::events(metaCapa("eventos", 2),
        EventLayerDocument({vigiaQueEscucha("regiones", "no_existe")}))).isOk());
    // Capa inexistente.
    MOTOR_CHECK(!doc.addLayer(MapLayer::events(metaCapa("eventos", 2),
        EventLayerDocument({vigiaQueEscucha("ausente", "zona_norte")}))).isOk());
    // Capa que existe pero es de tiles.
    const Result<bool> tipo = doc.addLayer(MapLayer::events(metaCapa("eventos", 2),
        EventLayerDocument({vigiaQueEscucha("suelo", "zona_norte")})));
    MOTOR_CHECK(!tipo.isOk() && tipo.errorMessage().find("vigia_p1") != std::string::npos);
    // Atomico: ni capas nuevas, ni cambios, y el documento sigue valido.
    MOTOR_CHECK(doc.layers().size() == antes);
    MOTOR_CHECK(doc.validate().isOk() && ProjectSerializer::serializeMap(doc).value() == json);
    // Referencia valida: entra.
    MOTOR_CHECK(doc.addLayer(MapLayer::events(metaCapa("eventos", 2),
        EventLayerDocument({vigiaQueEscucha("regiones", "zona_norte")}))).isOk());
    MOTOR_CHECK(doc.layers().size() == antes + 1 && doc.validate().isOk());
    // Editar tiles no cambia de comportamiento (no barre referencias).
    MOTOR_CHECK(doc.setTile(lyr("suelo"), GridCoord{1, 1}, 2).isOk());
}

MOTOR_TEST_CASE(un_mapa_con_la_capa_de_eventos_antes_que_la_de_regiones_carga_bien) {
    MapDocument doc(ids::MapId::parse("mapa").value(), "mapa", 8, 8, 64, 32);
    MapRegionDocument zona;
    zona.id = ids::RegionId::parse("zona_norte").value();
    zona.name = "Zona";
    zona.bounds = GridBounds{0, 3, 0, 3};
    MOTOR_CHECK(
        doc.addLayer(MapLayer::regions(metaCapa("regiones", 5), RegionLayerDocument({zona}))).isOk());
    MOTOR_CHECK(doc.addLayer(MapLayer::events(metaCapa("eventos", 0),
        EventLayerDocument({vigiaQueEscucha("regiones", "zona_norte")}))).isOk());
    MOTOR_CHECK(doc.layers()[0].meta().id == lyr("eventos"));  // eventos primero en el fichero
    const Result<std::string> json = ProjectSerializer::serializeMap(doc);
    MOTOR_CHECK(json.isOk());
    MOTOR_CHECK(json.value().find("\"eventos\"") < json.value().find("\"regiones\""));
    const Result<MapDocument> vuelta = ProjectSerializer::deserializeMap(json.value());
    MOTOR_CHECK(vuelta.isOk());
    MOTOR_CHECK(vuelta.isOk() &&
                ProjectSerializer::serializeMap(vuelta.value()).value() == json.value());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
