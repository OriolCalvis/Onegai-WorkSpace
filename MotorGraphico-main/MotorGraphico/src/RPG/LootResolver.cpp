#include "RPG/LootResolver.h"
#include "RPG/RandomEngine.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"

#include <algorithm>
#include <numeric>

namespace RPG {

namespace {

int roll_weighted(const std::vector<LootTableDefinition::Entry>& entries, RandomEngine& rng) {
    if (entries.empty()) return -1;
    int total = 0;
    for (const auto& e : entries) total += e.weight;
    if (total <= 0) return -1;
    int roll = rng.roll_in_range(1, total);
    int acc = 0;
    for (std::size_t i = 0; i < entries.size(); ++i) {
        acc += entries[i].weight;
        if (roll <= acc) return static_cast<int>(i);
    }
    return static_cast<int>(entries.size()) - 1;
}

int roll_gold(int minGold, int maxGold, RandomEngine& rng) {
    if (minGold >= maxGold) return minGold;
    return rng.roll_in_range(minGold, maxGold);
}

bool check_condition(const std::string& condition) {
    return true;
}

bool roll_chance(int chance, RandomEngine& rng) {
    if (chance <= 0) return false;
    if (chance >= 100) return true;
    return rng.roll_in_range(1, 100) <= chance;
}

} // namespace

LootResult resolve_loot_table(const LootTableDefinition& table, RandomEngine& rng) {
    LootResult result;
    result.lootTableId = table.id;
    result.gold = roll_gold(table.minGold, table.maxGold, rng);

    int entryIdx = roll_weighted(table.entries, rng);
    if (entryIdx >= 0) {
        const auto& entry = table.entries[entryIdx];
        int qty = entry.min == entry.max ? entry.min : rng.roll_in_range(entry.min, entry.max);
for (int i = 0; i < qty; ++i) {
            result.entryItems.push_back(entry.refId);
        }
    }

    for (const auto& drop : table.drops) {
        if (!check_condition(drop.condition)) continue;
        if (drop.chance <= 0) continue;
if (roll_chance(drop.chance, rng)) {
            result.dropItems.push_back(drop.itemId);
        }
    }

    return result;
}

LootResult resolve_loot_table_deterministic(const LootTableDefinition& table, uint64_t seed) {
    Xoroshiro128p rng(seed);
    return resolve_loot_table(table, rng);
}

} // namespace RPG