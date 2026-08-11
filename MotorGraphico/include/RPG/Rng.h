#pragma once

#include <cstdint>

// Fuente de azar INYECTADA (GAMEMACHINE_NECESIDADES.md P0-5: "no usa
// aleatoriedad global implicita").
//
// Por que una interfaz y no std::rand() ni un std::mt19937 global:
//  - Un combate con azar global no se puede testear. Con IRng inyectado,
//    demo_dice_engine fija la semilla y comprueba resultados EXACTOS.
//  - Un bug de combate reportado por un jugador se puede reproducir si se
//    guarda la semilla de la partida.
//  - El azar del loot y el del combate pueden ser flujos distintos si
//    algun dia hace falta (que abrir el inventario no cambie la tirada
//    siguiente).
//
// Es una interfaz "I" con un solo metodo a proposito: cuanto mas pequeño
// el contrato, mas facil implementarlo en un test (ver SequenceRng).
class IRng {
public:
    virtual ~IRng() = default;

    // Entero uniforme en [minInclusive, maxInclusive].
    virtual int nextInt(int minInclusive, int maxInclusive) = 0;
};

// Generador determinista por semilla. xorshift64* en vez de <random>:
// std::mt19937 + uniform_int_distribution NO garantiza la misma secuencia
// entre implementaciones de la biblioteca estandar, asi que un test que
// fija la semilla podria pasar en macOS y fallar en Linux. Esto son 6
// lineas y da la MISMA secuencia en todas partes, que es justo lo que un
// test de combate necesita.
class SeededRng : public IRng {
public:
    explicit SeededRng(std::uint64_t seed = 0x9E3779B97F4A7C15ULL)
        : m_state(seed != 0 ? seed : 0x9E3779B97F4A7C15ULL) {}

    int nextInt(int minInclusive, int maxInclusive) override;

    std::uint64_t seedState() const { return m_state; }

private:
    std::uint64_t nextRaw();
    std::uint64_t m_state;
};
