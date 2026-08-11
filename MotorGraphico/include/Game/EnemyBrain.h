#pragma once

#include "Core/Math/GridCoord.h"
#include "Render/ICombatant.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/TierRules.h"

// Lo que un enemigo ES, sin nada de como se DIBUJA: vida, estado de IA y
// patrulla. GL-free a proposito (ni TextureAtlas, ni SpriteBatch, ni
// Entity), asi que GameSession -- que tampoco toca GL -- puede poseerlo
// directamente y probarse sin ventana.
//
// Resuelve la fractura #1 de ARCHITECTURE.md. Habia DOS representaciones
// paralelas del enemigo, sin relacion entre si:
//
//   - Enemy (Render/Enemy.h): sprite animado + IA de patrulla +
//     ICombatant, pero exige un TextureAtlas*, asi que GameSession no
//     podia usarla sin dejar de ser GL-free. Acabo muerta en el juego
//     real: solo la usaban los demos.
//   - CatalogCombatant (oculto en GameSession.cpp): solo vida, sin IA
//     ni patrulla. Era lo que el juego usaba de verdad.
//
// El coste de esa fractura era concreto y medible: LevelLoader parseaba
// patrolMin/patrolMax del JSON, WorldEnemy ni los guardaba, y NADIE
// movia a los enemigos. El dato de patrulla se leia y se tiraba -- los
// enemigos de la ciudad estaban clavados en el suelo mientras la IA que
// los movia existia, escrita y probada, en la clase muerta.
//
// La solucion no es fusionar las dos clases (una necesita GL y la otra
// no puede tenerlo), sino PARTIR por donde estaba la costura: aqui la
// logica, y el sprite (Enemy) leyendo de aqui.
//
// ====== Fractura #1 (extensión C4): CharacterSheet fuente única ======
//
// Añadimos un RPG::CharacterSheet* m_sheet externo no propietario.
// Si != nullptr: stat(), maxHealth(), tier(), combatant_id(), defenses()
// leen DESDE el sheet usando sus cached values (recompute_derived()).
// Si nullptr: legacy m_health/m_stats/m_tier igual que antes (backward
// compatible con todos los demos existentes).
//
// Además, populate_from_monster_catalog() rellena TANTO el sheet como
// los legacy defaults, partiendo de una MonsterDefinition del catalogo.
class EnemyBrain : public ICombatant {
public:
    // Estados de IA. Mismos valores que los de Enemy (kIdle/kPatrol/
    // kChase) para no cambiar el significado de m_aiState al mover la
    // logica: un enemigo quieto sigue siendo kIdle.
    static constexpr int kIdle = 0;
    static constexpr int kPatrol = 1;
    static constexpr int kChase = 2;

    // patrolMin == patrolMax (o rango invertido) = enemigo quieto, que
    // es exactamente lo que LevelLoader produce cuando el JSON no trae
    // patrulla (ver ObjectSpawn). No es un caso de error: es lo normal.
    EnemyBrain(GridCoord position, GridCoord patrolMin, GridCoord patrolMax, int maxHealth = 50,
               float stepInterval = 0.5f);

    // Avanza la patrulla. deltaTime en segundos; da un paso cada
    // stepInterval, acumulando el sobrante (a 20 fps sigue moviendose al
    // mismo ritmo que a 120, mismo criterio que Enemy::update original).
    //
    // canEnter: se consulta ANTES de pisar cada celda. Es lo que impide
    // que un enemigo patrulle a traves de un muro o del jugador sin que
    // EnemyBrain tenga que conocer el TileMap (seguiria siendo GL-free,
    // pero pasaria a depender del mapa; con el callback, quien sabe de
    // colisiones -- GameSession -- decide, y esta clase solo pregunta).
    // Un callback vacio deja pasar todo, util en tests.
    template <typename CanEnterFn>
    void update(float deltaTime, CanEnterFn canEnter) {
        if (m_patrolMax.x <= m_patrolMin.x) {
            m_aiState = kIdle;  // rango degenerado: nada que recorrer
            return;
        }
        m_aiState = kPatrol;
        m_stepElapsed += deltaTime;
        while (m_stepElapsed >= m_stepInterval) {
            m_stepElapsed -= m_stepInterval;
            stepOnce(canEnter);
        }
    }

