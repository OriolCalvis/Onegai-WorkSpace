#include "RPG/LootResolver.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/RandomEngine.h"

#include <unordered_map>
#include <cmath>
#include <numeric>

#include "TestHarness.h"

namespace {

using RPG::Catalogs::LootTableCatalog;
using RPG::LootTableDefinition;
using RPG::LootResult;
using RPG::RandomEngine;
using RPG::Xoroshiro128p;

LootTableCatalog load_loot_catalog() {
    LootTableCatalog cat;
    auto r = cat.loadFromFile("assets/catalogs/loot_tables.json",
                              RPG::Catalogs::CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

MOTOR_TEST_CASE(deterministic_seed) {
    const LootTableCatalog cat = load_loot_catalog();
    const LootTableDefinition* def = cat.find("loot_adalid_del_rey_de_armas");
    MOTOR_CHECK(def != nullptr);

    LootResult r1 = RPG::resolve_loot_table_deterministic(*def, 12345);
    LootResult r2 = RPG::resolve_loot_table_deterministic(*def, 12345);
    MOTOR_CHECK(r1.gold == r2.gold);
    MOTOR_CHECK(r1.entryItems == r2.entryItems);
    MOTOR_CHECK(r1.dropItems == r2.dropItems);

    LootResult r3 = RPG::resolve_loot_table_deterministic(*def, 54321);
    MOTOR_CHECK(r3.gold != r1.gold || r3.entryItems != r1.entryItems || r3.dropItems != r1.dropItems);
}

MOTOR_TEST_CASE(gold_range) {
    const LootTableCatalog cat = load_loot_catalog();
    const LootTableDefinition* def = cat.find("loot_adalid_del_rey_de_armas");
    MOTOR_CHECK(def != nullptr);
    MOTOR_CHECK(def->minGold == 300);
    MOTOR_CHECK(def->maxGold == 750);

    Xoroshiro128p rng(999);
    for (int i = 0; i < 100; ++i) {
        LootResult res = RPG::resolve_loot_table(*def, rng);
        MOTOR_CHECK(res.gold >= def->minGold);
        MOTOR_CHECK(res.gold <= def->maxGold);
    }
}

MOTOR_TEST_CASE(entries_parallel_to_drops) {
    const LootTableCatalog cat = load_loot_catalog();
    int parallel = 0;
    cat.forEach([&](const LootTableDefinition& def) {
        if (def.drops.size() == def.entries.size()) ++parallel;
    });
    MOTOR_CHECK(parallel == 201);
}

MOTOR_TEST_CASE(items_not_empty) {
    const LootTableCatalog cat = load_loot_catalog();
    int nonEmpty = 0;
    cat.forEach([&](const LootTableDefinition& def) {
        Xoroshiro128p rng(def.id.size());
        LootResult res = RPG::resolve_loot_table(def, rng);
        if (!res.entryItems.empty()) ++nonEmpty;
    });
    MOTOR_CHECK(nonEmpty > 150);
}

MOTOR_TEST_CASE(statistical_distribution) {
    const LootTableCatalog cat = load_loot_catalog();
    const LootTableDefinition* def = cat.find("loot_alijo_bosque_t1");
    MOTOR_CHECK(def != nullptr);

    std::unordered_map<std::string, int> counts;
    const int trials = 10000;

    for (int i = 0; i < trials; ++i) {
        Xoroshiro128p rng(i);
        LootResult res = RPG::resolve_loot_table(*def, rng);
        for (const auto& item : res.entryItems) {
            counts[item]++;
        }
    }

    int totalWeight = 0;
    for (const auto& entry : def->entries) {
        totalWeight += entry.weight;
    }

    for (const auto& entry : def->entries) {
        auto it = counts.find(entry.refId);
        if (it != counts.end()) {
            double expected = trials * (static_cast<double>(entry.weight) / totalWeight);
            double tolerance = 0.15 * expected;
            MOTOR_CHECK(std::abs(it->second - expected) < tolerance);
        }
    }
}

MOTOR_TEST_CASE(drops_chance_respected) {
    const LootTableCatalog cat = load_loot_catalog();
    const LootTableDefinition* def = cat.find("loot_adalid_del_rey_de_armas");
    MOTOR_CHECK(def != nullptr);

    int drop100 = 0;
    int drop60 = 0;
    int drop25 = 0;
    const int trials = 5000;

    for (int i = 0; i < trials; ++i) {
        Xoroshiro128p rng(i);
        LootResult res = RPG::resolve_loot_table(*def, rng);
        for (const auto& item : res.dropItems) {
            if (item == "pocion_de_vigor") drop100++;
            else if (item == "cuchillo_de_cazador_de_hierro_santo") drop60++;
            else if (item == "cuchillo_de_cazador_de_estrella") drop25++;
        }
    }

    MOTOR_CHECK(drop100 > trials * 0.95);
    MOTOR_CHECK(drop60 > trials * 0.50 && drop60 < trials * 0.70);
    MOTOR_CHECK(drop25 > trials * 0.15 && drop25 < trials * 0.35);
}

MOTOR_TEST_CASE(tier_range) {
    const LootTableCatalog cat = load_loot_catalog();
    int valid = 0;
    cat.forEach([&](const LootTableDefinition& def) {
        if (def.tierMin <= def.tierMax && def.tierMin >= 1 && def.tierMax <= 5) {
            ++valid;
        }
    });
    MOTOR_CHECK(valid == 201);
}

} // namespace

int main() { return motor_test::runAllTests(); }