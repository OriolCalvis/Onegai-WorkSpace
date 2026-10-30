#pragma once

#include <cstdint>

namespace RPG {

// Objetivo canónico de una habilidad. Lo consume el catálogo de cartas y
// también la capa de combate; no hay una segunda representación en Game.
enum class SkillTarget : uint8_t {
    SELF = 0,
    SINGLE_ENEMY,
    ALL_ENEMIES,
    SINGLE_ALLY,
    ALL_ALLIES,
    ZONE_RADIUS,

    // Grafías del API Game anterior. Son alias del mismo enum para que los
    // llamadores heredados no tengan que cambiar a la vez que Application.
    Self = SELF,
    SingleEnemy = SINGLE_ENEMY,
    AllEnemies = ALL_ENEMIES,
    SingleAlly = SINGLE_ALLY,
    AllAllies = ALL_ALLIES,
    ZoneRadius = ZONE_RADIUS
};

}  // namespace RPG
