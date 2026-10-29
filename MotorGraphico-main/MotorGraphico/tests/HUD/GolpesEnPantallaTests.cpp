// COMBATE-01 — Los numeros de los golpes: se quedan pegados a quien los recibe
// aunque la camara se mueva, no se pisan y se apagan al segundo (AV14-02: el
// destello rojo de los bordes se ha ido; lo que se tine es tu hoja).
#include "HUD/Pergamino/GolpesEnPantalla.h"
#include "TestHarness.h"

using Pergamino::GolpesEnPantalla;

MOTOR_TEST_CASE(golpes_saltan_no_se_pisan_y_se_apagan) {
    GolpesEnPantalla g;
    g.lanza(Vector2{100.0f, 50.0f}, "-5", GolpesEnPantalla::Tipo::Dano);
    g.lanza(Vector2{100.0f, 50.0f}, "Fallo", GolpesEnPantalla::Tipo::Nada);
    MOTOR_CHECK(g.golpes().size() == 2);
    MOTOR_CHECK(g.golpes()[1].mundo.y < g.golpes()[0].mundo.y);  // el segundo, encima
    g.avanza(0.5f);
    MOTOR_CHECK(g.golpes().size() == 2);
    g.avanza(GolpesEnPantalla::kVida);
    MOTOR_CHECK(g.golpes().empty());
    g.lanza(Vector2{0.0f, 0.0f}, "+3", GolpesEnPantalla::Tipo::Cura);
    g.olvida();
    MOTOR_CHECK(g.golpes().empty());
}

int main() { return motor_test::runAllTests(); }
