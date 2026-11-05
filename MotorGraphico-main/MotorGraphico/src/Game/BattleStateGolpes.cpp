// AV14-01 — Los golpes que resuelve la batalla (ver include/Game/GolpesCombate.h).
//
// Cada accion (la de un aliado, el turno de un enemigo, el cierre de la ronda)
// hace una foto de las vidas antes y, al acabar, apunta lo que ha cambiado. El
// grado sale de la tirada de la mesa contra ese mismo (la ultima de la accion):
// critico, acierto o roce si entro; fallo o pifia si no entro y nadie cambio de
// vida. Sin tirada (el veneno al cerrar la ronda, el combate sin cartas), el
// grado que diga quien llama.
#include "Game/BattleState.h"

#include "Render/ICombatant.h"

#include <algorithm>

namespace Game {

const char* nombreDe(GolpeCombate::Grado g) {
    switch (g) {
        case GolpeCombate::Grado::Fallo: return "Fallo";
        case GolpeCombate::Grado::Pifia: return "Pifia";
        case GolpeCombate::Grado::Roce: return "Roce";
        case GolpeCombate::Grado::Acierto: return "Acierto";
        case GolpeCombate::Grado::Critico: return "Crítico";
        case GolpeCombate::Grado::Cura: return "Cura";
        case GolpeCombate::Grado::Periodico: return "Periódico";
    }
    return "";
}

}  // namespace Game

BattleState::FotoVidas BattleState::fotoVidas() const {
    FotoVidas f;
    for (const BattleParticipant& p : m_allies) f.aliados.push_back(p.combatant ? p.combatant->health() : 0);
    for (const BattleParticipant& p : m_enemies) f.enemigos.push_back(p.combatant ? p.combatant->health() : 0);
    if (m_mesa != nullptr)
        if (const auto* t = m_mesa->tiradasVisibles(); t != nullptr && !t->empty()) f.tirada = t->back().numero;
    return f;
}

void BattleState::anotaGolpes(const FotoVidas& antes, const std::string& de, Game::GolpeCombate::Grado porDefecto) {
    colocaLosNuevos();  // D0037: una carta puede haber invocado a alguien
    using Grado = Game::GolpeCombate::Grado;
    std::vector<const RPG::TiradaVisible*> nuevas;
    if (m_mesa != nullptr)
        if (const auto* ts = m_mesa->tiradasVisibles())
            for (const RPG::TiradaVisible& t : *ts)
                if (t.numero > antes.tirada && t.enfrentada) nuevas.push_back(&t);
    const auto suTirada = [&](const std::string& id) -> const RPG::TiradaVisible* {
        for (auto it = nuevas.rbegin(); it != nuevas.rend(); ++it)
            if ((*it)->contraId == id) return *it;
        return nullptr;
    };
    // D0036: `de` llega como id; el nombre es lo que se cuenta.
    std::string nombreDe;
    for (const auto* gente : {&m_allies, &m_enemies})
        for (const BattleParticipant& p : *gente)
            if (p.id == de) nombreDe = p.name;
    std::vector<std::string> vistos;
    const auto lado = [&](const std::vector<BattleParticipant>& gente, const std::vector<int>& vidas, bool aliado) {
        for (std::size_t i = 0; i < gente.size() && i < vidas.size(); ++i) {
            const BattleParticipant& p = gente[i];
            if (p.combatant == nullptr || p.combatant->health() == vidas[i]) continue;
            Game::GolpeCombate g;
            g.aAliado = aliado;
            g.i = i;
            g.a = p.name;
            g.de = nombreDe;
            g.deId = de;
            g.cambio = p.combatant->health() - vidas[i];
            g.grado = g.cambio > 0 ? Grado::Cura : porDefecto;
            if (const RPG::TiradaVisible* t = suTirada(p.id); t != nullptr && g.cambio < 0) {
                g.de = t->quien;
                g.deId = t->quienId;
                g.que = t->que;
                g.grado = t->grado == RPG::Degree::CRITICAL ? Grado::Critico
                        : t->grado == RPG::Degree::PARTIAL  ? Grado::Roce
                                                            : Grado::Acierto;
            }
            vistos.push_back(p.id);
            m_campo.apunta(std::move(g));
        }
    };
    lado(m_allies, antes.aliados, true);
    lado(m_enemies, antes.enemigos, false);
    // Lo que no entro: la tirada enfrentada que se quedo corta, sobre quien no cambio de vida.
    for (const RPG::TiradaVisible* t : nuevas) {
        if (t->grado >= RPG::Degree::SUCCESS || std::find(vistos.begin(), vistos.end(), t->contraId) != vistos.end())
            continue;
        Game::GolpeCombate g;
        bool hallado = false;
        for (const auto* gente : {&m_allies, &m_enemies})
            for (std::size_t i = 0; i < gente->size() && !hallado; ++i)
                if ((*gente)[i].id == t->contraId) {
                    hallado = true;
                    g.aAliado = gente == &m_allies;
                    g.i = i;
                }
        if (!hallado) continue;
        g.a = t->contra;
        g.de = t->quien;
        g.deId = t->quienId;
        g.que = t->que;
        g.grado = t->pifia() ? Grado::Pifia : t->grado == RPG::Degree::PARTIAL ? Grado::Roce : Grado::Fallo;
        vistos.push_back(t->contraId);
        m_campo.apunta(std::move(g));
    }
    // AV13-29 — Quien cae deja libre su casilla (el heroe no: con el se acaba el combate).
    if (m_campo.tablero)
        for (const auto* gente : {&m_allies, &m_enemies})
            for (std::size_t i = 0; i < gente->size(); ++i)
                if ((gente != &m_allies || i > 0) && (*gente)[i].combatant != nullptr && !(*gente)[i].combatant->isAlive())
                    m_campo.tablero->quita((*gente)[i].id);
}
