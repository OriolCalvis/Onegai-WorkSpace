// H4-02 / E-11 — ver editor/include/ProjectPanel.h.
#include "ProjectPanel.h"

#include "EditorIconos.h"
#include "Editor/EditorAyuda.h"
#include "imgui.h"

#include <algorithm>
#include <cctype>

namespace Editor {
namespace {
using Mode = ProjectPanelState::Mode;
using Kind = ProjectPanelState::Effect::Kind;

std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

const char* title(Mode mode) {
    switch (mode) {
        case Mode::NewProject:
            return "Nuevo proyecto###proyecto_formulario";
        case Mode::NewMap:
            return "Nuevo mapa###proyecto_formulario";
        case Mode::DuplicateMap:
            return "Duplicar mapa###proyecto_formulario";
        case Mode::RenameMap:
            return "Renombrar mapa###proyecto_formulario";
        case Mode::RemoveMap:
            return "Quitar mapa del proyecto###proyecto_formulario";
        case Mode::Title:
            return "Título del juego###proyecto_formulario";
        case Mode::Entry:
            return "Mapa de inicio###proyecto_formulario";
        case Mode::None:
            break;
    }
    return "###proyecto_formulario";
}

const char* confirmLabel(Mode mode) {
    switch (mode) {
        case Mode::NewProject:
        case Mode::NewMap:
            return "Crear";
        case Mode::DuplicateMap:
            return "Duplicar";
        case Mode::RenameMap:
            return "Renombrar";
        case Mode::RemoveMap:
            return "Quitar";
        case Mode::Title:
            return "Guardar";
        case Mode::Entry:
            return "Empezar aquí";
        case Mode::None:
            break;
    }
    return "Aceptar";
}

std::string doneMessage(Mode mode, const std::string& text, const std::string& mapName) {
    switch (mode) {
        case Mode::NewProject:
            return "Proyecto «" + text + "» creado. Ahora dale su primer mapa.";
        case Mode::NewMap:
            return "Mapa «" + text + "» creado.";
        case Mode::DuplicateMap:
            return "Copia «" + text + "» creada.";
        case Mode::RenameMap:
            return "Mapa renombrado a «" + text + "».";
        case Mode::RemoveMap:
            return "«" + mapName + "» ya no está en el proyecto; sus ficheros siguen en disco.";
        case Mode::Title:
            return "Título del juego guardado.";
        case Mode::Entry:
            return "El juego empezará en «" + mapName + "».";
        case Mode::None:
            break;
    }
    return "Listo";
}

const ProjectMap* findMap(const std::vector<ProjectMap>& maps, const std::string& level) {
    const auto it = std::find_if(maps.begin(), maps.end(),
                                 [&](const ProjectMap& m) { return m.level == level; });
    return it == maps.end() ? nullptr : &*it;
}
}  // namespace

ProjectPanel::ProjectPanel(std::string assetsRoot) : m_state(std::move(assetsRoot)) {}

void ProjectPanel::load(std::string& status) {
    const auto loaded = m_state.reload();
    if (!loaded.isOk())
        status = loaded.errorMessage();
}

Vector2 ProjectPanel::testPoint(const std::string& control) const {
    const auto it = m_points.find(control);
    return it == m_points.end() ? Vector2{} : it->second;
}

const std::string& ProjectPanel::openCanonical(const EditorDocumentSession& session) {
    const std::string path = session.hasDocument() ? session.levelPath() : std::string();
    if (path != m_openPath) {
        m_openPath = path;
        m_openCanonical = path.empty() ? std::string() : canonicalPath(path);
    }
    return m_openCanonical;
}

void ProjectPanel::record(const std::string& control) {
    const auto a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
    m_points[control] = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
}

void ProjectPanel::draw(EditorDocumentSession& session, std::string& status, bool blocked,
                        const std::string& openProject) {
    ImGui::Begin("Proyecto");
    ImGui::BeginDisabled(blocked);
    if (ImGui::Button("Nuevo proyecto..."))
        m_state.begin(Mode::NewProject);
    record("nuevo_proyecto");
    EditorIconos::tooltip("Nuevo proyecto", "", ayudaDeControl("proyecto.nuevo_proyecto"));
    ImGui::EndDisabled();
    drawOpenMapActions(session, blocked);
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##map_search", "Buscar mapa...", m_search.data(), m_search.size());
    record("search");
    EditorIconos::tooltip("Buscar mapa", "", ayudaDeControl("proyecto.buscar"));
    ImGui::Separator();
    drawTree(session, status, blocked, openProject);
    ImGui::End();
    drawForm(session, status);
}

void ProjectPanel::drawTree(EditorDocumentSession& session, std::string& status, bool blocked,
                            const std::string& openProject) {
    const std::string filter = lower(m_search.data());
    ImGui::BeginDisabled(blocked);
    for (const auto& project : m_state.projects()) {
        ImGui::PushID(project.id.c_str());
        // Se despliega solo el de --project al arrancar y el ultimo creado o tocado.
        const std::string& focus =
            !m_state.focusProject().empty() ? m_state.focusProject() : openProject;
        if (project.id == focus && m_unfolded != focus) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            m_unfolded = focus;
        }
        // Y, la primera vez, el que tiene el mapa abierto.
        const auto& maps = m_state.maps(project.id);
        const bool hasOpenMap = session.hasDocument() &&
                                std::any_of(maps.begin(), maps.end(), [&](const ProjectMap& m) {
                                    return m.canonical == openCanonical(session);
                                });
        if (hasOpenMap)
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        const bool open = ImGui::TreeNode(project.name.c_str());
        record("proyecto:" + project.id);
        if (ImGui::BeginPopupContextItem("menu_proyecto")) {
            if (ImGui::MenuItem("Nuevo mapa..."))
                m_state.begin(Mode::NewMap, project.id);
            if (ImGui::MenuItem("Título del juego..."))
                m_state.begin(Mode::Title, project.id);
            ImGui::EndPopup();
        }
        if (open) {
            // El primer hijo del proyecto: asi no tapa nombres largos.
            if (ImGui::SmallButton("+ Nuevo mapa"))
                m_state.begin(Mode::NewMap, project.id);
            record("nuevo_mapa:" + project.id);
            EditorIconos::tooltip("Nuevo mapa", "", ayudaDeControl("proyecto.nuevo_mapa"));
            for (const auto& map : maps) {
                if (!filter.empty() && lower(map.name).find(filter) == std::string::npos &&
                    lower(map.level).find(filter) == std::string::npos)
                    continue;
                ImGui::PushID(map.level.c_str());
                const std::string label = map.name + (map.entry ? "  (inicio)" : "") +
                                          (map.readable ? "" : "  (no se puede leer)");
                const bool current =
                    session.hasDocument() && openCanonical(session) == map.canonical;
                if (ImGui::Selectable(label.c_str(), current)) {
                    const auto opened =
                        session.request(EditorDocumentSession::Action::Open, map.path);
                    if (!opened.isOk())
                        status = opened.errorMessage();
                    else if (opened.value())
                        status = "Listo";
                }
                record("mapa:" + map.level);
                EditorIconos::tooltip(map.level.c_str(), "", ayudaDeControl("proyecto.mapa"));
                if (ImGui::BeginPopupContextItem("menu_mapa")) {
                    if (ImGui::MenuItem("Duplicar..."))
                        m_state.begin(Mode::DuplicateMap, project.id, map.level);
                    if (ImGui::MenuItem("Renombrar..."))
                        m_state.begin(Mode::RenameMap, project.id, map.level);
                    if (ImGui::MenuItem("Empezar aquí", nullptr, false, !map.entry))
                        m_state.begin(Mode::Entry, project.id, map.level);
                    if (ImGui::MenuItem("Quitar del proyecto...", nullptr, false, !map.entry))
                        m_state.begin(Mode::RemoveMap, project.id, map.level);
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            if (maps.empty())
                ImGui::TextDisabled("Sin mapas todavía.");
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    if (m_state.projects().empty())
        ImGui::TextWrapped("No hay proyectos. Crea uno con «Nuevo proyecto».");
    ImGui::EndDisabled();
}

// Lo mismo que el menu del clic derecho, a la vista: el mapa abierto se puede
// duplicar, renombrar, hacer de inicio o quitar sin saber que hay un menu.
void ProjectPanel::drawOpenMapActions(const EditorDocumentSession& session, bool blocked) {
    if (!session.hasDocument())
        return;
    for (const auto& project : m_state.projects()) {
        const auto& maps = m_state.maps(project.id);
        const auto it = std::find_if(maps.begin(), maps.end(), [&](const ProjectMap& m) {
            return m.canonical == openCanonical(session);
        });
        if (it == maps.end())
            continue;
        ImGui::SeparatorText("Mapa abierto");
        ImGui::TextWrapped("%s  ·  %s", it->name.c_str(), project.name.c_str());
        ImGui::BeginDisabled(blocked);
        const struct {
            const char* label;
            const char* control;
            Mode mode;
            bool enabled;
        } buttons[] = {
            {"Duplicar", "abierto.duplicar", Mode::DuplicateMap, true},
            {"Renombrar", "abierto.renombrar", Mode::RenameMap, true},
            {"Empezar aquí", "abierto.inicio", Mode::Entry, !it->entry},
            {"Quitar", "abierto.quitar", Mode::RemoveMap, !it->entry},
        };
        // Los cuatro botones no caben en un panel estrecho: pasan a la linea
        // siguiente cuando el proximo no cabe (el patron de ImGui para fluir).
        const ImGuiStyle& style = ImGui::GetStyle();
        const float right = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        for (const auto& button : buttons) {
            const float width = ImGui::CalcTextSize(button.label).x + style.FramePadding.x * 2;
            if (&button != &buttons[0] &&
                ImGui::GetItemRectMax().x + style.ItemSpacing.x + width < right)
                ImGui::SameLine();
            ImGui::BeginDisabled(!button.enabled);
            if (ImGui::Button(button.label))
                m_state.begin(button.mode, project.id, it->level);
            ImGui::EndDisabled();
            record(button.control);
            EditorIconos::tooltip(button.label, "",
                                  ayudaDeControl(std::string("proyecto.") + button.control));
        }
        ImGui::EndDisabled();
        return;
    }
}

void ProjectPanel::drawForm(EditorDocumentSession& session, std::string& status) {
    auto& form = m_state.form();
    if (form.mode == Mode::None) {
        m_boundForm.clear();
        return;
    }
    const std::string key =
        std::to_string(static_cast<int>(form.mode)) + "|" + form.project + "|" + form.level;
    if (key != m_boundForm) {
        m_boundForm = key;
        m_text.assign(std::max<std::size_t>(160, form.text.size() + 1), '\0');
        std::copy(form.text.begin(), form.text.end(), m_text.begin());
        ImGui::OpenPopup("###proyecto_formulario");
    }
    const auto* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, {0.5f, 0.5f});
    if (!ImGui::BeginPopupModal(title(form.mode), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        m_state.cancel();  // cerrado desde fuera (Esc)
        return;
    }
    const Project* project = m_state.project(form.project);
    const ProjectMap* map = findMap(m_state.maps(form.project), form.level);
    const std::string mapName = map ? map->name : form.level;
    bool enter = false;
    const bool hasText = form.mode == Mode::NewProject || form.mode == Mode::NewMap ||
                         form.mode == Mode::DuplicateMap || form.mode == Mode::RenameMap ||
                         form.mode == Mode::Title;
    if (form.mode == Mode::NewMap || form.mode == Mode::Title)
        ImGui::TextUnformatted(("Proyecto: " + (project ? project->name : form.project)).c_str());
    if (form.mode == Mode::DuplicateMap || form.mode == Mode::RenameMap)
        ImGui::TextUnformatted(("Mapa: " + mapName).c_str());
    if (hasText) {
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere();
        ImGui::SetNextItemWidth(360 * ImGui::GetFontSize() / 16.0f);
        enter = ImGui::InputTextWithHint(
            "##texto", form.mode == Mode::Title ? "Vacío: el nombre del proyecto" : "Nombre",
            m_text.data(), m_text.size(), ImGuiInputTextFlags_EnterReturnsTrue);
        record("form.texto");
        form.text = m_text.data();
        EditorIconos::tooltip(
            "Nombre", "",
            ayudaDeControl(form.mode == Mode::Title ? "proyecto.titulo" : "proyecto.nombre"));
    }
    if (form.mode == Mode::NewProject || form.mode == Mode::NewMap ||
        form.mode == Mode::DuplicateMap) {
        if (form.text != m_previewFor) {
            m_previewFor = form.text;
            m_preview = m_state.previewFileId();
        }
        ImGui::TextDisabled(
            "%s",
            m_preview.empty()
                ? "Escribe un nombre con alguna letra o cifra."
                : (std::string(form.mode == Mode::NewProject ? "Id: " : "Fichero: ") + m_preview)
                      .c_str());
    }
    if (form.mode == Mode::NewMap) {
        ImGui::SetNextItemWidth(120 * ImGui::GetFontSize() / 16.0f);
        ImGui::InputInt("Ancho", &form.width);
        record("form.ancho");
        ImGui::SetNextItemWidth(120 * ImGui::GetFontSize() / 16.0f);
        ImGui::InputInt("Alto", &form.height);
        record("form.alto");
        form.width = std::clamp(form.width, ProjectMaps::kMinSide, ProjectMaps::kMaxSide);
        form.height = std::clamp(form.height, ProjectMaps::kMinSide, ProjectMaps::kMaxSide);
        EditorIconos::tooltip("Tamaño", "", ayudaDeControl("proyecto.tamano"));
        const auto& tilesets = m_state.tilesets();
        if (tilesets.empty())
            ImGui::TextWrapped("No hay ningún mapa del que copiar un tileset.");
        else {
            form.tileset = std::clamp(form.tileset, 0, static_cast<int>(tilesets.size()) - 1);
            ImGui::SetNextItemWidth(360 * ImGui::GetFontSize() / 16.0f);
            if (ImGui::BeginCombo("Tileset", tilesets[form.tileset].name.c_str())) {
                for (int i = 0; i < static_cast<int>(tilesets.size()); ++i) {
                    if (ImGui::Selectable(tilesets[i].name.c_str(), i == form.tileset))
                        form.tileset = i;
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Sale de %s", tilesets[i].source.c_str());
                }
                ImGui::EndCombo();
            }
            record("form.tileset");
            EditorIconos::tooltip("Tileset", "", ayudaDeControl("proyecto.tileset"));
        }
    }
    // Un modal que se ajusta a su contenido no puede envolver el texto contra
    // su propio ancho (se encogeria hasta una columna): el ancho lo fija esto.
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 380 * ImGui::GetFontSize() / 16.0f);
    if (form.mode == Mode::RemoveMap)
        ImGui::TextWrapped(
            "«%s» sale del proyecto. Sus ficheros se quedan en disco: si te equivocas, se puede "
            "volver a añadir.",
            mapName.c_str());
    if (form.mode == Mode::Entry)
        ImGui::TextWrapped(
            "El juego empezará en «%s». Ahora empieza en: %s.", mapName.c_str(),
            project && !project->entry.empty() ? project->entry.c_str() : "ninguna parte");
    if (!form.error.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 0.45f, 0.4f, 1.0f});
        ImGui::TextWrapped("%s", form.error.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::PopTextWrapPos();
    if (ImGui::Button(confirmLabel(form.mode)) || enter) {
        const Mode mode = form.mode;
        const std::string text = form.text;
        const auto result = m_state.confirm(session);
        if (result.isOk()) {
            status = doneMessage(mode, text, mapName);
            const auto& effect = result.value();
            Result<bool> applied = Result<bool>::Ok(true);
            if (effect.kind == Kind::Open || effect.kind == Kind::Reload)
                applied = session.request(EditorDocumentSession::Action::Open, effect.path);
            if (effect.kind == Kind::Close)
                applied = session.request(EditorDocumentSession::Action::Close);
            if (!applied.isOk())
                status = applied.errorMessage();
            if (!m_state.formOpen())
                ImGui::CloseCurrentPopup();
        }
    }
    record("form.confirmar");
    ImGui::SameLine();
    if (ImGui::Button("Cancelar")) {
        m_state.cancel();
        ImGui::CloseCurrentPopup();
    }
    record("form.cancelar");
    ImGui::EndPopup();
}

}  // namespace Editor
