// AV13-11 — El orden de turno por iniciativa (Game::OrdenDeTurno).
#include "Game/OrdenDeTurno.h"
#include "RPG/RandomEngine.h"
#include "TestHarness.h"

#include <vector>

namespace {

// Saca los dados que se le dicen, en orden.
class Dados : public RPG::RandomEngine {
public:
    explicit Dados(std::vector<int> v) : m_v(std::move(v)) {}
    int roll_d6() override { return m_v[m_i++ % m_v.size()]; }
    int roll_in_range(int, int) override { return roll_d6(); }
    uint64_t seed() const override { return 0; }

private:
    std::vector<int> m_v;
    std::size_t m_i = 0;
};

}  // namespace

MOTOR_TEST_CASE(la_iniciativa_ordena_de_mayor_a_menor_con_la_des) {
    // Heroe 3+1 = 4; enemigo 0: 2+2 = 4 (gana por bono); enemigo 1: 6+0 = 6.
    Dados d({3, 2, 6});
    Game::OrdenDeTurno o;
    o.tira(1, {2, 0}, d);
    MOTOR_CHECK(o.puestos().size() == 3);
    MOTOR_CHECK(o.puestos()[0].enemigo && o.puestos()[0].i == 1);
    MOTOR_CHECK(o.puestos()[1].enemigo && o.puestos()[1].i == 0);
    MOTOR_CHECK(!o.puestos()[2].enemigo);
    MOTOR_CHECK(o.quien() == &o.puestos()[0]);
    MOTOR_CHECK(o.resumen("Sulivan", {"Lobo", "Cuervo"}) == "Cuervo 6, Lobo 4, Sulivan 4.");
}

MOTOR_TEST_CASE(en_empate_total_el_heroe_va_antes_y_luego_el_orden_de_la_batalla) {
    Dados d({4});
    Game::OrdenDeTurno o;
    o.tira(1, {1, 1}, d);
    MOTOR_CHECK(!o.puestos()[0].enemigo);
    MOTOR_CHECK(o.puestos()[1].i == 0 && o.puestos()[2].i == 1);
}

MOTOR_TEST_CASE(avanzar_salta_a_los_caidos_y_dice_cuando_hay_ronda_nueva) {
    Dados d({6, 5, 4});  // heroe, enemigo 0, enemigo 1
    Game::OrdenDeTurno o;
    o.tira(0, {0, 0}, d);
    bool enemigo0Vivo = true;
    const auto vivo = [&](const Game::Puesto& p) { return !(p.enemigo && p.i == 0) || enemigo0Vivo; };
    MOTOR_CHECK(!o.avanza(vivo));  // heroe -> enemigo 0
    MOTOR_CHECK(o.quien()->enemigo && o.quien()->i == 0);
    MOTOR_CHECK(!o.avanza(vivo));  // -> enemigo 1
    MOTOR_CHECK(o.avanza(vivo));   // -> heroe: ronda nueva
    MOTOR_CHECK(!o.quien()->enemigo);
    enemigo0Vivo = false;
    MOTOR_CHECK(!o.avanza(vivo));  // el enemigo 0 ha caido: al enemigo 1
    MOTOR_CHECK(o.quien()->i == 1);
    // Si cae el que tiene el turno, saltaCaidos pasa al siguiente.
    const auto soloHeroe = [](const Game::Puesto& p) { return !p.enemigo; };
    MOTOR_CHECK(o.saltaCaidos(soloHeroe));
    MOTOR_CHECK(!o.quien()->enemigo);
    MOTOR_CHECK(!o.saltaCaidos(soloHeroe));
}

int main() { return motor_test::runAllTests(); }
