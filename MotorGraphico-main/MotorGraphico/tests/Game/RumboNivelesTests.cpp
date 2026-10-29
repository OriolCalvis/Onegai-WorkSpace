// MAPA-01 — Por que puerta se va hacia el objetivo (Game::RumboNiveles), con
// un mundo de mentira: la ciudad este -> la plaza -> el mercado (dos puertas
// desde la plaza) y la herreria; un callejon sin salida.
#include "Game/RumboNiveles.h"
#include "TestHarness.h"

#include <map>

namespace {

Game::RumboNiveles mundo() {
    static const std::map<std::string, std::vector<std::string>> puertas = {
        {"este.json", {"assets/levels/plaza.json", "assets/levels/casa_te.json"}},
        {"plaza.json", {"assets/levels/mercado.json", "assets/levels/mercado.json", "assets/levels/herreria.json",
                        "assets/levels/este.json"}},
        {"mercado.json", {"assets/levels/plaza.json"}},
        {"herreria.json", {"assets/levels/plaza.json"}},
        {"casa_te.json", {"assets/levels/este.json"}},
    };
    return Game::RumboNiveles([](const std::string& nivel) {
        auto it = puertas.find(Game::RumboNiveles::nombre(nivel));
        return it == puertas.end() ? std::vector<std::string>{} : it->second;
    });
}

}  // namespace

MOTOR_TEST_CASE(la_puerta_que_lleva_al_objetivo_y_la_primera_del_camino) {
    Game::RumboNiveles r = mundo();
    // En la plaza: la puerta del mercado (las dos llevan al mismo sitio).
    const auto enLaPlaza = r.primerosPasos("assets/levels/plaza.json", {"assets/levels/mercado.json"});
    MOTOR_CHECK(enLaPlaza.size() == 1 && enLaPlaza.count("assets/levels/mercado.json") == 1);
    // Desde la ciudad este, la herreria esta pasando por la plaza.
    const auto desdeElEste = r.primerosPasos("assets/levels/este.json", {"interior/herreria.json"});
    MOTOR_CHECK(desdeElEste.size() == 1 && desdeElEste.count("assets/levels/plaza.json") == 1);
    // Ya estas, o no hay camino: nada.
    MOTOR_CHECK(r.primerosPasos("assets/levels/mercado.json", {"assets/levels/mercado.json"}).empty());
    MOTOR_CHECK(r.primerosPasos("assets/levels/este.json", {"assets/levels/castillo.json"}).empty());
    MOTOR_CHECK(r.primerosPasos("assets/levels/este.json", {}).empty());
    // Varias metas: la mas cercana (la casa de te esta a una puerta).
    const auto cerca = r.primerosPasos("assets/levels/este.json", {"assets/levels/herreria.json", "assets/levels/casa_te.json"});
    MOTOR_CHECK(cerca.size() == 1 && cerca.count("assets/levels/casa_te.json") == 1);
}

MOTOR_TEST_CASE(los_niveles_de_verdad_la_ciudad_este_lleva_a_la_herreria_por_la_plaza) {
    Game::RumboNiveles r;  // lee assets/levels (el test corre en build/)
    const auto pasos = r.primerosPasos("assets/levels/ciudad_este.json", {"assets/levels/interior_herreria.json"});
    MOTOR_CHECK(pasos.size() == 1 && pasos.count("assets/levels/ciudad_centro.json") == 1);
}

int main() { return motor_test::runAllTests(); }
