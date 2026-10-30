#pragma once

#include <string>
#include <vector>
#include <optional>

#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/RandomEngine.h"

namespace RPG {

struct LootResult {
    std::string lootTableId;
    int gold = 0;
    std::vector<std::string> entryItems;
    std::vector<std::string> dropItems;
};

LootResult resolve_loot_table(const LootTableDefinition& table, RandomEngine& rng);
LootResult resolve_loot_table_deterministic(const LootTableDefinition& table, uint64_t seed);

} // namespace RPG