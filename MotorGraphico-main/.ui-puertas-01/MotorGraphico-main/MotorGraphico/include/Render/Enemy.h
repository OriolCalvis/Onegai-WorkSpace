#pragma once

#include "Core/Math/GridCoord.h"
#include "Game/EnemyBrain.h"
#include "Render/AnimatedEntity.h"
#include "Render/ICombatant.h"

// Enemigo con IA minima (motor_grafico_clases.puml): patrulla
// deterministica de ida y vuelta entre patrolMin.x y patrolMax.x, un paso
// de celda cada m_stepInterval segundos. Deliberadamente simple y sin
// aleatoriedad (a diferencia de Player, aqui update() SI mueve la
// entidad): es la base sobre la que anadir deteccion de Player/persecucion
// mas adelante (kChase, ver mas abajo), no la IA final del motor.
//
// : public ICombatant (Fase 7 de motor_grafico_gantt_rpg.puml), mismo
// motivo que en Player: formaliza takeDamage()/health()/isAlive() (que ya
// tenia) como el contrato que usa Skill::ApplySkillEffect().
//
// Fractura #1 del analisis de coherencia (RESUELTA, ver ARCHITECTURE.md):
// Enemy es ahora solo el SPRITE del enemigo (animacion + dibujado). Todo
// lo que el enemigo ES -- vida, patrulla, estado de IA -- vive en un
// EnemyBrain que esta clase compone (ver EnemyBrain.h) y al que delega.
// Antes habia DOS clases de enemigo sin relacion: esta (con sprite + IA,
// pero que el juego real no instanciaba) y CatalogCombatant (oculta en
// GameSession.cpp, solo vida, sin patrulla). Esa duplicacion se elimino:
// CatalogCombatant dejo de existir, EnemyBrain es la unica logica de
// enemigo, y Enemy la muestra. Enemy sigue siendo ICombatant (delegando
// en su EnemyBrain) para que BattleState la use igual que antes.
class Enemy : public AnimatedEntity, public ICombatant {
public:
    // Alias de los estados de EnemyBrain: el codigo que ya escribia
    // Enemy::kPatrol sigue compilando, y hay UNA definicion de los
    // valores, no dos que se puedan desincronizar.
    static constexpr int kIdle = EnemyBrain::kIdle;
    static constexpr int kPatrol = EnemyBrain::kPatrol;
    static constexpr int kChase = EnemyBrain::kChase;

    // stepInterval (segundos por celda de patrulla) no esta en el
    // diagrama de clases, mismo criterio que AnimatedEntity::frameTime:
    // sin el, update() no sabria a que ritmo avanzar la patrulla.
    Enemy(GridCoord gridPosition, GridCoord patrolMin, GridCoord patrolMax, TextureAtlas* atlas,
          int tileWidth = 64, int tileHeight = 32, float stepInterval = 0.5f);

    // Avanza la animacion (AnimatedEntity::update) y, cada
    // m_stepInterval segundos, un paso de patrulla: +1 o -1 en grid_x,
    // invirtiendo direccion al tocar patrolMin.x/patrolMax.x. grid_y no
    // cambia (patrulla en linea recta sobre una fila).
    void update(float deltaTime) override;

    // --- ICombatant: TODO delegado en el brain. No hay una segunda copia
    // de la vida aqui que pudiera desincronizarse de la del combate. ---
    int health() const override { return m_brain.health(); }
    int maxHealth() const override { return m_brain.maxHealth(); }
    int aiState() const { return m_brain.aiState(); }

    // --- ICombatant RPG: delegación 1:1 en el brain. ---
    int stat(RPG::Stat s) const override { return m_brain.stat(s); }
    RPG::DefenseBlock defenses() const override { return m_brain.defenses(); }
    int defense_value(RPG::Defense d) const { return ICombatant::defense_value(d); }
    int initiative_bonus() const override { return m_brain.initiative_bonus(); }
    int tier() const override { return m_brain.tier(); }
    std::string combatant_id() const override { return m_brain.combatant_id(); }

    // Acceso a la logica, para quien necesite algo mas que el contrato
    // (por ejemplo, un GameSession que quiera consultar la patrulla).
    EnemyBrain& brain() { return m_brain; }
    const EnemyBrain& brain() const { return m_brain; }

    // No esta en el diagrama de clases: sin el, m_health seria de solo
    // lectura y no habria forma de probarlo (mismo criterio que
    // Player::takeDamage).
    void takeDamage(int amount) override;
    // No estaba en la version original (los enemigos de los demos nunca
    // se curaban): la anade ICombatant como parte del contrato -- un
    // enemigo con una habilidad de curacion (ver
    // assets/levels/test_level.json, "grito_de_guerra" es dano en area,
    // pero nada impide un futuro "grito_curativo") ya tiene donde
    // aplicarla sin caso especial en el sistema de habilidades.
    void heal(int amount) override;
    bool isAlive() const override { return m_brain.isAlive(); }

private:
    // Composicion, no herencia: Enemy YA hereda de AnimatedEntity (base
    // concreta) y de ICombatant (interfaz). Heredar tambien de EnemyBrain
    // seria una segunda base concreta, justo lo que prohibe la regla
    // morfologica de ARCHITECTURE.md.
    EnemyBrain m_brain;
};
