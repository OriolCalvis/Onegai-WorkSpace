// UI-BG3-01 — ver include/Game/CuadernoCreacion.h. Aqui: elegir, las
// preguntas de la historia, el kit de cada clase, los retratos de las rejillas, la vista
// previa del centro y la ficha final (sin equipo).
#include "Game/CuadernoCreacion.h"

#include <algorithm>
#include <cstdio>

#include "Core/Json/JsonValue.h"
#include "Game/NewGameFlow.h"

namespace Game {

using Hoja = CuadernoCreacion::Hoja;

namespace {
constexpr std::size_t n(Hoja h) { return static_cast<std::size_t>(h); }
}  // namespace

// --- Elegir ---------------------------------------------------------------

void CuadernoCreacion::elige(Hoja h, int indice) {
    const std::vector<std::string> lista = ids(h);
    if (indice < 0 || indice >= static_cast<int>(lista.size())) return;
    const std::string& id = lista[static_cast<std::size_t>(indice)];
    switch (h) {
        case Hoja::Raza:
            if (id != m_eleccion.raceId) {
                m_eleccion.raceId = id;
                if (m_editor) m_editor->empezar(id);
            }
            break;
        case Hoja::Clase: m_eleccion.classId = id; break;
        case Hoja::Origen:
            if (id != m_eleccion.backgroundId) {
                // Las siete preguntas salen de las listas del trasfondo.
                m_eleccion.backgroundId = id;
                for (int f = 0; f < kPreguntas; ++f) campoHistoria(f)->clear();
                m_nav.foco[n(Hoja::Historia)] = -1;
            }
            break;
        case Hoja::Deidad:
            m_eleccion.deityId = id;
            m_nav.deidadDecidida = true;
            break;
        default: break;
    }
    m_aviso.clear();
}

std::string* CuadernoCreacion::campoHistoria(int fila) {
    RPG::CreationChoice& e = m_eleccion;
    std::string* const campos[kPreguntas] = {&e.bondId, &e.fearId, &e.flawId, &e.goalId,
                                             &e.idealId, &e.personalityId, &e.virtueId};
    return campos[std::clamp(fila, 0, kPreguntas - 1)];
}

const std::string& CuadernoCreacion::elegidaHistoria(int fila) const {
    const RPG::CreationChoice& e = m_eleccion;
    const std::string* const campos[kPreguntas] = {&e.bondId, &e.fearId, &e.flawId, &e.goalId,
                                                   &e.idealId, &e.personalityId, &e.virtueId};
    return *campos[std::clamp(fila, 0, kPreguntas - 1)];
}

const std::vector<RPG::CharacterCreation::OpcionNarrativa>* CuadernoCreacion::opciones(int fila) const {
    const RPG::CharacterCreation::OpcionesTrasfondo* o = m_datos.reglas->opcionesDe(m_eleccion.backgroundId);
    if (o == nullptr || o->vacio()) return nullptr;
    switch (fila) {
        case 0: return &o->bonds;
        case 1: return &o->fears;
        case 2: return &o->flaws;
        case 3: return &o->goals;
        case 4: return &o->ideals;
        case 5: return &o->personalities;
        case 6: return &o->virtues;
        default: return nullptr;
    }
}

void CuadernoCreacion::eligeHistoria(int fila, int columna) {
    const std::vector<RPG::CharacterCreation::OpcionNarrativa>* ops = opciones(fila);
    if (ops == nullptr || columna < 0 || columna >= static_cast<int>(ops->size())) return;
    *campoHistoria(fila) = (*ops)[static_cast<std::size_t>(columna)].id;
    m_aviso.clear();
}

// --- Vista previa, retratos y kit -------------------------------------------

const KitBasico::Kit& CuadernoCreacion::kit(const std::string& claseId) const {
    static const KitBasico::Kit kVacio;
    const auto it = m_kits.find(claseId);
    if (it != m_kits.end()) return it->second;
    const RPG::ClassDefinition* clase = m_datos.reglas->classes().find(claseId);
    if (clase == nullptr || m_datos.equipo == nullptr || m_datos.kit == nullptr || m_datos.tiers == nullptr)
        return kVacio;
    return m_kits
        .emplace(claseId, KitBasico::deClase(*clase, *m_datos.equipo, *m_datos.tiers, *m_datos.kit, m_datos.personajes))
        .first->second;
}

CuadernoCreacion::Vista CuadernoCreacion::retrato(Hoja h, const std::string& id) const {
    Vista v;
    if (m_datos.personajes == nullptr) return v;
    std::string raza = h == Hoja::Raza ? id : m_eleccion.raceId;
    if (raza.empty()) {
        // Sin raza elegida, las clases se ensenan sobre la primera de la lista.
        const std::vector<std::string> razas = ids(Hoja::Raza);
        if (!razas.empty()) raza = razas.front();
    }
    const RazaVisual* rv = m_datos.personajes->raza(raza);
    if (rv == nullptr) return v;
    v.apariencia = (h != Hoja::Raza && m_editor && m_editor->apariencia().raza == raza) ? m_editor->apariencia()
                                                                                        : Apariencia::deRaza(*rv);
    if (h == Hoja::Clase) {
        // AV16-02: todo empieza a 0; solo se destacan los atributos principales de la clase.
        if (const RPG::ClassDefinition* clase = m_datos.reglas->classes().find(id)) {
            for (const std::string& s : {clase->primaryStat, clase->secondaryStat}) {
                const std::size_t i = s == "CON" ? 0 : s == "DES" ? 1 : s == "INT" ? 2 : s == "CAR" ? 3 : 4;
                if (i < 4) v.principales[i] = true;
            }
        }
        const KitBasico::Kit& k = kit(id);
        v.kit = k.bonos;
        v.conKit = !k.piezas.empty();
        for (const std::string& pieza : k.piezas) {
            const auto it = m_datos.personajes->piezaDeObjeto.find(pieza);
            if (it != m_datos.personajes->piezaDeObjeto.end()) v.equipo[it->second.ranura] = it->second.visible();
        }
    }
    return v;
}

CuadernoCreacion::Vista CuadernoCreacion::vista() const {
    // En Raza se ensena el pueblo senalado; en Clase, tu raza con el kit de
    // la clase senalada; en el resto, lo elegido (con su kit desde Clase).
    if (m_hoja == Hoja::Raza) {
        const std::string s = senalado();
        if (s.empty()) return Vista{};
        Vista v = retrato(Hoja::Raza, s);
        if (s == m_eleccion.raceId && m_editor && m_editor->apariencia().raza == s) v.apariencia = m_editor->apariencia();
        return v;
    }
    const std::string clase = m_hoja == Hoja::Clase ? senalado() : m_eleccion.classId;
    if (m_hoja == Hoja::Clase && !clase.empty()) return retrato(Hoja::Clase, clase);
    if (m_eleccion.raceId.empty()) return Vista{};
    if (m_hoja == Hoja::Aspecto || clase.empty()) return retrato(Hoja::Origen, "");
    return retrato(Hoja::Clase, clase);
}

double CuadernoCreacion::segundos() const {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - m_inicio).count();
}

