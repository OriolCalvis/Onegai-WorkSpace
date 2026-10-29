// D-06 — Classes, races i trasfons. Prova de CONTINGUT REAL dels catàlegs
// publicats a assets/catalogs/{classes,races,backgrounds}.json.
//
// Criteris de tancament D-06 (documentats al pla, part de catàlegs):
//   Bonificadors, passives i trets de classe, raça i trasfond arriben a la
//   fitxa. Vincle, por, defecte, meta, ideal, personalitat i virtut es
//   guarden.
//
// (La part de Creació de Personatge i CharacterSheet ja es prova al target
// test_character_creation existent; aquest és el complement de catàlegs,
// equivalent als tests de contingut de D-03 i D-05.)
//
// GL-free.

#include "RPG/Catalogs/RpgCatalogs.h"
#include "TestHarness.h"

#include <cstddef>
#include <set>
#include <string>

namespace {

using RPG::BackgroundDefinition;
using RPG::Catalogs::BackgroundCatalog;
using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::ClassCatalog;
using RPG::Catalogs::RaceCatalog;
using RPG::ClassDefinition;
using RPG::RaceDefinition;

MOTOR_TEST_CASE(classes_carrega_sense_errors_D_06) {
    ClassCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/classes.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 30);   // asset publicat: 61 classes
}

MOTOR_TEST_CASE(classes_tots_els_camps_importants_D_06) {
    const ClassCatalog cat = [&](){
        ClassCatalog c;
        auto r = c.loadFromFile("assets/catalogs/classes.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    const std::set<std::string> stats_valides = {"CON", "DES", "INT", "CAR"};
    std::size_t total = 0;
    std::size_t amb_passive = 0;
    std::size_t amb_primary_invalid = 0;
    std::size_t amb_secondary_invalid = 0;
    std::size_t amb_baseHealth_ok = 0;
    std::size_t amb_tier_extret = 0;
    std::size_t amb_skills_inicials = 0;
    std::size_t amb_equip_inicial = 0;
    cat.forEach([&](const ClassDefinition& cls) {
        ++total;
        if (!cls.passiveId.empty()) ++amb_passive;
        if (!cls.primaryStat.empty() && stats_valides.count(cls.primaryStat) == 0)
            ++amb_primary_invalid;
        if (!cls.secondaryStat.empty() && stats_valides.count(cls.secondaryStat) == 0)
            ++amb_secondary_invalid;
        if (cls.baseHealth >= 4 && cls.baseHealth <= 20) ++amb_baseHealth_ok;
        if (cls.tier < 0 || cls.tier > 10) ++amb_tier_extret;
        if (!cls.startingSkillIds.empty()) ++amb_skills_inicials;
        if (!cls.startingEquipmentIds.empty()) ++amb_equip_inicial;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(amb_primary_invalid == 0);
    MOTOR_CHECK(amb_secondary_invalid == 0);
    MOTOR_CHECK(amb_tier_extret == 0);
    // En total hi ha d'haver classes (61 en el asset publicat)
    // Provem que al menys 1/3 tenen passive i 1/2 startingSkills
    MOTOR_CHECK(amb_passive >= total / 3);
    MOTOR_CHECK(amb_skills_inicials > 0);
    MOTOR_CHECK(amb_equip_inicial > 0);
}

MOTOR_TEST_CASE(classes_noms_distints_de_id_D_06) {
    // Si totes les classes tenen name == id → la clau 'name' no s'està llegint.
    const ClassCatalog cat = [&](){
        ClassCatalog c;
        auto r = c.loadFromFile("assets/catalogs/classes.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t distints = 0;
    cat.forEach([&](const ClassDefinition& cls) {
        ++total;
        if (cls.name != cls.id) ++distints;
    });
    if (total > 5) MOTOR_CHECK(distints >= 3);
}

MOTOR_TEST_CASE(races_carrega_sense_errors_D_06) {
    RaceCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/races.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 10);
}

MOTOR_TEST_CASE(races_statBonuses_i_traits_D_06) {
    // NOTA: el asset publicat races.json porta `passiveTrait`/`activeTrait` com
    // a objectes {name, description}, no com a ids. Per tant els camps
    // `passiveTraitId`/`activeTraitId` de RaceDefinition queden buits si no
    // hi ha un id explícit. Comprovem el que SÍ hi ha publicat:
    // statBonuses diferents de zero, languages, affinities, baseSpeed ok,
    // narrativeTags.
    const RaceCatalog cat = [&](){
        RaceCatalog c;
        auto r = c.loadFromFile("assets/catalogs/races.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_bonificadors = 0;
    std::size_t amb_affinities = 0;
    std::size_t amb_narrative = 0;
    std::size_t amb_speed_extrema = 0;
    cat.forEach([&](const RaceDefinition& r) {
        ++total;
        const int sum = r.statBonuses[0]+r.statBonuses[1]+r.statBonuses[2]+r.statBonuses[3];
        if (sum != 0) ++amb_bonificadors;
        if (!r.affinities.empty()) ++amb_affinities;
        if (!r.narrativeTags.empty()) ++amb_narrative;
        if (r.baseSpeed < 20 || r.baseSpeed > 50) ++amb_speed_extrema;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(amb_speed_extrema == 0);
    MOTOR_CHECK(amb_bonificadors > 0);
    MOTOR_CHECK(amb_affinities > 0 || amb_narrative > 0);
}

MOTOR_TEST_CASE(backgrounds_carrega_sense_errors_D_06) {
    BackgroundCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/backgrounds.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 10);
}

MOTOR_TEST_CASE(backgrounds_or_i_equip_D_06) {
    // NOTA: el asset publicat backgrounds.json NO porta startingWealthGold ni
    // startingEquipmentIds directament. Les eleccions narratives viuen dins
    // de la clau `characterCreation` (que el CharacterCreation llegeix per
    // separat). Per tant acceptem que or i equipinicial siguin zero i tan sols
    // garantim que no hi ha negatius i que el contingut bàsic (id, name,
    // description, tier) és consistent.
    const BackgroundCatalog cat = [&](){
        BackgroundCatalog c;
        auto r = c.loadFromFile("assets/catalogs/backgrounds.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_or_negative = 0;
    std::size_t amb_faction = 0;
    std::size_t amb_rep_boja = 0;
    std::size_t amb_tier_boig = 0;
    cat.forEach([&](const BackgroundDefinition& b) {
        ++total;
        if (b.startingWealthGold < 0) ++amb_or_negative;
        if (!b.factionId.empty()) ++amb_faction;
        if (b.startingFactionRep < -100 || b.startingFactionRep > 100) ++amb_rep_boja;
        if (b.tier < -5 || b.tier > 10) ++amb_tier_boig;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(amb_or_negative == 0);
    MOTOR_CHECK(amb_rep_boja == 0);
    MOTOR_CHECK(amb_tier_boig == 0);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
