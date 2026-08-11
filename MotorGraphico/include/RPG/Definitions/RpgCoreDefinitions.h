#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "RPG/Stat.h"
#include "RPG/TierRules.h"

namespace RPG {

// ================== ClassDefinition ==================
// 1:1 con classes.json (tools/convertir_definiciones_rpg.py). Corresponde a
// la entidad "Class" del GDD Onegai Ed. 2 (clase de personaje). Hay 61.
struct ClassDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string lore;

    int tier = 1;                         // tier mínimo de la clase (normalmente 1)
    ItemRarity rarity = ItemRarity::Common;

    std::string role;                     // "caster", "martial", "healer", "striker", "tank"...
    int baseHealth = 8;                   // Vida base (antes del cálculo por stats+tier)
    std::string primaryStat = "INT";      // "CON", "DES", "INT", "CAR"
    std::string secondaryStat = "DES";

    // maxSkillCards (límite de cartas de habilidad conocidas, distinto de handLimit)
    int maxSkillCards = 10;

    // Compatibilidad de equipamiento (símbolos △ ○ □ ✦ ☠ ✝ ♞ ⚙)
    std::vector<std::string> compatibilityWeapons;
    std::vector<std::string> compatibilityArmor;
    std::vector<std::string> allowedEquipmentTags;
    std::string maxArmorWeight;  // "ligera"/"media"/"pesada"
    std::string maxWeaponWeight;

    // Cartas iniciales (ids que resuelve SkillCatalog + PassiveCatalog)
    std::string passiveId;                        // Passive de clase identitaria
    std::vector<std::string> startingSkillIds;    // skill_* y spell_*
    std::vector<std::string> startingEquipmentIds;// equip_*
    std::vector<std::string> specializationIds;   // Especializaciones Tier2+

    bool canMulticlassInto = false;
    std::vector<std::string> restrictedTags;

    // Helper de casting stat
    Stat primary_stat_enum() const;
    Stat secondary_stat_enum() const;
};

// ================== RaceDefinition ==================
// 1:1 con races.json (43 razas/subrazas). Corresponde a "Race" del GDD.
struct RaceDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string flavorText;

    int tier = 1;
    ItemRarity rarity = ItemRarity::Common;

    int baseSpeed = 30; // ft. (típico 30)
    std::vector<std::string> languages;
    std::vector<std::string> compatibleDeityIds;
    std::vector<std::string> narrativeTags;

    // Stat bonuses raciales
    std::array<int, 4> statBonuses = {0, 0, 0, 0};

    // Rasgos raciales (todos opcionales; las entradas son ids que resuelve
    // TraitCatalog o bien la descripcion embebida)
    std::string passiveTraitId;
    std::string activeTraitId;
    std::string racialTraitId;

    std::vector<std::string> affinities;
    std::vector<std::string> limitations;
    std::vector<std::string> unlocks;
};

// ================== BackgroundDefinition ==================
// "Trasfondo" del personaje. 12 en GDD; nosotros tenemos 50 de kits_iniciales.
struct BackgroundDefinition {
    std::string id;
    std::string name;
    std::string description;

    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;

    int startingWealthGold = 0;

    // Virtud / Defecto / Objetivo narrativos
    std::string virtue;
    std::string defect;
    std::string goal;

    // Equipo / skills de arranque (opcional)
    std::vector<std::string> startingEquipmentIds;
    std::vector<std::string> skillProficiencyIds;
    std::vector<std::string> featureIds;    // dote / trait otorgado

    std::string factionId;                  // facción por defecto del bg (opcional)
    int startingFactionRep = 0;             // reputación inicial con ella
};

