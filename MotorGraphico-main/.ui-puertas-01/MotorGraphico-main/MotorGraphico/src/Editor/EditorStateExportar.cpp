// Sacar el documento fuera: TMX para Tiled y JSON de nivel.
//
// Es la frontera con el formato: lo que aqui se escribe tiene que poder
// volver a leerse. Por eso el escapado de XML y de JSON vive aqui y en
// ningun otro sitio.
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


namespace {

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
    JsonWriter writer;
    writer.value(text);
    const std::string encoded = writer.str();
    return encoded.substr(1, encoded.size() - 3);  // comillas y salto final
}

// Minusculas ASCII. No se usa std::tolower con locale a proposito: los
// ids del catalogo son ASCII (lo garantiza el exportador) y una locale
// turca convertiria la I en otra cosa.

}  // namespace

std::string EditorState::exportTmx(const TmxTilesetSettings& s) const {
    // Mismo esqueleto exacto que assets/maps/test_map.tmx (el archivo de
    // referencia del parser): tileset embebido con <properties> de
    // colision por tile, una capa CSV. Los atributos que TileMap::
    // loadFromFile() ignora (version, renderorder...) se emiten igual
    // para que el archivo tambien abra en Tiled.
    std::ostringstream out;
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<map version=\"1.10\" orientation=\"orthogonal\" renderorder=\"right-down\""
        << " width=\"" << width() << "\" height=\"" << height() << "\" tilewidth=\"" << s.tileWidth
        << "\" tileheight=\"" << s.tileHeight
        << "\" infinite=\"0\" nextlayerid=\"2\" nextobjectid=\"1\">\n";

    out << " <tileset firstgid=\"1\" name=\"" << xmlEscape(s.tilesetName) << "\" tilewidth=\""
        << s.tilesetTileWidth << "\" tileheight=\"" << s.tilesetTileHeight << "\" tilecount=\""
        << s.tileCount << "\" columns=\"" << s.columns << "\">\n";
    out << "  <image source=\"" << xmlEscape(s.imageSource) << "\" width=\"" << s.imageWidth << "\" height=\""
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

    // Exportacion de interoperabilidad. El orden y la presentacion proceden
    // del documento; las entidades se exportan por exportLevelJson().
    for (int i = 0; i < layerCount(); ++i) {
        const MapLayer& capa = *tileLayer(i);
        out << " <layer id=\"" << (i + 1) << "\" name=\"" << xmlEscape(capa.meta().name) << "\" width=\""
            << width() << "\" height=\"" << height() << "\" visible=\""
            << (capa.meta().visible ? 1 : 0) << "\" opacity=\"" << capa.meta().opacity << "\">\n";
        out << "  <data encoding=\"csv\">\n";
        for (int y = 0; y < height(); ++y) {
            for (int x = 0; x < width(); ++x) {
                out << std::get<TileLayerDocument>(capa.data()).gidAt(x, y);
                // CSV de Tiled: coma tras cada valor salvo el ultimo del mapa.
                if (!(y == height() - 1 && x == width() - 1)) {
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
                                         const std::string& mapPath,
                                         const std::string& mapDocumentPath) const {
    std::ostringstream out;
    out << "{\n";
    out << "  \"name\": \"" << jsonEscape(levelName) << "\",\n";
    out << "  \"map\": \"" << jsonEscape(mapPath) << "\",\n";
    if (!mapDocumentPath.empty())
        out << "  \"mapDocument\": \"" << jsonEscape(mapDocumentPath) << "\",\n";
    out << "  \"playerStart\": { \"x\": " << m_playerStart.x << ", \"y\": " << m_playerStart.y
        << " },\n";
    out << "  \"objects\": [";
    for (std::size_t i = 0; i < objects().size(); ++i) {
        const ObjectSpawn& o = objects()[i];
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
        // Botin y evento: se escriben solo si llevan contenido, igual que
        // el resto de campos opcionales, para que un nivel sin cofres ni
        // eventos quede identico a antes de existir estos campos.
        if (!o.loot.empty()) {
            out << ", \"loot\": [";
            for (std::size_t l = 0; l < o.loot.size(); ++l) {
                if (l > 0) {
                    out << ", ";
                }
                out << "\"" << jsonEscape(o.loot[l]) << "\"";
            }
            out << "]";
        }
        if (!o.lootTable.empty()) {
            out << ", \"lootTable\": \"" << jsonEscape(o.lootTable) << "\"";
        }
        if (!o.eventId.empty()) {
            out << ", \"eventId\": \"" << jsonEscape(o.eventId) << "\"";
        }
        if (!o.displayName.empty()) {
            out << ", \"displayName\": \"" << jsonEscape(o.displayName) << "\"";
        }
        if (!o.variant.empty()) {
            out << ", \"variant\": \"" << jsonEscape(o.variant) << "\"";
        }
        if (o.scale != 1.0f) {
            out << ", \"scale\": " << o.scale;
        }
        if (!o.effectOverride.empty()) {
            out << ", \"effectOverride\": \"" << jsonEscape(o.effectOverride) << "\"";
        }
        if (!o.properties.empty()) {
            out << ", \"properties\": { ";
            std::size_t propertyIndex = 0;
            for (const auto& property : o.properties) {
                if (propertyIndex++ > 0) {
                    out << ", ";
                }
                out << "\"" << jsonEscape(property.first) << "\": \""
                    << jsonEscape(property.second) << "\"";
            }
            out << " }";
        }
        out << " }";
    }
    out << (objects().empty() ? "]\n" : "\n  ]\n");
    out << "}\n";
    return out.str();
}
