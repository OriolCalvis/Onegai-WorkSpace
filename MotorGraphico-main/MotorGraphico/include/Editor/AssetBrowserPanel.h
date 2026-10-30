// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-13  (fase E, 4 j) - Navegador visual i importació de recursos
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Map/MapDocument.h"

namespace RPG {

// E-13 — AssetBrowserPanel.
//
// Panel de navegación de recursos: búsqueda por nombre/tipo,
// previsualización de dimensiones, importación de PNG con
// referencia relativa estable.

struct AssetInfo {
    std::string id;
    std::string name;
    std::string type;
    std::string path;
    int width = 0;
    int height = 0;
    int frames = 1;
};

class AssetBrowserPanel {
public:
    explicit AssetBrowserPanel(MapDocument* doc = nullptr);

    std::vector<AssetInfo> search(const std::string& query) const;
    Result<std::string> importPng(const std::string& filePath, const std::string& name);
    const AssetInfo* getAsset(const std::string& id) const;
    bool hasName(const std::string& name) const;
    size_t assetCount() const;

private:
    MapDocument* m_doc = nullptr;
    std::vector<AssetInfo> m_assets;
};

} // namespace RPG
