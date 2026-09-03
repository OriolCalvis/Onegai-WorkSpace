#include "Editor/EditorState.h"

#include "TestHarness.h"

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

}  // namespace

int main() { return motor_test::runAllTests(); }
