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
#include "Editor/EditorValidation.h"
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

    // Cubo de relleno: toda la región conectada cambia en una operación
    // deshacible; la isla separada no se toca.
    editor.paintTile(0, 0, 1);
    editor.paintTile(1, 0, 1);
    editor.paintTile(3, 2, 1);
    editor.setTool(EditorTool::FillTiles);
    editor.applyAt(0, 0);
    require(editor.tileAt(0, 0) == 2 && editor.tileAt(1, 0) == 2);
    require(editor.tileAt(3, 2) == 1);
    require(editor.undo());
    require(editor.tileAt(0, 0) == 1 && editor.tileAt(1, 0) == 1);

    // Enlace visual: un objeto existente conserva su identidad y gana un
    // targetLevel serializable; undo lo elimina de nuevo.
    editor.placeObject(2, 2, "puerta");
    editor.setLevelPalette({"assets/levels/interior_casa.json"});
    editor.setTool(EditorTool::LinkLevel);
    editor.applyAt(2, 2);
    require(editor.objectAt(2, 2)->targetLevel == "assets/levels/interior_casa.json");
    require(editor.undo());
    require(editor.objectAt(2, 2)->targetLevel.empty());

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

void testHistoryAndPlayerStart() {
    EditorState editor(4, 3);
    editor.setTilePalette({1, 2});
    editor.paintTile(1, 1, 2);
    editor.placeObject(2, 1, "slime");
    editor.setPlayerStart(GridCoord{3, 2});
    require(editor.canUndo());
    require(editor.playerStart().x == 3 && editor.playerStart().y == 2);

    require(editor.undo());
    require(editor.playerStart().x == 0 && editor.playerStart().y == 0);
    require(editor.undo());
    require(editor.objectAt(2, 1) == nullptr);
    require(editor.undo());
    require(editor.tileAt(1, 1) == 0);
    require(!editor.canUndo());
    require(editor.canRedo());

    require(editor.redo());
    require(editor.tileAt(1, 1) == 2);
    require(editor.redo());
    require(editor.objectAt(2, 1) != nullptr);
    require(editor.redo());
    require(editor.playerStart().x == 3 && editor.playerStart().y == 2);
    require(!editor.canRedo());

    editor.setPlayerStart(GridCoord{99, 99});
    require(editor.playerStart().x == 3 && editor.playerStart().y == 2);
    editor.clearHistory();
    require(!editor.canUndo() && !editor.canRedo());

    std::cout << "[EDITOR] historial undo/redo + playerStart correcto.\n";
}

void testValidation() {
    EditorState editor(4, 3);
    ObjectCatalog catalog;
    ObjectDefinition slime;
    slime.id = "slime";
    catalog.add(slime);

    // Dos zonas: el inicio puede llegar a la primera, pero la segunda
    // queda separada por collisionGids y debe avisarse al autor.
    editor.paintTile(0, 0, 1);
    editor.paintTile(1, 0, 1);
    editor.paintTile(2, 0, 2);
    editor.paintTile(3, 0, 1);
    editor.setPlayerStart(GridCoord{0, 0});
    editor.placeObject(1, 0, "slime");
    auto valid = EditorValidation::check(editor, catalog, {2});
    require(valid.ok());
    require(valid.walkableTiles == 3);
    require(valid.reachableTiles == 2);
    require(valid.unreachableTiles == 1);
    require(valid.warnings.size() == 1);

    editor.placeObject(0, 1, "id_inexistente");
    auto invalidObject = EditorValidation::check(editor, catalog, {2});
    require(!invalidObject.ok());
    require(invalidObject.errors.size() == 1);

    editor.setPlayerStart(GridCoord{2, 0});
    auto invalidStart = EditorValidation::check(editor, catalog, {2});
    require(!invalidStart.ok());
    require(invalidStart.errors.size() == 2);

    std::cout << "[EDITOR] validacion de inicio, objetos y conectividad correcta.\n";
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

    editor.setPlayerStart(GridCoord{1, 1});
    std::string json = editor.exportLevelJson("Nivel del editor", "assets/maps/editor_map.tmx");
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
    auto emptyParsed = LevelLoader::loadFromString(bare.exportLevelJson("Vacio", "m.tmx"));
    require(emptyParsed.isOk());
    require(emptyParsed.value().objects.empty());

    // Escape: un nombre con comillas sobrevive el round-trip.
    auto quoted = LevelLoader::loadFromString(bare.exportLevelJson("La \"cueva\"", "m.tmx"));
    require(quoted.isOk());
    require(quoted.value().name == "La \"cueva\"");

    std::cout << "[EDITOR] round-trip JSON real (exportLevelJson -> LevelLoader) correcto.\n";
}

}  // namespace


