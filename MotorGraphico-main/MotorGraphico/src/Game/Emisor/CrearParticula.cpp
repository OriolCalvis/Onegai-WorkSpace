// U4-07.E04 — Nacimiento de una particula (ver Particula.h para el orden de los 9 sorteos).
#include "Game/Emisor/Particula.h"

#include <algorithm>
#include <cmath>

namespace Game::Emisor {

namespace {
constexpr double kPi = 3.14159265358979323846;
double lerp(double a, double b, double t) { return a + (b - a) * t; }
}  // namespace

std::uint64_t mezclarSemilla(std::uint64_t semilla) {
    std::uint64_t z = semilla + 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

double uniforme01(RPG::RandomEngine& r) { return static_cast<double>(r.roll_in_range(0, 999999)) / 1000000.0; }

std::unique_ptr<RPG::Xoroshiro128p> crearAzarEmisor(std::uint64_t semilla) {
    auto azar = std::make_unique<RPG::Xoroshiro128p>(mezclarSemilla(semilla));
    for (int i = 0; i < kSorteosDescartados; ++i) uniforme01(*azar);
    return azar;
}

Particula crearParticula(const DefinicionEmisor& d, const TipoDeParticula& tipo, RPG::RandomEngine& r) {
    // Los 9 sorteos, en orden y todos antes de usar ninguno: no hay rama que se salte uno.
    const double u1 = uniforme01(r), u2 = uniforme01(r), u3 = uniforme01(r);
    const double u4 = uniforme01(r), u5 = uniforme01(r), u6 = uniforme01(r);
    const double u7 = uniforme01(r), u8 = uniforme01(r), u9 = uniforme01(r);

    Particula p;
    p.vida = static_cast<float>(lerp(d.vidaMin, d.vidaMax, u1));
    const double rapidez = lerp(d.velocidadMin, d.velocidadMax, u2);
    const double theta = (d.anguloCentro + (u3 - 0.5) * d.abanico) * kPi / 180.0;
    p.vx0 = static_cast<float>(rapidez * std::cos(theta));
    p.vy0 = static_cast<float>(rapidez * std::sin(theta));
    if (!tipo.colores.empty()) {
        const auto n = tipo.colores.size();
        p.variante = static_cast<std::uint16_t>(std::min(n - 1, static_cast<std::size_t>(u4 * static_cast<double>(n))));
    }
    p.otraForma = u5 < static_cast<double>(tipo.conOtraForma) ? 1 : 0;
    p.ang0 = static_cast<float>(u6 * 2.0 * kPi);
    p.radio0 = static_cast<float>(6.0 + 4.0 * u7);
    p.oscura = u8 < static_cast<double>(d.contraste) ? 1 : 0;
    p.lado = static_cast<float>(u9 * 2.0 - 1.0);
    return p;
}

}  // namespace Game::Emisor
