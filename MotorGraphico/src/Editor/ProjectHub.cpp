#include "Editor/ProjectHub.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

// popen/pclose son POSIX; en Windows llevan guion bajo. El motor no tiene
// hoy una capa de plataforma, asi que el arreglo va aqui y no se inventa
// una abstraccion para dos nombres.
#if defined(_WIN32)
#define ABRIR_TUBERIA _popen
#define CERRAR_TUBERIA _pclose
#else
#define ABRIR_TUBERIA popen
#define CERRAR_TUBERIA pclose
#endif

namespace Editor {
namespace {

// Rellena a la derecha hasta 'ancho'. El HUD usa una fuente de ancho fijo
// (BitmapFont), asi que alinear con espacios da columnas de verdad.
std::string col(const std::string& s, std::size_t ancho) {
    std::string r = s.substr(0, ancho);
    r.resize(ancho, ' ');
    return r;
}

std::string num(int n, std::size_t ancho) {
    std::string s = std::to_string(n);
    return std::string(ancho > s.size() ? ancho - s.size() : 0, ' ') + s;
}

// Un id que va a acabar en una linea de shell. ProjectIndex::create() ya
// lo obliga, pero un manifiesto se puede editar a mano y el id de dentro
// mandar sobre el del fichero, asi que no se da por bueno.
bool idSeguro(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        const bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
        if (!ok) {
            return false;
        }
    }
    return true;
}

}  // namespace

Result<ProjectHub> ProjectHub::load(const std::string& assetsRoot) {
    auto r = ProjectIndex::scan(assetsRoot);
    if (!r.isOk()) {
        return Result<ProjectHub>::Error(r.errorMessage());
    }
    ProjectHub h;
    h.m_index = r.value();
    h.m_root = assetsRoot;
    h.rebuildLines();
    return Result<ProjectHub>::Ok(std::move(h));
}

void ProjectHub::rebuildLines() {
    m_lines.clear();
    for (const Project& p : m_index.projects()) {
        const ProjectCheck c = m_index.check(p.id);
        // "jugable" y "completo" no son lo mismo y la diferencia importa:
        // un pack sin aventura se edita igual, solo que no arranca.
        const char* estado = c.ok()
                                 ? (p.entryKind == Project::EntryKind::Adventure ? "listo" : "sin historia")
                                 : "con avisos";
        m_lines.push_back(col(p.name, 30) + " " + num((int)p.levels.size(), 4) +
                          " niv " + num((int)p.adventures.size(), 2) + " av   " + estado);
    }
    if (m_selected >= m_lines.size()) {
        m_selected = m_lines.empty() ? 0 : m_lines.size() - 1;
    }
}

const Project* ProjectHub::current() const {
    if (m_selected >= m_index.size()) {
        return nullptr;
    }
    return &m_index.projects()[m_selected];
}

void ProjectHub::moveUp() {
    if (m_index.size() == 0) {
        return;
    }
    // Da la vuelta: con cinco proyectos, llegar al ultimo desde el primero
    // no deberia costar cuatro pulsaciones.
    m_selected = (m_selected == 0) ? m_index.size() - 1 : m_selected - 1;
}

void ProjectHub::moveDown() {
    if (m_index.size() == 0) {
        return;
    }
    m_selected = (m_selected + 1) % m_index.size();
}

std::vector<std::string> ProjectHub::detail() const {
    std::vector<std::string> d;
    const Project* p = current();
    if (p == nullptr) {
        d.push_back("No hay ningun proyecto en assets/proyectos.");
        d.push_back("Pulsa N para crear el primero.");
        return d;
    }
    d.push_back(p->name);
    if (!p->epoch.empty()) {
        d.push_back("epoca:  " + p->epoch);
    }
    if (!p->author.empty()) {
        d.push_back("autor:  " + p->author);
    }
    d.push_back("id:     " + p->id + (p->prefix.empty() ? "" : "   prefijo " + p->prefix));
    d.push_back("");
    d.push_back("niveles " + std::to_string(p->levels.size()) +
                "   mapas " + std::to_string(p->maps.size()) +
                "   aventuras " + std::to_string(p->adventures.size()) +
                "   catalogos " + std::to_string(p->catalogs.size()));
    // Decir SOLO el nombre del fichero esconde el problema que tuvimos:
    // 'entrada' se relleno de dos formas distintas y una no resolvia. Aqui
    // se dice a que resolvio de verdad, no lo que pone el manifiesto.
    switch (p->entryKind) {
        case Project::EntryKind::None:
            d.push_back("arranque: (ninguno) - se puede editar, no jugar");
            break;
        case Project::EntryKind::Adventure:
            d.push_back("arranque: aventura " + p->entryPath);
            break;
        case Project::EntryKind::Level:
            d.push_back("arranque: nivel " + p->entryPath + "  (sin historia)");
            break;
        case Project::EntryKind::Missing:
            d.push_back("arranque: '" + p->entry + "' NO SE ENCUENTRA");
            break;
    }
    const ProjectCheck c = m_index.check(p->id);
    if (!c.ok()) {
        d.push_back("");
        // Solo los primeros: si a un pack le faltan 40 ficheros, la lista
        // entera tapa la pantalla y no ayuda mas que saber que faltan.
        for (std::size_t i = 0; i < c.problems.size() && i < 6; ++i) {
            d.push_back("! " + c.problems[i]);
        }
        if (c.problems.size() > 6) {
            d.push_back("! ...y " + std::to_string(c.problems.size() - 6) + " mas");
        }
    }
    return d;
}

