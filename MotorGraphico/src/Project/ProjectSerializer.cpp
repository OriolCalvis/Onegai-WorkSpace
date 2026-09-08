#include "Project/ProjectSerializer.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {

constexpr int kProjectSchemaVersion = 1;
constexpr int kMapSchemaVersion = 1;

Result<std::string> requiredString(const JsonValue& object, const char* key,
                                   const char* context) {
    const JsonValue& value = object[key];
    if (!value.isString() || value.asString().empty()) {
        return Result<std::string>::Error(std::string(context) + ": falta string no vacío '" + key +
                                          "'");
    }
    return Result<std::string>::Ok(value.asString());
}

Result<int> requiredInt(const JsonValue& object, const char* key, const char* context) {
    const JsonValue& value = object[key];
    if (!value.isNumber()) {
        return Result<int>::Error(std::string(context) + ": falta número '" + key + "'");
    }
    return Result<int>::Ok(value.asInt());
}

Result<bool> requiredBool(const JsonValue& object, const char* key, const char* context) {
    const JsonValue& value = object[key];
    if (!value.isBool()) {
        return Result<bool>::Error(std::string(context) + ": falta booleano '" + key + "'");
    }
    return Result<bool>::Ok(value.asBool());
}

Result<bool> checkSchema(const JsonValue& root, int expected, const char* context) {
    if (!root.isObject()) {
        return Result<bool>::Error(std::string(context) + ": la raíz debe ser un objeto JSON");
    }
    if (!root["schemaVersion"].isNumber()) {
        return Result<bool>::Error(std::string(context) + ": falta 'schemaVersion'");
    }
    const double actual = root["schemaVersion"].asNumber();
    if (actual != expected) {
        return Result<bool>::Error(std::string(context) + ": schemaVersion incompatible v" +
                                   std::to_string(actual) + "; se esperaba v" +
                                   std::to_string(expected));
    }
    return Result<bool>::Ok(true);
}

void writeStrings(JsonWriter& writer, const char* key, const std::vector<std::string>& values) {
    writer.key(key);
    writer.beginArray();
    for (const std::string& value : values) {
        writer.value(value);
    }
    writer.endArray();
}

Result<std::vector<std::string>> readStrings(const JsonValue& object, const char* key,
                                             const char* context) {
    const JsonValue& values = object[key];
    if (!values.isArray()) {
        return Result<std::vector<std::string>>::Error(std::string(context) + ": falta array '" +
                                                       key + "'");
    }
    std::vector<std::string> result;
    result.reserve(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (!values[index].isString()) {
            return Result<std::vector<std::string>>::Error(
                std::string(context) + ": '" + key + "' contiene un valor no string");
        }
        result.push_back(values[index].asString());
    }
    return Result<std::vector<std::string>>::Ok(std::move(result));
}

template <class Id>
Result<Id> readId(const JsonValue& object, const char* key, const char* context) {
    const Result<std::string> value = requiredString(object, key, context);
    if (!value.isOk()) {
        return Result<Id>::Error(value.errorMessage());
    }
    const Result<Id> parsed = Id::parse(value.value());
    if (!parsed.isOk()) {
        return Result<Id>::Error(std::string(context) + ": '" + key + "' " +
                                 parsed.errorMessage());
    }
    return parsed;
}

Result<GridCoord> readGridCoord(const JsonValue& value, const char* context) {
    if (!value.isObject()) {
        return Result<GridCoord>::Error(std::string(context) + ": posición debe ser objeto");
    }
    const Result<int> x = requiredInt(value, "x", context);
    const Result<int> y = requiredInt(value, "y", context);
    if (!x.isOk()) {
        return Result<GridCoord>::Error(x.errorMessage());
    }
    if (!y.isOk()) {
        return Result<GridCoord>::Error(y.errorMessage());
    }
    return Result<GridCoord>::Ok(GridCoord{x.value(), y.value()});
}

void writeGridCoord(JsonWriter& writer, const char* key, GridCoord position) {
    writer.key(key);
    writer.beginObject();
    writer.member("x", position.x);
    writer.member("y", position.y);
    writer.endObject();
}

