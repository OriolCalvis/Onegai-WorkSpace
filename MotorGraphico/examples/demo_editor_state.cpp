// Fase 12 (motor_grafico_gantt_rpg.puml): nucleo del editor de niveles.
// GL-free (EditorState es modelo puro, ver su header): se compila y
// ejecuta de verdad, sin ventana.
//
// Verifica:
//  - Edicion: pintar/borrar tiles (fuera de rango = no-op), colocar/
//    reemplazar/quitar objetos (una celda = un objeto), applyAt() segun
//    la herramienta activa, ciclado de paletas con wraparound.
//  - Round-trip TMX REAL: exportTmx() se escribe a disco y lo carga el
//    TileMap::loadFromFile() DE VERDAD (el mismo parser tinyxml2 del
//    motor, no un mock): mismas dimensiones, mismos GIDs celda a celda,
//    misma colision.
//  - Round-trip JSON REAL: exportLevelJson() -> LevelLoader::
//    loadFromString(): mismo nombre/mapa/playerStart/objetos.
#include "Editor/EditorState.h"
#include "Level/LevelLoader.h"
#include "Render/TileMap.h"

#include "Check.h"

#include <cstdio>
#include <fstream>
#include <iostream>

namespace {

void testEditingOps() {
    EditorState editor(4, 3);
    require(editor.width() == 4 && editor.height() == 3);
    require(editor.tileAt(0, 0) == 0);  // mapa en blanco

    // Pintar/borrar, con clamps.
    editor.paintTile(1, 1, 2);
    require(editor.tileAt(1, 1) == 2);
    editor.paintTile(99, 99, 2);  // fuera de rango: no-op, sin crash
    editor.paintTile(-1, 0, 2);
    require(editor.tileAt(99, 99) == 0);  // consulta fuera de rango: 0
    editor.eraseTile(1, 1);
    require(editor.tileAt(1, 1) == 0);

    // Objetos: una celda = un objeto (reemplazo), quitar.
    editor.placeObject(2, 1, "arbusto");
    require(editor.objects().size() == 1);
    require(editor.objectAt(2, 1) != nullptr && editor.objectAt(2, 1)->objectId == "arbusto");
    editor.placeObject(2, 1, "pocion");  // reemplaza, no duplica
    require(editor.objects().size() == 1);
    require(editor.objectAt(2, 1)->objectId == "pocion");
    // Sin patrulla: patrolMin == patrolMax == position (MVP, ver header).
    require(editor.objectAt(2, 1)->patrolMin.x == 2 && editor.objectAt(2, 1)->patrolMax.y == 1);
    editor.removeObjectAt(2, 1);
    require(editor.objects().empty());
    require(editor.objectAt(2, 1) == nullptr);
    editor.placeObject(0, 0, "");  // id vacio: no-op
    require(editor.objects().empty());

    std::cout << "[EDITOR] pintar/borrar/colocar/quitar (con clamps y reemplazo) correcto.\n";
}

void testToolsAndPalettes() {
    EditorState editor(4, 3);
    editor.setTilePalette({1, 2});
    editor.setObjectPalette({"arbusto", "slime", "pocion"});

    // PaintTile con la paleta: pinta el gid seleccionado.
    require(editor.selectedTileGid() == 1);
    editor.applyAt(0, 0);
    require(editor.tileAt(0, 0) == 1);
    editor.nextPaletteEntry();  // paleta de tiles (herramienta activa)
    require(editor.selectedTileGid() == 2);
    editor.nextPaletteEntry();  // wraparound
    require(editor.selectedTileGid() == 1);
    editor.prevPaletteEntry();  // wraparound hacia atras
    require(editor.selectedTileGid() == 2);

    // PlaceObject cicla SU paleta, independiente de la de tiles.
    editor.setTool(EditorTool::PlaceObject);
    require(editor.selectedObjectId() == "arbusto");
    editor.nextPaletteEntry();
    require(editor.selectedObjectId() == "slime");
    editor.applyAt(3, 2);
    require(editor.objectAt(3, 2) != nullptr && editor.objectAt(3, 2)->objectId == "slime");

    // Erase*/Remove* via applyAt.
    editor.setTool(EditorTool::EraseTile);
    editor.applyAt(0, 0);
    require(editor.tileAt(0, 0) == 0);
    editor.setTool(EditorTool::RemoveObject);
    editor.applyAt(3, 2);
    require(editor.objectAt(3, 2) == nullptr);

    // Paletas vacias: seleccion "nada", applyAt no-op sin crash.
    EditorState empty(2, 2);
    require(empty.selectedTileGid() == 0);
    require(empty.selectedObjectId().empty());
    empty.nextPaletteEntry();
    empty.applyAt(0, 0);
    require(empty.tileAt(0, 0) == 0);
    empty.setTool(EditorTool::PlaceObject);
    empty.applyAt(0, 0);
    require(empty.objects().empty());

    std::cout << "[EDITOR] herramientas + paletas (ciclado, wraparound, vacias) correcto.\n";
}

void testTmxRoundTrip() {
    // Reproduce el contenido de test_map.tmx (4x3, borde de gid 1,
    // centro de gid 2 con colision) desde el editor y comprueba que el
    // PARSER REAL del motor lo carga identico.
    EditorState editor(4, 3);
    editor.setTilePalette({1, 2});
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 4; ++x) {
            bool center = (y == 1 && (x == 1 || x == 2));
            editor.paintTile(x, y, center ? 2 : 1);
        }
    }

    TmxTilesetSettings settings;  // defaults = el tileset de test_map.tmx
    settings.collisionGids = {2};
    std::string tmx = editor.exportTmx(settings);

    const char* path = "editor_roundtrip_test.tmx";
    {
        std::ofstream out(path, std::ios::binary);
        out << tmx;
    }
    TileMap map;
    auto loaded = map.loadFromFile(path);
    require(loaded.isOk());
    require(map.getWidth() == 4 && map.getHeight() == 3);
    require(map.getLayerCount() == 1);
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 4; ++x) {
            require(map.getTile(0, x, y).getTilesetID() == editor.tileAt(x, y));
        }
    }
    // Colision: los gid 2 la llevan (via <properties> del tileset), los
    // gid 1 no -- igual que en test_map.tmx.
    require(map.getTile(0, 1, 1).hasCollision());
    require(!map.getTile(0, 0, 0).hasCollision());
    std::remove(path);

    std::cout << "[EDITOR] round-trip TMX real (exportTmx -> TileMap::loadFromFile) correcto.\n";
}

