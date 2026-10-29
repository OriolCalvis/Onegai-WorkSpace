#pragma once

// DATOS: Lo que respira enfrente: monstruos, invocaciones y monturas.
//
// Comparten la forma de una ficha de combate. Monsters es la mas grande
// (432) y la que mas revisiones de datos recibe.
//
// Salio de include/RPG/Definitions/RpgCoreDefinitions.h el 23/09/2026
// (EST-30): aquella cabecera tenia 687 lineas con veintiuna fichas dentro.
// Son structs puros —campos y nada mas— y por eso no llevan controlador:
// la norma N2 pide clase de datos, y esto ya lo es.
//
// Se corta por las mismas familias que include/RPG/Catalogs/
// CatalogLoaders<Familia>.h, para que la ficha y su cargador se lean uno
// al lado del otro. No se incluye directamente: entra por
// RpgCoreDefinitions.h, que sigue siendo el paraguas.


#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "RPG/Stat.h"
#include "RPG/TierRules.h"


namespace RPG {

// ================== MonsterDefinition (432 enemigos/PNJ hostiles) ==================
struct MonsterDefinition {
    std::string id;
    std::string name;
    std::string flavorText;
    std::string description;

    // "criatura", "elite", "jefe", "inocente"
    std::string rank;
    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;

    // Role táctico (AI enemigo P1)
    std::string role; // "conjurador", "tanque", "striker", "soporte", "asesino"...
    std::string faction;
    std::string factionName;

    // Stats y defensas
    std::array<int, 4> stats = {0,0,0,0};
    int maxHealth = 0;
    int ca = 10;
    int defMental = 10;
    int resFis = 10;
    int precMag = 10;
    int movement = 5;

    // Ataques (se transforman en SkillDefinition al vuelo en BattleStateOnegai,
    // o bien guardamos ids aquí si el GameMaster lo parametriza)
    struct MonsterAttack {
        std::string name;
        std::string defense;   // "CA", "defensa_mental", ...
        std::string effect;
        int magnitudeHint = 0; // para magnitude_by_degree heuristico
    };
    std::vector<MonsterAttack> attacks;

    std::string passiveId;         // Passive otorgada (ej "instinto de faccion")
    std::vector<std::string> conditionsInflictedIds;
    std::vector<std::string> skillIds;            // skill_* o spell_* que conoce
    std::vector<std::string> lootIds;             // equipment_* / cons_* que dropea

    // Jefe con multiples fases (P1: la MVP las declara, no las usa)
    struct MonsterPhase {
        std::string name;
        int hpThresholdPercent = 0;
        std::vector<std::string> gainedSkillIds;
    };
    std::vector<MonsterPhase> phases;
};

// ================== SummonDefinition (201) ==================
// "Ally efímero" invocado por hechizo/habilidad. Se trata como MonsterDefinition
// pero pertenece al caster.
struct SummonDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;

    int durationRounds = 3;
    // D-07 — Valor crudo de la carta (`duration`: "concentration"/"1_min").
    // "1_min" a rondas lo decide el ejecutor (D-11 O2), no el cargador.
    std::string durationRaw;
    int maxHealth = 0;
    int ca = 10;
    int movement = 5;
    std::array<int, 4> stats = {0,0,0,0};

    std::vector<std::string> attackSkillIds;
    std::vector<std::string> passiveIds;
    std::string summonRole; // "dps"/"tank"/"healer"/"utility"

    // Desaparición al daño? (no) — los summons clásicos mueren a 0 HP normalmente
    bool disappearAtZeroHp = true;
    bool concentrationRequired = false;   // pila de canalización
    // D-07 — Quién lo manda (`control`: jugador/car_invocador/automatica, en
    // crudo: `car_invocador` es opaco) y qué carta lo invoca (`summonedBy`:
    // el reverso de `granted_summon_id`, lo necesitará D-11 O2).
    std::string control;
    std::string summonedBySkillId;
};

// ================== MountDefinition (50) ==================
struct MountDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;
    int price = 0;

    int movement = 8;         // casillas/turno
    int maxHealth = 20;
    int ca = 12;
    // D-07 — Stats de la montura (la carta trae CON/DES; CAR/INT quedan a 0).
    std::array<int, 4> stats = {0, 0, 0, 0};
    int carryWeightKg = 100;  // capacidad de carga (faccional/transporte)
    // D-07 — Plazas (`capacity` de la carta: 1 o 2 jinetes; NO son kg, no
    // va a carryWeightKg).
    int capacity = 1;

    // Bonuses mientras se monta (sustituyen a los del PJ si son mayores)
    std::array<int, 4> mountedStatBonus = {0,0,0,0};
    int mountedCaBonus = 0;
    int disengageBonus = 0;   // retiros sin oportunidad

    bool canFly = false;
    bool canSwim = false;
    std::vector<std::string> grantedPassiveIdsWhileMounted;
    // D-07 — Terrenos (`terrain`, en crudo: inferir canFly de "aire" sería
    // inventar), etiquetas de jinete (`mountedTags`), regla de desmonte
    // (`dismountTrigger`, prosa para el futuro ejecutor) y requisito de
    // monta (`ridingRequirements{minimo, stat}`). `mountedSkill` (objeto
    // sin id) queda para D-11.
    std::vector<std::string> terrain;
    std::vector<std::string> mountedTags;
    std::string dismountTrigger;
    std::string requiredRideStat;
    int requiredRideMin = 0;
    // D-07 — Cuidados (`upkeep`, 50/50): prosa en crudo para el futuro ejecutor.
    std::string upkeep;
};

}  // namespace RPG
