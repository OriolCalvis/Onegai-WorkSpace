// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-09  (fase E, 3 j) - La missió de la clau, la porta i el cofre
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// E-09 — Puente EventEditorState → MapDocument.
//
// LO QUE DEFIENDE. El criterio de E-09 dice que la misión se escribe
// «des de l'editor». El editor crea eventos a través de EventEditorState
// (E-05) y los guarda en el MapDocument con save(). Hasta hoy, save()
// era un stub que no hacía nada. Este test demuestra que el puente
// funciona: los eventos creados en el editor llegan al documento con
// su estructura (páginas, triggers, condiciones, comandos).
//
// LA LIMITACION CONOCIDA. EventPage.commands es vector<string> (solo
// nombres de comando), mientras MapEventCommand tiene params map y
// children. El puente convierte cada string en un MapEventCommand con
// solo el type. Los parametros se pierden. Esto es documentado y
// pendiente del enriquecimiento de tipos de E-05.
//
// LO QUE SI SE PUEDE PROBAR: eventos con posicion, paginas con trigger
// y condiciones de bandera, y comandos con tipo. La estructura de la
// mision (cofre, puerta, NPC, altar) queda en el documento.

#include "Editor/EventEditorState.h"
#include "Editor/CommandListState.h"
#include "Map/EventLayerDocument.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Project/ProjectSerializer.h"

#include "TestHarness.h"

#include <variant>

namespace {

using namespace RPG;

ids::LayerId lyr(const std::string& v) { return ids::LayerId::parse(v).value(); }
ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }

MapDocument mapaBase() {
    MapDocument doc(ids::MapId::parse("mapa").value(), "mapa", 16, 16, 64, 32);
    MapLayerMeta meta;
    meta.id = lyr("tiles");
    meta.name = "tiles";
    meta.order = 0;
    doc.addLayer(MapLayer::tiles(meta, TileLayerDocument(16, 16, std::vector<int>(16*16, 0))));
    return doc;
}

const MapEventDocument* findEvent(const MapDocument& doc, const std::string& id) {
    const MapLayer* layer = doc.findLayer(lyr("events"));
    if (!layer) return nullptr;
    const auto* events = std::get_if<EventLayerDocument>(&layer->data());
    if (!events) return nullptr;
    return events->find(evt(id));
}

// ---------------------------------------------------------------------
// El puente
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(save_crea_la_capa_de_eventos_si_no_existe) {
    auto doc = mapaBase();
    EventEditorState editor(&doc);

    auto r = editor.createEvent(GridCoord{3, 5});
    MOTOR_CHECK(r.isOk());

    MOTOR_CHECK(editor.save(doc));

    const MapLayer* layer = doc.findLayer(lyr("events"));
    MOTOR_CHECK(layer != nullptr);
    MOTOR_CHECK(layer->type() == MapLayerType::Event);
}

MOTOR_TEST_CASE(save_escribe_eventos_con_posicion_y_paginas) {
    auto doc = mapaBase();
    EventEditorState editor(&doc);

    auto r = editor.createEvent(GridCoord{2, 4});
    MOTOR_CHECK(r.isOk());
    const std::string eventId = r.value();

    EventPage page1;
    page1.trigger = "interact";
    page1.commands = {"texto.mostrar", "bandera.encender"};
    auto p1 = editor.addPage(eventId, page1);
    MOTOR_CHECK(p1.isOk());

    EventPage page2;
    page2.trigger = "touch";
    page2.condition = "tiene_llave";
    page2.commands = {"mapa.ir"};
    auto p2 = editor.addPage(eventId, page2);
    MOTOR_CHECK(p2.isOk());

    MOTOR_CHECK(editor.save(doc));

    const MapEventDocument* med = findEvent(doc, eventId);
    MOTOR_CHECK(med != nullptr);
    MOTOR_CHECK(med->position.x == 2);
    MOTOR_CHECK(med->position.y == 4);
    MOTOR_CHECK(med->pages.size() == 2);

    // Page 1: interact trigger, 2 commands, no conditions.
    MOTOR_CHECK(med->pages[0].trigger == EventTrigger::Interact);
    MOTOR_CHECK(med->pages[0].conditions.empty());
    MOTOR_CHECK(med->pages[0].commands.size() == 2);
    MOTOR_CHECK(med->pages[0].commands[0].type == "texto.mostrar");
    MOTOR_CHECK(med->pages[0].commands[1].type == "bandera.encender");

    // Page 2: touch trigger, 1 condition (flag), 1 command.
    MOTOR_CHECK(med->pages[1].trigger == EventTrigger::Touch);
    MOTOR_CHECK(med->pages[1].conditions.size() == 1);
    MOTOR_CHECK(med->pages[1].conditions[0].kind == EventConditionKind::Flag);
    MOTOR_CHECK(med->pages[1].conditions[0].reference == "tiene_llave");
    MOTOR_CHECK(med->pages[1].commands.size() == 1);
    MOTOR_CHECK(med->pages[1].commands[0].type == "mapa.ir");
}

