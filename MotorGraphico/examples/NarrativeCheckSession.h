#pragma once

#include <algorithm>
#include <utility>

#include "Check.h"
#include "Game/GameSession.h"

namespace demo {

// Fixture sin mapa ni GL: solo aporta el stat del jugador. La resolucion
// de dados, compatibilidad de aventuras y escritura de flags son las
// mismas de la partida, dentro de GameSession.
class NarrativeCheckBody final : public ICombatant {
public:
    explicit NarrativeCheckBody(int dice) : m_dice(dice) {}
    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::clamp(m_health + amount, 0, 10); }
    int health() const override { return m_health; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return m_health > 0; }
    int stat(RPG::Stat) const override { return m_dice; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "demo_skill_check"; }

private:
    int m_dice;
    int m_health = 10;
};

inline void resolveNarrativeCheck(const RPG::Catalogs::SkillCatalog& catalog, int dice,
                                 const RPG::SkillCheckRequest& request, RPG::RandomEngine& rng,
                                 RPG::NarrativeState& state) {
    require(catalog.find(request.skillId) != nullptr);
    RPG::NarrativeEffect effect;
    effect.type = RPG::EffectType::SkillCheck;
    effect.skillCheck = request;
    RPG::NarrativeBeat beat;
    beat.id = "__demo_skill_check";
    beat.once = false;
    beat.trigger.type = RPG::TriggerType::Enter;
    beat.trigger.target = "__demo_skill_check";
    beat.effects.push_back(std::move(effect));
    RPG::AdventureScript adventure;
    adventure.beats.push_back(std::move(beat));
    RPG::NarrativeEngine narrative;
    narrative.setAdventure(&adventure);
    NarrativeCheckBody player(dice);
    GameSession session(nullptr, nullptr, nullptr, LevelDefinition{}, &player, nullptr, nullptr);
    session.setNd6SkillCatalog(&catalog);
    session.setRandomEngine(rng);
    session.setNarrative(&narrative, &state);
    require(session.enterLevelNarrative("__demo_skill_check"));
}

}  // namespace demo
