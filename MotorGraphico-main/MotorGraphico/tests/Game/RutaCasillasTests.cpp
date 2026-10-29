// AV13-07 — La ruta por casillas (Game::RutaCasillas): muro, rodeo, sin
// camino, presupuesto justo, terreno dificil y ocupadas.
#include "Game/RutaCasillas.h"
#include "TestHarness.h"

#include <chrono>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

// '#' muro, '~' terreno dificil (cuesta 2), 'o' ocupada; lo demas, suelo.
struct Plano {
    std::vector<std::string> f;
    char en(int x, int y) const { return f[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)]; }
    Game::RutaCasillas ruta() const {
        return Game::RutaCasillas({0, 0}, static_cast<int>(f[0].size()), static_cast<int>(f.size()),
                                  [this](int x, int y) { return en(x, y) != '#' && en(x, y) != 'o'; },
                                  [this](int x, int y) { return en(x, y) == '~' ? 2 : 1; });
    }
};

}  // namespace

MOTOR_TEST_CASE(la_ruta_rodea_el_muro_y_no_entra_en_lo_ocupado) {
    const Plano p{{
        ".....",
        ".###.",
        ".#o..",
        ".....",
    }};
    const Game::RutaCasillas r = p.ruta();
    // De (0,2) a (3,2): el muro y la casilla ocupada obligan a bajar.
    const auto camino = r.ruta({0, 2}, {3, 2});
    MOTOR_CHECK(camino.size() == 5);
    MOTOR_CHECK(camino.back().x == 3 && camino.back().y == 2);
    MOTOR_CHECK(r.coste({0, 2}, {3, 2}) == 5);
    for (const GridCoord& c : camino) MOTOR_CHECK(p.en(c.x, c.y) == '.');
    // Encerrado: sin camino.
    const Plano q{{".#.", "##.", "..."}};
    MOTOR_CHECK(q.ruta().ruta({0, 0}, {2, 2}).empty() && q.ruta().coste({0, 0}, {2, 2}) == -1);
    MOTOR_CHECK(r.ruta({1, 0}, {1, 0}).empty() && r.coste({1, 0}, {1, 0}) == 0);
}

MOTOR_TEST_CASE(con_un_presupuesto_se_llega_hasta_donde_se_llega) {
    const Plano p{{
        ".......",
        ".......",
        ".......",
    }};
    const Game::RutaCasillas r = p.ruta();
    // Desde el centro (3,1) con 2: el rombo de radio 2 recortado al plano.
    const auto a = r.alcanzables({3, 1}, 2);
    std::set<std::pair<int, int>> en;
    for (const auto& c : a) {
        en.insert({c.casilla.x, c.casilla.y});
        MOTOR_CHECK(c.coste >= 1 && c.coste <= 2);
    }
    MOTOR_CHECK(a.size() == 10);  // 5 en su fila (sin ella) y 3 arriba y 3 abajo, menos la de salida
    MOTOR_CHECK(en.count({1, 1}) && en.count({5, 1}) && en.count({2, 0}) && !en.count({3, 1}) && !en.count({1, 0}));
    MOTOR_CHECK(r.alcanzables({3, 1}, 0).empty());
}

MOTOR_TEST_CASE(el_terreno_dificil_cuesta_el_doble) {
    const Plano p{{
        "..~..",
        "..~..",
        ".....",
    }};
    const Game::RutaCasillas r = p.ruta();
    // Cruzar el barro: 1 + 2 + 1 + 1 = 5; rodearlo por abajo cuesta 8: gana el barro.
    MOTOR_CHECK(r.coste({0, 0}, {4, 0}) == 5);
    // Con 3 no se llega a (3,0) cruzando el barro (1 + 2 + 1 = 4).
    std::set<std::pair<int, int>> en;
    for (const auto& c : r.alcanzables({0, 0}, 3)) en.insert({c.casilla.x, c.casilla.y});
    MOTOR_CHECK(en.count({2, 0}) && !en.count({3, 0}));
}

MOTOR_TEST_CASE(cien_por_cien_casillas_en_menos_de_cinco_ms) {
    std::vector<std::string> f(100, std::string(100, '.'));
    for (int y = 10; y < 90; ++y) f[static_cast<std::size_t>(y)][50] = '#';
    const Plano p{f};
    const Game::RutaCasillas r = p.ruta();
    const auto t0 = std::chrono::steady_clock::now();
    const auto camino = r.ruta({0, 50}, {99, 50});
    const auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    MOTOR_CHECK(!camino.empty());
    MOTOR_CHECK(ms < 5.0);
}

int main() { return motor_test::runAllTests(); }
