#include "Import/TmxMapImporter.h"

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include <tinyxml2.h>

namespace {

void addIssue(TmxMapImportReport& report, ImportSeverity severity, const std::string& message,
              int line = 0) {
    report.issues.push_back(ImportIssue{severity, message, line});
}

bool readInt(const tinyxml2::XMLElement* element, const char* attribute, int* output,
             TmxMapImportReport& report, const char* context) {
    if (element->QueryIntAttribute(attribute, output) != tinyxml2::XML_SUCCESS) {
        addIssue(report, ImportSeverity::Error,
                 std::string(context) + " sin atributo entero '" + attribute + "'",
                 element->GetLineNum());
        return false;
    }
    return true;
}

bool parseCsv(const char* text, std::vector<int>* gids, TmxMapImportReport& report, int line) {
    const std::string csv = text != nullptr ? text : "";
    std::size_t cursor = 0;
    while (cursor <= csv.size()) {
        const std::size_t comma = csv.find(',', cursor);
        const std::size_t end = comma == std::string::npos ? csv.size() : comma;
        const std::string token = csv.substr(cursor, end - cursor);
        const std::size_t first = token.find_first_not_of(" \t\r\n");
        if (first != std::string::npos) {
            const std::size_t last = token.find_last_not_of(" \t\r\n");
            const std::string trimmed = token.substr(first, last - first + 1);
            char* parsedEnd = nullptr;
            const long value = std::strtol(trimmed.c_str(), &parsedEnd, 10);
            if (parsedEnd == trimmed.c_str() || *parsedEnd != '\0' || value < 0 ||
                value > 2147483647L) {
                addIssue(report, ImportSeverity::Error,
                         "<data> contiene GID CSV inválido '" + trimmed + "'", line);
                return false;
            }
            gids->push_back(static_cast<int>(value));
        }
        if (comma == std::string::npos) {
            break;
        }
        cursor = comma + 1;
    }
    return true;
}

ids::LayerId layerIdFor(const char* tmxName, int index, TmxMapImportReport& report, int line) {
    const std::string fallback = "layer_" + std::to_string(index);
    if (tmxName != nullptr && *tmxName != '\0') {
        const Result<ids::LayerId> parsed = ids::LayerId::parse(tmxName);
        if (parsed.isOk()) {
            return parsed.value();
        }
        addIssue(report, ImportSeverity::Warning,
                 "nombre de capa TMX no válido como ID; se usa '" + fallback + "'", line);
    } else {
        addIssue(report, ImportSeverity::Warning,
                 "capa TMX sin nombre; se usa ID interno '" + fallback + "'", line);
    }
    return ids::LayerId::parse(fallback).value();
}

void collectCollisionGids(const tinyxml2::XMLElement* map, TmxMapImportReport& report,
                          std::vector<int>* collisionGids) {
    for (const tinyxml2::XMLElement* tileset = map->FirstChildElement("tileset"); tileset != nullptr;
         tileset = tileset->NextSiblingElement("tileset")) {
        if (tileset->Attribute("source") != nullptr) {
            addIssue(report, ImportSeverity::Warning,
                     "tileset externo no inspeccionado: se importan GIDs pero no su colisión",
                     tileset->GetLineNum());
            continue;
        }
        int firstGid = 0;
        if (!readInt(tileset, "firstgid", &firstGid, report, "<tileset>")) {
            continue;
        }
        for (const tinyxml2::XMLElement* tile = tileset->FirstChildElement("tile"); tile != nullptr;
             tile = tile->NextSiblingElement("tile")) {
            int localId = 0;
            if (!readInt(tile, "id", &localId, report, "<tile>")) {
                continue;
            }
            const tinyxml2::XMLElement* properties = tile->FirstChildElement("properties");
            if (properties == nullptr) {
                continue;
            }
            for (const tinyxml2::XMLElement* property = properties->FirstChildElement("property");
                 property != nullptr; property = property->NextSiblingElement("property")) {
                const char* name = property->Attribute("name");
                if (name != nullptr && std::string(name) == "collision" &&
                    property->BoolAttribute("value", false)) {
                    collisionGids->push_back(firstGid + localId);
                }
            }
        }
    }
}

}  // namespace

