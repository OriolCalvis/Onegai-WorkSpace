// EDITOR-UX-01 — ver editor/include/ToolsPanel.h. El cuerpo de drawObjects/
// drawTiles viene de EditorApplication.cpp (E-10) tal cual, mas los tooltips.
#include "ToolsPanel.h"

#include <algorithm>

#include "Editor/EditorAyuda.h"

#include "imgui.h"

namespace Editor {

namespace {
void reportar(std::string& status, const Result<bool>& result) {
    if (!result.isOk()) status = result.errorMessage();
    else if (result.value()) status = "Listo";
}
}  // namespace

void ToolsPanel::record(const std::string& control) {
    const auto a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
    m_points[control] = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
}

Vector2 ToolsPanel::testPoint(const std::string& control) const {
    const auto it = m_points.find(control);
    return it == m_points.end() ? Vector2{-100, -100} : it->second;
}

void ToolsPanel::drawTools(const ToolsContext& ctx) {
    const float lado = 18.0f * ctx.uiScale;
    if (ImGui::BeginTable("tools", 2)) {
        for (const auto& action : editorActionDefinitions()) {
            if (action.group != ActionGroup::Tools) continue;
            ImGui::TableNextColumn();
            const AyudaAccion& ayuda = ayudaDeAccion(action.id);
            const bool elegida = ctx.actions.tool() == action.id;
            if (m_iconos.boton(claveDeAccion(action.id), ayuda.icono, action.label, lado, elegida))
                reportar(ctx.status, ctx.actions.execute(action.id, ctx.session));
            // El tooltip cubre icono y etiqueta: el ultimo item es la etiqueta,
            // pero el humo debe pulsar el boton (record guarda su centro).
            if (m_iconos.ultimoBotonHovered())
                EditorIconos::mostrar(action.label, shortcutLabel(action, ctx.mac) + " con el mapa enfocado", ayuda.ayuda);
            if (action.id == EditorAction::Fill) {
                const ImVec2 c = m_iconos.centroUltimoBoton();
                m_points["fill"] = {c.x, c.y};
            }
        }
        ImGui::EndTable();
    }
}

void ToolsPanel::drawObjects(const ToolsContext& ctx) {
    MapView& map = ctx.map;
    const bool catalogReady = map.path == ctx.session.levelPath();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##object_search", "Buscar objeto...", ctx.objectSearch, ctx.objectSearchSize);
    record("objectSearch");
    EditorIconos::tooltip("Buscar objeto", "", ayudaDeControl("herramientas.objeto.buscar"));
    ImGuiTextFilter filter(ctx.objectSearch);
    const float listHeight = std::clamp(ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeightWithSpacing() * 2,
        ImGui::GetTextLineHeightWithSpacing() * 1.5f, 125 * ctx.uiScale);
    ImGui::BeginChild("Catálogo de objetos", {0, listHeight}, ImGuiChildFlags_Borders);
    int matches = 0;
    if (catalogReady) for (const auto& id : map.objectIds) {
        const auto* def = map.catalog.find(id);
        if (!filter.PassFilter(def->name.c_str())) continue;
        ++matches;
        ImGui::PushID(id.c_str());
        // La identidad del widget es el ID; el nombre es texto literal.
        if (ImGui::Selectable("##object", ctx.selectedObject == id)) ctx.selectedObject = id;
        const auto a = ImGui::GetItemRectMin();
        if (id == "pocion") record("objectRow");
        ImGui::GetWindowDrawList()->AddText(a, ImGui::GetColorU32(ImGuiCol_Text), def->name.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\nID: %s\n%s", def->name.c_str(), id.c_str(), ayudaDeControl("herramientas.objeto"));
        ImGui::PopID();
    }
    if (!matches) ImGui::TextWrapped(catalogReady && map.catalog.size() ? "Sin coincidencias" : "No hay catálogo de objetos para este mapa");
    ImGui::EndChild();
    const auto* chosen = catalogReady ? map.catalog.find(ctx.selectedObject) : nullptr;
    m_iconos.imagen("objetos", 16.0f * ctx.uiScale);
    ImGui::SameLine();
    ImGui::TextWrapped("Objeto: %s", chosen ? chosen->name.c_str() : "selecciona uno en la lista");
    EditorIconos::tooltip("Objeto elegido", "", ayudaDeControl("herramientas.objeto"));
}

void ToolsPanel::drawTiles(const ToolsContext& ctx) {
    MapView& map = ctx.map;
    m_iconos.imagen("baldosa", 16.0f * ctx.uiScale);
    ImGui::SameLine();
    ImGui::Text("Baldosa %d", ctx.selectedGid);
    EditorIconos::tooltip("Baldosa elegida", "", ayudaDeControl("herramientas.baldosa"));
    const int count = ctx.session.tileset().tileCount;
    ctx.selectedGid = std::clamp(ctx.selectedGid, 1, std::max(1, count));
    if (map.atlas && map.path == ctx.session.levelPath()) {
        ImGui::BeginChild("Paleta de baldosas", {0, 125 * ctx.uiScale}, ImGuiChildFlags_Borders);
        const int perRow = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / (58 * ctx.uiScale)));
        for (int gid = 1; gid <= count; ++gid) {
            const auto uv = map.atlas->getUV(gid);
            ImGui::PushID(gid);
            const bool chosen = gid == ctx.selectedGid;
            if (chosen) ImGui::PushStyleColor(ImGuiCol_Button, {0.18f, 0.65f, 0.52f, 1});
            if (ImGui::ImageButton("tile", static_cast<ImTextureID>(map.textureId), {46 * ctx.uiScale, 23 * ctx.uiScale}, {uv.u0, uv.v0}, {uv.u1, uv.v1})) ctx.selectedGid = gid;
            if (chosen) ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Baldosa %d%s", gid, chosen ? " (elegida)" : "");
            ImGui::PopID();
            if (gid % perRow) ImGui::SameLine();
        }
        ImGui::EndChild();
    }
}

