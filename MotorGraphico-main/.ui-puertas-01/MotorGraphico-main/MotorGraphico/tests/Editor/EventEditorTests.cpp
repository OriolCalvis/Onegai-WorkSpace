// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-05  (fase E, 8 j) - Editor de pàgines d'esdeveniment
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Editor/EventEditorState.h"

#include "TestHarness.h"
#include "Core/Math/GridCoord.h"
#include "Map/EventLayerDocument.h"
#include "Map/MapDocument.h"

#include <string>
#include <vector>

namespace {

void require_eq(const std::string& what, int got, int expected) {
    if (got != expected) throw std::runtime_error(what + ": " + std::to_string(got) + " != " + std::to_string(expected));
}

void require_true(const std::string& what, bool ok) {
    if (!ok) throw std::runtime_error(what + ": expected true");
}

void require_eq_str(const std::string& what, const std::string& got, const std::string& expected) {
    if (got != expected) throw std::runtime_error(what + ": '" + got + "' != '" + expected + "'");
}

}  // namespace

MOTOR_TEST_CASE(crear_esdeveniment_sobre_casella) {
    MapDocument doc;
    RPG::EventEditorState editor(&doc);

    auto res = editor.createEvent(GridCoord{2, 1});
    require_true("crear_esdeveniment retorna ok", res.isOk());
    require_eq_str("id no buit", res.value(), "event_0");
    require_true("esdeveniment existeix", editor.getEvent(GridCoord{2, 1}) != nullptr);
}

MOTOR_TEST_CASE(afegir_pagina_a_esdeveniment) {
    MapDocument doc;
    RPG::EventEditorState editor(&doc);

    auto res = editor.createEvent(GridCoord{0, 0});
    std::string eventId = res.value();

    RPG::EventPage page;
    page.trigger = "enter";
    page.condition = "has_item(key)";
    page.commands = {"open_door"};

    auto pageRes = editor.addPage(eventId, page);
    require_true("afegir_pagina retorna ok", pageRes.isOk());
    require_eq("una pàgina", editor.pageCount(eventId), 1);
}

MOTOR_TEST_CASE(duplicar_pagina) {
    MapDocument doc;
    RPG::EventEditorState editor(&doc);

    auto res = editor.createEvent(GridCoord{1, 1});
    std::string eventId = res.value();

    RPG::EventPage page;
    page.trigger = "parlar";
    page.commands = {"say_hello"};

    auto addRes = editor.addPage(eventId, page);
    std::string pageId = addRes.value();

    auto dupRes = editor.duplicatePage(eventId, pageId);
    require_true("duplicar retorna ok", dupRes.isOk());
    require_eq("dues pàgines", editor.pageCount(eventId), 2);
}

MOTOR_TEST_CASE(esborrar_pagina) {
    MapDocument doc;
    RPG::EventEditorState editor(&doc);

    auto res = editor.createEvent(GridCoord{3, 2});
    std::string eventId = res.value();

    RPG::EventPage page;
    page.trigger = "enter";
    editor.addPage(eventId, page);
    editor.addPage(eventId, page);

    require_eq("abans esborrar", editor.pageCount(eventId), 2);
    bool ok = editor.removePage(eventId, "page_0");
    require_true("esborrar retorna true", ok);
    require_eq("després esborrar", editor.pageCount(eventId), 1);
}

MOTOR_TEST_CASE(reordenar_pagina) {
    MapDocument doc;
    RPG::EventEditorState editor(&doc);

    auto res = editor.createEvent(GridCoord{0, 0});
    std::string eventId = res.value();

    RPG::EventPage page;
    page.trigger = "enter";
    editor.addPage(eventId, page);
    editor.addPage(eventId, page);
    editor.addPage(eventId, page);

    require_eq("abans reordenar", editor.pageCount(eventId), 3);
    bool ok = editor.reorderPage(eventId, "page_2", 0);
    require_true("reordenar retorna true", ok);
}

MOTOR_TEST_CASE(desfer_creacio) {
    MapDocument doc;
    RPG::EventEditorState editor(&doc);

    auto res = editor.createEvent(GridCoord{1, 0});
    require_true("crear ok", res.isOk());
    require_true("esdeveniment existeix", editor.getEvent(GridCoord{1, 0}) != nullptr);

    bool ok = editor.undo();
    require_true("desfr funciona", ok);
    require_true("esdeveniment ja no existeix", editor.getEvent(GridCoord{1, 0}) == nullptr);
}

MOTOR_TEST_CASE(multiples_esdeveniments) {
    MapDocument doc;
    RPG::EventEditorState editor(&doc);

    auto r1 = editor.createEvent(GridCoord{0, 0});
    auto r2 = editor.createEvent(GridCoord{3, 2});
    require_true("dos esdeveniments ok", r1.isOk() && r2.isOk());
    require_eq_str("posicions diferents", r1.value(), "event_0");
    require_eq_str("segon id", r2.value(), "event_1");
    require_true("primer existeix", editor.getEvent(GridCoord{0, 0}) != nullptr);
    require_true("segon existeix", editor.getEvent(GridCoord{3, 2}) != nullptr);
}

// El harness es header-only: cada binari de tests acaba cridant
// runAllTests(). Sense aixo el fitxer compila i despres no enllaça, que
// es exactament el "_main" indefinit que donava aquest target.
int main() { return motor_test::runAllTests(); }
