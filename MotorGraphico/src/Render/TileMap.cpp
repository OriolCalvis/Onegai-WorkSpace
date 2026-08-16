#include "Render/TileMap.h"

#include "Core/Errors/EngineException.h"
#include "Core/Math/IsoMath.h"
#include "Render/Camera.h"

#include <algorithm>
#include <array>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

namespace {

// GID (global tile id de Tiled) -> si esa celda bloquea el movimiento.
// Recorre todos los <tileset> del <map> buscando, para cada <tile id="N">
// embebido, una <property name="collision" type="bool" value="true"/>
// dentro de <properties>, y la traduce a GID absoluto (firstgid + N).
std::unordered_map<int, bool> collectCollisionGids(const tinyxml2::XMLElement* mapElem) {
    std::unordered_map<int, bool> collisionByGid;

    for (const tinyxml2::XMLElement* tileset = mapElem->FirstChildElement("tileset");
         tileset != nullptr; tileset = tileset->NextSiblingElement("tileset")) {
        if (tileset->Attribute("source") != nullptr) {
            throw MapParseException(
                "tileset externo (\"source\") no soportado: solo tilesets embebidos",
                tileset->GetLineNum());
        }

        int firstgid = 0;
        if (tileset->QueryIntAttribute("firstgid", &firstgid) != tinyxml2::XML_SUCCESS) {
            throw MapParseException("<tileset> sin atributo \"firstgid\"", tileset->GetLineNum());
        }

        for (const tinyxml2::XMLElement* tile = tileset->FirstChildElement("tile"); tile != nullptr;
             tile = tile->NextSiblingElement("tile")) {
            int localID = 0;
            if (tile->QueryIntAttribute("id", &localID) != tinyxml2::XML_SUCCESS) {
                continue;
            }

            const tinyxml2::XMLElement* properties = tile->FirstChildElement("properties");
            if (properties == nullptr) {
                continue;
            }
            for (const tinyxml2::XMLElement* prop = properties->FirstChildElement("property");
                 prop != nullptr; prop = prop->NextSiblingElement("property")) {
                const char* name = prop->Attribute("name");
                if (name != nullptr && std::string(name) == "collision" &&
                    prop->BoolAttribute("value", false)) {
                    collisionByGid[firstgid + localID] = true;
                }
            }
        }
    }

    return collisionByGid;
}

// Resuelve "relative" (tal y como viene en el TMX) contra la CARPETA de
// "basePath" (el propio archivo TMX), y normaliza los ".." resultantes:
// "assets/maps/x.tmx" + "../textures/y.png" -> "assets/textures/y.png".
// Se hace a mano y no con std::filesystem porque este motor apunta a
// C++17 sobre varias plataformas y filesystem obliga a enlazar
// stdc++fs en algunos toolchains -- para una concatenacion de rutas no
// compensa esa dependencia. Una ruta absoluta se devuelve intacta.
std::string resolveRelativeTo(const std::string& basePath, const std::string& relative) {
    if (relative.empty() || relative[0] == '/') {
        return relative;
    }
    const std::size_t slash = basePath.find_last_of('/');
    if (slash == std::string::npos) {
        return relative;  // el TMX estaba en el directorio actual
    }

    // Trocear "dir/../otro" y resolver los ".." contra lo ya acumulado.
    std::string combined = basePath.substr(0, slash + 1) + relative;
    std::vector<std::string> parts;
    std::istringstream stream(combined);
    std::string part;
    while (std::getline(stream, part, '/')) {
        if (part == "." || part.empty()) {
            continue;
        }
        if (part == ".." && !parts.empty() && parts.back() != "..") {
            parts.pop_back();
            continue;
        }
        parts.push_back(part);
    }

    std::string result;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            result += '/';
        }
        result += parts[i];
    }
    return result;
}

std::vector<int> parseCsvRow(const std::string& csv, int line) {
    std::vector<int> gids;
    std::istringstream stream(csv);
    std::string token;
    while (std::getline(stream, token, ',')) {
        // El CSV de Tiled trae saltos de linea dentro del texto de
        // <data>: cada token puede llevar "\n"/espacios alrededor.
        std::size_t start = token.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            continue;  // token vacio (linea en blanco, coma final...)
        }
        std::size_t end = token.find_last_not_of(" \t\r\n");
        std::string trimmed = token.substr(start, end - start + 1);

        try {
            gids.push_back(std::stoi(trimmed));
        } catch (const std::exception&) {
            throw MapParseException(
                "valor no numerico en <data encoding=\"csv\">: \"" + trimmed + "\"", line);
        }
    }
    return gids;
}

}  // namespace

