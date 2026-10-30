// EnemyBrainData — estado GL-free de un enemigo del mundo.
//
// EnemyBrain dirige patrulla y contrato ICombatant; esta clase guarda el
// estado que ese flujo manipula. No conoce GameSession, render ni OpenGL.
// Mantenerlo separado permite probar y persistir las invariantes sin hacer
// crecer al controlador con otra representación paralela del enemigo.

#pragma once

#include <algorithm>
#include <array>
#include <string>

#include "Core/Math/GridCoord.h"
#include "RPG/CharacterSheet.h"

struct EnemyBrainData {
    static constexpr int kIdle = 0;
    static constexpr int kPatrol = 1;
    static constexpr int kChase = 2;

    EnemyBrainData() = default;

    EnemyBrainData(GridCoord initialPosition, GridCoord initialPatrolMin,
                   GridCoord initialPatrolMax, int initialMaxHealth,
                   float initialStepInterval)
        : position(initialPosition)
        , patrolMin(initialPatrolMin)
        , patrolMax(initialPatrolMax)
        , health(initialMaxHealth)
        , maxHealth(initialMaxHealth)
        , stepInterval(initialStepInterval) {
        normalizeInitialState();
        // Un EnemyBrain nuevo siempre arranca lleno. Conserva el contrato del
        // constructor anterior cuando el catálogo trae vida 0 o negativa.
        health = maxHealth;
    }

    GridCoord position;
    GridCoord patrolMin;
    GridCoord patrolMax;
    int health = 100;
    int maxHealth = 100;
    int aiState = kPatrol;
    int direction = 1;
    float stepInterval = 0.5f;
    float stepElapsed = 0.0f;
    std::array<int, 4> stats = {0, 0, 0, 0};
    int equipArmor = 0;
    int tier = 1;
    std::string combatantId = "enemy";
    RPG::CharacterSheet* sheet = nullptr;  // no propietaria; la posee GameSession.

private:
    // Conserva los clamps que antes vivían en EnemyBrain. No exponemos una
    // API de restauración todavía: no hay una ruta de guardado que la use.
    void normalizeInitialState() noexcept {
        maxHealth = std::max(maxHealth, 1);
        health = std::clamp(health, 0, maxHealth);
        stepInterval = std::max(stepInterval, 0.01f);
        tier = std::clamp(tier, 1, 5);
        if (direction != -1 && direction != 1) direction = 1;
    }
};
