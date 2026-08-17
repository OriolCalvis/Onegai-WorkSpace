#include "Game/Skill.h"

#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/EnhanceHooks.h"
#include "Render/ICombatant.h"

#include <algorithm>

SkillSet::SkillSet(int maxMana) : m_mana(maxMana), m_maxMana(maxMana) {}

void SkillSet::learn(const std::string& skillId) { m_known[skillId] = true; }

bool SkillSet::knows(const std::string& skillId) const {
    return m_known.find(skillId) != m_known.end();
}

std::vector<std::string> SkillSet::knownSkillIds() const {
    std::vector<std::string> ids;
    ids.reserve(m_known.size());
    for (const auto& entry : m_known) {
        ids.push_back(entry.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

void SkillSet::setMaxMana(int maxMana) {
    m_maxMana = maxMana;
    if (m_mana > m_maxMana) {
        m_mana = m_maxMana;
    }
}

bool SkillSet::canUse(const Skill& skill) const {
    return knows(skill.id) && m_mana >= skill.mpCost;
}

void SkillSet::spend(const Skill& skill) {
    m_mana -= skill.mpCost;
    if (m_mana < 0) {
        m_mana = 0;
    }
}

void SkillSet::restoreMana(int amount) {
    m_mana += amount;
    if (m_mana > m_maxMana) {
        m_mana = m_maxMana;
    }
}

void SkillCatalog::add(Skill skill) { m_skills[skill.id] = std::move(skill); }

bool SkillCatalog::has(const std::string& id) const { return m_skills.find(id) != m_skills.end(); }

const Skill* SkillCatalog::find(const std::string& id) const {
    auto it = m_skills.find(id);
    return it != m_skills.end() ? &it->second : nullptr;
}

namespace {

// Helper comun: construye ExecutionContext + SkillDefinition, invoca
// SkillExecutor, aplica el resultado sobre target (Damage o Heal) y
// devuelve SkillApplyResult con el HP real aplicado.
SkillApplyResult executeViaSkillExecutor(const RPG::SkillDefinition& def, ICombatant& caster,
                                         ICombatant& target, RPG::RandomEngine& rng,
                                         SkillEffect effect) {
    using namespace RPG;

    ExecutionContext ctx;
    ctx.caster = nullptr;  // Usamos override_base_dice (Render/ICombatant
    ctx.target = nullptr;  //   no puede ser CharacterSheet* directamente).
    ctx.skill = &def;
    ctx.flags = ContextBonusFlag::NONE;
    ctx.qte = nullptr;
    ctx.rng = &rng;
    ctx.is_player_attacking = true;
    ctx.extra_dice = 0;
    ctx.force_dice_mod = DiceMod::NORMAL;
    ctx.post_roll_success_bonus = 0.0f;
    ctx.can_upgrade_botch_to_partial = false;
    ctx.can_upgrade_partial_to_success = false;
    ctx.perfect_critical_confirm_allowed = false;

    // === Overrides Modelo/Vista ===
    // Base dice: 1 + stat del caster (GDD §2.2: "pool N, 1 base").
    ctx.override_base_dice = 1 + caster.stat(def.casting_stat);
    // Defensa objetivo: según save_attribute de la Skill.
    int def_int = 10;
    switch (def.save_attribute) {
        case SaveAttribute::NONE:
            def_int = target.defense_value(Defense::ARMOR_CLASS);
            break;
        case SaveAttribute::CON:
            def_int = target.defense_value(Defense::PHYSICAL_SAVE);
            break;
        case SaveAttribute::DES:
            def_int = 10 + target.stat(Stat::DES);
            break;
        case SaveAttribute::INT:
            def_int = target.defense_value(Defense::WILL_SAVE);
            break;
        case SaveAttribute::CAR:
            def_int = 10 + target.stat(Stat::CAR);
            break;
    }
    ctx.override_target_defense = def_int;

    SkillExecutor::Out out = SkillExecutor::execute(ctx);

    int before = target.health();
    int magnitude = out.final_magnitude;
    if (magnitude < 0)
        magnitude = 0;

    if (effect == SkillEffect::Damage) {
        target.takeDamage(magnitude);
    } else {
        target.heal(magnitude);
    }

    int after = target.health();
    SkillApplyResult res;
    res.hpChange = (effect == SkillEffect::Damage) ? (before - after) : (after - before);
    if (res.hpChange < 0)
        res.hpChange = 0;
    res.degree = static_cast<int>(out.final_degree);
    return res;
}

// Construye un SkillDefinition con magnitudes prescritas (B/P/S/C).
// Usa los nombres reales de campos del catálogo de SkillDefinition.h; NO
// inventamos campos nuevos para mantener compatibilidad con el catálogo
// skills.json real.
RPG::SkillDefinition make_skel_def(const std::string& id, RPG::Stat casting_stat,
                                   RPG::SaveAttribute save_attr, int mag_botch, int mag_partial,
                                   int mag_success, int mag_critical) {
    RPG::SkillDefinition def;
    def.id = id;
    def.name = id;
    def.casting_stat = casting_stat;
    def.save_attribute = save_attr;
    def.action_type = RPG::ActionType::ACCION_PRINCIPAL;
    def.recovery = RPG::RecoveryPile::ACTIVE;
    def.target = RPG::SkillTarget::SINGLE_ENEMY;
    def.magnitude_by_degree[0] = static_cast<float>(mag_botch);
    def.magnitude_by_degree[1] = static_cast<float>(mag_partial);
    def.magnitude_by_degree[2] = static_cast<float>(mag_success);
    def.magnitude_by_degree[3] = static_cast<float>(mag_critical);
    return def;
}

}  // namespace

SkillApplyResult ApplyBasicAttackNd6(ICombatant& caster, ICombatant& target,
                                     RPG::RandomEngine& rng) {
    using namespace RPG;
    // SUCCESS = 10, coincide con kBasicAttackPower histórico.
    SkillDefinition def = make_skel_def("atac_basic", Stat::DES, SaveAttribute::NONE, 0, 5, 10, 15);
    return executeViaSkillExecutor(def, caster, target, rng, SkillEffect::Damage);
}

SkillApplyResult ApplySkillEffect(const Skill& skill, ICombatant& caster, ICombatant& target,
                                  RPG::RandomEngine& rng) {
    using namespace RPG;

    const int p = (skill.power < 0) ? 0 : skill.power;
    const int mag_partial = std::max(1, p / 2);
    const int mag_success = p;
    const int mag_critical = static_cast<int>(p * 1.5f);

    Stat casting_stat = Stat::DES;
    SaveAttribute save_attr = SaveAttribute::NONE;
    ::SkillEffect effect = skill.effect;

    if (effect == ::SkillEffect::Heal) {
        casting_stat = Stat::INT;
        save_attr = SaveAttribute::NONE;
    } else {
        casting_stat = Stat::DES;
        save_attr = SaveAttribute::NONE;
    }

    SkillDefinition def =
        make_skel_def(skill.id, casting_stat, save_attr, 0, mag_partial, mag_success, mag_critical);
    return executeViaSkillExecutor(
        def, caster, target, rng,
        (effect == ::SkillEffect::Damage) ? SkillEffect::Damage : SkillEffect::Heal);
}
