// AV13-06 — El tablero del combate (Game::TableroCombate): el recuadro de
// casillas del nivel, quien esta en cada una, la distancia y la linea de vision.
#include "Game/TableroCombate.h"
#include "TestHarness.h"

#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {

// Un nivel de mentira dibujado: '#' bloquea; cualquier otra cosa se pisa.
struct Nivel {
    std::vector<std::string> filas;
    int ancho() const { return filas.empty() ? 0 : static_cast<int>(filas[0].size()); }
    int alto() const { return static_cast<int>(filas.size()); }
    Game::TableroCombate::Bloquea bloquea() const {
        return [f = filas](int x, int y) {
            if (y < 0 || y >= static_cast<int>(f.size()) || x < 0 || x >= static_cast<int>(f[0].size())) return true;
            return f[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] == '#';
        };
    }
};

Nivel salaConColumna() {
    return Nivel{{
        "..........",
        "..........",
        "....#.....",
        "....#.....",
        "..........",
        "..........",
    }};
}

}  // namespace

MOTOR_TEST_CASE(el_recuadro_se_centra_en_la_pelea_y_cabe_en_el_nivel) {
    // Un nivel de 40x30 y dos que pelean en (20,10) y (22,12): 15x15 centrado (el juego usa kLado = 11).
    Game::TableroCombate t = Game::TableroCombate::alrededor({{20, 10}, {22, 12}}, 40, 30, nullptr, 15);
    MOTOR_CHECK(t.ancho() == 15 && t.alto() == 15);
    MOTOR_CHECK(t.origen().x == 14 && t.origen().y == 4);
    MOTOR_CHECK(t.dentro({20, 10}) && t.dentro({22, 12}));
    MOTOR_CHECK(!t.dentro({13, 10}) && t.dentro({14, 10}) && t.dentro({28, 18}) && !t.dentro({29, 18}));
    // En una esquina del nivel, se desplaza (no se recorta): sigue siendo 15x15.
    Game::TableroCombate e = Game::TableroCombate::alrededor({{1, 1}}, 40, 30, nullptr, 15);
    MOTOR_CHECK(e.origen().x == 0 && e.origen().y == 0 && e.ancho() == 15 && e.alto() == 15);
    // Junto al borde derecho del nivel (64 de ancho, como ciudad_centro): el de 11 se corre a la izquierda.
    Game::TableroCombate d = Game::TableroCombate::alrededor({{62, 32}, {62, 33}}, 64, 64, nullptr);
    MOTOR_CHECK(d.ancho() == Game::TableroCombate::kLado && d.origen().x == 64 - Game::TableroCombate::kLado);
    // Un nivel mas pequeno que el recuadro: el nivel entero.
    const Nivel n = salaConColumna();
    Game::TableroCombate s = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    MOTOR_CHECK(s.ancho() == 10 && s.alto() == 6 && s.origen().x == 0);
}

MOTOR_TEST_CASE(quien_esta_donde_y_las_casillas_libres) {
    const Nivel n = salaConColumna();
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    MOTOR_CHECK(t.transitable({0, 0}) && !t.transitable({4, 2}) && !t.transitable({-1, 0}));
    MOTOR_CHECK(t.pon("Heroe", {2, 2}));
    MOTOR_CHECK(!t.pon("Saqueador 1", {2, 2}));  // ocupada
    MOTOR_CHECK(!t.pon("Saqueador 1", {4, 3}));  // la columna
    MOTOR_CHECK(t.pon("Saqueador 1", {5, 2}));
    MOTOR_CHECK(t.quienEsta({2, 2}) != nullptr && *t.quienEsta({2, 2}) == "Heroe");
    MOTOR_CHECK(!t.libre({2, 2}) && t.libre({3, 2}));
    MOTOR_CHECK(t.mueve("Heroe", {3, 2}) && t.libre({2, 2}));
    MOTOR_CHECK(!t.mueve("Heroe", {5, 2}));  // la del saqueador
    MOTOR_CHECK(!t.mueve("Nadie", {0, 0}));
    t.quita("Saqueador 1");
    MOTOR_CHECK(!t.dondeEsta("Saqueador 1") && t.libre({5, 2}) && t.ocupantes().size() == 1);
}

MOTOR_TEST_CASE(distancia_con_diagonal_y_linea_de_vision_que_corta_la_columna) {
    MOTOR_CHECK(Game::TableroCombate::distancia({0, 0}, {3, 1}) == 3);
    MOTOR_CHECK(Game::TableroCombate::alLado({2, 2}, {3, 3}) && !Game::TableroCombate::alLado({2, 2}, {4, 2}));
    const Nivel n = salaConColumna();
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    MOTOR_CHECK(!t.seVen({2, 2}, {7, 3}));  // la columna (4,2)-(4,3) en medio
    MOTOR_CHECK(t.seVen({2, 0}, {7, 0}));   // por encima de la columna
    MOTOR_CHECK(t.seVen({2, 5}, {7, 4}));   // por debajo
    MOTOR_CHECK(t.seVen({3, 2}, {4, 2}));   // al lado de la columna: los extremos no tapan
    MOTOR_CHECK(t.seVen({7, 3}, {7, 3}));
}

