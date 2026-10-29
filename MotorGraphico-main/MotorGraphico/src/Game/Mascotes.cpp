// MASC-01 — ver include/Game/Mascotes.h.
#include "Game/Mascotes.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <set>

namespace Game {

namespace {
constexpr int kDx[4] = {1, 0, -1, 0};
constexpr int kDy[4] = {0, 1, 0, -1};

float distancia(const Mascota& m, const GridCoord& c) {
    return std::hypot(static_cast<float>(c.x) + 0.5f - m.x, static_cast<float>(c.y) + 0.5f - m.y);
}

int separacio(const GridCoord& a, const GridCoord& b) { return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y)); }
}  // namespace

std::uint32_t Mascotes::seguent() {
    // xorshift32, com la bandada: barat i determinista.
    std::uint32_t x = m_estat;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    m_estat = x;
    return x;
}

GridCoord Mascotes::cella(const Mascota& m) {
    return GridCoord{static_cast<int>(std::floor(m.x)), static_cast<int>(std::floor(m.y))};
}

int Mascotes::direccioDe(int dx, int dy) {
    if (dx > 0) return 0;
    if (dy > 0) return 1;
    if (dx < 0) return 2;
    return 3;
}

int Mascotes::direccioCap(float dx, float dy) {
    if (std::fabs(dx) >= std::fabs(dy)) return dx >= 0.0f ? 0 : 2;
    return dy >= 0.0f ? 1 : 3;
}

void Mascotes::prepara(const std::vector<std::pair<int, GridCoord>>& llista, const std::vector<EspecieMascota>& especies,
                       Transitable transitable, std::uint32_t llavor) {
    m_especies = especies;
    m_transitable = std::move(transitable);
    m_mascotes.clear();
    m_estat = llavor == 0 ? 1u : llavor;
    if (!m_transitable) return;
    for (const auto& [especie, casa] : llista) {
        if (especie < 0 || especie >= static_cast<int>(m_especies.size()) || !m_transitable(casa.x, casa.y)) continue;
        Mascota m;
        m.especie = especie;
        m.casa = casa;
        m.x = static_cast<float>(casa.x) + 0.5f;
        m.y = static_cast<float>(casa.y) + 0.5f;
        m.direccio = static_cast<int>(seguent() % 4u);
        m.espera = 1.0f + 4.0f * atzar();
        m_mascotes.push_back(m);
    }
}

bool Mascotes::lliure(int x, int y, std::size_t qui, const GridCoord& jugador) const {
    if (!m_transitable(x, y) || (x == jugador.x && y == jugador.y)) return false;
    for (std::size_t i = 0; i < m_mascotes.size(); ++i) {
        if (i == qui) continue;
        const GridCoord c = cella(m_mascotes[i]);
        if (c.x == x && c.y == y) return false;
        const auto& cami = m_mascotes[i].cami;
        if (!cami.empty() && cami.front().x == x && cami.front().y == y) return false;
    }
    return true;
}

std::vector<std::pair<GridCoord, int>> Mascotes::abast(std::size_t qui, const GridCoord& jugador, int passos) const {
    std::vector<std::pair<GridCoord, int>> nodes;
    std::vector<int> fondaria;
    std::set<std::pair<int, int>> vistes;
    const GridCoord inici = cella(m_mascotes[qui]);
    nodes.push_back({inici, -1});
    fondaria.push_back(0);
    vistes.insert({inici.x, inici.y});
    for (std::size_t k = 0; k < nodes.size(); ++k) {
        if (fondaria[k] >= passos) continue;
        for (int d = 0; d < 4; ++d) {
            const GridCoord n{nodes[k].first.x + kDx[d], nodes[k].first.y + kDy[d]};
            if (vistes.count({n.x, n.y}) != 0 || !lliure(n.x, n.y, qui, jugador)) continue;
            vistes.insert({n.x, n.y});
            nodes.push_back({n, static_cast<int>(k)});
            fondaria.push_back(fondaria[k] + 1);
        }
    }
    return nodes;
}

std::vector<GridCoord> Mascotes::camiFins(const std::vector<std::pair<GridCoord, int>>& nodes, int desti) {
    std::vector<GridCoord> cami;
    for (int k = desti; k > 0; k = nodes[static_cast<std::size_t>(k)].second) cami.push_back(nodes[static_cast<std::size_t>(k)].first);
    std::reverse(cami.begin(), cami.end());
    return cami;
}

