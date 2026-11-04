// C2-04c — test_fuego_nivel: las casillas que arden (orden estable) y su semilla.
#include <set>

#include "Game/FuegoCeldas.h"
#include "TestHarness.h"

using namespace Game;

MOTOR_TEST_CASE(celdas_a_encender_en_orden_estable) {
    EscenaViva s;
    for (const auto& c : {GridCoord{21, 13}, GridCoord{19, 12}, GridCoord{20, 13}, GridCoord{21, 12}, GridCoord{19, 13}})
        s.fuego(c, true);
    const auto v = celdasAEncender(s);
    MOTOR_CHECK(v.size() == 5);
    const GridCoord esperado[] = {{19, 12}, {21, 12}, {19, 13}, {20, 13}, {21, 13}};
    for (std::size_t i = 0; i < v.size() && i < 5; ++i) MOTOR_CHECK(v[i].x == esperado[i].x && v[i].y == esperado[i].y);
    s.fuego(GridCoord{20, 13}, false);
    MOTOR_CHECK(celdasAEncender(s).size() == 4);
    MOTOR_CHECK(celdasAEncender(EscenaViva{}).empty());
}

MOTOR_TEST_CASE(semilla_determinista_y_distinta_por_celda) {
    MOTOR_CHECK(semillaDe(GridCoord{19, 12}) == semillaDe(GridCoord{19, 12}));
    std::set<std::uint32_t> vistas;
    for (int x = 15; x < 25; ++x)
        for (int y = 10; y < 16; ++y) vistas.insert(semillaDe(GridCoord{x, y}));
    MOTOR_CHECK(vistas.size() == 60);
    MOTOR_CHECK(semillaDe(GridCoord{1, 2}) != semillaDe(GridCoord{2, 1}));
}

int main() { return motor_test::runAllTests(); }
