#pragma once

#include <cstdint>
#include <string>

namespace RPG {

// 4 atributos base del sistema (Golden Core GDD Llibre II §5.1).
// Todos empiezan en 0 en el CharacterSheet; solo reciben bonificadores
// de: Equipo, Raza, Passives, Condiciones. No hay Point-Buy.
enum class Stat : uint8_t { CON = 0, DES = 1, INT = 2, CAR = 3 };

// Rangos máximos por tier (lo define TierRules). Por ahora clamp 0..8.
constexpr int STAT_RANK_MIN = 0;
constexpr int STAT_RANK_MAX = 8;

inline std::string stat_name(Stat s) {
    switch (s) {
        case Stat::CON: return "CON";
        case Stat::DES: return "DES";
        case Stat::INT: return "INT";
        case Stat::CAR: return "CAR";
    }
    return "?";
}

inline Stat stat_from_index(int i) {
    switch (i & 3) {
        case 0: return Stat::CON;
        case 1: return Stat::DES;
        case 2: return Stat::INT;
        case 3: return Stat::CAR;
    }
    return Stat::CON;
}

} // namespace RPG
