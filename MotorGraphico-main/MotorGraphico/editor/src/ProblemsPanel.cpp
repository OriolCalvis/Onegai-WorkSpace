// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-15  (fase E, 3 j) - Problemes i cerca d'esdeveniments
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "ProblemsPanel.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
// std::strncpy (drawReferences). Sin esto, g++ en Linux no compila
// este fichero — y la CI construye motor_editor en ubuntu-latest con
// g++ y clang++. En macOS colaba porque libc++ lo arrastra por otra
// cabecera; eso no es una garantia, es una casualidad.
// (Repuesto el 23/09: una sesion guardo encima y se lo llevo.)
#include <cstring>

namespace Editor {
namespace {

const char* SeverityLabel(RPG::DiagnosticSeverity s) {
    switch (s) {
        case RPG::DiagnosticSeverity::Error:   return "[!]";
        case RPG::DiagnosticSeverity::Warning: return "[~]";
    }
    return "[?]";
}

ImU32 SeverityColor(RPG::DiagnosticSeverity s) {
    switch (s) {
        case RPG::DiagnosticSeverity::Error:
            return IM_COL32(220, 60, 60, 255);
        case RPG::DiagnosticSeverity::Warning:
            return IM_COL32(220, 180, 40, 255);
    }
    return IM_COL32(180, 180, 180, 255);
}

std::string SitioTexto(const ProblemLocation& where) {
    std::string texto = where.mapId;
    if (!where.eventId.empty()) {
        texto += " > ";
        texto += where.eventId;
    }
    if (where.hasPage) {
        texto += " / pag ";
        texto += std::to_string(where.pageIndex + 1);
    }
    if (where.hasCommand && !where.commandPath.empty()) {
        texto += " [";
        for (std::size_t i = 0; i < where.commandPath.size(); ++i) {
            if (i > 0) texto += '.';
            texto += std::to_string(where.commandPath[i]);
        }
        texto += ']';
        if (!where.commandType.empty()) {
            texto += ' ';
            texto += where.commandType;
        }
    }
    return texto;
}

ReferenceKind KindFromIndex(int index) {
    switch (index) {
        case 0: return ReferenceKind::Flag;
        case 1: return ReferenceKind::Variable;
        case 2: return ReferenceKind::LocalSwitch;
        case 3: return ReferenceKind::Event;
        case 4: return ReferenceKind::Map;
        case 5: return ReferenceKind::Object;
    }
    return ReferenceKind::Flag;
}

}  // namespace

ProblemsPanel::Point ProblemsPanel::testPoint(const std::string& control) const {
    const auto it = m_points.find(control);
    return it == m_points.end() ? Point{} : it->second;
}

void ProblemsPanel::record(const std::string& control) {
    const auto a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
    m_points[control] = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
}

void ProblemsPanel::draw(ProblemsState& state, const MapDocument* document,
                          std::string& status, NavigateCallback onNavigate) {
    if (m_focusRequested) {
        ImGui::SetNextWindowFocus();
        m_focusRequested = false;
    }

    const bool visible = ImGui::Begin("Problemas");
    if (!visible) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("##problemas_tabs")) {
        if (ImGui::BeginTabItem("Problemas")) {
            drawProblemList(state, document, status, onNavigate);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Buscar")) {
            drawSearch(document);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Referencias")) {
            drawReferences(document);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // Barra de estado inferior: contadores y puerta de playtest.
    ImGui::Separator();
    const auto errores = state.errorCount();
    const auto avisos = state.warningCount();
    if (errores > 0) {
        ImGui::TextColored(ImVec4(0.86f, 0.24f, 0.24f, 1.0f),
                           "%zu error%s", errores, errores == 1 ? "" : "es");
        ImGui::SameLine();
    }
    ImGui::Text("%zu aviso%s", avisos, avisos == 1 ? "" : "s");
    ImGui::SameLine();
    if (state.canPlaytest()) {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "| listo para probar");
    } else {
        ImGui::TextColored(ImVec4(0.86f, 0.24f, 0.24f, 1.0f),
                           "| no se puede probar");
    }
    record("status");

    ImGui::End();
}

void ProblemsPanel::drawProblemList(ProblemsState& state,
                                     const MapDocument* document,
                                     std::string& status,
                                     const NavigateCallback& onNavigate) {
    const auto& problemas = state.problems();
    if (problemas.empty()) {
        ImGui::TextWrapped("Sin problemas. Valida un mapa para ver los hallazgos.");
        return;
    }

    // Aviso de cambio de foco.
    switch (state.lastFocusChange()) {
        case ProblemsState::FocusChange::MovedToNext:
            ImGui::TextColored(ImVec4(0.86f, 0.7f, 0.2f, 1.0f),
                               "El problema que mirabas se resolvio; foco en el siguiente.");
            break;
        case ProblemsState::FocusChange::Lost:
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                               "Todos los problemas se resolvieron.");
            break;
        default:
            break;
    }

