#pragma once

#include <array>
#include <string>

#include "RPG/Defense.h"
#include "RPG/Stat.h"

// Interfaz de "participante en combate Onegai Nd6".
//
// FASE 0 → extendida (P0-1). Antes solo tenia takeDamage/heal/health
// (Golden Core 2024). Ahora expone stats, defensas, iniciativa, tier y
// una id semántica para log/HUD. Player.h y Enemy.h (Render/) ya
// implementaban takeDamage(); ahora extienden la interfaz entera.
//
// SEPARACIÓN MODELO/VISTA (ARCHITECTURE.md): el motor RPG (GL-free) solo
// conoce ICombatant. NO conoce Player/Enemy.h (Render/). Así BattleState,
// DicePoolEngine y SkillExecutor pueden probarse sin contexto GL.
class ICombatant {
public:
    virtual ~ICombatant() = default;

    // ==== (existentes, no se tocan) ====
    virtual void takeDamage(int amount) = 0;
    virtual void heal(int amount) = 0;
    virtual int health() const = 0;
    virtual int maxHealth() const = 0;
    virtual bool isAlive() const = 0;

    // ==== (Nuevos, P0-1) ====

    // Rango de un stat (CON/DES/INT/CAR). Valor 0..8 típico; 0 = base
    // inicial del GDD (0 pura; equipo/raza añaden).
    virtual int stat(RPG::Stat s) const = 0;

    // Bloque completo de defensas enteras (10+stat+eq). Se convierte a Cd
    // float con DicePoolEngine::defense_to_cd() en los resolutores.
    virtual RPG::DefenseBlock defenses() const = 0;

    // Bonus de iniciativa (se añade a la pool Nd6 opcional de iniciativa).
    // Por defecto = stat(DES). Pasivas pueden sobreescribir.
    virtual int initiative_bonus() const { return stat(RPG::Stat::DES); }

    // Tier 1..3 (I, II, III - Golden Core GDD).
    virtual int tier() const = 0;

    // Id semántico para log: "guerrera_maria", "llop_ferotge_03", etc.
    virtual std::string combatant_id() const = 0;

    // Hepler: devuelve una defensa concreta (más cómodo en código).
    int defense_value(RPG::Defense d) const { return defenses().get(d); }
};