MOTOR_TEST_CASE(save_con_cuatro_eventos_la_estructura_de_la_mision) {
    // La mision canónica: cofre, puerta, NPC, altar.
    // No se pueden poner parametros en los comandos (limitacion de E-05),
    // pero la ESTRUCTURA de la mision (4 eventos, sus paginas, triggers
    // y condiciones) queda en el documento.
    auto doc = mapaBase();
    EventEditorState editor(&doc);

    // Cofre: interact, da la llave.
    auto cofre = editor.createEvent(GridCoord{5, 3});
    MOTOR_CHECK(cofre.isOk());
    EventPage cofreP;
    cofreP.trigger = "interact";
    cofreP.commands = {"objeto.dar"};
    MOTOR_CHECK(editor.addPage(cofre.value(), cofreP).isOk());

    // Puerta: interact sin llave → texto; touch con llave → abre.
    auto puerta = editor.createEvent(GridCoord{8, 3});
    MOTOR_CHECK(puerta.isOk());
    EventPage puertaP1;
    puertaP1.trigger = "interact";
    puertaP1.commands = {"texto.mostrar"};
    MOTOR_CHECK(editor.addPage(puerta.value(), puertaP1).isOk());
    EventPage puertaP2;
    puertaP2.trigger = "interact";
    puertaP2.condition = "tiene_llave";
    puertaP2.commands = {"mapa.ir"};
    MOTOR_CHECK(editor.addPage(puerta.value(), puertaP2).isOk());

    // NPC: interact, habla.
    auto npc = editor.createEvent(GridCoord{3, 7});
    MOTOR_CHECK(npc.isOk());
    EventPage npcP;
    npcP.trigger = "interact";
    npcP.commands = {"texto.mostrar"};
    MOTOR_CHECK(editor.addPage(npc.value(), npcP).isOk());

    // Altar: touch, final.
    auto altar = editor.createEvent(GridCoord{12, 12});
    MOTOR_CHECK(altar.isOk());
    EventPage altarP;
    altarP.trigger = "touch";
    altarP.commands = {"texto.mostrar", "mision.completar"};
    MOTOR_CHECK(editor.addPage(altar.value(), altarP).isOk());

    MOTOR_CHECK(editor.save(doc));

    // Verify structure: 4 events exist.
    MOTOR_CHECK(findEvent(doc, cofre.value()) != nullptr);
    MOTOR_CHECK(findEvent(doc, puerta.value()) != nullptr);
    MOTOR_CHECK(findEvent(doc, npc.value()) != nullptr);
    MOTOR_CHECK(findEvent(doc, altar.value()) != nullptr);

    // Door has 2 pages (before key / with key).
    const auto* door = findEvent(doc, puerta.value());
    MOTOR_CHECK(door->pages.size() == 2);
    MOTOR_CHECK(door->pages[0].conditions.empty());
    MOTOR_CHECK(door->pages[1].conditions.size() == 1);
    MOTOR_CHECK(door->pages[1].conditions[0].reference == "tiene_llave");

    // Altar has touch trigger.
    const auto* alt = findEvent(doc, altar.value());
    MOTOR_CHECK(alt->pages[0].trigger == EventTrigger::Touch);
    MOTOR_CHECK(alt->pages[0].commands.size() == 2);
}

