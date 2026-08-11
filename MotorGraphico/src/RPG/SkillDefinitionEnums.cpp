#include "RPG/Definitions/SkillDefinition.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <utility>

namespace RPG {

// ============================================================
// Helpers generales lowercase (solo para strings cortos de enum)
// ============================================================
static std::string lower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// ============================================================
// ActionType
// ============================================================
ActionType action_type_from_string(const std::string& s_in) {
    std::string s = lower(s_in);
    if (s.empty() || s == "accion_principal" || s == "principal" || s == "main_action" || s == "main") return ActionType::ACCION_PRINCIPAL;
    if (s == "movimiento" || s == "move" || s == "movement") return ActionType::MOVEMENT;
    if (s == "reaccion" || s == "reaction") return ActionType::REACTION;
    if (s == "canalizacion" || s == "channel" || s == "canalizar") return ActionType::CHANNEL;
    if (s == "libre" || s == "free" || s == "bonus" || s == "free_action") return ActionType::FREE_ACTION;
    if (s == "pasiva" || s == "passive" || s == "ninguno") return ActionType::PASIVA;
    return ActionType::ACCION_PRINCIPAL;
}
std::string to_string(ActionType t) {
    switch (t) {
        case ActionType::ACCION_PRINCIPAL: return "accion_principal";
        case ActionType::MOVEMENT:         return "movimiento";
        case ActionType::REACTION:         return "reaccion";
        case ActionType::CHANNEL:          return "canalizacion";
        case ActionType::FREE_ACTION:      return "libre";
        case ActionType::PASIVA:           return "pasiva";
    }
    return "accion_principal";
}

// ============================================================
// RecoveryPile
// ============================================================
RecoveryPile recovery_from_string(const std::string& s_in) {
    std::string s = lower(s_in);
    if (s == "activo" || s == "per_turn" || s == "cada_turno" || s == "per_ronda") return RecoveryPile::ACTIVE;
    if (s == "descanso_corto" || s == "short_rest" || s == "corto") return RecoveryPile::SHORT_REST;
    if (s == "descanso_largo" || s == "long_rest" || s == "largo") return RecoveryPile::LONG_REST;
    if (s == "pasiva" || s == "passive") return RecoveryPile::PASSIVE;
    if (s == "none" || s == "ninguno" || s.empty()) return RecoveryPile::NONE;
    return RecoveryPile::LONG_REST;
}
std::string to_string(RecoveryPile r) {
    switch (r) {
        case RecoveryPile::NONE:       return "ninguno";
        case RecoveryPile::ACTIVE:     return "activo";
        case RecoveryPile::SHORT_REST: return "descanso_corto";
        case RecoveryPile::LONG_REST:  return "descanso_largo";
        case RecoveryPile::PASSIVE:    return "pasiva";
    }
    return "descanso_largo";
}

// ============================================================
// SkillTarget
// ============================================================
SkillTarget skill_target_from_string(const std::string& s_in) {
    std::string s = lower(s_in);
    if (s == "self") return SkillTarget::SELF;
    if (s == "single_enemy" || s == "single enemy" || s == "enemigo") return SkillTarget::SINGLE_ENEMY;
    if (s == "all_enemies" || s == "todos_enemigos") return SkillTarget::ALL_ENEMIES;
    if (s == "single_ally" || s == "aliado") return SkillTarget::SINGLE_ALLY;
    if (s == "all_allies" || s == "todos_aliados") return SkillTarget::ALL_ALLIES;
    if (s == "zone" || s == "zona" || s == "radius" || s == "zone_radius") return SkillTarget::ZONE_RADIUS;
    return SkillTarget::SINGLE_ENEMY;
}
std::string to_string(SkillTarget t) {
    switch (t) {
        case SkillTarget::SELF:         return "self";
        case SkillTarget::SINGLE_ENEMY: return "single_enemy";
        case SkillTarget::ALL_ENEMIES:  return "all_enemies";
        case SkillTarget::SINGLE_ALLY:  return "single_ally";
        case SkillTarget::ALL_ALLIES:   return "all_allies";
        case SkillTarget::ZONE_RADIUS:  return "zone_radius";
    }
    return "single_enemy";
}

// ============================================================
// SaveAttribute
// ============================================================
SaveAttribute save_attribute_from_string(const std::string& s_in) {
    std::string s = lower(s_in);
    if (s == "con" || s == "constitucion") return SaveAttribute::CON;
    if (s == "des" || s == "destreza")     return SaveAttribute::DES;
    if (s == "int" || s == "inteligencia") return SaveAttribute::INT;
    if (s == "car" || s == "carisma")      return SaveAttribute::CAR;
    return SaveAttribute::NONE;
}
std::string to_string(SaveAttribute t) {
    switch (t) {
        case SaveAttribute::NONE: return "";
        case SaveAttribute::CON:  return "CON";
        case SaveAttribute::DES:  return "DES";
        case SaveAttribute::INT:  return "INT";
        case SaveAttribute::CAR:  return "CAR";
    }
    return "";
}

}  // namespace RPG
