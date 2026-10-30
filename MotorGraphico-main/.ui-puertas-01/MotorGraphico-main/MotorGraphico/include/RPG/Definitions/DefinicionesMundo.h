#pragma once

// DATOS: Lo que pone el mapa: trampas, botin y NPCs.
//
// Es lo que consume el editor al colocar cosas en un nivel.
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

// ================== TrapDefinition (51 trampas) ==================
struct TrapDefinition {
    std::string id;
    std::string name;
    std::string description;
    std::string flavorText;
    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;

    int damage = 6;            // danyo base (pisa trampa)
    std::string saveAttribute; // "DES", "CON"...
    float cd = 1.0f;           // CD Nd6 si procede
    bool detectBySearch = true;
    int detectDifficulty = 0;  // dificultad percepción pasiva

    // D-07 — Detección y desarme de la carta (`detection{stat,cd}` y
    // `disarm{stat,cd,herramienta}`, 51/51): stat en crudo, cd en éxitos
    // Nd6 (la carta usa 0.5/1/1.5/2/2.5). `trigger`/`onFail` son prosa.
    std::string detectStat;
    float detectCd = 0.0f;
    std::string disarmStat;
    float disarmCd = 0.0f;
    std::string disarmTool;
    // D-07 — Categoría (`mecánica/mágica/narrativa`, 51/51).
    std::string category;

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

    // D-07 — Rango de tiers (`tierRange[2]` de la carta; `tier` plano es 0
    // en las 201), pista narrativa (`clue`) y sistema alternativo de drops
    // (`drops[]`: chance + condición en crudo + item, en paralelo a
    // `entries[]` peso/min/max; D-11 decidirá cuál ejecuta).
    int tierMin = 0;
    int tierMax = 0;
    std::string clue;
    struct Drop {
        std::string itemId;
        int chance = 0;
        std::string condition;
    };
    std::vector<Drop> drops;
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

}  // namespace RPG