Result<bool> atomicWrite(const std::string& path, const std::string& text) {
    if (path.empty()) {
        return Result<bool>::Error("ruta de documento vacía");
    }
    const std::string temporary = path + ".tmp";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) {
            return Result<bool>::Error("no se puede abrir el temporal '" + temporary + "'");
        }
        output.write(text.data(), static_cast<std::streamsize>(text.size()));
        output.flush();
        if (!output) {
            std::remove(temporary.c_str());
            return Result<bool>::Error("no se puede escribir el temporal '" + temporary + "'");
        }
    }

#if defined(_WIN32)
    if (!MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        std::remove(temporary.c_str());
        return Result<bool>::Error("no se puede sustituir atómicamente '" + path + "'");
    }
#else
    if (std::rename(temporary.c_str(), path.c_str()) != 0) {
        std::remove(temporary.c_str());
        return Result<bool>::Error("no se puede sustituir atómicamente '" + path + "'");
    }
#endif
    return Result<bool>::Ok(true);
}

Result<std::string> readFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return Result<std::string>::Error("no se puede abrir '" + path + "'");
    }
    std::ostringstream content;
    content << input.rdbuf();
    if (!input.good() && !input.eof()) {
        return Result<std::string>::Error("no se puede leer '" + path + "'");
    }
    return Result<std::string>::Ok(content.str());
}

}  // namespace

Result<std::string> ProjectSerializer::serializeProject(const Editor::Project& project) {
    const Result<bool> validation = project.validate();
    if (!validation.isOk()) {
        return Result<std::string>::Error("proyecto inválido: " + validation.errorMessage());
    }

    JsonWriter writer;
    writer.beginObject();
    writer.member("schemaVersion", kProjectSchemaVersion);
    writer.member("id", project.id);
    writer.member("name", project.name);
    writer.member("description", project.description);
    writer.member("epoch", project.epoch);
    writer.member("author", project.author);
    writer.member("prefix", project.prefix);
    writer.key("settings");
    writer.beginObject();
    writer.member("schemaVersion", project.settings.schemaVersion);
    writer.member("mapsDirectory", project.settings.mapsDirectory);
    writer.member("levelsDirectory", project.settings.levelsDirectory);
    writer.member("adventuresDirectory", project.settings.adventuresDirectory);
    writer.member("catalogsDirectory", project.settings.catalogsDirectory);
    writer.member("initialMap", project.settings.initialMap);
    writer.endObject();
    writer.member("entry", project.entry);
    writeStrings(writer, "levels", project.levels);
    writeStrings(writer, "maps", project.maps);
    writeStrings(writer, "adventures", project.adventures);
    writeStrings(writer, "catalogs", project.catalogs);
    writer.endObject();
    return Result<std::string>::Ok(writer.str());
}