    // --- ICombatant ---
    void takeDamage(int amount) override;
    void heal(int amount) override;
    int health() const override { return m_health; }
    int maxHealth() const override {
        if (m_sheet) {
            return m_sheet->healthCap();
        }
        return m_maxHealth;
    }
    bool isAlive() const override { return m_health > 0; }

    int stat(RPG::Stat s) const override {
        if (m_sheet)
            return m_sheet->stat(s);
        switch (s) {
            case RPG::Stat::CON:
                return m_stats[0];
            case RPG::Stat::DES:
                return m_stats[1];
            case RPG::Stat::INT:
                return m_stats[2];
            case RPG::Stat::CAR:
                return m_stats[3];
        }
        return 0;
    }
    RPG::DefenseBlock defenses() const override {
        if (m_sheet) {
            RPG::DefenseBlock b = m_sheet->defenseValues();
            return b;
        }
        RPG::DefenseBlock b;
        b.values[0] = 10 + stat(RPG::Stat::DES) + m_equip_armor;  // CA
        b.values[1] = 10 + stat(RPG::Stat::CON);                  // TS física
        b.values[2] = 10 + stat(RPG::Stat::INT);                  // TS voluntad
        b.values[3] = 10 + stat(RPG::Stat::CAR);                  // DC hechizo
        return b;
    }
    int defense_value(RPG::Defense d) const { return ICombatant::defense_value(d); }
    int initiative_bonus() const override { return stat(RPG::Stat::DES); }
    int tier() const override { return m_sheet ? m_sheet->tier : m_tier; }
    std::string combatant_id() const override { return m_sheet ? m_sheet->id : m_combatant_id; }

    // ====== Fractura #1 CharacterSheet ======
    void set_character_sheet(RPG::CharacterSheet* s) {
        m_sheet = s;
        if (s && m_health == m_maxHealth) {
            // Sync inicial: si estabamos a max HP, pasamos al sheet->healthCap
            m_maxHealth = s->healthCap() ? s->healthCap() : m_maxHealth;
            if (m_maxHealth <= 0)
                m_maxHealth = 1;
            m_health = m_maxHealth;
        }
    }
    RPG::CharacterSheet* character_sheet() { return m_sheet; }
    const RPG::CharacterSheet* character_sheet() const { return m_sheet; }

    // Rellena el brain desde MonsterDefinition. Si sheet no es nullptr,
    // también lo inicializa y llama a recompute_derived(). Después:
    //   - m_health = monster.maxHealth
    //   - m_stats = monster.stats
    //   - m_tier = monster.tier
    //   - m_combatant_id = monster.id
    //   - m_sheet si existe: id, displayName, classId = monster.role,
    //     raceId "", backgroundId "", tier = monster.tier,
    //     baseStats = monster.stats (CON DES INT CAR), displayName = monster.name.
    void populate_from_monster_catalog(const RPG::MonsterDefinition& mon,
                                       const RPG::TierRules* tierRules = nullptr,
                                       const RPG::ClassDefinition* defaultClass = nullptr) {
        m_health = mon.maxHealth > 0 ? mon.maxHealth : 1;
        m_maxHealth = m_health;
        m_stats = mon.stats;
        m_tier = mon.tier < 1 ? 1 : (mon.tier > 5 ? 5 : mon.tier);
        m_combatant_id = mon.id;
        if (m_sheet) {
            m_sheet->id = mon.id;
            m_sheet->displayName = mon.name;
            m_sheet->classId = mon.role;
            m_sheet->raceId.clear();
            m_sheet->backgroundId.clear();
            m_sheet->deityId.clear();
            m_sheet->tier = m_tier;
            m_sheet->baseStats = mon.stats;
            m_sheet->racialBonuses = {0, 0, 0, 0};
            m_sheet->classBonuses = {0, 0, 0, 0};
            m_sheet->equippedIds.fill({});
            m_sheet->equipBonuses = {0, 0, 0, 0};
            m_sheet->equipArmorBonus = 0;
            m_sheet->equipShieldBonus = 0;
            m_sheet->physicalSaveBonus = 0;
            m_sheet->mentalSaveBonus = 0;
            m_sheet->spellSaveExtra = 0;
            // Llenar knownSkillIds desde monster.skillIds si quieres (opcional P1):
            m_sheet->knownSkillIds = mon.skillIds;
            // Llenar passiveIds desde monster.passiveId si existe:
            if (!mon.passiveId.empty())
                m_sheet->passiveIds.push_back(mon.passiveId);
            if (tierRules) {
                m_sheet->recompute_derived(*tierRules, defaultClass);
                if (m_sheet->healthCap() > 0) {
                    m_maxHealth = m_sheet->healthCap();
                    // El health actual:
                    //   - si antes era el mismo que el maximo, sincronizar al sheet maximo
                    //   - si no (habia curacion/dano parcial) mantener el ratio
                    const int prevMax = static_cast<int>(mon.maxHealth);
                    if (prevMax > 0) {
                        int ratio_hp = (m_health * m_maxHealth) / prevMax;
                        if (ratio_hp < 1)
                            ratio_hp = 1;
                        if (ratio_hp > m_maxHealth)
                            ratio_hp = m_maxHealth;
                        m_health = ratio_hp;
                    } else {
                        m_health = m_maxHealth;
                    }
                }
            }
        }
    }

