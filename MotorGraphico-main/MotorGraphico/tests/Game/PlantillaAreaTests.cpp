// AV13-08 — Dibujos de aceptacion: # afectada, . fuera del area.
#include "Game/PlantillaArea.h"
#include "TestHarness.h"

#include <initializer_list>
#include <limits>
#include <string>

namespace {
using Forma = Game::PlantillaArea::Forma;

// El dibujo declara el resultado completo, incluido su orden y cardinalidad.
// No reproduce las formulas geometricas de la implementacion.
void dibujo(Forma forma, int tamano, GridCoord origen, GridCoord hacia,
            GridCoord esquina, std::initializer_list<const char*> filas,
            const std::function<bool(int, int)>& bloquea = {}) {
    std::vector<GridCoord> esperadas;
    int y = esquina.y;
    for (const char* fila : filas) {
        const std::string texto(fila);
        for (std::size_t x = 0; x < texto.size(); ++x) {
            MOTOR_CHECK(texto[x] == '#' || texto[x] == '.');
            if (texto[x] == '#') esperadas.push_back({esquina.x + static_cast<int>(x), y});
        }
        ++y;
    }
    const auto reales = Game::casillas({forma, tamano}, origen, hacia, bloquea);
    MOTOR_CHECK(reales.size() == esperadas.size());
    for (std::size_t i = 0; i < reales.size(); ++i) {
        MOTOR_CHECK(reales[i].x == esperadas[i].x);
        MOTOR_CHECK(reales[i].y == esperadas[i].y);
    }
}
}  // namespace

MOTOR_TEST_CASE(circulo_radio_1) {
    dibujo(Forma::Circulo, 1, {0, 0}, {99, 99}, {-1, -1}, {
        ".#.",
        "###",
        ".#."});
}

MOTOR_TEST_CASE(circulo_radio_2) {
    dibujo(Forma::Circulo, 2, {0, 0}, {}, {-2, -2}, {
        "..#..",
        ".###.",
        "#####",
        ".###.",
        "..#.."});
}

MOTOR_TEST_CASE(circulo_radio_3_desplazado) {
    dibujo(Forma::Circulo, 3, {7, -4}, {}, {4, -7}, {
        "...#...",
        ".#####.",
        ".#####.",
        "#######",
        ".#####.",
        ".#####.",
        "...#..."});
}

MOTOR_TEST_CASE(cono_este_90_grados) {
    dibujo(Forma::Cono, 3, {}, {9, 0}, {-3, -3}, {
        ".......",
        ".....#.",
        "....##.",
        "....###",
        "....##.",
        ".....#.",
        "......."});
}

MOTOR_TEST_CASE(cono_oeste_90_grados) {
    dibujo(Forma::Cono, 3, {}, {-1, 0}, {-3, -3}, {
        ".......",
        ".#.....",
        ".##....",
        "###....",
        ".##....",
        ".#.....",
        "......."});
}

MOTOR_TEST_CASE(cono_norte_90_grados) {
    dibujo(Forma::Cono, 3, {}, {0, -1}, {-3, -3}, {
        "...#...",
        ".#####.",
        "..###..",
        ".......",
        ".......",
        ".......",
        "......."});
}

MOTOR_TEST_CASE(cono_sur_90_grados) {
    dibujo(Forma::Cono, 3, {}, {0, 1}, {-3, -3}, {
        ".......",
        ".......",
        ".......",
        ".......",
        "..###..",
        ".#####.",
        "...#..."});
}

MOTOR_TEST_CASE(cono_diagonal_sureste) {
    dibujo(Forma::Cono, 3, {}, {1, 20}, {-3, -3}, {
        ".......",
        ".......",
        ".......",
        "....###",
        "...###.",
        "...###.",
        "...#..."});
}

MOTOR_TEST_CASE(cono_otras_diagonales) {
    dibujo(Forma::Cono, 2, {}, {-1, -1}, {-2, -2}, {
        "..#..", ".##..", "##...", ".....", "....."});
    dibujo(Forma::Cono, 2, {}, {1, -1}, {-2, -2}, {
        "..#..", "..##.", "...##", ".....", "....."});
    dibujo(Forma::Cono, 2, {}, {-1, 1}, {-2, -2}, {
        ".....", ".....", "##...", ".##..", "..#.."});
}

MOTOR_TEST_CASE(linea_diagonal_prolonga_destino_cercano) {
    dibujo(Forma::Linea, 3, {}, {1, 1}, {}, {
        "....", ".#..", "..#.", "...#"});
}

MOTOR_TEST_CASE(linea_bresenham_pendiente_no_diagonal) {
    dibujo(Forma::Linea, 6, {}, {3, 1}, {}, {
        ".#.....",
        "..###..",
        ".....##"});
}

