#include "Editor/ProjectIndex.h"

#include <cstdio>
#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"

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

std::vector<std::string> lista(const JsonValue& v) {
    std::vector<std::string> out;
    if (!v.isArray()) {
        return out;
    }
    out.reserve(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) {
        out.push_back(v[i].asString(""));
    }
    return out;
}

// Escapa lo justo para volver a escribir un manifiesto. NO se emiten
// escapes \uXXXX: el parser JSON del motor no los decodifica a proposito
// (ver JsonValue.cpp), asi que el UTF-8 va crudo y punto.
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

// Averigua a que apunta 'entrada'. El campo se escribio de dos formas
// distintas -- ruta completa en boundington y oeste_norte, nombre suelto
// en este_norte -- porque nunca se documento cual era. En vez de declarar
// buena una de las dos y romper el trabajo de otro, se admiten ambas y se
// deja anotado en el propio Project a que resolvio.
void resuelveEntrada(Project& p, const std::string& assetsRoot) {
    p.entryKind = Project::EntryKind::None;
    p.entryPath.clear();
    if (p.entry.empty()) {
        return;
    }
    // Con barra: es una ruta y se toma tal cual, sin adivinar.
    if (p.entry.find('/') != std::string::npos) {
        if (existe(p.entry)) {
            p.entryPath = p.entry;
            // La carpeta dice que es. No se abre el fichero para mirarlo
            // dentro: scan() lo hace al listar el editor y leer 96
            // ficheros para pintar una lista es caro y no hace falta.
            p.entryKind = p.entry.find("/adventures/") != std::string::npos
                              ? Project::EntryKind::Adventure
                              : Project::EntryKind::Level;
        } else {
            p.entryKind = Project::EntryKind::Missing;
        }
        return;
    }
    // Nombre suelto: aventuras primero, que es el caso normal.
    const std::string comoAv = assetsRoot + "/adventures/" + p.entry;
    if (existe(comoAv)) {
        p.entryPath = comoAv;
        p.entryKind = Project::EntryKind::Adventure;
        return;
    }
    const std::string comoNivel = assetsRoot + "/levels/" + p.entry;
    if (existe(comoNivel)) {
        p.entryPath = comoNivel;
        p.entryKind = Project::EntryKind::Level;
        return;
    }
    p.entryKind = Project::EntryKind::Missing;
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
    const std::vector<std::string> ids = lista(raiz.value()["proyectos"]);
    if (ids.empty()) {
        return Result<ProjectIndex>::Error(rutaIndice + ": la lista 'proyectos' esta vacia");
    }

    for (const std::string& id : ids) {
        const std::string ruta = assetsRoot + "/proyectos/" + id + ".json";
        std::string t;
        if (!leeFichero(ruta, t)) {
            // Un id en el indice sin su manifiesto no tumba el editor: se
            // anota como huerfano y los demas proyectos siguen abriendose.
            idx.m_orphans.push_back("proyectos/" + id + ".json (declarado en index.json, no existe)");
            continue;
        }
        auto p = JsonValue::parse(t);
        if (!p.isOk()) {
            idx.m_orphans.push_back("proyectos/" + id + ".json (JSON invalido: " + p.errorMessage() + ")");
            continue;
        }
        const JsonValue& v = p.value();
        Project pr;
        pr.id = v["id"].asString(id);
        pr.name = v["nombre"].asString(pr.id);
        pr.description = v["descripcion"].asString("");
        pr.epoch = v["epoca"].asString("");
        pr.author = v["autor"].asString("");
        pr.prefix = v["prefijo"].asString("");
        pr.entry = v["entrada"].asString("");
        pr.levels = lista(v["niveles"]);
        pr.maps = lista(v["mapas"]);
        pr.adventures = lista(v["aventuras"]);
        pr.catalogs = lista(v["catalogos"]);
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
        const char* carpeta;
    };
    const Grupo grupos[] = {
        {&p->levels, "levels"}, {&p->maps, "maps"},
        {&p->adventures, "adventures"}, {&p->catalogs, "objects"},
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
                             "/adventures/ ni en " + m_root + "/levels/");
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
    if (existente.isOk() && existente.value().find(id) != nullptr) {
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

    std::FILE* f = std::fopen(destino.c_str(), "wb");
    if (f == nullptr) {
        return Result<Project>::Error("no se pudo escribir " + destino);
    }
    std::fprintf(f,
                 "{\n"
                 " \"_nota\": \"Manifiesto de proyecto. Creado desde el editor.\",\n"
                 " \"id\": \"%s\",\n \"nombre\": \"%s\",\n \"descripcion\": \"\",\n"
                 " \"epoca\": \"%s\",\n \"autor\": \"%s\",\n \"prefijo\": \"%s\",\n"
                 " \"niveles\": [],\n \"mapas\": [],\n \"aventuras\": [],\n"
                 " \"catalogos\": [],\n \"entrada\": \"\"\n}\n",
                 esc(id).c_str(), esc(p.name).c_str(), esc(epoch).c_str(),
                 esc(author).c_str(), esc(prefix).c_str());
    std::fclose(f);

    // Y al indice, o el proyecto existe en disco y nadie lo ve.
    std::vector<std::string> ids;
    if (existente.isOk()) {
        for (const Project& q : existente.value().projects()) {
            ids.push_back(q.id);
        }
    }
    ids.push_back(id);
    if (!escribeIndice(assetsRoot, ids)) {
        return Result<Project>::Error("no se pudo actualizar " + assetsRoot + "/proyectos/index.json");
    }

    return Result<Project>::Ok(std::move(p));
}

Result<bool> ProjectIndex::remove(const std::string& assetsRoot, const std::string& id) {
    auto actual = scan(assetsRoot);
    if (!actual.isOk()) {
        return Result<bool>::Error(actual.errorMessage());
    }
    if (actual.value().find(id) == nullptr) {
        return Result<bool>::Error("no existe un proyecto con id '" + id + "'");
    }

    // Primero el indice: si el manifiesto no se deja borrar (permisos, un
    // mount raro), el proyecto ya no aparece y no se queda un id en la
    // lista apuntando a algo que el editor va a intentar abrir.
    std::vector<std::string> ids;
    for (const Project& q : actual.value().projects()) {
        if (q.id != id) {
            ids.push_back(q.id);
        }
    }
    if (!escribeIndice(assetsRoot, ids)) {
        return Result<bool>::Error("no se pudo actualizar el indice");
    }
    const std::string ruta = assetsRoot + "/proyectos/" + id + ".json";
    if (std::remove(ruta.c_str()) != 0 && existe(ruta)) {
        return Result<bool>::Error("fuera del indice, pero no se pudo borrar " + ruta);
    }
    return Result<bool>::Ok(true);
}

}  // namespace Editor
