#include "Editor/EditorState.h"

#include "TestHarness.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <utility>
#include <vector>

// G-02 — Las herramientas de edición que faltan: cuentagotas, rectángulo y
// línea, cada una como UNA operación de historial (criterio del plan).
//
// Lo que defienden los casos, además del «un trazo, un deshacer»:
// - un trazo que no cambia nada no crea paso ni marca cambios (si no, el
//   autor pulsa Ctrl+Z y «no pasa nada»);
// - lo que se sale del mapa se recorta sin mover el dibujo;
// - la línea es la misma se arrastre en el sentido que se arrastre;
// - el cuentagotas no es una edición, y cuando el filtro oculta lo que
//   elige, lo enseña sin cambiar las otras selecciones.

namespace {

using Grid = std::vector<std::vector<int>>;

Grid capa(const EditorState& e, int layer) {
    Grid g(static_cast<std::size_t>(e.height()), std::vector<int>(static_cast<std::size_t>(e.width())));
    for (int y = 0; y < e.height(); ++y)
        for (int x = 0; x < e.width(); ++x) g[y][x] = e.tileAt(layer, x, y);
    return g;
}

int pintadas(const EditorState& e, int gid, int layer = 0) {
    int n = 0;
    for (const auto& fila : capa(e, layer)) n += static_cast<int>(std::count(fila.begin(), fila.end(), gid));
    return n;
}

// Cuántos pasos de deshacer hay, sin perderlos: deshace todo y lo rehace.
int pasos(EditorState& e) {
    int n = 0;
    while (e.undo()) ++n;
    for (int i = 0; i < n; ++i) MOTOR_CHECK(e.redo());
    return n;
}

bool contiene(const std::vector<GridCoord>& cells, int x, int y) {
    return std::any_of(cells.begin(), cells.end(),
                       [&](const GridCoord& c) { return c.x == x && c.y == y; });
}

std::vector<std::pair<int, int>> ordenadas(std::vector<GridCoord> cells) {
    std::vector<std::pair<int, int>> v;
    for (const GridCoord& c : cells) v.emplace_back(c.x, c.y);
    std::sort(v.begin(), v.end());
    return v;
}

Result<bool> bloquear(EditorState& e, int index, bool locked) {
    int visto = -1;
    for (const MapLayer& layer : e.document().layers()) {
        if (layer.type() != MapLayerType::Tile) continue;
        if (++visto != index) continue;
        MapLayerMeta meta = layer.meta();
        meta.locked = locked;
        return e.setLayerMeta(index, meta);
    }
    return Result<bool>::Error("capa no encontrada");
}

}  // namespace

MOTOR_TEST_CASE(rectangulo_lleno_es_un_paso) {
    EditorState e(10, 8);
    e.markClean();
    MOTOR_CHECK(e.paintRect({4, 3}, {1, 1}, 5));  // extremos en cualquier orden
    MOTOR_CHECK(pintadas(e, 5) == 12);            // 4 x 3
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 10; ++x)
            MOTOR_CHECK((e.tileAt(x, y) == 5) == (x >= 1 && x <= 4 && y >= 1 && y <= 3));
    MOTOR_CHECK(e.isModified());
    MOTOR_CHECK(pasos(e) == 1);

    MOTOR_CHECK(e.undo());
    MOTOR_CHECK(pintadas(e, 5) == 0);
    MOTOR_CHECK(!e.canUndo());
    MOTOR_CHECK(e.redo());
    MOTOR_CHECK(pintadas(e, 5) == 12);

    // Borrar con gid 0 también es un rectángulo: sólo cuenta lo que cambia.
    MOTOR_CHECK(e.paintRect({0, 0}, {9, 7}, 0));
    MOTOR_CHECK(pintadas(e, 5) == 0);
    MOTOR_CHECK(pasos(e) == 2);

    // Un mapa entero de 40x40 (1600 celdas) sigue siendo un solo paso.
    EditorState grande(40, 40);
    MOTOR_CHECK(grande.paintRect({0, 0}, {39, 39}, 3));
    MOTOR_CHECK(pintadas(grande, 3) == 1600);
    MOTOR_CHECK(pasos(grande) == 1);
    MOTOR_CHECK(grande.undo());
    MOTOR_CHECK(pintadas(grande, 3) == 0);
}

