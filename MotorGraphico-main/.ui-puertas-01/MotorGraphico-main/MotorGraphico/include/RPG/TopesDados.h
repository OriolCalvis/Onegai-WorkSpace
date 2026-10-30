// Los topes de dados que da el equipo en cada tier.
//
// Oriol (05/10): «hay que respetar los caps de dados máximos que cada tier
// puede ofrecer a la espec más alta. Y nunca jamás un jugador puede tener las
// tres a lo más alto [...]: con el equipamiento al máximo, una al tope, otra
// superando 3/4 de la más alta y las dos últimas en 2/4 del más alto, en el
// mejor equipo de Tier 1. Pero que sea extremadamente difícil de conseguir».
//
// La regla, con T = topeDadosEquipo del tier (assets/rules/tier_rules.json),
// sobre lo que suma el EQUIPO (la raza va aparte y suma encima):
//
//   el atributo con más bono del equipo   como mucho T
//   el segundo                            como mucho 3/4 de T más uno (sin pasar de T)
//   el tercero y el cuarto                como mucho la mitad de T
//
// Con T = 4 (Tier 1) el mejor equipo posible da 4, 4, 2 y 2: dos atributos
// pueden llegar arriba, tres nunca. El kit del cofre (+2, +2, +1, +1) es el
// mismo reparto con T = 2.
//
// aplica() recorta un reparto que se pase: el orden lo da el bono (de mayor a
// menor) y, a igual bono, el orden CON, DES, INT, CAR. Los bonos negativos
// (penalizadores) no se tocan. Con T <= 0 no hay tope.
//
// GL-free, sin catálogos y solo cabecera: CharacterSheet::stat() (en línea)
// lo usa con el tope que recompute_derived() lee de TierRules, y así ningún
// binario que lea una ficha tiene que enlazar nada nuevo.
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

namespace RPG {
namespace TopesDados {

struct Reparto {
    int primero = 0;  // el atributo más alto
    int segundo = 0;
    int resto = 0;    // el tercero y el cuarto
};

// El mejor reparto que permite el tope (lo que da el mejor equipo del tier).
inline Reparto maximo(int tope) {
    Reparto r;
    if (tope <= 0) return r;
    r.primero = tope;
    r.segundo = std::min(tope, tope * 3 / 4 + 1);
    r.resto = tope / 2;
    return r;
}

// Los bonos del equipo con el tope aplicado.
inline std::array<int, 4> aplica(const std::array<int, 4>& bonos, int tope) {
    if (tope <= 0) return bonos;
    std::array<std::size_t, 4> orden{0, 1, 2, 3};
    std::stable_sort(orden.begin(), orden.end(), [&](std::size_t a, std::size_t b) { return bonos[a] > bonos[b]; });
    const Reparto r = maximo(tope);
    const int techo[4] = {r.primero, r.segundo, r.resto, r.resto};
    std::array<int, 4> salida = bonos;
    for (std::size_t puesto = 0; puesto < 4; ++puesto) {
        const std::size_t i = orden[puesto];
        salida[i] = std::min(bonos[i], techo[puesto]);
    }
    return salida;
}

// ¿El reparto cabe en el tope sin recortar nada?
inline bool respeta(const std::array<int, 4>& bonos, int tope) { return aplica(bonos, tope) == bonos; }

}  // namespace TopesDados
}  // namespace RPG
