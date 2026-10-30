// EDITOR-UX-01 — Ayuda y referencias visuales de cada opcion de motor_editor,
// como datos y sin ImGui: que icono lleva cada accion (assets/textures/
// iconos_editor.json), que explica su tooltip y que explica cada control de
// los paneles (Proyecto, Herramientas, Inspector, Capas). Los paneles solo
// piden aqui el texto; la prueba comprueba que ninguna opcion se queda sin
// ayuda ni sin icono.
#pragma once

#include <string>
#include <vector>

#include "Editor/EditorActions.h"

namespace Editor {

struct AyudaAccion {
    const char* icono;  // id en iconos_editor.json
    const char* ayuda;  // una o dos frases: que hace y como se usa
};

const AyudaAccion& ayudaDeAccion(EditorAction id);
// "save", "paint"... (la clave de la tabla `acciones` del manifiesto).
const char* claveDeAccion(EditorAction id);

// Controles de los paneles: id estable -> texto. "" si no existe.
const char* ayudaDeControl(const std::string& id);
const std::vector<std::string>& controlesConAyuda();

// Lo que hace el boton izquierdo/derecho con la herramienta activa (pie
// del panel Herramientas y ayuda de controles).
std::string usoDelRaton(EditorAction tool);

}  // namespace Editor
