// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 6 paquetes del corte B:
//   C-14  (fase C, 6 j) - L'editor treballa sobre MapDocument
//   C-15  (fase C, 2 j) - Desar deixa el projecte coherent
//   E-12  (fase E, 4 j) - Propietats de mapa, capes i instàncies
//   G-01  (fase G, 2 j) - Autosave i recuperació
//   G-02  (fase G, 3 j) - Les eines d'edició que falten
//   G-03  (fase G, 5 j) - Historial per comandes
//
// AVISO - FICHERO DE CRUCE. Con 6 paquetes encima, es de los
// sitios donde dos agentes se pisan sin enterarse. Antes de una
// refactorizacion grande aqui, mira si hay trabajo sin commitear
// (git status) y di en que paquete estas trabajando.
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Editor/EditorState.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "Level/LevelDocumentBridge.h"
#include "Core/Json/JsonWriter.h"
#include <sstream>
#include <vector>

#include "Editor/EditorStateHelpers.h"

using EditorInternal::newLayerMeta;
using EditorInternal::replaceLayer;

namespace {

std::string aMinusculas(const std::string& s) {
    std::string r = s;
    for (char& c : r) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return r;
}


// Reconstruye solo una capa sobre el documento candidato. El documento vivo
// no se toca hasta que el lote completo supera su validacion.

}  // namespace

EditorState::EditorState(int width, int height)
    : m_document(ids::MapId::parse("editor_map").value(), "Mapa",
                 std::max(width, 1), std::max(height, 1), 64, 32) {
    MapLayerMeta meta;
    meta.id = ids::LayerId::parse("suelo").value();
    meta.name = "suelo";
    const auto added = m_document.addLayer(MapLayer::tiles(meta, TileLayerDocument(
        this->width(), this->height(),
        std::vector<int>(static_cast<std::size_t>(this->width()) * this->height(), 0))));
    if (!added.isOk()) throw std::logic_error(added.errorMessage());
}

Result<bool> EditorState::setDocument(MapDocument document) {
    const auto valid = document.validate();
    if (!valid.isOk()) return valid;
    const bool hasTiles = std::any_of(document.layers().begin(), document.layers().end(),
        [](const MapLayer& layer) { return layer.type() == MapLayerType::Tile; });
    if (!hasTiles) return Result<bool>::Error("el editor necesita una capa de tiles");
    m_document = std::move(document);
    m_activeLayer = 0;
    m_playerStart = {0, 0};
    clearHistory();
    clearSelection();
    m_clipboard = Clipboard{};
    markClean();
    invalidateObjectView();
    return Result<bool>::Ok(true);
}

const MapLayer* EditorState::tileLayer(int index) const {
    if (index < 0) return nullptr;
    for (const MapLayer& layer : m_document.layers()) {
        if (layer.type() == MapLayerType::Tile && index-- == 0) return &layer;
    }
    return nullptr;
}

int EditorState::layerCount() const {
    return static_cast<int>(std::count_if(m_document.layers().begin(), m_document.layers().end(),
        [](const MapLayer& layer) { return layer.type() == MapLayerType::Tile; }));
}

void EditorState::setActiveLayer(int index) {
    if (tileLayer(index) != nullptr) m_activeLayer = index;
}

const std::string& EditorState::layerName(int index) const {
    static const std::string empty;
    const MapLayer* layer = tileLayer(index);
    return layer == nullptr ? empty : layer->meta().name;
}

bool EditorState::layerVisible(int index) const {
    const MapLayer* layer = tileLayer(index);
    return layer != nullptr && layer->meta().visible;
}

bool EditorState::layerLocked(int index) const {
    const MapLayer* layer = tileLayer(index);
    return layer == nullptr || layer->meta().locked;
}

float EditorState::layerOpacity(int index) const {
    const MapLayer* layer = tileLayer(index);
    return layer == nullptr ? 0.0f : layer->meta().opacity;
}

Result<bool> EditorState::setLayerMeta(int index, MapLayerMeta meta) {
    const MapLayer* layer = tileLayer(index);
    if (layer == nullptr) return Result<bool>::Error("capa fuera de rango");
    const MapLayerMeta& old = layer->meta();
    if (old.id != meta.id) return Result<bool>::Error("el id de capa es estable");
    if (old.name == meta.name && old.visible == meta.visible && old.locked == meta.locked &&
        old.opacity == meta.opacity && old.order == meta.order) return Result<bool>::Ok(true);
    const ids::LayerId activeId = tileLayer(m_activeLayer)->meta().id;
    MapDocument next = m_document;
    const auto changed = next.setLayerMeta(old.id, std::move(meta));
    if (!changed.isOk()) return changed;
    commitDocument(std::move(next));
    for (int i = 0; i < layerCount(); ++i) {
        if (tileLayer(i)->meta().id == activeId) m_activeLayer = i;
    }
    return Result<bool>::Ok(true);
}