void ProjectHub::setMessage(bool ok, const std::string& titulo, const std::string& cuerpo) {
    m_lastOk = ok;
    m_message.clear();
    m_message.push_back(titulo);
    std::string linea;
    for (char c : cuerpo) {
        if (c == '\n') {
            m_message.push_back(linea);
            linea.clear();
        } else if (c != '\r') {
            linea += c;
        }
    }
    if (!linea.empty()) {
        m_message.push_back(linea);
    }
    m_mode = Mode::Message;
}

void ProjectHub::refresh() {
    auto r = ProjectIndex::scan(m_root);
    if (r.isOk()) {
        m_index = r.value();
        rebuildLines();
    }
}

void ProjectHub::build() {
    const Project* p = current();
    if (p == nullptr) {
        setMessage(false, "No hay proyecto que compilar", "");
        return;
    }
    if (!idSeguro(p->id)) {
        setMessage(false, "Id no valido: '" + p->id + "'",
                   "Solo [a-z0-9_]. Revisa el manifiesto a mano.");
        return;
    }
    const std::string cmd = "python3 tools/build_proyecto.py " + p->id + " 2>&1";
    std::FILE* tuberia = ABRIR_TUBERIA(cmd.c_str(), "r");
    if (tuberia == nullptr) {
        setMessage(false, "No se pudo lanzar el empaquetado",
                   "Hace falta python3 en el PATH.\n" + cmd);
        return;
    }
    std::string salida;
    char buf[512];
    while (std::fgets(buf, sizeof(buf), tuberia) != nullptr) {
        salida += buf;
    }
    // pclose devuelve el estado de espera, no el codigo de salida: el
    // script devuelve 1 si algun proyecto no quedo jugable, y eso hay que
    // poder distinguirlo de "fue bien".
    const int estado = CERRAR_TUBERIA(tuberia);
    const bool ok = (estado == 0);
    setMessage(ok, ok ? "Build de '" + p->id + "' lista en builds/" + p->id + "/"
                      : "El empaquetado de '" + p->id + "' termino con avisos",
               salida);
}

ProjectHub::Action ProjectHub::key(char k) {
    // --- Escribiendo el id de un proyecto nuevo ---
    if (m_mode == Mode::NewProject) {
        if (k == 27) {                       // ESC: cancelar
            m_draft.clear();
            m_mode = Mode::List;
            return Action::None;
        }
        if (k == '\b') {
            if (!m_draft.empty()) {
                m_draft.pop_back();
            }
            return Action::None;
        }
        if (k == '\n') {
            if (m_draft.empty()) {
                return Action::None;
            }
            // El prefijo sale del id, no se pide aparte: pedirlo es una
            // pregunta mas en una pantalla donde ya hay que teclear, y la
            // respuesta correcta es casi siempre esta. Se puede cambiar
            // luego en el manifiesto.
            const std::string prefijo = m_draft.substr(0, 3) + "_";
            auto r = ProjectIndex::create(m_root, m_draft, m_draft, prefijo, "editor", "");
            if (!r.isOk()) {
                setMessage(false, "No se pudo crear '" + m_draft + "'", r.errorMessage());
                m_draft.clear();
                return Action::None;
            }
            const std::string creado = m_draft;
            m_draft.clear();
            refresh();
            // Dejarlo marcado: acabas de crearlo, es lo que quieres tocar.
            for (std::size_t i = 0; i < m_index.size(); ++i) {
                if (m_index.projects()[i].id == creado) {
                    m_selected = i;
                }
            }
            setMessage(true, "Proyecto '" + creado + "' creado",
                       "Prefijo " + prefijo + ". Esta vacio: abrelo con ENTER,\n"
                       "dibuja un nivel y guardalo con F5.");
            return Action::None;
        }
        const bool valido = (k >= 'a' && k <= 'z') || (k >= '0' && k <= '9') || k == '_';
        if (valido && m_draft.size() < 24) {
            m_draft += k;
        }
        return Action::None;
    }

    // --- Mostrando un mensaje: cualquier tecla vuelve ---
    if (m_mode == Mode::Message) {
        m_message.clear();
        m_mode = Mode::List;
        return Action::None;
    }

    // --- La lista ---
    switch (k) {
        case 'w':
            moveUp();
            return Action::None;
        case 's':
            moveDown();
            return Action::None;
        case 'n':
            m_draft.clear();
            m_mode = Mode::NewProject;
            return Action::None;
        case 'b':
            build();
            return Action::None;
        case 27:
            return Action::Quit;
        case '\n': {
            const Project* p = current();
            if (p == nullptr) {
                return Action::None;
            }
            // Un proyecto sin niveles se abre igual, con una rejilla en
            // blanco: hay que poder empezar el primer nivel de algo. No se
            // avisa con un modal -- el panel de detalle ya dice "niveles
            // 0" antes de pulsar, y un aviso aqui saldria en bucle porque
            // al volver a la lista el proyecto sigue sin niveles.
            m_openId = p->id;
            return Action::Open;
        }
        default:
            return Action::None;
    }
}

}  // namespace Editor
