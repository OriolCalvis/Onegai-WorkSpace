#include "Editor/EditorState.h"

#include <algorithm>
#include <sstream>

namespace {

// Escape minimo para strings dentro de JSON (comillas y backslash; los
// ids/nombres de nivel no llevan control chars, y si algun dia los
// llevan, JsonValue::parse los rechazaria en el round-trip de
// demo_editor_state.cpp -- fallo visible, no corrupcion silenciosa).
std::string jsonEscape(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        if (c == '"' || c == '\\') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

}  // namespace

EditorState::EditorState(int width, int height)
    : m_width(std::max(width, 1))
    , m_height(std::max(height, 1))
    , m_tiles(static_cast<std::size_t>(m_width) * m_height, 0) {}

void EditorState::setTilePalette(std::vector<int> gids) {
    m_tilePalette = std::move(gids);
    m_tileSelection = 0;
}

void EditorState::setObjectPalette(std::vector<std::string> objectIds) {
    m_objectPalette = std::move(objectIds);
    m_objectSelection = 0;
}

void EditorState::nextPaletteEntry() {
    if (m_tool == EditorTool::PaintTile && !m_tilePalette.empty()) {
        m_tileSelection = (m_tileSelection + 1) % m_tilePalette.size();
    } else if (m_tool == EditorTool::PlaceObject && !m_objectPalette.empty()) {
        m_objectSelection = (m_objectSelection + 1) % m_objectPalette.size();
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
    }
}

int EditorState::selectedTileGid() const {
    return m_tilePalette.empty() ? 0 : m_tilePalette[m_tileSelection];
}

std::string EditorState::selectedObjectId() const {
    return m_objectPalette.empty() ? std::string() : m_objectPalette[m_objectSelection];
}

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
    }
}

void EditorState::paintTile(int x, int y, int gid) {
    if (!inBounds(x, y) || gid < 0) {
        return;
    }
    const std::size_t index = static_cast<std::size_t>(y) * m_width + x;
    if (m_tiles[index] == gid) {
        return;
    }
    recordUndo();
    m_tiles[index] = gid;
}

void EditorState::eraseTile(int x, int y) { paintTile(x, y, 0); }

void EditorState::placeObject(int x, int y, const std::string& objectId) {
    if (!inBounds(x, y) || objectId.empty()) {
        return;
    }
    const ObjectSpawn* existing = objectAt(x, y);
    if (existing != nullptr && existing->objectId == objectId && existing->targetLevel.empty()) {
        return;
    }
    recordUndo();
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
                                   [&](const ObjectSpawn& o) {
                                       return o.position.x == x && o.position.y == y;
                                   }),
                    m_objects.end());
    ObjectSpawn spawn;
    spawn.objectId = objectId;
    spawn.position = GridCoord{x, y};
    // Sin patrulla en el MVP del editor: igual que un JSON sin
    // patrolMin/patrolMax (ver LevelLoader), quieto en su celda.
    spawn.patrolMin = spawn.position;
    spawn.patrolMax = spawn.position;
    m_objects.push_back(std::move(spawn));
}

void EditorState::placeSpawn(const ObjectSpawn& spawn) {
    if (!inBounds(spawn.position.x, spawn.position.y) || spawn.objectId.empty()) {
        return;
    }
    recordUndo();
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
                                   [&](const ObjectSpawn& o) {
                                       return o.position.x == spawn.position.x &&
                                              o.position.y == spawn.position.y;
                                   }),
                    m_objects.end());
    m_objects.push_back(spawn);
}

void EditorState::removeObjectAt(int x, int y) {
    const auto first = std::find_if(m_objects.begin(), m_objects.end(), [&](const ObjectSpawn& o) {
        return o.position.x == x && o.position.y == y;
    });
    if (first == m_objects.end()) {
        return;
    }
    recordUndo();
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
                                   [&](const ObjectSpawn& o) {
                                       return o.position.x == x && o.position.y == y;
                                   }),
                    m_objects.end());
}

void EditorState::setPlayerStart(GridCoord position) {
    if (!inBounds(position.x, position.y) ||
        (position.x == m_playerStart.x && position.y == m_playerStart.y)) {
        return;
    }
    recordUndo();
    m_playerStart = position;
}

EditorState::Snapshot EditorState::snapshot() const {
    return Snapshot{m_tiles, m_objects, m_playerStart};
}

void EditorState::restore(Snapshot state) {
    m_tiles = std::move(state.tiles);
    m_objects = std::move(state.objects);
    m_playerStart = state.playerStart;
}