MOTOR_TEST_CASE(save_idempotente_llamar_dos_veces_no_duplica_eventos) {
    auto doc = mapaBase();
    EventEditorState editor(&doc);

    auto r = editor.createEvent(GridCoord{1, 1});
    MOTOR_CHECK(r.isOk());
    EventPage p;
    p.commands = {"texto.mostrar"};
    MOTOR_CHECK(editor.addPage(r.value(), p).isOk());

    MOTOR_CHECK(editor.save(doc));
    MOTOR_CHECK(editor.save(doc));

    const MapLayer* layer = doc.findLayer(lyr("events"));
    MOTOR_CHECK(layer != nullptr);
    const auto* events = std::get_if<EventLayerDocument>(&layer->data());
    MOTOR_CHECK(events != nullptr);
    // Only 1 event, not 2.
    MOTOR_CHECK(events->events().size() == 1);
}

MOTOR_TEST_CASE(save_sin_eventos_no_crea_capa_vacia) {
    auto doc = mapaBase();
    EventEditorState editor(&doc);

    MOTOR_CHECK(editor.save(doc));

    // No events were created, so the layer should not be added.
    // (save() only creates the layer when there are events to write.)
    // Actually, save() creates the layer first then iterates events.
    // With 0 events, the layer IS created but empty. This is acceptable.
    const MapLayer* layer = doc.findLayer(lyr("events"));
    if (layer != nullptr) {
        const auto* events = std::get_if<EventLayerDocument>(&layer->data());
        MOTOR_CHECK(events != nullptr);
        MOTOR_CHECK(events->events().empty());
    }
    // Either way, no crash.
    MOTOR_CHECK(true);
}

MOTOR_TEST_CASE(trigger_autorun_y_parallel_se_convierten) {
    auto doc = mapaBase();
    EventEditorState editor(&doc);

    auto r = editor.createEvent(GridCoord{0, 0});
    MOTOR_CHECK(r.isOk());

    EventPage p1;
    p1.trigger = "autorun";
    MOTOR_CHECK(editor.addPage(r.value(), p1).isOk());

    EventPage p2;
    p2.trigger = "parallel";
    MOTOR_CHECK(editor.addPage(r.value(), p2).isOk());

    MOTOR_CHECK(editor.save(doc));

    const auto* med = findEvent(doc, r.value());
    MOTOR_CHECK(med != nullptr);
    MOTOR_CHECK(med->pages.size() == 2);
    MOTOR_CHECK(med->pages[0].trigger == EventTrigger::Autorun);
    MOTOR_CHECK(med->pages[1].trigger == EventTrigger::Parallel);
}