void EditorState::setLayerName(int index, const std::string& name) {
    const MapLayer* layer = tileLayer(index);
    if (layer == nullptr || name.empty()) return;
    MapLayerMeta meta = layer->meta();
    meta.name = name;
    setLayerMeta(index, std::move(meta));
}

int EditorState::addLayer(const std::string& name) {
    MapDocument next = m_document;
    MapLayerMeta meta = newLayerMeta(next, "capa");
    meta.name = name.empty() ? "capa" + std::to_string(layerCount() + 1) : name;
    const auto added = next.addLayer(MapLayer::tiles(meta, TileLayerDocument(width(), height(),
        std::vector<int>(static_cast<std::size_t>(width()) * height(), 0))));
    if (!added.isOk()) return -1;
    commitDocument(std::move(next));
    m_activeLayer = layerCount() - 1;
    return m_activeLayer;
}

bool EditorState::removeLayer(int index) {
    const MapLayer* layer = tileLayer(index);
    if (layer == nullptr || layerCount() <= 1 || layer->meta().locked) return false;
    MapDocument next = m_document;
    if (!next.removeLayer(layer->meta().id).isOk()) return false;
    commitDocument(std::move(next));
    if (index < m_activeLayer) --m_activeLayer;
    m_activeLayer = std::min(m_activeLayer, layerCount() - 1);
    return true;
}

void EditorState::setTilePalette(std::vector<int> gids) {
    m_tilePaletteAll = std::move(gids);
    rebuildFilteredPalettes();
}

void EditorState::setObjectPalette(std::vector<std::string> objectIds) {
    m_objectPaletteAll = std::move(objectIds);
    rebuildFilteredPalettes();
}

void EditorState::setTilePaletteLabels(std::unordered_map<int, std::string> labels) {
    m_tilePaletteLabels = std::move(labels);
    rebuildFilteredPalettes();
}

void EditorState::setObjectPaletteLabels(
    std::unordered_map<std::string, std::string> labels) {
    m_objectPaletteLabels = std::move(labels);
    rebuildFilteredPalettes();
}

void EditorState::setLevelPalette(std::vector<std::string> levelPaths) {
    m_levelPaletteAll = std::move(levelPaths);
    rebuildFilteredPalettes();
}

void EditorState::setPaletteFilter(const std::string& texto) {
    m_filter = texto;
    rebuildFilteredPalettes();
}

void EditorState::rebuildFilteredPalettes() {
    // Filtro vacio = paleta entera. Comparacion sin mayusculas y por
    // subcadena: quien busca "taber" no sabe si el id es "tabernero_xila"
    // o "npc_taberna", y obligarle a acertar el prefijo seria devolverle
    // el problema.
    const std::string aguja = aMinusculas(m_filter);
    m_tilePalette.clear();
    m_objectPalette.clear();
    m_levelPalette.clear();

    for (int gid : m_tilePaletteAll) {
        // El GID sigue siendo buscable, pero el nombre editorial permite
        // escribir "arena", "templo" o "hielo" sin conocer el atlas.
        std::string searchable = std::to_string(gid);
        const auto label = m_tilePaletteLabels.find(gid);
        if (label != m_tilePaletteLabels.end()) {
            searchable += " " + label->second;
        }
        if (aguja.empty() || aMinusculas(searchable).find(aguja) != std::string::npos) {
            m_tilePalette.push_back(gid);
        }
    }
    for (const std::string& id : m_objectPaletteAll) {
        std::string searchable = id;
        const auto label = m_objectPaletteLabels.find(id);
        if (label != m_objectPaletteLabels.end()) {
            searchable += " " + label->second;
        }
        if (aguja.empty() || aMinusculas(searchable).find(aguja) != std::string::npos) {
            m_objectPalette.push_back(id);
        }
    }
    for (const std::string& ruta : m_levelPaletteAll) {
        if (aguja.empty() || aMinusculas(ruta).find(aguja) != std::string::npos) {
            m_levelPalette.push_back(ruta);
        }
    }

    // La seleccion vuelve al principio: mantener el indice tras filtrar
    // apunta a otra entrada, y colocar el objeto equivocado sin enterarse
    // es peor que perder la posicion.
    m_tileSelection = 0;
    m_objectSelection = 0;
    m_levelSelection = 0;
}

