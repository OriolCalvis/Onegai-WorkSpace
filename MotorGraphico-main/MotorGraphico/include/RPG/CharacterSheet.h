#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "RPG/Definitions/RpgCoreDefinitions.h" // Class/Race/Background/Passive/...
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/Defense.h"
#include "RPG/Stat.h"
#include "RPG/TierRules.h"

namespace RPG {

// ============================================================
// CharacterSheet — Fuente ÚNICA de verdad (GAMEMACHINE_NECESIDADES §2 P0-15.
//
// Invariante de Fractura #1: "1 concepto = 1 representación":
//
//   Player.h / Enemy.h NO almacenan stats/HP/tier/ids de clase/raza/equipamiento:
// TODO LO saben leer y escribir ES AQUÍ. Las entidades Render delegan en un
// puntero no propietario a CharacterSheet.
//
// GL-free 100%: no toca NADA de OpenGL ni Player/Enemy.
//
// BattleState / SkillExecutor / InventoryEngine / DialogueTreeEngine toman por
// referencia (CharacterSheet&). Es el eje central del GameMachine.
// ============================================================

// Slots de equipamiento (P0-10 InventoryEngine).
enum class EquipSlot : uint8_t {
    Head = 0, Torso, Legs, Feet, MainHand, OffHand, Accessory, SLOT_COUNT
};

// 6 slots canónicos Onegai (la mayoria) + Accessory opcional:
constexpr size_t EQUIP_SLOT_COUNT = 6;

// Estado de condición activa (tick)
struct ActiveCondition {
    std::string conditionId;
    std::string sourceId;     // quien lo aplicó (para logs)
    int roundsRemaining = 0;
    int stacks = 1;
};

class CharacterSheet {
public:
    // Identidad
    std::string id;             // "pj_guerrera_maria" o "monster_aprendiz_hueco_03"
    std::string displayName;  // nombre visible para HUD
    std::string classId;
    std::string secondClassId;  // multiclass si allowsMulticlass(tier)=true
    std::string raceId;
    std::string backgroundId;
    std::string deityId;        // deidad adorada (si es el PJ)
    int tier = TIER_MIN;

    // Gold y facciones
    int gold = 0;
    std::string primaryFactionId;
    std::map<std::string, int> factionRep; // id: -100..+100

    // Moralidad 1D: héroe↔villano (heredado de GameSession)
    int morality = 0;

    // Stats (CON/DES/INT/CAR). El PJ inicializa desde catálogos)
    std::array<int, 4> baseStats = {0, 0, 0, 0};  // puro Point-Buy/GDD punto 0
    std::array<int, 4> racialBonuses  = {0,0,0,0}; // desde RaceDefinition
    std::array<int, 4> classBonuses   = {0,0,0,0}; // futuras bonos de clase inicial
    std::array<int, 4> equipBonuses   = {0,0,0,0};
    std::array<int, 4> featBonuses    = {0,0,0,0};
    std::array<int, 4> conditionModifiers = {0,0,0,0}; // condiciones activas

    // Defensas extra
    int equipArmorBonus = 0;
    int equipShieldBonus = 0;
    int physicalSaveBonus = 0;
    int mentalSaveBonus = 0;
    int spellSaveExtra = 0;

    // Inspiración (puntos de Heroe, GDD)
    int inspirationPoints = 0;

    // Equipo (ref ids -> EquipmentDefinition. idx = EquipSlot. "" si vacio
    std::array<std::string, 7> equippedIds;   // 6 canonico + accessory (7 por extension
    // Inventario de equipo (mochila): equipmentId + stack count
    std::vector<std::pair<std::string, int>> inventoryEquipment;
    // Consumibles (id + stacks)
    std::vector<std::pair<std::string, int>> inventoryConsumables;
    // Materiales (libres)
    std::vector<std::pair<std::string, int>> inventoryMisc;