void Mascotes::reposa(Mascota& m, float durada) {
    m.estat = EstatMascota::Repos;
    m.rellotge = 0.0f;
    m.espera = durada;
    m.cami.clear();
    m.variant = atzar() < 0.35f ? 1 : 0;
}

void Mascotes::passeja(std::size_t qui, const GridCoord& jugador) {
    Mascota& m = m_mascotes[qui];
    const int radi = m_especies[static_cast<std::size_t>(m.especie)].radi;
    const auto nodes = abast(qui, jugador, radi * 2 + 2);
    // Lluny de casa (despres de saludar), primer les cel·les mes properes a casa.
    const GridCoord on = cella(m);
    const bool lluny = separacio(on, m.casa) > radi;
    std::vector<int> candidates;
    int millor = 1 << 30;
    for (std::size_t k = 1; k < nodes.size(); ++k) {
        const int sep = separacio(nodes[k].first, m.casa);
        if (lluny) {
            if (sep < millor) {
                millor = sep;
                candidates.clear();
            }
            if (sep == millor) candidates.push_back(static_cast<int>(k));
        } else if (sep <= radi && separacio(nodes[k].first, on) >= 2) {
            candidates.push_back(static_cast<int>(k));
        }
    }
    if (candidates.empty()) {
        reposa(m, 1.5f + 2.0f * atzar());
        return;
    }
    const int desti = candidates[seguent() % candidates.size()];
    m.cami = camiFins(nodes, desti);
    m.estat = EstatMascota::Camina;
    m.variant = 0;
    m.rellotge = 0.0f;
}

void Mascotes::saluda(std::size_t qui, const GridCoord& jugador) {
    Mascota& m = m_mascotes[qui];
    const GridCoord on = cella(m);
    if (std::abs(on.x - jugador.x) + std::abs(on.y - jugador.y) == 1) {
        m.estat = EstatMascota::Saluda;
        m.rellotge = 0.0f;
        m.espera = kSalutacio;
        m.cami.clear();
        m.direccio = direccioDe(jugador.x - on.x, jugador.y - on.y);
        return;
    }
    const auto nodes = abast(qui, jugador, 8);
    for (std::size_t k = 1; k < nodes.size(); ++k) {
        const GridCoord& c = nodes[k].first;
        if (std::abs(c.x - jugador.x) + std::abs(c.y - jugador.y) != 1) continue;
        m.cami = camiFins(nodes, static_cast<int>(k));  // la mes propera: la cerca va per capes
        m.estat = EstatMascota::Camina;
        m.variant = 1;
        m.rellotge = 0.0f;
        return;
    }
    m.calma = 6.0f;  // no hi pot arribar: ja ho tornara a provar
}

void Mascotes::fuig(std::size_t qui, const GridCoord& jugador) {
    Mascota& m = m_mascotes[qui];
    const int radi = m_especies[static_cast<std::size_t>(m.especie)].radi;
    const auto nodes = abast(qui, jugador, 6);
    const auto lluny = [&](const GridCoord& c) {
        return std::hypot(static_cast<float>(c.x - jugador.x), static_cast<float>(c.y - jugador.y));
    };
    int desti = -1;
    float millor = lluny(cella(m)) + 1.5f;
    for (std::size_t k = 1; k < nodes.size(); ++k) {
        const float d = lluny(nodes[k].first);
        if (d > millor && separacio(nodes[k].first, m.casa) <= radi + 2) {
            millor = d;
            desti = static_cast<int>(k);
        }
    }
    m.calma = 8.0f;
    if (desti < 0) return;  // arraconada: es queda quieta
    m.cami = camiFins(nodes, desti);
    m.estat = EstatMascota::Fuig;
    m.rellotge = 0.0f;
}

