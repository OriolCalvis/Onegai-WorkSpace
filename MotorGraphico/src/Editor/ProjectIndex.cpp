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
        c.problems.push_back("sin 'entrada': no hay aventura por la que arrancar");
    } else if (!existe(p->entry)) {
        c.problems.push_back("entrada '" + p->entry + "' no existe");
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

    Project p;
    p.id = id;
    p.name = name.empty() ? id : name;
    p.prefix = prefix;
    p.author = author;
    p.epoch = epoch;

    const std::string ruta = assetsRoot + "/proyectos/" + id + ".json";
    std::FILE* f = std::fopen(ruta.c_str(), "wb");
    if (f == nullptr) {
        return Result<Project>::Error("no se pudo escribir " + ruta);
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
    const std::string rutaIdx = assetsRoot + "/proyectos/index.json";
    std::FILE* fi = std::fopen(rutaIdx.c_str(), "wb");
    if (fi == nullptr) {
        return Result<Project>::Error("no se pudo actualizar " + rutaIdx);
    }
    std::fprintf(fi, "{\n \"_nota\": \"Lista de proyectos vivos. La lee Editor/ProjectIndex.\",\n"
                     " \"proyectos\": [\n");
    for (std::size_t i = 0; i < ids.size(); ++i) {
        std::fprintf(fi, "  \"%s\"%s\n", esc(ids[i]).c_str(), i + 1 < ids.size() ? "," : "");
    }
    std::fprintf(fi, " ]\n}\n");
    std::fclose(fi);

    return Result<Project>::Ok(std::move(p));
}

}  // namespace Editor