void EditorState::nextPaletteEntry() {
    if (m_tool == EditorTool::PaintTile && !m_tilePalette.empty()) {
        m_tileSelection = (m_tileSelection + 1) % m_tilePalette.size();
    } else if (m_tool == EditorTool::PlaceObject && !m_objectPalette.empty()) {
        m_objectSelection = (m_objectSelection + 1) % m_objectPalette.size();
    } else if (m_tool == EditorTool::LinkLevel && !m_levelPalette.empty()) {
        m_levelSelection = (m_levelSelection + 1) % m_levelPalette.size();
    }
}

void EditorState::prevPaletteEntry() {
    // Mismo caso especial de wraparound hacia atras que
    // HudCommandMenu::moveUp() (size_t no puede ser -1).
    if (m_tool == EditorTool::PaintTile && !m_tilePalette.empty()) {
        m_tileSelection = (m_tileSelection == 0) ? m_tilePalette.size() - 1 : m_tileSelection - 1;
    } else if (m_tool == EditorTool::PlaceObject && !m_objectPalette.empty()) {
        m_objectSelection =
            (m_objectSelection == 0) ? m_objectPalette.size() - 1 : m_objectSelection - 1;
    } else if (m_tool == EditorTool::LinkLevel && !m_levelPalette.empty()) {
        m_levelSelection =
            (m_levelSelection == 0) ? m_levelPalette.size() - 1 : m_levelSelection - 1;
    }
}

int EditorState::selectedTileGid() const {
    return m_tilePalette.empty() ? 0 : m_tilePalette[m_tileSelection];
}

std::string EditorState::selectedObjectId() const {
    return m_objectPalette.empty() ? std::string() : m_objectPalette[m_objectSelection];
}

std::string EditorState::selectedLevelPath() const {
    return m_levelPalette.empty() ? std::string() : m_levelPalette[m_levelSelection];
}


bool EditorState::commitDocument(MapDocument document) {
    if (!document.validate().isOk()) return false;
    recordUndo();
    m_document = std::move(document);
    invalidateObjectView();
    markModified();  // C-15 — Cualquier commit es un cambio que debe guardarse
    return true;
}

void EditorState::invalidateObjectView() {
    m_objectViewValid = false;
}

const std::vector<ObjectSpawn>& EditorState::objects() const {
    if (!m_objectViewValid) {
        m_objectView = level_document::ToObjectSpawns(m_document);
        m_objectViewValid = true;
    }
    return m_objectView;
}

EditorState::Snapshot EditorState::snapshot() const {
    return Snapshot{m_document, m_activeLayer, m_playerStart};
}

void EditorState::restore(Snapshot state) {
    m_document = std::move(state.document);
    m_activeLayer = std::clamp(state.activeLayer, 0, layerCount() - 1);
    m_playerStart = state.playerStart;
    markModified();
    invalidateObjectView();
}


void EditorState::recordUndo() {
    constexpr std::size_t kMaxHistory = 256;
    if (m_undo.size() == kMaxHistory) {
        m_undo.erase(m_undo.begin());
    }
    m_undo.push_back(snapshot());
    m_redo.clear();
    markModified();
}

bool EditorState::undo() {
    if (m_undo.empty()) {
        return false;
    }
    m_redo.push_back(snapshot());
    restore(std::move(m_undo.back()));
    m_undo.pop_back();
    return true;
}

bool EditorState::redo() {
    if (m_redo.empty()) {
        return false;
    }
    m_undo.push_back(snapshot());
    restore(std::move(m_redo.back()));
    m_redo.pop_back();
    return true;
}

void EditorState::clearHistory() {
    m_undo.clear();
    m_redo.clear();
}

int EditorState::tileAt(int x, int y) const { return tileAt(m_activeLayer, x, y); }

int EditorState::tileAt(int layer, int x, int y) const {
    const MapLayer* found = tileLayer(layer);
    return found == nullptr ? 0 : std::get<TileLayerDocument>(found->data()).gidAt(x, y);
}

const ObjectSpawn* EditorState::objectAt(int x, int y) const {
    for (const ObjectSpawn& object : objects()) {
        if (object.position.x == x && object.position.y == y) return &object;
    }
    return nullptr;
}
