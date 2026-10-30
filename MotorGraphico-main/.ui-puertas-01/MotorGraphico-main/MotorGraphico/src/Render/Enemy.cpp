#include "Render/Enemy.h"

Enemy::Enemy(GridCoord gridPosition, GridCoord patrolMin, GridCoord patrolMax, TextureAtlas* atlas,
             int tileWidth, int tileHeight, float stepInterval)
    : AnimatedEntity(gridPosition, atlas, tileWidth, tileHeight)
    // 50 de vida por defecto: el valor historico de esta clase, que los
    // demos dan por hecho. Los enemigos del JUEGO no pasan por aqui --
    // GameSession construye su EnemyBrain con el maxHealth del catalogo.
    , m_brain(gridPosition, patrolMin, patrolMax, /*maxHealth=*/50, stepInterval) {}

void Enemy::update(float deltaTime) {
    AnimatedEntity::update(deltaTime);

    // La patrulla la decide el brain; aqui solo se refleja. Sin callback
    // de colision (siempre puede entrar): esta clase se usa en demos sin
    // mapa, y quien SI tiene mapa (GameSession) mueve sus enemigos por su
    // cuenta con el criterio de colision real.
    const GridCoord before = m_brain.position();
    m_brain.update(deltaTime, [](const GridCoord&) { return true; });
    const GridCoord after = m_brain.position();

    if (after.x != before.x || after.y != before.y) {
        setGridPosition(after);
        play(m_brain.direction() > 0 ? "walk_right" : "walk_left");
    }
}

void Enemy::takeDamage(int amount) { m_brain.takeDamage(amount); }
void Enemy::heal(int amount) { m_brain.heal(amount); }
