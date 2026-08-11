#pragma once

#include <cstdint>
#include "RPG/Rng.h"   // adaptador IRng <-> RandomEngine (unificacion P0)

namespace RPG {

// Generador de números pseudo-aleatorios GL-free, inyectable. Se usa en TODAS
// las tiradas del juego. Implementaciones recomendadas: Xoroshiro128+ (fast,
// buena calidad para juegos; permite reproducir partidas guardando seed).
//
// IMPORTANTE: se inyecta, NO es un singleton global. Así los tests y la
// demo_battle pueden pasar una RandomEngine reproducible (semilla fija) y
// comparar salidas byte-a-byte sin flakiness.
//
// ===== UNIFICACION RandomEngine vs IRng (B6) =====
// Había 2 interfaces sin relación:
//   1) RandomEngine  : roll_d6() + roll_in_range() + seed()  ← RPG / motor (canónico de aquí en adelante)
//   2) ::IRng        : nextInt(a, b)                         ← LEGACY (ResourceManager, tests antiguos)
//
// Proporcionamos 2 adaptadores puente para poder usar cualquiera en cualquier
// sitio, sin romper código existente. Ver adaptadores abajo:
//   RngAsRandomEngine     (::IRng        → RPG::RandomEngine)
//   RandomEngineAsIRng    (RPG::RandomEngine → ::IRng)
//
// No se borra la interfaz ::IRng para compatibilidad; pero el código NUEVO
// debe preferir RPG::RandomEngine (es la que tiene roll_d6, unidad base del
// motor Nd6 Onegai).
class RandomEngine {
public:
    virtual ~RandomEngine() = default;

    virtual int      roll_d6() = 0;                 // 1..6 uniforme
    virtual int      roll_in_range(int a, int b) = 0; // [a, b] uniforme
    virtual uint64_t seed() const = 0;
};

// ============================================================
// Adaptador 1: usar un ::IRng legacy como si fuera RandomEngine.
// roll_d6 = nextInt(1,6)
// seed()  : devuelve 0 si no se puede acceder a la semilla (::IRng no la expone)
// ============================================================
class RngAsRandomEngine : public RandomEngine {
public:
    explicit RngAsRandomEngine(::IRng& r) : m_rng(&r) {}
    int  roll_d6()                           override { return m_rng->nextInt(1, 6); }
    int  roll_in_range(int a, int b)         override { if (b<a) { int t=a;a=b;b=t;} return m_rng->nextInt(a, b); }
    uint64_t seed() const                    override { return 0; }
private:
    ::IRng* m_rng;
};

} // namespace RPG

// ============================================================
// Adaptador 2 (fuera de namespace): usar RPG::RandomEngine como ::IRng legacy.
// Lo ponemos FUERA del namespace RPG para que coincida con el tipo global ::IRng.
// ============================================================
class RandomEngineAsIRng : public IRng {
public:
    explicit RandomEngineAsIRng(RPG::RandomEngine& r) : m_re(&r) {}
    int nextInt(int a, int b) override { return m_re->roll_in_range(a, b); }
private:
    RPG::RandomEngine* m_re;
};

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
