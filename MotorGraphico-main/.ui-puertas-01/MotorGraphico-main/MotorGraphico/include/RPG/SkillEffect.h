// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   D-11  (fase D, 12 j) - Vocabulari d'efectes d'habilitat
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstdint>
#include <string>
#include <optional>

#include "Render/ICombatant.h"
#include "RPG/RandomEngine.h"

namespace RPG {
using ::ICombatant;

// ========== D-11 — Vocabulari d'efectes d'habilitat ==========
//
// DEC-02: "Una habilitat ha de fer exactament el que diu la carta."
// L'estructura d'una habilitat és sempre la mateixa (magnitud per
// grau, objectiu, canals); el que canvia és L'EFECTE que dispara.
//
// Cada família té contracte tipat, executor i prova pròpia.
// Un efecte no suportat es rebutja amb diagnòstic, sense gastar
// cost ni torn (SkillExecutor Step 8, abans de Step 9).
//
// effect_scaling del JSON (camp declarat per D-02) determina
// com escala la magnitud: "CON", "INT", "none", etc.

enum class EffectFamily : uint8_t {
    NONE = 0,
    DAMAGE,       // Ferir
    HEAL,         // Curar
    CONDITION,    // Aplicar condició (ardor, congelació, enverinament...)
    SUMMON,       // Invocar
    BUFF,         // Bonificar aliats
    DEBUFF,       // Penalizar enemigo
    EXTRA_DICE,   // Més daus
    REROLL,       // Repetir tirada
    STEAL,        // Robar
    CUSTOM        // Efecte definible per l'autor de la carta
};

enum class ScalingStat : uint8_t {
    NONE = 0,
    CON,
    INT,
    DES,
    CAR,
    TIER          // Escala amb el tier de la carta (1-5)
};

// Efecte resolt d'un camp effect_scaling del JSON.
struct SkillEffectDef {
    EffectFamily family = EffectFamily::NONE;
    ScalingStat scaling = ScalingStat::NONE;
    std::string scaling_raw;  // Valor brut del JSON ("CON", "INT", etc.)
    int base_magnitude = 0;   // Magnitud base (per escala)

    // Per efectes de condició: quina condició aplicar.
    std::string condition_id;

    // Per efectes d'invocació: quin summon crear.
    std::string summon_id;

    // Si true, l'efecte consumeix l'skill (per defecte true).
    bool consumes_skill = true;
};

// ==== Helpers string->enum ====
EffectFamily effect_family_from_string(const std::string& s);
std::string effect_family_to_string(EffectFamily f);

ScalingStat scaling_stat_from_string(const std::string& s);
std::string scaling_stat_to_string(ScalingStat s);

// Parseja effect_scaling (crud del JSON) i retorna l'efecte
// tipat. Si el scaling no es reconeix, retorna std::nullopt
// (l'executor ho rebutja amb diagnòstic).
std::optional<SkillEffectDef> parse_effect_scaling(const std::string& raw_scaling);

// Retorna la magnitud escalada segons el scaling i el caster.
int scale_magnitude(const SkillEffectDef& effect, const CharacterSheet* caster,
                    int base_magnitude);

// Executa l'efecte sobre el target. Retorna true si l'efecte
// s'aplicà correctament, false si no es reconeix o fallà.
// Definida a SkillEffect.cpp (necessita ICombatant complet).
bool execute_effect(const SkillEffectDef& effect, ICombatant& target,
                    RandomEngine& rng, int base_magnitude,
                    const CharacterSheet* caster = nullptr);

} // namespace RPG
