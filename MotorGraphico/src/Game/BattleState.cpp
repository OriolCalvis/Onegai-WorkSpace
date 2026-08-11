#include "Game/BattleState.h"

#include "Render/ICombatant.h"
#include "Level/ObjectCatalog.h"
#include "Game/Skill.h"

#include <algorithm>

namespace {

// Etiquetas cortas de grado Nd6 ONEgAI para el log de combate (Fase 9).
const char* degreeLabel(int d) {
    switch (d) {
        case 0:
            return "Fracàs";
        case 1:
            return "Parcial";
        case 2:
            return "Èxit";
        case 3:
            return "Crític";
        default:
            return "?";
    }
}

bool participantAlive(const BattleParticipant& p) {
    return p.combatant != nullptr && p.combatant->isAlive();
}

// Primer participante vivo de "side", en orden. -1 si no queda ninguno.
// Sin aleatoriedad a proposito (ver el comentario de resolveEnemyTurn()
// en el header): mismo objetivo siempre para el mismo estado, para poder
// testear el resultado exacto de una ronda.
int firstAliveIndex(const std::vector<BattleParticipant>& side) {
    for (std::size_t i = 0; i < side.size(); ++i) {
        if (participantAlive(side[i])) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

}  // namespace

BattleState::BattleState(std::vector<BattleParticipant> allies,
                         std::vector<BattleParticipant> enemies, SkillCatalog* catalog,
                         ObjectCatalog* objectCatalog)
    : m_allies(std::move(allies))
    , m_enemies(std::move(enemies))
    , m_catalog(catalog)
    , m_objectCatalog(objectCatalog) {
    checkOutcome();
}

void BattleState::logLine(const std::string& line) { m_log.push_back(line); }

void BattleState::checkOutcome() {
    if (m_outcome != BattleOutcome::InProgress) {
        return;  // ya termino (incluye Fled): nada lo puede pisar despues
    }
    if (firstAliveIndex(m_allies) < 0) {
        m_outcome = BattleOutcome::Defeat;
    } else if (firstAliveIndex(m_enemies) < 0) {
        m_outcome = BattleOutcome::Victory;
    }
}

void BattleState::resolveAllyAction(std::size_t allyIndex, const BattleAction& action) {
    if (m_outcome != BattleOutcome::InProgress) {
        return;
    }
    if (allyIndex >= m_allies.size() || !participantAlive(m_allies[allyIndex])) {
        return;
    }
    BattleParticipant& actor = m_allies[allyIndex];

    if (action.type == BattleActionType::Flee) {
        m_outcome = BattleOutcome::Fled;
        logLine(actor.name + " huye del combate.");
        return;
    }

    if (action.type == BattleActionType::Item) {
        // Antes de la validacion de objetivo enemigo: un item actua
        // sobre el propio actor (ver BattleAction en el header), asi que
        // targetIndex no aplica -- igual que Flee.
        resolveItemAction(actor, action.itemId);
        return;
    }

    if (action.targetIndex < 0 ||
        static_cast<std::size_t>(action.targetIndex) >= m_enemies.size() ||
        !participantAlive(m_enemies[static_cast<std::size_t>(action.targetIndex)])) {
        return;  // objetivo invalido/muerto: el llamador (HUD/IA) deberia haberlo filtrado
    }
    BattleParticipant& target = m_enemies[static_cast<std::size_t>(action.targetIndex)];

    if (action.type == BattleActionType::Attack) {
        // === Ataque básico vía Nd6 ONEgAI (antes: kBasicAttackPower) ===
        SkillApplyResult r = ApplyBasicAttackNd6(*actor.combatant, *target.combatant, *m_rngInUse);
        logLine(actor.name + " ataca a " + target.name + " (" + degreeLabel(r.degree) + ": " +
                std::to_string(r.hpChange) + " de dano).");
    } else {  // UseSkill
        const Skill* skill = m_catalog != nullptr ? m_catalog->find(action.skillId) : nullptr;
        if (skill == nullptr || actor.skills == nullptr || !actor.skills->canUse(*skill)) {
            logLine(actor.name + " no puede usar \"" + action.skillId + "\".");
            return;
        }
        actor.skills->spend(*skill);
        SkillApplyResult r =
            ApplySkillEffect(*skill, *actor.combatant, *target.combatant, *m_rngInUse);
        logLine(actor.name + " usa " + skill->name + " sobre " + target.name + " (" +
                degreeLabel(r.degree) + ": " + std::to_string(r.hpChange) + ").");
    }

    checkOutcome();
}

void BattleState::resolveItemAction(BattleParticipant& actor, const std::string& itemId) {
    // Cada fallo deja una linea en el log en vez de ser un no-op mudo
    // (mismo criterio que UseSkill sin mana, ver resolveAllyAction en el
    // header): el HUD (Fase 9) muestra el motivo igual que un turno
    // valido. Ninguno de los fallos consume el item ni el turno de
    // logica interna -- BattleState no lleva cuenta de turnos, eso es de
    // quien orqueste el bucle (ver el comentario de la clase).
    if (actor.inventory == nullptr) {
        logLine(actor.name + " no lleva inventario.");
        return;
    }
    auto it = std::find(actor.inventory->begin(), actor.inventory->end(), itemId);
    if (it == actor.inventory->end()) {
        logLine(actor.name + " no tiene \"" + itemId + "\".");
        return;
    }
    const ObjectDefinition* def =
        m_objectCatalog != nullptr ? m_objectCatalog->find(itemId) : nullptr;
    if (def == nullptr || def->category != ObjectCategory::Pickup) {
        logLine(actor.name + " no puede usar \"" + itemId + "\".");
        return;
    }
    if (def->pickup.effect == PickupEffect::None) {
        // Un item sin efecto consumible (una llave) NO se gasta: usarlo
        // en combate simplemente no hace nada, y sigue en el inventario
        // para su uso real (abrir la puerta) fuera del combate.
        logLine(def->name + " no tiene efecto en combate.");
        return;
    }

    // Efecto valido: aplicar y CONSUMIR (borrar UNA instancia -- dos
    // pociones = dos entradas, ver BattleParticipant::inventory).
    if (def->pickup.effect == PickupEffect::Heal) {
        int before = actor.combatant->health();
        actor.combatant->heal(def->pickup.power);
        int amount = actor.combatant->health() - before;  // real, con clamp (como ApplySkillEffect)
        logLine(actor.name + " usa " + def->name + " (+" + std::to_string(amount) + " PV).");
    } else {  // RestoreMana
        if (actor.skills == nullptr) {
            // Sin SkillSet no hay mana que restaurar: no consumir un
            // item que no puede hacer nada (mismo criterio que None).
            logLine(actor.name + " no tiene mana que restaurar.");
            return;
        }
        int before = actor.skills->mana();
        actor.skills->restoreMana(def->pickup.power);
        int amount = actor.skills->mana() - before;
        logLine(actor.name + " usa " + def->name + " (+" + std::to_string(amount) + " PM).");
    }
    actor.inventory->erase(it);
}

void BattleState::resolveEnemyTurn() {
    if (m_outcome != BattleOutcome::InProgress) {
        return;
    }

    for (BattleParticipant& enemy : m_enemies) {
        if (m_outcome != BattleOutcome::InProgress) {
            break;  // el combate pudo terminar a mitad de ronda (ver el comentario del header)
        }
        if (!participantAlive(enemy)) {
            continue;
        }

        int targetIdx = firstAliveIndex(m_allies);
        if (targetIdx < 0) {
            break;  // sin aliados vivos: checkOutcome() ya lo marco Defeat en el turno anterior
        }
        BattleParticipant& target = m_allies[static_cast<std::size_t>(targetIdx)];

        // IA minima: primera habilidad conocida (orden alfabetico, ver
        // SkillSet::knownSkillIds()) que pueda pagar; si ninguna, ataque
        // basico.
        const Skill* chosen = nullptr;
        if (enemy.skills != nullptr && m_catalog != nullptr) {
            for (const std::string& id : enemy.skills->knownSkillIds()) {
                const Skill* candidate = m_catalog->find(id);
                if (candidate != nullptr && enemy.skills->canUse(*candidate)) {
                    chosen = candidate;
                    break;
                }
            }
        }

        if (chosen != nullptr) {
            enemy.skills->spend(*chosen);
            SkillApplyResult r =
                ApplySkillEffect(*chosen, *enemy.combatant, *target.combatant, *m_rngInUse);
            logLine(enemy.name + " usa " + chosen->name + " sobre " + target.name + " (" +
                    degreeLabel(r.degree) + ": " + std::to_string(r.hpChange) + ").");
        } else {
            // === Ataque basico enemigo via Nd6 ONEgAI ===
            SkillApplyResult r =
                ApplyBasicAttackNd6(*enemy.combatant, *target.combatant, *m_rngInUse);
            logLine(enemy.name + " ataca a " + target.name + " (" + degreeLabel(r.degree) + ": " +
                    std::to_string(r.hpChange) + " de dano).");
        }

        checkOutcome();
    }
}
