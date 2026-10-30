// EST-26 (relevo A) — LA FONTANERIA DE IMGUI DEL EDITOR.
//
// Tres cosas que no son el editor, sino lo que hay debajo: el contexto de
// ImGui y su apagado, la traduccion de un nombre de atajo a una tecla, y
// el reparto inicial de los paneles. Estaban dentro de
// EditorApplication.cpp junto a las otras 700 lineas.
//
// EL APAGADO ES LO QUE JUSTIFICA LA CLASE. GuiContext monta ImGui en el
// constructor y lo desmonta en el destructor, incluida la salida por
// excepcion: si el renderer no arranca, lo que ya se habia montado se
// desmonta antes de lanzar. Un ImGui a medio apagar no falla aqui, falla
// en el driver y sin traza.

#pragma once

#include "imgui.h"

struct GLFWwindow;

namespace Editor {

struct GuiContext {
    bool platform = false, renderer = false;
    explicit GuiContext(GLFWwindow* window);
    void shutdown();
    ~GuiContext();
};

// Nombres propios del contrato de acciones, nunca nombres de depuración de ImGui.
ImGuiKey shortcutKey(const char* label);

void defaultLayout(ImGuiID dock, ImVec2 size);

}  // namespace Editor
