#pragma once

// DATOS: Lo que cuenta la historia: aventuras, eventos, lugares y encuentros.
//
// Aguas arriba esta el canon (Onegai 2); aqui solo se declara la forma.
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

// ================== EncounterTableDefinition (GDB-10.3) ==================
struct EncounterTableDefinition {
    std::string id;
    std::string name;
    std::string description;
    int tier = 0;
    ItemRarity rarity = ItemRarity::Common;

    struct Entry {
        std::string refId;       // "monster_XXX" / "npc_XXX" / "loot_XXX"
        int weight = 1;
        int min = 1;
        int max = 1;
    };
    std::vector<Entry> entries;

    // Condiciones de activación (opcional)
    struct Condition {
        std::string type;        // "zone" / "time" / "flag" / "random"
        std::string value;       // zone_id / time_range / flag_name / threshold
        std::string comparison;  // "==" / "!=" / ">=" / "<=" / ">" / "<"
    };
    std::vector<Condition> conditions;

    // Loot table asociado (opcional)
    std::string lootTableId;

    // Rareza del encuentro
    int minPartyLevel = 1;
    int maxPartyLevel = 5;
};

}  // namespace RPG
