// AV13-07 — Ver include/Game/RutaCasillas.h.
#include "Game/RutaCasillas.h"

#include <algorithm>
#include <climits>
#include <queue>
#include <utility>

namespace Game {

struct RutaCasillas::Mapa {
    std::vector<int> coste;   // INT_MAX = no se llega
    std::vector<int> viene;   // indice de la casilla anterior (-1 = salida o nada)
};

RutaCasillas::RutaCasillas(GridCoord origen, int ancho, int alto, Pisa pisa, Coste coste)
    : m_origen(origen), m_ancho(std::max(0, ancho)), m_alto(std::max(0, alto)), m_pisa(std::move(pisa)),
      m_coste(std::move(coste)) {}

bool RutaCasillas::dentro(GridCoord c) const {
    return c.x >= m_origen.x && c.y >= m_origen.y && c.x < m_origen.x + m_ancho && c.y < m_origen.y + m_alto;
}

int RutaCasillas::indice(GridCoord c) const { return (c.y - m_origen.y) * m_ancho + (c.x - m_origen.x); }

RutaCasillas::Mapa RutaCasillas::explora(GridCoord desde, int presupuesto) const {
    Mapa m;
    const std::size_t n = static_cast<std::size_t>(m_ancho * m_alto);
    m.coste.assign(n, INT_MAX);
    m.viene.assign(n, -1);
    if (!dentro(desde)) return m;
    // (coste, orden de llegada, indice): el orden de llegada fija el desempate.
    using Nodo = std::pair<std::pair<int, int>, int>;
    std::priority_queue<Nodo, std::vector<Nodo>, std::greater<Nodo>> cola;
    int llegada = 0;
    m.coste[static_cast<std::size_t>(indice(desde))] = 0;
    cola.push({{0, llegada++}, indice(desde)});
    const GridCoord vecinos[4] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    while (!cola.empty()) {
        const auto [clave, i] = cola.top();
        cola.pop();
        if (clave.first != m.coste[static_cast<std::size_t>(i)]) continue;
        const GridCoord aqui{m_origen.x + i % m_ancho, m_origen.y + i / m_ancho};
        for (const GridCoord& v : vecinos) {
            const GridCoord sig{aqui.x + v.x, aqui.y + v.y};
            if (!dentro(sig) || !m_pisa || !m_pisa(sig.x, sig.y)) continue;
            int paso = m_coste ? std::max(1, m_coste(sig.x, sig.y)) : 1;
            if (m_extra) paso += std::max(0, m_extra(aqui, sig));
            const int total = clave.first + paso;
            if (presupuesto >= 0 && total > presupuesto) continue;
            const std::size_t j = static_cast<std::size_t>(indice(sig));
            if (total < m.coste[j]) {
                m.coste[j] = total;
                m.viene[j] = i;
                cola.push({{total, llegada++}, static_cast<int>(j)});
            }
        }
    }
    return m;
}

std::vector<GridCoord> RutaCasillas::ruta(GridCoord desde, GridCoord hasta) const {
    if (!dentro(desde) || !dentro(hasta) || (desde.x == hasta.x && desde.y == hasta.y)) return {};
    const Mapa m = explora(desde, -1);
    int i = indice(hasta);
    if (m.coste[static_cast<std::size_t>(i)] == INT_MAX) return {};
    std::vector<GridCoord> out;
    const int salida = indice(desde);
    while (i != salida && i >= 0) {
        out.push_back({m_origen.x + i % m_ancho, m_origen.y + i / m_ancho});
        i = m.viene[static_cast<std::size_t>(i)];
    }
    std::reverse(out.begin(), out.end());
    return out;
}

std::vector<RutaCasillas::Alcanzable> RutaCasillas::alcanzables(GridCoord desde, int presupuesto) const {
    std::vector<Alcanzable> out;
    if (presupuesto <= 0) return out;
    const Mapa m = explora(desde, presupuesto);
    const int salida = dentro(desde) ? indice(desde) : -1;
    for (int i = 0; i < m_ancho * m_alto; ++i) {
        const int c = m.coste[static_cast<std::size_t>(i)];
        if (i == salida || c == INT_MAX) continue;
        out.push_back({{m_origen.x + i % m_ancho, m_origen.y + i / m_ancho}, c});
    }
    return out;
}

int RutaCasillas::coste(GridCoord desde, GridCoord hasta) const {
    if (!dentro(desde) || !dentro(hasta)) return -1;
    if (desde.x == hasta.x && desde.y == hasta.y) return 0;
    const int c = explora(desde, -1).coste[static_cast<std::size_t>(indice(hasta))];
    return c == INT_MAX ? -1 : c;
}

}  // namespace Game
