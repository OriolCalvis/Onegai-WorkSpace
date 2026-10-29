// H2-02 — A quien apunta la carta senalada de la barra de combate (ver
// include/Game/BarraCombate.h).
//
// JUGABILIDAD_FASE_H.md, H2-02: «En combate, elegir un hechizo de area -> se
// marcan los afectados -> cancelar no gasta PM». La regla de a quien alcanza
// cada objetivo es la de la mesa (CartasEnCombate::alcance), la misma con la
// que se juega la carta: aqui solo se elige al senalado y se dice a quien se
// marca. Nada se gasta hasta jugarla.
#include "Game/BarraCombate.h"

#include <algorithm>

namespace Game {

namespace {

// AV13-13 — A quien se puede apuntar: con tablero, los que estan a tiro de la
// carta (Game/AlcanceCartas.h); sin el, todos los que siguen en pie.
const std::vector<bool>& elegibles(const VistaBarra& v, const CartaBarra& c, bool aliado) {
    const std::vector<bool>& aTiro = aliado ? c.aliadosATiro : c.enemigosATiro;
    if (!aTiro.empty()) return aTiro;
    return aliado ? v.aliadosVivos : v.enemigosVivos;
}

}  // namespace

bool BarraCombate::eligeObjetivo(const CartaBarra& c, bool& aliado) {
    if (c.id == "#pocion" || c.id == "#huir") return false;
    switch (c.objetivo) {
        case RPG::SkillTarget::SINGLE_ENEMY:
        case RPG::SkillTarget::ZONE_RADIUS:
            aliado = false;
            return true;
        case RPG::SkillTarget::ALL_ENEMIES:  // AV13-14: con tablero, un area que no sale de ti se apunta a un enemigo
            aliado = false;
            return c.hayArea && !c.area.enTi && c.alcance >= 0;  // con tablero (sin el, a todos)
        case RPG::SkillTarget::SINGLE_ALLY:
            aliado = true;
            return true;
        default: return false;
    }
}

int BarraCombate::objetivoDe(const VistaBarra& v, const CartaBarra& c) const {
    bool aliado = false;
    if (!eligeObjetivo(c, aliado)) return -1;
    const std::vector<bool>& vivos = elegibles(v, c, aliado);
    const int elegido = aliado ? m_elegidos.aliado : m_elegidos.enemigo;
    if (elegido < 0 || static_cast<std::size_t>(elegido) >= vivos.size() || !vivos[static_cast<std::size_t>(elegido)]) {
        // AV13-13: con tablero, sin elegido (o fuera de alcance), el primero a tiro.
        if ((aliado ? c.aliadosATiro : c.enemigosATiro).empty()) return -1;
        const auto it = std::find(vivos.begin(), vivos.end(), true);
        return it == vivos.end() ? -1 : static_cast<int>(it - vivos.begin());
    }
    return elegido;
}

std::vector<CartasEnCombate::Alcanzado> BarraCombate::afectados(const VistaBarra& v) const {
    const CartaBarra* c = carta(v, m_foco);
    if (c == nullptr || m_verCreador || m_emergente.abierto || rivalPendiente() || c->id == "#huir") return {};
    if (c->hayArea && (!c->aliadosEnArea.empty() || !c->enemigosEnArea.empty())) {  // AV13-14: los de dentro
        std::vector<CartasEnCombate::Alcanzado> dentro;
        for (std::size_t i = 0; i < c->aliadosEnArea.size(); ++i) if (c->aliadosEnArea[i]) dentro.push_back({true, i});
        for (std::size_t i = 0; i < c->enemigosEnArea.size(); ++i) if (c->enemigosEnArea[i]) dentro.push_back({false, i});
        return dentro;
    }
    bool aliado = false;
    if (eligeObjetivo(*c, aliado)) {  // AV13-13: el de una sola, entre los que estan a tiro
        const std::vector<bool>& el = elegibles(v, *c, aliado);
        return CartasEnCombate::alcance(c->objetivo, objetivoDe(v, *c), aliado ? el : v.aliadosVivos,
                                        aliado ? v.enemigosVivos : el);
    }
    return CartasEnCombate::alcance(c->objetivo, objetivoDe(v, *c), v.aliadosVivos, v.enemigosVivos);
}

void BarraCombate::mueveObjetivo(const VistaBarra& v, int paso) {
    const CartaBarra* c = carta(v, m_foco);
    bool aliado = false;
    if (c == nullptr || m_emergente.abierto || !eligeObjetivo(*c, aliado)) return;
    const std::vector<bool>& vivos = elegibles(v, *c, aliado);
    const int n = static_cast<int>(vivos.size());
    if (n == 0) return;
    // Desde el marcado ahora (el elegido, o el primero en pie si no hay).
    int actual = objetivoDe(v, *c);
    if (actual < 0) {
        const auto a = afectados(v);
        actual = a.empty() ? 0 : static_cast<int>(a.front().i);
    }
    for (int k = 1; k <= n; ++k) {
        const int i = ((actual + paso * k) % n + n) % n;
        if (vivos[static_cast<std::size_t>(i)]) {
            (aliado ? m_elegidos.aliado : m_elegidos.enemigo) = i;
            return;
        }
    }
}

void BarraCombate::senalaObjetivo(const VistaBarra& v, int indice) {
    const CartaBarra* c = carta(v, m_foco);
    bool aliado = false;
    if (c == nullptr || indice < 0 || !eligeObjetivo(*c, aliado)) return;
    if (aliado != (indice < kObjetivoEnemigo)) return;  // la carta no apunta a ese bando
    const int i = aliado ? indice : indice - kObjetivoEnemigo;
    const std::vector<bool>& vivos = elegibles(v, *c, aliado);
    if (static_cast<std::size_t>(i) >= vivos.size() || !vivos[static_cast<std::size_t>(i)]) return;
    (aliado ? m_elegidos.aliado : m_elegidos.enemigo) = i;
}

}  // namespace Game
