// ProjectSerializer::serializeMap — el documento de mapa, a JSON.
//
// Salio de ProjectSerializer.cpp el 21/09/2026 (ley de los 600 lineas).
// Escribir un mapa y leerlo son dos trabajos distintos que cambian por
// motivos distintos: quien anade un campo al formato toca uno de los dos,
// no los mil ochenta de antes.

#include "Project/ProjectSerializer.h"

#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "Project/SerializerHelpers.h"

using namespace ProjectIO;  // NOLINT(build/namespaces)


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

    // C-23 — Post-procesat per nivell
    writer.member("fogEnabled", map.fogEnabled());
    writer.member("fogRevealRadius", map.fogRevealRadius());
    writer.member("lightingEnabled", map.lightingEnabled());
    writer.member("ambientR", map.ambientR());
    writer.member("ambientG", map.ambientG());
    writer.member("ambientB", map.ambientB());

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
        } else if (layer.type() == MapLayerType::Region) {
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
        } else {
            // E-01 — Capa de eventos. Antes esta rama era el `else` de
            // region; nombrar los cuatro tipos evita que un quinto se
            // cuele escribiendose como evento.
            writer.member("type", "event");
            writer.key("events");
            writer.beginArray();
            for (const MapEventDocument& event :
                 std::get<EventLayerDocument>(layer.data()).events()) {
                writer.beginObject();
                writer.member("id", event.id.value());
                writer.member("name", event.name);
                writeGridCoord(writer, "position", event.position);
                writer.key("pages");
                writer.beginArray();
                for (const MapEventPage& page : event.pages) {
                    writer.beginObject();
                    writer.member("id", page.id.value());
                    writer.member("name", page.name);
                    writer.member("trigger", eventTriggerToText(page.trigger));
                    if (isRegionTrigger(page.trigger)) {
                        // U3-02.E06a: solo si hay region; los JSON antiguos no cambian.
                        writer.key("region");
                        writer.beginObject();
                        writer.member("layer", page.regionLayerId.value());
                        writer.member("id", page.regionId.value());
                        writer.endObject();
                    }
                    writer.member("requiresAllConditions", page.requiresAllConditions);
                    writer.member("appearanceObjectId", page.appearanceObjectId);
                    writer.key("conditions");
                    writer.beginArray();
                    for (const MapEventCondition& condition : page.conditions) {
                        writer.beginObject();
                        writer.member("kind", eventConditionKindToText(condition.kind));
                        writer.member("reference", condition.reference);
                        writer.member("comparison", condition.comparison);
                        writer.member("value", condition.value);
                        writer.member("negated", condition.negated);
                        writer.endObject();
                    }
                    writer.endArray();
                    writer.key("commands");
                    writeEventCommands(writer, page.commands);
                    writer.endObject();
                }
                writer.endArray();
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
