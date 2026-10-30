// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-21  (fase C, 1 j) - Un sol generador d'atzar
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstdint>

namespace RPG {

// Generador de números pseudo-aleatorios GL-free, inyectable. Se usa en TODAS
// las tiradas del juego. Implementaciones recomendadas: Xoroshiro128+ (fast,
// buena calidad para juegos; permite reproducir partidas guardando seed).
//
// IMPORTANTE: se inyecta, NO es un singleton global. Así los tests y la
// demo_battle pueden pasar una RandomEngine reproducible (semilla fija) y
// comparar salidas byte-a-byte sin flakiness.
//
// ===== Una sola interfaz de azar (C-21) =====
// Historicamente habia dos: ::IRng/SeededRng (sin ninguna referencia en el
// codigo y sin definicion — Rng.cpp nunca existio, no habria enlazado) y
// RPG::RandomEngine. C-21 retira la muerta: RandomEngine es la unica
// interfaz de azar del motor. El codigo nuevo no anade ninguna otra.
class RandomEngine {
public:
    virtual ~RandomEngine() = default;

    virtual int      roll_d6() = 0;                 // 1..6 uniforme
    virtual int      roll_in_range(int a, int b) = 0; // [a, b] uniforme
    virtual uint64_t seed() const = 0;
};

} // namespace RPG

namespace RPG {

// Implementación Xoroshiro128+ simple, sin dependencias externas. Header-only
// para poder usarla en tests y demos sin añadir unidades de compilación.
class Xoroshiro128p : public RandomEngine {
public:
    explicit Xoroshiro128p(uint64_t s = 0x9E3779B97F4A7C15ULL) : m_seed(s != 0 ? s : 0x9E3779B97F4A7C15ULL) {
        s0 = m_seed;
        s1 = 0x853C49E6748FEA9BULL ^ m_seed;
        if (s0 == 0 && s1 == 0) s0 = 1ULL;
    }

    int roll_d6() override {
        return static_cast<int>(next_double() * 6.0) + 1;
    }

    int roll_in_range(int a, int b) override {
        if (b < a) { int t = a; a = b; b = t; }
        uint64_t range = static_cast<uint64_t>(b - a + 1);
        return a + static_cast<int>(next_uint64() % range);
    }

    uint64_t seed() const override { return m_seed; }

private:
    uint64_t s0, s1;
    const uint64_t m_seed;

    static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    uint64_t next_uint64() {
        const uint64_t r = s0 + s1;
        s1 ^= s0;
        s0 = rotl(s0, 24) ^ s1 ^ (s1 << 16);
        s1 = rotl(s1, 37);
        return r;
    }

    double next_double() {
        return static_cast<double>(next_uint64() >> 11) * (1.0 / 9007199254740992.0);
    }
};

} // namespace RPG
