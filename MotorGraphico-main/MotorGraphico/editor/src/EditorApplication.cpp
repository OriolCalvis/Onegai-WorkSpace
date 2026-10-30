// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-10  (fase E, 6 j) - Espai de treball visual i controls coherents
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "EditorApplication.h"
#include "EditorGui.h"
#include "EditorOpciones.h"
#include "EditorVistaDeMapa.h"
#include "InspectorPanel.h"
#include "LayerPanel.h"
#include "ProjectPanel.h"
#include "ToolsPanel.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Core/Resources/ShaderManager.h"
#include "Core/Resources/Texture.h"
#include "Core/Resources/TextureAtlas.h"
#include "Core/Resources/TextureManager.h"
#include "Editor/EditorDocumentSession.h"
#include "Editor/EditorActions.h"
#include "Editor/EditorAyuda.h"
#include "Engine/Window.h"
#include "Project/ProjectCatalog.h"
#include "Project/ProjectIndex.h"
#include "Render/Camera.h"
#include "Render/Framebuffer.h"
#include "Render/IsometricRenderer.h"
#include "Runtime/RuntimeTileMapBuilder.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace Editor {
namespace {
using Action = EditorDocumentSession::Action;
using Choice = EditorDocumentSession::Choice;


}  // namespace

int EditorApplication::run(int argc, char** argv) {
    // EST-26 — la linea de comandos vive en EditorOpciones, sin ventana y
    // con prueba propia. Aqui solo se traduce su Result a la frontera de
    // excepciones que monta main.cpp (EST-11): el mensaje que llega al log
    // es ahora el que dice que opcion falla y por que, no «stoi: no
    // conversion».
    const auto analizadas = AnalizaArgumentos(argc, argv);
    if (!analizadas.isOk()) throw std::runtime_error(analizadas.errorMessage());
    const EditorOpciones& opciones = analizadas.value();
    if (opciones.ayuda) {
        std::cout << AyudaDeUso();
        return 0;
    }
    // Copias locales con el nombre de siempre: el resto de run() se queda
    // exactamente como estaba, y initial y projectId siguen siendo
    // modificables porque mas abajo se resuelven contra el proyecto.
    int frames = opciones.frames;
    const bool smoke = opciones.smoke;
    const float uiScale = opciones.uiScale;
    std::string initial = opciones.initial;
    std::string projectId = opciones.projectId;
    const std::string screenshot = opciones.screenshot;
    // Solo el harness crea esta marca dentro de su directorio temporal.
    if (smoke && (frames < 140 || !std::filesystem::exists(".motor-editor-smoke")))
        throw std::runtime_error("La prueba de edición requiere un workspace temporal del harness");

    Window window(static_cast<int>(1280 * uiScale), static_cast<int>(720 * uiScale), "MotorGraphico — Editor");
    GuiContext gui(window.handle());
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
    std::string iniPath;
    if (frames >= 0) io.IniFilename = nullptr;
    else {
        if (const char* config = std::getenv("XDG_CONFIG_HOME")) iniPath = (std::filesystem::path(config) / "MotorGraphico/editor.ini").string();
        else if (const char* home = std::getenv("HOME")) iniPath = (std::filesystem::path(home) / ".config/MotorGraphico/editor.ini").string();
        std::error_code ec;
        if (!iniPath.empty()) std::filesystem::create_directories(std::filesystem::path(iniPath).parent_path(), ec);
        io.IniFilename = !iniPath.empty() && !ec ? iniPath.c_str() : nullptr;
    }
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 4; style.FrameRounding = 3;
    style.WindowPadding = {12, 12}; style.ItemSpacing = {8, 8};
    style.Colors[ImGuiCol_WindowBg] = {0.10f, 0.115f, 0.14f, 1};
    style.Colors[ImGuiCol_Header] = {0.18f, 0.32f, 0.37f, 1};
    style.Colors[ImGuiCol_Button] = {0.17f, 0.30f, 0.35f, 1};
    style.ScaleAllSizes(uiScale);
    ImFontConfig font;
    int framebufferWidth = 0, framebufferHeight = 0, logicalWidth = 0, logicalHeight = 0;
    glfwGetFramebufferSize(window.handle(), &framebufferWidth, &framebufferHeight);
    glfwGetWindowSize(window.handle(), &logicalWidth, &logicalHeight);
    const float pixelRatio = std::max(1.0f, static_cast<float>(framebufferWidth) / std::max(1, logicalWidth));
    font.SizePixels = 16 * uiScale * pixelRatio;
    io.Fonts->AddFontDefault(&font);
    io.FontGlobalScale = 1.0f / pixelRatio;

    ShaderManager shaders;
    auto shader = shaders.load("editor_sprite", "assets/shaders/sprite");
    if (!shader.isOk()) throw std::runtime_error(shader.errorMessage());
    std::string status;
    // H4-02 — El arbol de proyectos y la gestion de mapas viven en su panel;
    // `projects` es su vector, asi que ve los proyectos y mapas que se crean.
    ProjectPanel projectPanel;
    projectPanel.load(status);
    const auto& projects = projectPanel.projects();
    if (!projectId.empty()) {
        const auto it = std::find_if(projects.begin(), projects.end(), [&](const auto& p) { return p.id == projectId; });
        if (it == projects.end() || it->levels.empty()) throw std::runtime_error("Proyecto sin mapas: " + projectId);
        initial = (std::filesystem::path("assets") / it->settings.levelsDirectory / it->levels.front()).generic_string();
    }
    EditorDocumentSession session;
    auto report = [&](const Result<bool>& result) {
        if (!result.isOk()) status = result.errorMessage();
        else if (result.value()) status = "Listo";
    };
    if (!initial.empty()) {
        const auto result = session.request(Action::Open, initial);
        if (!result.isOk()) throw std::runtime_error(result.errorMessage());
    }
    MapView map;
    LayerPanel layerPanel;
    int selectedGid = 1;
    EditorActions actions;
    GridCoord selected{-1, -1};
    char objectSearch[128] = {};
    std::string selectedObject, catalogPath;
    bool smokePainted = false, smokeSaved = false, smokeRendered = false;
    ids::LayerId smokeLayer;
    ImVec2 smokePointer{-100, -100};
    ImVec2 smokeMapPoint{}, smokeSecondPoint{}, smokeCancelPoint{}, smokeFillCell{};
    // EDITOR-UX-01 — Herramientas, ayuda e Inspector viven en sus paneles.
    ToolsPanel toolsPanel;
    InspectorPanel inspectorPanel(toolsPanel.iconos());
    auto panelPoint = [&](const char* control) { const auto p = toolsPanel.testPoint(control); return ImVec2{p.x, p.y}; };
    auto projectPoint = [&](const char* control) { const auto p = projectPanel.testPoint(control); return ImVec2{p.x, p.y}; };
    for (int frame = 0; !session.quitReady() && (frames < 0 || frame < frames); ++frame) {
        window.pollEvents();
        if (window.shouldClose()) {
            glfwSetWindowShouldClose(window.handle(), GLFW_FALSE);
            if (!session.confirmationPending() && !layerPanel.modalOpen()) report(actions.execute(EditorAction::Quit, session));
        }
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame();
        if (smoke) {
            // El backend puede encolar la posición del ratón físico entre
            // pulsación y suelta. El harness controla su propio flujo de input.
            io.ClearEventsQueue();
            if (frame == 2 || frame == 6 || frame == 11 || frame == 14)
                smokePointer = frame == 2 ? smokeMapPoint : (frame == 6 ? smokeCancelPoint : (frame == 11 ? smokeSecondPoint : projectPoint("search")));
            if (frame == 20 || frame == 22 || frame == 24 || frame == 34)
                smokePointer = frame == 22 ? panelPoint("fill") : (frame == 24 ? smokeFillCell : smokeMapPoint);
            if (frame == 44) smokePointer = smokeMapPoint;
            if (frame == 48) smokePointer = panelPoint("objectSearch");
            if (frame == 52) smokePointer = panelPoint("objectRow");
            if (frame == 54 || frame == 58 || frame == 64 || frame == 76) smokePointer = smokeSecondPoint;
            if (frame == 82 || frame == 84 || frame == 90 || frame == 92 || frame == 94 || frame == 100 ||
                frame == 110 || frame == 118 || frame == 122 || frame == 124 || frame == 128) {
                const std::string control = frame == 82 ? "add" : frame == 84 ? "name" : frame == 90 ? "rename" :
                    frame == 92 ? "visible" : (frame == 94 || frame == 110) ? "locked" :
                    frame == 100 ? "layer:" + smokeLayer.value() : (frame == 118 || frame == 124) ? "remove" :
                    frame == 122 ? "cancel" : "confirm";
                const auto point = layerPanel.testPoint(control); smokePointer = {point.x, point.y};
            }
            if (frame == 102 || frame == 106 || frame == 112) smokePointer = smokeSecondPoint;
            // H4-02: «+ Mapa» -> nombre -> Crear -> pintar -> Ctrl+S -> reabrir.
            if (frame == 140 || frame == 145 || frame == 150)
                smokePointer = frame == 140 ? projectPoint("nuevo_mapa:smoke") : frame == 145 ? projectPoint("form.confirmar") : smokeMapPoint;
            io.AddMousePosEvent(smokePointer.x, smokePointer.y);
            if (frame == 140 || frame == 145 || frame == 150 || frame == 141 || frame == 146 || frame == 151)
                io.AddMouseButtonEvent(ImGuiMouseButton_Left, frame == 140 || frame == 145 || frame == 150);
            if (frame == 143) io.AddInputCharactersUTF8("Cueva del eco");
            if (frame == 152 || frame == 153) {
                io.AddKeyEvent(io.ConfigMacOSXBehaviors ? ImGuiMod_Super : ImGuiMod_Ctrl, frame == 152);
                io.AddKeyEvent(ImGuiKey_S, frame == 152);
            }
            if (frame == 2 || frame == 6 || frame == 11 || frame == 14) {
                io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
            }
            if (frame == 3 || frame == 7 || frame == 12 || frame == 15) io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
            if (frame == 9 || frame == 18) {
                io.AddKeyEvent(io.ConfigMacOSXBehaviors ? ImGuiMod_Super : ImGuiMod_Ctrl, true);
                io.AddKeyEvent(frame == 9 ? ImGuiKey_S : ImGuiKey_Z, true);
            }
            if (frame == 10 || frame == 19) {
                io.AddKeyEvent(io.ConfigMacOSXBehaviors ? ImGuiMod_Super : ImGuiMod_Ctrl, false);
                io.AddKeyEvent(frame == 10 ? ImGuiKey_S : ImGuiKey_Z, false);
            }
            if (frame == 16) { io.AddKeyEvent(ImGuiKey_G, true); io.AddInputCharacter('g'); }
            if (frame == 17) io.AddKeyEvent(ImGuiKey_G, false);
            if (frame == 20 || frame == 22 || frame == 24 || frame == 34) {
                io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
            }
            if (frame == 21 || frame == 23 || frame == 25 || frame == 35) io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
            if (frame == 27 || frame == 30 || frame == 36) {
                io.AddKeyEvent(io.ConfigMacOSXBehaviors ? ImGuiMod_Super : ImGuiMod_Ctrl, true);
                if (frame == 30) io.AddKeyEvent(ImGuiMod_Shift, true);
                io.AddKeyEvent(frame == 36 ? ImGuiKey_S : ImGuiKey_Z, true);
            }
            if (frame == 28 || frame == 31 || frame == 37) {
                io.AddKeyEvent(frame == 37 ? ImGuiKey_S : ImGuiKey_Z, false);
                io.AddKeyEvent(ImGuiMod_Shift, false);
                io.AddKeyEvent(io.ConfigMacOSXBehaviors ? ImGuiMod_Super : ImGuiMod_Ctrl, false);
            }
            if (frame == 32 || frame == 33) io.AddKeyEvent(ImGuiKey_E, frame == 32);
            if (frame == 40 || frame == 41 || frame == 42 || frame == 43) io.AddKeyEvent(ImGuiKey_F1, frame == 40 || frame == 42);
            if (frame == 44 || frame == 48 || frame == 52 || frame == 54 || frame == 58 || frame == 76) io.AddMouseButtonEvent(ImGuiMouseButton_Left,true);
            if (frame == 45 || frame == 49 || frame == 53 || frame == 55 || frame == 59 || frame == 77) io.AddMouseButtonEvent(ImGuiMouseButton_Left,false);
            if (frame == 46 || frame == 47 || frame == 50 || frame == 51 || frame == 62 || frame == 63)
                io.AddKeyEvent(ImGuiKey_O,frame == 46 || frame == 50 || frame == 62);
            if (frame == 50) io.AddInputCharactersUTF8("pocion");
            if (frame == 56 || frame == 57) io.AddKeyEvent(ImGuiKey_X,frame == 56);
            if (frame == 64 || frame == 65) io.AddMouseButtonEvent(ImGuiMouseButton_Right,frame == 64);
            if (frame == 60 || frame == 61 || frame == 66 || frame == 67 || frame == 68 || frame == 69) {
                const bool down = frame == 60 || frame == 66 || frame == 68;
                io.AddKeyEvent(io.ConfigMacOSXBehaviors ? ImGuiMod_Super : ImGuiMod_Ctrl,down);
                io.AddKeyEvent(frame >= 68 ? ImGuiKey_S : ImGuiKey_Z,down);
            }
            // El modal recién abierto necesita terminar su frame de aparición antes del clic de confirmación.
            if (frame == 80 || frame == 81 || frame == 108 || frame == 109 || frame == 116 || frame == 117 || frame == 136 || frame == 137)
                io.AddKeyEvent(ImGuiKey_F2,frame % 2 == 0);
            if (frame == 104 || frame == 105) io.AddKeyEvent(ImGuiKey_B,frame == 104);
            if (frame == 82 || frame == 84 || frame == 90 || frame == 92 || frame == 94 || frame == 100 || frame == 102 ||
                frame == 106 || frame == 110 || frame == 112 || frame == 118 || frame == 122 || frame == 124 || frame == 128)
                io.AddMouseButtonEvent(ImGuiMouseButton_Left,true);
            if (frame == 83 || frame == 85 || frame == 91 || frame == 93 || frame == 95 || frame == 101 || frame == 103 ||
                frame == 107 || frame == 111 || frame == 113 || frame == 119 || frame == 123 || frame == 125 || frame == 129)
                io.AddMouseButtonEvent(ImGuiMouseButton_Left,false);
            if (frame == 86 || frame == 87 || frame == 96 || frame == 97 || frame == 120 || frame == 121 || frame == 130 || frame == 131 || frame == 132 || frame == 133) {
                io.AddKeyEvent(io.ConfigMacOSXBehaviors ? ImGuiMod_Super : ImGuiMod_Ctrl,frame % 2 == 0);
                io.AddKeyEvent(frame < 90 ? ImGuiKey_A : (frame == 120 || frame == 121 || frame == 130 || frame == 131) ? ImGuiKey_Z : ImGuiKey_S,frame % 2 == 0);
            }
            if (frame == 88) io.AddInputCharactersUTF8("Detalle ## alto");
        }
        ImGui::NewFrame();
        const auto* mainViewport = ImGui::GetMainViewport();
        const auto dock = ImGui::GetID("MotorEditorDock");
        defaultLayout(dock, mainViewport->WorkSize);
        ImGui::DockSpaceOverViewport(dock, mainViewport);
        const bool blocked = session.confirmationPending() || layerPanel.modalOpen() || projectPanel.modalOpen();
        const ToolsContext toolsContext{session, actions, map, selectedGid, selectedObject, objectSearch, sizeof(objectSearch), status, blocked, uiScale, io.ConfigMacOSXBehaviors};
        if (ImGui::BeginMainMenuBar()) {
            for (const auto& menu : {std::pair<const char*, ActionGroup>{"Archivo", ActionGroup::File},
                                    {"Editar", ActionGroup::Edit}, {"Ver", ActionGroup::View}}) {
                if (ImGui::BeginMenu(menu.first)) {
                    for (const auto& action : editorActionDefinitions()) {
                        if (action.group != menu.second) continue;
                        const auto shortcut = shortcutLabel(action, io.ConfigMacOSXBehaviors);
                        if (ImGui::MenuItem(action.label, shortcut.c_str(),
                            action.id == EditorAction::Help && actions.helpVisible(), actions.enabled(action.id, session) && !layerPanel.modalOpen()))
                            report(actions.execute(action.id, session));
                        EditorIconos::tooltip(action.label, shortcut, ayudaDeAccion(action.id).ayuda);
                    }
                    ImGui::EndMenu();
                }
            }
            ImGui::TextDisabled("  %s%s", session.hasDocument() ? session.name().c_str() : "Sin mapa", session.dirty() ? " • cambios sin guardar" : "");
            ImGui::EndMainMenuBar();
        }
        projectPanel.draw(session, status, blocked, projectId);

        // Resolver el catálogo antes de mostrar la paleta, también al abrir otro mapa.
        if (session.hasDocument()) {
            if (catalogPath != session.levelPath()) {
                selectedObject.clear(); objectSearch[0] = '\0';
                selected = {-1, -1}; catalogPath = session.levelPath();
            }
            try { map.sync(session, shader.value(), projects); }
            catch (const std::exception& e) { status = e.what(); }
        } else {
            map.path.clear(); catalogPath.clear(); selectedObject.clear();
        }
        toolsPanel.draw(toolsContext);

        ImGui::Begin("Mapa", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        // El campo de texto ya se ha procesado; no se filtran atajos al mapa.
        const ActionInput actionInput{io.WantTextInput, ImGui::IsAnyItemActive(), blocked, ImGui::IsWindowFocused()};
        std::optional<EditorAction> requestedAction;
        for (const auto& definition : editorActionDefinitions()) {
            for (const auto* label : {definition.key, definition.alternateKey}) {
                const auto key = shortcutKey(label);
                if (key == ImGuiKey_None || !ImGui::IsKeyPressed(key, false)) continue;
                // ImGui normaliza Cmd a KeyCtrl en AddKeyEvent en macOS.
                requestedAction = actions.shortcut(label, io.KeyCtrl, io.KeyShift,
                    io.KeyAlt || io.KeySuper, actionInput, session);
                if (requestedAction) break;
            }
            if (requestedAction) break; // Z figura tanto en Deshacer como en Rehacer.
        }
        if (requestedAction) report(actions.execute(*requestedAction, session));
        const auto size = ImGui::GetContentRegionAvail();
        if (session.hasDocument() && size.x > 1 && size.y > 1) {
            try {
                map.sync(session, shader.value(), projects);
                if (actions.takeCenterRequest()) map.center();
                const ImVec2 scale{std::max(0.1f, io.DisplayFramebufferScale.x), std::max(0.1f, io.DisplayFramebufferScale.y)};
                const int width = std::max(1, static_cast<int>(size.x * scale.x));
                const int height = std::max(1, static_cast<int>(size.y * scale.y));
                map.camera.update(io.DeltaTime);
                map.render(width, height);
                smokeRendered = true;
                const auto origin = ImGui::GetCursorScreenPos();
                ImGui::Image(static_cast<ImTextureID>(map.target->colorTextureID()), size, {0, 1}, {1, 0});
                if (smoke) {
                    smokeMapPoint = map.screen({1, 1}, origin, scale);
                    smokeSecondPoint = map.screen({2, 1}, origin, scale);
                    smokeFillCell = map.screen({3, 3}, origin, scale);
                }
                const auto mouse = io.MousePos;
                const ViewportInput input{origin.x, origin.y, size.x, size.y, scale.x, scale.y,
                    ImGui::IsItemHovered(), io.WantTextInput || ImGui::IsAnyItemActive(), blocked};
                if (const auto point = input.framebufferPoint(mouse.x, mouse.y)) {
                    const auto world = map.camera.screenToWorld(*point);
                    const auto cell = renderedMapCell(world, map.runtime.getTileWidth(), map.runtime.getTileHeight());
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        selected = cell;
                        report(actions.applyAt(session, cell, selectedGid, false, &map.catalog, selectedObject));
                    }
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) report(actions.applyAt(session, cell, selectedGid, true, &map.catalog, selectedObject));
                    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
                        map.camera.move({-io.MouseDelta.x * scale.x / map.camera.zoom(), -io.MouseDelta.y * scale.y / map.camera.zoom()});
                    if (io.MouseWheel != 0) map.camera.setZoom(std::clamp(map.camera.zoom() * std::pow(1.15f, io.MouseWheel), 0.15f, 4.0f));
                }
                auto* draw = ImGui::GetWindowDrawList();
                draw->PushClipRect(origin, {origin.x + size.x, origin.y + size.y}, true);
                for (const auto& object : session.state().objects()) {
                    const auto center = map.screen(object.position, origin, scale);
                    draw->AddCircleFilled(center, 4 * uiScale, IM_COL32(245, 194, 93, 255));
                    const auto* def = map.catalog.find(object.objectId);
                    if (input.hovered && !blocked && !io.WantTextInput &&
                        std::hypot(mouse.x - center.x, mouse.y - center.y) < 10 * uiScale)
                        ImGui::SetTooltip("%s", def ? def->name.c_str() : object.objectId.c_str());
                }
                const auto start = map.screen(session.state().playerStart(), origin, scale);
                draw->AddCircle(start, 10 * uiScale, IM_COL32(95, 232, 175, 255), 0, 2);
                if (selected.x >= 0 && selected.y >= 0) draw->AddCircle(map.screen(selected, origin, scale), 14 * uiScale, IM_COL32(100, 190, 255, 255), 0, 2);
                draw->PopClipRect();
            } catch (const std::exception& e) { status = e.what(); ImGui::TextWrapped("%s", status.c_str()); }
        } else ImGui::TextWrapped("Selecciona un mapa en el panel Proyecto para empezar.");
        ImGui::End();

        inspectorPanel.draw(session, map.catalog, selected, status, uiScale);

        layerPanel.draw(session, status, actions.takeLayersRequest());

        toolsPanel.drawHelp(toolsContext, mainViewport);

        if (session.confirmationPending()) ImGui::OpenPopup("Cambios sin guardar");
        ImGui::SetNextWindowPos(mainViewport->GetCenter(), ImGuiCond_Appearing, {0.5f, 0.5f});
        if (ImGui::BeginPopupModal("Cambios sin guardar", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("El mapa tiene cambios pendientes.");
            ImGui::TextUnformatted("¿Quieres guardarlos antes de continuar?");
            for (const auto& choice : {std::pair<const char*, Choice>{"Guardar", Choice::Save}, {"Descartar", Choice::Discard}, {"Cancelar", Choice::Cancel}}) {
                if (ImGui::Button(choice.first)) {
                    report(session.resolve(choice.second));
                    if (!session.confirmationPending()) ImGui::CloseCurrentPopup();
                }
                if (smoke && choice.second == Choice::Cancel) {
                    const auto a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax(); smokeCancelPoint = {(a.x+b.x)*0.5f, (a.y+b.y)*0.5f};
                }
                ImGui::SameLine();
            }
            ImGui::NewLine(); ImGui::TextWrapped("%s", status.c_str());
            ImGui::EndPopup();
        }
        // El harness inyecta ratón/teclas por ImGui, no modifica tiles directamente.
        if (smoke && frame == 1) selectedGid = 2;
        if (smoke && frame == 3) {
            smokePainted = session.dirty() && session.state().tileAt(1, 1) == 2;
            if (!smokePainted) throw std::runtime_error("El clic no pintó la celda visible");
            report(session.request(Action::Close));
        }
        if (smoke && frame == 7 && (session.confirmationPending() || !session.dirty()))
            throw std::runtime_error("El botón Cancelar no conservó el mapa");
        if (smoke && frame == 10) {
            if (session.dirty()) throw std::runtime_error("Ctrl+S no guardó el mapa");
            report(session.request(Action::Open, initial));
            smokeSaved = !session.dirty() && session.state().tileAt(1, 1) == 2;
        }
        if (smoke && frame == 12 && (!session.dirty() || session.state().tileAt(2, 1) != 2))
            throw std::runtime_error("Falta una edición deshacible antes de probar el foco");
        if (smoke && frame == 19 && (*projectPanel.search() || actions.tool() != EditorAction::Paint || !session.dirty() || session.state().tileAt(2, 1) != 2))
            throw std::runtime_error("Escribir/deshacer texto modificó el mapa");
        if (smoke && frame == 17 && (std::string(projectPanel.search()) != "g" || actions.tool() != EditorAction::Paint))
            throw std::runtime_error("La búsqueda no recibió la G escrita");
        if (smoke && frame == 19) report(actions.execute(EditorAction::Save, session));
        if (smoke && frame == 23 && actions.tool() != EditorAction::Fill)
            throw std::runtime_error("El botón Rellenar no seleccionó la herramienta");
        if (smoke && frame == 25 && (!session.dirty() || session.state().tileAt(6,4) != 2))
            throw std::runtime_error("El relleno no alcanzó la región conectada");
        if (smoke && frame == 28 && (session.dirty() || session.state().tileAt(6,4) != 1))
            throw std::runtime_error("Deshacer relleno no restauró el mapa en un paso");
        if (smoke && frame == 31 && (!session.dirty() || session.state().tileAt(6,4) != 2))
            throw std::runtime_error("Ctrl/Cmd+Shift+Z no rehizo el relleno");
        if (smoke && frame == 35 && (actions.tool() != EditorAction::Erase || session.state().tileAt(1,1) != 0 || session.state().tileAt(2,1) != 2))
            throw std::runtime_error("El atajo E no borró solo la celda seleccionada");
        if (smoke && frame == 38) {
            if (session.dirty()) throw std::runtime_error("El atajo Guardar no persistió las herramientas");
            report(session.request(Action::Open, initial));
            if (session.state().tileAt(1,1) != 0 || session.state().tileAt(6,4) != 2)
                throw std::runtime_error("Reabrir perdió el relleno o borrado");
        }
        if (smoke && frame == 41 && !actions.helpVisible()) throw std::runtime_error("F1 no abrió la ayuda");
        if (smoke && frame == 43 && actions.helpVisible()) throw std::runtime_error("F1 no cerró la ayuda");
        if (smoke && frame == 51 && (actions.tool() != EditorAction::Objects || std::string(objectSearch) != "pocion"))
            throw std::runtime_error("La búsqueda de objetos no capturó el texto/atajo O");
        if (smoke && frame == 53 && selectedObject != "pocion") throw std::runtime_error("El selector no eligió Pocion por nombre");
        if (smoke && frame == 55 && (!session.state().objectAt(2,1) || session.state().tileAt(2,1) != 2 || !session.dirty()))
            throw std::runtime_error("Colocar el objeto no respetó la baldosa");
        if (smoke && (frame == 59 || frame == 65) && (session.state().objectAt(2,1) || session.state().tileAt(2,1) != 2))
            throw std::runtime_error("X o el botón derecho no borraron solo el objeto");
        if (smoke && (frame == 61 || frame == 67) && !session.state().objectAt(2,1))
            throw std::runtime_error("Deshacer no recuperó la instancia borrada");
        if (smoke && frame == 70) {
            if (session.dirty()) throw std::runtime_error("Guardar no persistió la instancia");
            report(session.request(Action::Open,initial));
            const auto* object = session.state().objectAt(2,1);
            if (!object || object->objectId != "pocion") throw std::runtime_error("Reabrir perdió la instancia");
        }
        if (smoke && frame == 72) report(session.request(Action::Open,"assets/levels/sin_catalogo.json"));
        if (smoke && frame == 77 && (!selectedObject.empty() || map.catalog.size() || session.dirty() || session.state().objectAt(2,1)))
            throw std::runtime_error("El catálogo anterior se filtró al cambiar de mapa");
        if (smoke && frame == 83) {
            if (session.state().layerCount() != 2 || !session.dirty()) throw std::runtime_error("Nueva capa no creó un cambio deshacible");
            smokeLayer = session.activeTileLayer();
        }
        if (smoke && frame == 91 && session.tileLayers().back().name != "Detalle ## alto")
            throw std::runtime_error("El formulario no renombró la capa");
        if (smoke && frame == 95 && (session.tileLayers().back().visible || !session.tileLayers().back().locked))
            throw std::runtime_error("Visible/Bloqueada no actualizaron los metadatos");
        if (smoke && frame == 98) {
            if (session.dirty()) throw std::runtime_error("Guardar capas no completado");
            report(session.request(Action::Open,"assets/levels/sin_catalogo.json"));
            const auto layers = session.tileLayers();
            if (layers.size()!=2 || layers.back().id!=smokeLayer || layers.back().name!="Detalle ## alto" || layers.back().visible || !layers.back().locked)
                throw std::runtime_error("Reabrir perdió las propiedades de capas");
        }
        if (smoke && frame == 107 && (session.activeTileLayer()!=smokeLayer || session.state().tileAt(2,1)!=0 || session.dirty()))
            throw std::runtime_error("El bloqueo no protegió la capa activa");
        if (smoke && frame == 113 && (session.state().tileAt(2,1)!=2 || session.state().tileAt(0,2,1)!=1))
            throw std::runtime_error("Pintar no respetó la capa seleccionada");
        if (smoke && frame == 121 && (!layerPanel.modalOpen() || session.state().layerCount()!=2 || session.state().tileAt(2,1)!=2))
            throw std::runtime_error("El atajo escapó de la confirmación de borrado");
        if (smoke && frame == 123 && (layerPanel.modalOpen() || session.state().layerCount()!=2))
            throw std::runtime_error("Cancelar borrado no conservó las capas");
        if (smoke && frame == 129 && (layerPanel.modalOpen() || session.state().layerCount()!=1))
            throw std::runtime_error("Confirmar borrado no eliminó la capa");
        if (smoke && frame == 131 && (session.state().layerCount()!=2 || session.activeTileLayer()!=smokeLayer || session.state().tileAt(2,1)!=2))
            throw std::runtime_error("Deshacer no recuperó la capa y sus baldosas");
        if (smoke && frame == 134) {
            if (session.dirty()) throw std::runtime_error("El guardado final de capas falló");
            report(session.request(Action::Open,"assets/levels/sin_catalogo.json"));
            if (session.tileLayers().size()!=2 || session.tileLayers().back().id!=smokeLayer || session.state().tileAt(1,2,1)!=2)
                throw std::runtime_error("El ciclo borrar/deshacer/guardar perdió la capa");
        }
        const std::string nuevo = "assets/levels/smoke_cueva_del_eco.json";
        if (smoke && frame == 148 && (!session.hasDocument() || session.levelPath() != nuevo || session.state().width() != 16))
            throw std::runtime_error("Nuevo mapa no creó ni abrió el mapa");
        if (smoke && frame == 156) {
            if (session.dirty()) throw std::runtime_error("Guardar el mapa nuevo no se completó");
            report(session.request(Action::Open, nuevo));
            if (session.state().tileAt(1, 1) != 2 || session.name() != "Cueva del eco") throw std::runtime_error("Reabrir perdió el mapa nuevo");
        }
        ImGui::Render();
        int width, height; glfwGetFramebufferSize(window.handle(), &width, &height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); glViewport(0, 0, width, height);
        glClearColor(0.08f, 0.09f, 0.11f, 1); glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (smoke && frame == 71 && !screenshot.empty()) capture(screenshot + ".objects.ppm", width, height);
        if (frames > 0 && frame == frames - 1 && !screenshot.empty()) capture(screenshot, width, height);
        const auto error = glGetError();
        if (error != GL_NO_ERROR) throw std::runtime_error("Error OpenGL: " + std::to_string(error));
        window.swapBuffers();
    }
    if (smoke && (!smokePainted || !smokeSaved || !smokeRendered)) throw std::runtime_error("No se completó editar/guardar/reabrir");
    std::cout << "motor_editor: salida normal, GL 0" << (smoke ? "; editar/cancelar/guardar/reabrir OK; rellenar/borrar/rehacer/ayuda OK; objetos/catalogo/historial OK; capas/seleccion/bloqueo/confirmacion OK; nuevo mapa/pintar/guardar/reabrir OK" : "") << '\n';
    return 0;
}
}  // namespace Editor
