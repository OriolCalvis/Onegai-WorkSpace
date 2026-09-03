#include "RPG/ConditionEngine.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace {
void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

using RPG::CharacterSheet;
using RPG::ConditionDefinition;
using RPG::ConditionEngine;
using RPG::Catalogs::ConditionCatalog;

ConditionDefinition condition(const char* id, int rounds = 3) {
    ConditionDefinition def;
    def.id = id;
    def.defaultRounds = rounds;
    def.statMod = {0, -1, 0, 0};
    return def;
}

void stacking_and_removal() {
    CharacterSheet sheet;
    auto def = condition("bleeding");
    def.stacking = ConditionDefinition::Stacking::STACK_INTENSITY;
    def.maxStacks = 3;
    ConditionCatalog cat;
    cat.insert(def);
    for (int i = 0; i < 5; ++i) {
        const auto result = ConditionEngine::apply(sheet, def, cat, "attacker");
        require(result.isOk(), "apply failed");
        require(result.value() == std::min(i + 1, 3), "maxStacks not respected");
    }
    require(sheet.conditions.size() == 1, "duplicate active condition");
    require(sheet.conditions.front().sourceId == "attacker", "source lost");
    require(sheet.conditionModifiers[1] == -3, "stack modifiers not recalculated");
    require(ConditionEngine::remove(sheet, def.id, cat, 2).value() == 1,
            "partial removal failed");
    require(sheet.conditionModifiers[1] == -1, "partial removal kept old modifiers");
    require(ConditionEngine::remove(sheet, def.id, cat).value() == 0,
            "full removal failed");
    require(sheet.conditions.empty() && sheet.conditionModifiers[1] == 0,
            "full removal kept condition/modifiers");
}

void duration_and_replacement() {
    for (auto mode : {ConditionDefinition::Stacking::NONE,
                      ConditionDefinition::Stacking::DURATION}) {
        CharacterSheet sheet;
        auto def = condition("slow");
        def.stacking = mode;
        ConditionCatalog cat;
        cat.insert(def);
        require(ConditionEngine::apply(sheet, def, cat).isOk(), "apply failed");
        ConditionEngine::tick_conditions(sheet, cat);
        require(ConditionEngine::apply(sheet, def, cat, "new_source").value() == 1,
                "duration-only condition stacked");
        require(sheet.conditions.front().roundsRemaining == 3, "duration not refreshed");
        require(sheet.conditions.front().sourceId == "new_source", "source not refreshed");
    }
}

void expiry_and_last_tick() {
    CharacterSheet sheet;
    auto def = condition("poison", 2);
    def.damagePerRound = 4;
    def.halfSpeed = true;
    ConditionCatalog cat;
    cat.insert(def);
    ConditionEngine::apply(sheet, def, cat);
    const auto first = ConditionEngine::tick_conditions(sheet, cat);
    require(first.size() == 1 && first[0].damageInflicted == 4 && !first[0].expired,
            "first tick incorrect");
    const auto last = ConditionEngine::tick_conditions(sheet, cat);
    require(last.size() == 1 && last[0].damageInflicted == 4 && last[0].expired,
            "last round must still produce its tick");
    require(last[0].stacksRemaining == 0 && sheet.conditions.empty(), "condition did not expire");
    require(sheet.conditionModifiers[1] == 0 && !ConditionEngine::has_half_speed(sheet, cat),
            "expired condition still affects character");
    require(ConditionEngine::tick_conditions(sheet, cat).empty(), "expired condition ticked twice");
}

void purge_expired_and_empty_stacks() {
    CharacterSheet sheet;
    ConditionCatalog cat;
    for (const char* id : {"expired", "empty", "live"}) cat.insert(condition(id));
    sheet.conditions = {{"expired", "", 0, 2}, {"empty", "", 3, 0}, {"live", "", 2, 1}};
    sheet.conditionModifiers[1] = -99;
    require(ConditionEngine::purge_expired(sheet, cat) == 2, "purge must remove expired OR empty entries");
    require(sheet.conditions.size() == 1 && sheet.conditions[0].conditionId == "live",
            "purge removed a live condition");
    require(sheet.conditionModifiers[1] == -1, "purge kept stale stat modifiers");
    require(ConditionEngine::purge_expired(sheet, cat) == 0, "purge is not idempotent");
}

