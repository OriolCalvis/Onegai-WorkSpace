#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "RPG/DicePoolEngine.h"
#include "RPG/Stat.h"
#include "RPG/TierRules.h"

namespace RPG {

// ========== SkillDefinition (Onegai Nd6 + GDD fusionado) ==========
//
// Correspondencia 1:1 con la salida de tools/convertir_definiciones_rpg.py
// (assets/catalogs/skills.json y spells.json).
//
// "Que hace" la carta (magnitud en 4 grados) NO varía en runtime; son datos
// constantes del catalogo. Las variantes por contexto (ventaja, bonus
// pasivos, timing QTE) se aplican en SkillExecutor (ver EnhanceHooks.h).

enum class ActionType : uint8_t {
    ACCION_PRINCIPAL = 0, // La mayoria
    MOVEMENT         = 1, // 👣 Desplazamiento o tecnica de movimiento
    REACTION         = 2, // 🛡 1/ronda maximo
    CHANNEL          = 3, // ⏳ Canaliza N turnos
    FREE_ACTION      = 4, // Accion gratuita (cambia stance, habla...)
    PASIVA           = 5  // No juega (pasiva otorgada por otra cosa)
};

// Recuperación / pila
enum class RecoveryPile : uint8_t {
    NONE        = 0, // No recupera: consumible o de un solo uso
    ACTIVE      = 1, // Cada turno (vuelve a la mano si no se gastó ya? No: 3 pilas Onegai, ACTIVE = "va al mazo activo al descanso corto?")
    SHORT_REST  = 2, // Pila descanso corto. GDD Onegai.
    LONG_REST   = 3, // Pila descanso largo (por defecto la mayoria)
    PASSIVE     = 4  // No se juega
};

// Target. Onegai trabaja con esto y con zonas 1x1 / 3x3 / radius; pero en
// MVP Fase A solo 6 valores básicos (mismo que Skill.h del motor).
enum class SkillTarget : uint8_t {
    SELF = 0,
    SINGLE_ENEMY,
    ALL_ENEMIES,
    SINGLE_ALLY,
    ALL_ALLIES,
    ZONE_RADIUS
};

// Salvamento del objetivo.
enum class SaveAttribute : uint8_t { NONE = 0, CON, DES, INT, CAR };

// Que significa magnitude_by_degree (danyo/cura/mover...).
enum class MagnitudeType : uint8_t {
    DAMAGE = 0,
    HEAL,
    MOVE_DISTANCE,
    CONDITION_DURATION,
    OTHER
};

// ==== Helpers string->enum ====
// Usados al parsear JSON de skills.json (ver convertir_definiciones_rpg.py
// recovery "activo"/"descanso_corto"/"descanso_largo"/"pasiva" y actionType).
ActionType     action_type_from_string(const std::string& s);
std::string    to_string(ActionType t);

RecoveryPile   recovery_from_string(const std::string& s);
std::string    to_string(RecoveryPile r);

SkillTarget    skill_target_from_string(const std::string& s);
std::string    to_string(SkillTarget t);

SaveAttribute  save_attribute_from_string(const std::string& s);
std::string    to_string(SaveAttribute t);

struct SkillDefinition {
    // ==== Campos catalogo JSON ====
    std::string id;
    std::string name;
    std::string description;    // Breve para el HUD
    std::string flavor_text;

    int tier_min = 0;
    ItemRarity rarity = ItemRarity::Common;

    ActionType action_type = ActionType::ACCION_PRINCIPAL;
    int channel_rounds = 0;
    RecoveryPile recovery = RecoveryPile::LONG_REST;
    SkillTarget target = SkillTarget::SINGLE_ENEMY;

    Stat casting_stat = Stat::DES; // N dados de la pool = stat_total(casting_stat)
    SaveAttribute save_attribute = SaveAttribute::NONE;
    float override_cd_if_save = 0.0f; // 0.0 = usa el DC por stat

    // ===== Nd6: Magnitud por grado de éxito =====
    // Orden: [BOTCH, PARTIAL, SUCCESS, CRITICAL]
    // Mismo que antes pero explicitamos indices y hacemos float para ser
    // compatible con mitjes 0.5: si el JSON original trae 4.5 lo podemos
    // guardar (luego SkillExecutor lo trunca a int o no, a gusto del caller).
    float magnitude_by_degree[4] = { 0.0f, 2.0f, 4.0f, 8.0f };
    MagnitudeType magnitude_type = MagnitudeType::DAMAGE;

    // Aplicar condición (ver ConditionDefinition y ConditionEngine Fase B).
    std::string apply_condition_id;
    Degree apply_condition_min_degree = Degree::SUCCESS;
    int condition_duration_rounds = 3;

    // Etiquetas sinergias
    std::vector<std::string> tags;
    std::vector<std::string> required_tags;
    std::vector<std::string> incompatible_tags;

    // Si es summon: carta de invocación -> id del summon catalogado
    std::string granted_summon_id;

    // ==== Helpers ====
    bool has_tag(const std::string& t) const {
        for (auto& x : tags) if (x == t) return true;
        return false;
    }
    float magnitude_f(Degree d) const {
        int i = static_cast<int>(d);
        if (i < 0 || i > 3) return 0.0f;
        return magnitude_by_degree[i];
    }
    int magnitude(Degree d) const {
        return static_cast<int>(magnitude_f(d));
    }
    int tier_min_clamped() const {
        if (tier_min < TIER_MIN) return TIER_MIN;
        if (tier_min > TIER_MAX) return TIER_MAX;
        return tier_min;
    }
};

// Presets para tests y cartas iniciales generadas a mano (mantener por
// compatibilidad con el código que los usaba). No son una fuente de verdad
// para el juego en sí (la fuente de verdad es skills.json).
struct MagnitudePreset {
    const char* name;
    float mag[4];
};
constexpr MagnitudePreset MAGNITUDE_PRESETS[] = {
    {"Petit (1d6 col·loquial)",   {0.0f, 2.0f,  4.0f,  8.0f}},
    {"Mitjà (1d8)",               {0.0f, 3.0f,  6.0f, 12.0f}},
    {"Gran  (1d10)",              {0.0f, 4.0f,  8.0f, 16.0f}},
    {"Llegendari (1d12)",         {0.0f, 6.0f, 12.0f, 24.0f}},
    {"Curació Petit",             {0.0f, 2.0f,  4.0f,  8.0f}},
    {"Curació Gran",              {0.0f, 5.0f, 10.0f, 20.0f}},
};

}  // namespace RPG
