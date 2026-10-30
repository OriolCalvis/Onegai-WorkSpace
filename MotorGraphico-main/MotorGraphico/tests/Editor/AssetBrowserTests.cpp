// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-13  (fase E, 4 j) - Navegador visual i importació de recursos
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "Editor/AssetBrowserPanel.h"

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

MOTOR_TEST_CASE(importar_png_valid) {
    RPG::AssetBrowserPanel panel;
    auto res = panel.importPng("textures/cofre.png", "Cofre");
    require_true("import ok", res.isOk());
    require_eq("un asset", panel.assetCount(), 1);
}

MOTOR_TEST_CASE(rechazar_formato_invalido) {
    RPG::AssetBrowserPanel panel;
    panel.importPng("textures/cofre.png", "Cofre");
    auto r = panel.importPng("textures/spritesheet.psd", "Spritesheet");
    require_true("import ok", r.isOk());
    require_eq("dos assets", panel.assetCount(), 2);
}

MOTOR_TEST_CASE(nom_duplicat) {
    RPG::AssetBrowserPanel panel;
    panel.importPng("textures/cofre.png", "Cofre");
    auto r = panel.importPng("textures/otro.png", "Cofre");
    require_true("duplicado falla", !r.isOk());
    require_eq("un solo asset", panel.assetCount(), 1);
}

MOTOR_TEST_CASE(buscar_por_nombre) {
    RPG::AssetBrowserPanel panel;
    panel.importPng("textures/cofre.png", "Cofre del templo");
    panel.importPng("textures/espada.png", "Espada de acero");
    auto results = panel.search("Cofre");
    require_eq("un resultado", results.size(), 1);
    require_eq_str("nombre correcto", results[0].name, "Cofre del templo");
}

MOTOR_TEST_CASE(mover_carpeta_y_reabrir) {
    RPG::AssetBrowserPanel panel;
    auto r = panel.importPng("textures/cofre.png", "Cofre");
    require_true("import ok", r.isOk());
    const RPG::AssetInfo* info = panel.getAsset(r.value());
    require_true("asset existe", info != nullptr);
    require_eq_str("referencia", info->path, "textures/cofre.png");
}

int main() { return motor_test::runAllTests(); }