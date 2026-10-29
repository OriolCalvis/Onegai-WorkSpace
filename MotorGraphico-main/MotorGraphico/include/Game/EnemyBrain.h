// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   D-08  (fase D, 3 j) - Els enemics del joc són els del catàleg
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include "Game/EnemyBrainData.h"
#include "Render/ICombatant.h"
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
// EnemyBrainData conserva un RPG::CharacterSheet* externo no propietario.
// Si != nullptr: stat(), maxHealth(), tier(), combatant_id(), defenses()
// leen DESDE el sheet usando sus cached values (recompute_derived()).
// Si nullptr: los valores legacy de EnemyBrainData igual que antes (backward
// compatible con todos los demos existentes).
//
// Además, populate_from_monster_catalog() rellena TANTO el sheet como
// los legacy defaults, partiendo de una MonsterDefinition del catalogo.
class EnemyBrain : public ICombatant {
public:
    // Estados de IA. Mismos valores que los de Enemy (kIdle/kPatrol/
    // kChase) para no cambiar el significado del estado al mover la
    // logica: un enemigo quieto sigue siendo kIdle.
    static constexpr int kIdle = EnemyBrainData::kIdle;
    static constexpr int kPatrol = EnemyBrainData::kPatrol;
    static constexpr int kChase = EnemyBrainData::kChase;

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
        if (m_data.patrolMax.x <= m_data.patrolMin.x) {
            m_data.aiState = kIdle;  // rango degenerado: nada que recorrer
            return;
        }
        m_data.aiState = kPatrol;
        m_data.stepElapsed += deltaTime;
        while (m_data.stepElapsed >= m_data.stepInterval) {
            m_data.stepElapsed -= m_data.stepInterval;
            stepOnce(canEnter);
        }
    }

    // --- ICombatant ---
    void takeDamage(int amount) override;
    void heal(int amount) override;
    int health() const override { return m_data.health; }
    int maxHealth() const override {
        if (m_data.sheet) {
            return m_data.sheet->healthCap();
        }
        return m_data.maxHealth;
    }
    bool isAlive() const override { return m_data.health > 0; }

    int stat(RPG::Stat s) const override {
        if (m_data.sheet)
            return m_data.sheet->stat(s);
        switch (s) {
            case RPG::Stat::CON:
                return m_data.stats[0];
            case RPG::Stat::DES:
                return m_data.stats[1];
            case RPG::Stat::INT:
                return m_data.stats[2];
            case RPG::Stat::CAR:
                return m_data.stats[3];
        }
        return 0;
    }
    RPG::DefenseBlock defenses() const override {
        if (m_data.sheet) {
            RPG::DefenseBlock b = m_data.sheet->defenseValues();
            return b;
        }
        RPG::DefenseBlock b;
        b.values[0] = 10 + stat(RPG::Stat::DES) + m_data.equipArmor;  // CA
        b.values[1] = 10 + stat(RPG::Stat::CON);                  // TS física
        b.values[2] = 10 + stat(RPG::Stat::INT);                  // TS voluntad
        b.values[3] = 10 + stat(RPG::Stat::CAR);                  // DC hechizo
        return b;
    }
    int defense_value(RPG::Defense d) const { return ICombatant::defense_value(d); }
    int initiative_bonus() const override { return stat(RPG::Stat::DES); }
    int tier() const override { return m_data.sheet ? m_data.sheet->tier : m_data.tier; }
    std::string combatant_id() const override {
        return m_data.sheet ? m_data.sheet->id : m_data.combatantId;
    }

    // ====== Fractura #1 CharacterSheet ======
    void set_character_sheet(RPG::CharacterSheet* s) {
        m_data.sheet = s;
        if (s && m_data.health == m_data.maxHealth) {
            // Sync inicial: si estabamos a max HP, pasamos al sheet->healthCap
            m_data.maxHealth = s->healthCap() ? s->healthCap() : m_data.maxHealth;
            if (m_data.maxHealth <= 0)
                m_data.maxHealth = 1;
            m_data.health = m_data.maxHealth;
        }
    }
    RPG::CharacterSheet* character_sheet() override { return m_data.sheet; }
    const RPG::CharacterSheet* character_sheet() const override { return m_data.sheet; }

    // Rellena el brain desde MonsterDefinition. Si sheet no es nullptr,
    // también lo inicializa y llama a recompute_derived(). Después:
    //   - EnemyBrainData::health = monster.maxHealth
    //   - EnemyBrainData::stats = monster.stats
    //   - EnemyBrainData::tier = monster.tier
    //   - EnemyBrainData::combatantId = monster.id
    //   - EnemyBrainData::sheet si existe: id, displayName, classId = monster.role,
    //     raceId "", backgroundId "", tier = monster.tier,
    //     baseStats = monster.stats (CON DES INT CAR), displayName = monster.name.
    void populate_from_monster_catalog(const RPG::MonsterDefinition& mon,
                                       const RPG::TierRules* tierRules = nullptr,
                                       const RPG::ClassDefinition* defaultClass = nullptr) {
        m_data.health = mon.maxHealth > 0 ? mon.maxHealth : 1;
        m_data.maxHealth = m_data.health;
        m_data.stats = mon.stats;
        m_data.tier = mon.tier < 1 ? 1 : (mon.tier > 5 ? 5 : mon.tier);
        m_data.combatantId = mon.id;
        if (m_data.sheet) {
            m_data.sheet->id = mon.id;
            m_data.sheet->displayName = mon.name;
            m_data.sheet->classId = mon.role;
            m_data.sheet->raceId.clear();
            m_data.sheet->backgroundId.clear();
            m_data.sheet->deityId.clear();
            m_data.sheet->tier = m_data.tier;
            m_data.sheet->baseStats = mon.stats;
            m_data.sheet->racialBonuses = {0, 0, 0, 0};
            m_data.sheet->classBonuses = {0, 0, 0, 0};
            m_data.sheet->equippedIds.fill({});
            m_data.sheet->equipBonuses = {0, 0, 0, 0};
            m_data.sheet->equipArmorBonus = 0;
            m_data.sheet->equipShieldBonus = 0;
            m_data.sheet->physicalSaveBonus = 0;
            m_data.sheet->mentalSaveBonus = 0;
            m_data.sheet->spellSaveExtra = 0;
            // Llenar knownSkillIds desde monster.skillIds si quieres (opcional P1):
            m_data.sheet->knownSkillIds = mon.skillIds;
            // Llenar passiveIds desde monster.passiveId si existe:
            if (!mon.passiveId.empty())
                m_data.sheet->passiveIds.push_back(mon.passiveId);
            if (tierRules) {
                m_data.sheet->recompute_derived(*tierRules, defaultClass);
                if (m_data.sheet->healthCap() > 0) {
                    m_data.maxHealth = m_data.sheet->healthCap();
                    // El health actual:
                    //   - si antes era el mismo que el maximo, sincronizar al sheet maximo
                    //   - si no (habia curacion/dano parcial) mantener el ratio
                    const int prevMax = static_cast<int>(mon.maxHealth);
                    if (prevMax > 0) {
                        int ratio_hp = (m_data.health * m_data.maxHealth) / prevMax;
                        if (ratio_hp < 1)
                            ratio_hp = 1;
                        if (ratio_hp > m_data.maxHealth)
                            ratio_hp = m_data.maxHealth;
                        m_data.health = ratio_hp;
                    } else {
                        m_data.health = m_data.maxHealth;
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
                m_data.stats[0] = v;
                break;
            case RPG::Stat::DES:
                m_data.stats[1] = v;
                break;
            case RPG::Stat::INT:
                m_data.stats[2] = v;
                break;
            case RPG::Stat::CAR:
                m_data.stats[3] = v;
                break;
        }
    }
    void set_equip_armor(int a) { m_data.equipArmor = (a < 0 ? 0 : a); }
    void set_tier(int t) { m_data.tier = (t < 1 ? 1 : (t > 5 ? 5 : t)); }
    void set_combatant_id(const std::string& id) { m_data.combatantId = id; }
    // set_max_health legacy:
    void set_max_health(int v) {
        m_data.maxHealth = (v < 1 ? 1 : v);
        if (m_data.health > m_data.maxHealth)
            m_data.health = m_data.maxHealth;
    }
    void set_health(int v) {
        m_data.health = (v < 0 ? 0 : (v > m_data.maxHealth ? m_data.maxHealth : v));
    }

    const GridCoord& position() const { return m_data.position; }
    void setPosition(const GridCoord& position) { m_data.position = position; }
    int aiState() const { return m_data.aiState; }
    // +1 si camina hacia patrolMax, -1 hacia patrolMin. Lo lee el sprite
    // para elegir la animacion ("walk_right"/"walk_left") sin duplicar
    // aqui el concepto de animacion.
    int direction() const { return m_data.direction; }

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
        GridCoord next = m_data.position;
        next.x += m_data.direction;

        if (next.x >= m_data.patrolMax.x) {
            next.x = m_data.patrolMax.x;
            m_data.direction = -1;
        } else if (next.x <= m_data.patrolMin.x) {
            next.x = m_data.patrolMin.x;
            m_data.direction = 1;
        }
        if (!canEnter(next)) {
            m_data.direction = -m_data.direction;
            return;
        }
        m_data.position = next;
    }

    // Colaborador de datos no GL. EnemyBrain dirige la patrulla y el contrato
    // ICombatant; no conserva una segunda copia de la ficha o de su estado.
    EnemyBrainData m_data;
};
