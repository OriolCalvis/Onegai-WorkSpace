#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "RPG/Stat.h"

namespace RPG {

// =========================== Rarity & Tier range =========================
// Sistema Onegai RPG Edicion 2 — GDD Sistema_Cartas_Tiers.md §3, §5.
// Range tiers 0..5 inclusive. GDD explicito (No Golden Core de 3 tiers).
constexpr int TIER_MIN = 0;
constexpr int TIER_MAX = 5;

enum class ItemRarity : uint8_t {
    Common    = 0,
    Uncommon  = 1,
    Rare      = 2,
    Epic      = 3,
    Legendary = 4,
    Mythic    = 5   // reserva para contenido final (faltan pocos: 5=mitico)
};

// Orden canonico por nombre lowercase (coincide con `rarityOrder` del JSON).
// Los catálogos y el HUD lo usan para parsear strings "rare" -> ItemRarity.
// Devuelve -1 si desconocida (mismo criterio que rarityRank anterior, solo
// que ahora con 6 rarezas).
int rarityRank(const std::string& rarity_name);
ItemRarity rarity_from_name(const std::string& rarity_name);
std::string rarity_name(ItemRarity r);

// ================================= TierRule ==============================
// 1 fila del array "tiers" en assets/rules/tier_rules.json.
// 1:1 con los campos del JSON. _optional = el JSON puede no traerlo; en
// loadFromString se les pone default razonable.
struct TierRule {
    int tier = 0;
    std::string name;
    std::string description;

    int handLimit = 6;
    std::string maxEquipRarity = "common";

    int healthCap = 27;
    int healthBonus = 0;

    bool allowMulticlass = false;
    int multiclassHandLimit = 0;

    int maxSummons = 1;
    int maxFeats = 0;
    int maxDivineCards = 0;
};

// ================================= TierRules =============================
// Singleton semantic (se instancia uno solo por GameSession, que es el
// propietario y lo carga desde JSON). NO es un singleton C++ real — GameSession
// lo posee y CharacterSheet/DicePoolEngine lo toman por const ref; lo
// importante es que el "qué tier significa" sea una única fuente de verdad,
// no hardcodear límites por ahí (respeta fractura #1).
class TierRules {
public:
    TierRules();

    // Carga desde JSON canonico en assets/rules/tier_rules.json (§P0-3).
    // Todo-o-nada: si algún tier está mal, devuelve Error y NO toca m_tiers.
    Result<int> loadFromString(const std::string& jsonText);
    Result<int> loadFromFile(const std::string& path);

    // Búsqueda: nullptr si tier fuera de rango / no existente (mismo criterio
    // permisivo que ObjectCatalog / find() general).
    const TierRule* find(int tier) const;
    const TierRule& at(int tier) const;

    // ==== Helpers ====
    int  handLimit(int tier, bool multiclass = false) const;
    int  healthCap(int tier) const;
    int  healthBonus(int tier) const;
    bool allowsMulticlass(int tier) const;
    bool canEquipRarity(int tier, const std::string& rarity) const;
    int  maxSummons(int tier) const;
    int  maxFeats(int tier) const;
    int  maxDivineCards(int tier) const;

    // Fórmula canónica GAMEMACHINE_NECESIDADES.md:
    //   Vida = min(VidaBaseClase + CON*3 + CAR*1 + BonoTier(tier-0 o de la formula? healthBonus(tier)),
    //              healthCap(tier))
    // + BonoEquipo (equipArmorHPBonus o lo que sea) se suma en el caller.
    int compute_cap_health(int tier,
                           int classBaseHealth,
                           int CON,
                           int CAR,
                           int extra_equipment_health_bonus = 0) const;

    std::size_t size() const { return m_tiers.size(); }

private:
    std::vector<TierRule> m_tiers;
};

} // namespace RPG
