#include "Game/Skill.h"

#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/EnhanceHooks.h"
#include "RPG/SkillEffect.h"
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
    // Ed. 3 (bitácora 2.17): ya no hay CA/CD — el daño se resuelve en
    // tirada enfrentada y el defensor tira el pool de su stat gobernante
    // (hechizo/maldición: el stat prefijado por la carta; físico: el MAYOR
    // de DES/CON, que es lo que elige una defensa no jugadora). Curar y
    // buffar no se resiste: sin defensor, grado por la tirada propia.
    if (effect == SkillEffect::Damage) {
        Stat stat_defensa;
        switch (def.save_attribute) {
            case SaveAttribute::CON: stat_defensa = Stat::CON; break;
            case SaveAttribute::INT: stat_defensa = Stat::INT; break;
            case SaveAttribute::CAR: stat_defensa = Stat::CAR; break;
            case SaveAttribute::NONE:
            default: {
                const int des = target.stat(Stat::DES);
                const int con = target.stat(Stat::CON);
                stat_defensa = (con > des) ? Stat::CON : Stat::DES;
                break;
            }
        }
        ctx.override_target_dice = target.stat(stat_defensa);
    }

    SkillExecutor::Out out = SkillExecutor::execute(ctx);

    int before = target.health();
    int magnitude = out.final_magnitude;
    if (magnitude < 0)
        magnitude = 0;

    // D-11: si l'efecte no es suportat, rebutjar sense gastar
    // cost ni torn (DEC-28). No aplicar dany/cura.
    if (!out.effect_supported) {
        SkillApplyResult res;
        res.hpChange = 0;
        res.degree = static_cast<int>(out.final_degree);
        res.pendingConditionId = out.pending_condition_id;
        res.pendingConditionRounds = out.pending_condition_rounds;
        res.pendingSummonId = out.pending_summon_id;
        res.effectFamily = out.effect_family;
        res.effectSupported = false;
        res.effectDiagnostic = out.effect_diagnostic;
        return res;
    }

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
    res.pendingConditionId = out.pending_condition_id;
    res.pendingConditionRounds = out.pending_condition_rounds;
    res.pendingSummonId = out.pending_summon_id;
    // D-11: vocabulari d'efectes.
    res.effectFamily = out.effect_family;
    res.effectSupported = out.effect_supported;
    res.effectDiagnostic = out.effect_diagnostic;
    res.appliedEffects = out.applied_effects;
    return res;
}

// Construye un SkillDefinition con magnitudes prescritas (B/P/S/C) y la
// condicion de la carta (D-09). Usa los nombres reales de campos del
// catálogo de SkillDefinition.h; NO inventamos campos nuevos para mantener
// compatibilidad con el catálogo skills.json real.
RPG::SkillDefinition make_skel_def(const std::string& id, RPG::Stat casting_stat,
                                   RPG::SaveAttribute save_attr, int mag_botch, int mag_partial,
                                   int mag_success, int mag_critical,
                                   RPG::SkillTarget target = RPG::SkillTarget::SINGLE_ENEMY,
                                   const std::string& cond_id = "",
                                   RPG::Degree cond_min_degree = RPG::Degree::SUCCESS,
                                   int cond_rounds = 3) {
    RPG::SkillDefinition def;
    def.id = id;
    def.name = id;
    def.casting_stat = casting_stat;
    def.save_attribute = save_attr;
    def.action_type = RPG::ActionType::ACCION_PRINCIPAL;
    def.recovery = RPG::RecoveryPile::ACTIVE;
    def.target = target;
    def.magnitude_by_degree[0] = static_cast<float>(mag_botch);
    def.magnitude_by_degree[1] = static_cast<float>(mag_partial);
    def.magnitude_by_degree[2] = static_cast<float>(mag_success);
    def.magnitude_by_degree[3] = static_cast<float>(mag_critical);
    def.apply_condition_id = cond_id;
    def.apply_condition_min_degree = cond_min_degree;
    def.condition_duration_rounds = cond_rounds;
    return def;
}

}  // namespace

RPG::SkillDefinition definicionAtaqueBasico() {
    using namespace RPG;
    // SUCCESS = 10, coincide con kBasicAttackPower histórico.
    return make_skel_def("atac_basic", Stat::DES, SaveAttribute::NONE, 0, 5, 10, 15);
}

SkillApplyResult ApplyBasicAttackNd6(ICombatant& caster, ICombatant& target,
                                     RPG::RandomEngine& rng) {
    return executeViaSkillExecutor(definicionAtaqueBasico(), caster, target, rng,
                                   SkillEffect::Damage);
}

RPG::SkillDefinition definicionDeHabilidad(const Skill& skill) {
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
        make_skel_def(skill.id, casting_stat, save_attr, 0, mag_partial, mag_success, mag_critical,
                      skill.target, skill.applyConditionId,
                      (skill.applyConditionMinDegree < 0) ? RPG::Degree::BOTCH
                      : (skill.applyConditionMinDegree > 3) ? RPG::Degree::CRITICAL
                                                           : static_cast<RPG::Degree>(skill.applyConditionMinDegree),
                      skill.conditionDurationRounds);
    def.effect_scaling = (effect == ::SkillEffect::Heal) ? "INT" : "CON";
    return def;
}

SkillApplyResult ApplySkillEffect(const Skill& skill, ICombatant& caster, ICombatant& target,
                                  RPG::RandomEngine& rng) {
    return executeViaSkillExecutor(
        definicionDeHabilidad(skill), caster, target, rng,
        (skill.effect == ::SkillEffect::Damage) ? ::SkillEffect::Damage : ::SkillEffect::Heal);
}
