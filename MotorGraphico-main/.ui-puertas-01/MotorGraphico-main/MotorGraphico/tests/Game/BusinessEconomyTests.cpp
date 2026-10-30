// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo toca 1 paquete del corte B:
//   C-24  (fase C, 5 j) - Xarxa de seguretat al camí que es juga
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios i proves: planificacion/PLA_DEFINITIU.md
// Mapa complet fitxer <-> paquet: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloc: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// C-24 — Xarxa de seguretat: economia del negoci
//
// Prova les funcions pures d'ocupacio i ingress, el cobrament de
// lloguers i la moral associada. Les operacions que depenen del
// GameMode (buyCurrentBusiness, adjustCurrentRent) necessiten un mapa
// amb NPCs per entrar en mode Business via interact(); aixo les fa
// integrals i les deixa per a un futur test d'integracio. Aqui nomes
// es proven les peces pures i el traspas d'estat.

#include "TestHarness.h"

#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "Render/ICombatant.h"

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

struct Fixture {
    ObjectCatalog catalog;
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
};

}  // namespace

// =====================================================================
// occupancyPercent — funcio pura
// =====================================================================

MOTOR_TEST_CASE(occupancy_at_100_percent_is_full) {
    MOTOR_CHECK(GameSession::occupancyPercent(100) == 100);
}

MOTOR_TEST_CASE(occupancy_below_100_is_always_full) {
    MOTOR_CHECK(GameSession::occupancyPercent(50) == 100);
    MOTOR_CHECK(GameSession::occupancyPercent(0) == 100);
    MOTOR_CHECK(GameSession::occupancyPercent(99) == 100);
}

MOTOR_TEST_CASE(occupancy_above_100_decreases) {
    // 110% -> 98% (lost 2 points: (110-100)*2/5 = 4, 100-4=96... wait)
    // Formula: 100 - (rentPercent - 100) * 2 / 5
    // 110: 100 - 10*2/5 = 100 - 4 = 96
    MOTOR_CHECK(GameSession::occupancyPercent(110) == 96);
    // 150: 100 - 50*2/5 = 100 - 20 = 80
    MOTOR_CHECK(GameSession::occupancyPercent(150) == 80);
    // 200: 100 - 100*2/5 = 100 - 40 = 60
    MOTOR_CHECK(GameSession::occupancyPercent(200) == 60);
}

MOTOR_TEST_CASE(occupancy_above_200_clamps_to_zero) {
    // 350: 100 - 250*2/5 = 100 - 100 = 0
    MOTOR_CHECK(GameSession::occupancyPercent(350) == 0);
    MOTOR_CHECK(GameSession::occupancyPercent(400) == 0);
}

MOTOR_TEST_CASE(occupancy_at_kMax_rent) {
    // kMaxRent = 200
    MOTOR_CHECK(GameSession::occupancyPercent(GameSession::kMaxRent) == 60);
}

// =====================================================================
// incomeFor — funcio pura
// =====================================================================

MOTOR_TEST_CASE(income_at_100_percent_rent) {
    // incomeFor(10, 100) = 10 * 100 * 100 / 10000 = 10
    MOTOR_CHECK(GameSession::incomeFor(10, 100) == 10);
}

MOTOR_TEST_CASE(income_at_50_percent_rent) {
    // 10 * 50 * 100 / 10000 = 5
    MOTOR_CHECK(GameSession::incomeFor(10, 50) == 5);
}

MOTOR_TEST_CASE(income_at_150_percent_rent) {
    // occupancy(150) = 80
    // 10 * 150 * 80 / 10000 = 12
    MOTOR_CHECK(GameSession::incomeFor(10, 150) == 12);
}

MOTOR_TEST_CASE(income_at_200_percent_rent) {
    // occupancy(200) = 60
    // 10 * 200 * 60 / 10000 = 12
    MOTOR_CHECK(GameSession::incomeFor(10, 200) == 12);
}

MOTOR_TEST_CASE(income_optimum_is_around_175) {
    // At 175: occupancy = 100 - 75*2/5 = 100 - 30 = 70
    // income = 10 * 175 * 70 / 10000 = 12
    // At 180: occupancy = 100 - 80*2/5 = 100 - 32 = 68
    // income = 10 * 180 * 68 / 10000 = 12
    // At 170: occupancy = 100 - 70*2/5 = 100 - 28 = 72
    // income = 10 * 170 * 72 / 10000 = 12
    // The optimum is plateaued around 170-180, all yielding 12
    const int at170 = GameSession::incomeFor(10, 170);
    const int at175 = GameSession::incomeFor(10, 175);
    const int at180 = GameSession::incomeFor(10, 180);
    // All should be >= income at 100% (which is 10)
    MOTOR_CHECK(at170 >= 10);
    MOTOR_CHECK(at175 >= 10);
    MOTOR_CHECK(at180 >= 10);
    // And the optimum should be higher than at 100%
    MOTOR_CHECK(at175 > GameSession::incomeFor(10, 100));
}

