#pragma once

// DATOS: Lo que llevas sin elegirlo: pasivas, dotes, rasgos y deidades.
//
// Cuatro fichas pequenas que se consultan al resolver una tirada o un
// favor. Cambian poco y casi siempre a la vez.
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

    // H2-06 — effects[].mechanicHook de la pasiva, crudos (el DSL de las
    // cartas; ver SkillDefinition::mechanicHooks).
    std::vector<std::string> mechanicHooks;
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

// ================== DeityDefinition ==================
// DIOSES-01 (08/10/2026) — Oriol: «Los dioses y lore es mío». Las religiones
// del juego son los Creadores de su vault (rank "creador"); las diez deidades
// locales que trajo el importador ("deidad_local") ya no se ofrecen al crear
// personaje ni salen en el códice, y siguen en el catálogo solo porque las
// nombran objetos, hechizos, villanos y clases (ver su "_nota").
struct DeityDefinition {
    std::string id;
    std::string name;
    std::string title;     // "Diosa de la Vida"...
    std::string alignment; // Héroe, Neutral, Villano, etc.
    std::string description;
    std::string flavorText;  // en los Creadores, su mensaje (Astrologia.docx)
    std::string rank;      // "creador" o "deidad_local"
    bool esCreador() const { return rank == "creador"; }
    ItemRarity rarity = ItemRarity::Common;
    std::vector<std::string> domains;      // "Guerra", "Magia", "Naturaleza"...
    std::vector<std::string> symbolTags;   // tags de compatibilidad ✝ ☠ etc.
    // D-07 — Razas/trasfondos compatibles (`compatibleWith` de la carta).
    std::vector<std::string> compatibleWithIds;
    // D-07 — Obligaciones del devoto (prosa, una por entrada) y favor
    // (`{description, scaling}`): se guardan en crudo; tipar el favor es D-11.
    std::vector<std::string> obligations;
    std::string favorDescription;
    std::string favorScaling;

    int tier_min = 0;

    // Plegarias (ids de spell que puede lanzar el devoto)
    std::vector<std::string> prayerSkillIds;
    // Condición de roto (abandono): si el PJ rompe dogma, pierde estas passives
    std::vector<std::string> grantedPassiveIds;
    std::array<int, 4> dogmaStatBonus = {0,0,0,0};
};

}  // namespace RPG
