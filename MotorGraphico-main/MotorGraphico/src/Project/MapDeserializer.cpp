// ProjectSerializer::deserializeMap — el JSON, de vuelta a documento.
//
// Salio de ProjectSerializer.cpp el 21/09/2026 (ley de los 600 lineas).
// Es la mitad que valida: aqui es donde un campo que falta tiene que dar
// un error con nombre y no un mapa a medias.

#include "Project/ProjectSerializer.h"

#include <cmath>     // std::isfinite, al validar coordenadas del mapa
#include <limits>
#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "Project/SerializerHelpers.h"

using namespace ProjectIO;  // NOLINT(build/namespaces)


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

    // C-23 — Post-procesat per nivell (opcionales: mapas antiguos no los traen)
    if (root.has("fogEnabled") && root["fogEnabled"].isBool()) {
        map.setFogEnabled(root["fogEnabled"].asBool());
    }
    if (root.has("fogRevealRadius") && root["fogRevealRadius"].isNumber()) {
        map.setFogRevealRadius(root["fogRevealRadius"].asInt());
    }
    if (root.has("lightingEnabled") && root["lightingEnabled"].isBool()) {
        map.setLightingEnabled(root["lightingEnabled"].asBool());
    }
    if (root.has("ambientR") && root["ambientR"].isNumber()) {
        map.setAmbientR(static_cast<float>(root["ambientR"].asNumber()));
    }
    if (root.has("ambientG") && root["ambientG"].isNumber()) {
        map.setAmbientG(static_cast<float>(root["ambientG"].asNumber()));
    }
    if (root.has("ambientB") && root["ambientB"].isNumber()) {
        map.setAmbientB(static_cast<float>(root["ambientB"].asNumber()));
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
                // Un campo ausente conserva compatibilidad; uno presente con
                // tipo incorrecto no puede desaparecer al volver a guardar.
                const auto fieldError = [&](const std::string& field) {
                    return Result<MapDocument>::Error("entidad '" + document.id.value() +
                                                       "': campo inválido '" + field + "'");
                };
                for (const char* key : {"targetLevel", "lootTable", "eventId", "displayName",
                                        "variant", "effectOverride"}) {
                    if (entity.has(key) && !entity[key].isString()) return fieldError(key);
                }
                for (const char* key : {"patrolMin", "patrolMax", "targetPosition", "properties"}) {
                    if (entity.has(key) && !entity[key].isObject()) return fieldError(key);
                }
                if (entity.has("patrolMin") != entity.has("patrolMax")) {
                    return fieldError("patrolMin/patrolMax: se requieren ambos extremos");
                }
                if (entity.has("loot") && !entity["loot"].isArray()) return fieldError("loot");
                if (entity.has("scale") &&
                    (!entity["scale"].isNumber() || !std::isfinite(entity["scale"].asNumber()) ||
                     entity["scale"].asNumber() <= 0 ||
                     entity["scale"].asNumber() > std::numeric_limits<float>::max() ||
                     static_cast<float>(entity["scale"].asNumber()) <= 0)) {
                    return fieldError("scale");
                }
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
                        if (!property.second.isString()) return fieldError("properties." + property.first);
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
        } else if (type.value() == "event") {
            // E-01. Todo-o-nada como el resto: si una sola pagina no se
            // entiende, no se devuelve un mapa a medias con el evento
            // mutilado. Un evento que pierde una pagina en silencio es una
            // puerta que deja de pedir la llave.
            if (!node["events"].isArray()) {
                return Result<MapDocument>::Error("capa event: falta array 'events'");
            }
            std::vector<MapEventDocument> events;
            events.reserve(node["events"].size());
            for (std::size_t eventIndex = 0; eventIndex < node["events"].size(); ++eventIndex) {
                const JsonValue& eventNode = node["events"][eventIndex];
                const Result<ids::EventId> eventId =
                    readId<ids::EventId>(eventNode, "id", "evento");
                if (!eventId.isOk()) {
                    return Result<MapDocument>::Error(eventId.errorMessage());
                }
                const Result<GridCoord> position =
                    readGridCoord(eventNode["position"], "evento.position");
                if (!position.isOk()) {
                    return Result<MapDocument>::Error(position.errorMessage());
                }
                MapEventDocument document;
                document.id = eventId.value();
                document.name = eventNode["name"].asString();
                document.position = position.value();

                if (eventNode.has("pages")) {
                    if (!eventNode["pages"].isArray()) {
                        return Result<MapDocument>::Error("evento '" + eventId.value().value() +
                                                          "': 'pages' debe ser un array");
                    }
                    const JsonValue& pages = eventNode["pages"];
                    for (std::size_t pageIndex = 0; pageIndex < pages.size(); ++pageIndex) {
                        const JsonValue& pageNode = pages[pageIndex];
                        const Result<ids::EventId> pageId =
                            readId<ids::EventId>(pageNode, "id", "pagina de evento");
                        if (!pageId.isOk()) {
                            return Result<MapDocument>::Error(pageId.errorMessage());
                        }
                        const Result<EventTrigger> trigger = eventTriggerFromText(
                            pageNode["trigger"].asString("interact"), "pagina de evento");
                        if (!trigger.isOk()) {
                            return Result<MapDocument>::Error(trigger.errorMessage());
                        }
                        MapEventPage page;
                        page.id = pageId.value();
                        page.name = pageNode["name"].asString();
                        page.trigger = trigger.value();
                        page.requiresAllConditions =
                            pageNode.has("requiresAllConditions")
                                ? pageNode["requiresAllConditions"].asBool(true)
                                : true;
                        page.appearanceObjectId = pageNode["appearanceObjectId"].asString();
                        // U3-02.E06a: clave opcional; ausente = sin region.
                        if (pageNode.has("region")) {
                            const JsonValue& regionNode = pageNode["region"];
                            const Result<ids::LayerId> regionLayer =
                                readId<ids::LayerId>(regionNode, "layer", "region de pagina");
                            const Result<ids::RegionId> regionId =
                                readId<ids::RegionId>(regionNode, "id", "region de pagina");
                            if (!regionLayer.isOk() || !regionId.isOk()) {
                                return Result<MapDocument>::Error(
                                    !regionLayer.isOk() ? regionLayer.errorMessage()
                                                        : regionId.errorMessage());
                            }
                            page.regionLayerId = regionLayer.value();
                            page.regionId = regionId.value();
                        }

                        if (pageNode.has("conditions")) {
                            const JsonValue& conditions = pageNode["conditions"];
                            if (!conditions.isArray()) {
                                return Result<MapDocument>::Error(
                                    "pagina de evento: 'conditions' debe ser un array");
                            }
                            for (std::size_t c = 0; c < conditions.size(); ++c) {
                                const JsonValue& conditionNode = conditions[c];
                                const Result<EventConditionKind> kind = eventConditionKindFromText(
                                    conditionNode["kind"].asString("flag"), "condicion");
                                if (!kind.isOk()) {
                                    return Result<MapDocument>::Error(kind.errorMessage());
                                }
                                MapEventCondition condition;
                                condition.kind = kind.value();
                                condition.reference = conditionNode["reference"].asString();
                                condition.comparison = conditionNode["comparison"].asString("==");
                                condition.value = conditionNode["value"].asString();
                                condition.negated = conditionNode["negated"].asBool(false);
                                page.conditions.push_back(std::move(condition));
                            }
                        }

                        if (pageNode.has("commands")) {
                            const Result<std::vector<MapEventCommand>> commands =
                                readEventCommands(pageNode["commands"], "pagina de evento", 1);
                            if (!commands.isOk()) {
                                return Result<MapDocument>::Error(commands.errorMessage());
                            }
                            page.commands = commands.value();
                        }
                        document.pages.push_back(std::move(page));
                    }
                }
                events.push_back(std::move(document));
            }
            added = map.addLayer(
                MapLayer::events(std::move(meta), EventLayerDocument(std::move(events))));
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
