// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-06  (fase E, 9 j) - Editor vertical de comandes
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "Editor/CommandListState.h"

#include "TestHarness.h"
#include "Core/Math/GridCoord.h"

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

MapDocument* fakeDoc = nullptr;

} // namespace

MOTOR_TEST_CASE(afegir_comanda) {
    RPG::CommandListState editor;

    auto res = editor.addCommand("say_hello");
    require_true("afegir retorna ok", res.isOk());
    require_eq("una comanda", editor.commandCount(), 1);
}

MOTOR_TEST_CASE(inserir_comanda) {
    RPG::CommandListState editor;

    auto r1 = editor.addCommand("say_hello");
    auto r2 = editor.insertCommand(0, "open_door");
    require_true("inserir retorna ok", r2.isOk());
    require_eq("dues comandes", editor.commandCount(), 2);
}

MOTOR_TEST_CASE(esborrar_comanda) {
    RPG::CommandListState editor;

    auto r1 = editor.addCommand("say_hello");
    auto r2 = editor.addCommand("open_door");
    require_eq("abans esborrar", editor.commandCount(), 2);

    bool ok = editor.removeCommand(r1.value());
    require_true("esborrar retorna true", ok);
    require_eq("després esborrar", editor.commandCount(), 1);
}

MOTOR_TEST_CASE(moure_comanda) {
    RPG::CommandListState editor;

    auto r1 = editor.addCommand("say_hello");
    auto r2 = editor.addCommand("open_door");
    auto r3 = editor.addCommand("close_door");

    bool ok = editor.moveCommand(r2.value(), 0);
    require_true("moure retorna true", ok);
}

MOTOR_TEST_CASE(obrir_tancar_branqa) {
    RPG::CommandListState editor;

    auto r1 = editor.addCommand("if_condition");
    require_true("afegir ok", r1.isOk());

    auto branch = editor.openBranch(r1.value());
    require_true("obrir branca retorna ok", branch.isOk());

    bool closed = editor.closeBranch(r1.value());
    require_true("tancar branca retorna true", closed);
}

MOTOR_TEST_CASE(multiples_comandes) {
    RPG::CommandListState editor;

    auto r1 = editor.addCommand("say_hello");
    auto r2 = editor.addCommand("open_door");
    auto r3 = editor.addCommand("close_door");
    require_eq("tres comandes", editor.commandCount(), 3);
    require_eq_str("primer id", r1.value(), "cmd_0");
    require_eq_str("segon id", r2.value(), "cmd_1");
    require_eq_str("tercer id", r3.value(), "cmd_2");
}

int main() { return motor_test::runAllTests(); }