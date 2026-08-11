#include "Game/EnemyBrain.h"

#include <algorithm>

EnemyBrain::EnemyBrain(GridCoord position, GridCoord patrolMin, GridCoord patrolMax, int maxHealth,
                       float stepInterval)
    : m_position(position)
    , m_patrolMin(patrolMin)
    , m_patrolMax(patrolMax)
    // Vida minima 1: un enemigo con maxHealth 0 nace muerto y el combate
    // lo daria por terminado antes de empezar (mismo clamp defensivo que
    // hacia CatalogCombatant, la clase que esta sustituye).
    , m_health(std::max(maxHealth, 1))
    , m_maxHealth(std::max(maxHealth, 1))
    // Intervalo minimo: con 0 o negativo, el while de update() daria
    // pasos infinitos en un solo frame y colgaria el juego.
    , m_stepInterval(std::max(stepInterval, 0.01f)) {}

void EnemyBrain::takeDamage(int amount) { m_health = std::max(0, m_health - amount); }

// Mismo clamp que Player::heal()/Enemy::heal(): entre 0 y m_maxHealth,
// nunca por encima.
void EnemyBrain::heal(int amount) { m_health = std::clamp(m_health + amount, 0, m_maxHealth); }