void computed_damage_is_not_received_damage() {
    CharacterSheet sheet;
    auto def = condition("fragile_poison");
    def.damagePerRound = 2;
    def.breaksOnDamage = true;
    ConditionCatalog cat;
    cat.insert(def);
    ConditionEngine::apply(sheet, def, cat);
    const auto events = ConditionEngine::tick_conditions(sheet, cat);
    require(events.size() == 1 && events[0].damageInflicted == 2, "tick damage missing");
    require(!events[0].expired && sheet.hasCondition(def.id),
            "calculating tick damage must not pretend that HP was lost");
}

void received_damage_removes_only_fragile_conditions() {
    CharacterSheet attacker, target;
    ConditionCatalog cat;
    auto charm = condition("charmed", 4);
    charm.breaksOnDamage = true;
    charm.preventAction = true;
    auto sleep = charm;
    sleep.id = "asleep";
    const auto poison = condition("poison", 4);
    cat.insert(charm);
    cat.insert(sleep);
    cat.insert(poison);
    ConditionEngine::apply(attacker, charm, cat);
    for (const auto& def : {charm, sleep, poison}) ConditionEngine::apply(target, def, cat);
    require(ConditionEngine::on_damage_received(target, cat, 0) == 0, "blocked damage broke charm");
    require(ConditionEngine::on_damage_received(target, cat, -4) == 0, "healing broke charm");
    require(target.conditions.size() == 3, "non-damage removed conditions");
    require(ConditionEngine::on_damage_received(target, cat, 1) == 2, "damage must remove both fragile conditions");
    require(target.hasCondition(poison.id) && target.conditions.size() == 1, "damage removed poison");
    require(target.conditionModifiers[1] == -1, "damage kept stale modifiers");
    require(!ConditionEngine::has_prevent_action(target, cat), "damage kept charm action flag");
    require(attacker.hasCondition(charm.id), "inflicting damage broke the attacker's charm");
    require(ConditionEngine::on_damage_received(target, cat, 9) == 0, "damage removal repeated");
}

void dot_breaks_charm_when_damage_is_delivered() {
    CharacterSheet sheet;
    ConditionCatalog cat;
    auto charm = condition("charmed");
    charm.breaksOnDamage = true;
    auto poison = condition("poison");
    poison.damagePerRound = 2;
    cat.insert(charm);
    cat.insert(poison);
    ConditionEngine::apply(sheet, charm, cat);
    ConditionEngine::apply(sheet, poison, cat);
    const auto events = ConditionEngine::tick_conditions(sheet, cat);
    require(sheet.hasCondition(charm.id), "charm broke before receiving damage");
    for (const auto& event : events) {
        // El propietario de PV confirma el dano aplicado, no el atacante.
        if (event.damageInflicted > 0)
            ConditionEngine::on_damage_received(sheet, cat, event.damageInflicted);
    }
    require(!sheet.hasCondition(charm.id) && sheet.hasCondition(poison.id), "DOT did not break charm");
}