MOTOR_TEST_CASE(rectangulo_de_borde) {
    EditorState e(10, 8);
    MOTOR_CHECK(e.paintRect({1, 1}, {5, 4}, 2, false));
    MOTOR_CHECK(pintadas(e, 2) == 14);  // 5x4 = 20, menos el interior 3x2
    for (int y = 2; y <= 3; ++y)
        for (int x = 2; x <= 4; ++x) MOTOR_CHECK(e.tileAt(x, y) == 0);
    MOTOR_CHECK(e.tileAt(1, 1) == 2 && e.tileAt(5, 4) == 2 && e.tileAt(5, 2) == 2);
    MOTOR_CHECK(pasos(e) == 1);

    // Uno de ancho es su propio borde; un punto es una celda.
    MOTOR_CHECK(EditorState::RectCells({3, 0}, {3, 5}, false).size() == 6);
    MOTOR_CHECK(EditorState::RectCells({2, 2}, {2, 2}, false).size() == 1);
    MOTOR_CHECK(EditorState::RectCells({0, 0}, {1, 1}, false).size() == 4);
    // Sin celdas repetidas en el borde (las esquinas no se cuentan dos veces).
    auto borde = ordenadas(EditorState::RectCells({0, 0}, {6, 3}, false));
    MOTOR_CHECK(std::adjacent_find(borde.begin(), borde.end()) == borde.end());
    MOTOR_CHECK(borde.size() == 18);
}

MOTOR_TEST_CASE(lo_que_se_sale_del_mapa_se_recorta_sin_mover_el_dibujo) {
    // Lleno: se queda la parte de dentro.
    EditorState lleno(10, 8);
    MOTOR_CHECK(lleno.paintRect({-3, -2}, {2, 1}, 4));
    MOTOR_CHECK(pintadas(lleno, 4) == 6);  // x 0..2, y 0..1
    MOTOR_CHECK(lleno.tileAt(2, 1) == 4 && lleno.tileAt(3, 1) == 0 && lleno.tileAt(2, 2) == 0);

    // Borde con el lado izquierdo fuera: ese lado NO se pinta en la
    // columna 0 (recortarlo lo metería dentro).
    EditorState borde(10, 8);
    MOTOR_CHECK(borde.paintRect({-2, 1}, {3, 4}, 4, false));
    MOTOR_CHECK(pintadas(borde, 4) == 10);  // fila 1 (4) + fila 4 (4) + columna 3 (2)
    MOTOR_CHECK(borde.tileAt(0, 2) == 0 && borde.tileAt(0, 3) == 0);
    MOTOR_CHECK(borde.tileAt(0, 1) == 4 && borde.tileAt(3, 2) == 4);

    // El ratón muy lejos da el mismo borde, y en tiempo acotado.
    EditorState lejos(10, 8);
    const auto t0 = std::chrono::steady_clock::now();
    MOTOR_CHECK(lejos.paintRect({-2000000000, 1}, {3, 4}, 4, false));
    MOTOR_CHECK(lejos.paintRect({-5, -5}, {2000000000, 2000000000}, 6, true));
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();
    MOTOR_CHECK(ms < 2000);
    MOTOR_CHECK(pintadas(lejos, 6) == 80);
    MOTOR_CHECK(lejos.undo());
    MOTOR_CHECK(capa(lejos, 0) == capa(borde, 0));

    // Entero fuera: nada, ni paso ni cambios.
    EditorState fuera(10, 8);
    fuera.markClean();
    MOTOR_CHECK(!fuera.paintRect({10, 0}, {20, 5}, 4));
    MOTOR_CHECK(!fuera.paintRect({-9, -9}, {-1, 20}, 4, false));
    MOTOR_CHECK(!fuera.paintLine({-5, -1}, {12, -1}, 4));
    MOTOR_CHECK(!fuera.canUndo() && !fuera.isModified());

    // Línea que cruza el borde: sólo la parte de dentro.
    EditorState cruza(10, 8);
    MOTOR_CHECK(cruza.paintLine({-3, 2}, {12, 2}, 1));
    MOTOR_CHECK(pintadas(cruza, 1) == 10);
}

