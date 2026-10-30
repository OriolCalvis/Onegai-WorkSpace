// EH-16 — El resultado de las tiradas, en palabras (Game::frases).
#include "Game/FrasesDeCombate.h"
#include "TestHarness.h"

using RPG::Degree;
namespace frases = Game::frases;

MOTOR_TEST_CASE(una_tirada_enfrentada_se_dice_en_palabras) {
    MOTOR_CHECK(frases::enfrentada(Degree::SUCCESS, 1.5f, 0.0f, false) == "el ataque (1,5) supera la defensa (0)");
    MOTOR_CHECK(frases::enfrentada(Degree::CRITICAL, 3.0f, 0.0f, false) == "crítico: el ataque (3) arrolla la defensa (0)");
    MOTOR_CHECK(frases::enfrentada(Degree::PARTIAL, 0.0f, 0.0f, false) == "el ataque (0) y la defensa (0) empatan");
    MOTOR_CHECK(frases::enfrentada(Degree::BOTCH, 0.0f, 1.0f, false) == "la defensa (1) para el ataque (0)");
    MOTOR_CHECK(frases::enfrentada(Degree::BOTCH, 0.0f, 1.0f, true) == "pifia: el ataque falla del todo");
}

MOTOR_TEST_CASE(sin_rival_sin_defensa_y_el_dano) {
    MOTOR_CHECK(frases::sinRival(Degree::SUCCESS, 2.0f, false) == "acierto (2 éxitos)");
    MOTOR_CHECK(frases::sinRival(Degree::SUCCESS, 1.0f, false) == "acierto (1 éxito)");
    MOTOR_CHECK(frases::sinDefensa(Degree::CRITICAL) == "no se defiende: crítico");
    MOTOR_CHECK(frases::dano(7, "2d6") == "7 de daño (2d6)");
    MOTOR_CHECK(frases::dano(1, "solo roza") == "1 de daño (solo roza)");
    MOTOR_CHECK(frases::dano(0, "") == "Sin daño");
    MOTOR_CHECK(frases::numero(0.5f) == "0,5");
}

int main() { return motor_test::runAllTests(); }
