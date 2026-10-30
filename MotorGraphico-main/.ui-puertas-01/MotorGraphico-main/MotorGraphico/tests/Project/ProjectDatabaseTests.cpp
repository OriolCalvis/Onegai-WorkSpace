// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-16  (fase E, 8 j) - Edició RPG mínima del proyecto
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "Editor/ProjectDatabaseState.h"

#include "TestHarness.h"

#include <string>

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

} // namespace

MOTOR_TEST_CASE(crear_entrada) {
    RPG::ProjectDatabaseState db;
    auto res = db.createEntry("habilidad");
    require_true("crear ok", res.isOk());
    require_eq("una entrada", db.count("habilidad"), 1);
}

MOTOR_TEST_CASE(duplicar_entrada) {
    RPG::ProjectDatabaseState db;
    auto r1 = db.createEntry("enemigo");
    require_true("crear ok", r1.isOk());
    auto r2 = db.duplicateEntry(r1.value());
    require_true("duplicar ok", r2.isOk());
    require_eq("dos entradas", db.count("enemigo"), 2);
}

MOTOR_TEST_CASE(validar_entrada) {
    RPG::ProjectDatabaseState db;
    auto r1 = db.createEntry("objeto");
    require_true("crear ok", r1.isOk());
    bool valid = db.validateEntry(r1.value());
    require_true("validar retorna true", valid);
}

MOTOR_TEST_CASE(actualizar_campo) {
    RPG::ProjectDatabaseState db;
    auto r1 = db.createEntry("objeto");
    require_true("crear ok", r1.isOk());
    bool ok = db.updateField(r1.value(), "nombre", "Espada");
    require_true("actualizar ok", ok);
}

MOTOR_TEST_CASE(verificar_id) {
    RPG::ProjectDatabaseState db;
    auto r1 = db.createEntry("habilidad");
    require_true("crear ok", r1.isOk());
    bool exists = db.hasId(r1.value());
    require_true("id existe", exists);
    bool notExists = db.hasId("no_existe");
    require_true("id no existe", !notExists);
}

MOTOR_TEST_CASE(entrada_por_id) {
    RPG::ProjectDatabaseState db;
    auto r1 = db.createEntry("enemigo");
    require_true("crear ok", r1.isOk());
    const RPG::ProjectDatabaseEntry* entry = db.getEntry(r1.value());
    require_true("entrada existe", entry != nullptr);
    require_eq_str("tipo correcto", entry->type, "enemigo");
}

int main() { return motor_test::runAllTests(); }