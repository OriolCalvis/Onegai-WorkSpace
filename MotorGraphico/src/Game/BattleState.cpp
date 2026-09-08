#include "Game/BattleState.h"

#include "RPG/ConditionEngine.h"

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

// C-01 — Bando sobre el que actua una habilidad. Hasta ahora
// resolveAllyAction ignoraba Skill::target y buscaba SIEMPRE el objetivo
// dentro de m_enemies: "Cura" gastaba los PM del jugador y curaba al
// monstruo. Self/SingleAlly/AllAllies apuntan al bando propio; el resto,
// al contrario.
//
// Las variantes All* se resuelven aqui sobre UN solo participante del
// bando correcto, igual que las Single: el area de efecto es funcion
// nueva y la fase C no admite ninguna (regla del pla definitiu, punto 6).
// Lo que se arregla es el BANDO, que era el bug.
bool skillTargetsOwnSide(SkillTarget target) {
    return target == SkillTarget::Self || target == SkillTarget::SingleAlly ||
           target == SkillTarget::AllAllies;
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

void BattleState::notifyDamageReceived(ICombatant& target, int previousHealth) {
    const int lost = previousHealth - target.health();
    if (lost > 0 && m_conditions != nullptr && target.character_sheet() != nullptr) {
        RPG::ConditionEngine::on_damage_received(*target.character_sheet(), *m_conditions, lost);
    }
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

    // C-01 — La habilidad se resuelve ANTES de validar el objetivo,
    // porque es ella la que dice contra que bando se valida. De paso, el
    // orden deja el PM correcto: se comprueba que la habilidad existe y
    // se puede pagar, se comprueba el objetivo, y solo entonces se gasta
    // -- una sola vez y solo si el efecto se llega a aplicar. Antes, la
    // validacion de objetivo iba primero pero SIEMPRE contra m_enemies.
    const Skill* skill = nullptr;
    if (action.type == BattleActionType::UseSkill) {
        skill = m_catalog != nullptr ? m_catalog->find(action.skillId) : nullptr;
        if (skill == nullptr || actor.skills == nullptr || !actor.skills->canUse(*skill)) {
            logLine(actor.name + " no puede usar \"" + action.skillId + "\".");
            return;
        }
    }

    // Attack va siempre contra el bando contrario; UseSkill, contra el
    // que diga Skill::target.
    const bool ownSide = skill != nullptr && skillTargetsOwnSide(skill->target);
    std::vector<BattleParticipant>& side = ownSide ? m_allies : m_enemies;

    // Self no admite eleccion: el objetivo es quien actua, y targetIndex
    // se ignora (mismo criterio que Item/Flee). Para el resto,
    // targetIndex indexa el bando que se acaba de decidir.
    std::size_t chosenIndex = allyIndex;
    if (skill == nullptr || skill->target != SkillTarget::Self) {
        if (action.targetIndex < 0 ||
            static_cast<std::size_t>(action.targetIndex) >= side.size() ||
            !participantAlive(side[static_cast<std::size_t>(action.targetIndex)])) {
            return;  // objetivo invalido/muerto: el llamador (HUD/IA) deberia haberlo filtrado
        }
        chosenIndex = static_cast<std::size_t>(action.targetIndex);
    }
    BattleParticipant& target = side[chosenIndex];
    const int previousHealth = target.combatant->health();

    if (action.type == BattleActionType::Attack) {
        // === Ataque básico vía Nd6 ONEgAI (antes: kBasicAttackPower) ===
        SkillApplyResult r = ApplyBasicAttackNd6(*actor.combatant, *target.combatant, *m_rngInUse);
        logLine(actor.name + " ataca a " + target.name + " (" + degreeLabel(r.degree) + ": " +
                std::to_string(r.hpChange) + " de daño).");
    } else {  // UseSkill
        actor.skills->spend(*skill);
        SkillApplyResult r =
            ApplySkillEffect(*skill, *actor.combatant, *target.combatant, *m_rngInUse);
        logLine(actor.name + " usa " + skill->name + " sobre " + target.name + " (" +
                degreeLabel(r.degree) + ": " + std::to_string(r.hpChange) + ").");
    }

    notifyDamageReceived(*target.combatant, previousHealth);
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

        const int previousHealth = target.combatant->health();
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
                    std::to_string(r.hpChange) + " de daño).");
        }

        notifyDamageReceived(*target.combatant, previousHealth);
        checkOutcome();
    }
}
