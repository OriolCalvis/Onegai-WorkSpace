// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-20  (fase C, 2 j) - Un sol escriptor de condicions
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "RPG/CharacterSheet.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace RPG {

// ============================================================
// Helpers de RpgCoreDefinitions que no van en header (redundante ponerlos
// inline, mejor en este translation unit que linkea con el CharacterSheet).
// ============================================================
static Stat stat_from_string(const std::string& s_in) {
    std::string s;
    s.reserve(s_in.size());
    for (auto c : s_in) s.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    if (s == "CON" || s == "CONSTITUCION") return Stat::CON;
    if (s == "DES" || s == "DESTREZA")     return Stat::DES;
    if (s == "INT" || s == "INTELIGENCIA") return Stat::INT;
    if (s == "CAR" || s == "CARISMA")      return Stat::CAR;
    return Stat::INT;
}
Stat ClassDefinition::primary_stat_enum() const { return stat_from_string(primaryStat); }
Stat ClassDefinition::secondary_stat_enum() const { return stat_from_string(secondaryStat); }

// ============================================================
// Formula canonical (GDD Onegai Ed. 2)
//   CA física   = 10 + DES + peso_armadura_permitido + eqArmor + eqShield
//   Defensa mental = 10 + CAR + mentalSave
//   Resistencia física = 10 + CON + physicalSave
//   Prec. mágica = 10 + INT + spellSave
// ============================================================
DefenseBlock CharacterSheet::defenses(const ClassDefinition* /*cls*/) const {
    DefenseBlock d;
    int ca  = 10 + des() + equipArmorBonus + equipShieldBonus;
    int dfs = 10 + int_() + spellSaveExtra;
    int defM  = 10 + car() + mentalSaveBonus;
    int resF = 10 + con() + physicalSaveBonus;
    d.values[static_cast<int>(Defense::ARMOR_CLASS)] = ca;
    d.values[static_cast<int>(Defense::PHYSICAL_SAVE)] = resF;
    d.values[static_cast<int>(Defense::WILL_SAVE)] = defM;
    d.values[static_cast<int>(Defense::SPELL_SAVE_DC)] = dfs;
    return d;
}

// ============================================================
// Vida tope = min( baseHealth + CON*3 + CAR*1 + healthBonus_tier + eqBonus, healthCap_tier )
// ============================================================
int CharacterSheet::healthCap(const TierRules& rules, const ClassDefinition* cls) const {
    // La formula vive una sola vez en healthCapWith() (H2-04): esta y el
    // refresco de tierUp comparten la misma copia.
    return healthCapWith(rules, (cls) ? cls->baseHealth : 8);
}

// ============================================================
// Tier up: cada tier N→N+1 requiere (N+1) milestones. Por ejemplo T1→T2 2 hitos.
// Se le pasa el total de milestones obtenidos; devuelve true si al menos 1 salto.
// ============================================================
bool CharacterSheet::tierUp(const TierRules& rules, int milestoneCount) {
    const int tierAntes = tier;
    while (tier < TIER_MAX) {
        // Milestones necesarios para pasar a (tier+1) — GDD: T0→T1 = 1,
        // T1→T2 = 2, T2→T3 = 3, T3→T4 = 4, T4→T5 = 5.
        const int needed = tier + 1;
        if (milestoneCount < needed) break;
        // La regla del tier permite multiclass?
        const TierRule* tr = rules.find(tier + 1);
        if (!tr) break;
        tier += 1;
        milestoneCount -= needed;
    }
    if (tier == tierAntes) {
        return false;
    }
    refrescaTrasSubir(rules);
    return true;
}

// H2-04 — el cap cacheado no puede quedarse en el tier viejo: es lo que
// leen Player::maxHealth() y el HUD. El baseHealth viene del ultimo
// recompute (cachedBaseHealth); la formula es la misma de siempre.
// El tope de dados del equipo sube con el tier, y va antes porque la vida
// lee CON y CAR.
void CharacterSheet::refrescaTrasSubir(const TierRules& rules) {
    if (cachedTopeEquipo > 0) {
        const TierRule* fila = rules.find(tier);
        cachedTopeEquipo = fila != nullptr ? fila->topeDadosEquipo : 0;
    }
    cachedHealthCap = healthCapWith(rules, cachedBaseHealth);
}

CharacterSheet::TierChange CharacterSheet::grantLevels(int count, const TierRules& rules) {
    return grantLevelPoints(count > 0 ? count * kPuntosPorNivel : 0, rules);
}

CharacterSheet::TierChange CharacterSheet::grantLevelPoints(int puntos, const TierRules& rules) {
    if (puntos > 0) {
        const long total = static_cast<long>(std::max(1, nivel)) * kPuntosPorNivel +
                           std::clamp(progresoNivel, 0, kPuntosPorNivel - 1) + puntos;
        nivel = static_cast<int>(total / kPuntosPorNivel);
        progresoNivel = static_cast<int>(total % kPuntosPorNivel);
    }
    return alcanzaTier(rules);
}

