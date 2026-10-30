// H2-01 (Gantt Fase H) — el dado se ve: el widget ensena caras, exitos,
// grado y marca de pifia.
//
// Dos mitades, a proposito:
//
//   1. Casos con PoolResult ESCRITO A MANO: las reglas de presentacion
//      (que caras salen, cuando hay marca de pifia, que pone sin dados)
//      sobre un resultado cuyo contenido decide el test, no el azar.
//   2. Un caso de INTEGRACION con el motor real (DicePoolEngine::roll_pool
//      con Xoroshiro128p + opposed_degree): el grado que ensena el widget
//      es el que calcula el reglamento, no una copia escrita dos veces.
//
// GL-free: sin ventana, sin assets, sin copiar_assets.
#include "TestHarness.h"

#include "HUD/DiceRollWidget.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/RandomEngine.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

using Game::DiceRollWidget;
using RPG::Degree;
using RPG::DicePoolEngine;
using RPG::PoolResult;
using RPG::Xoroshiro128p;

namespace {

PoolResult poolCon(const std::vector<int>& caras, float exitos, bool pifia) {
    PoolResult r;
    r.rolls = caras;
    r.successes = exitos;
    r.critical = false;
    r.botch = pifia;
    return r;
}

bool contiene(const std::vector<std::string>& lineas, const std::string& trozo) {
    for (const std::string& l : lineas) {
        if (l.find(trozo) != std::string::npos) {
            return true;
        }
    }
    return false;
}

}  // namespace

MOTOR_TEST_CASE(las_caras_se_ven_una_por_dado) {
    DiceRollWidget w;
    w.show(poolCon({6, 4, 1}, 1.0f, false), Degree::SUCCESS, "Ataque: tu tirada");
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(w.visible());
    MOTOR_CHECK(lineas.size() == 4);
    MOTOR_CHECK(contiene(lineas, "[6]"));
    MOTOR_CHECK(contiene(lineas, "[4]"));
    MOTOR_CHECK(contiene(lineas, "[1]"));
    MOTOR_CHECK(!contiene(lineas, "[2]"));
}

MOTOR_TEST_CASE(los_exitos_salen_con_su_valor) {
    DiceRollWidget w;
    w.show(poolCon({5, 5, 2}, 1.0f, false), Degree::PARTIAL, "Prueba");
    MOTOR_CHECK(contiene(w.displayLines(), "Exitos: 1"));
    w.show(poolCon({5, 3, 2}, 0.5f, false), Degree::PARTIAL, "Prueba");
    MOTOR_CHECK(contiene(w.displayLines(), "Exitos: 0.5"));
}

MOTOR_TEST_CASE(los_cuatro_grados_tienen_etiqueta_distinta) {
    MOTOR_CHECK(std::string(DiceRollWidget::gradeLabel(Degree::BOTCH)) == "Fracàs");
    MOTOR_CHECK(std::string(DiceRollWidget::gradeLabel(Degree::PARTIAL)) == "Parcial");
    MOTOR_CHECK(std::string(DiceRollWidget::gradeLabel(Degree::SUCCESS)) == "Èxit");
    MOTOR_CHECK(std::string(DiceRollWidget::gradeLabel(Degree::CRITICAL)) == "Crític");
    DiceRollWidget w;
    w.show(poolCon({6}, 1.0f, false), Degree::SUCCESS, "X");
    MOTOR_CHECK(contiene(w.displayLines(), "Grado: Èxit"));
}

MOTOR_TEST_CASE(la_pifia_se_distingue_a_simple_vista) {
    DiceRollWidget w;
    // Pifia Ed. 3: ningun 5/6 y minimo dos 1es.
    w.show(poolCon({1, 1, 3}, 0.0f, true), Degree::BOTCH, "Ataque: tu tirada");
    MOTOR_CHECK(w.showsBotch());
    MOTOR_CHECK(contiene(w.displayLines(), "!! PIFIA !!"));
}

MOTOR_TEST_CASE(un_botch_sin_pifia_no_lleva_la_marca) {
    DiceRollWidget w;
    // BOTCH por derrota en la enfrentada (el defensor supera): el grado es
    // Fracàs pero NO hubo pifia — la marca distingue, no el grado.
    w.show(poolCon({4, 3, 2}, 0.0f, false), Degree::BOTCH, "Ataque: tu tirada");
    MOTOR_CHECK(!w.showsBotch());
    MOTOR_CHECK(!contiene(w.displayLines(), "PIFIA"));
    MOTOR_CHECK(contiene(w.displayLines(), "Grado: Fracàs"));
}

MOTOR_TEST_CASE(un_critico_no_es_pifia) {
    PoolResult r;
    r.rolls = {6, 6, 6};
    r.successes = 3.0f;
    r.critical = true;
    r.botch = false;
    DiceRollWidget w;
    w.show(r, Degree::CRITICAL, "Hechizo");
    MOTOR_CHECK(!w.showsBotch());
    MOTOR_CHECK(contiene(w.displayLines(), "Grado: Crític"));
}

