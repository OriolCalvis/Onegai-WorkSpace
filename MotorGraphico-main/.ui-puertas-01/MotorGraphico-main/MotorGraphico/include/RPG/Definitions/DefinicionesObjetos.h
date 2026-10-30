#pragma once

// DATOS: Condiciones, equipo y consumibles: lo que entra y sale del inventario.
//
// Las condiciones viven aqui porque quien las aplica es un objeto o un
// golpe, no la ficha.
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

// ================== ConditionDefinition (16 condiciones, D-04) ==================
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

    // D-04 — Duración declarada por la carta (clave `duration` del asset):
    // "hasta_curarse" (15 de las 16 publicadas) no expira por rondas — la
    // retira quien la cure — y "1_turn"/"N_turns" sí. Con ROUNDS manda
    // defaultRounds; con UNTIL_CURED las rondas solo mantienen viva la
    // condición (tick no las descuenta: ver ConditionEngine).
    enum class Duration { ROUNDS, UNTIL_CURED };
    Duration durationMode = Duration::ROUNDS;

    // Modificadores al final del tick
    int damagePerRound = 0;   // daño directo (sangrado, veneno)
    bool isNegative = true;   // la mayoría son negativos; curado/Inspirado se marcan false

    // Modificadores a stats (los aplica recalc_condition_modifiers sobre
    // sheet.conditionModifiers). Los de DEFENSA (caMod/saveMods) que había
    // aquí se han RETIRADO en D-04: los parseaba el cargador viejo, no los
    // leía nadie (verificado en src/include/tests/examples) y Ed. 3 deroga
    // la CA como número de dificultad. Las cartas expresan esos ajustes
    // como hooks crudos (bono_ca:2, penal_ca:2) y quedan en mechanicHooks
    // hasta que D-11 les dé ejecutor tipado.
    std::array<int, 4> statMod = {0,0,0,0};

    // Flags de comportamiento
    bool preventAction = false;       // paralizado/inconsciente
    bool halfSpeed = false;           // ralentizado
    bool speedZero = false;           // velocidad 0 (agarrado/anclado/paralizado, D-11)
    bool grantAdvantageToEnemies = false;   // cegado/caido
    bool grantDisadvantageToCaster = false; // cegado/enfurecido en algunos casos
    bool breaksOnDamage = false;      // encantado

    // D-04 — Efectos declarados por la carta, en su DSL crudo
    // (mechanicHook: "dano_por_turno:1", "desventaja:ataque",
    // "bloquea:vision"...). El cargador mapea a los campos de arriba SOLO
    // los que tienen traducción unívoca y respaldada por el GDD; el resto
    // queda aquí, machine-readable, para D-11 (vocabulario de efectos),
    // E-14 (selectores) y G-13 (menús de datos).
    std::vector<std::string> mechanicHooks;
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

    // "Head", "Torso", "Legs", "Feet", "MainHand", "OffHand", "Accessory", "TwoHanded", "Consumable";
    // PART-02: "Arms" (brazos, Tier 1), "Neck" (collar), "Belt" (cinturon), "Ring" (anillo) (Tier 2)
    std::string slot;
    // "light" / "medium" / "heavy"
    std::string weightCategory;
    // Símbolos de compatibilidad: △ ○ □ ✦ ☠ ✝ ♞ ⚙
    std::vector<std::string> compatibleSymbols;
    // Tags de equipo (Arcano, Ligera...)
    std::vector<std::string> grantedTags;

    std::array<int, 4> statBonuses = {0, 0, 0, 0};
    // D-05 — Requisitos para portarlo (`requiredStats` de la carta: orden
    // CON/DES/INT/CAR) y penalizadores mientras se lleva (`penalties`,
    // magnitudes a restar). can_equip() exige los primeros; apply_bonus()
    // resta los segundos.
    std::array<int, 4> requiredStats = {0, 0, 0, 0};
    std::array<int, 4> statPenalties = {0, 0, 0, 0};
    // NIVELES-01 (Oriol, 06/10: «limitar ciertos objetos con los niveles,
    // para nivelar el progreso»): nivel del heroe para llevarlo puesto. Se
    // puede tener en la mochila antes. 0 o 1 = sin requisito.
    int requiredLevel = 0;
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
    std::string flavorText;
    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;
    int price = 0;

    // D-05 — Lo que la carta SÍ trae (31/31): usos y tipo de acción
    // (`accion_menor`/`accion`/`reaccion`). `effect.description` es prosa,
    // no ejecutable (igual que mechanicTags).
    int uses = 1;
    std::string actionType;

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

}  // namespace RPG
