// AV13-06 — Ver include/Game/TableroCombate.h.
#include "Game/TableroCombate.h"

#include "Game/LineaDeVision.h"

#include <algorithm>
#include <cstdlib>

namespace Game {

namespace {

bool igual(GridCoord a, GridCoord b) { return a.x == b.x && a.y == b.y; }

// Coloca un tramo de `lado` casillas centrado en `centro` dentro de [0, total).
std::pair<int, int> tramo(int centro, int lado, int total) {
    if (total <= lado) return {0, std::max(0, total)};
    int inicio = centro - lado / 2;
    inicio = std::clamp(inicio, 0, total - lado);
    return {inicio, lado};
}

}  // namespace

TableroCombate TableroCombate::alrededor(const std::vector<GridCoord>& quienes, int anchoNivel, int altoNivel,
                                         Bloquea bloquea, int lado) {
    TableroCombate t;
    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (std::size_t i = 0; i < quienes.size(); ++i) {
        const GridCoord& c = quienes[i];
        if (i == 0 || c.x < minX) minX = c.x;
        if (i == 0 || c.x > maxX) maxX = c.x;
        if (i == 0 || c.y < minY) minY = c.y;
        if (i == 0 || c.y > maxY) maxY = c.y;
    }
    const auto [x0, w] = tramo((minX + maxX + 1) / 2, std::max(1, lado), anchoNivel);
    const auto [y0, h] = tramo((minY + maxY + 1) / 2, std::max(1, lado), altoNivel);
    t.m_origen = {x0, y0};
    t.m_ancho = w;
    t.m_alto = h;
    t.m_bloquea = std::move(bloquea);
    return t;
}

bool TableroCombate::dentro(GridCoord c) const {
    return c.x >= m_origen.x && c.y >= m_origen.y && c.x < m_origen.x + m_ancho && c.y < m_origen.y + m_alto;
}

bool TableroCombate::transitable(GridCoord c) const { return dentro(c) && !(m_bloquea && m_bloquea(c.x, c.y)); }

bool TableroCombate::libre(GridCoord c) const { return transitable(c) && quienEsta(c) == nullptr; }

bool TableroCombate::pon(const std::string& quien, GridCoord c) {
    if (!libre(c)) return false;
    quita(quien);
    m_ocupantes.emplace_back(quien, c);
    return true;
}

void TableroCombate::quita(const std::string& quien) {
    m_ocupantes.erase(std::remove_if(m_ocupantes.begin(), m_ocupantes.end(),
                                     [&](const auto& o) { return o.first == quien; }),
                      m_ocupantes.end());
}

bool TableroCombate::mueve(const std::string& quien, GridCoord a) {
    if (!dondeEsta(quien) || !libre(a)) return false;
    for (auto& o : m_ocupantes)
        if (o.first == quien) o.second = a;
    return true;
}

std::optional<GridCoord> TableroCombate::dondeEsta(const std::string& quien) const {
    for (const auto& o : m_ocupantes)
        if (o.first == quien) return o.second;
    return std::nullopt;
}

const std::string* TableroCombate::quienEsta(GridCoord c) const {
    for (const auto& o : m_ocupantes)
        if (igual(o.second, c)) return &o.first;
    return nullptr;
}

int TableroCombate::distancia(GridCoord a, GridCoord b) { return distanciaChebyshev(a, b); }

bool TableroCombate::seVen(GridCoord a, GridCoord b) const {
    return lineaDeVision(a, b, m_bloquea);  // Game/LineaDeVision.h: la misma cuenta que los disparadores de cinematica
}

std::optional<GridCoord> TableroCombate::libreMasCercana(GridCoord desde) const {
    const int radioMax = std::max(m_ancho, m_alto);
    for (int r = 0; r <= radioMax; ++r) {
        for (int y = desde.y - r; y <= desde.y + r; ++y) {
            for (int x = desde.x - r; x <= desde.x + r; ++x) {
                if (std::max(std::abs(x - desde.x), std::abs(y - desde.y)) != r) continue;  // solo el anillo
                if (libre({x, y})) return GridCoord{x, y};
            }
        }
    }
    return std::nullopt;
}

std::vector<GridCoord> TableroCombate::borde() const {
    std::vector<GridCoord> out;
    for (int y = m_origen.y; y < m_origen.y + m_alto; ++y) {
        for (int x = m_origen.x; x < m_origen.x + m_ancho; ++x) {
            const bool enBorde = x == m_origen.x || y == m_origen.y || x == m_origen.x + m_ancho - 1 ||
                                 y == m_origen.y + m_alto - 1;
            if (enBorde && transitable({x, y})) out.push_back({x, y});
        }
    }
    return out;
}

// ------------------------------------------------------------ AV13-12, moverse

int TableroCombate::pasosMax(const std::string& quien) const {
    int base = kPasos;
    for (const auto& b : m_bonos)
        if (b.first == quien) base = kPasos + b.second;
    const int freno = m_velocidad ? m_velocidad(quien) : 0;
    if (freno >= 2) return 0;
    return freno == 1 ? base / 2 : base;
}

void TableroCombate::ponBono(const std::string& quien, int bono) {
    for (auto& b : m_bonos)
        if (b.first == quien) {
            b.second = std::max(0, bono);
            return;
        }
    m_bonos.emplace_back(quien, std::max(0, bono));
}

int TableroCombate::pasos(const std::string& quien) const {
    for (const auto& g : m_gastados)
        if (g.first == quien) return std::max(0, pasosMax(quien) - g.second);
    return pasosMax(quien);
}

void TableroCombate::gasta(const std::string& quien, int n) {
    for (auto& g : m_gastados)
        if (g.first == quien) {
            g.second += n;
            return;
        }
    m_gastados.emplace_back(quien, n);
}

int TableroCombate::costeDePaso(GridCoord de, GridCoord a) const {
    return m_nivel && m_nivel(a.x, a.y) > m_nivel(de.x, de.y) ? 2 : 1;
}

RutaCasillas TableroCombate::rutasDe(const std::string& quien) const {
    RutaCasillas r(m_origen, m_ancho, m_alto, [this, quien](int x, int y) {
        const std::string* hay = quienEsta({x, y});
        return transitable({x, y}) && (hay == nullptr || *hay == quien);
    });
    if (m_nivel) r.ponExtra([this](GridCoord de, GridCoord a) { return costeDePaso(de, a) - 1; });
    return r;
}

std::vector<RutaCasillas::Alcanzable> TableroCombate::alcanzables(const std::string& quien) const {
    const auto donde = dondeEsta(quien);
    if (!donde) return {};
    return rutasDe(quien).alcanzables(*donde, pasos(quien));
}

std::vector<GridCoord> TableroCombate::rutaHasta(const std::string& quien, GridCoord destino) const {
    const auto donde = dondeEsta(quien);
    if (!donde || !libre(destino)) return {};
    const RutaCasillas rutas = rutasDe(quien);
    std::vector<GridCoord> r = rutas.ruta(*donde, destino);
    if (rutas.coste(*donde, destino) > pasos(quien)) return {};  // lo que cuesta, no las casillas
    return r;
}

std::vector<GridCoord> TableroCombate::anda(const std::string& quien, GridCoord destino) {
    const auto donde = dondeEsta(quien);
    std::vector<GridCoord> r = rutaHasta(quien, destino);
    if (!donde || r.empty() || !mueve(quien, destino)) return {};
    GridCoord antes = *donde;
    int coste = 0;
    for (const GridCoord& c : r) {
        coste += costeDePaso(antes, c);
        antes = c;
    }
    gasta(quien, coste);
    return r;
}

bool TableroCombate::aTiro(const std::string& quien, const std::string& objetivo, int alcance) const {
    const auto a = dondeEsta(quien), b = dondeEsta(objetivo);
    if (!a || !b) return false;
    const int d = distancia(*a, *b);
    if (d > std::max(1, alcance)) return false;
    return d <= 1 || seVen(*a, *b);
}

bool TableroCombate::acerca(const std::string& quien, const std::vector<std::string>& objetivos, int alcance,
                            std::vector<GridCoord>* ruta) {
    for (const std::string& o : objetivos)
        if (aTiro(quien, o, alcance)) return true;
    const auto desde = dondeEsta(quien);
    if (!desde) return false;
    // La mejor casilla alcanzable: primero, desde la que llega (la que menos
    // cuesta); si ninguna, la que queda mas cerca de alguno.
    std::optional<GridCoord> mejor;
    bool mejorATiro = false;
    int mejorDist = 1 << 20, mejorCoste = 1 << 20;
    for (const RutaCasillas::Alcanzable& a : alcanzables(quien)) {
        int dist = 1 << 20;
        bool llega = false;
        for (const std::string& o : objetivos) {
            const auto p = dondeEsta(o);
            if (!p) continue;
            const int d = distancia(a.casilla, *p);
            dist = std::min(dist, d);
            if (d <= std::max(1, alcance) && (d <= 1 || seVen(a.casilla, *p))) llega = true;
        }
        const bool gana = llega != mejorATiro ? llega
                          : llega        ? a.coste < mejorCoste
                                         : (dist < mejorDist || (dist == mejorDist && a.coste < mejorCoste));
        if (!mejor || gana) {
            mejor = a.casilla;
            mejorATiro = llega;
            mejorDist = dist;
            mejorCoste = a.coste;
        }
    }
    if (!mejor) return false;
    // Si no llega y no se acerca nada, se queda donde esta.
    int ahora = 1 << 20;
    for (const std::string& o : objetivos)
        if (const auto p = dondeEsta(o)) ahora = std::min(ahora, distancia(*desde, *p));
    if (!mejorATiro && mejorDist >= ahora) return false;
    std::vector<GridCoord> andada = anda(quien, *mejor);
    if (ruta != nullptr) *ruta = andada;
    return mejorATiro && !andada.empty();
}

}  // namespace Game
