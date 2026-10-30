// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-10  (fase E, 6 j) - Espai de treball visual i controls coherents
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Editor/EditorDocumentSession.h"

#include "Editor/EditorDocumentIO.h"
#include "Import/TmxMapImporter.h"
#include "Level/LevelDocumentBridge.h"
#include "Level/LevelLoader.h"
#include "Project/ProjectSerializer.h"
#include "Core/Math/IsoMath.h"

#include <cmath>
#include <filesystem>

namespace Editor {

Result<EditorDocumentSession::Document> EditorDocumentSession::load(const std::string& path) {
    const auto level = LevelLoader::loadFromFile(path);
    if (!level.isOk()) return Result<Document>::Error(level.errorMessage());
    Document next;
    next.path = path;
    next.level = level.value();
    const auto map = next.map.loadFromFile(next.level.mapPath);
    if (!map.isOk()) return Result<Document>::Error(map.errorMessage());
    if (next.map.getTilesetFirstGid() != 1)
        return Result<Document>::Error("Este exportador requiere firstgid=1; el mapa se conserva sin cambios");
    MapDocument native;
    if (!next.level.mapDocumentPath.empty()) {
        auto loaded = ProjectSerializer::loadMapFile(next.level.mapDocumentPath);
        if (!loaded.isOk()) return Result<Document>::Error(loaded.errorMessage());
        native = std::move(loaded.value());
    } else {
        const auto id = ids::MapId::parse(std::filesystem::path(next.level.mapPath).stem().string());
        if (!id.isOk()) return Result<Document>::Error(id.errorMessage());
        auto imported = TmxMapImporter{}.importFile(next.level.mapPath, id.value());
        // El shell no debe guardar un TMX importado con pérdida silenciosa.
        if (!imported.issues.empty())
            return Result<Document>::Error("Importación TMX: " + imported.issues.front().message);
        native = std::move(imported.document);
        auto entities = level_document::ToEntityLayer(next.level.objects);
        if (!entities.isOk()) return Result<Document>::Error(entities.errorMessage());
        const auto added = native.addLayer(MapLayer::entities(
            {ids::LayerId::parse("level_objects").value(), "Objetos", true, false, 1.0f,
             static_cast<int>(native.layers().size())}, std::move(entities.value())));
        if (!added.isOk()) return Result<Document>::Error(added.errorMessage());
    }
    const auto loaded = next.state.setDocument(std::move(native));
    if (!loaded.isOk()) return Result<Document>::Error(loaded.errorMessage());
    next.state.setPlayerStart(next.level.playerStart);
    next.state.clearHistory();
    auto& tiles = next.tileset;
    tiles.tilesetName = next.map.getTilesetName();
    tiles.tileWidth = next.map.getTileWidth();
    tiles.tileHeight = next.map.getTileHeight();
    tiles.imageSource = next.map.getTilesetImageSource();
    tiles.imageWidth = next.map.getTilesetImageWidth();
    tiles.imageHeight = next.map.getTilesetImageHeight();
    tiles.tilesetTileWidth = next.map.getTilesetTileWidth();
    tiles.tilesetTileHeight = next.map.getTilesetTileHeight();
    tiles.tileCount = next.map.getTilesetTileCount();
    tiles.columns = next.map.getTilesetColumns();
    tiles.collisionGids = next.map.getCollisionGids();
    next.savedContent = fingerprint(next);
    return Result<Document>::Ok(std::move(next));
}

std::string EditorDocumentSession::fingerprint(const Document& document) {
    const auto native = ProjectSerializer::serializeMap(document.state.document());
    if (!native.isOk()) throw std::logic_error(native.errorMessage());
    const auto start = document.state.playerStart();
    return native.value() + "\n" + std::to_string(start.x) + "," + std::to_string(start.y);
}

Result<bool> EditorDocumentSession::execute(const Pending& action) {
    if (action.action == Action::Open) {
        auto next = load(action.path);
        if (!next.isOk()) return Result<bool>::Error(next.errorMessage());
        m_document = std::move(next.value());
    } else {
        m_document.reset();
        m_quit = action.action == Action::Quit;
    }
    m_dirty = false;
    ++m_generation;
    ++m_revision;
    return Result<bool>::Ok(true);
}

Result<bool> EditorDocumentSession::request(Action action, const std::string& path) {
    if (m_pending) return Result<bool>::Error("Resuelve primero los cambios pendientes");
    Pending next{action, path};
    if (m_dirty) {
        m_pending = std::move(next);
        return Result<bool>::Ok(false);
    }
    return execute(next);
}

Result<bool> EditorDocumentSession::resolve(Choice choice) {
    if (!m_pending) return Result<bool>::Error("No hay una acción pendiente");
    if (choice == Choice::Cancel) {
        m_pending.reset();
        return Result<bool>::Ok(false);
    }
    if (choice == Choice::Save) {
        const auto result = save();
        if (!result.isOk()) return result;
    }
    const auto result = execute(*m_pending);
    if (result.isOk()) m_pending.reset();
    return result;
}

Result<bool> EditorDocumentSession::save() {
    if (!m_document) return Result<bool>::Error("No hay un mapa abierto");
    auto& document = *m_document;
    const auto saved = EditorDocumentIO::save(document.state, document.level.name,
        document.level.mapPath, document.path, document.tileset, document.level.enemies);
    if (!saved.isOk()) return saved;
    document.savedContent = fingerprint(document);
    m_dirty = false;
    return Result<bool>::Ok(true);
}

void EditorDocumentSession::changed() {
    m_dirty = fingerprint(*m_document) != m_document->savedContent;
    ++m_revision;
}

void EditorDocumentSession::paint(int x, int y, int gid) {
    if (!m_document || m_pending || gid < 0 || gid > m_document->tileset.tileCount) return;
    m_document->state.paintTile(x, y, gid);
    changed();
}
void EditorDocumentSession::fill(int x, int y, int gid) {
    if (!m_document || m_pending || gid < 0 || gid > m_document->tileset.tileCount) return;
    m_document->state.fillTiles(x, y, gid);
    changed();
}
void EditorDocumentSession::playerStart(GridCoord position) {
    if (!m_document || m_pending) return;
    m_document->state.setPlayerStart(position);
    changed();
}
Result<bool> EditorDocumentSession::placeObject(GridCoord cell, const std::string& id,
                                               const ObjectCatalog& catalog) {
    if (!m_document || m_pending) return Result<bool>::Ok(false);
    auto& state = m_document->state;
    if (cell.x < 0 || cell.y < 0 || cell.x >= state.width() || cell.y >= state.height()) return Result<bool>::Ok(false);
    if (!catalog.has(id)) return Result<bool>::Error("Selecciona un objeto del catálogo de este proyecto");
    if (const auto* existing = state.objectAt(cell.x, cell.y)) {
        if (existing->objectId == id) return Result<bool>::Ok(false);
        return Result<bool>::Error("La celda ya tiene un objeto. Bórralo antes de colocar otro");
    }
    state.placeObject(cell.x, cell.y, id);
    if (!state.objectAt(cell.x, cell.y))
        return Result<bool>::Error("No se pudo colocar el objeto; comprueba las capas bloqueadas");
    changed();
    return Result<bool>::Ok(true);
}
Result<bool> EditorDocumentSession::removeObject(GridCoord cell) {
    if (!m_document || m_pending) return Result<bool>::Ok(false);
    auto& state = m_document->state;
    if (!state.objectAt(cell.x, cell.y)) return Result<bool>::Ok(false);
    state.removeObjectAt(cell.x, cell.y);
    if (state.objectAt(cell.x, cell.y))
        return Result<bool>::Error("No se pudo borrar el objeto; comprueba las capas bloqueadas");
    changed();
    return Result<bool>::Ok(true);
}
void EditorDocumentSession::undo() {
    if (m_document && !m_pending && m_document->state.undo()) changed();
}
std::vector<MapLayerMeta> EditorDocumentSession::tileLayers() const {
    std::vector<MapLayerMeta> layers;
    if (m_document) for (const auto& layer : state().document().layers())
        if (layer.type() == MapLayerType::Tile) layers.push_back(layer.meta());
    return layers;
}
int EditorDocumentSession::tileLayerIndex(const ids::LayerId& id) const {
    const auto layers = tileLayers();
    for (std::size_t i = 0; i < layers.size(); ++i)
        if (layers[i].id == id) return static_cast<int>(i);
    return -1;
}
ids::LayerId EditorDocumentSession::activeTileLayer() const {
    const auto layers = tileLayers();
    return layers.empty() ? ids::LayerId{} : layers.at(state().activeLayer()).id;
}
Result<bool> EditorDocumentSession::selectTileLayer(const ids::LayerId& id) {
    if (!m_document || m_pending) return Result<bool>::Ok(false);
    const int index = tileLayerIndex(id);
    if (index < 0) return Result<bool>::Error("La capa de baldosas ya no existe");
    m_document->state.setActiveLayer(index); // Selección de vista: sin dirty ni historial.
    return Result<bool>::Ok(true);
}
Result<bool> EditorDocumentSession::addTileLayer() {
    if (!m_document || m_pending) return Result<bool>::Ok(false);
    if (m_document->state.addLayer("Capa " + std::to_string(state().layerCount() + 1)) < 0)
        return Result<bool>::Error("No se pudo crear la capa");
    changed();
    return Result<bool>::Ok(true);
}
Result<bool> EditorDocumentSession::updateTileLayer(MapLayerMeta meta) {
    if (!m_document || m_pending) return Result<bool>::Ok(false);
    const int index = tileLayerIndex(meta.id);
    if (index < 0) return Result<bool>::Error("La capa de baldosas ya no existe");
    if (meta.name.find_first_not_of(" \t\r\n") == std::string::npos)
        return Result<bool>::Error("La capa necesita un nombre");
    const auto before = fingerprint(*m_document);
    const auto result = m_document->state.setLayerMeta(index, std::move(meta));
    if (!result.isOk()) return result;
    if (fingerprint(*m_document) == before) return Result<bool>::Ok(false);
    changed();
    return Result<bool>::Ok(true);
}
Result<bool> EditorDocumentSession::removeTileLayer(const ids::LayerId& id) {
    if (!m_document || m_pending) return Result<bool>::Ok(false);
    const int index = tileLayerIndex(id);
    if (index < 0) return Result<bool>::Error("La capa de baldosas ya no existe");
    if (state().layerCount() <= 1) return Result<bool>::Error("Debe quedar al menos una capa de baldosas");
    if (state().layerLocked(index)) return Result<bool>::Error("Desbloquea la capa antes de borrarla");
    if (!m_document->state.removeLayer(index)) return Result<bool>::Error("No se pudo borrar la capa");
    changed();
    return Result<bool>::Ok(true);
}
void EditorDocumentSession::redo() {
    if (m_document && !m_pending && m_document->state.redo()) changed();
}

std::optional<Vector2> ViewportInput::framebufferPoint(float mouseX, float mouseY) const {
    if (!hovered || textEditing || modal || !(scaleX > 0) || !(scaleY > 0) ||
        !std::isfinite(scaleX) || !std::isfinite(scaleY) ||
        !std::isfinite(mouseX) || !std::isfinite(mouseY) ||
        !(mouseX >= left && mouseY >= top && mouseX < left + width && mouseY < top + height))
        return std::nullopt;
    return Vector2{(mouseX - left) * scaleX, (mouseY - top) * scaleY};
}

GridCoord renderedMapCell(Vector2 world, float tileWidth, float tileHeight) {
    return IsoMath::screenToGrid({world.x - tileWidth * 0.5f, world.y}, tileWidth, tileHeight);
}

}  // namespace Editor
