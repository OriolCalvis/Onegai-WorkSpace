// EDITOR-UX-01 — ver editor/include/EditorIconos.h.
#include "EditorIconos.h"

#include <glad/glad.h>

#include "Core/Resources/Texture.h"

namespace Editor {

void EditorIconos::cargar() {
    if (m_intentado) return;
    m_intentado = true;
    if (!m_manifiesto.cargar("assets/textures/iconos_editor.json").isOk()) return;
    auto textura = m_texturas.load("iconos_editor", "assets/textures/" + m_manifiesto.imagen());
    if (!textura.isOk()) return;
    textura.value()->bind();
    GLint nombre = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &nombre);
    m_textureId = static_cast<unsigned int>(nombre);
    // Pixel art: nada de filtrado bilineal al escalar el icono.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

bool EditorIconos::uv(const char* id, ImVec2& uv0, ImVec2& uv1) const {
    const int frame = m_manifiesto.frame(id);
    if (!listo() || frame <= 0) return false;
    const int columnas = m_manifiesto.columnas();
    const int filas = m_manifiesto.filas();
    if (columnas <= 0 || filas <= 0) return false;
    const int col = (frame - 1) % columnas;
    // TextureManager invierte el PNG al cargarlo: la fila 0 de la imagen
    // queda arriba en coordenadas v invertidas (misma regla que iconos.json).
    const int fila = filas - 1 - (frame - 1) / columnas;
    const float ancho = 1.0f / static_cast<float>(columnas);
    const float alto = 1.0f / static_cast<float>(filas);
    uv0 = {static_cast<float>(col) * ancho, static_cast<float>(fila + 1) * alto};
    uv1 = {static_cast<float>(col + 1) * ancho, static_cast<float>(fila) * alto};
    return true;
}

void EditorIconos::imagen(const char* id, float lado) {
    cargar();
    ImVec2 uv0, uv1;
    if (!uv(id, uv0, uv1)) return;
    ImGui::Image(static_cast<ImTextureID>(m_textureId), {lado, lado}, uv0, uv1);
}

bool EditorIconos::boton(const char* strId, const char* icono, const char* etiqueta, float lado, bool elegido) {
    cargar();
    ImVec2 uv0, uv1;
    if (elegido) ImGui::PushStyleColor(ImGuiCol_Button, {0.18f, 0.65f, 0.52f, 1});
    bool pulsado = false;
    if (uv(icono, uv0, uv1)) {
        ImGui::PushID(strId);
        pulsado = ImGui::ImageButton("##icono", static_cast<ImTextureID>(m_textureId), {lado, lado}, uv0, uv1);
        ImGui::PopID();
        const auto a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
        m_centroUltimoBoton = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
        m_ultimoHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        ImGui::SameLine(0.0f, 4.0f);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(etiqueta);
        m_ultimoHovered = m_ultimoHovered || ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
    } else {
        pulsado = ImGui::Button(etiqueta);
        const auto a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
        m_centroUltimoBoton = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
        m_ultimoHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
    }
    if (elegido) ImGui::PopStyleColor();
    return pulsado;
}

void EditorIconos::tooltip(const char* titulo, const std::string& atajo, const char* ayuda) {
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) mostrar(titulo, atajo, ayuda);
}

void EditorIconos::mostrar(const char* titulo, const std::string& atajo, const char* ayuda) {
    ImGui::BeginTooltip();
    ImGui::TextUnformatted(titulo);
    if (!atajo.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("(%s)", atajo.c_str());
    }
    if (ayuda != nullptr && *ayuda) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 22.0f);
        ImGui::TextUnformatted(ayuda);
        ImGui::PopTextWrapPos();
    }
    ImGui::EndTooltip();
}

}  // namespace Editor
