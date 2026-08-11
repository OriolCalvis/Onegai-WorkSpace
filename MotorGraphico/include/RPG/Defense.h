#pragma once

#include <array>
#include <cstdint>

#include "RPG/Stat.h"

namespace RPG {

// 4 Defensas (GDD Llibre II §5.2 Fórmules Definitives, más "Precisió Màgica"
// de NECESSIDADES P0-1 renombrado a SpellSaveDC porque semánticamente es
// "qué dificultad tienen los demás para salvar contra tus hechizos").
//
//  - ARMOR_CLASS  : CA Física   = 10 + DES + eqArmor (+ alternativa CON si
//                  armor pesada tancada)
//  - PHYSICAL_SAVE: Resistència Física = 10 + CON (verins, malalties, gas)
//  - WILL_SAVE    : Voluntat/CA Mental = 10 + CAR (encantaments, por)
//  - SPELL_SAVE_DC: Dificultat Salvament contra els meus encantaments =
//                   8 + INT + proficiency_like (si cal; per ara 8 + INT)
enum class Defense : uint8_t {
    ARMOR_CLASS = 0,
    PHYSICAL_SAVE = 1,
    WILL_SAVE = 2,
    SPELL_SAVE_DC = 3
};

constexpr int DEFENSE_COUNT = 4;

struct DefenseBlock {
    // Para cada Defense (0..3), el valor ENTERO final (10+stat+eq).
    // Se usa con DicePoolEngine::defense_to_cd() para pasar a float Cd 0..3.
    std::array<int, DEFENSE_COUNT> values = {10, 10, 10, 8};

    int get(Defense d) const { return values[static_cast<int>(d)]; }
    void set(Defense d, int v) { values[static_cast<int>(d)] = v; }

    // Hepler para HUD-log rápido.
    int ca_fisica() const { return get(Defense::ARMOR_CLASS); }
    int save_fis()  const { return get(Defense::PHYSICAL_SAVE); }
    int save_vol()  const { return get(Defense::WILL_SAVE); }
    int dc_mag()    const { return get(Defense::SPELL_SAVE_DC); }
};

} // namespace RPG