void testLevelJsonRoundTrip() {
    EditorState editor(4, 3);
    editor.placeObject(0, 2, "arbusto");
    editor.placeObject(3, 0, "slime");

    std::string json = editor.exportLevelJson("Nivel del editor", "assets/maps/editor_map.tmx",
                                             GridCoord{1, 1});
    auto parsed = LevelLoader::loadFromString(json);
    require(parsed.isOk());
    const LevelDefinition& level = parsed.value();
    require(level.name == "Nivel del editor");
    require(level.mapPath == "assets/maps/editor_map.tmx");
    require(level.playerStart.x == 1 && level.playerStart.y == 1);
    require(level.enemies.empty());  // el editor solo emite "objects"
    require(level.objects.size() == 2);
    require(level.objects[0].objectId == "arbusto");
    require(level.objects[0].position.x == 0 && level.objects[0].position.y == 2);
    // Sin patrulla en el JSON: LevelLoader la rellena con position.
    require(level.objects[0].patrolMin.x == 0 && level.objects[0].patrolMax.y == 2);
    require(level.objects[1].objectId == "slime");

    // Nivel sin objetos: JSON valido con array vacio.
    EditorState bare(2, 2);
    auto emptyParsed = LevelLoader::loadFromString(
        bare.exportLevelJson("Vacio", "m.tmx", GridCoord{0, 0}));
    require(emptyParsed.isOk());
    require(emptyParsed.value().objects.empty());

    // Escape: un nombre con comillas sobrevive el round-trip.
    auto quoted = LevelLoader::loadFromString(
        bare.exportLevelJson("La \"cueva\"", "m.tmx", GridCoord{0, 0}));
    require(quoted.isOk());
    require(quoted.value().name == "La \"cueva\"");

    std::cout << "[EDITOR] round-trip JSON real (exportLevelJson -> LevelLoader) correcto.\n";
}

}  // namespace

int main() {
    testEditingOps();
    testToolsAndPalettes();
    testTmxRoundTrip();
    testLevelJsonRoundTrip();
    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
