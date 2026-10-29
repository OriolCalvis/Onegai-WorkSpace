#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "TestHarness.h"

#include <string>
#include <vector>

namespace {

using RPG::Catalogs::EncounterTableCatalog;
using RPG::EncounterTableDefinition;

EncounterTableCatalog load_encounter_catalog() {
    EncounterTableCatalog cat;
    auto r = cat.loadFromFile("assets/catalogs/encounter_tables.json",
                              RPG::Catalogs::CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

MOTOR_TEST_CASE(encounter_catalog_loads_correctly) {
    const EncounterTableCatalog cat = load_encounter_catalog();
    MOTOR_CHECK(cat.size() == 5);
    MOTOR_CHECK(cat.find("encounter_bosque_t1") != nullptr);
    MOTOR_CHECK(cat.find("encounter_bosque_t2") != nullptr);
    MOTOR_CHECK(cat.find("encounter_ciudad_t1") != nullptr);
    MOTOR_CHECK(cat.find("encounter_mazmorra_t1") != nullptr);
    MOTOR_CHECK(cat.find("encounter_jefe_lobo") != nullptr);
}

MOTOR_TEST_CASE(encounter_bosque_t1_has_correct_data) {
    const EncounterTableCatalog cat = load_encounter_catalog();
    const EncounterTableDefinition* def = cat.find("encounter_bosque_t1");
    MOTOR_CHECK(def != nullptr);
    MOTOR_CHECK(def->id == "encounter_bosque_t1");
    MOTOR_CHECK(def->name == "Encuentro en el bosque (Tier 1)");
    MOTOR_CHECK(def->tier == 1);
    MOTOR_CHECK(def->rarity == RPG::ItemRarity::Common);
    MOTOR_CHECK(def->entries.size() == 3);
    MOTOR_CHECK(def->entries[0].refId == "monster_lobo");
    MOTOR_CHECK(def->entries[0].weight == 50);
    MOTOR_CHECK(def->entries[0].min == 1);
    MOTOR_CHECK(def->entries[0].max == 3);
    MOTOR_CHECK(def->conditions.size() == 2);
    MOTOR_CHECK(def->conditions[0].type == "zone");
    MOTOR_CHECK(def->conditions[0].value == "bosque");
    MOTOR_CHECK(def->conditions[0].comparison == "==");
    MOTOR_CHECK(def->conditions[1].type == "time");
    MOTOR_CHECK(def->conditions[1].value == "night");
    MOTOR_CHECK(def->conditions[1].comparison == "==");
    MOTOR_CHECK(def->lootTableId == "loot_alijo_bosque_t1");
    MOTOR_CHECK(def->minPartyLevel == 1);
    MOTOR_CHECK(def->maxPartyLevel == 2);
}

MOTOR_TEST_CASE(encounter_bosque_t2_tier_2) {
    const EncounterTableCatalog cat = load_encounter_catalog();
    const EncounterTableDefinition* def = cat.find("encounter_bosque_t2");
    MOTOR_CHECK(def != nullptr);
    MOTOR_CHECK(def->tier == 2);
    MOTOR_CHECK(def->minPartyLevel == 2);
    MOTOR_CHECK(def->maxPartyLevel == 3);
}

MOTOR_TEST_CASE(encounter_jefe_lobo_rare) {
    const EncounterTableCatalog cat = load_encounter_catalog();
    const EncounterTableDefinition* def = cat.find("encounter_jefe_lobo");
    MOTOR_CHECK(def != nullptr);
    MOTOR_CHECK(def->rarity == RPG::ItemRarity::Rare);
    MOTOR_CHECK(def->entries.size() == 1);
    MOTOR_CHECK(def->entries[0].refId == "monster_alfa_lobo");
    MOTOR_CHECK(def->entries[0].weight == 100);
    MOTOR_CHECK(def->entries[0].min == 1);
    MOTOR_CHECK(def->entries[0].max == 1);
    MOTOR_CHECK(def->conditions.size() == 1);
    MOTOR_CHECK(def->conditions[0].type == "flag");
    MOTOR_CHECK(def->conditions[0].value == "boss_lobo_disponible");
    MOTOR_CHECK(def->lootTableId == "loot_jefe_lobo");
}

MOTOR_TEST_CASE(encounter_entries_parallel_to_conditions) {
    const EncounterTableCatalog cat = load_encounter_catalog();
    int valid = 0;
    cat.forEach([&](const EncounterTableDefinition& def) {
        // Cada encuentro debe tener al menos una entrada
        if (!def.entries.empty()) ++valid;
    });
    MOTOR_CHECK(valid == 5);
}

MOTOR_TEST_CASE(encounter_min_max_party_levels) {
    const EncounterTableCatalog cat = load_encounter_catalog();
    int valid = 0;
    cat.forEach([&](const EncounterTableDefinition& def) {
        if (def.minPartyLevel >= 1 && def.maxPartyLevel <= 5 && def.minPartyLevel <= def.maxPartyLevel) {
            ++valid;
        }
    });
    MOTOR_CHECK(valid == 5);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
