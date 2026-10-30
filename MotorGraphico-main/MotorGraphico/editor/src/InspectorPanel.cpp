// EDITOR-UX-01 — ver editor/include/InspectorPanel.h.
#include "InspectorPanel.h"

#include "Editor/EditorAyuda.h"

#include "imgui.h"

namespace Editor {

void InspectorPanel::fila(const char* icono, const char* control, const char* titulo,
                          const std::string& texto, float uiScale) {
    m_iconos.imagen(icono, 16.0f * uiScale);
    ImGui::SameLine();
    ImGui::TextWrapped("%s", texto.c_str());
    EditorIconos::tooltip(titulo, "", ayudaDeControl(control));
}

void InspectorPanel::draw(const EditorDocumentSession& session, const ObjectCatalog& catalog,
                          GridCoord selected, const std::string& status, float uiScale) {
    ImGui::Begin("Inspector");
    if (session.hasDocument()) {
        const auto& state = session.state();
        fila("mapa", "inspector.nombre", "Mapa", session.name(), uiScale);
        ImGui::TextDisabled("%d × %d celdas", state.width(), state.height());
        EditorIconos::tooltip("Tamano", "", ayudaDeControl("inspector.tamano"));
        fila("objetos", "inspector.objetos", "Objetos", std::to_string(state.objects().size()) + " objetos", uiScale);
        fila("inicio", "inspector.inicio", "Inicio del jugador",
             "Inicio: " + std::to_string(state.playerStart().x) + ", " + std::to_string(state.playerStart().y), uiScale);
        ImGui::SeparatorText("Capas");
        EditorIconos::tooltip("Capas", "", ayudaDeControl("inspector.capas"));
        for (const auto& layer : state.document().layers()) {
            const auto& meta = layer.meta();
            m_iconos.imagen(meta.visible ? "visible" : "oculta", 16.0f * uiScale);
            ImGui::SameLine();
            ImGui::TextWrapped("%s%s", meta.name.c_str(), meta.locked ? " (bloqueada)" : "");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s%s%s", meta.name.c_str(), meta.visible ? "" : " · oculta", meta.locked ? " · bloqueada" : "");
        }
        if (selected.x >= 0) {
            ImGui::SeparatorText("Selección");
            EditorIconos::tooltip("Seleccion", "", ayudaDeControl("inspector.seleccion"));
            fila("seleccion", "inspector.seleccion", "Celda",
                 "Celda " + std::to_string(selected.x) + ", " + std::to_string(selected.y), uiScale);
            fila("baldosa", "inspector.seleccion", "Baldosa",
                 "Baldosa " + std::to_string(state.tileAt(selected.x, selected.y)), uiScale);
            if (const auto* object = state.objectAt(selected.x, selected.y)) {
                const auto* def = catalog.find(object->objectId);
                fila("objetos", "inspector.seleccion", "Objeto", "Objeto: " +
                     (!object->displayName.empty() ? object->displayName : (def ? def->name : object->objectId)), uiScale);
            }
        }
        ImGui::Separator();
        m_iconos.imagen(session.dirty() ? "aviso" : "ok", 16.0f * uiScale);
        ImGui::SameLine();
        ImGui::TextColored(session.dirty() ? ImVec4{1, 0.72f, 0.35f, 1} : ImVec4{0.45f, 0.85f, 0.68f, 1}, "%s",
                           session.dirty() ? "Cambios sin guardar" : "Guardado");
        EditorIconos::tooltip("Guardado", "", ayudaDeControl("inspector.guardado"));
    }
    ImGui::SeparatorText("Estado");
    const bool error = !status.empty() && status != "Listo";
    m_iconos.imagen(error ? "problema" : "ok", 16.0f * uiScale);
    ImGui::SameLine();
    ImGui::TextWrapped("%s", status.empty() ? "Listo" : status.c_str());
    EditorIconos::tooltip("Estado", "", ayudaDeControl("inspector.estado"));
    ImGui::End();
}

}  // namespace Editor
