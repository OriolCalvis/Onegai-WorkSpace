// AV13-10 — El borde del tablero en el suelo (Pergamino::SueloCombate): las
// aristas que dan fuera del recuadro, cortadas donde hay pared.
#include "HUD/Pergamino/SueloCombate.h"
#include "TestHarness.h"

MOTOR_TEST_CASE(el_borde_de_un_recuadro_libre_da_la_vuelta_entera) {
    Pergamino::SueloCombate s;
    s.pon({10, 20}, 3, 2, [](int, int) { return true; });
    MOTOR_CHECK(s.visible && s.esPisable(10, 20) && s.esPisable(12, 21) && !s.esPisable(13, 21));
    const auto a = s.aristas();
    MOTOR_CHECK(a.size() == 10);  // 3 + 3 por arriba y abajo, 2 + 2 por los lados
    // La primera: la casilla (10,20), su lado de arriba-derecha.
    MOTOR_CHECK(a[0].desde.x == 10 && a[0].desde.y == 20 && a[0].hasta.x == 11 && a[0].hasta.y == 20);
}

MOTOR_TEST_CASE(donde_hay_pared_el_borde_se_corta) {
    Pergamino::SueloCombate s;
    // La esquina de arriba a la izquierda es pared: pierde sus dos aristas.
    s.pon({0, 0}, 3, 3, [](int x, int y) { return !(x == 0 && y == 0); });
    MOTOR_CHECK(s.aristas().size() == 10);
    s.quita();
    MOTOR_CHECK(!s.visible);
}

int main() { return motor_test::runAllTests(); }
