// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-14  (fase C, 6 j) - L'editor treballa sobre MapDocument
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Editor/EditorState.h"

#include "TestHarness.h"
#include "Core/Json/JsonValue.h"
#include "Level/LevelDocumentBridge.h"
#include "Project/ProjectSerializer.h"

#include <limits>

namespace {

MOTOR_TEST_CASE(editing_operations_are_undoable) {
    EditorState editor(4, 3);

    editor.paintTile(1, 1, 2);
    editor.placeObject(2, 1, "slime");
    editor.setPlayerStart(GridCoord{3, 2});

    MOTOR_CHECK(editor.tileAt(1, 1) == 2);
    MOTOR_CHECK(editor.objectAt(2, 1) != nullptr);
    MOTOR_CHECK(editor.playerStart().x == 3);
    MOTOR_CHECK(editor.playerStart().y == 2);

    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(editor.playerStart().x == 0);
    MOTOR_CHECK(editor.playerStart().y == 0);
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(editor.objectAt(2, 1) == nullptr);
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(editor.tileAt(1, 1) == 0);
    MOTOR_CHECK(!editor.canUndo());
    MOTOR_CHECK(editor.canRedo());

    MOTOR_CHECK(editor.redo());
    MOTOR_CHECK(editor.tileAt(1, 1) == 2);
}

MOTOR_TEST_CASE(palette_filter_matches_editorial_labels) {
    EditorState editor(8, 8);
    editor.setObjectPalette({"npc_tabernero", "prop_taberna_mesa", "enemigo_lobo"});
    editor.setObjectPaletteLabels({{"npc_tabernero", "Tabernero humano"},
                                   {"prop_taberna_mesa", "Mesa de taberna"},
                                   {"enemigo_lobo", "Lobo demacrado"}});

    editor.setPaletteFilter("humano");

    MOTOR_CHECK(editor.objectPalette().size() == 1);
    MOTOR_CHECK(editor.objectPaletteTotal() == 3);
    MOTOR_CHECK(editor.selectedObjectId() == "npc_tabernero");

    editor.setTilePalette({1, 2, 3});
    editor.setTilePaletteLabels(
        {{1, "Pradera bosque"}, {2, "Arena desertica"}, {3, "Mosaico templo"}});
    editor.setPaletteFilter("arena");

    MOTOR_CHECK(editor.tilePalette().size() == 1);
    MOTOR_CHECK(editor.selectedTileGid() == 2);
}

MOTOR_TEST_CASE(layers_are_isolated_and_can_be_undone) {
    EditorState editor(4, 4);
    editor.paintTile(0, 0, 1);

    const int decorado = editor.addLayer("decorado");
    editor.paintTile(0, 0, 2);

    MOTOR_CHECK(decorado == 1);
    MOTOR_CHECK(editor.layerCount() == 2);
    MOTOR_CHECK(editor.activeLayer() == 1);
    MOTOR_CHECK(editor.tileAt(0, 0, 0) == 1);
    MOTOR_CHECK(editor.tileAt(1, 0, 0) == 2);

    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(editor.tileAt(1, 0, 0) == 0);
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(editor.layerCount() == 1);
    MOTOR_CHECK(editor.activeLayer() == 0);
    MOTOR_CHECK(!editor.removeLayer(0));
}

MapLayerMeta meta(const char* id, const char* name, int order) {
    MapLayerMeta result;
    result.id = ids::LayerId::parse(id).value();
    result.name = name;
    result.order = order;
    return result;
}

MapDocument authoredMap() {
    MapDocument document(ids::MapId::parse("plaza").value(), "Plaça central", 5, 4, 96, 48);
    MOTOR_CHECK(document.setCollisionGids({2, 4}).isOk());
    MOTOR_CHECK(document.addLayer(MapLayer::tiles(meta("ground", "Suelo", 10),
        TileLayerDocument(5, 4, std::vector<int>(20, 1)))).isOk());
    MapLayerMeta overlay = meta("roof", "Tejado \"azul\" & torres", 30);
    overlay.visible = false;
    overlay.opacity = 0.5f;
    MOTOR_CHECK(document.addLayer(MapLayer::tiles(overlay,
        TileLayerDocument(5, 4, std::vector<int>(20, 2)))).isOk());
    ObjectSpawn spawn;
    spawn.objectId = "puerta";
    spawn.position = {1, 1};
    spawn.patrolMin = {0, 1};
    spawn.patrolMax = {3, 2};
    spawn.targetLevel = "assets/levels/interior.json";
    spawn.targetPosition = {20, 15};
    spawn.hasTargetPosition = true;
    spawn.loot = {"llave", "pocion"};
    spawn.lootTable = "cofre_raro";
    spawn.eventId = "abrir_puerta";
    spawn.displayName = "Puerta del jardín";
    spawn.variant = "noche";
    spawn.scale = 1.75f;
    spawn.effectOverride = "azul";
    spawn.properties = {{"texto", "Uno\nDos\t\"Tres\""}};
    const auto entity = level_document::ToEntityDocument(spawn, ids::EntityId::parse("puerta_estable").value());
    MOTOR_CHECK(entity.isOk());
    MOTOR_CHECK(document.addLayer(MapLayer::entities(meta("actors", "Personajes", 20),
        EntityLayerDocument({entity.value()}))).isOk());
    MapRegionDocument region;
    region.id = ids::RegionId::parse("entrada").value();
    region.name = "Entrada";
    region.bounds = {0, 0, 2, 2};
    MOTOR_CHECK(document.addLayer(MapLayer::regions(meta("regions", "Regiones", 40),
        RegionLayerDocument({region}))).isOk());
    return document;
}

std::string serialized(const MapDocument& document) {
    auto result = ProjectSerializer::serializeMap(document);
    MOTOR_CHECK(result.isOk());
    return result.value();
}

const MapEntityDocument& firstEntity(const EditorState& editor) {
    return std::get<EntityLayerDocument>(editor.document().findLayer(
        ids::LayerId::parse("actors").value())->data()).entities().front();
}

MOTOR_TEST_CASE(document_is_authoritative_and_preserves_all_authored_layers) {
    EditorState editor(1, 1);
    const MapDocument original = authoredMap();
    MOTOR_CHECK(editor.setDocument(original).isOk());
    MOTOR_CHECK(editor.width() == 5);
    MOTOR_CHECK(editor.height() == 4);
    MOTOR_CHECK(editor.layerCount() == 2);
    MOTOR_CHECK(editor.document().layers().size() == 4);
    MOTOR_CHECK(serialized(editor.document()) == serialized(original));
    MOTOR_CHECK(!editor.layerVisible(1));
    MOTOR_CHECK(editor.layerOpacity(1) == 0.5f);
    MOTOR_CHECK(editor.objects().size() == 1);
    MOTOR_CHECK(editor.objectAt(1, 1)->eventId == "abrir_puerta");

    editor.paintTile(0, 0, 4);
    const auto* ground = editor.document().findLayer(ids::LayerId::parse("ground").value());
    MOTOR_CHECK(std::get<TileLayerDocument>(ground->data()).gidAt(0, 0) == 4);
    editor.setObjectDisplayName(1, 1, "Nueva puerta");
    MOTOR_CHECK(firstEntity(editor).displayName == "Nueva puerta");
    MOTOR_CHECK(firstEntity(editor).id.value() == "puerta_estable");
    MOTOR_CHECK(editor.objectAt(1, 1)->displayName == "Nueva puerta");
    MOTOR_CHECK(firstEntity(editor).targetPosition.x == 20);
    MOTOR_CHECK(firstEntity(editor).loot.size() == 2);
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(editor.objectAt(1, 1)->displayName == "Puerta del jardín");
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(serialized(editor.document()) == serialized(original));
    MOTOR_CHECK(editor.redo());
    MOTOR_CHECK(editor.tileAt(0, 0) == 4);
}

MOTOR_TEST_CASE(saving_reopening_keeps_names_visibility_order_and_stable_ids) {
    EditorState editor(1, 1);
    MOTOR_CHECK(editor.setDocument(authoredMap()).isOk());
    editor.setActiveLayer(1);
    MapLayerMeta changed = editor.document().findLayer(ids::LayerId::parse("roof").value())->meta();
    changed.order = 5;
    changed.locked = true;
    MOTOR_CHECK(editor.setLayerMeta(1, changed).isOk());
    MOTOR_CHECK(editor.activeLayer() == 0);
    MOTOR_CHECK(editor.layerName(0) == changed.name);
    MOTOR_CHECK(editor.layerLocked(0));
    const std::string saved = serialized(editor.document());
    auto loaded = ProjectSerializer::deserializeMap(saved);
    MOTOR_CHECK(loaded.isOk());
    EditorState reopened(2, 2);
    MOTOR_CHECK(reopened.setDocument(std::move(loaded.value())).isOk());
    MOTOR_CHECK(serialized(reopened.document()) == saved);
    MOTOR_CHECK(!reopened.layerVisible(0));
    MOTOR_CHECK(reopened.layerOpacity(0) == 0.5f);
    MOTOR_CHECK(reopened.layerLocked(0));
    MOTOR_CHECK(firstEntity(reopened).id.value() == "puerta_estable");
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(editor.activeLayer() == 1);
    MOTOR_CHECK(editor.document().findLayer(ids::LayerId::parse("roof").value())->meta().order == 30);
    MOTOR_CHECK(editor.redo());
    MOTOR_CHECK(editor.activeLayer() == 0);
    MOTOR_CHECK(serialized(editor.document()) == saved);
}

MOTOR_TEST_CASE(rejected_document_or_spawn_preserves_document_view_and_history) {
    EditorState editor(1, 1);
    MOTOR_CHECK(editor.setDocument(authoredMap()).isOk());
    editor.paintTile(0, 0, 7);
    const std::string before = serialized(editor.document());
    MOTOR_CHECK(!editor.setDocument(MapDocument{}).isOk());
    MOTOR_CHECK(serialized(editor.document()) == before);
    ObjectSpawn invalid = *editor.objectAt(1, 1);
    invalid.objectId = "Invalid ID";
    editor.placeSpawn(invalid);
    invalid.objectId = "puerta";
    invalid.patrolMax = {99, 99};
    editor.placeSpawn(invalid);
    MOTOR_CHECK(serialized(editor.document()) == before);
    MOTOR_CHECK(editor.objectAt(1, 1)->objectId == "puerta");
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(!editor.canUndo());
    MOTOR_CHECK(editor.tileAt(0, 0) == 1);
}

MOTOR_TEST_CASE(clipboard_copies_entities_with_new_ids_and_keeps_document_metadata) {
    EditorState editor(1, 1);
    MOTOR_CHECK(editor.setDocument(authoredMap()).isOk());
    editor.setSelection(1, 1, 2, 2);
    editor.copySelection();
    editor.pasteAt(3, 2);
    MOTOR_CHECK(editor.document().validate().isOk());
    MOTOR_CHECK(editor.objects().size() == 2);
    MOTOR_CHECK(firstEntity(editor).id.value() == "puerta_estable");
    const ObjectSpawn* copy = editor.objectAt(3, 2);
    MOTOR_CHECK(copy != nullptr);
    MOTOR_CHECK(copy->patrolMin.x == 2);
    MOTOR_CHECK(copy->patrolMax.x == 4);
    MOTOR_CHECK(copy->targetPosition.x == 20);
    MOTOR_CHECK(copy->lootTable == "cofre_raro");
    const auto& entities = std::get<EntityLayerDocument>(editor.document().findLayer(
        ids::LayerId::parse("actors").value())->data()).entities();
    MOTOR_CHECK(entities[0].id != entities[1].id);
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(editor.objectAt(3, 2) == nullptr);
    MOTOR_CHECK(editor.objects().size() == 1);
    MOTOR_CHECK(editor.redo());
    MOTOR_CHECK(editor.objectAt(3, 2) != nullptr);
    MOTOR_CHECK(!editor.layerVisible(1));
}

MOTOR_TEST_CASE(fill_and_erase_selection_are_single_document_transactions) {
    EditorState editor(1, 1);
    MOTOR_CHECK(editor.setDocument(authoredMap()).isOk());
    const std::string before = serialized(editor.document());
    editor.fillTiles(0, 0, 4);
    MOTOR_CHECK(editor.tileAt(0, 4, 3) == 4);
    MOTOR_CHECK(editor.tileAt(1, 4, 3) == 2);
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(serialized(editor.document()) == before);
    editor.setSelection(0, 0, 4, 3);
    editor.eraseSelection();
    MOTOR_CHECK(editor.objects().empty());
    MOTOR_CHECK(editor.tileAt(0, 4, 3) == 0);
    MOTOR_CHECK(editor.tileAt(1, 4, 3) == 0);
    MOTOR_CHECK(editor.document().layers().size() == 4);
    MOTOR_CHECK(editor.undo());
    MOTOR_CHECK(serialized(editor.document()) == before);
}

MOTOR_TEST_CASE(locked_layers_and_no_op_inspector_edits_do_not_create_history) {
    EditorState editor(1, 1);
    MapDocument document = authoredMap();
    auto locked = document.findLayer(ids::LayerId::parse("ground").value())->meta();
    locked.locked = true;
    MOTOR_CHECK(document.setLayerMeta(locked.id, locked).isOk());
    locked = document.findLayer(ids::LayerId::parse("actors").value())->meta();
    locked.locked = true;
    MOTOR_CHECK(document.setLayerMeta(locked.id, locked).isOk());
    MOTOR_CHECK(editor.setDocument(document).isOk());
    const std::string before = serialized(editor.document());
    editor.paintTile(0, 0, 5);
    editor.fillTiles(0, 0, 5);
    editor.removeObjectAt(1, 1);
    editor.placeObject(1, 1, "cofre");
    editor.setObjectVariant(1, 1, "dia");
    MOTOR_CHECK(!editor.removeLayer(0));
    MOTOR_CHECK(!editor.canUndo());
    MOTOR_CHECK(serialized(editor.document()) == before);
    MOTOR_CHECK(editor.setDocument(authoredMap()).isOk());
    editor.setObjectVariant(1, 1, "noche");
    editor.setObjectTargetPosition(1, 1, {20, 15});
    editor.setObjectProperty(1, 1, "texto", "Uno\nDos\t\"Tres\"");
    editor.addObjectLoot(1, 1, "llave");
    editor.setObjectScale(1, 1, std::numeric_limits<float>::quiet_NaN());
    MOTOR_CHECK(!editor.canUndo());
}

MOTOR_TEST_CASE(level_export_includes_optional_document_reference_and_valid_strings) {
    EditorState editor(1, 1);
    MOTOR_CHECK(editor.setDocument(authoredMap()).isOk());
    auto exported = JsonValue::parse(editor.exportLevelJson("Plaça\ncentral", "maps/plaza.tmx",
                                                            "maps/plaza.map.json"));
    MOTOR_CHECK(exported.isOk());
    MOTOR_CHECK(exported.value()["name"].asString() == "Plaça\ncentral");
    MOTOR_CHECK(exported.value()["mapDocument"].asString() == "maps/plaza.map.json");
    MOTOR_CHECK(exported.value()["objects"][0]["properties"]["texto"].asString() == "Uno\nDos\t\"Tres\"");
    auto legacy = JsonValue::parse(editor.exportLevelJson("Plaça", "maps/plaza.tmx"));
    MOTOR_CHECK(legacy.isOk());
    MOTOR_CHECK(!legacy.value().has("mapDocument"));
    const std::string tmx = editor.exportTmx(TmxTilesetSettings{});
    MOTOR_CHECK(tmx.find("visible=\"0\" opacity=\"0.5\"") != std::string::npos);
    MOTOR_CHECK(tmx.find("Tejado &quot;azul&quot; &amp; torres") != std::string::npos);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
