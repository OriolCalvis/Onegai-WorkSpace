#include "Project/ProjectIndex.h"

#include <cstdio>
#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Project/ProjectSerializer.h"
#include <algorithm>

namespace Editor {
namespace {

bool leeFichero(const std::string& ruta, std::string& out) {
    std::FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) {
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    const long sz = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    out.clear();
    if (sz > 0) {
        out.resize(static_cast<std::size_t>(sz));
        (void)std::fread(&out[0], 1, static_cast<std::size_t>(sz), f);
    }
    std::fclose(f);
    return true;
}

bool existe(const std::string& ruta) {
    std::FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) {
        return false;
    }
    std::fclose(f);
    return true;
}

// El indice solo escribe ids previamente validados como [a-z0-9_].
std::string esc(const std::string& s) {
    std::string r;
    r.reserve(s.size() + 8);
    for (char c : s) {
        if (c == '"' || c == '\\') {
            r += '\\';
        }
        r += c;
    }
    return r;
}

// Resuelve las rutas de entrada contra la raiz solicitada, tambien al
// escanear una build copiada fuera del directorio del motor.
void resuelveEntrada(Project& p, const std::string& assetsRoot) {
    p.entryKind = Project::EntryKind::None;
    p.entryPath.clear();
    if (p.entry.empty()) return;
    std::string relative = p.entry;
    if (relative.compare(0, 7, "assets/") == 0) relative.erase(0, 7);
    if (p.entry.find('/') != std::string::npos) {
        const auto path = assetsRoot + "/" + relative;
        if (!existe(path)) {
            p.entryKind = Project::EntryKind::Missing;
            return;
        }
        p.entryPath = path;
        const auto directory = p.settings.adventuresDirectory + "/";
        p.entryKind = relative.compare(0, directory.size(), directory) == 0
            ? Project::EntryKind::Adventure : Project::EntryKind::Level;
        return;
    }
    for (const auto& group : {std::make_pair(p.settings.adventuresDirectory, Project::EntryKind::Adventure),
                              std::make_pair(p.settings.levelsDirectory, Project::EntryKind::Level)}) {
        const auto path = assetsRoot + "/" + group.first + "/" + relative;
        if (existe(path)) {
            p.entryPath = path;
            p.entryKind = group.second;
            return;
        }
    }
    p.entryKind = Project::EntryKind::Missing;
}

bool validId(const std::string& id) {
    return !id.empty() && std::all_of(id.begin(), id.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
    });
}

// Reescribe assets/proyectos/index.json con esta lista de ids. Lo usan
// create() y remove(): el manifiesto en disco y el indice tienen que
// moverse juntos o el proyecto existe y no lo ve nadie (o al reves).
bool escribeIndice(const std::string& assetsRoot, const std::vector<std::string>& ids) {
    const std::string ruta = assetsRoot + "/proyectos/index.json";
    std::FILE* f = std::fopen(ruta.c_str(), "wb");
    if (f == nullptr) {
        return false;
    }
    std::fprintf(f, "{\n \"_nota\": \"Lista de proyectos vivos. La lee Editor/ProjectIndex.\",\n"
                    " \"proyectos\": [\n");
    for (std::size_t i = 0; i < ids.size(); ++i) {
        std::fprintf(f, "  \"%s\"%s\n", esc(ids[i]).c_str(), i + 1 < ids.size() ? "," : "");
    }
    std::fprintf(f, " ]\n}\n");
    std::fclose(f);
    return true;
}

}  // namespace

