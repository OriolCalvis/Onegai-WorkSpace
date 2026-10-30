// AV16-10 — Las capas de una celda de la segunda cuadricula (dual-grid).
#include "Game/Transiciones.h"
#include "TestHarness.h"

using Game::Transiciones;

namespace {
Transiciones tabla() {
    Transiciones t;
    t.pon({{1, 0}, {2, 1}, {3, 2}});  // 1 = agua, 2 = hierba, 3 = adoquin
    return t;
}
}  // namespace

MOTOR_TEST_CASE(transiciones_cuatro_esquinas_iguales_no_llevan_capa) {
    MOTOR_CHECK(tabla().capas({2, 2, 2, 2}).empty());
}

MOTOR_TEST_CASE(transiciones_una_esquina_distinta_dibuja_su_recorte_encima) {
    const auto c = tabla().capas({3, 2, 2, 2});  // el adoquin en la esquina de arriba
    MOTOR_CHECK(c.size() == 1 && c[0].fila == 2 && c[0].mascara == Transiciones::kArriba);
    const auto d = tabla().capas({2, 2, 3, 3});  // adoquin abajo-izquierda y abajo
    MOTOR_CHECK(d.size() == 1 && d[0].mascara == (Transiciones::kIzquierda | Transiciones::kAbajo));
}

MOTOR_TEST_CASE(transiciones_tres_suelos_van_de_abajo_arriba_y_el_alto_pisa_al_medio) {
    const auto c = tabla().capas({1, 2, 2, 3});  // agua, hierba, hierba, adoquin
    MOTOR_CHECK(c.size() == 2);
    MOTOR_CHECK(c[0].fila == 1 && c[0].mascara == (Transiciones::kDerecha | Transiciones::kIzquierda | Transiciones::kAbajo));
    MOTOR_CHECK(c[1].fila == 2 && c[1].mascara == Transiciones::kAbajo);
}

MOTOR_TEST_CASE(transiciones_un_suelo_sin_rango_deja_la_celda_dura) {
    MOTOR_CHECK(tabla().capas({1, 2, 99, 3}).empty());  // 99: un interior o un edificio
    MOTOR_CHECK(tabla().rango(99) == -1 && tabla().rango(3) == 2);
}

MOTOR_TEST_CASE(transiciones_el_json_real_se_carga) {
    Transiciones t;
    const auto r = t.carga("assets");
    MOTOR_CHECK(r.isOk());
    if (r.isOk() && r.value()) {
        MOTOR_CHECK(t.rango(17) >= 0 && t.rango(1) >= 0 && t.rango(14) >= 0);  // adoquin, pradera, rio
        MOTOR_CHECK(t.rango(17) > t.rango(1) && t.rango(1) > t.rango(14));
        MOTOR_CHECK(t.rango(20) == -1);  // marmol_real: interior, sin transiciones
    }
}

int main() { return motor_test::runAllTests(); }
