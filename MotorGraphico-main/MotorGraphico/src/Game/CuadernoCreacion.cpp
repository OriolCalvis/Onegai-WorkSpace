// UI-BG3-01 — ver include/Game/CuadernoCreacion.h.
#include "Game/CuadernoCreacion.h"

#include <algorithm>
#include <cctype>
#include <utility>


namespace Game {

namespace {

using Hoja = CuadernoCreacion::Hoja;
using Zona = CuadernoCreacion::Zona;

constexpr std::size_t n(Hoja h) { return static_cast<std::size_t>(h); }

bool esRejilla(Hoja h) { return h == Hoja::Raza || h == Hoja::Clase || h == Hoja::Origen || h == Hoja::Deidad; }

// Clave para ordenar y buscar: minusculas y sin tildes («Ángel» -> «angel»).
std::string plano(const std::string& s) {
    static const char kLatin1[] = "aaaaaaaceeeeiiiidnooooo*ouuuuyts";
    std::string out;
    for (std::size_t i = 0; i < s.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == 0xC3 && i + 1 < s.size()) {
            out += kLatin1[static_cast<unsigned char>(s[i + 1]) & 0x1F];
            ++i;
        } else {
            out += static_cast<char>(std::tolower(c));
        }
    }
    return out;
}

// Quita el ultimo punto de codigo UTF-8 de `s`.
void borraUltimo(std::string& s) {
    while (!s.empty()) {
        const unsigned char c = static_cast<unsigned char>(s.back());
        s.pop_back();
        if ((c & 0xC0) != 0x80) break;  // no era un byte de continuacion
    }
}

// Solo texto imprimible (las teclas de control no llegan como texto, pero
// por si acaso) y sin pasar del maximo de bytes.
void anade(std::string& campo, const std::string& texto, std::size_t maximo) {
    for (char c : texto) {
        if (static_cast<unsigned char>(c) < 0x20) continue;
        campo += c;
    }
    while (campo.size() > maximo) borraUltimo(campo);
}

std::string conMayuscula(std::string s) {
    if (!s.empty()) s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
    return s;
}

}  // namespace

CuadernoCreacion::CuadernoCreacion(const Datos& datos) : m_datos(datos), m_inicio(std::chrono::steady_clock::now()) {
    if (m_datos.personajes != nullptr) m_editor = std::make_unique<EditorApariencia>(*m_datos.personajes);
    m_nav.foco.fill(-1);
    entraEn(Hoja::Raza);
}

CuadernoCreacion::~CuadernoCreacion() = default;

const char* CuadernoCreacion::nombreHoja(Hoja h) {
    switch (h) {
        case Hoja::Raza: return "Raza";
        case Hoja::Aspecto: return "Aspecto";
        case Hoja::Clase: return "Clase";
        case Hoja::Origen: return "Origen";
        case Hoja::Historia: return "Historia";
        case Hoja::Deidad: return "Deidad";
        case Hoja::Nombre: return "Nombre";
    }
    return "";
}

const char* CuadernoCreacion::nombrePregunta(int fila) {
    static const char* const kNombres[kPreguntas] = {"Vínculo", "Miedo", "Defecto", "Meta", "Ideal", "Carácter",
                                                     "Virtud"};
    return fila >= 0 && fila < kPreguntas ? kNombres[fila] : "";
}

const std::vector<std::pair<std::string, std::string>>& CuadernoCreacion::roles() {
    static const std::vector<std::pair<std::string, std::string>> kRoles = {
        {"", "Todas"},        {"caster", "Lanzador"}, {"agile", "Ágil"}, {"tank", "Defensa"},
        {"balanced", "Equilibrio"}, {"support", "Apoyo"}, {"striker", "Golpe"}};
    return kRoles;
}

const char* CuadernoCreacion::nombreRol(const std::string& rol) {
    for (const auto& [id, nombre] : roles())
        if (id == rol) return nombre.c_str();
    return "";
}

const std::vector<std::string>& CuadernoCreacion::sugerencias() {
    static const std::vector<std::string> kNombres = {"Nerea", "Aldric", "Brisa", "Telmo", "Ivet", "Oren"};
    return kNombres;
}