void ToolsPanel::draw(const ToolsContext& ctx) {
    ImGui::Begin("Herramientas");
    ImGui::BeginDisabled(!ctx.session.hasDocument() || ctx.blocked);
    drawTools(ctx);
    if (ctx.session.hasDocument()) {
        const bool objectTool = ctx.actions.tool() == EditorAction::Objects || ctx.actions.tool() == EditorAction::RemoveObject;
        if (objectTool) drawObjects(ctx);
        else drawTiles(ctx);
        // Pie: la herramienta activa con su icono y el uso del raton.
        const auto& activa = *std::find_if(editorActionDefinitions().begin(), editorActionDefinitions().end(),
            [&](const ActionDefinition& d) { return d.id == ctx.actions.tool(); });
        m_iconos.imagen(ayudaDeAccion(activa.id).icono, 16.0f * ctx.uiScale);
        ImGui::SameLine();
        ImGui::TextWrapped("%s: %s", activa.label, usoDelRaton(activa.id).c_str());
        EditorIconos::tooltip("Uso del raton", "", ayudaDeControl("herramientas.uso"));
        for (const auto& action : editorActionDefinitions()) {
            if (action.id != EditorAction::Center && action.id != EditorAction::Save) continue;
            const AyudaAccion& ayuda = ayudaDeAccion(action.id);
            if (m_iconos.boton(claveDeAccion(action.id), ayuda.icono, action.label, 16.0f * ctx.uiScale, false))
                reportar(ctx.status, ctx.actions.execute(action.id, ctx.session));
            if (m_iconos.ultimoBotonHovered()) EditorIconos::mostrar(action.label, shortcutLabel(action, ctx.mac), ayuda.ayuda);
            if (action.id == EditorAction::Center) ImGui::SameLine();
        }
    }
    ImGui::EndDisabled();
    ImGui::End();
}

void ToolsPanel::drawHelp(const ToolsContext& ctx, const ImGuiViewport* viewport) {
    if (!ctx.actions.helpVisible()) return;
    ImGui::SetNextWindowSize({std::min(560 * ctx.uiScale, viewport->WorkSize.x - 32),
        std::min(520 * ctx.uiScale, viewport->WorkSize.y - 32)}, ImGuiCond_FirstUseEver);
    bool open = true;
    ImGui::Begin("Ayuda de controles", &open);
    const float lado = 16.0f * ctx.uiScale;
    ActionGroup grupo = ActionGroup::File;
    bool primero = true;
    for (const auto& action : editorActionDefinitions()) {
        if (primero || action.group != grupo) {
            grupo = action.group;
            primero = false;
            ImGui::SeparatorText(grupo == ActionGroup::File ? "Archivo" : grupo == ActionGroup::Edit ? "Editar" :
                                 grupo == ActionGroup::Tools ? "Herramientas" : "Ver");
        }
        const AyudaAccion& ayuda = ayudaDeAccion(action.id);
        m_iconos.imagen(ayuda.icono, lado);
        ImGui::SameLine();
        ImGui::TextWrapped("%s — %s: %s", shortcutLabel(action, ctx.mac).c_str(), action.label, ayuda.ayuda);
    }
    ImGui::SeparatorText("Raton en el mapa");
    ImGui::TextWrapped("%s", ayudaDeControl("mapa.vista"));
    ImGui::TextWrapped("Haz clic en Mapa para enfocar las herramientas. Los campos de texto y los modales capturan el teclado.");
    ImGui::SeparatorText("Marcas del mapa");
    m_iconos.imagen("inicio", lado); ImGui::SameLine(); ImGui::TextWrapped("Circulo verde: inicio del jugador.");
    m_iconos.imagen("objetos", lado); ImGui::SameLine(); ImGui::TextWrapped("Punto amarillo: objeto colocado (pasa el raton para ver cual).");
    m_iconos.imagen("seleccion", lado); ImGui::SameLine(); ImGui::TextWrapped("Circulo azul: ultima celda seleccionada (ver Inspector).");
    ImGui::End();
    if (!open) reportar(ctx.status, ctx.actions.execute(EditorAction::Help, ctx.session));
}

}  // namespace Editor
