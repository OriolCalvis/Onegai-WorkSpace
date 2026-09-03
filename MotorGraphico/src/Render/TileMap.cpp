#include "Render/TileMap.h"

#include "Render/Culling.h"

#include "Core/Errors/EngineException.h"
#include "Import/TmxMapImporter.h"
#include "Render/Camera.h"
#include "Runtime/RuntimeTileMapBuilder.h"

#include <algorithm>
#include <array>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <tinyxml2.h>

namespace {

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
        if (tilesetElem->Attribute("source") != nullptr) {
            throw MapParseException(
                "tileset externo (\"source\") no soportado por TileMap heredado",
                tilesetElem->GetLineNum());
        }
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

    const ids::MapId importId = ids::MapId::parse("tmx_legacy_import").value();
    const TmxMapImportReport report = TmxMapImporter().importFile(path, importId);
    if (report.hasErrors()) {
        const ImportIssue& issue = report.issues.front();
        throw MapParseException(issue.message, issue.line);
    }
    const Result<RuntimeTileMap> runtime = RuntimeTileMapBuilder().build(report.document);
    if (!runtime.isOk()) {
        throw MapParseException(runtime.errorMessage(), mapElem->GetLineNum());
    }
    static_cast<RuntimeTileMap&>(*this) = runtime.value();
    m_tilesetImage = std::move(tilesetImage);
    m_tilesetName = std::move(tilesetName);
    m_tilesetImageSource = std::move(tilesetImageSource);
    m_tilesetImageWidth = tilesetImageWidth;
    m_tilesetImageHeight = tilesetImageHeight;
    m_tilesetTileCount = tilesetTileCount;
    m_collisionGids = report.document.collisionGids();
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

GridBounds TileMap::visibleRange(const Camera& camera) const {
    // C-19 — Delega en la unica funcion de culling del motor
    // (Render/Culling.h). Este metodo se queda como API de
    // compatibilidad del importador TMX (ver TileMap.h), pero ya no
    // duplica el calculo: era copia literal del que tenia
    // IsometricRenderer.cpp.
    return VisibleGridRange(*this, camera);
}
