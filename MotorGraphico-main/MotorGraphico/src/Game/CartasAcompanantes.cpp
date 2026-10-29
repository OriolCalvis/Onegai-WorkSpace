// ACOMPANANTES-01 — Los acompañantes de la historia en la mesa (ver
// include/Game/Acompanante.h): su cuerpo, su turno tras el del heroe y a
// quien van los enemigos cuando el heroe no esta solo.
//
// REGLAS (Oriol, 05/10: «solo en Boundington, y solo en algunos momentos de
// la historia»; las fichas, de PARTIDAS.docx)
//  - Actuan solos, en orden, cuando el heroe cierra su turno: despues de sus
//    invocaciones y antes que los enemigos. Cada uno es una accion aparte en
//    el historial de tiradas (sus golpes salen como «1/2» y «2/2»).
//  - Un sanador (Xila, «Healing Touch 2/day») cura 1d<dado>+<mas> a quien
//    este en apuros (vida <= umbral de su maximo; el mas apurado y, a la
//    par, el primero), mientras le queden curas. Las curas son de toda la
//    aventura: las lleva GameSession en una variable.
//  - Si no cura, golpea tantas veces como diga (Venides, Multiataque: dos)
//    al ultimo enemigo en pie: el heroe suele ir al primero, y asi cada uno
//    entretiene a uno, como los hombres de Venides «retienen a los Perdidos».
//  - Los enemigos van a por quien les ha hecho daño el ultimo (el heroe o un
//    acompañante) si sigue en pie y se le puede atacar; si no, a por el
//    primero que se pueda atacar: el heroe. A Xila no se le ataca.
//  - Venides «siempre huye en cuanto su vida llega a 0»: a 0 PV se retira.
//    Si cae el heroe, los demas se retiran con el y la batalla se pierde.
#include "CartasEnCombateInterno.h"

#include <algorithm>
#include <utility>

#include "Game/Acompanante.h"