Result<Editor::Project> ProjectSerializer::deserializeProject(const std::string& text) {
    const Result<JsonValue> parsed = JsonValue::parse(text);
    if (!parsed.isOk()) {
        return Result<Editor::Project>::Error("proyecto JSON inválido: " + parsed.errorMessage());
    }
    const JsonValue& root = parsed.value();
    // C-11: rechazar tambien formatos mezclados, sin perder campos en silencio.
    if (root.isObject()) {
        bool legacy = !root.has("schemaVersion");
        for (const char* key : {"nombre", "descripcion", "epoca", "autor", "prefijo", "entrada",
                                "niveles", "mapas", "aventuras", "catalogos"})
            legacy = legacy || root.has(key);
        if (legacy) return Result<Editor::Project>::Error(
            "formato legacy: ejecuta python3 tools/migrar_proyectos.py --write");
    }
    const Result<bool> schema = checkSchema(root, kProjectSchemaVersion, "proyecto");
    if (!schema.isOk()) {
        return Result<Editor::Project>::Error(schema.errorMessage());
    }

    const std::set<std::string> allowed = {"schemaVersion", "id", "name", "description", "epoch",
        "author", "prefix", "settings", "entry", "levels", "maps", "adventures", "catalogs"};
    for (const auto& field : root.objectValues()) {
        if (!allowed.count(field.first)) return Result<Editor::Project>::Error(
            "proyecto: campo desconocido '" + field.first + "'");
    }
    const Result<std::string> id = requiredString(root, "id", "proyecto");
    const Result<std::string> name = requiredString(root, "name", "proyecto");
    if (!id.isOk()) {
        return Result<Editor::Project>::Error(id.errorMessage());
    }
    if (!name.isOk()) {
        return Result<Editor::Project>::Error(name.errorMessage());
    }
    if (!root["settings"].isObject()) {
        return Result<Editor::Project>::Error("proyecto: falta objeto 'settings'");
    }

    for (const char* key : {"description", "epoch", "author", "prefix", "entry"}) {
        if (!root[key].isString()) return Result<Editor::Project>::Error(
            std::string("proyecto: falta string '") + key + "'");
    }
    if (!root["settings"]["initialMap"].isString())
        return Result<Editor::Project>::Error("project.settings: falta string 'initialMap'");

    Editor::Project project;
    project.id = id.value();
    project.name = name.value();
    project.description = root["description"].asString("");
    project.epoch = root["epoch"].asString("");
    project.author = root["author"].asString("");
    project.prefix = root["prefix"].asString("");
    project.entry = root["entry"].asString("");

    const JsonValue& settings = root["settings"];
    const std::set<std::string> allowedSettings = {"schemaVersion", "mapsDirectory", "levelsDirectory",
        "adventuresDirectory", "catalogsDirectory", "initialMap"};
    for (const auto& field : settings.objectValues()) {
        if (!allowedSettings.count(field.first)) return Result<Editor::Project>::Error(
            "project.settings: campo desconocido '" + field.first + "'");
    }

    const Result<bool> settingsSchema = checkSchema(settings, Editor::ProjectSettings::kCurrentSchemaVersion,
                                                    "project.settings");
    if (!settingsSchema.isOk()) {
        return Result<Editor::Project>::Error(settingsSchema.errorMessage());
    }
    const Result<std::string> mapsDirectory =
        requiredString(settings, "mapsDirectory", "project.settings");
    const Result<std::string> levelsDirectory =
        requiredString(settings, "levelsDirectory", "project.settings");
    const Result<std::string> adventuresDirectory =
        requiredString(settings, "adventuresDirectory", "project.settings");
    const Result<std::string> catalogsDirectory =
        requiredString(settings, "catalogsDirectory", "project.settings");
    if (!mapsDirectory.isOk() || !levelsDirectory.isOk() || !adventuresDirectory.isOk() ||
        !catalogsDirectory.isOk()) {
        const std::string error = !mapsDirectory.isOk()      ? mapsDirectory.errorMessage()
                                  : !levelsDirectory.isOk()  ? levelsDirectory.errorMessage()
                                  : !adventuresDirectory.isOk()
                                      ? adventuresDirectory.errorMessage()
                                      : catalogsDirectory.errorMessage();
        return Result<Editor::Project>::Error(error);
    }
    project.settings.schemaVersion = settings["schemaVersion"].asInt();
    project.settings.mapsDirectory = mapsDirectory.value();
    project.settings.levelsDirectory = levelsDirectory.value();
    project.settings.adventuresDirectory = adventuresDirectory.value();
    project.settings.catalogsDirectory = catalogsDirectory.value();
    project.settings.initialMap = settings["initialMap"].asString("");

    const Result<std::vector<std::string>> levels = readStrings(root, "levels", "proyecto");
    const Result<std::vector<std::string>> maps = readStrings(root, "maps", "proyecto");
    const Result<std::vector<std::string>> adventures = readStrings(root, "adventures", "proyecto");
    const Result<std::vector<std::string>> catalogs = readStrings(root, "catalogs", "proyecto");
    if (!levels.isOk() || !maps.isOk() || !adventures.isOk() || !catalogs.isOk()) {
        const std::string error = !levels.isOk()      ? levels.errorMessage()
                                  : !maps.isOk()       ? maps.errorMessage()
                                  : !adventures.isOk() ? adventures.errorMessage()
                                                       : catalogs.errorMessage();
        return Result<Editor::Project>::Error(error);
    }
    project.levels = levels.value();
    project.maps = maps.value();
    project.adventures = adventures.value();
    project.catalogs = catalogs.value();

    const Result<bool> validation = project.validate();
    if (!validation.isOk()) {
        return Result<Editor::Project>::Error("proyecto inválido: " + validation.errorMessage());
    }
    return Result<Editor::Project>::Ok(std::move(project));
}

