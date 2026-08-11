#pragma once

#include <functional>
#include <string>
#include <vector>

#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "Core/Errors/Result.h"

namespace RPG {

// ============================================================
// ConditionEngine (GAMEMACHINE_P0 §8)
//
// Sistema de condiciones stackeables con tick por ronda.
//
//   - apply: añade 1 stack a la condición (incrementa roundsRemaining).
//     Si maxStacks superado → no stackea, solo refresca duración.
//   - remove_all / remove_stacks: quita stacks.
//   - tick_conditions: -1 ronda, trigger onTick (daño stackeable),
//     y si rounds=0 → remove auto + trigger onRemove().
//   - recalc_condition_modifiers: suma conditionModifiers[], flags
//     (preventAction, halfSpeed, grantAdvantageToEnemies, grantDisadvantage)
//     a partir de TODAS las condiciones activas.
//
// Triggers:
//   - onApply(sourceId, stacks_applied) — la primera vez que se añade o
//     cuando se añaden stacks; para Condiciones con dotMagnitude no
//     se hace daño en el apply (solo en tick), igual que Pathfinder 2E.
//   - onTick(tick) — damagePerStack * stacks (daño stackeable).
//   - onRemove(stacks_remaining) — cura, limpia buffs temporales, etc.
//
// Los triggers están en ConditionDefinition. onApply y onRemove son por
// ahora empty lambdas si el GDD no los define; los hooks se expanden en P1.
// ============================================================
class ConditionEngine {
public:
    // Estructura que devuelve tick_conditions: log de que ha pasado.
    struct TickEvent {
        std::string conditionId;
        int damageInflicted = 0;  // >0 dot; <0 heal; 0 sin daño
        bool expired = false;
        int stacksRemaining = 0;
    };

    // Aplica/stackea una condición. Si ya está activa → +1 stack y
    // refresca roundsRemaining al max(definition.durationRounds, current+1).
    // Retorna Ok(stacks_finales).
    static Result<int> apply(CharacterSheet& sheet,
                             const ConditionDefinition& cond,
                             const Catalogs::ConditionCatalog& cat,
                             const std::string& sourceId = "");

    // Quita stacks (por defecto todos si stacks <= 0). Si after remove
    // stacks == 0 borra la entrada y llama a on_remove triggers.
    // Retorna Ok(stacks_que_ quedan).
    static Result<int> remove(CharacterSheet& sheet,
                              const std::string& conditionId,
                              const Catalogs::ConditionCatalog& cat,
                              int stacks_to_remove = 0);

    // Elimina todas las condiciones con roundsRemaining == 0 que han
    // quedado colgadas. (Normalmente tick lo hace; helper limpieza.)
    static int purge_expired(CharacterSheet& sheet);

    // Tick 1 ronda: -1 rounds a cada condición activa, aplica daño dot,
    // si rounds==0 las expira (y dispara onRemove via remove).
    // Retorna vector de TickEvent (para logs / BattleState animations).
    static std::vector<TickEvent> tick_conditions(CharacterSheet& sheet,
                                                  const Catalogs::ConditionCatalog& cat);

    // Calcula conditionModifiers[] y flags. Se llama automáticamente en
    // apply/remove/tick. Es público porque la UI lo necesita para
    // recalcular después de cargar un save.
    static void recalc_condition_modifiers(CharacterSheet& sheet,
                                           const Catalogs::ConditionCatalog& cat);

    // ====== Helpers ======
    static int active_stacks(const CharacterSheet& sheet,
                             const std::string& conditionId);

    static bool has_flag(const CharacterSheet& sheet,
                         const std::string& conditionId,
                         const Catalogs::ConditionCatalog& cat,
                         const std::function<bool(const ConditionDefinition&)>& pred);

    static bool has_prevent_action(const CharacterSheet& sheet,
                                   const Catalogs::ConditionCatalog& cat);
    static bool has_half_speed(const CharacterSheet& sheet,
                               const Catalogs::ConditionCatalog& cat);
    static bool grants_advantage_to_enemies(const CharacterSheet& sheet,
                                            const Catalogs::ConditionCatalog& cat);
    static bool grants_disadvantage_to_caster(const CharacterSheet& sheet,
                                              const Catalogs::ConditionCatalog& cat);

private:
    // Busca una condición activa por id (retorna iterator).
    static std::vector<ActiveCondition>::iterator find_active(
        CharacterSheet& sheet, const std::string& conditionId);
    static std::vector<ActiveCondition>::const_iterator find_active_c(
        const CharacterSheet& sheet, const std::string& conditionId);
};

} // namespace RPG
