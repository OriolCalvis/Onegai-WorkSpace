#include "Editor/EditorState.h"

#include <algorithm>
#include <sstream>
#include <vector>

namespace {

// Escape minimo para strings dentro de JSON (comillas y backslash; los
// ids/nombres de nivel no llevan control chars, y si algun dia los
// llevan, JsonValue::parse los rechazaria en el round-trip de
// demo_editor_state.cpp -- fallo visible, no corrupcion silenciosa).
// El nombre de capa acaba dentro de un atributo XML. Antes no hacia falta
// escapar nada porque el nombre era la constante "suelo"; ahora lo elige
// el autor, y un nombre con comillas o & produciria un TMX que tinyxml2
// rechaza al volver a abrirlo -- es decir, guardar y perder el mapa.
std::string xmlEscape(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default:   out += c;        break;
        }
    }
    return out;
}

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

// Minusculas ASCII. No se usa std::tolower con locale a proposito: los
// ids del catalogo son ASCII (lo garantiza el exportador) y una locale
// turca convertiria la I en otra cosa.
std::string aMinusculas(const std::string& s) {
    std::string r = s;
    for (char& c : r) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return r;
}

}  // namespace

EditorState::EditorState(int width, int height)
    : m_width(std::max(width, 1))
    , m_height(std::max(height, 1)) {
    // Siempre hay al menos una capa. "suelo" es el nombre que ya emitia
    // exportTmx, asi que un mapa de una capa sale byte a byte igual que
    // antes y los TMX existentes no cambian.
    m_layers.push_back(Layer{"suelo",
                             std::vector<int>(static_cast<std::size_t>(m_width) * m_height, 0)});
}

// --- Capas ---

void EditorState::setActiveLayer(int index) {
    if (index >= 0 && index < layerCount()) {
        m_activeLayer = index;
    }
}

const std::string& EditorState::layerName(int index) const {
    static const std::string vacio;
    if (index < 0 || index >= layerCount()) {
        return vacio;
    }
    return m_layers[static_cast<std::size_t>(index)].name;
}

void EditorState::setLayerName(int index, const std::string& name) {
    if (index >= 0 && index < layerCount() && !name.empty()) {
        m_layers[static_cast<std::size_t>(index)].name = name;
    }
}

int EditorState::addLayer(const std::string& name) {
    recordUndo();
    m_layers.push_back(Layer{name.empty() ? "capa" + std::to_string(layerCount() + 1) : name,
                             std::vector<int>(static_cast<std::size_t>(m_width) * m_height, 0)});
    m_activeLayer = layerCount() - 1;
    return m_activeLayer;
}

