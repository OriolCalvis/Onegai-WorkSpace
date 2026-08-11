#include "Render/Player.h"

#include <algorithm>

#include "Engine/InputState.h"

Player::Player(GridCoord gridPosition, TextureAtlas* atlas, int tileWidth, int tileHeight)
    : AnimatedEntity(gridPosition, atlas, tileWidth, tileHeight) {}

void Player::update(float deltaTime) {
    // El movimiento ya se resolvio en handleInput() (discreto, no depende
    // del tiempo); aqui solo se mantiene viva la animacion actual.
    AnimatedEntity::update(deltaTime);
}

void Player::handleInput(const InputState& input) {
    GridCoord pos = gridPosition();

    if (input.moveUp) {
        pos.y -= 1;
        play("walk_up");
    } else if (input.moveDown) {
        pos.y += 1;
        play("walk_down");
    } else if (input.moveLeft) {
        pos.x -= 1;
        play("walk_left");
    } else if (input.moveRight) {
        pos.x += 1;
        play("walk_right");
    } else {
        play("idle");
    }

    setGridPosition(pos);
}

void Player::takeDamage(int amount) { m_health = std::max(0, m_health - amount); }

// CORRECCION (Fase 7, al anadir ICombatant::maxHealth()): la version
// anterior no clampaba contra ningun maximo (std::max(0, ...) solo evita
// bajar de 0), asi que una curacion podia dejar m_health por encima de lo
// que el propio Player considera su vida maxima. No se via desde fuera
// (nada leia maxHealth() todavia), pero con Skill::ApplySkillEffect()
// (Skill.h) usando heal() de verdad, un HudBar (setMaxValue(maxHealth()))
// mostraria una barra desbordada. std::clamp contra [0, m_maxHealth].
void Player::heal(int amount) { m_health = std::clamp(m_health + amount, 0, m_maxHealth); }

void Player::addItem(int itemID) { m_inventory.push_back(itemID); }
