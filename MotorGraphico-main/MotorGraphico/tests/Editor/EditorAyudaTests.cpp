// EDITOR-UX-01 — Ninguna opcion del editor sin ayuda ni sin icono.
#include "Editor/EditorAyuda.h"
#include "HUD/Iconos.h"
#include "TestHarness.h"

#include <cstring>
#include <set>
#include <string>

using namespace Editor;

MOTOR_TEST_CASE(cada_accion_tiene_ayuda_icono_y_clave_distinta) {
    std::set<std::string> claves, iconos;
    for (const auto& def : editorActionDefinitions()) {
        const AyudaAccion& ayuda = ayudaDeAccion(def.id);
        MOTOR_CHECK(std::strlen(ayuda.ayuda) >= 20 && std::strlen(ayuda.ayuda) <= 140);
        MOTOR_CHECK(std::strlen(ayuda.icono) > 0);
        MOTOR_CHECK(std::strlen(claveDeAccion(def.id)) > 0);
        MOTOR_CHECK(claves.insert(claveDeAccion(def.id)).second);
        MOTOR_CHECK(iconos.insert(ayuda.icono).second);  // un icono por accion, sin repetir
    }
}

MOTOR_TEST_CASE(los_iconos_de_las_acciones_existen_en_el_atlas_del_editor) {
    Hud::Iconos atlas;
    MOTOR_CHECK(atlas.cargar("assets/textures/iconos_editor.json").isOk());
    for (const auto& def : editorActionDefinitions()) {
        MOTOR_CHECK(atlas.frame(ayudaDeAccion(def.id).icono) > 0);
    }
    for (const char* control : {"visible", "oculta", "bloqueada", "desbloqueada", "nueva", "buscar",
                                "renombrar", "mapa", "problema", "aviso", "ok", "proyecto", "baldosa",
                                "seleccion"}) {
        MOTOR_CHECK(atlas.frame(control) > 0);
    }
}

MOTOR_TEST_CASE(cada_control_de_los_paneles_tiene_texto_y_lo_desconocido_devuelve_vacio) {
    const auto& ids = controlesConAyuda();
    MOTOR_CHECK(ids.size() >= 20);
    for (const std::string& id : ids) {
        const std::string texto = ayudaDeControl(id);
        MOTOR_CHECK(texto.size() >= 20 && texto.size() <= 140);
        MOTOR_CHECK(id.find('.') != std::string::npos);  // panel.control
    }
    for (const char* panel : {"proyecto", "herramientas", "inspector", "capas", "mapa"}) {
        bool alguno = false;
        for (const std::string& id : ids) alguno = alguno || id.rfind(std::string(panel) + ".", 0) == 0;
        MOTOR_CHECK(alguno);
    }
    MOTOR_CHECK(std::string(ayudaDeControl("no.existe")).empty());
}

MOTOR_TEST_CASE(el_uso_del_raton_depende_de_la_herramienta) {
    MOTOR_CHECK(usoDelRaton(EditorAction::Paint).find("pintar") != std::string::npos);
    MOTOR_CHECK(usoDelRaton(EditorAction::Objects).find("objeto") != std::string::npos);
    MOTOR_CHECK(usoDelRaton(EditorAction::Fill).find("rellenar") != std::string::npos);
    MOTOR_CHECK(usoDelRaton(EditorAction::PlayerStart).find("inicio") != std::string::npos);
    MOTOR_CHECK(usoDelRaton(EditorAction::Save) == usoDelRaton(EditorAction::Paint));
}

int main() { return motor_test::runAllTests(); }
