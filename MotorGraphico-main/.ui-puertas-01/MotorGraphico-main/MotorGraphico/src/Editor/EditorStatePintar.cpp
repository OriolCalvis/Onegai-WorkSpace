// Pintar: la brocha, el rectangulo, la linea y el cuentagotas.
//
// Todo lo que cambia baldosas bajo el cursor. Es lo que mas toca el
// autor y lo que mas cambia cuando aparece una herramienta nueva.
//
// Salio de src/Editor/EditorState.cpp el 24/09/2026 (EST-24): aquel fichero
// tenia 1075 lineas y seis paquetes encima (C-14, C-15, E-12, G-01, G-02,
// G-03). Se movio, no se reescribio.

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

void EditorState::applyAt(int x, int y) {
    switch (m_tool) {
        case EditorTool::PaintTile:
            paintTile(x, y, selectedTileGid());
            break;
        case EditorTool::EraseTile:
            eraseTile(x, y);
            break;
        case EditorTool::PlaceObject: {
            std::string id = selectedObjectId();
            if (!id.empty()) {
                placeObject(x, y, id);
            }
            break;
        }
        case EditorTool::RemoveObject:
            removeObjectAt(x, y);
            break;
        case EditorTool::SetPlayerStart:
            setPlayerStart(GridCoord{x, y});
            break;
        case EditorTool::FillTiles:
            fillTiles(x, y, selectedTileGid());
            break;
        case EditorTool::LinkLevel:
            setObjectTransition(x, y, selectedLevelPath());
            break;
    }
}

void EditorState::paintTile(int x, int y, int gid) {
    if (!inBounds(x, y) || gid < 0 || layerLocked(m_activeLayer) || tileAt(x, y) == gid) return;
    MapDocument next = m_document;
    if (next.setTile(tileLayer(m_activeLayer)->meta().id, {x, y}, gid).isOk())
        commitDocument(std::move(next));
}

void EditorState::eraseTile(int x, int y) { paintTile(x, y, 0); }

void EditorState::fillTiles(int x, int y, int gid) {
    if (!inBounds(x, y) || gid < 0 || layerLocked(m_activeLayer)) return;
    const int original = tileAt(x, y);
    if (original == gid) return;
    const MapLayer& layer = *tileLayer(m_activeLayer);
    TileLayerDocument tiles = std::get<TileLayerDocument>(layer.data());
    std::vector<GridCoord> pending{{x, y}};
    while (!pending.empty()) {
        const GridCoord cell = pending.back();
        pending.pop_back();
        if (!inBounds(cell.x, cell.y) || tiles.gidAt(cell.x, cell.y) != original) continue;
        tiles.setGid(cell.x, cell.y, gid);
        pending.push_back({cell.x + 1, cell.y});
        pending.push_back({cell.x - 1, cell.y});
        pending.push_back({cell.x, cell.y + 1});
        pending.push_back({cell.x, cell.y - 1});
    }
    MapDocument next = m_document;
    if (replaceLayer(next, MapLayer::tiles(layer.meta(), std::move(tiles))))
        commitDocument(std::move(next));
}

// ---------------------------------------------------------------------
// G-02 — Cuentagotas, rectángulo y línea
// ---------------------------------------------------------------------

std::vector<GridCoord> EditorState::RectCells(GridCoord a, GridCoord b, bool filled) {
    const int x0 = std::min(a.x, b.x), x1 = std::max(a.x, b.x);
    const int y0 = std::min(a.y, b.y), y1 = std::max(a.y, b.y);
    std::vector<GridCoord> cells;
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            // Borde: sólo la primera y última fila/columna. Un rectángulo
            // de 1 de ancho o alto es su propio borde.
            if (filled || y == y0 || y == y1 || x == x0 || x == x1) cells.push_back({x, y});
        }
    }
    return cells;
}

std::vector<GridCoord> EditorState::LineCells(GridCoord a, GridCoord b) {
    // Orden canónico: sin él, Bresenham de A a B y de B a A difieren en
    // las celdas «de media», y la vista previa del arrastre no coincidiría
    // con el trazo deshecho y rehecho al revés.
    if (b.x < a.x || (b.x == a.x && b.y < a.y)) std::swap(a, b);
    std::vector<GridCoord> cells;
    const int dx = std::abs(b.x - a.x);
    const int dy = -std::abs(b.y - a.y);
    const int sx = a.x < b.x ? 1 : -1;
    const int sy = a.y < b.y ? 1 : -1;
    int err = dx + dy;
    GridCoord p = a;
    cells.reserve(static_cast<std::size_t>(std::max(dx, -dy)) + 1);
    while (true) {
        cells.push_back(p);
        if (p.x == b.x && p.y == b.y) break;
        const int e2 = 2 * err;
        if (e2 >= dy) { err += dy; p.x += sx; }
        if (e2 <= dx) { err += dx; p.y += sy; }
    }
    return cells;
}

