// H2-06 — Las cartas de clase, sus pasivas, las condiciones que aplican y las
// invocaciones que traen: todo lo que el combate de cartas necesita leer.
//
// El juego ya cargaba skills.json (m_nd6Skills, para los skillCheck de la
// narrativa) y conditions.json, pero no passives.json ni summons.json, y nada
// traducia el texto de una carta a lo que hace (ver Game/GanchosCarta.h). Este
// catalogo lo junta: se carga UNA vez al arrancar (Application) y la sesion se
// lo pasa a cada combate. Las cartas guardan sus ganchos ya leidos, para no
// volver a partir cadenas en cada turno.
//
// GL-free. Sin estado de partida: el estado de un combate vive en
// Game::CartasEnCombate.
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Errors/Result.h"
#include "Game/GanchosCarta.h"
#include "RPG/Catalogs/RpgCatalogs.h"

namespace Game {

class CatalogoCartas {
public:
    struct Carta {
        const RPG::SkillDefinition* def = nullptr;
        std::vector<Gancho> ganchos;
        std::string bloqueo;  // motivoBloqueo(): "" = se puede jugar
    };

    // skills.json, passives.json, conditions.json y summons.json de la carpeta.
    static Result<std::unique_ptr<CatalogoCartas>> cargar(const std::string& carpeta);

    // Para pruebas: catalogos ya rellenos.
    CatalogoCartas(RPG::Catalogs::SkillCatalog cartas, RPG::Catalogs::PassiveCatalog pasivas,
                   RPG::Catalogs::ConditionCatalog condiciones,
                   RPG::Catalogs::SummonCatalog invocaciones);

    const Carta* carta(const std::string& id) const;
    const RPG::PassiveDefinition* pasiva(const std::string& id) const;
    const std::vector<Gancho>& ganchosDePasiva(const std::string& id) const;
    const RPG::ConditionDefinition* condicion(const std::string& id) const;
    const std::vector<Gancho>& ganchosDeCondicion(const std::string& id) const;
    const RPG::SummonDefinition* invocacion(const std::string& id) const;
    const RPG::Catalogs::ConditionCatalog& condiciones() const { return m_condiciones; }

private:
    RPG::Catalogs::SkillCatalog m_cartas;
    RPG::Catalogs::PassiveCatalog m_pasivas;
    RPG::Catalogs::ConditionCatalog m_condiciones;
    RPG::Catalogs::SummonCatalog m_invocaciones;
    std::unordered_map<std::string, Carta> m_leidas;
    std::unordered_map<std::string, std::vector<Gancho>> m_ganchosPasiva;
    std::unordered_map<std::string, std::vector<Gancho>> m_ganchosCondicion;
};

}  // namespace Game
