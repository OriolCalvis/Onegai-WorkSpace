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

// Qué resultados puede devolver un QTE timing, para Enhance Layer (Fase 2-3).
enum class QteResult { MISSED = 0, GOOD = 1, PERFECT = 2 };

// Interfaz inyectable que implementa Application (Render/) con el HUD de
// timing. BattleStateOnegai (GL-free) la CONSULTA — nunca dibuja.
class IQteProvider {
public:
    virtual ~IQteProvider() = default;
    // Ventana de timing para ESQUIVAR un ataque enemigo → DES salvage.
    virtual QteResult do_reaction_dodge_qte(int window_ms, const std::string& enemy_skill_id) = 0;
    // Ventana de timing para ATAQUE del jugador.
    virtual QteResult do_attack_timing_qte(int window_ms, const std::string& skill_id) = 0;
};

    // Parámetros de entrada a la Ejecución de una Skill (ver SkillExecutor abajo).
struct ExecutionContext {
    // Orígenes del cálculo. Se usan para generar los bonuses pasivos.
    // Si no tienes un CharacterSheet* (porque estás en Render/ICombatant),
    // usa los overrides de más abajo (≥0 toman precedencia).
    const class CharacterSheet* caster = nullptr;
    const class CharacterSheet* target = nullptr;
    const class SkillDefinition* skill = nullptr;

    ContextBonusFlag flags = ContextBonusFlag::NONE;
    IQteProvider* qte = nullptr;
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
    // EN DESUSO desde Ed. 3: la CA/CD se eliminan. Se conserva el campo
    // para no romper firmas hasta retirar los últimos usos; el executor
    // ya lo ignora.
    int override_target_defense = -1;  // Entero defensa (CA, TS Física, etc.)
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
        std::vector<std::string> applied_enhancements; // HUD log
        bool was_all_sixes_critical = false;
    };

    // ENTRY POINT.
    static Out execute(ExecutionContext& ctx);
};

} // namespace RPG
