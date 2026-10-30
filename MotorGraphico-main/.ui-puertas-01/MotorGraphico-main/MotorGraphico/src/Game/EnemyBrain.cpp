#include "Game/EnemyBrain.h"

#include <algorithm>

EnemyBrain::EnemyBrain(GridCoord position, GridCoord patrolMin, GridCoord patrolMax, int maxHealth,
                       float stepInterval)
    // EnemyBrainData conserva los clamps que antes vivian aqui: vida minima
    // para que un enemigo no nazca muerto e intervalo minimo para que el
    // bucle de patrulla no pueda quedarse infinito en un frame.
    : m_data(position, patrolMin, patrolMax, maxHealth, stepInterval) {}

void EnemyBrain::takeDamage(int amount) {
    m_data.health = std::max(0, m_data.health - amount);
}

// Mismo clamp que Player::heal()/Enemy::heal(): entre 0 y el máximo de datos,
// nunca por encima.
void EnemyBrain::heal(int amount) {
    m_data.health = std::clamp(m_data.health + amount, 0, m_data.maxHealth);
}