MOTOR_TEST_CASE(la_libre_mas_cercana_y_el_borde_por_donde_se_entra) {
    const Nivel n = salaConColumna();
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    MOTOR_CHECK(t.pon("Heroe", {3, 2}));
    // Junto al heroe: la primera libre del anillo 1 en orden de lectura.
    const auto junto = t.libreMasCercana({3, 2});
    MOTOR_CHECK(junto && junto->x == 2 && junto->y == 1);
    // Desde la columna, que no se pisa: el anillo 1 empieza en (3,1).
    const auto columna = t.libreMasCercana({4, 2});
    MOTOR_CHECK(columna && columna->x == 3 && columna->y == 1);
    // El borde del nivel de 10x6: 10+10+4+4 casillas, todas pisables.
    const std::vector<GridCoord> b = t.borde();
    MOTOR_CHECK(b.size() == 28);
    std::set<std::pair<int, int>> vistos;
    for (const GridCoord& c : b) vistos.insert({c.x, c.y});
    MOTOR_CHECK(vistos.count({0, 0}) && vistos.count({9, 5}) && !vistos.count({4, 2}) && !vistos.count({1, 1}));
}

// AV13-12 / AV16-12 — Moverse: cuatro casillas por ronda (+1 con DES 3), la ruta se gasta, y el enemigo
// se acerca hasta quedar al lado (o lo mas cerca que puede).
MOTOR_TEST_CASE(andar_gasta_los_pasos_de_la_ronda) {
    const Nivel n = salaConColumna();
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    MOTOR_CHECK(t.pon("Heroe", {0, 0}));
    MOTOR_CHECK(t.pasos("Heroe") == Game::TableroCombate::kPasos);
    MOTOR_CHECK(t.rutaHasta("Heroe", {9, 5}).empty());  // 14 casillas: no llega
    MOTOR_CHECK(t.rutaHasta("Heroe", {3, 2}).empty());  // 5 casillas: no llega con 4
    const auto r = t.anda("Heroe", {2, 1});
    MOTOR_CHECK(r.size() == 3 && t.pasos("Heroe") == 1);
    MOTOR_CHECK(t.dondeEsta("Heroe")->x == 2 && t.dondeEsta("Heroe")->y == 1);
    MOTOR_CHECK(t.anda("Heroe", {2, 5}).empty());  // 4 mas: no le quedan
    MOTOR_CHECK(t.alcanzables("Heroe").size() == 4);  // con 1: los cuatro vecinos
    t.reponPasos();
    MOTOR_CHECK(t.pasos("Heroe") == 4 && !t.anda("Heroe", {2, 5}).empty());
}

// AV16-12 — El bono de destreza: DES 3 o mas = una casilla mas cada ronda.
MOTOR_TEST_CASE(con_destreza_tres_se_anda_una_casilla_mas) {
    MOTOR_CHECK(Game::TableroCombate::kPasos == 4);
    MOTOR_CHECK(Game::TableroCombate::bonoDeDestreza(2) == 0 && Game::TableroCombate::bonoDeDestreza(3) == 1);
    MOTOR_CHECK(Game::TableroCombate::bonoDeDestreza(0) == 0 && Game::TableroCombate::bonoDeDestreza(8) == 1);
    const Nivel n = salaConColumna();
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    MOTOR_CHECK(t.pon("Lento", {0, 0}) && t.pon("Agil", {0, 1}));
    t.ponBono("Agil", Game::TableroCombate::bonoDeDestreza(3));
    MOTOR_CHECK(t.pasos("Lento") == 4 && t.pasos("Agil") == 5 && t.pasosMax("Agil") == 5);
    MOTOR_CHECK(!t.anda("Agil", {5, 1}).empty() && t.pasos("Agil") == 0);  // 5 casillas
    MOTOR_CHECK(t.rutaHasta("Lento", {5, 0}).empty());                       // 5 casillas: no llega con 4
    t.reponPasos();
    MOTOR_CHECK(t.pasos("Agil") == 5);  // la ronda nueva repone el bono tambien
}

