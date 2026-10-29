// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   D-05  (fase D, 3 j) - Objectes, equipament i consumibles
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <string>
#include <vector>

#include "Level/ObjectCatalog.h"
#include "Project/Project.h"

namespace Editor {

// Datos y rutas del mismo manifiesto, para que paleta y playtest usen
// exactamente los mismos catálogos. Sin catálogos, el proyecto queda vacío.
struct ProjectCatalog {
    ObjectCatalog objects;
    std::vector<std::string> paths;

    // Respeta settings.catalogsDirectory y el orden de overrides declarado.
    // Un error devuelve el proyecto/ruta afectados, nunca una carga parcial.
    static Result<ProjectCatalog> load(const Project& project,
                                       const std::string& assetsRoot = "assets");
};

}  // namespace Editor
