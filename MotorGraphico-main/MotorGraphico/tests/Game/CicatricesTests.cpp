// AV13-33 — Las cicatrices sobreviven a cargar la partida (Game::Cicatrices).
#include "Game/Cicatrices.h"
#include "TestHarness.h"

#include <cstdio>
#include <fstream>

MOTOR_TEST_CASE(las_cicatrices_se_suman_se_guardan_y_se_leen) {
    const std::string ruta = "test_cicatrices_tmp/cicatrices.txt";
    std::remove(ruta.c_str());
    Game::Cicatrices c = Game::Cicatrices::carga(ruta);  // no existe: ninguna
    MOTOR_CHECK(c.de("sulivan") == 0);
    MOTOR_CHECK(c.suma("sulivan") == 1);
    MOTOR_CHECK(c.suma("sulivan") == 2);
    MOTOR_CHECK(c.suma("venides") == 1);
    MOTOR_CHECK(c.guarda(ruta));
    const Game::Cicatrices leidas = Game::Cicatrices::carga(ruta);
    MOTOR_CHECK(leidas.de("sulivan") == 2 && leidas.de("venides") == 1 && leidas.de("nadie") == 0);
    MOTOR_CHECK(Game::Cicatrices::nota(1).find("una cicatriz") != std::string::npos);
    MOTOR_CHECK(Game::Cicatrices::nota(3).find("ya llevas 3") != std::string::npos);
    std::remove(ruta.c_str());
}

MOTOR_TEST_CASE(una_linea_rota_no_tumba_las_demas) {
    const std::string ruta = "test_cicatrices_tmp/rota.txt";
    {
        std::ofstream f(ruta);
        f << "sulivan\t2\nbasura sin tab\nxila\tdos\nvenides\t1\n";
    }
    const Game::Cicatrices c = Game::Cicatrices::carga(ruta);
    MOTOR_CHECK(c.de("sulivan") == 2 && c.de("venides") == 1 && c.de("xila") == 0);
    std::remove(ruta.c_str());
}

int main() { return motor_test::runAllTests(); }
