#include "LayerPanel.h"
#include "EditorIconos.h"
#include "Editor/EditorAyuda.h"
#include "imgui.h"
#include <algorithm>

namespace Editor {
namespace {
int resizeName(ImGuiInputTextCallbackData* data) {
    auto& buffer = *static_cast<std::vector<char>*>(data->UserData);
    buffer.resize(static_cast<std::size_t>(data->BufSize));
    data->Buf = buffer.data();
    return 0;
}
}
Vector2 LayerPanel::testPoint(const std::string& control) const {
    const auto it = m_points.find(control);
    return it == m_points.end() ? Vector2{} : it->second;
}
void LayerPanel::record(const std::string& control) {
    const auto a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
    m_points[control] = {(a.x+b.x)*0.5f, (a.y+b.y)*0.5f};
}
void LayerPanel::draw(EditorDocumentSession& session, std::string& status, bool focus) {
    auto report = [&](const Result<bool>& result) {
        if (!result.isOk()) status = result.errorMessage();
        else if (result.value()) status = "Listo";
    };
    if (focus) ImGui::SetNextWindowFocus();
    const bool visible = ImGui::Begin("Capas");
    if (modalOpen() && (session.revision() != m_removeRevision || session.generation() != m_generation)) {
        m_removing = {};
        status = "El documento cambió; se canceló el borrado de capa";
    }
    if (visible && session.hasDocument()) {
        auto layers = session.tileLayers();
        ImGui::TextUnformatted("Capas de baldosas");
        ImGui::BeginDisabled(session.confirmationPending() || modalOpen());
        const float height = ImGui::GetTextLineHeightWithSpacing() * 3;
        EditorIconos::tooltip("Capas de baldosas", "", ayudaDeControl("capas.lista"));
        if (ImGui::BeginListBox("##layers", {-1, height})) {
            for (const auto& layer : layers) {
                ImGui::PushID(layer.id.value().c_str());
                if (ImGui::Selectable("##layer", layer.id == session.activeTileLayer())) report(session.selectTileLayer(layer.id));
                record("layer:" + layer.id.value());
                ImGui::GetWindowDrawList()->AddText(ImGui::GetItemRectMin(), ImGui::GetColorU32(ImGuiCol_Text), layer.name.c_str());
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s%s%s", layer.name.c_str(), layer.visible ? "" : " · oculta", layer.locked ? " · bloqueada" : "");
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
        if (ImGui::Button("Nueva capa")) report(session.addTileLayer());
        record("add");
        EditorIconos::tooltip("Nueva capa", "", ayudaDeControl("capas.nueva"));
        layers = session.tileLayers();
        const auto activeId = session.activeTileLayer();
        const auto found = std::find_if(layers.begin(), layers.end(), [&](const auto& layer) { return layer.id == activeId; });
        if (found != layers.end()) {
            auto meta = *found; // Las mutaciones invalidan las vistas del documento.
            if (m_generation != session.generation() || m_boundId != meta.id.value() || m_savedName != meta.name) {
                m_generation = session.generation(); m_boundId = meta.id.value(); m_savedName = meta.name;
                m_name.assign(std::max<std::size_t>(128, meta.name.size()+1), '\0');
                std::copy(meta.name.begin(), meta.name.end(), m_name.begin());
            }
            ImGui::SeparatorText("Capa seleccionada");
            ImGui::SetNextItemWidth(-1);
            bool rename = ImGui::InputText("##layer_name", m_name.data(), m_name.size(),
                ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_EnterReturnsTrue, resizeName, &m_name);
            record("name");
            EditorIconos::tooltip("Nombre de la capa", "", ayudaDeControl("capas.nombre"));
            rename = ImGui::Button("Renombrar") || rename;
            record("rename");
            EditorIconos::tooltip("Renombrar", "", ayudaDeControl("capas.renombrar"));
            if (rename) {
                auto renamed = meta; renamed.name = m_name.data();
                const auto result = session.updateTileLayer(renamed); report(result);
                if (result.isOk()) meta = std::move(renamed);
            }
            bool shown = meta.visible;
            if (ImGui::Checkbox("Visible", &shown)) { meta.visible = shown; report(session.updateTileLayer(meta)); }
            record("visible");
            EditorIconos::tooltip(shown ? "Visible" : "Oculta", "", ayudaDeControl("capas.visible"));
            bool locked = meta.locked;
            if (ImGui::Checkbox("Bloqueada", &locked)) { meta.locked = locked; report(session.updateTileLayer(meta)); }
            record("locked");
            EditorIconos::tooltip(locked ? "Bloqueada" : "Desbloqueada", "", ayudaDeControl("capas.bloqueada"));
            ImGui::BeginDisabled(layers.size() <= 1 || meta.locked);
            if (ImGui::Button("Borrar capa...")) { m_removing = meta.id; m_removeRevision = session.revision(); }
            record("remove");
            EditorIconos::tooltip("Borrar capa", "", ayudaDeControl("capas.borrar"));
            ImGui::EndDisabled();
            if (layers.size() <= 1) ImGui::TextWrapped("Debe quedar una capa de baldosas.");
            else if (meta.locked) ImGui::TextWrapped("Desbloquea la capa para pintar o borrarla.");
        }
        ImGui::EndDisabled();
    } else if (visible) ImGui::TextWrapped("Abre un mapa para editar sus capas.");
    if (modalOpen()) ImGui::OpenPopup("Borrar capa");
    const auto* viewport = ImGui::GetMainViewport();
    const float scale = ImGui::GetFontSize() / 16.0f;
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({std::min(480 * scale, viewport->WorkSize.x - 32),
                            std::min(180 * scale, viewport->WorkSize.y - 32)}, ImGuiCond_Always);
    if (ImGui::BeginPopupModal("Borrar capa", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        if (!modalOpen()) ImGui::CloseCurrentPopup();
        else {
            ImGui::TextWrapped("Se borrarán las baldosas de esta capa. Puedes deshacer esta operación.");
            if (ImGui::Button("Borrar")) { report(session.removeTileLayer(m_removing)); m_removing = {}; ImGui::CloseCurrentPopup(); }
            record("confirm");
            ImGui::SameLine();
            if (ImGui::Button("Cancelar")) { m_removing = {}; ImGui::CloseCurrentPopup(); }
            record("cancel");
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
} // namespace Editor
