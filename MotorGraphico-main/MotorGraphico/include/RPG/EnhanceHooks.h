// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-22  (fase C, 2 j) - Retirada de l'API morta
//   D-09  (fase D, 2 j) - Les cartes apliquen les seves condicions
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace RPG {

// ============ EnhanceHooks (Fase 1: Passius + Contextuals) ============
//
// Enhancements PASSIVOS: bonificadors que el BattleState/GameSession calcula
// ANTES de tirar la pool Nd6. NUNCA tocan los dados después de tirar (salvo
// los 3 hooks explicitamente permitidos: post_bonus, upgrade_botch,
// upgrade_partial).

enum class ContextBonusFlag : uint32_t {
    NONE = 0,
    FLANKED_ENEMY       = 1u << 0,
    HIGH_GROUND         = 1u << 1,
    SURPRISED_ENEMY     = 1u << 2,
    INSPIRED            = 1u << 3,
    FATIGUED            = 1u << 4,
    EQUIP_INCOMPATIBLE  = 1u << 5,
    ENEMY_BLEEDING      = 1u << 6,  // si objectiu és sagnat
    CASTER_LOW_HP       = 1u << 7,  // < 30% PV: +0.5 èxit (per "berserk" passives)
    TARGET_CA_FLANKED   = 1u << 8
};

inline ContextBonusFlag operator|(ContextBonusFlag a, ContextBonusFlag b) {
    return static_cast<ContextBonusFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline ContextBonusFlag operator&(ContextBonusFlag a, ContextBonusFlag b) {
    return static_cast<ContextBonusFlag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline bool has_flag(ContextBonusFlag x, ContextBonusFlag f) {
    return (static_cast<uint32_t>(x) & static_cast<uint32_t>(f)) != 0;
}

// C-22: IQteProvider y QteResult retirados — no tenían ninguna
// implementación (el paso de timing de SkillExecutor era inabastable:
// todos los llamadores pasaban nullptr). Cuando el timing vuelva, vuelve
// con implementación real, no con la interfaz huérfana.

    // Parámetros de entrada a la Ejecución de una Skill (ver SkillExecutor abajo).
struct ExecutionContext {
    // Orígenes del cálculo. Se usan para generar los bonuses pasivos.
    // Si no tienes un CharacterSheet* (porque estás en Render/ICombatant),
    // usa los overrides de más abajo (≥0 toman precedencia).
    const class CharacterSheet* caster = nullptr;
    const class CharacterSheet* target = nullptr;
    const class SkillDefinition* skill = nullptr;

    ContextBonusFlag flags = ContextBonusFlag::NONE;
    class RandomEngine* rng = nullptr; // OBLIGATORIO

    bool is_player_attacking = true;

    // —— SALIDA (Se computa en SkillExecutor antes de tirar la pool) ——
    int extra_dice = 0;
    DiceMod force_dice_mod = DiceMod::NORMAL;
    float post_roll_success_bonus = 0.0f; // Se suma A LOS ÉXITOS DESPUÉS.

    bool can_upgrade_botch_to_partial = false; // Protegido novato/Passiva.
    bool can_upgrade_partial_to_success = false; // (Inspired spend, PASSIV "ferro")
    bool perfect_critical_confirm_allowed = false; // Timing PERFECT + SUCCESS → intenta 1d6 extra crític.

    // —— OVERRIDES (para integración Render/ICombatant sin CharacterSheet) ——
    // Si ≥0: SkillExecutor usa este valor DIRECTAMENTE y no lee
    // caster/target CharacterSheet. Ideal para capas adaptador como
    // Skill.cpp / BattleState.cpp que solo tienen acceso a ICombatant*
    // (Render) y no quieren romper la separación modelo/vista.
    int override_base_dice = -1;
    // Ed. 3 (bitácora 2.17): el defensor TAMBIÉN tira. Si ≥0, el defensor
    // tira este número de dados y la resolución es enfrentada
    // (opposed_degree). Sin defensor (target==nullptr y esto <0) la skill
    // no se resiste (curaciones, buffs): grado por la tirada propia.
    int override_target_dice = -1;
    // Modificadores del DEFENSOR (Oriol, 17/08): si el defensor lleva un
    // bufo/condición que da +1 dado (o ventaja/desventaja), se cuenta —
    // igual que para el atacante. Quien monte la enfrentada los rellena;
    // cuando el combate alimente condiciones al defensor, salen de aquí.
    int defender_extra_dice = 0;
    DiceMod defender_dice_mod = DiceMod::NORMAL;
    // C-22: override_target_defense retirado — EN DESUSO desde Ed. 3 y ya
    // sin ningún uso (el executor lo ignoraba y nadie lo rellenaba).
};

// Orquestador FINAL que respeta invariante Nd6 siempre.
class SkillExecutor {
public:
    struct Out {
        PoolResult pool;
        // Ed. 3: la tirada del defensor (visible en HUD/log: una tirada que
        // no se puede ver no se puede discutir). Vacía si no hubo defensor.
        PoolResult defender_pool;
        bool opposed = false;  // true si hubo tirada del defensor
        Degree final_degree;
        float effective_successes;
        int final_magnitude = 0;
        // D-09: condicio pendent d'aplicar. Buida si la carta no en porta
        // o el grau no arriba al minim (apply_condition_min_degree).
        // L'executor no toca fitxes: ho reporta i el combat ho aplica
        // (BattleState via ConditionEngine). Rondes segons la carta
        // (condition_duration_rounds), mai 0.
        std::string pending_condition_id;
        int pending_condition_rounds = 0;
        // D-11 O2: invocació pendent. Buida si la carta no en porta o el
        // grau no arriba al minim ( summon_min_degree, per defecte SUCCESS).
        // L'executor no materialitza: ho reporta i el combat ho
        // materialitza (BattleState via GameSession::spawnEnemyFromCatalog).
        std::string pending_summon_id;
        // D-11: vocabulari d'efectes.
        std::string effect_family;
        bool effect_supported = true;
        std::string effect_diagnostic;
        std::vector<std::string> applied_effects;
        std::vector<std::string> applied_enhancements;
        // El crítico de la propia tirada (dos 6 o más, D0013) dio el ×1,5.
        // El nombre viene de cuando pedía todos los dados en 6.
        bool was_all_sixes_critical = false;
    };

    // ENTRY POINT.
    static Out execute(ExecutionContext& ctx);
};

} // namespace RPG