MOTOR_TEST_CASE(linea_bresenham_simetrica_y_un_paso) {
    // Oráculo a mano: de (0,0) a (6,2).
    const std::vector<std::pair<int, int>> esperada = {{0, 0}, {1, 0}, {2, 1}, {3, 1}, {4, 1}, {5, 2}, {6, 2}};
    auto ida = EditorState::LineCells({0, 0}, {6, 2});
    std::vector<std::pair<int, int>> idaPares;
    for (const GridCoord& c : ida) idaPares.emplace_back(c.x, c.y);
    MOTOR_CHECK(idaPares == esperada);

    // En cualquier sentido, las mismas celdas. Se prueban todas las
    // parejas de un 7x7 alrededor del origen, no sólo un ejemplo.
    for (int x = -3; x <= 3; ++x) {
        for (int y = -3; y <= 3; ++y) {
            const GridCoord a{1, 2};
            const GridCoord b{1 + x * 2, 2 + y * 3};
            auto ab = EditorState::LineCells(a, b);
            auto ba = EditorState::LineCells(b, a);
            MOTOR_CHECK(ordenadas(ab) == ordenadas(ba));
            // Extremos incluidos, celdas contiguas (8-vecindad), sin huecos
            // ni repeticiones, y tantas como el lado mayor + 1.
            MOTOR_CHECK(contiene(ab, a.x, a.y) && contiene(ab, b.x, b.y));
            MOTOR_CHECK(static_cast<int>(ab.size()) ==
                        std::max(std::abs(b.x - a.x), std::abs(b.y - a.y)) + 1);
            for (std::size_t i = 1; i < ab.size(); ++i) {
                MOTOR_CHECK(std::abs(ab[i].x - ab[i - 1].x) <= 1);
                MOTOR_CHECK(std::abs(ab[i].y - ab[i - 1].y) <= 1);
                MOTOR_CHECK(!(ab[i].x == ab[i - 1].x && ab[i].y == ab[i - 1].y));
            }
        }
    }

    // Pintarla es un paso, y el documento es el mismo arrastrando al revés.
    EditorState e1(10, 8), e2(10, 8);
    MOTOR_CHECK(e1.paintLine({1, 7}, {8, 0}, 3));
    MOTOR_CHECK(e2.paintLine({8, 0}, {1, 7}, 3));
    MOTOR_CHECK(capa(e1, 0) == capa(e2, 0));
    MOTOR_CHECK(pintadas(e1, 3) == 8);
    MOTOR_CHECK(pasos(e1) == 1);
    MOTOR_CHECK(e1.undo());
    MOTOR_CHECK(pintadas(e1, 3) == 0);
}

