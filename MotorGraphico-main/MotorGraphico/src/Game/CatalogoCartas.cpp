// H2-06 — ver include/Game/CatalogoCartas.h.
#include "Game/CatalogoCartas.h"

#include <utility>

namespace Game {
namespace {

template <typename T>
std::string cargaUno(RPG::Catalogs::Catalog<T>& catalogo, const std::string& ruta) {
    const auto r = catalogo.loadFromFile(ruta);
    return r.isOk() ? std::string() : ruta + ": " + r.errorMessage();
}

const std::vector<Gancho>& sinGanchos() {
    static const std::vector<Gancho> vacio;
    return vacio;
}

}  // namespace

Result<std::unique_ptr<CatalogoCartas>> CatalogoCartas::cargar(const std::string& carpeta) {
    using R = Result<std::unique_ptr<CatalogoCartas>>;
    RPG::Catalogs::SkillCatalog cartas;
    RPG::Catalogs::PassiveCatalog pasivas;
    RPG::Catalogs::ConditionCatalog condiciones;
    RPG::Catalogs::SummonCatalog invocaciones;
    for (const std::string& error :
         {cargaUno(cartas, carpeta + "/skills.json"), cargaUno(pasivas, carpeta + "/passives.json"),
          cargaUno(condiciones, carpeta + "/conditions.json"),
          cargaUno(invocaciones, carpeta + "/summons.json")}) {
        if (!error.empty()) return R::Error("Cartas de combate: " + error);
    }
    return R::Ok(std::make_unique<CatalogoCartas>(std::move(cartas), std::move(pasivas),
                                                  std::move(condiciones),
                                                  std::move(invocaciones)));
}

CatalogoCartas::CatalogoCartas(RPG::Catalogs::SkillCatalog cartas,
                               RPG::Catalogs::PassiveCatalog pasivas,
                               RPG::Catalogs::ConditionCatalog condiciones,
                               RPG::Catalogs::SummonCatalog invocaciones)
    : m_cartas(std::move(cartas)),
      m_pasivas(std::move(pasivas)),
      m_condiciones(std::move(condiciones)),
      m_invocaciones(std::move(invocaciones)) {
    m_cartas.forEach([this](const RPG::SkillDefinition& def) {
        Carta c;
        c.def = &def;
        c.ganchos = leerGanchos(def.mechanicHooks);
        c.bloqueo = motivoBloqueo(c.ganchos);
        m_leidas.emplace(def.id, std::move(c));
    });
    m_pasivas.forEach([this](const RPG::PassiveDefinition& def) {
        m_ganchosPasiva.emplace(def.id, leerGanchos(def.mechanicHooks));
    });
    m_condiciones.forEach([this](const RPG::ConditionDefinition& def) {
        m_ganchosCondicion.emplace(def.id, leerGanchos(def.mechanicHooks));
    });
}

const CatalogoCartas::Carta* CatalogoCartas::carta(const std::string& id) const {
    const auto it = m_leidas.find(id);
    return it == m_leidas.end() ? nullptr : &it->second;
}

const RPG::PassiveDefinition* CatalogoCartas::pasiva(const std::string& id) const {
    return m_pasivas.find(id);
}

const std::vector<Gancho>& CatalogoCartas::ganchosDePasiva(const std::string& id) const {
    const auto it = m_ganchosPasiva.find(id);
    return it == m_ganchosPasiva.end() ? sinGanchos() : it->second;
}

const RPG::ConditionDefinition* CatalogoCartas::condicion(const std::string& id) const {
    return m_condiciones.find(id);
}

const std::vector<Gancho>& CatalogoCartas::ganchosDeCondicion(const std::string& id) const {
    const auto it = m_ganchosCondicion.find(id);
    return it == m_ganchosCondicion.end() ? sinGanchos() : it->second;
}

const RPG::SummonDefinition* CatalogoCartas::invocacion(const std::string& id) const {
    return m_invocaciones.find(id);
}

}  // namespace Game