    // Habilidades y conjuros conocidas (SkillCatalog)
    std::vector<std::string> knownSkillIds;
    std::vector<std::string> knownSpellIds;

    // Pasivas conocidas (PassiveCatalog + deities)
    std::vector<std::string> passiveIds;
    // Rasgos raciales, deidades
    std::vector<std::string> traitIds;
    std::vector<std::string> featIds;

    // Condiciones activas (tick por combate)
    std::vector<ActiveCondition> conditions;

    // ===== Stat totals (CON/DES/INT/CAR) = STAT_RANK_MIN..STAT_RANK_MAX
    int stat(Stat s) const {
        const int i = static_cast<int>(s);
        int v = baseStats[i] + racialBonuses[i] + classBonuses[i]
              + equipBonuses[i] + featBonuses[i] + conditionModifiers[i];
        if (v < STAT_RANK_MIN) v = STAT_RANK_MIN;
        if (v > STAT_RANK_MAX) v = STAT_RANK_MAX;
        return v;
    }
    int con() const { return stat(Stat::CON); }
    int des() const { return stat(Stat::DES); }
    int int_() const { return stat(Stat::INT); }
    int car() const { return stat(Stat::CAR); }

    // ===== Iniciativa: desventaja si
    int initiativeBonus() const {
        int v = des();
        for (auto& c : conditions) {
            (void)c; // las condiciones especificas lo modificaran en P1; aqui desventaja o advantage en Initiative pero
        }
        return v;
    }

    // ===== Defensas (GAMEMACHINE_NECES §0 formulas
    // Formula canonica P0
    DefenseBlock defenses(const ClassDefinition* cls) const;

    // ===== Vida tope
    int healthCap(const TierRules& rules, const ClassDefinition* cls) const;

    // ===== Cached values (Fractura #1: para ICombatant sin argumentos) =====
    // GameSession llama a recompute_derived() cada vez que cambia:
    //   - baseStats/racialBonuses/classBonuses/equipBonuses/...
    //   - tier, classId (ClassDefinition), equipArmorBonus, etc.
    // Los valores cacheados son la fuente de Player/Enemy via m_sheet->cachedXxx.
    mutable int         cachedHealthCap = 0;
    mutable DefenseBlock cachedDefenses;

    // Recomputa cachedHealthCap y cachedDefenses. ClassDefinition* y TierRules
    // se pasan aquí (el caller tiene ClassCatalog y TierRules). Si cls es
    // nullptr usa una estructura "vacía" (stats 0, baseHealth 0).
    void recompute_derived(const TierRules& rules, const ClassDefinition* cls) const {
        cachedDefenses = defenses(cls);
        cachedHealthCap = healthCap(rules, cls);
    }

    // Helpers sin argumentos (usan el cached — requieren haber llamado
    // recompute_derived antes. Si no se ha llamado: defaults 0s defensas).
    int healthCap() const { return cachedHealthCap; }
    const DefenseBlock& defenseValues() const { return cachedDefenses; }

    // ===== Hand size (limite mano)
    int handLimit(const TierRules& rules) const {
        return rules.handLimit(tier, !secondClassId.empty());
    }

    // ===== Tier up (unico camino: mediante grantMilestone.
    // Retorna false si no sube (no hay milestone suficiente O ya en TIER_MAX).
    bool tierUp(const TierRules& rules, int milestoneCount);

    // ===== Condiciones aplicar/quitar
    void applyCondition(const std::string& conditionId, int rounds, int stacks = 1, const std::string& sourceId = "");
    void removeCondition(const std::string& conditionId);
    bool hasCondition(const std::string& conditionId) const;
    int  conditionStacks(const std::string& conditionId) const;

    // Tick al final de cada ronda
    void tickEndOfRound();

    // ===== Facción
    int  getRep(const std::string& factionId) const;
    void adjustRep(const std::string& factionId, int delta);
    int  repPriceModPct(const std::string& factionId) const;
};

} // namespace RPG
