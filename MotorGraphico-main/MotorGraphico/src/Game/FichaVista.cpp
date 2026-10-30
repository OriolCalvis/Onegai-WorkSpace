// FICHA-01 — ver include/Game/FichaVista.h.
#include "Game/FichaVista.h"

#include <cctype>
#include <string>

#include "RPG/CharacterSheet.h"

namespace Game {

namespace {

// Por el orden de RPG::EquipSlot (PART-02: los brazos, el collar, el cinturon y el segundo anillo detras).
const char* const kRanuras[] = {"Head", "Torso", "Legs", "Feet", "MainHand", "OffHand", "Accessory",
                                "Arms", "Neck", "Belt", "Ring2"};
const char* const kEtiquetas[] = {"Cabeza", "Torso", "Piernas", "Pies", "Mano principal",
                                  "Mano izquierda", "Anillo", "Brazos", "Collar", "Cinturon",
                                  "Otro anillo"};
const char* const kIconosRanura[] = {"ranura_cabeza", "ranura_torso", "ranura_piernas",
                                     "ranura_pies", "ranura_mano_principal",
                                     "ranura_mano_izquierda", "ranura_accesorio", "ranura_torso",
                                     "ranura_accesorio", "ranura_accesorio", "ranura_accesorio"};
static_assert(sizeof(kRanuras) / sizeof(kRanuras[0]) == RPG::kRanurasDeEquipo, "una fila por ranura");
const char* const kStats[] = {"CON", "DES", "INT", "CAR"};
const char* const kIconosStat[] = {"con", "des", "int", "car"};

template <typename Catalogo>
std::string nombreDe(const Catalogo* catalogo, const std::string& id) {
    if (id.empty()) return "";
    if (catalogo != nullptr) {
        if (const auto* def = catalogo->find(id); def != nullptr && !def->name.empty()) {
            return def->name;
        }
    }
    return FichaVista::nombreLegible(id);
}

template <typename Catalogo>
void anadirRasgos(std::vector<FichaRasgo>& salida, const Catalogo* catalogo,
                  const std::vector<std::string>& ids, const char* icono) {
    for (const std::string& id : ids) {
        FichaRasgo rasgo;
        rasgo.id = id;
        rasgo.icono = icono;
        rasgo.nombre = nombreDe(catalogo, id);
        if (catalogo != nullptr) {
            if (const auto* def = catalogo->find(id)) rasgo.descripcion = def->description;
        }
        salida.push_back(std::move(rasgo));
    }
}

}  // namespace

struct FichaVista::Propios {
    RPG::Catalogs::ClassCatalog clases;
    RPG::Catalogs::RaceCatalog razas;
    RPG::Catalogs::BackgroundCatalog trasfondos;
    RPG::Catalogs::DeityCatalog deidades;
    RPG::Catalogs::EquipmentCatalog equipo;
    RPG::Catalogs::PassiveCatalog pasivas;
    RPG::Catalogs::FeatCatalog dotes;
    RPG::Catalogs::TraitCatalog rasgos;
    RPG::Catalogs::ConditionCatalog condiciones;
};

FichaVista::FichaVista() = default;
FichaVista::~FichaVista() = default;

Result<bool> FichaVista::cargarCatalogos(const std::string& carpeta) {
    m_propios = std::make_unique<Propios>();
    Propios& p = *m_propios;
    int cargados = 0;
    std::string errores;
    auto carga = [&](auto& catalogo, const char* fichero, auto*& destino) {
        const auto r = catalogo.loadFromFile(carpeta + "/" + fichero);
        if (r.isOk()) {
            destino = &catalogo;
            ++cargados;
        } else {
            destino = nullptr;
            errores += (errores.empty() ? "" : "; ") + r.errorMessage();
        }
    };
    carga(p.clases, "classes.json", m_catalogos.clases);
    carga(p.razas, "races.json", m_catalogos.razas);
    carga(p.trasfondos, "backgrounds.json", m_catalogos.trasfondos);
    carga(p.deidades, "deities.json", m_catalogos.deidades);
    carga(p.equipo, "equipment.json", m_catalogos.equipo);
    carga(p.pasivas, "passives.json", m_catalogos.pasivas);
    carga(p.dotes, "feats.json", m_catalogos.dotes);
    carga(p.rasgos, "traits.json", m_catalogos.rasgos);
    carga(p.condiciones, "conditions.json", m_catalogos.condiciones);
    if (cargados == 0) return Result<bool>::Error("FichaVista: ningun catalogo en " + carpeta + ": " + errores);
    return Result<bool>::Ok(true);
}

void FichaVista::usarCatalogos(const FichaCatalogos& catalogos) { m_catalogos = catalogos; }

const char* FichaVista::etiquetaRanura(const std::string& ranura) {
    for (std::size_t i = 0; i < RPG::kRanurasDeEquipo; ++i) {
        if (ranura == kRanuras[i]) return kEtiquetas[i];
    }
    return "?";
}

std::string FichaVista::nombreLegible(const std::string& id) {
    std::string out;
    out.reserve(id.size());
    bool primera = true;
    for (char c : id) {
        if (c == '_' || c == '-') {
            out.push_back(' ');
            continue;
        }
        out.push_back(primera ? static_cast<char>(std::toupper(static_cast<unsigned char>(c))) : c);
        primera = false;
    }
    return out;
}

FichaDatos FichaVista::construir(const RPG::CharacterSheet& ficha, int pv, int pvMax, int pm,
                                 int pmMax) const {
    FichaDatos d;
    d.vacia = false;
    d.nombre = ficha.displayName.empty() ? nombreLegible(ficha.id) : ficha.displayName;
    d.raza = nombreDe(m_catalogos.razas, ficha.raceId);
    d.clase = nombreDe(m_catalogos.clases, ficha.classId);
    if (!ficha.secondClassId.empty()) d.clase += " / " + nombreDe(m_catalogos.clases, ficha.secondClassId);
    d.trasfondo = nombreDe(m_catalogos.trasfondos, ficha.backgroundId);
    d.deidad = nombreDe(m_catalogos.deidades, ficha.deityId);
    d.tier = ficha.tier;
    d.nivel = ficha.nivel;
    d.progresoNivel = ficha.progresoNivel;
    d.pv = pv;
    d.pvMax = pvMax;
    d.pm = pm;
    d.pmMax = pmMax;
    d.oro = ficha.gold;
    d.inspiracion = ficha.inspirationPoints;

    for (int i = 0; i < 4; ++i) {
        FichaStat& s = d.stats[static_cast<std::size_t>(i)];
        s.clave = kStats[i];
        s.icono = kIconosStat[i];
        s.base = ficha.baseStats[static_cast<std::size_t>(i)];
        s.raza = ficha.racialBonuses[static_cast<std::size_t>(i)];
        s.clase = ficha.classBonuses[static_cast<std::size_t>(i)];
        s.equipo = ficha.equipBonuses[static_cast<std::size_t>(i)];
        s.dotes = ficha.featBonuses[static_cast<std::size_t>(i)];
        s.condiciones = ficha.conditionModifiers[static_cast<std::size_t>(i)];
        s.total = ficha.stat(RPG::stat_from_index(i));
    }

    for (std::size_t i = 0; i < RPG::kRanurasDeEquipo; ++i) {
        FichaRanura r;
        r.ranura = kRanuras[i];
        r.etiqueta = kEtiquetas[i];
        r.icono = kIconosRanura[i];
        r.objetoId = ficha.equippedIds[i];
        // PART-02: las que el tier aun no abre (los anillos, el collar y el cinturon, desde el 2)
        const auto ranura = static_cast<RPG::EquipSlot>(i);
        if (!RPG::ranuraAbierta(ranura, ficha.tier)) r.abreEnTier = RPG::tierDeRanura(ranura);
        r.nombre = !r.objetoId.empty()                     ? nombreDe(m_catalogos.equipo, r.objetoId)
                   : !RPG::ranuraAbierta(ranura, ficha.tier) ? "(desde el Tier " + std::to_string(RPG::tierDeRanura(ranura)) + ")"
                                                            : "(vacio)";
        d.ranuras.push_back(std::move(r));
    }

    anadirRasgos(d.rasgos, m_catalogos.pasivas, ficha.passiveIds, "pasiva");
    anadirRasgos(d.rasgos, m_catalogos.dotes, ficha.featIds, "dote");
    anadirRasgos(d.rasgos, m_catalogos.rasgos, ficha.traitIds, "rasgo");

    for (const RPG::ActiveCondition& activa : ficha.conditions) {
        FichaCondicion c;
        c.id = activa.conditionId;
        c.rondas = activa.roundsRemaining;
        c.acumulaciones = activa.stacks;
        c.nombre = nombreDe(m_catalogos.condiciones, c.id);
        if (m_catalogos.condiciones != nullptr) {
            if (const auto* def = m_catalogos.condiciones->find(c.id)) {
                c.descripcion = def->description;
                c.negativa = def->isNegative;
            }
        }
        d.condiciones.push_back(std::move(c));
    }
    return d;
}

std::vector<std::string> FichaVista::lineas(const FichaDatos& d) {
    std::vector<std::string> out;
    if (d.vacia) {
        out.push_back("(sin ficha)");
        return out;
    }
    std::string cabecera = d.nombre + "  Nivel " + std::to_string(d.nivel);
    if (d.progresoNivel > 0) cabecera += " (" + std::to_string(d.progresoNivel) + " %)";
    out.push_back(cabecera + "  T" + std::to_string(d.tier));
    std::string quien = d.raza;
    if (!d.clase.empty()) quien += (quien.empty() ? "" : " - ") + d.clase;
    if (!d.trasfondo.empty()) quien += (quien.empty() ? "" : " - ") + d.trasfondo;
    if (!quien.empty()) out.push_back(quien);
    out.push_back("PV " + std::to_string(d.pv) + "/" + std::to_string(d.pvMax) + "  PM " +
                  std::to_string(d.pm) + "/" + std::to_string(d.pmMax) + "  Oro " +
                  std::to_string(d.oro) + "  Insp " + std::to_string(d.inspiracion));
    for (const FichaStat& s : d.stats) {
        const int mods = s.raza + s.clase + s.equipo + s.dotes + s.condiciones;
        std::string linea = s.clave + " " + std::to_string(s.total) + "  (base " +
                            std::to_string(s.base);
        if (mods != 0) linea += (mods > 0 ? " +" : " ") + std::to_string(mods);
        linea += ")";
        out.push_back(linea);
    }
    for (const FichaRanura& r : d.ranuras) out.push_back(r.etiqueta + ": " + r.nombre);
    for (const FichaRasgo& r : d.rasgos) out.push_back(nombreLegible(r.icono) + ": " + r.nombre);
    for (const FichaCondicion& c : d.condiciones) {
        std::string linea = (c.negativa ? "- " : "+ ") + c.nombre;
        if (c.rondas > 0) linea += " (" + std::to_string(c.rondas) + " rondas)";
        if (c.acumulaciones > 1) linea += " x" + std::to_string(c.acumulaciones);
        out.push_back(linea);
    }
    return out;
}

}  // namespace Game