int CuadernoCreacion::filasVisibles(Hoja h) {
    switch (h) {
        case Hoja::Raza: return 4;
        case Hoja::Clase:
        case Hoja::Origen:
        case Hoja::Deidad: return 3;
        default: return 0;
    }
}

std::vector<std::string> CuadernoCreacion::ids(Hoja h) const {
    std::vector<std::pair<std::string, std::string>> v;  // (clave de orden, id)
    const RPG::CharacterCreation& r = *m_datos.reglas;
    const std::string buscado = plano(m_nav.busqueda);
    switch (h) {
        case Hoja::Raza:
            r.races().forEach([&](const RPG::RaceDefinition& d) {
                if (!d.playable) return;  // PNJ-02: los yokai siguen en el lore, no se juegan
                const std::string clave = plano(d.name);
                if (buscado.empty() || clave.find(buscado) != std::string::npos) v.emplace_back(clave, d.id);
            });
            break;
        case Hoja::Clase:
            r.classes().forEach([&](const RPG::ClassDefinition& d) {
                if (!d.playable) return;  // DIALOGOS-01: las 70 nuevas
                if (m_nav.rol.empty() || d.role == m_nav.rol) v.emplace_back(plano(d.name), d.id);
            });
            break;
        case Hoja::Origen:
            r.backgrounds().forEach([&](const RPG::BackgroundDefinition& d) { v.emplace_back(plano(d.name), d.id); });
            break;
        case Hoja::Deidad:
            r.deities().forEach([&](const RPG::DeityDefinition& d) {
                if (d.esCreador()) v.emplace_back(plano(d.name), d.id);  // DIOSES-01: solo los Creadores
            });
            break;
        default: return {};
    }
    std::sort(v.begin(), v.end());
    std::vector<std::string> out;
    if (h == Hoja::Deidad) out.emplace_back();  // «Ninguna» va la primera
    for (auto& par : v) out.push_back(std::move(par.second));
    return out;
}

std::string CuadernoCreacion::elegido(Hoja h) const {
    switch (h) {
        case Hoja::Raza: return m_eleccion.raceId;
        case Hoja::Clase: return m_eleccion.classId;
        case Hoja::Origen: return m_eleccion.backgroundId;
        case Hoja::Deidad: return m_eleccion.deityId;
        default: return {};
    }
}

std::string CuadernoCreacion::senalado() const {
    if (!esRejilla(m_hoja)) return elegido(m_hoja);
    const std::vector<std::string> lista = ids();
    const int f = foco();
    if (f >= 0 && f < static_cast<int>(lista.size())) return lista[static_cast<std::size_t>(f)];
    return elegido(m_hoja);
}

bool CuadernoCreacion::hecha(Hoja h) const {
    switch (h) {
        case Hoja::Raza: return !m_eleccion.raceId.empty();
        case Hoja::Aspecto:
            return !m_eleccion.raceId.empty() && !m_eleccion.classId.empty() && m_nav.visitadas.test(n(Hoja::Aspecto));
        case Hoja::Clase: return !m_eleccion.classId.empty();
        case Hoja::Origen: return !m_eleccion.backgroundId.empty();
        case Hoja::Historia:
            if (m_eleccion.backgroundId.empty()) return false;
            for (int f = 0; f < kPreguntas; ++f)
                if (elegidaHistoria(f).empty()) return false;
            return true;
        case Hoja::Deidad: return m_nav.deidadDecidida;
        case Hoja::Nombre: return !m_eleccion.displayName.empty();
    }
    return false;
}

std::vector<std::string> CuadernoCreacion::problemas() const {
    std::vector<std::string> p;
    if (m_eleccion.raceId.empty()) p.emplace_back("Elige tu raza.");
    if (m_eleccion.classId.empty()) p.emplace_back("Elige tu clase.");
    if (m_eleccion.backgroundId.empty()) {
        p.emplace_back("Elige tu origen.");
    } else {
        int faltan = 0;
        for (int f = 0; f < kPreguntas; ++f) faltan += elegidaHistoria(f).empty() ? 1 : 0;
        if (faltan == 1) p.emplace_back("Te falta una pregunta de tu historia.");
        if (faltan > 1) p.push_back("Te faltan " + std::to_string(faltan) + " preguntas de tu historia.");
    }
    if (m_eleccion.displayName.empty()) p.emplace_back("Escribe tu nombre.");
    if (p.empty())
        for (const std::string& s : m_datos.reglas->problemas(m_eleccion)) p.push_back(conMayuscula(s) + ".");
    return p;
}

