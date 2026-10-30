// AV13-33 — La escena de noqueo o rescate antes de «Has caído» (Game::EscenaNoqueo).
#include "Game/EscenaNoqueo.h"
#include "TestHarness.h"

MOTOR_TEST_CASE(sin_nadie_en_pie_es_un_noqueo_y_con_alguien_un_rescate) {
    const Game::EscenaNoqueo solo = Game::EscenaNoqueo::para("");
    MOTOR_CHECK(!solo.rescate() && solo.lineas().size() == 2);
    const Game::EscenaNoqueo venides = Game::EscenaNoqueo::para("Venides");
    MOTOR_CHECK(venides.rescate() && venides.lineas().size() == 3);
    MOTOR_CHECK(venides.lineas()[1].find("Venides") == 0);
}

MOTOR_TEST_CASE(cae_se_oscurece_lee_las_frases_y_acaba_en_negro) {
    Game::EscenaNoqueo e = Game::EscenaNoqueo::para("");
    MOTOR_CHECK(e.linea().empty() && e.caida() == 0.0f && e.velo() == 0.0f);
    e.avanza(Game::EscenaNoqueo::kCaida * 0.5f);
    MOTOR_CHECK(e.caida() > 0.4f && e.caida() < 0.6f && e.linea().empty());
    e.avanza(Game::EscenaNoqueo::kCaida * 0.5f + 1.0f);  // en mitad de la primera frase
    MOTOR_CHECK(e.caida() == 1.0f && e.linea() == e.lineas()[0] && e.alfaLinea() == 1.0f);
    MOTOR_CHECK(e.velo() > 0.55f && e.velo() < 0.8f);
    e.avanza(Game::EscenaNoqueo::kLinea);
    MOTOR_CHECK(e.linea() == e.lineas()[1]);
    MOTOR_CHECK(!e.acabada());
    e.avanza(Game::EscenaNoqueo::kLinea + Game::EscenaNoqueo::kFundido);
    MOTOR_CHECK(e.linea().empty() && e.velo() > 0.999f);
    e.avanza(0.01f);
    MOTOR_CHECK(e.acabada());
}

MOTOR_TEST_CASE(enter_pasa_a_la_frase_siguiente_y_al_final_acaba) {
    Game::EscenaNoqueo e = Game::EscenaNoqueo::para("Xila");
    e.salta();  // en la caida: a la primera frase
    MOTOR_CHECK(e.linea() == e.lineas()[0]);
    e.salta();
    MOTOR_CHECK(e.linea() == e.lineas()[1]);
    e.salta();
    MOTOR_CHECK(e.linea() == e.lineas()[2]);
    e.salta();  // tras la ultima: el fundido
    MOTOR_CHECK(e.linea().empty() && !e.acabada());
    e.salta();
    MOTOR_CHECK(e.acabada());
}

int main() { return motor_test::runAllTests(); }
