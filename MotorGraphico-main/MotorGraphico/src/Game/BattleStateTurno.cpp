// AV13-11 / AV13-12 — El turno sobre el tablero: quien juega cuando (la
// iniciativa) y hasta donde llega cada enemigo (acercarse y tener a tiro).
// Es BattleState, partido en dos ficheros para no pasar de 500 lineas.
#include "Game/BattleState.h"

#include "Game/Acompanante.h"
#include "Game/Skill.h"
#include "Render/ICombatant.h"

#include <string>
#include <vector>

namespace {

bool enPie(const BattleParticipant& p) { return p.combatant != nullptr && p.combatant->isAlive(); }

}  // namespace

int BattleState::alcanceDe(const BattleParticipant& enemigo) const {
    return enemigo.skills != nullptr && !enemigo.skills->knownSkillIds().empty() ? 5 : 1;
}

bool BattleState::llegaA(const BattleParticipant& enemigo, const BattleParticipant& aliado) const {
    return !m_campo.tablero || m_campo.tablero->aTiro(enemigo.id, aliado.id, alcanceDe(enemigo));
}

bool BattleState::acercaEnemigos(int solo) {
    if (!m_campo.tablero || m_outcome != BattleOutcome::InProgress) return false;
    std::vector<std::string> vivos;
    for (const BattleParticipant& a : m_allies)
        if (enPie(a)) vivos.push_back(a.id);
    bool alguno = false;
    for (std::size_t i = 0; i < m_enemies.size(); ++i) {
        const BattleParticipant& enemy = m_enemies[i];
        if (!enPie(enemy) || (solo >= 0 && static_cast<std::size_t>(solo) != i)) continue;
        std::vector<GridCoord> andada;
        const bool llega = m_campo.tablero->acerca(enemy.id, vivos, alcanceDe(enemy), &andada);
        if (andada.empty()) continue;
        alguno = true;
        logLine(enemy.name + (llega ? " se acerca." : " se acerca, pero no llega."));
    }
    return alguno;
}

void BattleState::tiraIniciativa() {
    if (m_allies.empty() || m_outcome != BattleOutcome::InProgress) return;
    const auto bono = [](const BattleParticipant& p) { return p.combatant ? p.combatant->initiative_bonus() : 0; };
    std::vector<int> bonosA, bonosE;
    std::vector<std::string> nombresA, nombresE;
    for (const BattleParticipant& a : m_allies) {  // AV13-29: el heroe y sus acompañantes
        bonosA.push_back(bono(a));
        nombresA.push_back(a.name);
    }
    for (const BattleParticipant& e : m_enemies) {
        bonosE.push_back(bono(e));
        nombresE.push_back(e.name);
    }
    m_campo.orden.tira(bonosA, bonosE, *m_rngInUse);
    // Los que ya no estan en pie (un enemigo que entra muerto) no juegan.
    m_campo.orden.saltaCaidos([this](const Game::Puesto& p) {
        const auto& lado = p.enemigo ? m_enemies : m_allies;
        return p.i < lado.size() && enPie(lado[p.i]);
    });
    logLine("Iniciativa (1d6 + DES): " + m_campo.orden.resumen(nombresA, nombresE));
}

int BattleState::aliadoEnTurno() const {
    const Game::Puesto* p = m_campo.orden.quien();
    if (p == nullptr) return 0;
    return p->enemigo ? -1 : static_cast<int>(p->i);
}

bool BattleState::loJuegaElJugador(std::size_t aliado) {
    if (m_mesa == nullptr) return aliado == 0;
    return m_mesa->juegaElJugador(contexto(), aliado);
}

void BattleState::resuelveAliadoSolo(std::size_t aliado) {
    if (m_outcome != BattleOutcome::InProgress || aliado >= m_allies.size() || !enPie(m_allies[aliado]) || !m_mesa)
        return;
    struct Apunta { BattleState* b; FotoVidas antes; std::string de; ~Apunta() { b->anotaGolpes(antes, de, Game::GolpeCombate::Grado::Acierto); } };
    const Apunta apunta{this, fotoVidas(), m_allies[aliado].id};
    if (m_campo.tablero) {  // anda antes de actuar: a por los enemigos si pega; si no (Xila), junto al heroe
        const auto* a = dynamic_cast<const Game::Acompanante*>(m_allies[aliado].combatant);
        const bool pega = a != nullptr && a->def().ataques > 0;
        std::vector<std::string> hacia;
        for (const BattleParticipant& e : pega ? m_enemies : std::vector<BattleParticipant>{m_allies.front()})
            if (enPie(e)) hacia.push_back(e.id);
        m_campo.tablero->acerca(m_allies[aliado].id, hacia, 1);
    }
    Game::ContextoBatalla c = contexto();
    m_mesa->actuaSolo(c, aliado);
    checkOutcome();
}

void BattleState::sumaAliado(BattleParticipant aliado) {
    aliado.id = "a" + std::to_string(m_allies.size());  // D0036
    m_allies.push_back(std::move(aliado));
    if (!m_campo.orden.vacio()) {  // AV13-29: con iniciativa, juega el ultimo de la ronda
        const ICombatant* cuerpo = m_allies.back().combatant;
        m_campo.orden.anade(Game::Puesto{false, m_allies.size() - 1, 0, cuerpo ? cuerpo->initiative_bonus() : 0});
    }
}

int BattleState::enemigoEnTurno() const {  // -1 tambien con un aliado en turno
    const Game::Puesto* p = m_campo.orden.quien();
    return p != nullptr && p->enemigo ? static_cast<int>(p->i) : -1;
}

void BattleState::pasaAlSiguiente() {
    if (m_campo.orden.vacio() || m_outcome != BattleOutcome::InProgress) return;
    const auto vivo = [this](const Game::Puesto& p) {
        const auto& lado = p.enemigo ? m_enemies : m_allies;
        return p.i < lado.size() && enPie(lado[p.i]);
    };
    if (m_campo.orden.avanza(vivo)) tickRound();  // ha dado la vuelta: ronda nueva
    // El dano por turno de tickRound pudo tumbar al que le tocaba.
    if (m_outcome == BattleOutcome::InProgress && m_campo.orden.saltaCaidos(vivo)) tickRound();
}

void BattleState::ponGolpeMortal(std::size_t enemigo, float umbral) {
    m_campo.golpeMortal = Game::CampoDeBatalla::GolpeMortal{true, enemigo, umbral, false};
}

void BattleState::guionGolpeMortal(std::size_t indice) {
    auto& g = m_campo.golpeMortal;
    if (!g.activo || g.dado || indice != g.enemigo || indice >= m_enemies.size() || m_allies.empty()) return;
    const BattleParticipant& enemigo = m_enemies[indice];
    ICombatant* heroe = m_allies.front().combatant;
    if (!enPie(enemigo) || heroe == nullptr) return;
    if (static_cast<float>(enemigo.combatant->health()) > g.umbral * static_cast<float>(enemigo.combatant->maxHealth()))
        return;
    // A las puertas: a 1 PV, tanto si el golpe se quedaba corto como si ya te mataba.
    if (heroe->health() > 1) heroe->takeDamage(heroe->health() - 1);
    if (heroe->health() <= 0) heroe->heal(1);
    if (m_outcome == BattleOutcome::Defeat) m_outcome = BattleOutcome::InProgress;
    logLine(enemigo.name + " te da un golpe mortal: caes de rodillas, a las puertas.");
    g.dado = true;
}
