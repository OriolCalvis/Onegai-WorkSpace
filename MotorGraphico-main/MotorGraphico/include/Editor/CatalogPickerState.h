// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-14  (fase E, 3 j) - Selectors de dades i referències
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"

// E-14 — Selectors de dades i referències.
//
// QUÉ RESUELVE. Molts llocs de l'editor necessiten referenciar una entrada
// de catàleg (un objecte, una habilitat, un enemic, una condició...): el
// inspector d'objectes (E-12), el formulari d'esdeveniments (E-05/E-06), el
// panell de Problemes (E-15). Sense un selector comú, l'usuari ha d'escriure
// l'ID a mà —error garantit, tutorial impossible—.
//
// COM FUNCIONA. Un picker mostra una llista filtrable d'entrades d'un
// catàleg concret, amb nom visible i ID intern. L'usuari cerca per nom
// (plegat: minúscules, sense accents —DEC-15—), navega amb fletxes i
// selecciona amb Enter. El picker no és duenó de res: rep una vista del
// catàleg (un vector d'entries) i torna l'ID seleccionat.
//
// GL-FREE. Igual que AssetBrowserState i ProblemsState: l'estat viu a
// include/Editor + src/Editor i es prova sense finestra. El panel ImGui
// que el dibuixa viu a editor/ i consumeix aquesta API.
//
// SELECCIÓ PER IDENTITAT. Com AssetBrowserState: la selecció es guarda
// per id, no per posició. Si la llista canvia (per cerca o filtre), la
// selecció es resol de nou; si l'id ja no hi és, es diu "Lost".

namespace RPG {

// Tipus de catàleg que el picker pot mostrar. Cada tipus correspon a un
// o més fitxers JSON d'assets/catalogs/.
enum class CatalogType {
    Object,
    Skill,
    Monster,
    Condition,
    Npc,
    Location,
    Map,  // alias per compatibilitat amb tests existents
    Adventure,
    Event,
    Equipment,
    Consumable,
    Spell,
    Trap,
    Deity,
    Mount,
    Summon,
    LootTable,
    Race,
    Class,
    Background,
    Feat,
    Trait,
    Passive,
    PregenCharacter,
    Count,
};

// Una entrada de catàleg simplificada per al picker.
struct CatalogEntry {
    std::string id;
    std::string name;
    CatalogType type = CatalogType::Object;
};

// Selector de dades genèric. Rep entrades d'un catàleg (o de varis
// combinats) i permet cercar, filtrar per tipus i seleccionar.
//
// No carrega JSON directament: qui construeix el picker alimenta
// addEntries() amb les entries que necessiti. Això desacopla el picker
// del format de cada catàleg (cada D-02..D-07 tenia el seu loader) i
// permet provar-lo amb dades inventades.
class CatalogPickerState {
public:
    CatalogPickerState() = default;

    // --- Alimentació ---
    // Afegeix entries al catàleg intern. Un id repetit SOBREESCRIBE
    // l'entrada anterior (mateix criteri que ObjectCatalog::add).
    void addEntries(const std::vector<CatalogEntry>& entries);
    void addEntry(CatalogEntry entry);
    std::size_t entryCount() const { return m_entries.size(); }

    // --- Càrrega des de fitxer ---
    // Carrega tots els catàleg JSON del directori donat
    // (assets/catalogs/). Retorna false si algun fitxer no es pot
    // obrir o no és JSON vàlid (els errors silenciosos són pitjors
    // que els visibles). El contingut es deduplica per id.
    bool loadCatalogs(const std::string& assetsDir);

    // --- Cerca i filtre ---
    // Cerca entries d'un tipus concret que continguin el query en el
    // nom (plegat). Query buida = totes les entries del tipus.
    std::vector<CatalogEntry> search(CatalogType type,
                                      const std::string& query) const;

    // --- Selecció ---
    // Selecciona una entrada per tipus + id. Retorna Ok si existeix,
    // Error si no (id inexistent o tipus incompatible).
    Result<CatalogEntry> select(CatalogType type,
                                 const std::string& id) const;

    // Comprova si una referència (id) existeix dins d'un tipus.
    // Util per a validació de referències perdudes (E-12/E-15).
    bool isValidReference(CatalogType type, const std::string& id) const;

    // --- Accés ---
    const std::vector<CatalogEntry>& entries() const { return m_entries; }
    const CatalogEntry* findById(const std::string& id) const;

private:
    std::vector<CatalogEntry> m_entries;
};

}  // namespace RPG

// NOTA DE UBICACIÓ. Igual que AssetBrowserState i ProblemsState: això
// viu en include/Editor + src/Editor i no en editor/ (la capa ImGui).
// No toca GL; es prova sense finestra. Quan el panell del selector ho
// necessiti, el mou a la llibreria.
