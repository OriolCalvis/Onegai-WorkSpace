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

#include "Project/ProjectCatalog.h"

#include <filesystem>
#include <utility>

namespace Editor {

Result<ProjectCatalog> ProjectCatalog::load(const Project& project,
                                           const std::string& assetsRoot) {
    const auto valid = project.validate();
    if (!valid.isOk()) {
        return Result<ProjectCatalog>::Error("Proyecto " + project.id + ": " +
                                              valid.errorMessage());
    }
    ProjectCatalog result;
    for (const auto& name : project.catalogs) {
        const auto path = (std::filesystem::path(assetsRoot) /
                           project.settings.catalogsDirectory / name).generic_string();
        const auto loaded = result.objects.loadFromFile(path);
        if (!loaded.isOk()) {
            return Result<ProjectCatalog>::Error("Proyecto " + project.id +
                ", catalogo " + path + ": " + loaded.errorMessage());
        }
        result.paths.push_back(path);
    }
    return Result<ProjectCatalog>::Ok(std::move(result));
}

}  // namespace Editor
