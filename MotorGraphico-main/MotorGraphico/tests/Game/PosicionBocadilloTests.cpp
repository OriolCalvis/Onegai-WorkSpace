// C2-04d — test_posicion_bocadillo: la formula de pantalla del bocadillo (sin GL).
#include <cmath>

#include "Game/PosicionBocadillo.h"
#include "TestHarness.h"

using namespace Game;

MOTOR_TEST_CASE(centro_de_pantalla) {
    // Pies justo en el centro de la camara: caen en el centro de la vista.
    const PosicionBocadillo r = posicionBocadillo(100, 50, 100, 50, 1.0f, 1280, 720, 32, 30);
    MOTOR_CHECK(r.x == 640.0f && r.pies == 360.0f);
    MOTOR_CHECK(r.x0 == 624.0f);                 // x - w/2
    MOTOR_CHECK(r.y0 == 360.0f - 58.0f - 30.0f - 8.0f);  // 264
    MOTOR_CHECK(r.visible);
}

MOTOR_TEST_CASE(zoom_dos_escala_la_distancia_y_la_altura) {
    const PosicionBocadillo r = posicionBocadillo(110, 60, 100, 50, 2.0f, 1280, 720, 32, 30);
    MOTOR_CHECK(r.x == 660.0f && r.pies == 380.0f);  // 10 px de mundo = 20 de pantalla
    MOTOR_CHECK(r.y0 == 380.0f - 116.0f - 30.0f - 8.0f);  // la cabeza sube 58 * zoom
    MOTOR_CHECK(r.visible);
}

MOTOR_TEST_CASE(fuera_de_pantalla_es_invisible) {
    MOTOR_CHECK(!posicionBocadillo(5000, 50, 100, 50, 1.0f, 1280, 720, 32, 30).visible);   // derecha
    MOTOR_CHECK(!posicionBocadillo(-5000, 50, 100, 50, 1.0f, 1280, 720, 32, 30).visible);  // izquierda
    MOTOR_CHECK(!posicionBocadillo(100, 5000, 100, 50, 1.0f, 1280, 720, 32, 30).visible);  // abajo
    MOTOR_CHECK(!posicionBocadillo(100, -5000, 100, 50, 1.0f, 1280, 720, 32, 30).visible); // arriba
    // El borde: un retal que asoma solo un poco aun se pinta.
    MOTOR_CHECK(posicionBocadillo(100 + 650, 50, 100, 50, 1.0f, 1280, 720, 32, 30).visible);
}

int main() { return motor_test::runAllTests(); }
