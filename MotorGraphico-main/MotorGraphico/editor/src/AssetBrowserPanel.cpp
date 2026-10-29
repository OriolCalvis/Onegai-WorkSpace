// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo toca 1 paquete del corte B:
//   E-13  (fase E, 4 j) - Navegador visual i importació de recursos
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "Editor/AssetBrowserPanel.h"
#include "Editor/CatalogPickerState.h"
#include "Map/MapDocument.h"

#include <string>
#include <vector>

namespace RPG {

AssetBrowserPanel::AssetBrowserPanel(MapDocument* doc) : m_doc(doc) {}

std::vector<AssetInfo> AssetBrowserPanel::search(const std::string& query) const {
    std::vector<AssetInfo> result;
    for (const auto& a : m_assets) {
        if (a.name.find(query) != std::string::npos || a.id.find(query) != std::string::npos) {
            result.push_back(a);
        }
    }
    return result;
}

Result<std::string> AssetBrowserPanel::importPng(const std::string& filePath, const std::string& name) {
    if (hasName(name)) {
        return Result<std::string>::Error("Nombre duplicado: " + name);
    }
    std::string id = "asset_" + std::to_string(m_assets.size());
    AssetInfo info;
    info.id = id;
    info.name = name;
    info.type = "png";
    info.path = filePath;
    info.width = 0;
    info.height = 0;
    info.frames = 1;
    m_assets.push_back(info);
    return Result<std::string>::Ok(id);
}

const AssetInfo* AssetBrowserPanel::getAsset(const std::string& id) const {
    for (const auto& a : m_assets) {
        if (a.id == id) return &a;
    }
    return nullptr;
}

bool AssetBrowserPanel::hasName(const std::string& name) const {
    for (const auto& a : m_assets) {
        if (a.name == name) return true;
    }
    return false;
}

size_t AssetBrowserPanel::assetCount() const { return m_assets.size(); }

} // namespace RPG