Result<ProjectIndex> ProjectIndex::scan(const std::string& assetsRoot) {
    ProjectIndex idx;
    idx.m_root = assetsRoot;

    const std::string rutaIndice = assetsRoot + "/proyectos/index.json";
    std::string texto;
    if (!leeFichero(rutaIndice, texto)) {
        return Result<ProjectIndex>::Error("no se pudo abrir " + rutaIndice);
    }
    auto raiz = JsonValue::parse(texto);
    if (!raiz.isOk()) {
        return Result<ProjectIndex>::Error(rutaIndice + ": " + raiz.errorMessage());
    }
    const auto& entries = raiz.value()["proyectos"];
    if (!entries.isArray()) {
        return Result<ProjectIndex>::Error(rutaIndice + ": falta array 'proyectos'");
    }
    std::vector<std::string> ids;
    for (std::size_t i = 0; i < entries.size(); ++i) {
        if (!entries[i].isString() || !validId(entries[i].asString()))
            return Result<ProjectIndex>::Error(rutaIndice + ": id de proyecto invalido");
        const auto id = entries[i].asString();
        if (std::find(ids.begin(), ids.end(), id) != ids.end())
            return Result<ProjectIndex>::Error(rutaIndice + ": id duplicado '" + id + "'");
        ids.push_back(id);
    }
    idx.m_declaredIds = ids;

    for (const std::string& id : ids) {
        const std::string ruta = assetsRoot + "/proyectos/" + id + ".json";
        std::string t;
        if (!leeFichero(ruta, t)) {
            // Un id en el indice sin su manifiesto no tumba el editor: se
            // anota como huerfano y los demas proyectos siguen abriendose.
            idx.m_orphans.push_back("proyectos/" + id + ".json (declarado en index.json, no existe)");
            continue;
        }
        auto parsed = ProjectSerializer::deserializeProject(t);
        if (!parsed.isOk()) {
            idx.m_orphans.push_back("proyectos/" + id + ".json (" + parsed.errorMessage() + ")");
            continue;
        }
        Project pr = std::move(parsed.value());
        if (pr.id != id) {
            idx.m_orphans.push_back("proyectos/" + id + ".json (id no coincide con index.json)");
            continue;
        }
        resuelveEntrada(pr, assetsRoot);
        idx.m_projects.push_back(std::move(pr));
    }
    return Result<ProjectIndex>::Ok(std::move(idx));
}

const Project* ProjectIndex::find(const std::string& id) const {
    for (const Project& p : m_projects) {
        if (p.id == id) {
            return &p;
        }
    }
    return nullptr;
}

ProjectCheck ProjectIndex::check(const std::string& id) const {
    ProjectCheck c;
    c.projectId = id;
    const Project* p = find(id);
    if (p == nullptr) {
        c.problems.push_back("no existe el proyecto '" + id + "'");
        return c;
    }

    struct Grupo {
        const std::vector<std::string>* lista;
        std::string carpeta;
    };
    const Grupo grupos[] = {
        {&p->levels, p->settings.levelsDirectory}, {&p->maps, p->settings.mapsDirectory},
        {&p->adventures, p->settings.adventuresDirectory}, {&p->catalogs, p->settings.catalogsDirectory},
    };
    for (const Grupo& g : grupos) {
        for (const std::string& f : *g.lista) {
            if (!existe(m_root + "/" + g.carpeta + "/" + f)) {
                c.problems.push_back(std::string(g.carpeta) + "/" + f + ": declarado y no existe");
                ++c.missingFiles;
            }
        }
    }
    c.levelsChecked = static_cast<int>(p->levels.size());

    // Un proyecto sin entrada no se puede lanzar. No es un error de datos
    // -- un pack puede ser solo escenarios -- pero el editor tiene que
    // poder decirlo antes de ofrecer el boton de jugar.
    if (p->entry.empty()) {
        c.problems.push_back("sin 'entrada': no hay aventura ni nivel por el que arrancar");
    } else if (p->entryKind == Project::EntryKind::Missing) {
        c.problems.push_back("entrada '" + p->entry + "' no aparece ni en " + m_root +
                             "/" + p->settings.adventuresDirectory + "/ ni en " + m_root +
                             "/" + p->settings.levelsDirectory + "/");
        ++c.missingFiles;
    }
    return c;
}