// --- Navegacion ---------------------------------------------------------

void CuadernoCreacion::entraEn(Hoja h) {
    m_hoja = h;
    m_nav.visitadas.set(n(h));
    m_nav.buscando = false;
    m_aviso.clear();
    int& f = m_nav.foco[n(h)];
    if (esRejilla(h)) {
        // El foco empieza en lo elegido (o en el primero).
        const std::vector<std::string> lista = ids(h);
        const auto it = std::find(lista.begin(), lista.end(), elegido(h));
        if (it != lista.end() && (h == Hoja::Deidad || !elegido(h).empty()))
            f = static_cast<int>(it - lista.begin());
        else if (f < 0 || f >= static_cast<int>(lista.size()))
            f = lista.empty() ? -1 : 0;
        ajustaScroll();
    } else if (h == Hoja::Aspecto) {
        f = m_editor ? static_cast<int>(m_editor->marcada()) : 0;
    } else if (h == Hoja::Historia) {
        if (f < 0) {
            f = 0;
            // La primera pregunta sin contestar.
            for (int fila = 0; fila < kPreguntas; ++fila) {
                if (elegidaHistoria(fila).empty()) {
                    f = fila * kColumnas;
                    break;
                }
            }
        }
    } else if (h == Hoja::Nombre) {
        f = std::max(f, -1);
    }
}

void CuadernoCreacion::anterior() {
    if (m_hoja != Hoja::Raza) entraEn(static_cast<Hoja>(n(m_hoja) - 1));
}

void CuadernoCreacion::siguiente() {
    if (m_hoja != Hoja::Nombre) entraEn(static_cast<Hoja>(n(m_hoja) + 1));
}

void CuadernoCreacion::irA(Hoja h) { entraEn(h); }

void CuadernoCreacion::atras() {
    if (m_nav.buscando) {
        m_nav.buscando = false;
    } else if (m_hoja == Hoja::Raza) {
        m_estado = Estado::Cancelado;
    } else {
        anterior();
    }
}

void CuadernoCreacion::continuar() {
    if (m_hoja == Hoja::Nombre) {
        const std::vector<std::string> p = problemas();
        if (p.empty()) m_estado = Estado::Terminado;
        else m_aviso = p.front();
        return;
    }
    if (!hecha(m_hoja)) {
        switch (m_hoja) {
            case Hoja::Raza: m_aviso = "Elige tu raza para seguir: clic o ENTER."; break;
            case Hoja::Aspecto: m_aviso = "Antes elige tu raza y tu clase."; break;
            case Hoja::Clase: m_aviso = "Elige tu clase para seguir."; break;
            case Hoja::Origen: m_aviso = "Elige tu origen para seguir."; break;
            case Hoja::Historia:
                m_aviso = m_eleccion.backgroundId.empty() ? "Antes elige tu origen."
                                                          : "Contesta las siete preguntas para seguir.";
                break;
            case Hoja::Deidad: m_aviso = "Elige una deidad o «Ninguna» para seguir."; break;
            case Hoja::Nombre: break;
        }
        return;
    }
    siguiente();
}

void CuadernoCreacion::ajustaScroll() {
    if (!esRejilla(m_hoja)) return;
    const int total = static_cast<int>(ids().size());
    const int visibles = filasVisibles(m_hoja);
    const int filas = (total + kColumnas - 1) / kColumnas;
    int& primera = m_nav.primera[n(m_hoja)];
    const int f = foco();
    if (f >= 0) {
        const int fila = f / kColumnas;
        if (fila < primera) primera = fila;
        if (fila >= primera + visibles) primera = fila - visibles + 1;
    }
    primera = std::clamp(primera, 0, std::max(0, filas - visibles));
}