MOTOR_TEST_CASE(trazo_sin_cambios_capa_bloqueada_y_gid_invalido_no_crean_paso) {
    EditorState e(10, 8);
    MOTOR_CHECK(e.paintRect({0, 0}, {3, 3}, 2));
    const int antes = pasos(e);  // pasos() deshace y rehace: antes de markClean
    e.markClean();

    // Mismo GID encima: nada cambia.
    MOTOR_CHECK(!e.paintRect({1, 1}, {2, 2}, 2));
    MOTOR_CHECK(!e.paintLine({0, 0}, {3, 3}, 2));
    MOTOR_CHECK(!e.paintRect({0, 0}, {3, 3}, 2, false));
    // GID negativo.
    MOTOR_CHECK(!e.paintRect({5, 5}, {6, 6}, -1));
    MOTOR_CHECK(!e.paintLine({5, 5}, {6, 6}, -3));
    MOTOR_CHECK(!e.isModified());
    MOTOR_CHECK(pasos(e) == antes);

    // Un trazo que cambia UNA celda de muchas sí es un paso.
    MOTOR_CHECK(e.paintLine({0, 0}, {4, 0}, 2));
    MOTOR_CHECK(pasos(e) == antes + 1);
    MOTOR_CHECK(e.tileAt(4, 0) == 2);

    // Capa bloqueada: ni rectángulo ni línea, y el documento intacto.
    MOTOR_CHECK(bloquear(e, 0, true).isOk());
    const Grid congelada = capa(e, 0);
    const int conBloqueo = pasos(e);
    MOTOR_CHECK(!e.paintRect({5, 5}, {9, 7}, 8));
    MOTOR_CHECK(!e.paintRect({5, 5}, {9, 7}, 8, false));
    MOTOR_CHECK(!e.paintLine({9, 0}, {0, 7}, 8));
    MOTOR_CHECK(capa(e, 0) == congelada);
    MOTOR_CHECK(pasos(e) == conBloqueo);

    // Pinta en la capa ACTIVA y no en otra.
    MOTOR_CHECK(bloquear(e, 0, false).isOk());
    const int segunda = e.addLayer("detalles");
    MOTOR_CHECK(segunda == 1 && e.activeLayer() == 1);
    MOTOR_CHECK(e.paintRect({5, 5}, {6, 6}, 9));
    MOTOR_CHECK(pintadas(e, 9, 1) == 4);
    MOTOR_CHECK(pintadas(e, 9, 0) == 0);
    MOTOR_CHECK(e.undo());
    MOTOR_CHECK(pintadas(e, 9, 1) == 0);
    MOTOR_CHECK(e.activeLayer() == 1);
}

MOTOR_TEST_CASE(cuentagotas_elige_sin_editar) {
    EditorState e(10, 8);
    e.setTilePalette({3, 7, 9});
    e.paintTile(2, 2, 7);
    e.paintTile(4, 4, 11);  // no está en la paleta
    const int antes = pasos(e);
    e.markClean();
    MOTOR_CHECK(e.selectedTileGid() == 3);

    MOTOR_CHECK(e.pickTileAt(2, 2));
    MOTOR_CHECK(e.selectedTileGid() == 7);
    MOTOR_CHECK(!e.isModified());     // no es un cambio del documento
    MOTOR_CHECK(pasos(e) == antes);   // ni un paso de historial
    MOTOR_CHECK(e.tileAt(2, 2) == 7);

    // Casos en que no elige nada: la selección no cambia.
    MOTOR_CHECK(!e.pickTileAt(0, 0));    // vacía
    MOTOR_CHECK(!e.pickTileAt(-1, 3));   // fuera
    MOTOR_CHECK(!e.pickTileAt(10, 0));   // fuera
    MOTOR_CHECK(!e.pickTileAt(4, 4));    // GID sin entrada en la paleta
    MOTOR_CHECK(e.selectedTileGid() == 7);

    // Lee la capa activa.
    e.addLayer("encima");
    e.paintTile(2, 2, 9);
    MOTOR_CHECK(e.pickTileAt(2, 2));
    MOTOR_CHECK(e.selectedTileGid() == 9);
    e.setActiveLayer(0);
    MOTOR_CHECK(e.pickTileAt(2, 2));
    MOTOR_CHECK(e.selectedTileGid() == 7);

    // Y lo que elige se puede usar enseguida: cuentagotas + rectángulo.
    MOTOR_CHECK(e.paintRect({6, 6}, {7, 7}, e.selectedTileGid()));
    MOTOR_CHECK(e.tileAt(0, 7, 7) == 7);
}