void stale_entries_neither_modify_nor_tick() {
    CharacterSheet sheet;
    ConditionCatalog cat;
    auto def = condition("stale");
    def.damagePerRound = 20;
    def.preventAction = def.halfSpeed = true;
    def.grantAdvantageToEnemies = def.grantDisadvantageToCaster = true;
    cat.insert(def);
    sheet.conditions = {{def.id, "", 0, 2}};
    require(!sheet.hasCondition(def.id) && sheet.conditionStacks(def.id) == 0,
            "sheet reports an expired condition as active");
    require(ConditionEngine::active_stacks(sheet, def.id) == 0, "expired stacks remain active");
    require(!ConditionEngine::has_flag(sheet, def.id, cat,
                [](const auto& d) { return d.preventAction; }), "expired flag remains active");
    require(!ConditionEngine::has_prevent_action(sheet, cat) && !ConditionEngine::has_half_speed(sheet, cat)
                && !ConditionEngine::grants_advantage_to_enemies(sheet, cat)
                && !ConditionEngine::grants_disadvantage_to_caster(sheet, cat), "expired behavior flags remain active");
    ConditionEngine::recalc_condition_modifiers(sheet, cat);
    require(sheet.conditionModifiers[1] == 0, "expired stats remain active");
    require(ConditionEngine::tick_conditions(sheet, cat).empty(), "expired DOT ticked again");
}

void reapplication_starts_fresh_after_expiry() {
    CharacterSheet sheet;
    ConditionCatalog cat;
    auto def = condition("stacking");
    def.stacking = ConditionDefinition::Stacking::STACK_INTENSITY;
    def.maxStacks = 5;
    cat.insert(def);
    sheet.conditions = {{def.id, "old", 0, 5}};
    require(ConditionEngine::apply(sheet, def, cat, "new").value() == 1, "expired stacks were resurrected");
    require(sheet.conditions.size() == 1 && sheet.conditions[0].roundsRemaining == 3,
            "reapplication did not restore a fresh duration");
    require(sheet.conditionModifiers[1] == -1, "reapplication kept expired modifiers");
}

void invalid_ids_and_missing_removal_are_errors() {
    CharacterSheet sheet;
    ConditionCatalog cat;
    require(!ConditionEngine::apply(sheet, condition(""), cat).isOk(), "empty id accepted");
    require(!ConditionEngine::remove(sheet, "missing", cat).isOk(), "missing removal accepted");
    require(sheet.conditions.empty(), "invalid operation changed state");
}

void minimum_duration_and_unlimited_stacks() {
    CharacterSheet sheet;
    ConditionCatalog cat;
    auto def = condition("unlimited", 0);
    def.stacking = ConditionDefinition::Stacking::STACK_INTENSITY;
    def.maxStacks = 0;
    cat.insert(def);
    for (int i = 1; i <= 8; ++i)
        require(ConditionEngine::apply(sheet, def, cat).value() == i, "unlimited stacking was capped");
    require(sheet.conditions[0].roundsRemaining == 1, "minimum duration not applied");
    const auto events = ConditionEngine::tick_conditions(sheet, cat);
    require(events.size() == 1 && events[0].expired && sheet.conditions.empty(), "minimum duration did not expire");
}
} // namespace

int main() {
    const std::pair<const char*, void (*)()> cases[] = {
        {"stacking_and_removal", stacking_and_removal},
        {"duration_and_replacement", duration_and_replacement},
        {"expiry_and_last_tick", expiry_and_last_tick},
        {"purge_expired_and_empty_stacks", purge_expired_and_empty_stacks},
        {"computed_damage_is_not_received_damage", computed_damage_is_not_received_damage},
        {"received_damage_removes_only_fragile_conditions", received_damage_removes_only_fragile_conditions},
        {"dot_breaks_charm_when_damage_is_delivered", dot_breaks_charm_when_damage_is_delivered},
        {"stale_entries_neither_modify_nor_tick", stale_entries_neither_modify_nor_tick},
        {"reapplication_starts_fresh_after_expiry", reapplication_starts_fresh_after_expiry},
        {"invalid_ids_and_missing_removal_are_errors", invalid_ids_and_missing_removal_are_errors},
        {"minimum_duration_and_unlimited_stacks", minimum_duration_and_unlimited_stacks},
    };
    int failures = 0;
    for (const auto& test : cases) {
        try {
            test.second();
            std::cout << "[PASS] " << test.first << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "[FAIL] " << test.first << ": " << error.what() << '\n';
        }
    }
    std::cout << std::size(cases) << " tests; " << failures << " failures\n";
    return failures ? 1 : 0;
}
