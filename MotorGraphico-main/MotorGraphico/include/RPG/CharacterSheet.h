// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-20  (fase C, 2 j) - Un sol escriptor de condicions
//   D-06  (fase D, 3 j) - Classes, races i trasfons
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

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
#include "RPG/TopesDados.h"

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
//
// PART-02 (Oriol, 07/10/2026): «tier 1 que solo es casco, pechera, brazos, piernas, zapatos y armas, no
// tiene nada de anillos ni todas esas cosas que se desbloquean en tier 2 [...] donde van collares,
// cinturon y anillos». Los indices de antes no cambian (las pruebas y la mochila los usan): Accessory es
// ahora el anillo de la mano principal, y detras van los brazos, el collar, el cinturon y el anillo de
// la otra mano.
enum class EquipSlot : uint8_t {
    Head = 0, Torso, Legs, Feet, MainHand, OffHand, Accessory, Arms, Neck, Belt, Ring2, SLOT_COUNT
};

// 6 slots canónicos Onegai (la mayoria) + Accessory opcional:
constexpr size_t EQUIP_SLOT_COUNT = 6;
// PART-02: todas las ranuras, y lo puesto en cada una (idx = EquipSlot, "" si vacia).
constexpr size_t kRanurasDeEquipo = static_cast<size_t>(EquipSlot::SLOT_COUNT);
using EquipoPuesto = std::array<std::string, kRanurasDeEquipo>;
// PART-02: el tier desde el que se abre cada ranura (los anillos, el collar y el cinturon, desde el 2).
constexpr int tierDeRanura(EquipSlot s) {
    return (s == EquipSlot::Accessory || s == EquipSlot::Neck || s == EquipSlot::Belt || s == EquipSlot::Ring2)
               ? 2
               : 1;
}
// PART-02: si la ranura esta abierta en ese tier (una ficha sin tier, 0, cuenta como del 1: la creacion
// empieza en el 1).
constexpr bool ranuraAbierta(EquipSlot s, int tier) {
    return (tier < 1 ? 1 : tier) >= tierDeRanura(s);
}

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

    // H2-04 — hitos concedidos (TOTAL historico). Un hito por capitulo; hacen
    // falta 2, 4, 8 y 11 para salir de T1, T2, T3 y T4: son el permiso para
    // pasar del nivel 20 (40, 60...) al tier siguiente. El guardado lo lleva
    // (playerMilestones).
    int milestones = 0;

    // NIVELES-01 (Oriol, 06/10/2026): veinte niveles por tier. El heroe
    // empieza en el 1 y las aventuras los dan (efecto grantLevel); para
    // pasar del 20 a Tier 2 hacen falta ademas dos hitos (cada capitulo da
    // uno). Boundington entero deja, como mucho, el 17.
    static constexpr int kNivelMax = 100;
    static constexpr int kNivelesPorTier = 20;
    // Hitos para salir de cada tier (Oriol, 06/10): cada capitulo da uno.
    // T1 -> T2, 2 (Boundington y el capitulo 2 que elijas); T2 -> T3, 4 mas;
    // T3 -> T4, 8; T4 -> T5, 11. Tier 5 es el techo. Por cada tier se
    // empieza de cero: los que sobran no pasan al siguiente.
    static int hitosParaSalirDe(int tierDesde);
    // Hitos hechos en el tier actual (vuelven a 0 al subir) y los que faltan.
    int hitosTier = 0;
    int hitosQueFaltan() const;
    // El guardado: deja la ficha en ese tier, nivel y hitos, con los topes
    // del tier al dia.
    void restauraProgresion(int tier, int nivel, int progreso, int hitosDelTier, int hitosTotales,
                            const TierRules& rules);
    int nivel = 1;
    // Avance hacia el nivel siguiente, en puntos (kPuntosPorNivel = un
    // nivel): un paso de una mision secundaria da su trozo (Oriol, 06/10).
    static constexpr int kPuntosPorNivel = 100;
    int progresoNivel = 0;

    // D-06 — Les set eleccions narratives del trasfondo. Cadascuna es
    // un id estable que el motor narrativo pot escoltar (un beat/flag de
    // l'aventura activa reacciona a ell). Guardats AQUI i no en un DTO
    // temporal per que sobrevisquin al canvi de nivell (loadLevel destrueix
    // GameSession) i al guardat (SaveGame los serialitza).
    std::string bondId;
    std::string fearId;
    std::string flawId;
    std::string goalId;
    std::string idealId;
    std::string personalityId;
    std::string virtueId;

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
    // D-05 — Vida extra de l'equip (maxHealthBonus de la carta). L'acumula
    // InventoryEngine com els altres bonus; G-12 el suma al healthCap() com a
    // terme propi (junt amb l'aproximacio equipArmorBonus/2), dins del min()
    // del compute_cap_health.
    int equipMaxHealthBonus = 0;
    int physicalSaveBonus = 0;
    int mentalSaveBonus = 0;
    int spellSaveExtra = 0;

    // Inspiración (puntos de Heroe, GDD)
    int inspirationPoints = 0;

    // Equipo (ref ids -> EquipmentDefinition. idx = EquipSlot. "" si vacio). PART-02: las once ranuras
    // (las 6 canonicas, los dos anillos, los brazos, el collar y el cinturon).
    EquipoPuesto equippedIds;
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

    // Almacen de condiciones. ConditionEngine es el unico escritor de reglas;
    // la restauracion de datos debe purgar/recalcular usando el catalogo.
    std::vector<ActiveCondition> conditions;

    // ===== Stat totals (CON/DES/INT/CAR) = STAT_RANK_MIN..STAT_RANK_MAX
    int stat(Stat s) const {
        const int i = static_cast<int>(s);
        // El equipo cuenta con el tope de dados de su tier (RPG/TopesDados.h);
        // la raza y lo demas suman encima.
        const int equipo = cachedTopeEquipo > 0
                               ? TopesDados::aplica(equipBonuses, cachedTopeEquipo)[static_cast<std::size_t>(i)]
                               : equipBonuses[i];
        int v = baseStats[i] + racialBonuses[i] + classBonuses[i]
              + equipo + featBonuses[i] + conditionModifiers[i];
        if (v < STAT_RANK_MIN) v = STAT_RANK_MIN;
        if (v > STAT_RANK_MAX) v = STAT_RANK_MAX;
        return v;
    }
    int con() const { return stat(Stat::CON); }
    int des() const { return stat(Stat::DES); }
    int int_() const { return stat(Stat::INT); }
    int car() const { return stat(Stat::CAR); }

    // ===== Defensas (GAMEMACHINE_NECES §0 formulas
    // Formula canonica P0
    DefenseBlock defenses(const ClassDefinition* cls) const;

    // ===== Vida tope
    int healthCap(const TierRules& rules, const ClassDefinition* cls) const;

    // La misma formula con el baseHealth explicito. UNICA copia: healthCap()
    // y el refresco de tierUp() la comparten, para que el cache nunca pueda
    // divergir de la formula (H2-04).
    int healthCapWith(const TierRules& rules, int baseHealth) const {
        const int eqBonus = equipArmorBonus / 2 + equipMaxHealthBonus;
        return rules.compute_cap_health(tier, baseHealth, con(), car(), eqBonus);
    }

    // ===== Cached values (Fractura #1: para ICombatant sin argumentos) =====
    // GameSession llama a recompute_derived() cada vez que cambia:
    //   - baseStats/racialBonuses/classBonuses/equipBonuses/...
    //   - tier, classId (ClassDefinition), equipArmorBonus, etc.
    // Los valores cacheados son la fuente de Player/Enemy via m_sheet->cachedXxx.
    mutable int         cachedHealthCap = 0;
    mutable DefenseBlock cachedDefenses;
    // baseHealth del ultimo recompute (cls ? cls->baseHealth : 8): lo unico
    // que tierUp no puede deducir por su cuenta, y lo necesita para refrescar
    // el cap cuando sube el tier sin catalogo a mano (H2-04).
    mutable int         cachedBaseHealth = 8;
    // El tope de dados del equipo en el tier de la ficha (TierRule::
    // topeDadosEquipo); 0 = sin tope, como antes de llamar a recompute_derived.
    mutable int         cachedTopeEquipo = 0;

    // Recomputa cachedHealthCap y cachedDefenses. ClassDefinition* y TierRules
    // se pasan aquí (el caller tiene ClassCatalog y TierRules). Si cls es
    // nullptr usa una estructura "vacía" (stats 0, baseHealth 0).
    void recompute_derived(const TierRules& rules, const ClassDefinition* cls) const {
        const TierRule* fila = rules.find(tier);
        cachedTopeEquipo = fila != nullptr ? fila->topeDadosEquipo : 0;
        cachedDefenses = defenses(cls);
        cachedBaseHealth = (cls) ? cls->baseHealth : 8;
        cachedHealthCap = healthCapWith(rules, cachedBaseHealth);
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
    // Mueve el tier Y refresca cachedHealthCap (H2-04): antes dejaba el cap
    // del tier viejo y quien leia healthCap() veia vida tope antigua. Desde
    // el juego no se llama a mano: el camino publico es grantMilestones().
    bool tierUp(const TierRules& rules, int milestoneCount);


    // Que cambio el tier tras conceder hitos (para el HUD: "subes a tier 3").
    struct TierChange { bool advanced = false; int from = 0; int to = 0; };

    // H2-04 / NIVELES-01 — concede `count` hitos (total historico en
    // `milestones`; los del tier en `hitosTier`). Los hitos son el permiso
    // para pasar de tier: con el nivel 20 y 2 hitos, Tier 2; ahi se empieza
    // de cero y con el 40 y 4 mas, Tier 3; con el 60 y 8, Tier 4; con el 80
    // y 11, Tier 5. Sin ellos el heroe se queda en el nivel 20 (40...). Deja la ficha coherente (topes
    // de vida y de dados recalculados); los PV ACTUALES no viven en la
    // ficha. count<=0 no desordena nada.
    TierChange grantMilestones(int count, const TierRules& rules);

    // NIVELES-01 — sube `count` niveles, hasta el tope que dejan los hitos
    // (20 sin ninguno, 40 con uno...: nivelMaximo) y, con nivel e hito,
    // sube de tier. El tier nunca baja. count<=0 no hace nada.
    TierChange grantLevels(int count, const TierRules& rules);
    // Lo mismo en puntos (100 = un nivel): 25 es un cuarto de nivel. En el
    // tope (nivelMaximo) el avance se queda a 0: sin hito no se acumula.
    TierChange grantLevelPoints(int puntos, const TierRules& rules);
    int nivelMaximo() const;

    // ===== Consultas de condiciones; aplicar/quitar/tick viven en ConditionEngine.
    bool hasCondition(const std::string& conditionId) const;
    int  conditionStacks(const std::string& conditionId) const;

    // ===== Facción
    int  getRep(const std::string& factionId) const;
    void adjustRep(const std::string& factionId, int delta);
    int  repPriceModPct(const std::string& factionId) const;

private:
    // Tras subir de tier: el tope de dados del equipo y el de vida, al dia.
    void refrescaTrasSubir(const TierRules& rules);
    // El tier que dan el nivel y los hitos (el menor de los dos).
    TierChange alcanzaTier(const TierRules& rules);
};

} // namespace RPG
