// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-16  (fase E, 8 j) - Edició RPG mínima del projecte
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Json/JsonValue.h"
#include "Core/Ids/ProjectIds.h"
#include "Core/Math/GridCoord.h"
#include "Editor/CatalogPickerState.h"
#include "Editor/EventEditorState.h"
#include "RPG/Catalogs/RpgCatalogs.h"

namespace RPG {

// E-16 — Edició RPG mínima del projecte.
//
// Formularis per crear objectes, equip, habilitats i enemics
// des de l'editor. Cada entitat té camps editables i es pot
// duplicar, validar i desar sense tocar el codi del joc.

struct ProjectDatabaseEntry {
    std::string id;
    std::string name;
    std::string type;         // objecte, equip, habilitat, enemic
    JsonValue fields;         // camps del formulari
    bool valid = false;
};

class ProjectDatabaseState {
public:
    explicit ProjectDatabaseState(MapDocument* doc = nullptr);

    // Crear una entitat del tipus indicat. Retorna l'ID.
    Result<std::string> createEntry(const std::string& type);

    // Duplicar una entitat existent. Retorna el nou ID.
    Result<std::string> duplicateEntry(const std::string& entryId);

    // Actualitzar un camp d'una entitat.
    bool updateField(const std::string& entryId, const std::string& field, const JsonValue& value);

    // Validar una entitat. Retorna true si és vàlida.
    bool validateEntry(const std::string& entryId);

    // Desar totes les entitats al JSON del projecte.
    bool save(const std::string& projectPath);

    // Carregar entitats des del JSON del projecte.
    bool load(const std::string& projectPath);

    // Obtenir el nombre d'entitats d'un tipus.
    size_t count(const std::string& type) const;

    // Obtenir una entitat per ID.
    const ProjectDatabaseEntry* getEntry(const std::string& id) const;

    // Verificar que un ID no col·lideix.
    bool hasId(const std::string& id) const;

private:
    MapDocument* m_doc = nullptr;
    std::vector<ProjectDatabaseEntry> m_entries;
};

} // namespace RPG