bool EditorState::removeLayer(int index) {
    // Un mapa sin capa de tiles no es un mapa: la ultima no se borra.
    if (index < 0 || index >= layerCount() || layerCount() <= 1) {
        return false;
    }
    recordUndo();
    m_layers.erase(m_layers.begin() + index);
    if (m_activeLayer >= layerCount()) {
        m_activeLayer = layerCount() - 1;
    }
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
        // Los tiles no tienen nombre: se busca por su numero. Sirve para
        // "quiero el 27" cuando el tileset es grande.
        if (aguja.empty() || std::to_string(gid).find(aguja) != std::string::npos) {
            m_tilePalette.push_back(gid);
        }
    }
    for (const std::string& id : m_objectPaletteAll) {
        if (aguja.empty() || aMinusculas(id).find(aguja) != std::string::npos) {
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
    if (!inBounds(x, y) || gid < 0) {
        return;
    }
    const std::size_t index = static_cast<std::size_t>(y) * m_width + x;
    std::vector<int>& capa = m_layers[static_cast<std::size_t>(m_activeLayer)].tiles;
    if (capa[index] == gid) {
        return;
    }
    recordUndo();
    m_layers[static_cast<std::size_t>(m_activeLayer)].tiles[index] = gid;
}

void EditorState::eraseTile(int x, int y) { paintTile(x, y, 0); }

void EditorState::fillTiles(int x, int y, int gid) {
    if (!inBounds(x, y) || gid < 0) {
        return;
    }
    const int original = tileAt(x, y);
    if (original == gid) {
        return;
    }
    recordUndo();
    std::vector<GridCoord> pending;
    pending.push_back(GridCoord{x, y});
    while (!pending.empty()) {
        const GridCoord current = pending.back();
        pending.pop_back();
        if (!inBounds(current.x, current.y) || tileAt(current.x, current.y) != original) {
            continue;
        }
        m_layers[static_cast<std::size_t>(m_activeLayer)]
            .tiles[static_cast<std::size_t>(current.y) * m_width + current.x] = gid;
        pending.push_back(GridCoord{current.x + 1, current.y});
        pending.push_back(GridCoord{current.x - 1, current.y});
        pending.push_back(GridCoord{current.x, current.y + 1});
        pending.push_back(GridCoord{current.x, current.y - 1});
    }
}

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
    // Nace quieto: patrulla = su propia celda, igual que un JSON sin
    // patrolMin/patrolMax (ver LevelLoader). Se le da recorrido despues
    // con setObjectPatrol().
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

void EditorState::setObjectTransition(int x, int y, const std::string& targetLevel) {
    if (targetLevel.empty()) {
        return;
    }
    auto object = std::find_if(m_objects.begin(), m_objects.end(), [&](const ObjectSpawn& spawn) {
        return spawn.position.x == x && spawn.position.y == y;
    });
    if (object == m_objects.end() ||
        (object->targetLevel == targetLevel && !object->hasTargetPosition)) {
        return;
    }
    recordUndo();
    object->targetLevel = targetLevel;
    object->hasTargetPosition = false;  // destino = playerStart del nivel de llegada
}

void EditorState::setPlayerStart(GridCoord position) {
    if (!inBounds(position.x, position.y) ||
        (position.x == m_playerStart.x && position.y == m_playerStart.y)) {
        return;
    }
    recordUndo();
    m_playerStart = position;
}


// =====================================================================
// SELECCION Y PORTAPAPELES
// =====================================================================

void EditorState::setSelection(int x0, int y0, int x1, int y1) {
    // Se normaliza (arrastrar de derecha a izquierda es tan valido como
    // al reves) y se recorta al mapa: seleccionar saliendose del borde es
    // lo normal cuando se arrastra rapido.
    Rect r;
    r.x0 = std::max(0, std::min(x0, x1));
    r.y0 = std::max(0, std::min(y0, y1));
    r.x1 = std::min(m_width - 1, std::max(x0, x1));
    r.y1 = std::min(m_height - 1, std::max(y0, y1));
    m_selection = r;
}

void EditorState::clearSelection() { m_selection = Rect{}; }

void EditorState::copySelection() {
    if (!m_selection.valid()) {
        return;
    }
    // Copiar no toca el documento: no hay recordUndo().
    Clipboard c;
    c.width = m_selection.width();
    c.height = m_selection.height();
    c.layers.assign(m_layers.size(), {});
    for (std::size_t l = 0; l < m_layers.size(); ++l) {
        c.layers[l].reserve(static_cast<std::size_t>(c.width) * c.height);
        for (int y = m_selection.y0; y <= m_selection.y1; ++y) {
            for (int x = m_selection.x0; x <= m_selection.x1; ++x) {
                c.layers[l].push_back(tileAt(static_cast<int>(l), x, y));
            }
        }
    }
    for (const ObjectSpawn& o : m_objects) {
        if (o.position.x < m_selection.x0 || o.position.x > m_selection.x1 ||
            o.position.y < m_selection.y0 || o.position.y > m_selection.y1) {
            continue;
        }
        // Posiciones RELATIVAS: el portapapeles no sabe donde se va a
        // pegar, y guardarlas absolutas obligaria a restar al pegar en
        // dos sitios distintos.
        ObjectSpawn copia = o;
        copia.position.x -= m_selection.x0;
        copia.position.y -= m_selection.y0;
        // La patrulla se mueve con el objeto; el destino de una puerta NO
        // (targetLevel apunta a otro fichero, no a coordenadas de este).
        copia.patrolMin.x -= m_selection.x0;
        copia.patrolMin.y -= m_selection.y0;
        copia.patrolMax.x -= m_selection.x0;
        copia.patrolMax.y -= m_selection.y0;
        c.objects.push_back(copia);
    }
    m_clipboard = std::move(c);
}

void EditorState::pasteAt(int x, int y) {
    if (!hasClipboard()) {
        return;
    }
    recordUndo();
    for (std::size_t l = 0; l < m_clipboard.layers.size() && l < m_layers.size(); ++l) {
        for (int dy = 0; dy < m_clipboard.height; ++dy) {
            for (int dx = 0; dx < m_clipboard.width; ++dx) {
                const int destX = x + dx;
                const int destY = y + dy;
                if (!inBounds(destX, destY)) {
                    continue;   // se recorta, no se rechaza la operacion entera
                }
                const int gid =
                    m_clipboard.layers[l][static_cast<std::size_t>(dy) * m_clipboard.width + dx];
                m_layers[l].tiles[static_cast<std::size_t>(destY) * m_width + destX] = gid;
            }
        }
    }
    for (const ObjectSpawn& o : m_clipboard.objects) {
        const int destX = x + o.position.x;
        const int destY = y + o.position.y;
        if (!inBounds(destX, destY)) {
            continue;
        }
        ObjectSpawn puesto = o;
        const int despX = destX - o.position.x;
        const int despY = destY - o.position.y;
        puesto.position = GridCoord{destX, destY};
        puesto.patrolMin = GridCoord{o.patrolMin.x + despX, o.patrolMin.y + despY};
        puesto.patrolMax = GridCoord{o.patrolMax.x + despX, o.patrolMax.y + despY};
        // Una celda, un objeto: pegar encima reemplaza, igual que colocar.
        for (std::size_t i = 0; i < m_objects.size(); ++i) {
            if (m_objects[i].position.x == destX && m_objects[i].position.y == destY) {
                m_objects.erase(m_objects.begin() + static_cast<std::ptrdiff_t>(i));
                break;
            }
        }
        m_objects.push_back(puesto);
    }
}

void EditorState::eraseSelection() {
    if (!m_selection.valid()) {
        return;
    }
    recordUndo();
    for (Layer& capa : m_layers) {
        for (int y = m_selection.y0; y <= m_selection.y1; ++y) {
            for (int x = m_selection.x0; x <= m_selection.x1; ++x) {
                capa.tiles[static_cast<std::size_t>(y) * m_width + x] = 0;
            }
        }
    }
    const Rect r = m_selection;
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
                                   [&r](const ObjectSpawn& o) {
                                       return o.position.x >= r.x0 && o.position.x <= r.x1 &&
                                              o.position.y >= r.y0 && o.position.y <= r.y1;
                                   }),
                    m_objects.end());
}