    // Lista.
    const float height = ImGui::GetTextLineHeightWithSpacing() *
                         std::min<std::size_t>(problemas.size(), 12);
    if (ImGui::BeginListBox("##problemas", {-1, height})) {
        for (std::size_t i = 0; i < problemas.size(); ++i) {
            const auto& p = problemas[i];
            const bool selected = state.hasFocus() && state.focusIndex() == i;
            ImGui::PushID(static_cast<int>(i));

            if (ImGui::Selectable("##row", selected)) {
                state.focusIndexAt(i);
            }
            record("problem:" + std::to_string(i));

            // Texto superpuesto: severidad + sitio + mensaje.
            const auto min = ImGui::GetItemRectMin();
            const auto dl = ImGui::GetWindowDrawList();
            const ImU32 sevColor = SeverityColor(p.severity);
            const ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);

            char label[512];
            std::snprintf(label, sizeof(label), "%s %s — %s",
                          SeverityLabel(p.severity),
                          SitioTexto(p.where).c_str(),
                          p.message.c_str());
            dl->AddText(min, p.isError() ? sevColor : textColor, label);

            // Tooltip con el detalle completo.
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s\n%s\n%s",
                                  p.isError() ? "Error" : "Aviso",
                                  SitioTexto(p.where).c_str(),
                                  p.message.c_str());
            }

            ImGui::PopID();
        }
        ImGui::EndListBox();
    }

    // Controles de navegacion.
    ImGui::BeginDisabled(!state.hasFocus());
    if (ImGui::Button("Ir ahi")) {
        record("goto");
        if (document != nullptr && onNavigate) {
            NavigationRequest nav = state.navigate(*document);
            onNavigate(nav);
            if (!nav.note.empty()) {
                status = nav.note;
            } else if (nav.ok()) {
                status = "Navegacion OK";
            }
        } else if (document == nullptr) {
            status = "Sin documento abierto";
        }
    }
    record("goto");

    ImGui::SameLine();
    if (ImGui::Button("Anterior")) { state.focusPrevious(); }
    record("prev");
    ImGui::SameLine();
    if (ImGui::Button("Siguiente")) { state.focusNext(); }
    record("next");
    ImGui::EndDisabled();

    // Detalle del problema enfocado.
    if (const ProblemEntry* f = state.focused()) {
        ImGui::SeparatorText("Detalle");
        ImGui::TextWrapped("%s: %s",
                           f->isError() ? "Error" : "Aviso",
                           f->message.c_str());
        ImGui::TextDisabled("%s", SitioTexto(f->where).c_str());
    }
}