void TileMap::parseOrThrow(const std::string& path) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS) {
        throw MapParseException(
            "no se pudo abrir/parsear el XML (" + std::string(doc.ErrorStr()) + ")", 0);
    }

    tinyxml2::XMLElement* mapElem = doc.RootElement();
    if (mapElem == nullptr || std::string(mapElem->Name()) != "map") {
        throw MapParseException("elemento raiz distinto de <map>", 0);
    }

    int width = 0, height = 0, tileWidth = 0, tileHeight = 0;
    if (mapElem->QueryIntAttribute("width", &width) != tinyxml2::XML_SUCCESS ||
        mapElem->QueryIntAttribute("height", &height) != tinyxml2::XML_SUCCESS ||
        mapElem->QueryIntAttribute("tilewidth", &tileWidth) != tinyxml2::XML_SUCCESS ||
        mapElem->QueryIntAttribute("tileheight", &tileHeight) != tinyxml2::XML_SUCCESS) {
        throw MapParseException("<map> sin width/height/tilewidth/tileheight",
                                mapElem->GetLineNum());
    }
    if (width <= 0 || height <= 0) {
        throw MapParseException("<map> con width/height no positivos", mapElem->GetLineNum());
    }

    std::unordered_map<int, bool> collisionByGid = collectCollisionGids(mapElem);

    std::vector<std::vector<std::vector<Tile>>> layers;

    for (tinyxml2::XMLElement* layerElem = mapElem->FirstChildElement("layer");
         layerElem != nullptr; layerElem = layerElem->NextSiblingElement("layer")) {
        tinyxml2::XMLElement* dataElem = layerElem->FirstChildElement("data");
        if (dataElem == nullptr) {
            throw MapParseException("<layer> sin <data>", layerElem->GetLineNum());
        }

        const char* encoding = dataElem->Attribute("encoding");
        if (encoding == nullptr || std::string(encoding) != "csv") {
            throw MapParseException("<data> con encoding no soportado (solo \"csv\")",
                                    dataElem->GetLineNum());
        }

        const char* text = dataElem->GetText();
        std::vector<int> gids = parseCsvRow(text != nullptr ? text : "", dataElem->GetLineNum());

        if (static_cast<int>(gids.size()) != width * height) {
            throw MapParseException("<data> tiene " + std::to_string(gids.size()) +
                                        " celdas, se esperaban " + std::to_string(width * height) +
                                        " (" + std::to_string(width) + "x" +
                                        std::to_string(height) + ")",
                                    dataElem->GetLineNum());
        }

        std::vector<std::vector<Tile>> grid(static_cast<std::size_t>(height));
        for (int y = 0; y < height; ++y) {
            grid[static_cast<std::size_t>(y)].reserve(static_cast<std::size_t>(width));
            for (int x = 0; x < width; ++x) {
                int gid = gids[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
                               static_cast<std::size_t>(x)];
                bool collision = collisionByGid.count(gid) > 0;
                grid[static_cast<std::size_t>(y)].emplace_back(gid, collision);
            }
        }
        layers.push_back(std::move(grid));
    }

    if (layers.empty()) {
        throw MapParseException("<map> sin ninguna <layer>", mapElem->GetLineNum());
    }

    // Datos del PRIMER <tileset> (ver la nota de m_tilesetImage en el
    // .h): con que imagen se dibuja este mapa y como esta dividida. No
    // es obligatorio -- un TMX sin <image> sigue cargando, y quien lo
    // dibuje tendra que decidir que textura usar.
    std::string tilesetImage;
    std::string tilesetName;
    std::string tilesetImageSource;
    int tilesetImageWidth = 0;
    int tilesetImageHeight = 0;
    int tilesetTileCount = 0;
    int tilesetTileWidth = 0;
    int tilesetTileHeight = 0;
    int tilesetColumns = 0;
    int tilesetFirstGid = 1;
    if (tinyxml2::XMLElement* tilesetElem = mapElem->FirstChildElement("tileset");
        tilesetElem != nullptr) {
        tilesetElem->QueryIntAttribute("firstgid", &tilesetFirstGid);
        tilesetElem->QueryIntAttribute("tilewidth", &tilesetTileWidth);
        tilesetElem->QueryIntAttribute("tileheight", &tilesetTileHeight);
        tilesetElem->QueryIntAttribute("columns", &tilesetColumns);
        tilesetElem->QueryIntAttribute("tilecount", &tilesetTileCount);
        if (const char* n = tilesetElem->Attribute("name")) {
            tilesetName = n;
        }
        if (tinyxml2::XMLElement* imageElem = tilesetElem->FirstChildElement("image");
            imageElem != nullptr) {
            const char* source = imageElem->Attribute("source");
            if (source != nullptr) {
                // La ruta del TMX es relativa AL PROPIO TMX ("../textures/
                // x.png"); aqui se resuelve contra su carpeta para que el
                // llamador pueda pasarla a TextureManager tal cual, sin
                // tener que saber donde estaba el mapa. La original se
                // guarda aparte porque es la que hay que volver a escribir
                // al guardar.
                tilesetImage = resolveRelativeTo(path, source);
                tilesetImageSource = source;
            }
            imageElem->QueryIntAttribute("width", &tilesetImageWidth);
            imageElem->QueryIntAttribute("height", &tilesetImageHeight);
        }
    }

    m_width = width;
    m_height = height;
    m_tileWidth = tileWidth;
    m_tileHeight = tileHeight;
    m_layers = std::move(layers);
    m_tilesetImage = std::move(tilesetImage);
    m_tilesetName = std::move(tilesetName);
    m_tilesetImageSource = std::move(tilesetImageSource);
    m_tilesetImageWidth = tilesetImageWidth;
    m_tilesetImageHeight = tilesetImageHeight;
    m_tilesetTileCount = tilesetTileCount;
    // Ordenados: el TMX no garantiza orden y el editor los vuelve a
    // escribir; que la lista baile entre guardados haria que el diff de
    // un mapa cambiara sin que nadie lo haya tocado.
    m_collisionGids.clear();
    m_collisionGids.reserve(collisionByGid.size());
    for (const auto& par : collisionByGid) {
        if (par.second) {
            m_collisionGids.push_back(par.first);
        }
    }
    std::sort(m_collisionGids.begin(), m_collisionGids.end());
    m_tilesetTileWidth = tilesetTileWidth;
    m_tilesetTileHeight = tilesetTileHeight;
    m_tilesetColumns = tilesetColumns;
    m_tilesetFirstGid = tilesetFirstGid;
}