MOTOR_TEST_CASE(cuentagotas_quita_el_filtro_y_respeta_las_otras_selecciones) {
    EditorState e(10, 8);
    e.setTilePalette({3, 7, 9});
    e.setTilePaletteLabels({{3, "barro"}, {7, "hielo"}, {9, "barranco"}});
    e.setObjectPalette({"arbol", "banco", "barril"});
    e.setLevelPalette({"niveles/cueva.json", "niveles/barca.json", "niveles/bahia.json"});
    e.paintTile(1, 1, 7);

    e.setPaletteFilter("ba");
    MOTOR_CHECK(e.tilePalette() == std::vector<int>({3, 9}));
    MOTOR_CHECK(e.objectPalette() == std::vector<std::string>({"banco", "barril"}));
    e.setTool(EditorTool::PlaceObject);
    e.nextPaletteEntry();
    MOTOR_CHECK(e.selectedObjectId() == "barril");
    e.setTool(EditorTool::LinkLevel);
    e.nextPaletteEntry();
    MOTOR_CHECK(e.selectedLevelPath() == "niveles/bahia.json");
    e.setTool(EditorTool::PaintTile);

    // El 7 está oculto por el filtro: se quita el filtro y se elige.
    MOTOR_CHECK(e.pickTileAt(1, 1));
    MOTOR_CHECK(e.paletteFilter().empty());
    MOTOR_CHECK(e.tilePalette().size() == 3);
    MOTOR_CHECK(e.selectedTileGid() == 7);
    // Las otras selecciones siguen siendo las mismas ENTRADAS (su índice
    // ha cambiado al desaparecer el filtro).
    MOTOR_CHECK(e.selectedObjectId() == "barril");
    MOTOR_CHECK(e.selectedLevelPath() == "niveles/bahia.json");

    // Un GID que no está en la paleta no quita el filtro.
    e.setPaletteFilter("hielo");
    e.paintTile(5, 5, 42);
    MOTOR_CHECK(!e.pickTileAt(5, 5));
    MOTOR_CHECK(e.paletteFilter() == "hielo");
}

MOTOR_TEST_CASE(historial_mezclado_se_deshace_y_rehace_entero) {
    EditorState e(12, 10);
    std::vector<Grid> estados{capa(e, 0)};
    MOTOR_CHECK(e.paintRect({0, 0}, {11, 9}, 1));
    estados.push_back(capa(e, 0));
    MOTOR_CHECK(e.paintRect({2, 2}, {9, 7}, 2, false));
    estados.push_back(capa(e, 0));
    MOTOR_CHECK(e.paintLine({0, 9}, {11, 0}, 3));
    estados.push_back(capa(e, 0));
    e.fillTiles(5, 5, 4);  // la zona conectada bajo (5,5), recortada por el borde y la línea
    estados.push_back(capa(e, 0));
    MOTOR_CHECK(e.pickTileAt(5, 5) == false);  // paleta vacía: nada que elegir
    MOTOR_CHECK(pasos(e) == 4);

    for (int i = 4; i > 0; --i) {
        MOTOR_CHECK(capa(e, 0) == estados[i]);
        MOTOR_CHECK(e.undo());
    }
    MOTOR_CHECK(capa(e, 0) == estados[0]);
    MOTOR_CHECK(!e.canUndo());
    for (int i = 1; i <= 4; ++i) {
        MOTOR_CHECK(e.redo());
        MOTOR_CHECK(capa(e, 0) == estados[i]);
    }

    // Un trazo nuevo tras deshacer descarta el rehacer, como las demás.
    MOTOR_CHECK(e.undo());
    MOTOR_CHECK(e.paintLine({0, 0}, {0, 9}, 5));
    MOTOR_CHECK(!e.canRedo());
}

int main() { return motor_test::runAllTests(); }