Result<RPG::CharacterSheet> CuadernoCreacion::finalizar() const {
    const std::vector<std::string> p = problemas();
    if (!p.empty()) return Result<RPG::CharacterSheet>::Error(p.front());
    return NewGameFlow::fichaSinEquipo(*m_datos.reglas, m_eleccion, *m_datos.tiers);
}

std::map<std::string, CuadernoCreacion::Rasgo> CuadernoCreacion::leeRasgos(const std::string& ruta) {
    std::map<std::string, Rasgo> out;
    FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) return out;
    std::string texto;
    char trozo[8192];
    for (std::size_t leidos; (leidos = std::fread(trozo, 1, sizeof trozo, f)) > 0;) texto.append(trozo, leidos);
    std::fclose(f);
    auto json = JsonValue::parse(texto);
    if (!json.isOk()) return out;
    const JsonValue& entradas = json.value()["entries"];
    for (std::size_t i = 0; i < entradas.size(); ++i) {
        const JsonValue& e = entradas[i];
        const JsonValue& rasgo = e["passiveTrait"];
        if (!rasgo.isObject()) continue;
        out[e["id"].asString()] = Rasgo{rasgo["name"].asString(), rasgo["description"].asString()};
    }
    return out;
}

std::string CuadernoCreacion::culturaDe(const std::string& d) {
    const std::size_t edad = d.find(" Edad:");
    if (edad == std::string::npos) return d;
    std::string c = d.substr(0, edad);
    const std::size_t religion = d.find("Religión/cultura:", edad);
    const std::size_t fin = religion == std::string::npos ? std::string::npos : d.find(". ", religion);
    if (fin != std::string::npos) c += " " + d.substr(fin + 2);
    while (!c.empty() && c.back() == ' ') c.pop_back();
    return c;
}

}  // namespace Game
