// EDITOR-UX-01 — El atlas de iconos del editor (assets/textures/
// iconos_editor.png + .json) montado para ImGui: una imagen por id y un
// boton con icono que se resalta cuando esta elegido. Si la textura o el
// manifiesto faltan, imagen() no pinta nada y boton() cae en un boton de
// texto: el editor sigue funcionando igual, solo sin dibujitos.
#pragma once

#include <string>

#include "Core/Resources/TextureManager.h"
#include "HUD/Iconos.h"

#include "imgui.h"

namespace Editor {

class EditorIconos {
public:
    // Carga perezosa: la primera llamada a imagen()/boton() intenta abrir
    // la textura (necesita un contexto GL vivo, por eso no va en el ctor).
    bool listo() const { return m_textureId != 0 && m_manifiesto.cargado(); }
    void cargar();

    // Icono de `lado` pixeles logicos, en el cursor actual de ImGui.
    void imagen(const char* id, float lado);
    // Icono + etiqueta como boton; `elegido` lo pinta con el color de
    // seleccion. Devuelve true al pulsarlo. Sin atlas: ImGui::Button.
    bool boton(const char* strId, const char* icono, const char* etiqueta, float lado, bool elegido);
    // Centro del ultimo boton() pintado (el pulsable, no su etiqueta), para
    // el harness de humo.
    ImVec2 centroUltimoBoton() const { return m_centroUltimoBoton; }
    // true si el raton esta sobre el ultimo boton() (icono o etiqueta).
    bool ultimoBotonHovered() const { return m_ultimoHovered; }
    // Tooltip con titulo, atajo (opcional) y explicacion, si el ultimo
    // item de ImGui esta bajo el raton; mostrar() lo pinta sin comprobar.
    static void tooltip(const char* titulo, const std::string& atajo, const char* ayuda);
    static void mostrar(const char* titulo, const std::string& atajo, const char* ayuda);

private:
    bool uv(const char* id, ImVec2& uv0, ImVec2& uv1) const;

    TextureManager m_texturas;
    Hud::Iconos m_manifiesto;
    unsigned int m_textureId = 0;
    ImVec2 m_centroUltimoBoton{-100, -100};
    bool m_ultimoHovered = false;
    bool m_intentado = false;
};

}  // namespace Editor