// AV16-14 — Congelado anda la mitad de los pasos; aturdido, ninguno. Se pregunta cada vez.
MOTOR_TEST_CASE(congelado_anda_la_mitad_y_aturdido_no_anda) {
    const Nivel n = salaConColumna();
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    MOTOR_CHECK(t.pon("Heroe", {0, 0}) && t.pon("Lobo", {0, 1}));
    t.ponBono("Heroe", 1);  // 5 pasos
    int freno = 0;
    t.ponVelocidad([&freno](const std::string& quien) { return quien == "Heroe" ? freno : 0; });
    MOTOR_CHECK(t.pasosMax("Heroe") == 5 && t.pasos("Heroe") == 5);
    freno = 1;  // congelado: 5/2 = 2
    MOTOR_CHECK(t.pasosMax("Heroe") == 2 && t.pasos("Heroe") == 2 && t.pasos("Lobo") == 4);
    MOTOR_CHECK(t.rutaHasta("Heroe", {3, 0}).empty() && !t.rutaHasta("Heroe", {2, 0}).empty());
    freno = 2;  // aturdido
    MOTOR_CHECK(t.pasos("Heroe") == 0 && t.rutaHasta("Heroe", {1, 0}).empty());
    freno = 0;  // se le quita: vuelve a andar
    MOTOR_CHECK(t.pasos("Heroe") == 5);
}

// AV19-02 — Subir un nivel de altura cuesta el doble de esfuerzo: 2 pasos.
MOTOR_TEST_CASE(subir_un_nivel_cuesta_el_doble_y_bajar_lo_normal) {
    const Nivel n = salaConColumna();
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    t.ponNiveles([](int x, int) { return x >= 3 ? 1 : 0; });  // a partir de x=3, un nivel mas arriba
    MOTOR_CHECK(t.pon("Heroe", {0, 0}));
    t.ponBono("Heroe", 2);  // 6 pasos, para que el escalon se note
    MOTOR_CHECK(t.costeDePaso({2, 0}, {3, 0}) == 2 && t.costeDePaso({3, 0}, {2, 0}) == 1);
    MOTOR_CHECK(t.rutaHasta("Heroe", {5, 0}).size() == 5);  // 5 casillas = 4 + el escalon (2) = 6: llega justo
    MOTOR_CHECK(t.rutaHasta("Heroe", {5, 1}).empty());       // 6 casillas + escalon = 7: no llega
    const auto r = t.anda("Heroe", {5, 0});
    MOTOR_CHECK(r.size() == 5 && t.pasos("Heroe") == 0);
    t.reponPasos();
    MOTOR_CHECK(!t.anda("Heroe", {0, 0}).empty() && t.pasos("Heroe") == 1);  // bajar: 5 casillas, 5 pasos
}

MOTOR_TEST_CASE(el_enemigo_se_acerca_y_solo_pega_a_tiro) {
    const Nivel n = salaConColumna();
    Game::TableroCombate t = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    MOTOR_CHECK(t.pon("Heroe", {1, 2}) && t.pon("Lobo", {5, 2}));
    MOTOR_CHECK(!t.aTiro("Lobo", "Heroe", 1));
    // Cuerpo a cuerpo: 4 de distancia y la columna en medio; la rodea por arriba y queda al lado.
    std::vector<GridCoord> ruta;
    MOTOR_CHECK(t.acerca("Lobo", {"Heroe"}, 1, &ruta));
    MOTOR_CHECK(t.aTiro("Lobo", "Heroe", 1) && !ruta.empty() && static_cast<int>(ruta.size()) <= 4);
    // Ya al lado: no se mueve.
    const GridCoord antes = *t.dondeEsta("Lobo");
    MOTOR_CHECK(t.acerca("Lobo", {"Heroe"}, 1));
    MOTOR_CHECK(t.dondeEsta("Lobo")->x == antes.x && t.dondeEsta("Lobo")->y == antes.y);
    // Un tirador con alcance 5 no se acerca de mas: ya llega viendole.
    Game::TableroCombate u = Game::TableroCombate::alrededor({{2, 2}}, n.ancho(), n.alto(), n.bloquea());
    MOTOR_CHECK(u.pon("Heroe", {1, 0}) && u.pon("Arquero", {6, 0}));
    MOTOR_CHECK(u.acerca("Arquero", {"Heroe"}, 5) && u.dondeEsta("Arquero")->x == 6);
    // Lejos de verdad (no llega con sus pasos): se acerca lo que puede y no pega.
    Game::TableroCombate v = Game::TableroCombate::alrededor({{0, 0}}, 30, 3, nullptr, 30);
    MOTOR_CHECK(v.pon("Heroe", {0, 1}) && v.pon("Lobo", {20, 1}));
    MOTOR_CHECK(!v.acerca("Lobo", {"Heroe"}, 1) && v.dondeEsta("Lobo")->x == 16);
}

int main() { return motor_test::runAllTests(); }