Result<Project> ProjectIndex::create(const std::string& assetsRoot, const std::string& id,
                                     const std::string& name, const std::string& prefix,
                                     const std::string& author, const std::string& epoch) {
    if (id.empty()) {
        return Result<Project>::Error("el id del proyecto no puede estar vacio");
    }
    for (char ch : id) {
        const bool valido = (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_';
        if (!valido) {
            return Result<Project>::Error("id invalido '" + id + "': solo [a-z0-9_]");
        }
    }
    // El prefijo es lo que impide que dos autores se pisen los ids. Sin
    // el, la contribucion de uno sobreescribe la del otro y nadie se
    // entera hasta que falta un NPC (paso en el experimento de los
    // cuatro: dos cuadrantes usando "ciudad_*").
    if (!prefix.empty() && prefix.back() != '_') {
        return Result<Project>::Error("el prefijo '" + prefix + "' deberia acabar en '_'");
    }

    auto existente = scan(assetsRoot);
    if (!existente.isOk()) return Result<Project>::Error(existente.errorMessage());
    if (std::find(existente.value().m_declaredIds.begin(), existente.value().m_declaredIds.end(), id)
            != existente.value().m_declaredIds.end()) {
        return Result<Project>::Error("ya existe un proyecto con id '" + id + "'");
    }
    // Y tampoco si hay manifiesto suelto sin entrada en el indice. Pasa
    // cuando alguien edita index.json a mano: el proyecto no aparece en la
    // lista, create() lo daria por libre y machacaria el fichero con un
    // esqueleto vacio. Se recupera con remove() y volver a crearlo.
    const std::string destino = assetsRoot + "/proyectos/" + id + ".json";
    if (existe(destino)) {
        return Result<Project>::Error("ya hay un manifiesto en " + destino +
                                      " (no listado en index.json): borralo o anadelo al indice");
    }

    Project p;
    p.id = id;
    p.name = name.empty() ? id : name;
    p.prefix = prefix;
    p.author = author;
    p.epoch = epoch;
    const Result<bool> validation = p.validate();
    if (!validation.isOk()) {
        return Result<Project>::Error(validation.errorMessage());
    }

    const auto saved = ProjectSerializer::saveProjectAtomic(destino, p);
    if (!saved.isOk()) return Result<Project>::Error(saved.errorMessage());

    // Conservar tambien ids con manifiestos ausentes o invalidos: crear
    // otro proyecto no debe borrarlos silenciosamente del indice.
    std::vector<std::string> ids = existente.value().m_declaredIds;
    ids.push_back(id);
    if (!escribeIndice(assetsRoot, ids)) {
        return Result<Project>::Error("no se pudo actualizar " + assetsRoot + "/proyectos/index.json");
    }

    return Result<Project>::Ok(std::move(p));
}

Result<bool> ProjectIndex::remove(const std::string& assetsRoot, const std::string& id) {
    if (!validId(id)) return Result<bool>::Error("id de proyecto invalido");
    auto actual = scan(assetsRoot);
    if (!actual.isOk()) {
        return Result<bool>::Error(actual.errorMessage());
    }
    const std::string ruta = assetsRoot + "/proyectos/" + id + ".json";
    const auto& declared = actual.value().m_declaredIds;
    const bool enIndice = std::find(declared.begin(), declared.end(), id) != declared.end();
    const bool enDisco = existe(ruta);
    // Basta con que exista de UNA de las dos formas. Si solo se aceptara
    // "esta en el indice", un manifiesto huerfano -- fichero suelto que
    // alguien saco del indice a mano, o que dejo una prueba abortada --
    // seria imposible de limpiar: remove() lo rechaza por no estar en el
    // indice y create() lo rechaza por estar en disco. El proyecto queda
    // atascado y no hay forma de salir sin borrar a mano.
    if (!enIndice && !enDisco) {
        return Result<bool>::Error("no existe un proyecto con id '" + id + "'");
    }

    // Primero el indice: si el manifiesto no se deja borrar (permisos, un
    // mount raro), el proyecto ya no aparece y no se queda un id en la
    // lista apuntando a algo que el editor va a intentar abrir.
    std::vector<std::string> ids;
    for (const auto& projectId : declared) {
        if (projectId != id) {
            ids.push_back(projectId);
        }
    }
    if (enIndice && !escribeIndice(assetsRoot, ids)) {
        return Result<bool>::Error("no se pudo actualizar el indice");
    }
    if (std::remove(ruta.c_str()) != 0 && existe(ruta)) {
        return Result<bool>::Error("fuera del indice, pero no se pudo borrar " + ruta);
    }
    return Result<bool>::Ok(true);
}

}  // namespace Editor
