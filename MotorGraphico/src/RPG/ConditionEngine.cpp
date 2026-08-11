#include "RPG/ConditionEngine.h"

#include <algorithm>
#include <cstdio>

namespace RPG {

// ---------------------------------------------------------------------------
// Utilidades internas
// ---------------------------------------------------------------------------
std::vector<ActiveCondition>::iterator ConditionEngine::find_active(
    CharacterSheet& sheet, const std::string& conditionId) {
    return std::find_if(sheet.conditions.begin(), sheet.conditions.end(),
        [&](const ActiveCondition& c) { return c.conditionId == conditionId; });
}
std::vector<ActiveCondition>::const_iterator ConditionEngine::find_active_c(
    const CharacterSheet& sheet, const std::string& conditionId) {
    return std::find_if(sheet.conditions.begin(), sheet.conditions.end(),
        [&](const ActiveCondition& c) { return c.conditionId == conditionId; });
}

int ConditionEngine::active_stacks(const CharacterSheet& sheet,
                                   const std::string& conditionId) {
    auto it = find_active_c(sheet, conditionId);
    return (it == sheet.conditions.end()) ? 0 : it->stacks;
}

bool ConditionEngine::has_flag(const CharacterSheet& sheet,
                               const std::string& conditionId,
                               const Catalogs::ConditionCatalog& cat,
                               const std::function<bool(const ConditionDefinition&)>& pred) {
    auto it = find_active_c(sheet, conditionId);
    if (it == sheet.conditions.end()) return false;
    const ConditionDefinition* d = cat.find(conditionId);
    return d && pred(*d) && it->stacks > 0;
}

bool ConditionEngine::has_prevent_action(const CharacterSheet& sheet,
                                         const Catalogs::ConditionCatalog& cat) {
    for (const auto& c : sheet.conditions) {
        if (c.stacks <= 0) continue;
        const ConditionDefinition* d = cat.find(c.conditionId);
        if (d && d->preventAction) return true;
    }
    return false;
}
bool ConditionEngine::has_half_speed(const CharacterSheet& sheet,
                                     const Catalogs::ConditionCatalog& cat) {
    for (const auto& c : sheet.conditions) {
        if (c.stacks <= 0) continue;
        const ConditionDefinition* d = cat.find(c.conditionId);
        if (d && d->halfSpeed) return true;
    }
    return false;
}
bool ConditionEngine::grants_advantage_to_enemies(const CharacterSheet& sheet,
                                                  const Catalogs::ConditionCatalog& cat) {
    for (const auto& c : sheet.conditions) {
        if (c.stacks <= 0) continue;
        const ConditionDefinition* d = cat.find(c.conditionId);
        if (d && d->grantAdvantageToEnemies) return true;
    }
    return false;
}
bool ConditionEngine::grants_disadvantage_to_caster(const CharacterSheet& sheet,
                                                    const Catalogs::ConditionCatalog& cat) {
    for (const auto& c : sheet.conditions) {
        if (c.stacks <= 0) continue;
        const ConditionDefinition* d = cat.find(c.conditionId);
        if (d && d->grantDisadvantageToCaster) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Apply / Remove
// ---------------------------------------------------------------------------
Result<int> ConditionEngine::apply(CharacterSheet& sheet,
                                   const ConditionDefinition& cond,
                                   const Catalogs::ConditionCatalog& cat,
                                   const std::string& sourceId) {
    if (cond.id.empty()) {
        return Result<int>::Error("ConditionEngine::apply: id vacio");
    }
    auto it = find_active(sheet, cond.id);
    if (it != sheet.conditions.end()) {
        switch (cond.stacking) {
            case ConditionDefinition::Stacking::NONE:
                // reemplaza: refresca duración, stacks a 1
                it->stacks = 1;
                it->roundsRemaining = cond.defaultRounds <= 0 ? 1 : cond.defaultRounds;
                break;
            case ConditionDefinition::Stacking::DURATION:
                // solo refresca duración
                it->roundsRemaining = std::max(it->roundsRemaining,
                                               cond.defaultRounds <= 0 ? 1 : cond.defaultRounds);
                break;
            case ConditionDefinition::Stacking::STACK_INTENSITY:
                // stacks +1 y refresca duración (respeta maxStacks)
                if (cond.maxStacks > 0 && it->stacks < cond.maxStacks) {
                    it->stacks += 1;
                } else if (cond.maxStacks <= 0) {
                    it->stacks += 1;
                }
                it->roundsRemaining = std::max(it->roundsRemaining,
                                               cond.defaultRounds <= 0 ? 1 : cond.defaultRounds);
                break;
        }
        if (!sourceId.empty()) it->sourceId = sourceId;
        recalc_condition_modifiers(sheet, cat);
        return Result<int>::Ok(it->stacks);
    }
    // Nueva condición
    ActiveCondition a;
    a.conditionId = cond.id;
    a.sourceId = sourceId;
    a.roundsRemaining = cond.defaultRounds <= 0 ? 1 : cond.defaultRounds;
    a.stacks = 1;
    sheet.conditions.push_back(std::move(a));
    recalc_condition_modifiers(sheet, cat);
    return Result<int>::Ok(1);
}

Result<int> ConditionEngine::remove(CharacterSheet& sheet,
                                    const std::string& conditionId,
                                    const Catalogs::ConditionCatalog& cat,
                                    int stacks_to_remove) {
    auto it = find_active(sheet, conditionId);
    if (it == sheet.conditions.end()) {
        return Result<int>::Error(std::string("Condition no activa: ") + conditionId);
    }
    if (stacks_to_remove <= 0 || stacks_to_remove >= it->stacks) {
        // Remover todos
        int remaining = it->stacks;
        sheet.conditions.erase(it);
        recalc_condition_modifiers(sheet, cat);
        return Result<int>::Ok(0);
    }
    it->stacks -= stacks_to_remove;
    if (it->stacks <= 0) {
        sheet.conditions.erase(it);
        recalc_condition_modifiers(sheet, cat);
        return Result<int>::Ok(0);
    }
    recalc_condition_modifiers(sheet, cat);
    return Result<int>::Ok(it->stacks);
}

int ConditionEngine::purge_expired(CharacterSheet& sheet) {
    auto before = sheet.conditions.end();
    auto new_end = std::remove_if(sheet.conditions.begin(), sheet.conditions.end(),
        [](const ActiveCondition& c) { return c.roundsRemaining <= 0 && c.stacks <= 0; });
    int n = 0;
    for (auto it = new_end; it != before; ++it) ++n;  // en realidad before es end(), así no
    // contamos bien: mejor la diferencia:
    n = static_cast<int>(std::distance(new_end, before));
    sheet.conditions.erase(new_end, sheet.conditions.end());
    return n;
}

// ---------------------------------------------------------------------------
// Tick (ronda)
// ---------------------------------------------------------------------------
std::vector<ConditionEngine::TickEvent> ConditionEngine::tick_conditions(
    CharacterSheet& sheet, const Catalogs::ConditionCatalog& cat) {
    std::vector<TickEvent> out;

    // Primero iteramos y marcamos rounds. Atención: borramos in-place con índices
    // para no invalidar iteradores durante erase por vencimiento.
    std::vector<size_t> to_remove_indexes;
    for (size_t i = 0; i < sheet.conditions.size(); ++i) {
        auto& ac = sheet.conditions[i];
        ac.roundsRemaining -= 1;
        TickEvent ev;
        ev.conditionId = ac.conditionId;
        ev.stacksRemaining = ac.stacks;

        // Daño de tick: damagePerRound * stacks (si >0: daño; <0: heal)
        const ConditionDefinition* def = cat.find(ac.conditionId);
        if (def && def->damagePerRound != 0) {
            ev.damageInflicted = def->damagePerRound * ac.stacks;
        }
        if (def && def->breaksOnDamage && ev.damageInflicted > 0) {
            // Condición se rompe al recibir daño (encantado...) — la ponemos a 0
            ac.roundsRemaining = 0;
        }
        if (ac.roundsRemaining <= 0) {
            ev.expired = true;
            ev.stacksRemaining = 0;
            to_remove_indexes.push_back(i);
        }
        out.push_back(ev);
    }
    // Borramos en orden inverso para que los índices sigan siendo válidos
    std::sort(to_remove_indexes.begin(), to_remove_indexes.end(),
              std::greater<size_t>());
    for (size_t idx : to_remove_indexes) {
        if (idx < sheet.conditions.size()) {
            sheet.conditions.erase(sheet.conditions.begin() +
                                   static_cast<std::ptrdiff_t>(idx));
        }
    }
    recalc_condition_modifiers(sheet, cat);
    return out;
}

// ---------------------------------------------------------------------------
// Recalcular modificadores (stat/def)
// ---------------------------------------------------------------------------
void ConditionEngine::recalc_condition_modifiers(CharacterSheet& sheet,
                                                 const Catalogs::ConditionCatalog& cat) {
    sheet.conditionModifiers = {0, 0, 0, 0};
    for (const auto& ac : sheet.conditions) {
        if (ac.stacks <= 0) continue;
        const ConditionDefinition* d = cat.find(ac.conditionId);
        if (!d) continue;
        for (int i = 0; i < 4; ++i) {
            sheet.conditionModifiers[i] += d->statMod[i] * ac.stacks;
        }
        // Defensas extra: si el PJ tiene campos de defensas condicionales
        // lo ideal sería tener conditionSaveBonus[] etc. Como por ahora no hay,
        // los modificadores de save quedan para P1; los flag los lee el caller
        // via grants_advantage... / has_prevent_action.
    }
}

} // namespace RPG