// =====================================================================
// PROPIEDADES DE UN OBJETO YA COLOCADO
// =====================================================================

ObjectSpawn* EditorState::mutableObjectAt(int x, int y) {
    for (ObjectSpawn& o : m_objects) {
        if (o.position.x == x && o.position.y == y) {
            return &o;
        }
    }
    return nullptr;
}

void EditorState::setObjectPatrol(int x, int y, GridCoord min, GridCoord max) {
    ObjectSpawn* o = mutableObjectAt(x, y);
    if (o == nullptr) {
        return;
    }
    // Se normaliza igual que la seleccion y se recorta al mapa: una
    // patrulla que sale del mapa manda al PNJ a caminar contra la nada.
    GridCoord a{std::max(0, std::min(min.x, max.x)), std::max(0, std::min(min.y, max.y))};
    GridCoord b{std::min(m_width - 1, std::max(min.x, max.x)),
                std::min(m_height - 1, std::max(min.y, max.y))};
    if (o->patrolMin.x == a.x && o->patrolMin.y == a.y && o->patrolMax.x == b.x &&
        o->patrolMax.y == b.y) {
        return;
    }
    recordUndo();
    mutableObjectAt(x, y)->patrolMin = a;
    mutableObjectAt(x, y)->patrolMax = b;
}