// ---------------------------------------------------------------------
// Lo que el editor necesitaba para que un humano pueda hacer niveles sin
// generarlos con un script: buscar en un catalogo de miles de entradas,
// capas, seleccionar y copiar, y tocar patrulla y destino de un objeto.
//
// Va aqui, GL-free, porque es donde se puede comprobar de verdad: lo que
// vive en level_editor.cpp solo se puede pasar por el compilador.
// ---------------------------------------------------------------------
void testFiltroCapasSeleccionYPropiedades() {
    // ---------------- FILTRO DE PALETA ----------------
    {
        EditorState e(8, 8);
        e.setObjectPalette({"npc_tabernero", "prop_taberna_mesa", "npc_herrero",
                            "PROP_Taberna_Silla", "enemigo_lobo"});
        require(e.objectPalette().size() == 5);
        require(e.objectPaletteTotal() == 5);

        e.setPaletteFilter("taber");
        // Sin mayusculas: "PROP_Taberna_Silla" tiene que entrar.
        require(e.objectPalette().size() == 3);
        require(e.objectPaletteTotal() == 5);        // el total no se pierde
        e.setTool(EditorTool::PlaceObject);
        require(e.selectedObjectId() == "npc_tabernero");
        e.nextPaletteEntry();
        require(e.selectedObjectId() == "prop_taberna_mesa");

        // Un filtro que no encuentra nada deja la paleta vacia, y colocar
        // con paleta vacia no puede reventar ni colocar un objeto sin id.
        e.setPaletteFilter("zzzz");
        require(e.objectPalette().empty());
        require(e.selectedObjectId().empty());
        e.applyAt(1, 1);
        require(e.objects().empty());

        e.setPaletteFilter("");
        require(e.objectPalette().size() == 5);
        std::printf("  filtro: 5 -> 3 -> 0 -> 5, sin mayusculas, sin perder el total. OK\n");
    }

    // ---------------- CAPAS ----------------
    {
        EditorState e(4, 4);
        require(e.layerCount() == 1);
        require(e.layerName(0) == "suelo");
        e.setTilePalette({1, 2});
        e.paintTile(0, 0, 1);

        const int decorado = e.addLayer("decorado");
        require(decorado == 1);
        require(e.activeLayer() == 1);          // la nueva queda activa
        e.paintTile(0, 0, 2);

        // Cada capa guarda lo suyo: pintar arriba no borra abajo. Esto es
        // exactamente lo que se perdia antes al guardar.
        require(e.tileAt(0, 0, 0) == 1);
        require(e.tileAt(1, 0, 0) == 2);
        require(e.tileAt(0, 0) == 2);           // sin capa = la activa

        // El TMX lleva las dos, con sus nombres.
        TmxTilesetSettings s;
        const std::string tmx = e.exportTmx(s);
        require(tmx.find("name=\"suelo\"") != std::string::npos);
        require(tmx.find("name=\"decorado\"") != std::string::npos);
        std::printf("  capas: pintar en una no toca la otra, y el TMX lleva las dos. OK\n");

        // Un nombre con comillas rompia el XML al reabrirlo.
        e.setLayerName(1, "te\"cho & torres");
        const std::string tmx2 = e.exportTmx(s);
        require(tmx2.find("te&quot;cho &amp; torres") != std::string::npos);
        require(tmx2.find("\"te\"cho") == std::string::npos);
        std::printf("  nombre de capa con comillas y & escapado en el TMX. OK\n");

        require(e.removeLayer(1));
        require(e.layerCount() == 1);
        require(e.activeLayer() == 0);          // no se queda apuntando fuera
        require(!e.removeLayer(0));             // la ultima no se borra
        std::printf("  borrar capa: reajusta la activa y no deja el mapa sin capas. OK\n");
    }

    // ---------------- DESHACER CON CAPAS ----------------
    {
        EditorState e(4, 4);
        e.paintTile(1, 1, 5);
        e.addLayer("techo");
        e.paintTile(1, 1, 7);
        require(e.layerCount() == 2);
        require(e.undo());                      // deshace el pincelazo
        require(e.tileAt(1, 1, 1) == 0);
        require(e.undo());                      // deshace la capa entera
        require(e.layerCount() == 1);
        require(e.activeLayer() == 0);          // y la activa vuelve a existir
        e.paintTile(2, 2, 3);                   // no revienta
        require(e.tileAt(2, 2) == 3);
        std::printf("  deshacer un addLayer devuelve la capa activa a una que existe. OK\n");
    }

    // ---------------- SELECCION Y PORTAPAPELES ----------------
    {
        EditorState e(10, 10);
        e.setTilePalette({1});
        e.setObjectPalette({"npc_x"});
        e.paintTile(1, 1, 4);
        e.paintTile(2, 1, 5);
        e.placeObject(1, 1, "npc_x");
        e.setObjectPatrol(1, 1, GridCoord{1, 1}, GridCoord{3, 1});

        // Arrastrar de derecha a izquierda vale igual, y salirse del mapa
        // se recorta en vez de rechazar.
        e.setSelection(2, 1, 1, 1);
        require(e.hasSelection());
        require(e.selection().x0 == 1 && e.selection().x1 == 2);
        e.setSelection(-5, -5, 2, 1);
        require(e.selection().x0 == 0 && e.selection().y0 == 0);

        e.setSelection(1, 1, 2, 1);
        e.copySelection();
        require(e.hasClipboard());
        require(e.clipboardWidth() == 2 && e.clipboardHeight() == 1);

        e.pasteAt(5, 5);
        require(e.tileAt(5, 5) == 4);
        require(e.tileAt(6, 5) == 5);
        const ObjectSpawn* pegado = e.objectAt(5, 5);
        require(pegado != nullptr);
        require(pegado->objectId == "npc_x");
        // La patrulla viaja CON el objeto: era 1..3 en x, pegada en 5 son 5..7.
        require(pegado->patrolMin.x == 5 && pegado->patrolMax.x == 7);
        std::printf("  copiar/pegar: tiles, objeto y su patrulla desplazada. OK\n");

        // Pegar es UN paso de historial aunque toque varias celdas.
        require(e.undo());
        require(e.tileAt(5, 5) == 0);
        require(e.tileAt(6, 5) == 0);
        require(e.objectAt(5, 5) == nullptr);
        require(e.tileAt(1, 1) == 4);           // el original intacto
        std::printf("  pegar se deshace de una sola vez. OK\n");

        // Pegar pegado al borde recorta, no falla.
        e.pasteAt(9, 9);
        require(e.tileAt(9, 9) == 4);           // el segundo tile cae fuera
        require(e.width() == 10);

        // Borrar la seleccion se lleva tiles Y objetos de dentro.
        e.setSelection(1, 1, 2, 1);
        e.eraseSelection();
        require(e.tileAt(1, 1) == 0);
        require(e.tileAt(2, 1) == 0);
        require(e.objectAt(1, 1) == nullptr);
        require(e.undo());
        require(e.tileAt(1, 1) == 4);
        require(e.objectAt(1, 1) != nullptr);
        std::printf("  borrar seleccion: tiles y objetos, y se deshace entero. OK\n");
    }

    // ---------------- PROPIEDADES Y SERIALIZACION ----------------
    {
        EditorState e(8, 8);
        e.setObjectPalette({"npc_guardia", "puerta"});
        e.placeObject(2, 2, "npc_guardia");
        e.placeObject(4, 4, "puerta");

        // Un objeto recien puesto NO exporta patrulla (esta quieto).
        require(e.exportLevelJson("n", "m.tmx").find("patrolMin") == std::string::npos);

        e.setObjectPatrol(2, 2, GridCoord{2, 2}, GridCoord{6, 2});
        e.setObjectTransition(4, 4, "assets/levels/interior.json");
        e.setObjectTargetPosition(4, 4, GridCoord{1, 7});

        const std::string js = e.exportLevelJson("nivel", "mapa.tmx");
        require(js.find("\"patrolMax\": { \"x\": 6, \"y\": 2 }") != std::string::npos);
        require(js.find("\"targetLevel\": \"assets/levels/interior.json\"") != std::string::npos);
        require(js.find("\"targetPosition\": { \"x\": 1, \"y\": 7 }") != std::string::npos);
        std::printf("  patrulla y destino se escriben en el JSON. OK\n");

        // La patrulla se recorta al mapa: mandar a un PNJ fuera del mapa
        // es mandarlo a caminar contra la nada.
        e.setObjectPatrol(2, 2, GridCoord{-3, 0}, GridCoord{99, 99});
        const ObjectSpawn* g = e.objectAt(2, 2);
        require(g->patrolMin.x == 0 && g->patrolMax.x == 7 && g->patrolMax.y == 7);
        require(e.undo());
        require(e.objectAt(2, 2)->patrolMax.x == 6);
        std::printf("  patrulla recortada al mapa, y con deshacer. OK\n");
    }

    std::cout << "[EDITOR] filtro, capas, seleccion y propiedades correctos.\n";
}

int main() {
    testEditingOps();
    testToolsAndPalettes();
    testHistoryAndPlayerStart();
    testValidation();
    testTmxRoundTrip();
    testLevelJsonRoundTrip();
    testFiltroCapasSeleccionYPropiedades();
    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
