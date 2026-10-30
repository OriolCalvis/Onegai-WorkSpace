// AV13-33 — test_pergamino: las cicatrices en la ficha del cuaderno.
#include <string>
#include <vector>

#include "HUD/Pergamino/PaginasJugador.h"
#include "TestHarness.h"

using namespace Pergamino;

MOTOR_TEST_CASE(cicatrices_sin_ninguna_no_hay_fila) {
    std::vector<DatoPagina> estado{{"Moral", "Neutral (0)"}};
    anotaCicatrices(estado, 0);
    anotaCicatrices(estado, -1);
    MOTOR_CHECK(estado.size() == 1);
}

MOTOR_TEST_CASE(cicatrices_con_alguna_la_fila_dice_cuantas) {
    std::vector<DatoPagina> estado{{"Moral", "Neutral (0)"}};
    anotaCicatrices(estado, 3);
    MOTOR_CHECK(estado.size() == 2);
    MOTOR_CHECK(estado.back().clave == "Cicatrices" && estado.back().valor == "3");
}