// NIVELES-01 (Oriol, 06/10): veinte niveles por tier y, para pasar, los
// hitos del tier (un hito por capitulo): 2 para salir de T1, 4 mas de T2, 8
// de T3 y 11 de T4; T5 es el techo. Sin ellos no se pasa del nivel 20 (40,
// 60, 80) ni del tier; con el nivel y los hitos, tier siguiente y se puede
// seguir al 21 (41...).
int CharacterSheet::hitosParaSalirDe(int tierDesde) {
    switch (tierDesde) {
        case 0:
        case 1: return 2;
        case 2: return 4;
        case 3: return 8;
        case 4: return 11;
        default: return 0;  // Tier 5: el techo, no se sale
    }
}

int CharacterSheet::nivelMaximo() const {
    // Con los hitos del tier hechos se puede pasar del tope del tier (y al
    // llegar a el se sube); sin ellos, no.
    const int tope = kNivelesPorTier * std::max(1, tier);
    const bool hechos = tier < TIER_MAX && hitosTier >= hitosParaSalirDe(tier);
    return std::min(kNivelMax, hechos ? tope + kNivelesPorTier : (tier >= TIER_MAX ? kNivelMax : tope));
}

int CharacterSheet::hitosQueFaltan() const {
    return tier >= TIER_MAX ? 0 : std::max(0, hitosParaSalirDe(tier) - hitosTier);
}

CharacterSheet::TierChange CharacterSheet::alcanzaTier(const TierRules& rules) {
    TierChange cambio;
    cambio.from = tier;
    nivel = std::max(1, nivel);
    // Se sube con el nivel del tope (20, 40...) y los hitos del tier; al
    // subir, los hitos vuelven a cero (Oriol: «por cada tier reinicias»).
    while (tier >= 1 && tier < TIER_MAX && hitosTier >= hitosParaSalirDe(tier) &&
           nivel >= kNivelesPorTier * tier && rules.find(tier + 1) != nullptr) {
        ++tier;
        hitosTier = 0;
    }
    if (nivel >= nivelMaximo()) {
        nivel = nivelMaximo();
        if (tier < TIER_MAX || nivel >= kNivelMax) progresoNivel = 0;  // en el tope no se acumula
    }
    progresoNivel = std::clamp(progresoNivel, 0, kPuntosPorNivel - 1);
    if (tier != cambio.from) refrescaTrasSubir(rules);
    cambio.to = tier;
    cambio.advanced = cambio.to != cambio.from;
    return cambio;
}

CharacterSheet::TierChange CharacterSheet::grantMilestones(int count, const TierRules& rules) {
    if (count > 0) {
        milestones += count;
        // Los del tier no se acumulan de mas: lo que sobra no pasa al siguiente.
        hitosTier = std::min(hitosParaSalirDe(tier), hitosTier + count);
    }
    return alcanzaTier(rules);
}

void CharacterSheet::restauraProgresion(int tierGuardado, int nivelGuardado, int progreso, int hitosDelTier,
                                         int hitosTotales, const TierRules& rules) {
    // Se rehace el camino: cada tier pasado con sus hitos y su nivel tope.
    while (tier < std::min(tierGuardado, TIER_MAX) && tier >= 1 && rules.find(tier + 1) != nullptr) {
        hitosTier = hitosParaSalirDe(tier);
        nivel = std::max(nivel, kNivelesPorTier * tier);
        alcanzaTier(rules);
    }
    milestones = std::max(0, hitosTotales);
    hitosTier = std::clamp(hitosDelTier, 0, std::max(0, hitosParaSalirDe(tier)));
    nivel = std::max(1, nivelGuardado);
    progresoNivel = progreso;
    alcanzaTier(rules);
}

// ============================================================
// Manejo de condiciones
// ============================================================
bool CharacterSheet::hasCondition(const std::string& conditionId) const {
    return conditionStacks(conditionId) > 0;
}
int  CharacterSheet::conditionStacks(const std::string& conditionId) const {
    for (const auto& c : conditions) {
        if (c.conditionId == conditionId && c.roundsRemaining > 0 && c.stacks > 0)
            return c.stacks;
    }
    return 0;
}

// ============================================================
// Facciones
// ============================================================
int CharacterSheet::getRep(const std::string& factionId) const {
    auto it = factionRep.find(factionId);
    return (it == factionRep.end()) ? 0 : it->second;
}
void CharacterSheet::adjustRep(const std::string& factionId, int delta) {
    auto it = factionRep.find(factionId);
    if (it == factionRep.end()) {
        factionRep[factionId] = delta;
    } else {
        it->second += delta;
    }
    int& v = factionRep[factionId];
    if (v < -100) v = -100;
    if (v > 100)  v = 100;
}
// Rep normalizado: -100 (aliado enemigo, precios ×2.0) .. +100 (aliado, 0.5x)
int CharacterSheet::repPriceModPct(const std::string& factionId) const {
    int rep = getRep(factionId);
    // lineal de -100→+100% a +100→-50% (P0 simplificado)
    // Rep -100: 100%, 0:0%, +100:-50%
    return static_cast<int>(-0.75f * rep);
}

}  // namespace RPG
