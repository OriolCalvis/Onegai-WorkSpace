// FICHA-01 — La ficha del jugador como datos, sin interfaz ni GL.
//
// Lo que ve el jugador al abrir su ficha: quien es (raza, clase, trasfondo,
// tier), sus cuatro stats desglosados en base / raza / clase / equipo /
// dotes / condiciones = total, PV/PM/oro/inspiracion, las siete ranuras de
// equipo con lo que llevan, sus pasivas, dotes y rasgos, y las condiciones
// activas con rondas y acumulaciones. Todo sale de RPG::CharacterSheet (la
// fuente unica de verdad) y de los catalogos; aqui no se calcula ninguna
// regla, solo se lee y se nombra.
//
// Los iconos son ids del manifiesto assets/textures/iconos.json (ver
// HUD/Iconos.h); la condicion lleva su propio id y es el HUD quien decide
// si tiene icono propio o el generico de mejora/perjuicio.
#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "RPG/Catalogs/RpgCatalogs.h"

namespace RPG { class CharacterSheet; }

namespace Game {

struct FichaStat {
    std::string clave;   // "CON", "DES", "INT", "CAR"
    std::string icono;   // "con", "des", ...
    int base = 0, raza = 0, clase = 0, equipo = 0, dotes = 0, condiciones = 0;
    int total = 0;       // el valor que usa el motor (ya acotado 0..8)
};

struct FichaRanura {
    std::string ranura;    // "Head", "Torso", ... (nombre de RPG::EquipSlot)
    std::string etiqueta;  // "Cabeza", "Torso", ...
    std::string icono;     // "ranura_cabeza", ...
    std::string objetoId;  // id de EquipmentDefinition, vacio si no lleva nada
    std::string nombre;    // nombre de la pieza, "(vacio)" o "(desde el Tier N)"
    int abreEnTier = 0;    // PART-02: el tier que la abre si aun esta cerrada (0 = abierta)
};

struct FichaRasgo {
    std::string id, nombre, descripcion;
    std::string icono;  // "pasiva", "dote" o "rasgo"
};

struct FichaCondicion {
    std::string id, nombre, descripcion;
    int rondas = 0;
    int acumulaciones = 1;
    bool negativa = true;
};

struct FichaDatos {
    bool vacia = true;  // sin ficha vinculada (partida legacy)
    std::string nombre, raza, clase, trasfondo, deidad;
    int tier = 1;
    int nivel = 1, progresoNivel = 0;  // NIVELES-01: nivel y % hacia el siguiente
    int pv = 0, pvMax = 0, pm = 0, pmMax = 0, oro = 0, inspiracion = 0;
    std::array<FichaStat, 4> stats;
    std::vector<FichaRanura> ranuras;
    std::vector<FichaRasgo> rasgos;
    std::vector<FichaCondicion> condiciones;
};

// Punteros no propietarios; cualquiera puede ser nulo (entonces se muestra
// el id legible en vez del nombre del catalogo).
struct FichaCatalogos {
    const RPG::Catalogs::ClassCatalog* clases = nullptr;
    const RPG::Catalogs::RaceCatalog* razas = nullptr;
    const RPG::Catalogs::BackgroundCatalog* trasfondos = nullptr;
    const RPG::Catalogs::DeityCatalog* deidades = nullptr;
    const RPG::Catalogs::EquipmentCatalog* equipo = nullptr;
    const RPG::Catalogs::PassiveCatalog* pasivas = nullptr;
    const RPG::Catalogs::FeatCatalog* dotes = nullptr;
    const RPG::Catalogs::TraitCatalog* rasgos = nullptr;
    const RPG::Catalogs::ConditionCatalog* condiciones = nullptr;
};

class FichaVista {
public:
    FichaVista();
    ~FichaVista();

    // Carga desde <carpeta>/{classes,races,backgrounds,deities,equipment,
    // passives,feats,traits,conditions}.json. Un catalogo que falle queda
    // nulo y se sigue: la ficha se pinta igual, con ids en vez de nombres.
    // Devuelve error solo si no se pudo cargar NINGUNO.
    Result<bool> cargarCatalogos(const std::string& carpeta);
    // Sustituye los catalogos por otros ya cargados (Application, pruebas).
    void usarCatalogos(const FichaCatalogos& catalogos);
    const FichaCatalogos& catalogos() const { return m_catalogos; }

    // pv/pm vienen de fuera: los PV actuales viven en Player (ICombatant) y
    // el mana en SkillSet, no en la ficha.
    FichaDatos construir(const RPG::CharacterSheet& ficha, int pv, int pvMax, int pm,
                         int pmMax) const;

    // Version en texto (una linea por fila), para el HUD sin iconos y para
    // comparar en pruebas.
    static std::vector<std::string> lineas(const FichaDatos& datos);
    // "guerrera_de_la_marca" -> "Guerrera de la marca".
    static std::string nombreLegible(const std::string& id);
    static const char* etiquetaRanura(const std::string& ranura);

private:
    struct Propios;
    std::unique_ptr<Propios> m_propios;  // catalogos cargados por cargarCatalogos
    FichaCatalogos m_catalogos;
};

}  // namespace Game