// ================== PassiveDefinition ==================
// "Pasiva" (66 en Onegai). Cada clase/raza otorga 1 o varias; también se pueden
// comprar como cartas de pila pasiva.
struct PassiveDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string flavorText;

    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;

    // Triggers data-driven. El SkillExecutor lee los triggers simples; los
    // triggers complejos (P2) necesitarán scripting o handlers.
    enum class Trigger : uint8_t {
        NONE = 0,
        ON_PLAY_LONGREST,
        ON_NOT_MOVED,
        ON_ALLY_HEALED,
        ON_TURN_START,
        ON_CRIT_TAKEN,
        ON_ATTACK_ROLLED,
        ON_DAMAGE_TAKEN,
        ON_CRIT_DEALT,
        ALWAYS_ACTIVE
    };
    Trigger trigger = Trigger::ALWAYS_ACTIVE;

    // Modificadores de contexto (ver EnhanceHooks).
    int extra_dice_on_trigger = 0;
    float success_bonus_on_trigger = 0.0f;
    std::string required_tag;   // si no es vacio, solo afecta a skills con este tag

    // Stat bonuses fijos que suma CharacterSheet (pasivos permanentes)
    std::array<int, 4> statBonuses = {0, 0, 0, 0};
    int armorBonus = 0;
    int physicalSaveBonus = 0;
    int mentalSaveBonus = 0;
    int spellSaveBonus = 0;
    int maxHealthBonus = 0;
    int handSizeBonus = 0;
};

// ================== FeatDefinition (Dotes, 6 en Onegai) ==================
struct FeatDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier_min = 1;
    ItemRarity rarity = ItemRarity::Rare;

    std::string requiredClassId;   // "" = ninguna
    std::string requiredBackgroundId;
    int requiredTier = 1;

    std::array<int, 4> statBonuses = {0,0,0,0};
    std::vector<std::string> grantedPassiveIds;
    std::vector<std::string> grantedSkillIds;
};

// ================== TraitDefinition (Rasgos raciales o de deidad; 2 en Onegai) ==================
struct TraitDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier_min = 0;
    ItemRarity rarity = ItemRarity::Common;

    bool isActive = false;  // false=pasiva, true=habilidad activa con coste
    int timesPerRest = 1;
    std::string recovery = "descanso_largo";
    std::string linkedSkillId; // si es activa, la skill que usa
};

// ================== DeityDefinition (10 en Onegai) ==================
struct DeityDefinition {
    std::string id;
    std::string name;
    std::string title;     // "Diosa de la Vida"...
    std::string alignment; // Héroe, Neutral, Villano, etc.
    std::string description;
    std::vector<std::string> domains;      // "Guerra", "Magia", "Naturaleza"...
    std::vector<std::string> symbolTags;   // tags de compatibilidad ✝ ☠ etc.

    int tier_min = 0;

    // Plegarias (ids de spell que puede lanzar el devoto)
    std::vector<std::string> prayerSkillIds;
    // Condición de roto (abandono): si el PJ rompe dogma, pierde estas passives
    std::vector<std::string> grantedPassiveIds;
    std::array<int, 4> dogmaStatBonus = {0,0,0,0};
};

// ================== ConditionDefinition (15 condiciones, P0-8) ==================
struct ConditionDefinition {
    std::string id;
    std::string name;
    std::string description;

    // Stacking: "none" (no stackea, reemplaza), "duration" (refresca duración),
    // "stacks" (suma intensidad, ej sagnat 1..5)
    enum class Stacking { NONE, DURATION, STACK_INTENSITY };
    Stacking stacking = Stacking::DURATION;
    int defaultRounds = 3;
    int maxStacks = 1;

    // Modificadores al final del tick
    int damagePerRound = 0;   // daño directo (sangrado, veneno)
    bool isNegative = true;   // la mayoría son negativos; curado/Inspirado se marcan false

    // Modificadores a stats/defensas (los que sumar al PJ que lo lleve)
    std::array<int, 4> statMod = {0,0,0,0};
    int caMod = 0;
    int physicalSaveMod = 0;
    int willSaveMod = 0;