Result<bool> TileMap::loadFromFile(const std::string& path) {
    try {
        parseOrThrow(path);
    } catch (const std::exception& e) {
        return Result<bool>::Error(std::string("Error cargando TileMap '") + path +
                                   "': " + e.what());
    }
    return Result<bool>::Ok(true);
}

const Tile& TileMap::getTile(int layer, int x, int y) const {
    if (layer < 0 || layer >= static_cast<int>(m_layers.size()) || y < 0 || y >= m_height ||
        x < 0 || x >= m_width) {
        throw std::out_of_range("TileMap::getTile(" + std::to_string(layer) + ", " +
                                std::to_string(x) + ", " + std::to_string(y) + ") fuera de rango");
    }
    return m_layers[static_cast<std::size_t>(layer)][static_cast<std::size_t>(y)]
                   [static_cast<std::size_t>(x)];
}

Tile& TileMap::getTile(int layer, int x, int y) {
    return const_cast<Tile&>(static_cast<const TileMap&>(*this).getTile(layer, x, y));
}

GridCoord TileMap::screenToGrid(const Vector2& screenPos) const {
    return IsoMath::screenToGrid(screenPos, static_cast<float>(m_tileWidth),
                                 static_cast<float>(m_tileHeight));
}

Vector2 TileMap::gridToScreen(const GridCoord& grid) const {
    return IsoMath::gridToScreen(grid, static_cast<float>(m_tileWidth),
                                 static_cast<float>(m_tileHeight));
}

GridBounds TileMap::visibleRange(const Camera& camera) const {
    if (m_width <= 0 || m_height <= 0) {
        return GridBounds{};  // mapa vacio (sin loadFromFile() todavia): rango vacio
    }

    const std::array<Vector2, 4> corners = {
        camera.screenToWorld(Vector2{0.0f, 0.0f}),
        camera.screenToWorld(Vector2{static_cast<float>(camera.viewportWidth()), 0.0f}),
        camera.screenToWorld(Vector2{0.0f, static_cast<float>(camera.viewportHeight())}),
        camera.screenToWorld(Vector2{static_cast<float>(camera.viewportWidth()),
                                     static_cast<float>(camera.viewportHeight())}),
    };

    int minX = screenToGrid(corners[0]).x;
    int maxX = minX;
    int minY = screenToGrid(corners[0]).y;
    int maxY = minY;
    for (int i = 1; i < 4; ++i) {
        GridCoord g = screenToGrid(corners[i]);
        minX = std::min(minX, g.x);
        maxX = std::max(maxX, g.x);
        minY = std::min(minY, g.y);
        maxY = std::max(maxY, g.y);
    }

    GridBounds bounds;
    bounds.minX = std::max(0, minX - 1);
    bounds.maxX = std::min(m_width - 1, maxX + 1);
    bounds.minY = std::max(0, minY - 1);
    bounds.maxY = std::min(m_height - 1, maxY + 1);
    return bounds;
}
