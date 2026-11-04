// U4-07.E05 — ver include/Game/Emisor/BufferParticulas.h.
#include "Game/Emisor/BufferParticulas.h"

#include <algorithm>

namespace Game::Emisor {

BufferParticulas::BufferParticulas(std::size_t aforo) : m_aforo(aforo) { m_datos.reserve(aforo); }

void BufferParticulas::anade(const Particula& p) {
    if (llena()) return;   // nunca crece por encima del aforo reservado
    m_datos.push_back(p);
}

std::size_t BufferParticulas::compacta() {
    const std::size_t antes = m_datos.size();
    m_datos.erase(std::remove_if(m_datos.begin(), m_datos.end(), [](const Particula& p) { return p.edad >= p.vida; }),
                  m_datos.end());
    return antes - m_datos.size();
}

std::size_t BufferParticulas::vacia() {
    const std::size_t antes = m_datos.size();
    m_datos.clear();
    return antes;
}

}  // namespace Game::Emisor