    // Flags de comportamiento
    bool preventAction = false;       // paralizado/inconsciente
    bool halfSpeed = false;           // agarrado/anclado
    bool grantAdvantageToEnemies = false;   // cegado/caido
    bool grantDisadvantageToCaster = false; // cegado/enfurecido en algunos casos
    bool breaksOnDamage = false;      // encantado
};

// ================== EquipmentDefinition (413) ==================
// "Armas/armaduras/accesorios" — lo que entra en 6 slots equipables.
struct EquipmentDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string flavorText;

    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;
    int price = 0;

    // "Head", "Torso", "Legs", "Feet", "MainHand", "OffHand", "Accessory", "TwoHanded", "Consumable"
    std::string slot;
    // "light" / "medium" / "heavy"
    std::string weightCategory;
    // Símbolos de compatibilidad: △ ○ □ ✦ ☠ ✝ ♞ ⚙
    std::vector<std::string> compatibleSymbols;
    // Tags de equipo (Arcano, Ligera...)
    std::vector<std::string> grantedTags;

    std::array<int, 4> statBonuses = {0, 0, 0, 0};
    int caBonus = 0;            // CA física extra
    int maxHealthBonus = 0;     // vida extra (p. ej "anillo de vitalidad")
    int physicalSaveBonus = 0;
    int willSaveBonus = 0;
    int spellSaveBonus = 0;
    int damageBonus = 0;        // daño extra pasivo al atacar (ej espada +1)

    // Si es arma ofensiva: la carta de habilidad "ataque con esta arma"
    std::string linkedSkillId;

    // Es two-handed? (ocupa MainHand + OffHand)
    bool twoHanded = false;
};

// ================== ConsumableDefinition (31) ==================
struct ConsumableDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;
    int price = 0;

    // Efectos inmediatos (P0 simple; P2 expandirá)
    int healHp = 0;
    int restorePileCards = 0;   // recarga X cartas de la pila "activa"
    bool clearOneCondition = false;
    std::string grantConditionId;  // se autoaplica esta condicion
    int grantConditionRounds = 3;
    int bonusGold = 0;
    std::string grantPassiveId;
    int grantPassiveRounds = 0; // si >0: temporal
};

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
    int carryWeightKg = 100;  // capacidad de carga (faccional/transporte)

    // Bonuses mientras se monta (sustituyen a los del PJ si son mayores)
    std::array<int, 4> mountedStatBonus = {0,0,0,0};
    int mountedCaBonus = 0;
    int disengageBonus = 0;   // retiros sin oportunidad

    bool canFly = false;
    bool canSwim = false;
    std::vector<std::string> grantedPassiveIdsWhileMounted;
};

// ================== TrapDefinition (51 trampas) ==================
struct TrapDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;

    int damage = 6;            // danyo base (pisa trampa)
    std::string saveAttribute; // "DES", "CON"...
    float cd = 1.0f;           // CD Nd6 si procede
    bool detectBySearch = true;
    int detectDifficulty = 0;  // dificultad percepción pasiva

    std::string applyConditionId;
    int applyConditionRounds = 2;
    bool oneShot = true;       // se gasta tras saltar
    int cooldownRounds = 0;    // si no oneShot
};

// ================== LootTableDefinition (201) ==================
struct LootTableDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;

    struct Entry {
        std::string refId;       // "equip_XXX" / "cons_XXX" / "__gold__" / "__loot_table_YYY__"
        int weight = 1;
        int min = 1;
        int max = 1;
    };
    std::vector<Entry> entries;

    // Oro garantizado mínimo/máximo por la tabla (además de entries si tienen __gold__)
    int minGold = 0;
    int maxGold = 0;
};

// ================== NpcDefinition (~12) ==================
struct NpcDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier = 1;

    std::string raceId;
    std::string classId;
    std::string factionId;
    int factionRepDefault = 0;
    std::string occupation;

    // Capacidades para el engine: tienda / negocio / dialogo / combate
    bool isMerchant = false;
    std::string shopLootId;      // LootTable que ofrece
    bool hasDialogue = false;
    std::string dialogueId;
    bool isHostile = false;
    std::string monsterId;        // si es combatiente

    // Oro del PNJ (para robo / misiones)
    int gold = 0;
};