void ProblemsPanel::drawSearch(const MapDocument* document) {
    ImGui::TextUnformatted("Buscar eventos por nombre:");
    const bool changed = ImGui::InputText("##search_query", m_searchQuery,
                                           sizeof(m_searchQuery));
    record("search_query");

    if (document == nullptr) {
        ImGui::TextWrapped("Abre un mapa para buscar.");
        return;
    }

    if (changed || m_searchResults.empty()) {
        m_searchResults = SearchEventsByName(*document, m_searchQuery);
    }

    ImGui::Text("%zu resultado%s", m_searchResults.size(),
                m_searchResults.size() == 1 ? "" : "s");

    const float height = ImGui::GetTextLineHeightWithSpacing() *
                         std::min<std::size_t>(m_searchResults.size(), 10);
    if (ImGui::BeginListBox("##search_results", {-1, height})) {
        for (std::size_t i = 0; i < m_searchResults.size(); ++i) {
            const auto& hit = m_searchResults[i];
            char label[256];
            std::snprintf(label, sizeof(label), "%s (%s)",
                          hit.eventName.c_str(), hit.where.eventId.c_str());
            if (ImGui::Selectable(label)) {
                // La seleccion enfoca el evento en el mapa. El editor
                // decide que hacer; el panel solo lo dice.
            }
            record("search_hit:" + std::to_string(i));
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s\n%s", hit.detail.c_str(),
                                  SitioTexto(hit.where).c_str());
            }
        }
        ImGui::EndListBox();
    }
}

void ProblemsPanel::drawReferences(const MapDocument* document) {
    ImGui::TextUnformatted("Buscar usos de:");

    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 12);
    if (ImGui::Combo("##ref_kind", &m_refKindIndex,
                     kRefKindNames, IM_ARRAYSIZE(kRefKindNames))) {
        m_refNameList.clear();
        m_refNameListIndex = -1;
        m_refResults.clear();
        if (document != nullptr) {
            m_refNameList = CollectReferenceNames(*document,
                                                   KindFromIndex(m_refKindIndex));
        }
    }
    record("ref_kind");

    if (document == nullptr) {
        ImGui::TextWrapped("Abre un mapa para buscar referencias.");
        return;
    }

    // Refrescar la lista de nombres si el documento cambio.
    if (m_refNameList.empty() && m_refKindIndex >= 0) {
        m_refNameList = CollectReferenceNames(*document,
                                               KindFromIndex(m_refKindIndex));
    }

    // Combo de nombres conocidos + texto libre.
    ImGui::SetNextItemWidth(-1);
    if (!m_refNameList.empty()) {
        std::vector<const char*> items;
        items.reserve(m_refNameList.size());
        for (const auto& n : m_refNameList) items.push_back(n.c_str());
        if (ImGui::Combo("##ref_name_list", &m_refNameListIndex,
                         items.data(), static_cast<int>(items.size()))) {
            if (m_refNameListIndex >= 0 &&
                m_refNameListIndex < static_cast<int>(m_refNameList.size())) {
                std::strncpy(m_refName, m_refNameList[m_refNameListIndex].c_str(),
                             sizeof(m_refName) - 1);
                m_refName[sizeof(m_refName) - 1] = '\0';
                m_refResults = FindReferences(*document,
                                               KindFromIndex(m_refKindIndex),
                                               m_refName);
            }
        }
        record("ref_name_list");
    }

    const bool nameChanged = ImGui::InputText("##ref_name", m_refName,
                                               sizeof(m_refName));
    record("ref_name");

    if (ImGui::Button("Buscar") || nameChanged) {
        m_refResults = FindReferences(*document,
                                       KindFromIndex(m_refKindIndex),
                                       m_refName);
    }
    record("ref_search");

    ImGui::Text("%zu uso%s", m_refResults.size(),
                m_refResults.size() == 1 ? "" : "s");

    const float height = ImGui::GetTextLineHeightWithSpacing() *
                         std::min<std::size_t>(m_refResults.size(), 10);
    if (ImGui::BeginListBox("##ref_results", {-1, height})) {
        for (std::size_t i = 0; i < m_refResults.size(); ++i) {
            const auto& hit = m_refResults[i];
            char label[256];
            std::snprintf(label, sizeof(label), "%s — %s",
                          hit.eventName.c_str(), hit.detail.c_str());
            if (ImGui::Selectable(label)) {
                // El editor decide que hacer con la seleccion.
            }
            record("ref_hit:" + std::to_string(i));
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", SitioTexto(hit.where).c_str());
            }
        }
        ImGui::EndListBox();
    }
}

}  // namespace Editor
