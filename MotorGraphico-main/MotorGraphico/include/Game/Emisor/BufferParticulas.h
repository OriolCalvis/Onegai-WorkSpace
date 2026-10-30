// U4-07.E05 — Buffer de aforo fijo de las particulas de UN emisor. GL-free.
//
// Toda la memoria se reserva en el constructor: despues, `datos().capacity()` no cambia (ni `anade` ni
// `compacta` reservan). Una particula esta viva mientras edad < vida. La compactacion es ESTABLE: el orden de
// los supervivientes es el de nacimiento (D-T7).
#pragma once

#include <cstddef>
#include <vector>

#include "Game/Emisor/Particula.h"

namespace Game::Emisor {

class BufferParticulas {
public:
    explicit BufferParticulas(std::size_t aforo);
    std::size_t vivas() const { return m_datos.size(); }
    std::size_t aforo() const { return m_aforo; }
    bool llena() const { return m_datos.size() >= m_aforo; }
    void anade(const Particula& p);                 // precondicion: !llena(); si no, se ignora
    std::vector<Particula>& datos() { return m_datos; }   // mutable: SOLO avanzarEmisor integra aqui
    const std::vector<Particula>& datos() const { return m_datos; }
    std::size_t compacta();                         // retira las de edad>=vida, orden estable; cuantas
    std::size_t vacia();                            // retira todas; cuantas

private:
    std::size_t m_aforo = 0;
    std::vector<Particula> m_datos;
};

}  // namespace Game::Emisor