// ================== AdventureDefinition (78) ==================
struct AdventureDefinition {
    std::string id;
    std::string name;
    std::string summary;
    int tier = 1;
    int recommendedTierMin = 1;
    int recommendedTierMax = 5;

    // Misiones asociadas
    std::vector<std::string> questIds;
    // NPCs que aparecen
    std::vector<std::string> requiredNpcIds;
    // Facción implicada principal
    std::string mainFactionId;

    // Story cards entregadas al empezar / al terminar
    std::vector<std::string> grantOnStartStoryCardIds;
    std::vector<std::string> grantOnEndStoryCardIds;
    std::string grantMilestoneOnEnd; // si no vacío: milestone para tier up
};

// ================== EventDefinition (1) ==================
struct EventDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier = 0;

    // Trigger world: "on_long_rest", "on_travel", "on_enter_zone" ...
    std::string triggerKind;
    std::vector<std::string> validZoneIds;
    float weight = 1.0f;  // probabilidad relativa

    struct Option {
        std::string label;
        // Consecuencias
        int gold = 0;
        std::string applyConditionId;
        int applyConditionRounds = 0;
        std::string grantQuestId;
        std::string startCombatWithMonsterId;
        std::string grantStoryCardId;
        std::string levelTransitionId;
    };
    std::vector<Option> options;
};

// ---------------------------------------------------------------------------
// LocationDefinition — el mundo. Cierra la brecha B24 de
// GAMEMACHINE_NECESIDADES ("no hay 'localizacion' con nombre narrativo,
// faccion controladora, tier recomendado, quests de zona, events").
//
// Fuente: dndWeebCC/data/mapa/geografia.json, el unico sitio del proyecto
// donde el mundo de Egaroth tiene geometria: 19 poligonos de nacion, 95
// ciudades con coordenadas y 26 zonas de terreno. Hasta ahora ese dato no
// salia del motor de cartas y el motor grafico no tenia mundo, solo tres
// niveles de ciudad sueltos.
//
// Las tres clases van en UN solo tipo con discriminador `kind` porque quien
// consulta ("que hay en Udrax", "dame lo de tier <= 2") no quiere saber si
// es nacion, ciudad o zona. Sigue el mismo criterio que ObjectCategory.
//
// OJO: el mapa esta FECHADO. `epoch` lleva el ano del que es instantanea
// (2000 b.f., el primer ano de las Grandes Cruzadas). No vale para eras
// anteriores ni posteriores, y por eso el dato viaja con el catalogo en vez
// de darse por supuesto.
// ---------------------------------------------------------------------------
struct LocationDefinition {
    enum class Kind { Nation, City, Zone };

    std::string id;
    std::string name;
    Kind kind = Kind::City;
    int tier = 0;                       // dificultad/importancia recomendada

    std::string controlledBy;           // nacion que la controla (ciudad/zona)
    std::string description;

    // Posicion en el mapa mundi. Las naciones no la tienen: tienen poligono.
    bool hasPosition = false;
    int x = 0;
    int y = 0;
    std::string polygon;                // "x,y x,y ..." solo para Nation

    // Ciudad
    std::string settlementSize;         // capital | ciudad | pueblo | aldea
    std::string population;
    std::string ruler;
    std::string trait;
    bool pendingCanonName = false;      // marcador aun sin nombre canonico

    // Zona
    std::string terrain;                // montana | bosque | desierto | santuario...
    std::string danger;

    std::vector<std::string> storyIds;
    std::vector<std::string> npcIds;
    std::vector<std::string> deityIds;
    std::vector<std::string> raceIds;
};

}  // namespace RPG