TmxMapImportReport TmxMapImporter::importFile(const std::string& path, ids::MapId mapId) const {
    TmxMapImportReport report;
    if (mapId.empty()) {
        addIssue(report, ImportSeverity::Error, "el importador necesita un MapId válido");
        return report;
    }

    tinyxml2::XMLDocument xml;
    if (xml.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS) {
        addIssue(report, ImportSeverity::Error,
                 "no se pudo abrir o parsear TMX: " + std::string(xml.ErrorStr()), xml.ErrorLineNum());
        return report;
    }
    const tinyxml2::XMLElement* map = xml.RootElement();
    if (map == nullptr || std::string(map->Name()) != "map") {
        addIssue(report, ImportSeverity::Error, "la raíz TMX debe ser <map>");
        return report;
    }
    if (map->BoolAttribute("infinite", false)) {
        addIssue(report, ImportSeverity::Error,
                 "TMX infinito no soportado: MapDocument requiere dimensiones finitas", map->GetLineNum());
        return report;
    }

    int width = 0;
    int height = 0;
    int tileWidth = 0;
    int tileHeight = 0;
    const bool hasDimensions = readInt(map, "width", &width, report, "<map>") &&
                               readInt(map, "height", &height, report, "<map>") &&
                               readInt(map, "tilewidth", &tileWidth, report, "<map>") &&
                               readInt(map, "tileheight", &tileHeight, report, "<map>");
    if (!hasDimensions || width <= 0 || height <= 0 || tileWidth <= 0 || tileHeight <= 0) {
        if (hasDimensions) {
            addIssue(report, ImportSeverity::Error, "<map> tiene dimensiones no positivas",
                     map->GetLineNum());
        }
        return report;
    }

    const char* orientation = map->Attribute("orientation");
    if (orientation != nullptr && std::string(orientation) != "orthogonal") {
        addIssue(report, ImportSeverity::Warning,
                 "orientación TMX distinta de orthogonal importada como grid genérico",
                 map->GetLineNum());
    }
    if (map->FirstChildElement("properties") != nullptr) {
        addIssue(report, ImportSeverity::Warning,
                 "propiedades de <map> no representadas aún en MapDocument", map->GetLineNum());
    }

    MapDocument document(mapId, mapId.value(), width, height, tileWidth, tileHeight);
    std::vector<int> collisionGids;
    collectCollisionGids(map, report, &collisionGids);
    const Result<bool> collisionResult = document.setCollisionGids(std::move(collisionGids));
    if (!collisionResult.isOk()) {
        addIssue(report, ImportSeverity::Error, collisionResult.errorMessage(), map->GetLineNum());
    }

    int layerOrder = 0;
    for (const tinyxml2::XMLElement* layer = map->FirstChildElement("layer"); layer != nullptr;
         layer = layer->NextSiblingElement("layer"), ++layerOrder) {
        int layerWidth = width;
        int layerHeight = height;
        if (!readInt(layer, "width", &layerWidth, report, "<layer>") ||
            !readInt(layer, "height", &layerHeight, report, "<layer>")) {
            continue;
        }
        if (layerWidth != width || layerHeight != height) {
            addIssue(report, ImportSeverity::Error,
                     "<layer> con dimensiones distintas a las del <map>", layer->GetLineNum());
            continue;
        }
        const tinyxml2::XMLElement* data = layer->FirstChildElement("data");
        if (data == nullptr) {
            addIssue(report, ImportSeverity::Error, "<layer> sin <data>", layer->GetLineNum());
            continue;
        }
        const char* encoding = data->Attribute("encoding");
        if (encoding == nullptr || std::string(encoding) != "csv") {
            addIssue(report, ImportSeverity::Error,
                     "<data> usa encoding no soportado; solo CSV es importable", data->GetLineNum());
            continue;
        }
        std::vector<int> gids;
        if (!parseCsv(data->GetText(), &gids, report, data->GetLineNum())) {
            continue;
        }
        if (gids.size() != static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {
            addIssue(report, ImportSeverity::Error,
                     "<data> no tiene width*height GIDs", data->GetLineNum());
            continue;
        }

        const char* name = layer->Attribute("name");
        const ids::LayerId id = layerIdFor(name, layerOrder, report, layer->GetLineNum());
        MapLayerMeta meta{id, name != nullptr && *name != '\0' ? name : id.value(),
                          layer->BoolAttribute("visible", true), layer->BoolAttribute("locked", false),
                          layer->FloatAttribute("opacity", 1.0f), layerOrder};
        const Result<bool> added = document.addLayer(
            MapLayer::tiles(std::move(meta), TileLayerDocument(width, height, std::move(gids))));
        if (!added.isOk()) {
            addIssue(report, ImportSeverity::Error, "capa TMX inválida: " + added.errorMessage(),
                     layer->GetLineNum());
        }
    }

    for (const tinyxml2::XMLElement* child = map->FirstChildElement(); child != nullptr;
         child = child->NextSiblingElement()) {
        const std::string name = child->Name();
        if (name == "objectgroup" || name == "group" || name == "imagelayer") {
            addIssue(report, ImportSeverity::Warning,
                     "elemento TMX <" + name + "> no representado; se omitió de MapDocument",
                     child->GetLineNum());
        }
    }

    if (!report.hasErrors()) {
        const Result<bool> validation = document.validate();
        if (!validation.isOk()) {
            addIssue(report, ImportSeverity::Error, "MapDocument importado inválido: " +
                                                     validation.errorMessage(), map->GetLineNum());
        }
    }
    if (!report.hasErrors()) {
        report.document = std::move(document);
    }
    return report;
}