    // Setters para poblar desde ObjectCatalog / GameSession.
    void set_stat(RPG::Stat s, int v) {
        if (v < 0)
            v = 0;
        if (v > 8)
            v = 8;
        switch (s) {
            case RPG::Stat::CON:
                m_stats[0] = v;
                break;
            case RPG::Stat::DES:
                m_stats[1] = v;
                break;
            case RPG::Stat::INT:
                m_stats[2] = v;
                break;
            case RPG::Stat::CAR:
                m_stats[3] = v;
                break;
        }
    }
    void set_equip_armor(int a) { m_equip_armor = (a < 0 ? 0 : a); }
    void set_tier(int t) { m_tier = (t < 1 ? 1 : (t > 5 ? 5 : t)); }
    void set_combatant_id(const std::string& id) { m_combatant_id = id; }
    // set_max_health legacy:
    void set_max_health(int v) {
        m_maxHealth = (v < 1 ? 1 : v);
        if (m_health > m_maxHealth)
            m_health = m_maxHealth;
    }
    void set_health(int v) { m_health = (v < 0 ? 0 : (v > m_maxHealth ? m_maxHealth : v)); }

    const GridCoord& position() const { return m_position; }
    void setPosition(const GridCoord& position) { m_position = position; }
    int aiState() const { return m_aiState; }
    // +1 si camina hacia patrolMax, -1 hacia patrolMin. Lo lee el sprite
    // para elegir la animacion ("walk_right"/"walk_left") sin duplicar
    // aqui el concepto de animacion.
    int direction() const { return m_direction; }

private:
    // Un paso de patrulla.
    //
    // En los EXTREMOS del rango se invierte el sentido en el mismo tic en
    // que se llega (recortando el destino al extremo), no en el
    // siguiente. La diferencia parece menor y no lo es: rebotar "al
    // intentar salir" deja al enemigo parado un tic en cada punta, y su
    // ida y vuelta deja de ser regular. Este es el comportamiento
    // original de Enemy::update, y hay un test que lo fija
    // (demo_animated_entity::testEnemy) -- fue el que detecto la
    // diferencia al extraer esta clase.
    //
    // Ante un OBSTACULO si se gasta el tic dando media vuelta: ahi el
    // enemigo choca de verdad con algo, y quedarse empujando contra una
    // pared parece roto.
    template <typename CanEnterFn>
    void stepOnce(CanEnterFn canEnter) {
        GridCoord next = m_position;
        next.x += m_direction;

        if (next.x >= m_patrolMax.x) {
            next.x = m_patrolMax.x;
            m_direction = -1;
        } else if (next.x <= m_patrolMin.x) {
            next.x = m_patrolMin.x;
            m_direction = 1;
        }
        if (!canEnter(next)) {
            m_direction = -m_direction;
            return;
        }
        m_position = next;
    }

    GridCoord m_position;
    GridCoord m_patrolMin;
    GridCoord m_patrolMax;
    int m_health;
    int m_maxHealth;
    int m_aiState = kPatrol;
    int m_direction = 1;
    float m_stepInterval;
    float m_stepElapsed = 0.0f;

    // Stubs GDD por defecto; ObjectCatalog/GameSession los pisan.
    std::array<int, 4> m_stats = {0, 0, 0, 0};
    int m_equip_armor = 0;
    int m_tier = 1;
    std::string m_combatant_id = "enemy";

    // Fractura #1: fuente única de verdad
    RPG::CharacterSheet* m_sheet = nullptr;
};