namespace Game {

using T = Gancho::Tipo;

// --- El cuerpo ---------------------------------------------------------

Acompanante::Acompanante(RPG::AcompananteDef def, int curasQueQuedan)
    : m_def(std::move(def)), m_vida(m_def.vida > 0 ? m_def.vida : 1),
      m_curas(std::clamp(curasQueQuedan, 0, m_def.cura.veces)) {}

void Acompanante::takeDamage(int cantidad) { m_vida = cantidad >= m_vida ? 0 : m_vida - std::max(0, cantidad); }

void Acompanante::heal(int cantidad) { m_vida = std::min(m_def.vida, m_vida + std::max(0, cantidad)); }

bool Acompanante::gastaCura() {
    if (m_curas <= 0) return false;
    --m_curas;
    return true;
}

std::string Acompanante::claveCuras(const std::string& aventura, const std::string& id) {
    return "curas_" + id + (aventura.empty() ? std::string() : "_" + aventura);
}

// --- En la mesa --------------------------------------------------------

ICombatant* CartasEnCombate::sumaAcompanante(const RPG::AcompananteDef& def, int curasQueQuedan) {
    m_acompanantes.push_back(std::make_unique<Acompanante>(def, curasQueQuedan));
    return m_acompanantes.back().get();
}

Acompanante* CartasEnCombate::acompananteDe(const ContextoBatalla& c, std::size_t aliado) const {
    if (aliado >= c.aliados.size()) return nullptr;
    for (const auto& a : m_acompanantes)
        if (a.get() == c.aliados[aliado].combatant) return a.get();
    return nullptr;
}

bool CartasEnCombate::atacable(const ContextoBatalla& c, std::size_t aliado) const {
    if (!vivo(c, {Lado::Aliado, aliado})) return false;
    const Acompanante* a = acompananteDe(c, aliado);
    return a == nullptr || a->def().atacable;
}

int CartasEnCombate::objetivoDeEnemigo(const ContextoBatalla& c, std::size_t enemigo) const {
    // AV13-12: con tablero, solo a quien llega (el agresor si puede; si no, el primero a tiro).
    const auto llega = [&](std::size_t i) { return atacable(c, i) && (!c.llega || c.llega(enemigo, i)); };
    const int agresor = enemigo < m_enemigos.size() ? m_enemigos[enemigo].agresor : -1;
    if (agresor >= 0 && llega(static_cast<std::size_t>(agresor))) return agresor;
    for (std::size_t i = 0; i < c.aliados.size(); ++i)
        if (llega(i)) return static_cast<int>(i);
    return -1;
}

// Solo el heroe y sus acompañantes se ganan al enemigo: las invocaciones,
// como hasta ahora, no le quitan la atencion al heroe.
void CartasEnCombate::apuntaAgresor(const ContextoBatalla& c, Ref fuente, Ref objetivo) {
    if (fuente.lado != Lado::Aliado || objetivo.lado != Lado::Enemigo) return;
    if (fuente.i != 0 && acompananteDe(c, fuente.i) == nullptr) return;
    situ(objetivo).agresor = static_cast<int>(fuente.i);
}

void CartasEnCombate::caeAliado(ContextoBatalla& c, std::size_t aliado) {
    if (aliado == 0) {
        for (const auto& a : m_acompanantes) a->retirate();
        return;
    }
    if (const Acompanante* a = acompananteDe(c, aliado)) {
        c.log.push_back(a->def().nombre + (a->def().seRetira ? " se retira del combate." : " cae."));
    }
}

int CartasEnCombate::aQuienCura(const ContextoBatalla& c, const Acompanante& sanador) const {
    if (!sanador.cura() || sanador.curasQueQuedan() <= 0) return -1;
    int mejor = -1;
    float apuro = 2.0f;
    for (std::size_t i = 0; i < c.aliados.size(); ++i) {
        const ICombatant* x = c.aliados[i].combatant;
        if (x == nullptr || !x->isAlive() || x->maxHealth() <= 0 || x->health() >= x->maxHealth()) continue;
        const float queda = static_cast<float>(x->health()) / static_cast<float>(x->maxHealth());
        if (queda <= sanador.def().cura.umbral && queda < apuro) {
            apuro = queda;
            mejor = static_cast<int>(i);
        }
    }
    return mejor;
}

void CartasEnCombate::turnoAcompanantes(ContextoBatalla& c) {
    for (const auto& a : m_acompanantes) {
        int yo = -1;
        for (std::size_t i = 0; i < c.aliados.size() && yo < 0; ++i)
            if (c.aliados[i].combatant == a.get()) yo = static_cast<int>(i);
        if (yo < 0) continue;
        if (primerVivo(c, Lado::Enemigo) < 0) return;  // ya no queda nadie
        actuaUno(c, *a, static_cast<std::size_t>(yo));
    }
}

// AV13-29 — A quien puede pegar el aliado `yo`: el preferido si sigue en pie y lo tiene al lado; si
// no, el ultimo enemigo en pie que tenga al lado (sin tablero, el ultimo en pie). -1: nadie.
int CartasEnCombate::blancoAlLado(const ContextoBatalla& c, std::size_t yo, int preferido) const {
    const auto vale = [&](std::size_t e) { return vivo(c, {Lado::Enemigo, e}) && (!c.alLado || c.alLado(yo, e)); };
    if (preferido >= 0 && static_cast<std::size_t>(preferido) < c.enemigos.size() && vale(static_cast<std::size_t>(preferido)))
        return preferido;
    for (std::size_t i = c.enemigos.size(); i-- > 0;)
        if (vale(i)) return static_cast<int>(i);
    return -1;
}

// Lo que hace solo un acompañante: cura a quien este en apuros si le quedan curas; si no, golpea al
// ultimo enemigo en pie tantas veces como ataques tenga.
void CartasEnCombate::actuaUno(ContextoBatalla& c, Acompanante& a, std::size_t yo) {
    const Ref ref{Lado::Aliado, yo};
    if (!vivo(c, ref)) return;
    if (tiene(c, ref, T::Incapacitado)) {
        c.log.push_back(a.def().nombre + " no puede actuar.");
        return;
    }
    m_tiradas.empiezaAccion();  // lo suyo sale aparte de lo del heroe
    const int herido = aQuienCura(c, a);
    if (herido >= 0 && a.gastaCura()) {
        ICombatant& x = *c.aliados[static_cast<std::size_t>(herido)].combatant;
        const int antes = x.health();
        x.heal(c.rng.roll_in_range(1, a.def().cura.dado) + a.def().cura.mas);
        c.log.push_back(a.def().nombre + " cura a " + c.aliados[static_cast<std::size_t>(herido)].name + " (+" +
                        std::to_string(x.health() - antes) + " PV; le quedan " + std::to_string(a.curasQueQuedan()) + ").");
        return;
    }
    for (int golpe = 0; golpe < a.def().ataques && vivo(c, ref); ++golpe) {
        const int e = blancoAlLado(c, yo, -1);  // el ultimo enemigo en pie que tenga a mano
        if (e < 0) break;
        golpeDeArma(c, ref, {Lado::Enemigo, static_cast<std::size_t>(e)}, nullptr);
    }
}

// --- AV13-29 (Oriol, 14/10, D0015 §1): los acompañantes con su turno, jugados por el jugador ---

bool CartasEnCombate::juegaElJugador(const ContextoBatalla& c, std::size_t aliado) const {
    if (aliado == 0) return true;
    const Acompanante* a = acompananteDe(c, aliado);
    return a != nullptr && !a->def().soloIA;  // los de "ia": true, solos; las invocaciones no tienen turno
}

bool CartasEnCombate::actuaSolo(ContextoBatalla& c, std::size_t aliado) {
    Acompanante* a = acompananteDe(c, aliado);
    if (a == nullptr) return false;
    asegurarTamanos(c);
    actuaUno(c, *a, aliado);
    return true;
}

int CartasEnCombate::curasDe(const ContextoBatalla& c, std::size_t aliado) const {
    const Acompanante* a = acompananteDe(c, aliado);
    return a == nullptr || !a->cura() ? -1 : a->curasQueQuedan();
}

bool CartasEnCombate::resolverAcompanante(ContextoBatalla& c, std::size_t aliado, const BattleAction& accion) {
    Acompanante* a = acompananteDe(c, aliado);
    if (a == nullptr) return false;  // una invocacion u otro: el camino de siempre
    asegurarTamanos(c);
    const Ref yo{Lado::Aliado, aliado};
    SituacionCombate& s = m_aliados[aliado];
    if (accion.type == BattleActionType::UseSkill && accion.skillId == kAccionPasar) {
        c.log.push_back(a->def().nombre + " termina su turno.");
        s.actuo = false;  // el turno que viene, otra accion
        return true;
    }
    if (s.actuo) {
        c.log.push_back(a->def().nombre + " ya ha actuado este turno.");
        return true;
    }
    if (tiene(c, yo, T::Incapacitado)) {
        c.log.push_back(a->def().nombre + " no puede actuar.");
        return true;
    }
    m_tiradas.empiezaAccion();
    if (accion.type == BattleActionType::Attack) {
        int e = accion.targetIndex;
        if (e < 0 || static_cast<std::size_t>(e) >= c.enemigos.size() || !vivo(c, {Lado::Enemigo, static_cast<std::size_t>(e)}))
            e = primerVivo(c, Lado::Enemigo);
        // Si el primer golpe lo tumba, los que le quedan van a otro que tenga al lado.
        for (int golpe = 0; e >= 0 && golpe < std::max(1, a->def().ataques) && vivo(c, yo); ++golpe) {
            golpeDeArma(c, yo, {Lado::Enemigo, static_cast<std::size_t>(e)}, nullptr);
            if (!vivo(c, {Lado::Enemigo, static_cast<std::size_t>(e)})) e = blancoAlLado(c, aliado, -1);
        }
        s.actuo = true;
        return true;
    }
    if (accion.type == BattleActionType::UseSkill && accion.skillId == kAccionCurar) {
        const std::size_t quien = accion.targetIndex >= 0 ? static_cast<std::size_t>(accion.targetIndex) : 0;
        if (!a->cura() || quien >= c.aliados.size() || !vivo(c, {Lado::Aliado, quien}) || !a->gastaCura()) {
            c.log.push_back(a->def().nombre + " no puede curar ahora.");
            return true;
        }
        ICombatant& x = *c.aliados[quien].combatant;
        const int antes = x.health();
        x.heal(c.rng.roll_in_range(1, a->def().cura.dado) + a->def().cura.mas);
        c.log.push_back(a->def().nombre + " cura a " + c.aliados[quien].name + " (+" + std::to_string(x.health() - antes) +
                        " PV; le quedan " + std::to_string(a->curasQueQuedan()) + ").");
        s.actuo = true;
        return true;
    }
    return false;  // huir, objetos: no son cosa de un acompañante
}

}  // namespace Game
