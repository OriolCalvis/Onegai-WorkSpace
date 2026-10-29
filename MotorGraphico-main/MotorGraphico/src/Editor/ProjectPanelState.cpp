// H4-02 / E-11 — ver include/Editor/ProjectPanelState.h.
#include "Editor/ProjectPanelState.h"

#include <algorithm>

#include "Project/ProjectIndex.h"

namespace Editor {
namespace {

std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

const ProjectMap* findMap(const std::vector<ProjectMap>& maps, const std::string& level) {
    const auto it = std::find_if(maps.begin(), maps.end(),
                                 [&](const ProjectMap& map) { return map.level == level; });
    return it == maps.end() ? nullptr : &*it;
}

}  // namespace

ProjectPanelState::ProjectPanelState(std::string assetsRoot) : m_root(std::move(assetsRoot)) {}

Result<bool> ProjectPanelState::reload() {
    auto index = ProjectIndex::scan(m_root);
    m_tilesets.clear();
    if (!index.isOk()) {
        m_projects.clear();
        m_maps.clear();
        return Result<bool>::Error(index.errorMessage());
    }
    m_projects = index.value().projects();
    m_maps.clear();
    for (const Project& project : m_projects)
        m_maps[project.id] = ProjectMaps::list(m_root, project);
    return Result<bool>::Ok(true);
}

const Project* ProjectPanelState::project(const std::string& id) const {
    const auto it = std::find_if(m_projects.begin(), m_projects.end(),
                                 [&](const Project& project) { return project.id == id; });
    return it == m_projects.end() ? nullptr : &*it;
}

const std::vector<ProjectMap>& ProjectPanelState::maps(const std::string& projectId) const {
    static const std::vector<ProjectMap> none;
    const auto it = m_maps.find(projectId);
    return it == m_maps.end() ? none : it->second;
}

bool ProjectPanelState::begin(Mode mode, const std::string& projectId, const std::string& level) {
    Form next;
    next.mode = mode;
    next.project = projectId;
    next.level = level;
    if (mode == Mode::None)
        return false;
    if (mode != Mode::NewProject) {
        const Project* owner = project(projectId);
        if (owner == nullptr)
            return false;
        if (mode == Mode::Title)
            next.text = owner->settings.title;
        if (mode != Mode::NewMap && mode != Mode::Title) {
            const ProjectMap* map = findMap(maps(projectId), level);
            if (map == nullptr)
                return false;
            if (mode == Mode::DuplicateMap)
                next.text = "Copia de " + map->name;
            if (mode == Mode::RenameMap)
                next.text = map->name;
        }
    }
    m_form = std::move(next);
    return true;
}

void ProjectPanelState::cancel() { m_form = Form{}; }

const std::vector<ProjectTileset>& ProjectPanelState::tilesets() {
    auto it = m_tilesets.find(m_form.project);
    if (it == m_tilesets.end())
        it = m_tilesets
                 .emplace(m_form.project, ProjectMaps::tilesets(m_root, m_projects, m_form.project))
                 .first;
    return it->second;
}

std::string ProjectPanelState::previewFileId() const {
    if (m_form.mode == Mode::NewProject) {
        const auto id = ProjectIndex::suggestId(trim(m_form.text));
        if (!id.isOk())
            return {};
        std::vector<std::string> taken;
        for (const Project& project : m_projects)
            taken.push_back(project.id);
        return ProjectIndex::uniqueId(id.value(), taken);
    }
    const Project* owner = project(m_form.project);
    if (owner == nullptr || (m_form.mode != Mode::NewMap && m_form.mode != Mode::DuplicateMap))
        return {};
    const auto id = ProjectMaps::suggestFileId(m_root, *owner, m_form.text);
    return id.isOk() ? id.value() : std::string();
}

Result<bool> ProjectPanelState::refuseIfOpenAndDirty(const EditorDocumentSession& session,
                                                     const std::string& path) const {
    if (session.hasDocument() && canonicalPath(session.levelPath()) == canonicalPath(path) &&
        (session.dirty() || session.confirmationPending()))
        return Result<bool>::Error("guarda o descarta los cambios de este mapa antes de seguir");
    return Result<bool>::Ok(true);
}

Result<ProjectPanelState::Effect> ProjectPanelState::confirm(const EditorDocumentSession& session) {
    auto result = run(session);
    if (!result.isOk())
        m_form.error = result.errorMessage();
    return result;
}

Result<ProjectPanelState::Effect> ProjectPanelState::run(const EditorDocumentSession& session) {
    using R = Result<Effect>;
    const Form form = m_form;
    const ProjectMap* map = findMap(maps(form.project), form.level);
    const std::string path = map != nullptr ? map->path : std::string();
    auto done = [&](Effect effect) {
        reload();
        m_focus = form.project;
        cancel();
        return R::Ok(std::move(effect));
    };
    switch (form.mode) {
        case Mode::None:
            return R::Error("no hay ningun formulario abierto");
        case Mode::NewProject: {
            const std::string name = trim(form.text);
            const auto base = ProjectIndex::suggestId(name);
            if (!base.isOk())
                return R::Error("ese nombre no sirve: escribe al menos una letra o un numero");
            // El prefijo sale del id, como en la pantalla de proyectos (H4-01):
            // sus tres primeras letras o cifras ("la_cueva" -> "lac_", no "la__").
            std::string prefix;
            for (const char c : base.value())
                if (c != '_' && prefix.size() < 3)
                    prefix += c;
            const auto created =
                ProjectIndex::createNamed(m_root, name, prefix + "_", "editor", "");
            if (!created.isOk())
                return R::Error(created.errorMessage());
            reload();
            m_focus = created.value().id;
            // Un proyecto sin mapas no se puede jugar: se sigue con el primero.
            begin(Mode::NewMap, created.value().id);
            return R::Ok(Effect{});
        }
        case Mode::NewMap: {
            const auto& choices = tilesets();
            if (choices.empty())
                return R::Error(
                    "no hay ningun mapa del que copiar el tileset: importa antes un TMX");
            const std::size_t index = static_cast<std::size_t>(
                std::clamp(form.tileset, 0, static_cast<int>(choices.size()) - 1));
            const auto created = ProjectMaps::create(m_root, form.project, form.text, form.width,
                                                     form.height, choices[index]);
            if (!created.isOk())
                return R::Error(created.errorMessage());
            return done(Effect{Effect::Kind::Open, created.value().path});
        }
        case Mode::DuplicateMap: {
            const auto copy = ProjectMaps::duplicate(m_root, form.project, form.level, form.text);
            if (!copy.isOk())
                return R::Error(copy.errorMessage());
            return done(Effect{Effect::Kind::Open, copy.value().path});
        }
        case Mode::RenameMap: {
            const auto clean = refuseIfOpenAndDirty(session, path);
            if (!clean.isOk())
                return R::Error(clean.errorMessage());
            const auto renamed = ProjectMaps::rename(m_root, form.project, form.level, form.text);
            if (!renamed.isOk())
                return R::Error(renamed.errorMessage());
            // El nombre vive en el nivel: el documento abierto se recarga
            // para que el proximo Guardar no escriba el viejo encima.
            const bool open =
                session.hasDocument() && canonicalPath(session.levelPath()) == canonicalPath(path);
            return done(open ? Effect{Effect::Kind::Reload, session.levelPath()} : Effect{});
        }
        case Mode::RemoveMap: {
            const auto clean = refuseIfOpenAndDirty(session, path);
            if (!clean.isOk())
                return R::Error(clean.errorMessage());
            const auto removed = ProjectMaps::remove(m_root, form.project, form.level);
            if (!removed.isOk())
                return R::Error(removed.errorMessage());
            const bool open =
                session.hasDocument() && canonicalPath(session.levelPath()) == canonicalPath(path);
            return done(open ? Effect{Effect::Kind::Close, session.levelPath()} : Effect{});
        }
        case Mode::Title: {
            const auto titled = ProjectMaps::setTitle(m_root, form.project, form.text);
            if (!titled.isOk())
                return R::Error(titled.errorMessage());
            return done(Effect{});
        }
        case Mode::Entry: {
            const auto entry = ProjectMaps::makeEntry(m_root, form.project, form.level);
            if (!entry.isOk())
                return R::Error(entry.errorMessage());
            return done(Effect{});
        }
    }
    return R::Error("formulario desconocido");
}

}  // namespace Editor