void CuadernoCreacion::mueve(int dx, int dy) {
    int& f = m_nav.foco[n(m_hoja)];
    if (esRejilla(m_hoja)) {
        const int total = static_cast<int>(ids().size());
        if (total == 0) return;
        f = f < 0 ? 0 : std::clamp(f + dx + dy * kColumnas, 0, total - 1);
        ajustaScroll();
    } else if (m_hoja == Hoja::Aspecto) {
        if (!m_editor) return;
        if (dy < 0) m_editor->arriba();
        if (dy > 0) m_editor->abajo();
        if (dx < 0) m_editor->izquierda();
        if (dx > 0) m_editor->derecha();
        f = static_cast<int>(m_editor->marcada());
    } else if (m_hoja == Hoja::Historia) {
        const int fila = std::clamp(std::max(f, 0) / kColumnas + dy, 0, kPreguntas - 1);
        const int columna = std::clamp(std::max(f, 0) % kColumnas + dx, 0, kColumnas - 1);
        f = fila * kColumnas + columna;
    } else if (m_hoja == Hoja::Nombre && dx != 0) {
        const int total = static_cast<int>(sugerencias().size());
        f = ((f < 0 ? (dx > 0 ? -1 : 0) : f) + dx + total) % total;
        m_eleccion.displayName = sugerencias()[static_cast<std::size_t>(f)];
    }
}

void CuadernoCreacion::acepta() {
    if (m_nav.buscando) {
        m_nav.buscando = false;
        m_nav.foco[n(Hoja::Raza)] = ids().empty() ? -1 : 0;
        ajustaScroll();
        return;
    }
    const int f = foco();
    if (esRejilla(m_hoja)) {
        const std::vector<std::string> lista = ids();
        if (f < 0 || f >= static_cast<int>(lista.size())) return;
        // ENTER sobre lo ya elegido = seguir (dos ENTER: elegir y pasar).
        if (hecha(m_hoja) && lista[static_cast<std::size_t>(f)] == elegido(m_hoja)) continuar();
        else elige(m_hoja, f);
    } else if (m_hoja == Hoja::Aspecto) {
        const auto fila = m_editor ? m_editor->filaMarcada() : EditorApariencia::Fila::Piel;
        if (m_editor && (fila == EditorApariencia::Fila::AlAzar || fila == EditorApariencia::Fila::ComoSuRaza))
            m_editor->derecha();
        else
            continuar();
    } else if (m_hoja == Hoja::Historia) {
        const int fila = std::max(f, 0) / kColumnas, columna = std::max(f, 0) % kColumnas;
        const std::vector<RPG::CharacterCreation::OpcionNarrativa>* ops = opciones(fila);
        if (ops == nullptr) {
            continuar();
            return;
        }
        if (hecha(Hoja::Historia) && columna < static_cast<int>(ops->size()) &&
            elegidaHistoria(fila) == (*ops)[static_cast<std::size_t>(columna)].id) {
            continuar();
            return;
        }
        eligeHistoria(fila, columna);
        if (fila + 1 < kPreguntas) m_nav.foco[n(Hoja::Historia)] = (fila + 1) * kColumnas + columna;
    } else {
        continuar();
    }
}

void CuadernoCreacion::rueda(int pasos) {
    if (!esRejilla(m_hoja) || pasos == 0) return;
    const int total = static_cast<int>(ids().size());
    const int filas = (total + kColumnas - 1) / kColumnas;
    int& primera = m_nav.primera[n(m_hoja)];
    primera = std::clamp(primera - pasos, 0, std::max(0, filas - filasVisibles(m_hoja)));
}

bool CuadernoCreacion::escribiendo() const {
    return (m_hoja == Hoja::Raza && m_nav.buscando) || m_hoja == Hoja::Nombre;
}

void CuadernoCreacion::escribe(const std::string& utf8) {
    if (utf8.empty()) return;
    if (m_hoja == Hoja::Raza && m_nav.buscando) {
        anade(m_nav.busqueda, utf8, kNombreMax);
        m_nav.foco[n(Hoja::Raza)] = 0;
        m_nav.primera[n(Hoja::Raza)] = 0;
    } else if (m_hoja == Hoja::Nombre) {
        anade(m_eleccion.displayName, utf8, kNombreMax);
        m_aviso.clear();
    }
}