void Mascotes::camina(std::size_t qui, float dt, const GridCoord& jugador) {
    Mascota& m = m_mascotes[qui];
    const float velocitat = m_especies[static_cast<std::size_t>(m.especie)].velocitat * (m.estat == EstatMascota::Fuig ? 1.7f : 1.0f);
    float queda = velocitat * dt;
    while (queda > 0.0f && !m.cami.empty()) {
        const GridCoord seg = m.cami.front();
        const GridCoord on = cella(m);
        if ((seg.x != on.x || seg.y != on.y) && !lliure(seg.x, seg.y, qui, jugador)) {
            reposa(m, 0.8f + atzar());  // algu s'hi ha posat al davant
            return;
        }
        const float tx = static_cast<float>(seg.x) + 0.5f, ty = static_cast<float>(seg.y) + 0.5f;
        const float dx = tx - m.x, dy = ty - m.y;
        const float falta = std::fabs(dx) + std::fabs(dy);  // sempre en eix
        if (falta > 1e-4f) m.direccio = direccioCap(dx, dy);
        const float pas = std::min(queda, falta);
        if (falta > 1e-4f) {
            m.x += dx / falta * pas;
            m.y += dy / falta * pas;
        }
        m.recorregut += pas;
        queda -= pas;
        if (pas >= falta - 1e-4f) {
            m.x = tx;
            m.y = ty;
            m.cami.erase(m.cami.begin());
        }
    }
    if (!m.cami.empty()) return;
    if (m.estat == EstatMascota::Camina && m.variant == 1) {
        const GridCoord on = cella(m);
        m.estat = EstatMascota::Saluda;
        m.rellotge = 0.0f;
        m.espera = kSalutacio;
        m.direccio = direccioCap(static_cast<float>(jugador.x - on.x), static_cast<float>(jugador.y - on.y));
        return;
    }
    reposa(m, m.estat == EstatMascota::Fuig ? 2.0f + 2.0f * atzar() : 2.0f + 5.0f * atzar());
}

void Mascotes::avanza(float dt, const GridCoord& jugador) {
    if (m_mascotes.empty() || dt <= 0.0f) return;
    for (std::size_t i = 0; i < m_mascotes.size(); ++i) {
        Mascota& m = m_mascotes[i];
        const EspecieMascota& e = m_especies[static_cast<std::size_t>(m.especie)];
        m.rellotge += dt;
        m.calma = std::max(0.0f, m.calma - dt);
        const float d = distancia(m, jugador);
        const bool interessa = m.calma <= 0.0f && (e.afectuosa ? d < kDistanciaInteres : d < kDistanciaFuga);
        switch (m.estat) {
            case EstatMascota::Camina:
            case EstatMascota::Fuig:
                if (m.estat == EstatMascota::Camina && m.variant == 0 && interessa) {
                    if (e.afectuosa) saluda(i, jugador);
                    else fuig(i, jugador);
                    if (m.estat != EstatMascota::Camina || m.variant != 0) break;
                }
                camina(i, dt, jugador);
                break;
            case EstatMascota::Repos:
                if (interessa) {
                    if (e.afectuosa) saluda(i, jugador);
                    else fuig(i, jugador);
                } else if (m.rellotge >= m.espera) {
                    if (e.dorm && separacio(cella(m), m.casa) <= 1 && atzar() < 0.3f) {
                        m.estat = EstatMascota::Dorm;
                        m.rellotge = 0.0f;
                        m.espera = 8.0f + 8.0f * atzar();
                        m.direccio = (m.direccio == 0 || m.direccio == 3) ? 0 : 1;  // dorm de cara
                    } else {
                        passeja(i, jugador);
                    }
                }
                break;
            case EstatMascota::Dorm:
                if (d < 1.6f) {
                    m.calma = 0.0f;
                    if (e.afectuosa) saluda(i, jugador);
                    else fuig(i, jugador);
                } else if (m.rellotge >= m.espera) {
                    reposa(m, 1.0f + 1.5f * atzar());
                }
                break;
            case EstatMascota::Saluda: {
                const GridCoord on = cella(m);
                if (on.x != jugador.x || on.y != jugador.y)
                    m.direccio = direccioCap(static_cast<float>(jugador.x - on.x), static_cast<float>(jugador.y - on.y));
                if (m.rellotge >= m.espera || d > kDistanciaInteres + 1.5f) {
                    m.calma = kCalmaDespres;
                    reposa(m, 1.0f + 2.0f * atzar());
                }
                break;
            }
        }
    }
}

}  // namespace Game