MOTOR_TEST_CASE(sin_dados_no_revienta_y_lo_dice) {
    PoolResult r;  // pool vacio: 0 dados por una condicion que resta.
    DiceRollWidget w;
    w.show(r, Degree::PARTIAL, "Defensa");
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 4);
    MOTOR_CHECK(contiene(lineas, "(sin dados)"));
    MOTOR_CHECK(!w.showsBotch());
}

MOTOR_TEST_CASE(la_etiqueta_es_la_primera_linea) {
    DiceRollWidget w;
    w.show(poolCon({6}, 1.0f, false), Degree::SUCCESS, "Hablar: Percepción");
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(!lineas.empty());
    MOTOR_CHECK(lineas[0] == "Hablar: Percepción");
}

MOTOR_TEST_CASE(mostrar_dos_veces_sustituye_no_acumula) {
    DiceRollWidget w;
    w.show(poolCon({6, 6}, 2.0f, false), Degree::SUCCESS, "Primera");
    w.show(poolCon({1}, 0.0f, false), Degree::BOTCH, "Segunda");
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 4);
    MOTOR_CHECK(lineas[0] == "Segunda");
    MOTOR_CHECK(!contiene(lineas, "[6]"));
}

MOTOR_TEST_CASE(ocultar_deja_el_widget_vacio) {
    DiceRollWidget w;
    MOTOR_CHECK(!w.visible());
    MOTOR_CHECK(w.displayLines().empty());
    w.show(poolCon({6}, 1.0f, false), Degree::SUCCESS, "X");
    MOTOR_CHECK(w.visible());
    w.hide();
    MOTOR_CHECK(!w.visible());
    MOTOR_CHECK(w.displayLines().empty());
    MOTOR_CHECK(!w.showsBotch());
}

MOTOR_TEST_CASE(el_grado_que_ensena_lo_calcula_el_reglamento) {
    // Integracion: la tirada la hace el motor de verdad con semilla fija y
    // el grado lo decide opposed_degree; el widget solo lo ensena. Valores
    // medidos (semilla 1234: [4,3,5] -> 0.5 exitos) y fijados: si el motor
    // cambia su aritmetica, este caso lo dice.
    Xoroshiro128p rng(1234);
    const PoolResult atacante = DicePoolEngine::roll_pool(3, RPG::DiceMod::NORMAL, rng);
    MOTOR_CHECK(atacante.rolls.size() == 3);
    MOTOR_CHECK(atacante.rolls[0] == 4);
    MOTOR_CHECK(atacante.rolls[1] == 3);
    MOTOR_CHECK(atacante.rolls[2] == 5);
    PoolResult defensor;
    defensor.rolls = {1, 2, 3};
    defensor.successes = 0.0f;
    defensor.botch = false;
    const Degree grado = DicePoolEngine::opposed_degree(atacante, defensor);
    MOTOR_CHECK(grado == Degree::SUCCESS);

    DiceRollWidget w;
    w.show(atacante, grado, "Ataque: tu tirada");
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 4);
    MOTOR_CHECK(contiene(lineas, "Exitos: 0.5"));
    MOTOR_CHECK(contiene(lineas, "Grado: Èxit"));
    MOTOR_CHECK(!w.showsBotch());
}

// D0013 — el crítico son dos 6: con tres dados, dos 6 y un 1 la tirada es
// crítica y el widget lo enseña; con un solo 6, no. Las caras las fija el
// test (un RandomEngine que devuelve lo que se le dice), el grado lo
// decide el motor.
namespace {
class CarasFijas : public RPG::RandomEngine {
public:
    explicit CarasFijas(std::vector<int> caras) : m_caras(std::move(caras)) {}
    int roll_d6() override { return m_caras.empty() ? 1 : m_caras[m_siguiente++ % m_caras.size()]; }
    int roll_in_range(int a, int) override { return a; }
    uint64_t seed() const override { return 0; }

private:
    std::vector<int> m_caras;
    std::size_t m_siguiente = 0;
};
}  // namespace

MOTOR_TEST_CASE(dos_seises_de_tres_dados_se_ven_como_critico) {
    PoolResult defensor;  // sin dados: no pifia ni suma
    CarasFijas dosSeis({6, 1, 6});
    const PoolResult critico = DicePoolEngine::roll_pool(3, RPG::DiceMod::NORMAL, dosSeis);
    MOTOR_CHECK(critico.critical);
    DiceRollWidget w;
    w.show(critico, DicePoolEngine::opposed_degree(critico, defensor), "Ataque: tu tirada");
    MOTOR_CHECK(contiene(w.displayLines(), "Grado: Crític"));

    CarasFijas unSeis({6, 1, 5});
    const PoolResult normal = DicePoolEngine::roll_pool(3, RPG::DiceMod::NORMAL, unSeis);
    MOTOR_CHECK(!normal.critical);
    w.show(normal, DicePoolEngine::opposed_degree(normal, defensor), "Ataque: tu tirada");
    MOTOR_CHECK(contiene(w.displayLines(), "Grado: Èxit"));
}

int main() { return motor_test::runAllTests(); }