Result<std::string> ProjectSerializer::serializeMap(const MapDocument& map) {
    const Result<bool> validation = map.validate();
    if (!validation.isOk()) {
        return Result<std::string>::Error("mapa inválido: " + validation.errorMessage());
    }

    JsonWriter writer;
    writer.beginObject();
    writer.member("schemaVersion", kMapSchemaVersion);
    writer.member("id", map.id().value());
    writer.member("name", map.name());
    writer.member("width", map.width());
    writer.member("height", map.height());
    writer.member("tileWidth", map.tileWidth());
    writer.member("tileHeight", map.tileHeight());
    writer.key("collisionGids");
    writer.beginArray();
    for (int gid : map.collisionGids()) {
        writer.value(gid);
    }
    writer.endArray();
    writer.key("layers");
    writer.beginArray();
    for (const MapLayer& layer : map.layers()) {
        writer.beginObject();
        writer.member("id", layer.meta().id.value());
        writer.member("name", layer.meta().name);
        writer.member("visible", layer.meta().visible);
        writer.member("locked", layer.meta().locked);
        writer.member("opacity", layer.meta().opacity);
        writer.member("order", layer.meta().order);

        if (layer.type() == MapLayerType::Tile) {
            writer.member("type", "tile");
            writer.key("tiles");
            writer.beginArray();
            for (int gid : std::get<TileLayerDocument>(layer.data()).gids()) {
                writer.value(gid);
            }
            writer.endArray();
        } else if (layer.type() == MapLayerType::Entity) {
            writer.member("type", "entity");
            writer.key("entities");
            writer.beginArray();
            for (const MapEntityDocument& entity :
                 std::get<EntityLayerDocument>(layer.data()).entities()) {
                writer.beginObject();
                writer.member("id", entity.id.value());
                writer.member("objectId", entity.objectId.value());
                writeGridCoord(writer, "position", entity.position);

                // C-13 — Los campos de autoría. Se escriben SOLO si
                // difieren del valor por defecto: un mapa sin puertas ni
                // botín sigue produciendo el mismo JSON que antes, los
                // diffs siguen siendo legibles y ningún fichero existente
                // cambia por recompilar. Sigue siendo sin pérdida porque
                // el lector restaura exactamente esos mismos defectos.
                if (entity.patrolMin.x != entity.position.x ||
                    entity.patrolMin.y != entity.position.y ||
                    entity.patrolMax.x != entity.position.x ||
                    entity.patrolMax.y != entity.position.y) {
                    writeGridCoord(writer, "patrolMin", entity.patrolMin);
                    writeGridCoord(writer, "patrolMax", entity.patrolMax);
                }
                if (!entity.targetLevel.empty()) {
                    writer.member("targetLevel", entity.targetLevel);
                }
                if (entity.hasTargetPosition) {
                    writeGridCoord(writer, "targetPosition", entity.targetPosition);
                }
                if (!entity.loot.empty()) {
                    writer.key("loot");
                    writer.beginArray();
                    for (const std::string& lootId : entity.loot) {
                        writer.value(lootId);
                    }
                    writer.endArray();
                }
                if (!entity.lootTable.empty()) {
                    writer.member("lootTable", entity.lootTable);
                }
                if (!entity.eventId.empty()) {
                    writer.member("eventId", entity.eventId);
                }
                if (!entity.displayName.empty()) {
                    writer.member("displayName", entity.displayName);
                }
                if (!entity.variant.empty()) {
                    writer.member("variant", entity.variant);
                }
                if (entity.scale != 1.0f) {
                    writer.member("scale", entity.scale);
                }
                if (!entity.effectOverride.empty()) {
                    writer.member("effectOverride", entity.effectOverride);
                }
                if (!entity.properties.empty()) {
                    // std::map: el orden de las claves es determinista, y
                    // por eso guardar dos veces el mismo documento da el
                    // mismo fichero byte a byte.
                    writer.key("properties");
                    writer.beginObject();
                    for (const auto& property : entity.properties) {
                        writer.member(property.first, property.second);
                    }
                    writer.endObject();
                }
                writer.endObject();
            }
            writer.endArray();
        } else {
            writer.member("type", "region");
            writer.key("regions");
            writer.beginArray();
            for (const MapRegionDocument& region :
                 std::get<RegionLayerDocument>(layer.data()).regions()) {
                writer.beginObject();
                writer.member("id", region.id.value());
                writer.member("name", region.name);
                writer.key("bounds");
                writer.beginObject();
                writer.member("minX", region.bounds.minX);
                writer.member("maxX", region.bounds.maxX);
                writer.member("minY", region.bounds.minY);
                writer.member("maxY", region.bounds.maxY);
                writer.endObject();
                writer.endObject();
            }
            writer.endArray();
        }
        writer.endObject();
    }
    writer.endArray();
    writer.endObject();
    return Result<std::string>::Ok(writer.str());
}

