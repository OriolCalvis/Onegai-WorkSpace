// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-12  (fase E, 4 j) - Propietats de mapa, capes i instàncies
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
#include "Core/Math/GridCoord.h"
#include "Map/MapDocument.h"

namespace RPG {

// E-12 — MapPropertiesPanel.
//
// Panel de propiedades del mapa: nombre, dimensiones,
// tileset, entrada, opciones de luz/boira. Capas:
// crear, renombrar, ordenar, mostrar/ocultar y bloquear.

struct LayerInfo {
    std::string name;
    bool visible = true;
    bool locked = false;
    int order = 0;
};

class MapPropertiesPanel {
public:
    explicit MapPropertiesPanel(MapDocument* doc = nullptr);

    std::string mapName() const;
    void setMapName(const std::string& name);

    GridCoord dimensions() const;
    void setDimensions(const GridCoord& dim);

    std::string tileset() const;
    void setTileset(const std::string& tileset);

    bool createLayer(const std::string& name);
    bool renameLayer(const std::string& oldName, const std::string& newName);
    bool setLayerVisible(const std::string& name, bool visible);
    bool setLayerLocked(const std::string& name, bool locked);
    bool reorderLayer(const std::string& name, int newIndex);
    size_t layerCount() const;

private:
    MapDocument* m_doc = nullptr;
    std::vector<LayerInfo> m_layers;
    std::string m_mapName;
    GridCoord m_dimensions;
    std::string m_tileset;
};

} // namespace RPG
