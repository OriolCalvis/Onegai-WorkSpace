// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-12  (fase E, 4 j) - Propietats de mapa, capes i instàncies
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "Editor/MapPropertiesPanel.h"
#include "Editor/ObjectInspector.h"

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

MOTOR_TEST_CASE(crear_capa) {
    RPG::MapPropertiesPanel panel;
    bool ok = panel.createLayer("suelo");
    require_true("crear capa ok", ok);
    require_eq("una capa", panel.layerCount(), 1);
}

MOTOR_TEST_CASE(renombrar_capa) {
    RPG::MapPropertiesPanel panel;
    panel.createLayer("suelo");
    bool ok = panel.renameLayer("suelo", "terreno");
    require_true("renombrar ok", ok);
    require_eq("una capa", panel.layerCount(), 1);
}

MOTOR_TEST_CASE(ocultar_capa) {
    RPG::MapPropertiesPanel panel;
    panel.createLayer("suelo");
    bool ok = panel.setLayerVisible("suelo", false);
    require_true("ocultar ok", ok);
}

MOTOR_TEST_CASE(bloquear_capa) {
    RPG::MapPropertiesPanel panel;
    panel.createLayer("suelo");
    bool ok = panel.setLayerLocked("suelo", true);
    require_true("bloquear ok", ok);
}

MOTOR_TEST_CASE(reordenar_capa) {
    RPG::MapPropertiesPanel panel;
    panel.createLayer("capa_a");
    panel.createLayer("capa_b");
    panel.createLayer("capa_c");
    bool ok = panel.reorderLayer("capa_c", 0);
    require_true("reordenar ok", ok);
}

MOTOR_TEST_CASE(crear_capa_duplicada) {
    RPG::MapPropertiesPanel panel;
    panel.createLayer("suelo");
    bool ok = panel.createLayer("suelo");
    require_true("duplicada falla", !ok);
    require_eq("una sola capa", panel.layerCount(), 1);
}

int main() { return motor_test::runAllTests(); }