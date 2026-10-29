#pragma once

#include <functional>
#include <string>
#include <vector>

#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "Core/Errors/Result.h"

namespace RPG {

// Unico escritor de las reglas de condiciones sobre CharacterSheet.
// apply respeta Stacking (reemplazar, refrescar o sumar intensidad),
// remove/purge/tick recalculan los modificadores desde el catalogo.
// tick_conditions calcula efectos, pero NO cambia PV: el propietario de
// la vida aplica cada efecto y notifica la perdida real con on_damage_received.
// No hay callbacks onApply/onRemove en ConditionDefinition.
class ConditionEngine {
public:
    // Estructura que devuelve tick_conditions: log de que ha pasado.
    struct TickEvent {
        std::string conditionId;
        int damageInflicted = 0;  // >0 dot; <0 heal; 0 sin daño
        bool expired = false;
        int stacksRemaining = 0;
    };

    // Aplica una carga segun stacking, respeta maxStacks y refresca la
    // duracion (minimo una ronda). Retorna Ok(stacks_finales).
    static Result<int> apply(CharacterSheet& sheet,
                             const ConditionDefinition& cond,
                             const Catalogs::ConditionCatalog& cat,
                             const std::string& sourceId = "");

    // Quita stacks (todos si stacks_to_remove <= 0) y recalcula modificadores.
    // Retorna Ok(stacks_restantes); un id ausente es un error.
    static Result<int> remove(CharacterSheet& sheet,
                              const std::string& conditionId,
                              const Catalogs::ConditionCatalog& cat,
                              int stacks_to_remove = 0);

    // Elimina entradas con rondas O cargas agotadas y recalcula modificadores.
    static int purge_expired(CharacterSheet& sheet,
                             const Catalogs::ConditionCatalog& cat);

    // Notificar SOLO los PV realmente perdidos por el portador, tras aplicar
    // el dano (incluido DOT). Cero/curacion/dano bloqueado no rompen nada.
    // Retorna cuantas condiciones breaksOnDamage se han retirado.
    static int on_damage_received(CharacterSheet& sheet,
                                  const Catalogs::ConditionCatalog& cat,
                                  int healthLost);

    // Tick de una ronda. La ultima ronda tambien produce su efecto.
    // Las entradas ya caducadas se purgan sin producir otro tick.
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
    static bool has_speed_zero(const CharacterSheet& sheet,
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
