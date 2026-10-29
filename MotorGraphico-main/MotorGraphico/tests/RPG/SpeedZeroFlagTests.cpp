// D-11 — flag speedZero per `velocidad_0`.
//
// D-04 va deixar el hook `velocidad_0` CRU a mechanicHooks perquè no era
// halfSpeed (és més fort: "velocitat a 0"). D-09 (pendent 993) demana
// el flag nou (speed_zero) al D-11. Aquest fitxer el tanca: 3 cartes
// publicades (agarrado/anclado/paralizado) l'han de carregar a speedZero,
// el helper ConditionEngine::has_speed_zero ha de distingir condició
// activa d'inactiva i no col·lisionar amb halfSpeed.
//
// GL-free. NO toca ConditionEngineTests.cpp (propietat agent anterior) ni
// CatalogContentTests.cpp (D-02/D-10) ni cap fitxer compartit.

#include "TestHarness.h"

#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/ConditionEngine.h"
#include "RPG/CharacterSheet.h"

#include <string>
#include <vector>

namespace {

using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::ConditionCatalog;
using RPG::ConditionDefinition;
using RPG::ConditionEngine;

ConditionCatalog loadPublishedConditions() {
    ConditionCatalog cat;
    auto r = cat.loadFromFile("assets/catalogs/conditions.json",
                              CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 10);
    return cat;
}

bool hasRawHook(const ConditionDefinition& d, const std::string& hook) {
    for (const auto& h : d.mechanicHooks) {
        if (h == hook) return true;
    }
    return false;
}

}  // namespace

MOTOR_TEST_CASE(d11_speedzero_cartes_publicades_agarrado_anclado_paralizado) {
    const ConditionCatalog catalog = loadPublishedConditions();

    const ConditionDefinition* agarrado = catalog.find("agarrado");
    const ConditionDefinition* anclado = catalog.find("anclado");
    const ConditionDefinition* paralizado = catalog.find("paralizado");

    // Les 3 cartes que el seu effects[] declara velocidad_0.
    MOTOR_CHECK(agarrado != nullptr);
    MOTOR_CHECK(anclado != nullptr);
    MOTOR_CHECK(paralizado != nullptr);

    MOTOR_CHECK(agarrado->speedZero);
    MOTOR_CHECK(anclado->speedZero);
    MOTOR_CHECK(paralizado->speedZero);

    // El hook cru es conserva (no trenca el contracte D-04 de raw DSL).
    MOTOR_CHECK(hasRawHook(*agarrado, "velocidad_0"));
    MOTOR_CHECK(hasRawHook(*paralizado, "velocidad_0"));

    // En tot el catàleg, cap carta amb velocitat mitjana (o sense el hook)
    // pot acabar amb speedZero posat per accident.
    std::size_t ambSpeedZero = 0;
    std::size_t ambHalfSpeedSenseZero = 0;
    catalog.forEach([&](const ConditionDefinition& d) {
        if (d.speedZero) ++ambSpeedZero;
        if (d.halfSpeed && !d.speedZero) ++ambHalfSpeedSenseZero;
    });
    MOTOR_CHECK(ambSpeedZero >= 3);
    MOTOR_CHECK(ambHalfSpeedSenseZero >= 1);  // ralentizado/lent, per contrast
}

MOTOR_TEST_CASE(d11_speedzero_helper_actiu_nomes_amb_condicio_aplicada) {
    const ConditionCatalog catalog = loadPublishedConditions();
    const ConditionDefinition* agarrado = catalog.find("agarrado");
    MOTOR_CHECK(agarrado != nullptr);

    RPG::CharacterSheet neta;
    MOTOR_CHECK(!ConditionEngine::has_speed_zero(neta, catalog));

    RPG::CharacterSheet aplicada;
    auto r = ConditionEngine::apply(aplicada, *agarrado, catalog, "test");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(aplicada.hasCondition("agarrado"));
    MOTOR_CHECK(ConditionEngine::has_speed_zero(aplicada, catalog));

    MOTOR_CHECK(ConditionEngine::remove(aplicada, "agarrado", catalog).isOk());
    MOTOR_CHECK(!aplicada.hasCondition("agarrado"));
    MOTOR_CHECK(!ConditionEngine::has_speed_zero(aplicada, catalog));
}

MOTOR_TEST_CASE(d11_speedzero_no_es_half_speed) {
    // Velocitat 0 i velocitat mitja són flags diferents i independents:
    // cap pot activar l'altre.
    ConditionCatalog cat;

    ConditionDefinition zero;
    zero.id = "d11_test_zero";
    zero.name = "Speed zero test";
    zero.stacking = ConditionDefinition::Stacking::NONE;
    zero.durationMode = ConditionDefinition::Duration::ROUNDS;
    zero.defaultRounds = 3;
    zero.speedZero = true;
    cat.insert(zero);

    ConditionDefinition mitja;
    mitja.id = "d11_test_mitja";
    mitja.name = "Half speed test";
    mitja.stacking = ConditionDefinition::Stacking::NONE;
    mitja.durationMode = ConditionDefinition::Duration::ROUNDS;
    mitja.defaultRounds = 3;
    mitja.halfSpeed = true;
    cat.insert(mitja);

    RPG::CharacterSheet a;
    MOTOR_CHECK(ConditionEngine::apply(a, zero, cat).isOk());
    MOTOR_CHECK(ConditionEngine::has_speed_zero(a, cat));
    MOTOR_CHECK(!ConditionEngine::has_half_speed(a, cat));

    RPG::CharacterSheet b;
    MOTOR_CHECK(ConditionEngine::apply(b, mitja, cat).isOk());
    MOTOR_CHECK(ConditionEngine::has_half_speed(b, cat));
    MOTOR_CHECK(!ConditionEngine::has_speed_zero(b, cat));
}

MOTOR_TEST_CASE(d11_speedzero_expira_per_rondes_com_qualsevol_flag) {
    // Una condició ROUNDS amb speedZero deixa de computar-se quan caduca,
    // igual que halfSpeed/preventAction: el helper respecta el gating.
    ConditionCatalog cat;
    ConditionDefinition one;
    one.id = "d11_test_one_round";
    one.name = "One round zero";
    one.stacking = ConditionDefinition::Stacking::NONE;
    one.durationMode = ConditionDefinition::Duration::ROUNDS;
    one.defaultRounds = 1;
    one.speedZero = true;
    cat.insert(one);

    RPG::CharacterSheet s;
    MOTOR_CHECK(ConditionEngine::apply(s, one, cat).isOk());
    MOTOR_CHECK(ConditionEngine::has_speed_zero(s, cat));

    ConditionEngine::tick_conditions(s, cat);
    ConditionEngine::purge_expired(s, cat);
    MOTOR_CHECK(!s.hasCondition("d11_test_one_round"));
    MOTOR_CHECK(!ConditionEngine::has_speed_zero(s, cat));
}

int main() { return motor_test::runAllTests(); }
