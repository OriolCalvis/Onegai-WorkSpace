// EST-23b — LOS DATOS DE UNA PARTIDA, SEPARADOS DE LA CLASE QUE LOS MUEVE.
//
// GameSession.h tenia 40 miembros propios (umbral: 15). Este fichero
// los concentra en una unica clase de datos. GameSession solo los
// guarda via unique_ptr y los expone a traves de sessionData().

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Game/Morality.h"
#include "Game/SessionTipos.h"
#include "Level/LevelDefinition.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/Definitions/DefinicionesPersonaje.h"
#include "RPG/RandomEngine.h"
#include "RPG/TierRules.h"
#include "RPG/NarrativeEngine.h"
#include "World/World.h"

class RuntimeTileMap;
class ObjectCatalog;
class SkillCatalog;
class ICombatant;
class SkillSet;
class BattleState;
class Morality;
namespace RPG { namespace eventos { struct Catalogo; } }  // C8-03c

struct InventoryRules {
    const RPG::Catalogs::EquipmentCatalog* equipment = nullptr;
    const RPG::ClassDefinition* playerClass = nullptr;
    const RPG::TierRules* tiers = nullptr;
};

struct SessionData {
    RuntimeTileMap* m_map = nullptr;
    ObjectCatalog* m_catalog = nullptr;
    SkillCatalog* m_skillCatalog = nullptr;
    const RPG::Catalogs::MonsterCatalog* m_monsterCatalog = nullptr;
    LevelDefinition m_level;

    ICombatant* m_playerBody = nullptr;
    SkillSet* m_playerSkills = nullptr;
    std::vector<std::string>* m_inventory = nullptr;
    GridCoord m_playerPosition;

    Engine::World* m_world = nullptr;
    std::unique_ptr<Engine::World> m_ownedWorld;

    GameMode m_mode = GameMode::Exploration;
    const RPG::Catalogs::ConditionCatalog* m_conditions = nullptr;
    std::unique_ptr<BattleState> m_battle;
    std::vector<Engine::EntityHandle> m_battleEnemyHandles;  // los del combate (uno o un grupo)
    std::string m_lastPickupId;
    std::vector<std::string> m_log;

    std::string m_dialogueSpeaker;
    std::vector<std::string> m_dialogueLines;
    std::vector<ShopOffer> m_shopOffers;
    int m_shopBuybackPercent = 50;
    InventoryRules m_inventoryRules;
    LevelTransition m_transition;
    int m_gold = 0;

    std::vector<OwnedBusiness> m_businesses;
    std::string m_currentBusinessId;
    std::string m_currentBusinessName;
    int m_currentBusinessPrice = 0;
    int m_currentBusinessIncome = 0;

    Morality m_morality;

    int m_stepsSinceRent = 0;
    static constexpr int kStepsPerRentCycle = 40;

    const RPG::NarrativeEngine* m_narrative = nullptr;
    RPG::NarrativeState* m_narrativeState = nullptr;
    const RPG::eventos::Catalogo* m_eventos = nullptr;  // C8-03c: no propietario; nullptr = sin eventos
    bool m_evaluandoEventos = false;                    // C8-03c: una sola evaluacion a la vez
    bool m_eventosPendientes = false;                   // C8-03c: llamada de dentro; se repite al acabar la de fuera

    RPG::Xoroshiro128p m_rng;
    RPG::RandomEngine* m_rngInUse = &m_rng;

    const RPG::Catalogs::SkillCatalog* m_nd6Skills = nullptr;

    int m_nextSummonIndex = 0;
    // El combate que ha pedido la historia y aun no ha acabado: sus flags y
    // si es ineludible (CAER-01). Vacio fuera de combate o en uno del mapa.
    std::optional<RPG::BattleRequest> m_pendingBattle;
    std::vector<RPG::BattleRequest> m_battleQueue;
};
