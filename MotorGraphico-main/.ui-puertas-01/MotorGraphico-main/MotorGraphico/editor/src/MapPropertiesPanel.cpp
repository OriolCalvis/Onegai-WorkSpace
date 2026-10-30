// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo toca 1 paquete del corte B:
//   E-12  (fase E, 4 j) - Propietats de mapa, capes i instàncies
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "Editor/ObjectInspector.h"
#include "Editor/EventEditorState.h"

#include <string>
#include <vector>

namespace RPG {

// E-12 — MapPropertiesPanel implementation.
//
// Panel de propiedades del mapa: nombre, dimensiones, tileset,
// entrada, opciones de luz/boira. Capas: crear, renombrar,
// ordenar, mostrar/ocultar y bloquear.

struct LayerInfo {
    std::string name;
    bool visible = true;
    bool locked = false;
    int order = 0;
};

class MapPropertiesPanel {
public:
    explicit MapPropertiesPanel(MapDocument* doc = nullptr);

    // Obtener el nombre del mapa.
    std::string mapName() const;

    // Establecer el nombre del mapa.
    void setMapName(const std::string& name);

    // Obtener las dimensiones.
    GridCoord dimensions() const;

    // Establecer las dimensiones.
    void setDimensions(const GridCoord& dim);

    // Obtener el tileset.
    std::string tileset() const;

    // Establecer el tileset.
    void setTileset(const std::string& tileset);

    // Crear una capa.
    bool createLayer(const std::string& name);

    // Renombrar una capa.
    bool renameLayer(const std::string& oldName, const std::string& newName);

    // Ocultar/mostrar una capa.
    bool setLayerVisible(const std::string& name, bool visible);

    // Bloquear/desbloquear una capa.
    bool setLayerLocked(const std::string& name, bool locked);

    // Reordenar una capa.
    bool reorderLayer(const std::string& name, int newIndex);

    // Obtener el número de capas.
    size_t layerCount() const;

private:
    MapDocument* m_doc = nullptr;
    std::vector<LayerInfo> m_layers;
    std::string m_mapName;
    GridCoord m_dimensions;
    std::string m_tileset;
};

MapPropertiesPanel::MapPropertiesPanel(MapDocument* doc) : m_doc(doc) {}

std::string MapPropertiesPanel::mapName() const { return m_mapName; }
void MapPropertiesPanel::setMapName(const std::string& name) { m_mapName = name; }
GridCoord MapPropertiesPanel::dimensions() const { return m_dimensions; }
void MapPropertiesPanel::setDimensions(const GridCoord& dim) { m_dimensions = dim; }
std::string MapPropertiesPanel::tileset() const { return m_tileset; }
void MapPropertiesPanel::setTileset(const std::string& t) { m_tileset = t; }

bool MapPropertiesPanel::createLayer(const std::string& name) {
    for (const auto& l : m_layers) {
        if (l.name == name) return false;
    }
    LayerInfo l;
    l.name = name;
    l.order = static_cast<int>(m_layers.size());
    m_layers.push_back(l);
    return true;
}

bool MapPropertiesPanel::renameLayer(const std::string& oldName, const std::string& newName) {
    for (auto& l : m_layers) {
        if (l.name == oldName) {
            l.name = newName;
            return true;
        }
    }
    return false;
}

bool MapPropertiesPanel::setLayerVisible(const std::string& name, bool visible) {
    for (auto& l : m_layers) {
        if (l.name == name) {
            l.visible = visible;
            return true;
        }
    }
    return false;
}

bool MapPropertiesPanel::setLayerLocked(const std::string& name, bool locked) {
    for (auto& l : m_layers) {
        if (l.name == name) {
            l.locked = locked;
            return true;
        }
    }
    return false;
}

bool MapPropertiesPanel::reorderLayer(const std::string& name, int newIndex) {
    size_t oldIndex = 0;
    bool found = false;
    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (m_layers[i].name == name) {
            oldIndex = i;
            found = true;
            break;
        }
    }
    if (!found) return false;
    if (newIndex < 0 || newIndex >= static_cast<int>(m_layers.size())) return false;
    LayerInfo l = m_layers[oldIndex];
    m_layers.erase(m_layers.begin() + oldIndex);
    m_layers.insert(m_layers.begin() + newIndex, l);
    return true;
}

size_t MapPropertiesPanel::layerCount() const { return m_layers.size(); }

} // namespace RPG