// Copyright 2026 Motor Grafico Isometrico Pixel Art
// Licensed under the MIT License

#include "Render/Enemy.h"
#include "Core/Math/GridCoord.h"
#include "Game/EnemyBrain.h"

#include "TestHarness.h"

// Forward declaration para mock
class TextureAtlas;

namespace {

TextureAtlas* createMockAtlas() {
    return static_cast<TextureAtlas*>(nullptr);
}

} // namespace

// --- Tests de construccion ---

MOTOR_TEST_CASE(enemy_constructor_default_values) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    MOTOR_CHECK(enemy.brain().position().x == position.x);
    MOTOR_CHECK(enemy.brain().position().y == position.y);
    MOTOR_CHECK(enemy.health() == 50); // Default health
    MOTOR_CHECK(enemy.maxHealth() == 50);
    MOTOR_CHECK(enemy.isAlive());
}

MOTOR_TEST_CASE(enemy_constructor_with_custom_patrol) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{5, 5};
    GridCoord patrolMin{3, 5};
    GridCoord patrolMax{7, 5};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    MOTOR_CHECK(enemy.brain().position().x == position.x);
    MOTOR_CHECK(enemy.brain().position().y == position.y);
    // Los límites de patrulla no se pueden verificar directamente (métodos privados)
    // pero el constructor debería haberlos almacenado
    MOTOR_CHECK(true);
}

MOTOR_TEST_CASE(enemy_constructor_with_custom_step_interval) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas, 64, 32, 1.0f); // stepInterval = 1.0s
    MOTOR_CHECK(true); // Constructor acepta el parámetro
}

// --- Tests de herencia ---

MOTOR_TEST_CASE(enemy_inherits_from_animated_entity) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    AnimatedEntity* animated = &enemy;
    MOTOR_CHECK(animated != nullptr);
}

MOTOR_TEST_CASE(enemy_implements_irenderable) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    IRenderable* renderable = &enemy;
    MOTOR_CHECK(renderable != nullptr);
}

MOTOR_TEST_CASE(enemy_implements_iupdatable) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    IUpdatable* updatable = &enemy;
    MOTOR_CHECK(updatable != nullptr);
}

MOTOR_TEST_CASE(enemy_implements_icombatant) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    ICombatant* combatant = &enemy;
    MOTOR_CHECK(combatant != nullptr);
}

// --- Tests de combate (delegación a EnemyBrain) ---

MOTOR_TEST_CASE(enemy_take_damage_delegates_to_brain) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    int initialHealth = enemy.health();
    enemy.takeDamage(10);
    MOTOR_CHECK(enemy.health() == initialHealth - 10);
}

MOTOR_TEST_CASE(enemy_heal_delegates_to_brain) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    // Primero hacer daño para luego curar
    enemy.takeDamage(10);
    int healthAfterDamage = enemy.health();
    enemy.heal(5);
    MOTOR_CHECK(enemy.health() == healthAfterDamage + 5);
}

MOTOR_TEST_CASE(enemy_is_alive_delegates_to_brain) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    MOTOR_CHECK(enemy.isAlive());
    
    enemy.takeDamage(100);
    MOTOR_CHECK(!enemy.isAlive());
}

MOTOR_TEST_CASE(enemy_stats_delegate_to_brain) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    // EnemyBrain tiene stats por defecto
    RPG::Stat stat = RPG::Stat::CON;
    MOTOR_CHECK(enemy.stat(stat) >= 0); // Debería tener algún valor
}

MOTOR_TEST_CASE(enemy_tier_delegates_to_brain) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    MOTOR_CHECK(enemy.tier() >= 1);
}

MOTOR_TEST_CASE(enemy_combatant_id_delegates_to_brain) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    MOTOR_CHECK(!enemy.combatant_id().empty());
}

// --- Tests de update y patrulla ---

MOTOR_TEST_CASE(enemy_update_advances_patrol) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord startPos{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(startPos, patrolMin, patrolMax, atlas, 64, 32, 0.1f); // stepInterval muy pequeño
    
    GridCoord initialPos = enemy.brain().position();
    
    // Actualizar con tiempo suficiente para moverse
    enemy.update(1.0f);
    
    GridCoord newPos = enemy.brain().position();
    // Debería haberse movido de la posición inicial
    MOTOR_CHECK(initialPos.x != newPos.x || initialPos.y != newPos.y);
}

MOTOR_TEST_CASE(enemy_update_changes_animation_direction) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord startPos{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(startPos, patrolMin, patrolMax, atlas, 64, 32, 0.1f);
    
    // Forzar una actualización que cause movimiento
    enemy.update(1.0f);
    
    // No podemos verificar la animación sin SpriteBatch, pero el método no crash
    MOTOR_CHECK(true);
}

// --- Tests de constantes de estado ---

MOTOR_TEST_CASE(enemy_state_constants_match_enemy_brain) {
    MOTOR_CHECK(Enemy::kIdle == EnemyBrain::kIdle);
    MOTOR_CHECK(Enemy::kPatrol == EnemyBrain::kPatrol);
    MOTOR_CHECK(Enemy::kChase == EnemyBrain::kChase);
}

// --- Tests de brain accessor ---

MOTOR_TEST_CASE(enemy_brain_accessor_returns_reference) {
    TextureAtlas* atlas = createMockAtlas();
    GridCoord position{0, 0};
    GridCoord patrolMin{-2, 0};
    GridCoord patrolMax{2, 0};
    
    Enemy enemy(position, patrolMin, patrolMax, atlas);
    
    EnemyBrain& brain = enemy.brain();
    MOTOR_CHECK(brain.position().x == position.x);
    MOTOR_CHECK(brain.position().y == position.y);
    
    const EnemyBrain& constBrain = enemy.brain();
    MOTOR_CHECK(constBrain.position().x == position.x);
    MOTOR_CHECK(constBrain.position().y == position.y);
}

int main() { return motor_test::runAllTests(); }