void EditorState::recordUndo() {
    constexpr std::size_t kMaxHistory = 256;
    if (m_undo.size() == kMaxHistory) {
        m_undo.erase(m_undo.begin());
    }
    m_undo.push_back(snapshot());
    m_redo.clear();
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

int EditorState::tileAt(int x, int y) const {
    if (!inBounds(x, y)) {
        return 0;
    }
    return m_tiles[static_cast<std::size_t>(y) * m_width + x];
}

const ObjectSpawn* EditorState::objectAt(int x, int y) const {
    for (const ObjectSpawn& o : m_objects) {
        if (o.position.x == x && o.position.y == y) {
            return &o;
        }
    }
    return nullptr;
}

std::string EditorState::exportTmx(const TmxTilesetSettings& s) const {
    // Mismo esqueleto exacto que assets/maps/test_map.tmx (el archivo de
    // referencia del parser): tileset embebido con <properties> de
    // colision por tile, una capa CSV. Los atributos que TileMap::
    // loadFromFile() ignora (version, renderorder...) se emiten igual
    // para que el archivo tambien abra en Tiled.
    std::ostringstream out;
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<map version=\"1.10\" orientation=\"orthogonal\" renderorder=\"right-down\""
        << " width=\"" << m_width << "\" height=\"" << m_height << "\" tilewidth=\"" << s.tileWidth
        << "\" tileheight=\"" << s.tileHeight
        << "\" infinite=\"0\" nextlayerid=\"2\" nextobjectid=\"1\">\n";

    out << " <tileset firstgid=\"1\" name=\"" << s.tilesetName << "\" tilewidth=\""
        << s.tilesetTileWidth << "\" tileheight=\"" << s.tilesetTileHeight << "\" tilecount=\""
        << s.tileCount << "\" columns=\"" << s.columns << "\">\n";
    out << "  <image source=\"" << s.imageSource << "\" width=\"" << s.imageWidth << "\" height=\""
        << s.imageHeight << "\"/>\n";
    for (int gid : s.collisionGids) {
        // En el tileset de Tiled los <tile id> son 0-based (gid - firstgid).
        out << "  <tile id=\"" << (gid - 1) << "\">\n";
        out << "   <properties>\n";
        out << "    <property name=\"collision\" type=\"bool\" value=\"true\"/>\n";
        out << "   </properties>\n";
        out << "  </tile>\n";
    }
    out << " </tileset>\n";

    out << " <layer id=\"1\" name=\"suelo\" width=\"" << m_width << "\" height=\"" << m_height
        << "\">\n";
    out << "  <data encoding=\"csv\">\n";
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            out << m_tiles[static_cast<std::size_t>(y) * m_width + x];
            // CSV de Tiled: coma tras cada valor salvo el ultimo del mapa.
            if (!(y == m_height - 1 && x == m_width - 1)) {
                out << ",";
            }
        }
        out << "\n";
    }
    out << "  </data>\n";
    out << " </layer>\n";
    out << "</map>\n";
    return out.str();
}

std::string EditorState::exportLevelJson(const std::string& levelName,
                                         const std::string& mapPath) const {
    std::ostringstream out;
    out << "{\n";
    out << "  \"name\": \"" << jsonEscape(levelName) << "\",\n";
    out << "  \"map\": \"" << jsonEscape(mapPath) << "\",\n";
    out << "  \"playerStart\": { \"x\": " << m_playerStart.x << ", \"y\": " << m_playerStart.y
        << " },\n";
    out << "  \"objects\": [";
    for (std::size_t i = 0; i < m_objects.size(); ++i) {
        const ObjectSpawn& o = m_objects[i];
        if (i > 0) {
            out << ",";
        }
        // Sin patrolMin/patrolMax: LevelLoader los rellena con position
        // (objeto quieto), exactamente la semantica del MVP del editor.
        out << "\n    { \"objectId\": \"" << jsonEscape(o.objectId)
            << "\", \"position\": { \"x\": " << o.position.x << ", \"y\": " << o.position.y << " }";
        // El destino SI se conserva: abrir un nivel con puertas, mover un
        // tile y guardar no puede dejar mudas todas las puertas del mapa.
        // El editor todavia no permite EDITAR el destino (no hay UI para
        // ello), pero destruir dato que no sabes editar es peor que no
        // tocarlo.
        if (!o.targetLevel.empty()) {
            out << ", \"targetLevel\": \"" << jsonEscape(o.targetLevel) << "\"";
            if (o.hasTargetPosition) {
                out << ", \"targetPosition\": { \"x\": " << o.targetPosition.x
                    << ", \"y\": " << o.targetPosition.y << " }";
            }
        }
        out << " }";
    }
    out << (m_objects.empty() ? "]\n" : "\n  ]\n");
    out << "}\n";
    return out.str();
}