MOTOR_TEST_CASE(linea_inversa_ordenada_por_filas) {
    dibujo(Forma::Linea, 3, {}, {-9, -9}, {-3, -3}, {
        "#...", ".#..", "..#.", "...."});
    dibujo(Forma::Linea, 3, {}, {-9, 0}, {-3, 0}, {"###."});
    dibujo(Forma::Linea, 3, {}, {0, -9}, {0, -3}, {"#", "#", "#", "."});
}

MOTOR_TEST_CASE(cuadrado_lado_3) {
    dibujo(Forma::Cuadrado, 3, {5, 6}, {99, 99}, {4, 5}, {
        "###", "###", "###"});
}

MOTOR_TEST_CASE(cuadrado_par_centrado_con_desempate_fijo) {
    dibujo(Forma::Cuadrado, 2, {}, {}, {-1, -1}, {"##.", "##.", "..."});
}

MOTOR_TEST_CASE(muro_vertical_corta_circulo) {
    // Muro x=1. Como seVen(), la casilla del muro cuenta; x=2 queda oculta.
    dibujo(Forma::Circulo, 2, {}, {}, {-2, -2}, {
        "..#..",
        ".###.",
        "####.",
        ".###.",
        "..#.."}, [](int x, int) { return x == 1; });
}

MOTOR_TEST_CASE(muro_corta_tambien_cono_linea_y_cuadrado) {
    const auto muro = [](int x, int) { return x == 1; };
    dibujo(Forma::Cono, 3, {}, {1, 0}, {-3, -3}, {
        ".......", ".......", "....#..", "....#..",
        "....#..", ".......", "......."}, muro);
    dibujo(Forma::Linea, 4, {}, {1, 0}, {}, {".#..."}, muro);
    dibujo(Forma::Cuadrado, 5, {}, {}, {-2, -2}, {
        "###..", "####.", "####.", "####.", "###.."}, muro);
}

MOTOR_TEST_CASE(vista_diagonal_mismo_desempate_que_tablero) {
    // Hacia (2,1), Bresenham pasa primero por (1,1); hacia (2,0), por (1,0).
    const auto muro = [](int x, int y) { return x == 1 && y == 1; };
    const auto area = Game::casillas({Forma::Cuadrado, 5}, {}, {}, muro);
    bool ve20 = false, ve21 = false;
    for (GridCoord c : area) {
        if (c.x == 2 && c.y == 0) ve20 = true;
        if (c.x == 2 && c.y == 1) ve21 = true;
    }
    MOTOR_CHECK(ve20);
    MOTOR_CHECK(!ve21);
}

MOTOR_TEST_CASE(vista_no_consulta_extremos) {
    int consultas = 0;
    dibujo(Forma::Linea, 1, {4, 5}, {4, 6}, {4, 5}, {".", "#"},
            [&](int, int) { ++consultas; return true; });
    MOTOR_CHECK(consultas == 0);
}

MOTOR_TEST_CASE(tamanos_degenerados_y_direccion_nula) {
    for (Forma forma : {Forma::Circulo, Forma::Cono, Forma::Linea, Forma::Cuadrado}) {
        MOTOR_CHECK(Game::casillas({forma, -1}, {}, {1, 0}).empty());
    }
    dibujo(Forma::Circulo, 0, {2, 3}, {}, {2, 3}, {"#"});
    MOTOR_CHECK(Game::casillas({Forma::Cono, 0}, {}, {1, 0}).empty());
    MOTOR_CHECK(Game::casillas({Forma::Linea, 0}, {}, {1, 0}).empty());
    MOTOR_CHECK(Game::casillas({Forma::Cuadrado, 0}, {}, {}).empty());
    MOTOR_CHECK(Game::casillas({Forma::Cono, 3}, {4, 5}, {4, 5}).empty());
    MOTOR_CHECK(Game::casillas({Forma::Linea, 3}, {4, 5}, {4, 5}).empty());
}

MOTOR_TEST_CASE(coordenadas_extremas_no_desbordan) {
    constexpr int maximo = std::numeric_limits<int>::max();
    constexpr int minimo = std::numeric_limits<int>::min();
    const auto linea = Game::casillas({Forma::Linea, 3}, {maximo, maximo}, {minimo, minimo});
    MOTOR_CHECK(linea.size() == 3);
    MOTOR_CHECK(linea.front().x == maximo - 3);
    MOTOR_CHECK(linea.back().y == maximo - 1);
    const auto circulo = Game::casillas({Forma::Circulo, 1}, {maximo, minimo}, {});
    MOTOR_CHECK(circulo.size() == 3);
}

int main() { return motor_test::runAllTests(); }
