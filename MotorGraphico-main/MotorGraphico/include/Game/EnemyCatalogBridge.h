#pragma once

#include "Game/EnemyBrain.h"
#include "Game/Skill.h"
#include "World/World.h"
#include "Level/ObjectCatalog.h"
#include "RPG/Catalogs/RpgCatalogs.h"

// D-08 — Helpers per crear enemics des del catàleg MonsterDefinition.
// Això és la "plomeria" que connecta ObjectCatalog (gràfic) amb
// MonsterCatalog (RPG). GameSession.cii l'ha de cridar en lloc de
// fer `make_shared<EnemyBrain>(..., def->combat.maxHealth)`.
//
// GL-free a propòsit: cap include de Render/ ni Texture.

namespace EnemyCatalog {

// H2-07 — Resol la ficha del monstre a partir de l'id de l'objecte.
//
// Per què existeix: `assets/objects/` anomena els enemics `enemy_<id>`
// (enemy_dragon_negro_anciano) i `monsters.json` els crida `<id>`
// (dragon_negro_anciano). Són 460 ids d'enemig al ObjectCatalog i cap
// coincideix amb el nom del MonsterCatalog. Sense això el `find()` tornava
// nullptr i l'enemic queia al fallback de `ObjectCatalog.combat.maxHealth`
// (10 a la majoria): 430 de 460 enemigs jugaven amb la vida del placeholder,
// no amb la del catàleg (12-80 segons el tier).
//
// L'ordre importa i és a favor del canó:
//   1. exacte      — si el catàleg té la id tal qual, esa és la ficha
//   2. sense prefix — si no, es trenca el `enemy_` i es torna a mirar
//   3. nullptr     — de veritat no hi és (això sí és el fallback real)
//
// El pas 1 és el que fa que això no canviï res del que ja funciona: si un id
// ja era exacte, es queda amb el seu. L'únic que canvia són els ids que abans
// no resolien.
inline const RPG::MonsterDefinition* findMonster(
    const RPG::Catalogs::Catalog<RPG::MonsterDefinition>* monsterCatalog,
    const std::string& objectId) {
    if (monsterCatalog == nullptr || objectId.empty()) return nullptr;
    if (const RPG::MonsterDefinition* exacte = monsterCatalog->find(objectId)) return exacte;
    static constexpr const char* kPrefix = "enemy_";
    constexpr std::size_t kPrefixLen = 6;
    if (objectId.size() > kPrefixLen && objectId.compare(0, kPrefixLen, kPrefix) == 0) {
        return monsterCatalog->find(objectId.substr(kPrefixLen));
    }
    return nullptr;
}

// Crea un WorldEnemy a partir d'un ObjectDefinition (del nivell) i,
// si es troba, un MonsterDefinition (del catàleg RPG).
//
// Paràmetres:
//   objectId        - ID de l'ObjectDefinition (spawn.objectId)
//   def             - ObjectDefinition del ObjectCatalog (pot ser nullptr)
//   monsterCatalog  - MonsterCatalog carregat (pot ser nullptr; fallback a ObjectCatalog)
//   position        - Posició al mapa
//   patrolMin/Max   - Rang de patrulla (del spawn)
//   summoned        - Si és invocat (no patrulla)
//   identity        - Identity ordinal (spawnIndex o ordinal propi)
//   health          - Vida explícita (-1 = usar la del catàleg)
//   tierRules       - Per computing derived stats (pot ser nullptr)
inline Engine::WorldEnemy createEnemy(
    const std::string& objectId,
    const ObjectDefinition* def,
    const RPG::Catalogs::Catalog<RPG::MonsterDefinition>* monsterCatalog,
    GridCoord position,
    GridCoord patrolMin,
    GridCoord patrolMax,
    bool summoned = false,
    int identity = 0,
    int health = -1,
    const RPG::TierRules* tierRules = nullptr)
{
    Engine::WorldEnemy enemy;
    enemy.objectId = objectId;
    enemy.summoned = summoned;
    enemy.spawnIndex = identity;
    enemy.position = position;

    // Determinar maxHealth: ObjectCatalog → 10 per defecte
    int maxHealth = 10;
    if (def != nullptr) {
        maxHealth = def->combat.maxHealth;
    }

    // Buscar MonsterDefinition al catàleg RPG. H2-07: el `find` tolerant al
    // prefix `enemy_`, que és on estaban 430 de 460 enemigs.
    const RPG::MonsterDefinition* monDef = findMonster(monsterCatalog, objectId);

    // Crear EnemyBrain
    enemy.brain = std::make_shared<EnemyBrain>(position, patrolMin, patrolMax, maxHealth);

    // Poblar des del catàleg RPG si existeix
    if (monDef != nullptr) {
        enemy.brain->populate_from_monster_catalog(*monDef, tierRules, nullptr);
    } else if (def != nullptr) {
        // ACOMPANANTES-01 — Sin ficha de monstruo, los dados del objeto
        // (combat.stats): asi el contenido decide como de duro es.
        const RPG::Stat orden[4] = {RPG::Stat::CON, RPG::Stat::DES, RPG::Stat::INT, RPG::Stat::CAR};
        for (std::size_t s = 0; s < 4; ++s) enemy.brain->set_stat(orden[s], def->combat.stats[s]);
    }

    // Vida explícita (per save/load o invocacions)
    if (health >= 0) {
        enemy.brain->set_health(health);
    }

    // SkillSet: preferir MonsterDefinition.skillIds, fallback a ObjectCatalog
    int maxMana = 0;
    std::vector<std::string> skillIds;

    if (monDef != nullptr) {
        maxMana = 0;  // MonsterDefinition no té maxMana; el player mana ve del CharacterSheet
        skillIds = monDef->skillIds;
    } else if (def != nullptr) {
        maxMana = def->combat.maxMana;
        skillIds = def->combat.skillIds;
    }

    enemy.skills = std::make_shared<SkillSet>(maxMana);
    for (const std::string& skillId : skillIds) {
        enemy.skills->learn(skillId);
    }

    return enemy;
}

// Versió simplificada per a GameSession constructor (sense summoned/identity/health)
inline Engine::WorldEnemy createFromSpawn(
    const ObjectSpawn& spawn,
    const ObjectDefinition* def,
    const RPG::Catalogs::Catalog<RPG::MonsterDefinition>* monsterCatalog,
    const RPG::TierRules* tierRules = nullptr)
{
    return createEnemy(spawn.objectId, def, monsterCatalog,
                       spawn.position, spawn.patrolMin, spawn.patrolMax,
                       false, static_cast<int>(0), -1, tierRules);
}

}  // namespace EnemyCatalog