Result<MapDocument> ProjectSerializer::deserializeMap(const std::string& text) {
    const Result<JsonValue> parsed = JsonValue::parse(text);
    if (!parsed.isOk()) {
        return Result<MapDocument>::Error("mapa JSON inválido: " + parsed.errorMessage());
    }
    const JsonValue& root = parsed.value();
    const Result<bool> schema = checkSchema(root, kMapSchemaVersion, "mapa");
    if (!schema.isOk()) {
        return Result<MapDocument>::Error(schema.errorMessage());
    }
    const Result<ids::MapId> id = readId<ids::MapId>(root, "id", "mapa");
    const Result<std::string> name = requiredString(root, "name", "mapa");
    const Result<int> width = requiredInt(root, "width", "mapa");
    const Result<int> height = requiredInt(root, "height", "mapa");
    const Result<int> tileWidth = requiredInt(root, "tileWidth", "mapa");
    const Result<int> tileHeight = requiredInt(root, "tileHeight", "mapa");
    if (!id.isOk() || !name.isOk() || !width.isOk() || !height.isOk() || !tileWidth.isOk() ||
        !tileHeight.isOk()) {
        const std::string error = !id.isOk()          ? id.errorMessage()
                                  : !name.isOk()       ? name.errorMessage()
                                  : !width.isOk()      ? width.errorMessage()
                                  : !height.isOk()     ? height.errorMessage()
                                  : !tileWidth.isOk()  ? tileWidth.errorMessage()
                                                       : tileHeight.errorMessage();
        return Result<MapDocument>::Error(error);
    }
    if (!root["layers"].isArray()) {
        return Result<MapDocument>::Error("mapa: falta array 'layers'");
    }
    if (!root["collisionGids"].isArray()) {
        return Result<MapDocument>::Error("mapa: falta array 'collisionGids'");
    }

    MapDocument map(id.value(), name.value(), width.value(), height.value(), tileWidth.value(),
                    tileHeight.value());
    std::vector<int> collisionGids;
    collisionGids.reserve(root["collisionGids"].size());
    for (std::size_t collisionIndex = 0; collisionIndex < root["collisionGids"].size();
         ++collisionIndex) {
        if (!root["collisionGids"][collisionIndex].isNumber()) {
            return Result<MapDocument>::Error("mapa: collisionGids contiene un valor no numérico");
        }
        collisionGids.push_back(root["collisionGids"][collisionIndex].asInt());
    }
    const Result<bool> collisionResult = map.setCollisionGids(std::move(collisionGids));
    if (!collisionResult.isOk()) {
        return Result<MapDocument>::Error("mapa: " + collisionResult.errorMessage());
    }
    for (std::size_t index = 0; index < root["layers"].size(); ++index) {
        const JsonValue& node = root["layers"][index];
        if (!node.isObject()) {
            return Result<MapDocument>::Error("mapa: layer no es objeto en índice " +
                                              std::to_string(index));
        }
        const Result<ids::LayerId> layerId = readId<ids::LayerId>(node, "id", "capa");
        const Result<std::string> layerName = requiredString(node, "name", "capa");
        const Result<bool> visible = requiredBool(node, "visible", "capa");
        const Result<bool> locked = requiredBool(node, "locked", "capa");
        const Result<int> order = requiredInt(node, "order", "capa");
        if (!node["opacity"].isNumber()) {
            return Result<MapDocument>::Error("capa: falta número 'opacity'");
        }
        if (!layerId.isOk() || !layerName.isOk() || !visible.isOk() || !locked.isOk() ||
            !order.isOk()) {
            const std::string error = !layerId.isOk()      ? layerId.errorMessage()
                                      : !layerName.isOk()   ? layerName.errorMessage()
                                      : !visible.isOk()     ? visible.errorMessage()
                                      : !locked.isOk()      ? locked.errorMessage()
                                                           : order.errorMessage();
            return Result<MapDocument>::Error(error);
        }
        MapLayerMeta meta{layerId.value(), layerName.value(), visible.value(), locked.value(),
                          static_cast<float>(node["opacity"].asNumber()), order.value()};
        const Result<std::string> type = requiredString(node, "type", "capa");
        if (!type.isOk()) {
            return Result<MapDocument>::Error(type.errorMessage());
        }

        Result<bool> added = Result<bool>::Error("capa sin construir");
        if (type.value() == "tile") {
            if (!node["tiles"].isArray()) {
                return Result<MapDocument>::Error("capa tile: falta array 'tiles'");
            }
            std::vector<int> gids;
            gids.reserve(node["tiles"].size());
            for (std::size_t gidIndex = 0; gidIndex < node["tiles"].size(); ++gidIndex) {
                if (!node["tiles"][gidIndex].isNumber()) {
                    return Result<MapDocument>::Error("capa tile: GID no numérico");
                }
                gids.push_back(node["tiles"][gidIndex].asInt());
            }
            added = map.addLayer(
                MapLayer::tiles(std::move(meta), TileLayerDocument(map.width(), map.height(), std::move(gids))));
        } else if (type.value() == "entity") {
            if (!node["entities"].isArray()) {
                return Result<MapDocument>::Error("capa entity: falta array 'entities'");
            }
            std::vector<MapEntityDocument> entities;
            entities.reserve(node["entities"].size());
            for (std::size_t entityIndex = 0; entityIndex < node["entities"].size(); ++entityIndex) {
                const JsonValue& entity = node["entities"][entityIndex];
                const Result<ids::EntityId> entityId = readId<ids::EntityId>(entity, "id", "entidad");
                const Result<ids::AssetId> assetId = readId<ids::AssetId>(entity, "objectId", "entidad");
                const Result<GridCoord> position = readGridCoord(entity["position"], "entidad");
                if (!entityId.isOk() || !assetId.isOk() || !position.isOk()) {
                    return Result<MapDocument>::Error(!entityId.isOk() ? entityId.errorMessage()
                                                      : !assetId.isOk() ? assetId.errorMessage()
                                                                        : position.errorMessage());
                }
                MapEntityDocument document;
                document.id = entityId.value();
                document.objectId = assetId.value();
                document.position = position.value();

                // C-13 — Todos los campos son OPCIONALES al leer: un
                // mapa escrito antes de este paquete no los trae, y
                // tiene que seguir cargando exactamente igual. Ausente
                // no es un error, es el valor por defecto.
                //
                // La patrulla ausente se rellena con la posición, que es
                // como LevelLoader expresa "este objeto no patrulla"
                // desde la Fase 6. Así el documento y el nivel dicen lo
                // mismo con los mismos datos.
                document.patrolMin = position.value();
                document.patrolMax = position.value();
                if (entity["patrolMin"].isObject() && entity["patrolMax"].isObject()) {
                    const Result<GridCoord> patrolMin =
                        readGridCoord(entity["patrolMin"], "entidad.patrolMin");
                    const Result<GridCoord> patrolMax =
                        readGridCoord(entity["patrolMax"], "entidad.patrolMax");
                    if (!patrolMin.isOk() || !patrolMax.isOk()) {
                        return Result<MapDocument>::Error(!patrolMin.isOk()
                                                              ? patrolMin.errorMessage()
                                                              : patrolMax.errorMessage());
                    }
                    document.patrolMin = patrolMin.value();
                    document.patrolMax = patrolMax.value();
                }

                document.targetLevel = entity["targetLevel"].asString();
                if (entity["targetPosition"].isObject()) {
                    const Result<GridCoord> target =
                        readGridCoord(entity["targetPosition"], "entidad.targetPosition");
                    if (!target.isOk()) {
                        return Result<MapDocument>::Error(target.errorMessage());
                    }
                    document.targetPosition = target.value();
                    document.hasTargetPosition = true;
                }

                if (entity["loot"].isArray()) {
                    document.loot.reserve(entity["loot"].size());
                    for (std::size_t lootIndex = 0; lootIndex < entity["loot"].size();
                         ++lootIndex) {
                        if (!entity["loot"][lootIndex].isString()) {
                            return Result<MapDocument>::Error(
                                "entidad: 'loot' contiene un valor no textual");
                        }
                        document.loot.push_back(entity["loot"][lootIndex].asString());
                    }
                }
                document.lootTable = entity["lootTable"].asString();
                document.eventId = entity["eventId"].asString();
                document.displayName = entity["displayName"].asString();
                document.variant = entity["variant"].asString();
                if (entity["scale"].isNumber()) {
                    document.scale = static_cast<float>(entity["scale"].asNumber());
                }
                document.effectOverride = entity["effectOverride"].asString();
                if (entity["properties"].isObject()) {
                    for (const auto& property : entity["properties"].objectValues()) {
                        document.properties[property.first] = property.second.asString();
                    }
                }

                entities.push_back(std::move(document));
            }
            added = map.addLayer(MapLayer::entities(std::move(meta), EntityLayerDocument(std::move(entities))));
        } else if (type.value() == "region") {
            if (!node["regions"].isArray()) {
                return Result<MapDocument>::Error("capa region: falta array 'regions'");
            }
            std::vector<MapRegionDocument> regions;
            regions.reserve(node["regions"].size());
            for (std::size_t regionIndex = 0; regionIndex < node["regions"].size(); ++regionIndex) {
                const JsonValue& region = node["regions"][regionIndex];
                const Result<ids::RegionId> regionId = readId<ids::RegionId>(region, "id", "región");
                const Result<std::string> regionName = requiredString(region, "name", "región");
                const JsonValue& bounds = region["bounds"];
                const Result<int> minX = requiredInt(bounds, "minX", "región.bounds");
                const Result<int> maxX = requiredInt(bounds, "maxX", "región.bounds");
                const Result<int> minY = requiredInt(bounds, "minY", "región.bounds");
                const Result<int> maxY = requiredInt(bounds, "maxY", "región.bounds");
                if (!regionId.isOk() || !regionName.isOk() || !minX.isOk() || !maxX.isOk() ||
                    !minY.isOk() || !maxY.isOk()) {
                    return Result<MapDocument>::Error(
                        !regionId.isOk() ? regionId.errorMessage()
                        : !regionName.isOk() ? regionName.errorMessage()
                        : !minX.isOk() ? minX.errorMessage()
                        : !maxX.isOk() ? maxX.errorMessage()
                        : !minY.isOk() ? minY.errorMessage()
                                         : maxY.errorMessage());
                }
                regions.push_back({regionId.value(), regionName.value(),
                                   {minX.value(), maxX.value(), minY.value(), maxY.value()}});
            }
            added = map.addLayer(MapLayer::regions(std::move(meta), RegionLayerDocument(std::move(regions))));
        } else {
            return Result<MapDocument>::Error("capa: tipo desconocido '" + type.value() + "'");
        }
        if (!added.isOk()) {
            return Result<MapDocument>::Error("mapa: capa inválida: " + added.errorMessage());
        }
    }
    const Result<bool> validation = map.validate();
    if (!validation.isOk()) {
        return Result<MapDocument>::Error("mapa inválido: " + validation.errorMessage());
    }
    return Result<MapDocument>::Ok(std::move(map));
}

Result<bool> ProjectSerializer::saveProjectAtomic(const std::string& path,
                                                  const Editor::Project& project) {
    const Result<std::string> text = serializeProject(project);
    return text.isOk() ? atomicWrite(path, text.value()) : Result<bool>::Error(text.errorMessage());
}

Result<bool> ProjectSerializer::saveMapAtomic(const std::string& path, const MapDocument& map) {
    const Result<std::string> text = serializeMap(map);
    return text.isOk() ? atomicWrite(path, text.value()) : Result<bool>::Error(text.errorMessage());
}

Result<Editor::Project> ProjectSerializer::loadProjectFile(const std::string& path) {
    const Result<std::string> text = readFile(path);
    return text.isOk() ? deserializeProject(text.value())
                       : Result<Editor::Project>::Error(text.errorMessage());
}

Result<MapDocument> ProjectSerializer::loadMapFile(const std::string& path) {
    const Result<std::string> text = readFile(path);
    return text.isOk() ? deserializeMap(text.value())
                       : Result<MapDocument>::Error(text.errorMessage());
}