bool EditorState::paintCells(const std::vector<GridCoord>& cells, int gid) {
    if (gid < 0 || layerLocked(m_activeLayer)) return false;
    const MapLayer* layer = tileLayer(m_activeLayer);
    if (layer == nullptr) return false;
    TileLayerDocument tiles = std::get<TileLayerDocument>(layer->data());
    bool changed = false;
    for (const GridCoord& cell : cells) {
        if (!inBounds(cell.x, cell.y) || tiles.gidAt(cell.x, cell.y) == gid) continue;
        if (!tiles.setGid(cell.x, cell.y, gid).isOk()) return false;  // todo o nada
        changed = true;
    }
    if (!changed) return false;
    MapDocument next = m_document;
    if (!replaceLayer(next, MapLayer::tiles(layer->meta(), std::move(tiles)))) return false;
    return commitDocument(std::move(next));
}

bool EditorState::paintRect(GridCoord a, GridCoord b, int gid, bool filled) {
    // Recorte antes de enumerar: un arrastre con el ratón muy fuera del
    // lienzo no debe generar millones de celdas que luego se descartan.
    if (width() <= 0 || height() <= 0) return false;
    const bool fuera = std::max(a.x, b.x) < 0 || std::max(a.y, b.y) < 0 ||
                       std::min(a.x, b.x) >= width() || std::min(a.y, b.y) >= height();
    if (fuera) return false;
    if (filled) {
        a = {std::clamp(a.x, 0, width() - 1), std::clamp(a.y, 0, height() - 1)};
        b = {std::clamp(b.x, 0, width() - 1), std::clamp(b.y, 0, height() - 1)};
        return paintCells(RectCells(a, b, true), gid);
    }
    // El borde NO se recorta al mapa: si un lado cae fuera, ese lado no se
    // pinta (recortarlo lo metería dentro y pintaría otra fila). Sí se
    // acerca a una celda del borde exterior, que sigue fuera: el resultado
    // es el mismo y el perímetro deja de depender de lo lejos que esté el
    // ratón.
    const auto acercar = [](GridCoord p, int w, int h) {
        return GridCoord{std::clamp(p.x, -1, w), std::clamp(p.y, -1, h)};
    };
    return paintCells(RectCells(acercar(a, width(), height()), acercar(b, width(), height()), false),
                      gid);
}

bool EditorState::paintLine(GridCoord a, GridCoord b, int gid) {
    return paintCells(LineCells(a, b), gid);
}

bool EditorState::pickTileAt(int x, int y) {
    if (!inBounds(x, y)) return false;
    const int gid = tileAt(x, y);
    if (gid <= 0) return false;
    auto buscar = [&]() -> bool {
        const auto it = std::find(m_tilePalette.begin(), m_tilePalette.end(), gid);
        if (it == m_tilePalette.end()) return false;
        m_tileSelection = static_cast<std::size_t>(it - m_tilePalette.begin());
        return true;
    };
    if (buscar()) return true;
    if (std::find(m_tilePaletteAll.begin(), m_tilePaletteAll.end(), gid) == m_tilePaletteAll.end()) {
        return false;
    }
    // Oculto por el filtro: se quita el filtro. rebuildFilteredPalettes
    // devuelve las selecciones a 0; las de objetos y niveles se restauran
    // por identidad, porque el autor no ha pedido cambiarlas y el índice
    // antiguo apuntaría a otra entrada de la lista ya sin filtrar.
    const std::string objeto = selectedObjectId();
    const std::string nivel = selectedLevelPath();
    setPaletteFilter("");
    const auto o = std::find(m_objectPalette.begin(), m_objectPalette.end(), objeto);
    if (o != m_objectPalette.end()) m_objectSelection = static_cast<std::size_t>(o - m_objectPalette.begin());
    const auto l = std::find(m_levelPalette.begin(), m_levelPalette.end(), nivel);
    if (l != m_levelPalette.end()) m_levelSelection = static_cast<std::size_t>(l - m_levelPalette.begin());
    return buscar();
}

