// EST-26 (relevo A) — Implementacion. Ver la cabecera.

#include "EditorGui.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace Editor {

GuiContext::GuiContext(GLFWwindow* window) {
    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    platform = ImGui_ImplGlfw_InitForOpenGL(window, true);
    renderer = platform && ImGui_ImplOpenGL3_Init("#version 330 core");
    if (!renderer) { shutdown(); throw std::runtime_error("No se pudo iniciar ImGui"); }
}

void GuiContext::shutdown() {
    if (renderer) ImGui_ImplOpenGL3_Shutdown();
    if (platform) ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

GuiContext::~GuiContext() { shutdown(); }

// Nombres propios del contrato de acciones, nunca nombres de depuración de ImGui.
ImGuiKey shortcutKey(const char* label) {
    const std::string key(label);
    if (key.size() == 1 && key[0] >= 'A' && key[0] <= 'Z')
        return static_cast<ImGuiKey>(ImGuiKey_A + key[0] - 'A');
    if (key.size() == 1 && key[0] >= '0' && key[0] <= '9')
        return static_cast<ImGuiKey>(ImGuiKey_0 + key[0] - '0');
    if (key == "Home") return ImGuiKey_Home;
    if (key.size() >= 2 && key[0] == 'F') {
        char* end = nullptr;
        const long number = std::strtol(key.c_str() + 1, &end, 10);
        if (*end == '\0' && number >= 1 && number <= 12)
            return static_cast<ImGuiKey>(ImGuiKey_F1 + number - 1);
    }
    return ImGuiKey_None;
}

void defaultLayout(ImGuiID dock, ImVec2 size) {
    if (ImGui::DockBuilderGetNode(dock)) return;
    ImGui::DockBuilderAddNode(dock, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dock, size);
    auto center = dock;
    const auto& style = ImGui::GetStyle();
    const float toolWidth = ImGui::CalcTextSize("Borrar objeto").x + ImGui::GetFrameHeight() + style.ItemInnerSpacing.x + style.CellPadding.x * 2;
    const float leftWidth = toolWidth * 2 + style.WindowPadding.x * 2 + style.ScrollbarSize;
    const float leftFraction = std::clamp(leftWidth / size.x, 0.20f, 0.45f);
    const auto left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, leftFraction, nullptr, &center);
    const float inspectorWidth = ImGui::CalcTextSize("Inicio: 000, 000").x + style.WindowPadding.x * 2 + style.ScrollbarSize;
    const float rightFraction = std::clamp(inspectorWidth / (size.x * (1 - leftFraction)), 0.27f, 0.50f);
    const auto right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, rightFraction, nullptr, &center);
    auto upper = left;
    const auto lower = ImGui::DockBuilderSplitNode(upper, ImGuiDir_Down, 0.50f, nullptr, &upper);
    ImGui::DockBuilderDockWindow("Proyecto", upper);
    ImGui::DockBuilderDockWindow("Herramientas", lower);
    // E-10: mirar una capa no debe ocultar la seleccion ni el estado del
    // Inspector. En ventanas bajas conservamos las pestanas: dos paneles
    // demasiado pequenos solo esconderian los controles tras el scroll.
    const float lineHeight = ImGui::GetTextLineHeightWithSpacing();
    const float minPanelHeight = 9 * lineHeight + 2 * style.WindowPadding.y;
    if (size.y >= 2 * minPanelHeight) {
        auto inspector = right;
        const float minFraction = minPanelHeight / size.y;
        const float layersFraction = std::clamp(0.55f, minFraction, 1.0f - minFraction);
        const auto layers = ImGui::DockBuilderSplitNode(inspector, ImGuiDir_Down,
                                                        layersFraction, nullptr, &inspector);
        ImGui::DockBuilderDockWindow("Inspector", inspector);
        ImGui::DockBuilderDockWindow("Capas", layers);
    } else {
        ImGui::DockBuilderDockWindow("Inspector", right);
        ImGui::DockBuilderDockWindow("Capas", right);
    }
    ImGui::DockBuilderDockWindow("Mapa", center);
    ImGui::DockBuilderFinish(dock);
}

}  // namespace Editor
