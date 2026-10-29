// Implementacion de las dos piezas compartidas. Cuerpos movidos tal cual
// del namespace anonimo de EditorState.cpp (24/09/2026, EST-24).

#include "Editor/EditorStateHelpers.h"

#include <limits>   // newLayerMeta numera la capa nueva buscando el mayor orden

namespace EditorInternal {

bool replaceLayer(MapDocument& document, MapLayer layer) {
    if (layer.type() == MapLayerType::Tile) {
        return document.editTiles(layer.meta().id, [&](TileLayerDocument& tiles) {
            tiles = std::get<TileLayerDocument>(layer.data());
            return Result<bool>::Ok(true);
        }).isOk();
    }
    if (!document.removeLayer(layer.meta().id).isOk()) return false;
    return document.addLayer(std::move(layer)).isOk();
}


MapLayerMeta newLayerMeta(const MapDocument& document, const std::string& prefix) {
    MapLayerMeta meta;
    for (int suffix = 1; ; ++suffix) {
        meta.id = ids::LayerId::parse(prefix + "_" + std::to_string(suffix)).value();
        if (document.findLayer(meta.id) == nullptr) break;
    }
    for (const MapLayer& layer : document.layers()) {
        if (layer.meta().order == std::numeric_limits<int>::max()) {
            // addLayer rechazara este orden ocupado sin modificar el editor.
            meta.order = layer.meta().order;
            return meta;
        }
        meta.order = std::max(meta.order, layer.meta().order + 1);
    }
    return meta;
}


}  // namespace EditorInternal