void CuadernoCreacion::borra() {
    if (m_hoja == Hoja::Raza && m_nav.buscando) {
        borraUltimo(m_nav.busqueda);
        m_nav.foco[n(Hoja::Raza)] = 0;
        m_nav.primera[n(Hoja::Raza)] = 0;
    } else if (m_hoja == Hoja::Nombre) {
        borraUltimo(m_eleccion.displayName);
    }
}

// --- Raton ----------------------------------------------------------------

void CuadernoCreacion::senala(Zona z, int indice) {
    switch (z) {
        case Zona::Tesela:
            if (esRejilla(m_hoja) && indice >= 0 && indice < static_cast<int>(ids().size()))
                m_nav.foco[n(m_hoja)] = indice;
            break;
        case Zona::Tarjeta:
            if (m_hoja == Hoja::Historia && indice >= 0 && indice < kPreguntas * kColumnas)
                m_nav.foco[n(m_hoja)] = indice;
            break;
        case Zona::FilaAspecto:
        case Zona::Menos:
        case Zona::Mas:
            if (m_hoja == Hoja::Aspecto) marcaFilaAspecto(indice);
            break;
        default: break;
    }
}

void CuadernoCreacion::pulsa(Zona z, int indice) {
    if (z != Zona::Buscar) m_nav.buscando = false;
    switch (z) {
        case Zona::Pestana:
        case Zona::Editar:
            if (indice >= 0 && indice < kHojas) irA(static_cast<Hoja>(indice));
            break;
        case Zona::Tesela:
            senala(z, indice);
            if (esRejilla(m_hoja)) elige(m_hoja, indice);
            break;
        case Zona::Rol:
            if (indice >= 0 && indice < static_cast<int>(roles().size()) && m_hoja == Hoja::Clase) {
                m_nav.rol = roles()[static_cast<std::size_t>(indice)].first;
                const std::vector<std::string> lista = ids();
                const auto it = std::find(lista.begin(), lista.end(), m_eleccion.classId);
                m_nav.foco[n(Hoja::Clase)] = it != lista.end() ? static_cast<int>(it - lista.begin()) : 0;
                m_nav.primera[n(Hoja::Clase)] = 0;
                ajustaScroll();
            }
            break;
        case Zona::Buscar:
            if (m_hoja == Hoja::Raza) m_nav.buscando = true;
            break;
        case Zona::Tarjeta:
            if (m_hoja == Hoja::Historia && indice >= 0) {
                m_nav.foco[n(m_hoja)] = indice;
                eligeHistoria(indice / kColumnas, indice % kColumnas);
            }
            break;
        case Zona::FilaAspecto: marcaFilaAspecto(indice); break;
        case Zona::Menos:
        case Zona::Mas:
            if (m_hoja == Hoja::Aspecto && m_editor) {
                marcaFilaAspecto(indice);
                if (z == Zona::Menos) m_editor->izquierda();
                else m_editor->derecha();
            }
            break;
        case Zona::Girar: m_nav.rumbo = ((m_nav.rumbo < 0 ? 2 : m_nav.rumbo) + (indice < 0 ? 3 : 1)) % 4; break;
        case Zona::Sugerencia:
            if (indice >= 0 && indice < static_cast<int>(sugerencias().size())) {
                m_eleccion.displayName = sugerencias()[static_cast<std::size_t>(indice)];
                m_nav.foco[n(Hoja::Nombre)] = indice;
                m_aviso.clear();
            }
            break;
        case Zona::Continuar: continuar(); break;
    }
}

void CuadernoCreacion::marcaFilaAspecto(int fila) {
    if (!m_editor || fila < 0 || fila >= static_cast<int>(m_editor->filas().size())) return;
    for (std::size_t vueltas = 0; static_cast<int>(m_editor->marcada()) != fila && vueltas < m_editor->filas().size();
         ++vueltas)
        m_editor->abajo();
    m_nav.foco[n(Hoja::Aspecto)] = static_cast<int>(m_editor->marcada());
}

}  // namespace Game
