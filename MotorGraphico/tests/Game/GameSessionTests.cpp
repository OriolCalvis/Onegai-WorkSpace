// C-03 — El estado del jugador sobrevive al cambio de nivel.
//
// El bug: Application::loadLevel destruye la GameSession y construye
// otra, y de la vieja rescataba SOLO el oro. Entrar a una posada te
// dejaba la reputacion a 0, sin los negocios comprados y con el contador
// de rentas a cero -- es decir, con la mitad de la partida borrada por
// cruzar una puerta.
//
// Lo que se prueba aqui es la funcion de traspaso (captureCarryOver /
// applyCarryOver), que es exactamente lo que loadLevel llama a los dos
// lados de la frontera. Unit y sin GL: la sesion se construye sin mapa
// (TileMap nullptr), como en SaveGameTests.

#include "TestHarness.h"

#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

class TestCombatant : public ICombatant {
public:
    explicit TestCombatant(int maxHealth) : m_health(maxHealth), m_maxHealth(maxHealth) {}

    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::min(m_maxHealth, m_health + amount); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }

    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }

private:
    int m_health;
    int m_maxHealth;
};

LevelDefinition makeLevel(const char* name) {
    LevelDefinition level;
    level.name = name;
    level.playerStart = GridCoord{0, 0};
    return level;
}

PlayerCarryOver makeState() {
    PlayerCarryOver state;
    state.gold = 90;
    state.moralityValue = -30;
    state.stepsSinceRent = 17;
    state.businesses.push_back(OwnedBusiness{"posada", "Posada del Cuervo", 12, 140});
    state.businesses.push_back(OwnedBusiness{"herreria", "Herreria de Bran", 20, 80});
    return state;
}

// Una sesion "de nivel 1" ya jugada: 90 de oro, moral -30 y dos negocios.
struct Fixture {
    ObjectCatalog catalog;
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
};

}  // namespace

// El caso del pla: reconstruir la sesion sobre OTRO nivel y comprobar los
// cuatro valores. Antes de C-03 solo sobrevivia el oro.
MOTOR_TEST_CASE(el_estado_del_jugador_cruza_el_cambio_de_nivel) {
    Fixture f;
    GameSession primera(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    primera.applyCarryOver(makeState());

    // La sesion saliente lee de verdad lo que se le puso.
    MOTOR_CHECK(primera.gold() == 90);
    MOTOR_CHECK(primera.morality().value() == -30);
    MOTOR_CHECK(primera.businesses().size() == 2);

    // Lo que hace loadLevel: capturar de la vieja, construir la nueva
    // sobre otro nivel y aplicarselo.
    const PlayerCarryOver carried = primera.captureCarryOver();

    GameSession segunda(nullptr, &f.catalog, &f.skills, makeLevel("Posada"), &f.body,
                        &f.playerSkills, &f.inventory);
    segunda.applyCarryOver(carried);

    MOTOR_CHECK(segunda.gold() == 90);
    MOTOR_CHECK(segunda.morality().value() == -30);
    MOTOR_CHECK(segunda.captureCarryOver().stepsSinceRent == 17);
    MOTOR_CHECK(segunda.businesses().size() == 2);
    MOTOR_CHECK(segunda.businesses()[0].objectId == "posada");
    MOTOR_CHECK(segunda.businesses()[0].rentPercent == 140);
    MOTOR_CHECK(segunda.businesses()[1].objectId == "herreria");
    MOTOR_CHECK(segunda.businesses()[1].baseIncome == 20);
}

// Sin traspaso, una sesion nueva nace vacia: es lo que veia el jugador al
// cruzar una puerta antes de C-03, y lo que hace que la prueba de arriba
// signifique algo.
MOTOR_TEST_CASE(una_sesion_recien_construida_nace_sin_estado_de_jugador) {
    Fixture f;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Posada"), &f.body,
                        &f.playerSkills, &f.inventory);

    MOTOR_CHECK(session.gold() == 0);
    MOTOR_CHECK(session.morality().value() == 0);
    MOTOR_CHECK(session.businesses().empty());
    MOTOR_CHECK(session.captureCarryOver().stepsSinceRent == 0);
}

// applyCarryOver asigna, no acumula: aplicarlo dos veces deja lo mismo.
// Importa porque loadLevel lo llama una vez por carga y una recarga del
// mismo nivel no debe duplicar el oro.
MOTOR_TEST_CASE(aplicar_el_traspaso_dos_veces_no_duplica_nada) {
    Fixture f;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);

    session.applyCarryOver(makeState());
    session.applyCarryOver(makeState());

    MOTOR_CHECK(session.gold() == 90);
    MOTOR_CHECK(session.businesses().size() == 2);
    MOTOR_CHECK(session.morality().value() == -30);
}

// El traspaso no toca lo que no es suyo: la vida y el inventario viven
// fuera de la sesion y no deben verse afectados.
MOTOR_TEST_CASE(el_traspaso_no_toca_vida_ni_inventario) {
    Fixture f;
    f.body.takeDamage(25);
    f.inventory.push_back("pocion");

    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    session.applyCarryOver(makeState());

    MOTOR_CHECK(f.body.health() == 35);
    MOTOR_CHECK(f.inventory.size() == 1);
}

int main() { return motor_test::runAllTests(); }
