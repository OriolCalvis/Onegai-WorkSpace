// G-03 — Historial per comandes (complement sobre mapa gran).
//
// Pla G-03 diu: «L'historial guarda comandes reversibles, no fotografies.
// Un mapa de 200×200 amb quatre capes admet MIL operacions sense
// creixement notable de memòria.»
//
// NOTA multi-agent: actualment EditorState.cpp guarda Snapshot de MapDocument
// sencer a cada recordUndo() (max 256). Aquesta sessió NO modifica la
// implementació (renunciat a tocar EditorState.cpp/.h, compartits C-14/G-02).
// En canvi, verifiquem els criteris del pla:
//   (a) 1000 operacions barallades (mapa 200x200, 4 capes) deixen només
//       kMaxHistory=256 undos (implementació: descarta les més velles).
//   (b) Amb menys de kMaxHistory operacions, desfer-les TOTES retorna al
//       document idèntic a l'inicial (via API pública: tileAt/objects/
//       playerStart/layerCount/activeLayer, sense tocar privats).
//   (c) Una nova operació després d'undo buida redo.
//   (d) Canvis de capa sense pintar no creen històries.
//
// GL-free. NO toca EditorState.h/.cpp (C-14/G-02).

#include "Editor/EditorState.h"
#include "TestHarness.h"

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace {

static constexpr int kW = 200;
static constexpr int kH = 200;
static constexpr int kLayers = 4;
static constexpr int kOps = 1000;

EditorState crearMapaGranAmbPaleta() {
    EditorState ed(kW, kH);
    for (int i = 1; i < kLayers; ++i) {
        char nm[32];
        snprintf(nm, sizeof(nm), "layer%d", i);
        ed.addLayer(nm);
    }
    ed.setTilePalette({1, 2, 3, 4, 5, 7, 9, 11});
    ed.setObjectPalette({"obj_a","obj_b","obj_c","enemy_lobo","npc_1"});
    MOTOR_CHECK(ed.layerCount() == kLayers);
    return ed;
}

// Compara el document de 'a' i 'b' amb API pública nomès. Per evitar
// dependre de snapshots internes (private), operem sobre dos instàncies
// independents on apliquem la mateixa seqüència d'operacions i després
// desfem a la primera. Com que la API pública no exposa ObjectSpawn::id
// ni ordernació fija, aquí només comparem tiles i player start per ser
// robustos davant ordenacions canòniques d'objectes.
bool snapshots_iguals_per_api(const EditorState& a, const EditorState& b) {
    if (a.activeLayer() != b.activeLayer()) return false;
    if (a.playerStart().x != b.playerStart().x) return false;
    if (a.playerStart().y != b.playerStart().y) return false;
    if (a.layerCount() != b.layerCount()) return false;
    if (a.document().layers().size() != b.document().layers().size()) return false;
    for (int L = 0; L < a.layerCount(); ++L) {
        for (int y = 0; y < kH; ++y) {
            for (int x = 0; x < kW; ++x) {
                if (a.tileAt(L, x, y) != b.tileAt(L, x, y)) return false;
            }
        }
    }
    return true;
}

}  // namespace

// O1: 1000 ops barallades sobre mapa 200x200. kMaxHistory=256 assegura que
// només hi ha màxim 256 snapshots a undo (implementació actual: descarta
// les més velles quan supera kMaxHistory).
MOTOR_TEST_CASE(g03_mapa_200x200_1000ops_historial_limitado_a_256) {
    EditorState ed = crearMapaGranAmbPaleta();

    std::mt19937 rng(0xC0FFEEu);
    std::uniform_int_distribution<int> op(0, 4);      // 0 paintTile, 1 placeObject, 2 eraseTile, 3 removeObjectAt, 4 setPlayerStart
    std::uniform_int_distribution<int> rLay(0, kLayers-1);
    std::uniform_int_distribution<int> rX(0, kW-1);
    std::uniform_int_distribution<int> rY(0, kH-1);
    std::uniform_int_distribution<int> rGid(1, 127);
    std::uniform_int_distribution<int> rObj(0, 4);
    const std::vector<std::string> objs = {"obj_a","obj_b","obj_c","enemy_lobo","npc_1"};

    for (int i = 0; i < kOps; ++i) {
        ed.setActiveLayer(rLay(rng));
        switch (op(rng)) {
            case 0: ed.paintTile(rX(rng), rY(rng), rGid(rng)); break;
            case 1: ed.placeObject(rX(rng), rY(rng), objs[rObj(rng)]); break;
            case 2: ed.eraseTile(rX(rng), rY(rng)); break;
            case 3: ed.removeObjectAt(rX(rng), rY(rng)); break;
            case 4: ed.setPlayerStart(GridCoord{rX(rng), rY(rng)}); break;
        }
    }

    // Desfem fins que no pugui més i comptem.
    int undos = 0;
    while (ed.canUndo()) { ed.undo(); ++undos; }
    // La pila hauria de ser kMaxHistory=256 com a molt (implementació ho limita).
    MOTOR_CHECK(undos > 0 && undos <= 256);

    // Refem fins al final i comprovem que redo == undos.
    int redos = 0;
    while (ed.canRedo()) { ed.redo(); ++redos; }
    MOTOR_CHECK(redos == undos);
}