void EditorState::setObjectTargetPosition(int x, int y, GridCoord target) {
    ObjectSpawn* o = mutableObjectAt(x, y);
    if (o == nullptr) {
        return;
    }
    recordUndo();
    ObjectSpawn* m = mutableObjectAt(x, y);
    m->targetPosition = target;
    // Sin esta bandera, exportLevelJson no escribe el destino y el
    // jugador sigue apareciendo en el playerStart del nivel de destino:
    // el dato estaria puesto y no serviria de nada.
    m->hasTargetPosition = true;
}

EditorState::Snapshot EditorState::snapshot() const {
    return Snapshot{m_layers, m_activeLayer, m_objects, m_playerStart};
}

void EditorState::restore(Snapshot state) {
    m_layers = std::move(state.layers);
    // La capa activa tambien se restaura: deshacer un "anadir capa" con
    // el indice apuntando a una capa que ya no existe reventaria en el
    // siguiente pincelazo.
    m_activeLayer = state.activeLayer;
    if (m_activeLayer >= layerCount()) {
        m_activeLayer = layerCount() - 1;
    }
    if (m_activeLayer < 0) {
        m_activeLayer = 0;
    }
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

int EditorState::tileAt(int x, int y) const { return tileAt(m_activeLayer, x, y); }

int EditorState::tileAt(int layer, int x, int y) const {
    if (!inBounds(x, y) || layer < 0 || layer >= layerCount()) {
        return 0;
    }
    return m_layers[static_cast<std::size_t>(layer)]
        .tiles[static_cast<std::size_t>(y) * m_width + x];
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

    // Una <layer> por capa, en orden: la primera es la de abajo. Un mapa
    // de una sola capa sale identico al de antes (se llamaba "suelo"),
    // asi que los TMX ya guardados no cambian ni un byte.
    for (std::size_t i = 0; i < m_layers.size(); ++i) {
        const Layer& capa = m_layers[i];
        out << " <layer id=\"" << (i + 1) << "\" name=\"" << xmlEscape(capa.name) << "\" width=\""
            << m_width << "\" height=\"" << m_height << "\">\n";
        out << "  <data encoding=\"csv\">\n";
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                out << capa.tiles[static_cast<std::size_t>(y) * m_width + x];
                // CSV de Tiled: coma tras cada valor salvo el ultimo del mapa.
                if (!(y == m_height - 1 && x == m_width - 1)) {
                    out << ",";
                }
            }
            out << "\n";
        }
        out << "  </data>\n";
        out << " </layer>\n";
    }
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
        out << "\n    { \"objectId\": \"" << jsonEscape(o.objectId)
            << "\", \"position\": { \"x\": " << o.position.x << ", \"y\": " << o.position.y << " }";
        // La patrulla se escribe SOLO si difiere de position. Sin esto,
        // abrir un nivel con PNJs que patrullan y volver a guardarlo los
        // dejaba a todos quietos: LevelLoader rellena patrolMin/Max con
        // position cuando faltan, asi que la perdida no daba ni un error,
        // solo una ciudad donde de repente nadie se mueve.
        const bool patrulla = o.patrolMin.x != o.position.x || o.patrolMin.y != o.position.y ||
                              o.patrolMax.x != o.position.x || o.patrolMax.y != o.position.y;
        if (patrulla) {
            out << ", \"patrolMin\": { \"x\": " << o.patrolMin.x << ", \"y\": " << o.patrolMin.y
                << " }, \"patrolMax\": { \"x\": " << o.patrolMax.x << ", \"y\": " << o.patrolMax.y
                << " }";
        }
        // El destino se conserva y ahora tambien se puede editar
        // (setObjectTransition / setObjectTargetPosition).
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