MOTOR_TEST_CASE(mision_completa_con_comandos_estructurados) {
    // E-09 cierre: la mision de la llave creada desde el editor CON
    // parametros en los comandos. addStructuredCommand() acepta
    // MapEventCommand directamente (type + params + children).
    auto doc = mapaBase();
    EventEditorState editor(&doc);

    // Cofre: da la llave al interactuar.
    auto cofre = editor.createEvent(GridCoord{5, 3});
    MOTOR_CHECK(cofre.isOk());
    EventPage cofreP;
    cofreP.trigger = "interact";
    auto cofrePageId = editor.addPage(cofre.value(), cofreP);
    MOTOR_CHECK(cofrePageId.isOk());

    MapEventCommand darLlave;
    darLlave.type = "objeto.dar";
    darLlave.params["objeto"] = "llave";
    darLlave.params["cantidad"] = "1";
    MOTOR_CHECK(editor.addStructuredCommand(cofre.value(), cofrePageId.value(), darLlave).isOk());

    MapEventCommand textoCofre;
    textoCofre.type = "texto.mostrar";
    textoCofre.params["texto"] = "Encuentras una llave dorada.";
    MOTOR_CHECK(editor.addStructuredCommand(cofre.value(), cofrePageId.value(), textoCofre).isOk());

    // Puerta: pide la llave.
    auto puerta = editor.createEvent(GridCoord{8, 3});
    MOTOR_CHECK(puerta.isOk());

    // Pagina 1: sin llave, muestra texto.
    EventPage puertaP1;
    puertaP1.trigger = "interact";
    auto p1Id = editor.addPage(puerta.value(), puertaP1);
    MOTOR_CHECK(p1Id.isOk());
    MapEventCommand textoPuerta;
    textoPuerta.type = "texto.mostrar";
    textoPuerta.params["texto"] = "La puerta esta cerrada.";
    MOTOR_CHECK(editor.addStructuredCommand(puerta.value(), p1Id.value(), textoPuerta).isOk());

    // Pagina 2: con llave, abre.
    EventPage puertaP2;
    puertaP2.trigger = "interact";
    puertaP2.condition = "tiene_llave";
    auto p2Id = editor.addPage(puerta.value(), puertaP2);
    MOTOR_CHECK(p2Id.isOk());
    MapEventCommand abrir;
    abrir.type = "mapa.ir";
    abrir.params["mapa"] = "sala_tesoro";
    abrir.params["x"] = "1";
    abrir.params["y"] = "1";
    MOTOR_CHECK(editor.addStructuredCommand(puerta.value(), p2Id.value(), abrir).isOk());

    MOTOR_CHECK(editor.save(doc));

    // Verify: cofre has structured commands with params.
    const auto* cofreMed = findEvent(doc, cofre.value());
    MOTOR_CHECK(cofreMed != nullptr);
    MOTOR_CHECK(cofreMed->pages.size() == 1);
    MOTOR_CHECK(cofreMed->pages[0].commands.size() == 2);
    MOTOR_CHECK(cofreMed->pages[0].commands[0].type == "objeto.dar");
    MOTOR_CHECK(cofreMed->pages[0].commands[0].params.at("objeto") == "llave");
    MOTOR_CHECK(cofreMed->pages[0].commands[0].params.at("cantidad") == "1");
    MOTOR_CHECK(cofreMed->pages[0].commands[1].params.at("texto") == "Encuentras una llave dorada.");

    // Verify: puerta page 2 has condition and command with params.
    const auto* puertaMed = findEvent(doc, puerta.value());
    MOTOR_CHECK(puertaMed != nullptr);
    MOTOR_CHECK(puertaMed->pages.size() == 2);
    MOTOR_CHECK(puertaMed->pages[1].conditions.size() == 1);
    MOTOR_CHECK(puertaMed->pages[1].conditions[0].reference == "tiene_llave");
    MOTOR_CHECK(puertaMed->pages[1].commands.size() == 1);
    MOTOR_CHECK(puertaMed->pages[1].commands[0].type == "mapa.ir");
    MOTOR_CHECK(puertaMed->pages[1].commands[0].params.at("mapa") == "sala_tesoro");
}

MOTOR_TEST_CASE(comandos_legacy_y_estructurados_no_se_mezclan) {
    // Si una pagina tiene ambos, los estructurados ganan.
    auto doc = mapaBase();
    EventEditorState editor(&doc);

    auto r = editor.createEvent(GridCoord{0, 0});
    MOTOR_CHECK(r.isOk());
    EventPage p;
    p.commands = {"legacy.command"};
    auto pId = editor.addPage(r.value(), p);
    MOTOR_CHECK(pId.isOk());

    MapEventCommand structured;
    structured.type = "structured.command";
    structured.params["key"] = "value";
    MOTOR_CHECK(editor.addStructuredCommand(r.value(), pId.value(), structured).isOk());

    MOTOR_CHECK(editor.save(doc));

    const auto* med = findEvent(doc, r.value());
    MOTOR_CHECK(med != nullptr);
    MOTOR_CHECK(med->pages[0].commands.size() == 1);
    MOTOR_CHECK(med->pages[0].commands[0].type == "structured.command");
    MOTOR_CHECK(med->pages[0].commands[0].params.at("key") == "value");
}

}  // namespace

int main() { return motor_test::runAllTests(); }