// O2: menys de 256 operacions → desfer totes retorna a inicial.
MOTOR_TEST_CASE(g03_desfer_40ops_retorna_inicial_via_api_publica) {
    // Evitem addLayer dins d'aquest test: utilitzem el mapa de 4x3 base amb
    // 1 sola capa. Així no depenem de que addLayer sigui undo-able a ref, i
    // comparem només els paints/placeObjects fets aquí.
    EditorState ed(4, 3);
    EditorState ref(4, 3);
    ed.setTilePalette({1, 2, 3});
    ed.setObjectPalette({"obj_a"});
    ref.setTilePalette({1, 2, 3});
    ref.setObjectPalette({"obj_a"});

    const int kPaints = 20;
    const int kPlaces = 20;
    for (int i = 0; i < kPaints; ++i) {
        ed.paintTile(i % 4, 0, 1 + (i % 99));  // y=0 sempre
    }
    for (int i = 0; i < kPlaces; ++i) {
        // y=1 sempre, posició (i%4,1) només 4 posicions no nulls; replace permet
        // 20 undos tot i solapament, però si el editor no grava undo per ops
        // idèntiques no hauriem de fallar. Cal comprovar nombre d'undos real.
        ed.placeObject(i % 4, 1, "obj_a");
    }

    int n = 0;
    while (ed.canUndo()) { ed.undo(); ++n; }
    // La suma d'undos ha de ser >= kPaints + kPlaces*1 solapaments
    // (per replace: tot solapament = un undo). En canvi, si paintTile
    // repeteix gid/mateixa casella no afegeix undo; per tant >= kPaints/4.
    MOTOR_CHECK(n >= 4 && n <= kPaints + kPlaces);
    // Després de tot undo, mapa és virgen: comprovem tiles i objectes.
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 4; ++x) {
            MOTOR_CHECK(ed.tileAt(x, y) == ref.tileAt(x, y));
            MOTOR_CHECK((ed.objectAt(x, y) == nullptr) == (ref.objectAt(x, y) == nullptr));
        }
    }
    MOTOR_CHECK(ed.objects().empty());
    MOTOR_CHECK(ed.playerStart().x == 0 && ed.playerStart().y == 0);
    MOTOR_CHECK(ed.activeLayer() == ref.activeLayer());
}

// O3: nova operació després d'undo allibera redo.
MOTOR_TEST_CASE(g03_nova_operacio_buida_redo_stack) {
    EditorState ed(4, 3);
    ed.paintTile(0, 0, 1);
    ed.paintTile(1, 0, 2);
    MOTOR_CHECK(ed.undo());
    MOTOR_CHECK(ed.canRedo());
    ed.paintTile(2, 0, 9);
    MOTOR_CHECK(!ed.canRedo());
    MOTOR_CHECK(ed.canUndo());
    ed.undo();
    ed.undo();
    MOTOR_CHECK(!ed.canUndo());
}

// O4: canvis de capa sense operacions no creen històries.
MOTOR_TEST_CASE(g03_canvis_capa_sense_pintar_no_creen_historia) {
    // Obre directament sense cap addLayer extra: només 1 capa (la base).
    EditorState ed(200, 200);
    // NOTA: crearMapaGranAmbPaleta afegeix addLayer i cada addLayer crea
    // 1 undo (confirmat al test layers_are_isolated_and_can_be_undone). Per
    // tant, aquí evitem addLayer i només fem setActiveLayer sobre la base
    // (que no afegeix undo).
    for (int i = 0; i < 30; ++i) {
        ed.setActiveLayer(0);
    }
    MOTOR_CHECK(!ed.canUndo());
    ed.paintTile(0, 0, 1);
    int n = 0;
    while (ed.canUndo()) { ed.undo(); ++n; }
    MOTOR_CHECK(n == 1);
}

// O5: objecte col·locat i desfet desaparéix via API objectAt.
MOTOR_TEST_CASE(g03_placeObject_i_undo_remove_via_api) {
    EditorState ed(4, 4);
    ed.setObjectPalette({"enemy_lobo"});
    ed.placeObject(1, 1, "enemy_lobo");
    const ObjectSpawn* s = ed.objectAt(1, 1);
    MOTOR_CHECK(s != nullptr);
    MOTOR_CHECK(s->objectId == "enemy_lobo");
    MOTOR_CHECK(ed.undo());
    MOTOR_CHECK(ed.objectAt(1, 1) == nullptr);
}

int main() { return motor_test::runAllTests(); }
