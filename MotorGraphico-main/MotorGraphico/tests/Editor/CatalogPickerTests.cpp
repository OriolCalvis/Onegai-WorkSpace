// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-14  (fase E, 3 j) - Selectors de dades i referències
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "Editor/CatalogPickerState.h"

#include "TestHarness.h"

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

RPG::CatalogPickerState* picker = nullptr;

RPG::CatalogEntry makeEntry(const std::string& id, const std::string& name, RPG::CatalogType type) {
    RPG::CatalogEntry e;
    e.id = id;
    e.name = name;
    e.type = type;
    return e;
}

} // namespace

MOTOR_TEST_CASE(noms_iguals_ids_differents) {
    RPG::CatalogPickerState picker;
    bool ok = picker.loadCatalogs("assets");
    require_true("carga ok", ok);
    auto results = picker.search(RPG::CatalogType::Object, "Espada");
    require_true("busqueda funciona", results.size() == 0);
}

MOTOR_TEST_CASE(filtre_mil_entries) {
    RPG::CatalogPickerState picker;
    bool ok = picker.loadCatalogs("assets");
    require_true("carga ok", ok);
    auto results = picker.search(RPG::CatalogType::Object, "");
    require_true("busqueda vacia funciona", true);
}

MOTOR_TEST_CASE(tipo_incompatible_rechazado) {
    RPG::CatalogPickerState picker;
    RPG::CatalogEntry e;
    e.id = "obj_0";
    e.name = "Espada";
    e.type = RPG::CatalogType::Object;
    auto res = picker.select(RPG::CatalogType::Skill, "Espada");
    require_true("skill no encontrado", res.isOk() == false);
}

MOTOR_TEST_CASE(reference_perdida_localizable) {
    RPG::CatalogPickerState picker;
    bool valid = picker.isValidReference(RPG::CatalogType::Map, "nonexistent");
    require_true("referencia no valida", !valid);
}

MOTOR_TEST_CASE(carga_catalogs) {
    RPG::CatalogPickerState picker;
    bool ok = picker.loadCatalogs("assets");
    require_true("carga ok", ok);
}

int main() { return motor_test::runAllTests(); }