MOTOR_TEST_CASE(income_with_zero_base_is_zero) {
    MOTOR_CHECK(GameSession::incomeFor(0, 100) == 0);
    MOTOR_CHECK(GameSession::incomeFor(0, 200) == 0);
}

// =====================================================================
// collectRent — funcio d'instancia (via carry-over)
// =====================================================================

MOTOR_TEST_CASE(collect_rent_adds_gold_from_businesses) {
    Fixture f;
    PlayerCarryOver state;
    state.gold = 0;
    // Una posada amb baseIncome 12 i alquiler just (100%)
    state.businesses.push_back(OwnedBusiness{"posada", "Posada", 12, 100});

    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    session.applyCarryOver(state);

    const int collected = session.collectRent();
    // incomeFor(12, 100) = 12 * 100 * 100 / 10000 = 12
    MOTOR_CHECK(collected == 12);
    MOTOR_CHECK(session.gold() == 12);
}

MOTOR_TEST_CASE(collect_rent_with_abusive_rent_gives_more_gold_but_costs_morality) {
    Fixture f;
    PlayerCarryOver state;
    state.gold = 0;
    state.moralityValue = 0;
    state.businesses.push_back(OwnedBusiness{"herreria", "Herreria", 20, 150});

    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    session.applyCarryOver(state);

    const int collected = session.collectRent();
    // incomeFor(20, 150) = 20 * 150 * 80 / 10000 = 24
    MOTOR_CHECK(collected == 24);
    MOTOR_CHECK(session.gold() == 24);
    // Moral should decrease: (100-150)/25 = -2
    MOTOR_CHECK(session.morality().value() < 0);
}

MOTOR_TEST_CASE(collect_rent_with_generous_rent_gives_less_gold_but_boosts_morality) {
    Fixture f;
    PlayerCarryOver state;
    state.gold = 0;
    state.moralityValue = 0;
    state.businesses.push_back(OwnedBusiness{"posada", "Posada", 12, 50});

    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    session.applyCarryOver(state);

    const int collected = session.collectRent();
    // incomeFor(12, 50) = 12 * 50 * 100 / 10000 = 6
    MOTOR_CHECK(collected == 6);
    MOTOR_CHECK(session.gold() == 6);
    // Moral should increase: (100-50)/25 = 2
    MOTOR_CHECK(session.morality().value() > 0);
}

MOTOR_TEST_CASE(collect_rent_no_businesses_gives_zero) {
    Fixture f;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);

    MOTOR_CHECK(session.collectRent() == 0);
    MOTOR_CHECK(session.gold() == 0);
}

MOTOR_TEST_CASE(collect_rent_multiple_businesses_sums_income) {
    Fixture f;
    PlayerCarryOver state;
    state.gold = 0;
    state.businesses.push_back(OwnedBusiness{"posada", "Posada", 12, 100});
    state.businesses.push_back(OwnedBusiness{"herreria", "Herreria", 20, 100});
    state.businesses.push_back(OwnedBusiness{"taller", "Taller", 8, 100});

    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    session.applyCarryOver(state);

    const int collected = session.collectRent();
    // 12 + 20 + 8 = 40
    MOTOR_CHECK(collected == 40);
    MOTOR_CHECK(session.gold() == 40);
}

// =====================================================================
// ownsBusiness
// =====================================================================

MOTOR_TEST_CASE(owns_business_returns_true_for_owned) {
    Fixture f;
    PlayerCarryOver state;
    state.businesses.push_back(OwnedBusiness{"posada", "Posada", 12, 100});

    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    session.applyCarryOver(state);

    MOTOR_CHECK(session.ownsBusiness("posada"));
    MOTOR_CHECK(!session.ownsBusiness("herreria"));
}

// =====================================================================
// Propietats dels limits
// =====================================================================

MOTOR_TEST_CASE(kMinRent_and_kMaxRent_are_consistent) {
    MOTOR_CHECK(GameSession::kMinRent < GameSession::kMaxRent);
    MOTOR_CHECK(GameSession::kMinRent == 50);
    MOTOR_CHECK(GameSession::kMaxRent == 200);
}

int main() { return motor_test::runAllTests(); }
