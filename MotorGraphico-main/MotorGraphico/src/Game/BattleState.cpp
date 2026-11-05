// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 4 paquetes del corte B:
//   C-01  (fase C, 2 j) - La curació cura qui toca
//   D-09  (fase D, 2 j) - Les cartes apliquen les seves condicions
//   D-11  (fase D, 12 j) - Vocabulari d'efectes d'habilitat
//   G-13  (fase G, 5 j) - Comandes de combat i objectius des de dades
//
// AVISO - FICHERO DE CRUCE. Con 4 paquetes encima, es de los
// sitios donde dos agentes se pisan sin enterarse. Antes de una
// refactorizacion grande aqui, mira si hay trabajo sin commitear
// (git status) y di en que paquete estas trabajando.
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Game/BattleState.h"

#include "RPG/ConditionEngine.h"

#include "Render/ICombatant.h"
#include "Level/ObjectCatalog.h"
#include "Game/Skill.h"

#include <algorithm>

namespace {

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

// D0036 — El id de cada participante, por bando e indice: nunca se quita a nadie de un bando.
static std::vector<BattleParticipant> conIds(std::vector<BattleParticipant> gente, const char* bando) {
    for (std::size_t i = 0; i < gente.size(); ++i) gente[i].id = bando + std::to_string(i);
    return gente;
}

BattleState::BattleState(std::vector<BattleParticipant> allies,
                         std::vector<BattleParticipant> enemies, SkillCatalog* catalog,
                         ObjectCatalog* objectCatalog)
    : m_allies(conIds(std::move(allies), "a"))
    , m_enemies(conIds(std::move(enemies), "e"))
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

Game::ContextoBatalla BattleState::contexto() {
    Game::ContextoBatalla c{m_allies, m_enemies, m_log, *m_rngInUse, m_catalog, m_conditions};
    if (m_campo.tablero)
        c.llega = [this](std::size_t e, std::size_t a) {
            return e < m_enemies.size() && a < m_allies.size() && llegaA(m_enemies[e], m_allies[a]);
        };
    if (m_campo.tablero)
        c.casillaDe = [this](bool aliado, std::size_t i) -> std::optional<GridCoord> {
            const auto& lado = aliado ? m_allies : m_enemies;
            return i < lado.size() ? m_campo.tablero->dondeEsta(lado[i].id) : std::nullopt;
        };
    if (m_campo.tablero)
        c.muro = [this](int x, int y) {  // como Game::esMuro: no se pisa y no hay nadie
            return !m_campo.tablero->transitable(GridCoord{x, y}) && m_campo.tablero->quienEsta(GridCoord{x, y}) == nullptr;
        };
    if (m_campo.tablero)
        c.alLado = [this](std::size_t a, std::size_t e) {
            return a < m_allies.size() && e < m_enemies.size() && m_campo.tablero->aTiro(m_allies[a].id, m_enemies[e].id, 1);
        };
    return c;
}

void BattleState::checkOutcome() {
    if (m_outcome != BattleOutcome::InProgress) {
        return;  // ya termino (incluye Fled): nada lo puede pisar despues
    }
    // AV13-33 (D0015 §7): si cae el heroe se acaba, aunque los suyos sigan en pie (te rescatan).
    if (firstAliveIndex(m_allies) < 0 || (m_allies.front().combatant != nullptr && !m_allies.front().combatant->isAlive())) {
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
    // AV14-01: al acabar, lo que ha cambiado, como golpes.
    struct Apunta { BattleState* b; FotoVidas antes; std::string de; ~Apunta() { b->anotaGolpes(antes, de, Game::GolpeCombate::Grado::Acierto); } };
    const Apunta apunta{this, fotoVidas(), m_allies[allyIndex].id};
    // CAER-01 — Antes que la mesa: si no, la mesa daria el turno por cerrado.
    if (action.type == BattleActionType::Flee && m_sinHuida) {
        logLine("De este combate no se huye.");
        return;
    }
    // H2-06 — Con mesa de cartas, el heroe juega desde ella (Game/MesaCombate.h).
    if (m_mesa != nullptr) {
        Game::ContextoBatalla c = contexto();
        if (m_mesa->resolverAliado(c, allyIndex, action)) {
            checkOutcome();
            return;
        }
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

    // AV13-35 — El ataque y las habilidades los resuelve la mesa (siempre hay una en el juego). Lo que
    // la mesa no conoce no hace nada: el combate antiguo, sin mesa, se borro.
    if (action.type == BattleActionType::UseSkill) logLine(actor.name + " no puede usar \"" + action.skillId + "\".");
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
    // El combate pudo terminar a mitad de ronda (ver el comentario del header).
    for (std::size_t i = 0; i < m_enemies.size() && m_outcome == BattleOutcome::InProgress; ++i)
        resuelveEnemigo(i);
}

void BattleState::resuelveEnemigo(std::size_t indice) {
    if (m_outcome != BattleOutcome::InProgress || indice >= m_enemies.size()) return;
    BattleParticipant& enemy = m_enemies[indice];
    // AV14-01: al acabar, lo que ha cambiado, como golpes.
    struct Apunta { BattleState* b; FotoVidas antes; std::string de; ~Apunta() { b->anotaGolpes(antes, de, Game::GolpeCombate::Grado::Acierto); } };
    const Apunta apunta{this, fotoVidas(), enemy.id};
    struct Guion { BattleState* b; std::size_t i; ~Guion() { b->guionGolpeMortal(i); } };  // AV13-34, antes que Apunta
    const Guion guion{this, indice};
    {
        if (!participantAlive(enemy)) {
            return;
        }
        // AV13-12 — Con tablero, el enemigo solo pega si queda a tiro de alguien: al lado, o a
        // 5 casillas viendole si tiene habilidades. Se acerca antes, en acercaEnemigos().
        if (m_campo.tablero) {
            bool aTiro = false;
            for (const BattleParticipant& a : m_allies) aTiro = aTiro || (participantAlive(a) && llegaA(enemy, a));
            if (!aTiro) return;
        }
        // H2-06 — Con mesa de cartas, el enemigo ataca contra los estados y
        // las reacciones del heroe (Game/MesaCombate.h).
        if (m_mesa != nullptr) {
            Game::ContextoBatalla c = contexto();
            if (m_mesa->resolverEnemigo(c, indice)) {
                checkOutcome();
                return;
            }
        }

        // AV13-35 — Sin mesa, el enemigo no hace nada: el combate antiguo (su IA de habilidades) se borro.
    }
}

// H0-06 — Un tick por ronda de combate. CondicionEngine::tick_conditions
// calcula los efectos pero NO toca la vida (ver ConditionEngine.h): el
// propietario de la vida es quien la aplica. Aqui BattleState es ese
// propietario, igual que cuando resuelve un golpe: aplica el damageInflicted
// acumulado del DOT con takeDamage y notifica la perdida real con
// on_damage_received (por si una condicion activa rompe al recibir dano).
void BattleState::tickRound() {
    if (m_outcome != BattleOutcome::InProgress) {
        return;  // nada que ticar: el combate ya termino (incluye Fled)
    }
    // AV14-01: al acabar, lo que ha cambiado, como golpes.
    struct Apunta { BattleState* b; FotoVidas antes; std::string de; ~Apunta() { b->anotaGolpes(antes, de, Game::GolpeCombate::Grado::Periodico); } };
    const Apunta apunta{this, fotoVidas(), std::string()};
    const auto tickSide = [this](std::vector<BattleParticipant>& side) {
        for (BattleParticipant& participant : side) {
            if (!participantAlive(participant)) {
                continue;
            }
            RPG::CharacterSheet* sheet = participant.combatant->character_sheet();
            if (sheet == nullptr || m_conditions == nullptr || sheet->conditions.empty()) {
                continue;
            }
            const int previousHealth = participant.combatant->health();
            const auto events = RPG::ConditionEngine::tick_conditions(*sheet, *m_conditions);
            int totalDot = 0;
            for (const auto& ev : events) {
                if (ev.damageInflicted > 0) {
                    totalDot += ev.damageInflicted;
                }
                if (ev.expired) {
                    logLine(participant.name + ": " + ev.conditionId + " se ha agotado.");
                }
            }
            if (totalDot > 0) {
                participant.combatant->takeDamage(totalDot);
                logLine(participant.name + " sufre " + std::to_string(totalDot) +
                        " de daño por condiciones.");
                notifyDamageReceived(*participant.combatant, previousHealth);
            }
            checkOutcome();
            if (m_outcome != BattleOutcome::InProgress) {
                return;  // el DOT pudo acabar con un bando: no seguir ticando
            }
        }
    };
    tickSide(m_allies);
    tickSide(m_enemies);
    if (m_campo.tablero) {  // AV13-12: ronda nueva, pasos nuevos; los caidos dejan su casilla
        m_campo.tablero->reponPasos();
        for (const auto* lado : {&m_allies, &m_enemies})
            for (const BattleParticipant& p : *lado)
                if (!participantAlive(p)) m_campo.tablero->quita(p.id);
    }
    // H2-06 — Los estados de las cartas tambien avanzan (dano por turno,
    // duraciones, invocaciones que se acaban).
    if (m_mesa != nullptr && m_outcome == BattleOutcome::InProgress) {
        Game::ContextoBatalla c = contexto();
        m_mesa->finDeRonda(c);
        checkOutcome();
    }
}
