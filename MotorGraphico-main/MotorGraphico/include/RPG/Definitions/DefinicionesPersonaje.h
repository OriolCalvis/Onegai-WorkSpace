#pragma once

// DATOS: Quien eres: clase, raza y trasfondo.
//
// Las tres las lee de golpe la creacion de personaje y comparten
// vocabulario (competencias, bonos, equipo inicial).
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
    // DIALOGOS-01 (Oriol, 13/10) — false: la clase sigue en el catalogo (PNJ,
    // enemigos, lore), pero la creacion no la ofrece ni la acepta. Se juegan
    // las 70 nuevas: las seis de CLASES-03, las 44 SP y las 20 propias.
    bool playable = true;
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
    // Lo que esta clase PUEDE aprender despues, no lo que trae puesto.
    // Venia en startingCards.learnableSkills y se tiraba entero: sin esto
    // no hay forma de saber que puede subir un personaje al progresar.
    std::vector<std::string> learnableSkillIds;
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

    // PNJ-02 (05/10/2026) — false: el pueblo sigue en el catalogo por el lore,
    // pero nadie lo juega (la creacion no lo ofrece ni lo acepta). Los yokai:
    // «son espiritus, ya dejaron de tener cuerpo material» (Oriol).
    bool playable = true;
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

}  // namespace RPG
