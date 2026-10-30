#pragma once
// DANO-02 (Oriol, 06/10/2026) — Cuanto dano hace una tirada enfrentada.
//
// Golpe fisico (Atacar, las armas, las cartas que no son magia): los dos
// tiran y se restan los exitos. La diferencia, redondeada hacia arriba, son
// los d6 que tira el atacante:
//
//     2 exitos contra 1      -> 1d6
//     1,5 contra 1           -> 1d6 (medio exito cuenta como un dado)
//     empate                 -> roza: 1 de dano, sin dados (D0014)
//     pierde                 -> 0
//     el defensor pifia      -> todos los exitos del atacante en d6 (minimo 1)
//     todos 6, dos dados o + -> +1d6
//     el atacante pifia      -> 0
//
// Las cartas con dados propios (Corte Real 2d6) los suman a los de la
// diferencia, solo si se gana; los bonos de dano del turno (Puno Volcanico,
// Marca Profana) se suman encima.
//
// Magia (la carta dice contra que se salva: defensa_mental o
// resistencia_fisica): el lanzador tira contra la salvacion del objetivo.
//
//     el objetivo falla      -> los dados de la carta enteros, y sus estados
//     el objetivo pifia      -> los dados enteros y sus estados, con el doble de rondas
//                               (Oriol, 14/10, D0015: «te entra el dano, te comes el
//                               efecto duplicado»; antes daba un dado mas)
//     todos 6, dos dados o + -> un dado mas de la carta
//     la supera (o empata)   -> la mitad del dano y NUNCA sus estados, salvo
//                               que la carta diga lo contrario
//     el lanzador pifia      -> nada
#include <algorithm>
#include <cmath>

namespace RPG::dano {

struct Enfrentamiento {
    float atacante = 0.0f;  // exitos (ya con bonos)
    float defensor = 0.0f;
    bool pifiaAtacante = false;
    bool pifiaDefensor = false;
    bool todosSeis = false;  // del atacante, con dos dados o mas (o un critico forzado)
};

inline int redondeaArriba(float exitos) { return exitos <= 0.0f ? 0 : static_cast<int>(std::ceil(exitos - 1e-4f)); }

// D0014 (Oriol, 14/10/2026): «el empate hace 1 de dano (roce)». Antes no hacia
// nada y el 59 % de los ataques se perdia (el heroe de 3 dados empata un 28 %
// contra un saqueador de 1). El roce es 1 fijo: sin los dados de la carta ni
// los bonos del turno. Cuenta tambien el 0 contra 0. Con una pifia no hay roce
// (la del atacante no hace nada; la del defensor ya da todos los exitos).
inline constexpr int kRoce = 1;

inline bool roza(const Enfrentamiento& e) {
    if (e.pifiaAtacante || e.pifiaDefensor) return false;
    return std::fabs(e.atacante - e.defensor) < 1e-4f;
}

// Los d6 de un golpe fisico (0 = no entra; el roce va aparte: roza()).
inline int dadosFisicos(const Enfrentamiento& e) {
    if (e.pifiaAtacante) return 0;
    if (e.pifiaDefensor) return std::max(1, redondeaArriba(e.atacante)) + (e.todosSeis ? 1 : 0);
    const float diferencia = e.atacante - e.defensor;
    if (diferencia <= 0.0f) return 0;
    return redondeaArriba(diferencia) + (e.todosSeis ? 1 : 0);
}

enum class Salvacion { Nada, Supera, Falla, Pifia };

inline Salvacion salvacion(const Enfrentamiento& e) {
    if (e.pifiaAtacante) return Salvacion::Nada;
    if (e.pifiaDefensor) return Salvacion::Pifia;
    return e.atacante > e.defensor ? Salvacion::Falla : Salvacion::Supera;
}

// Dados de mas de la carta de magia: el critico del lanzador (todos 6). La pifia del
// objetivo ya no da un dado: duplica el efecto (efectoDoble).
inline int dadosExtraMagia(const Enfrentamiento& e) {
    const Salvacion s = salvacion(e);
    if (s == Salvacion::Nada || s == Salvacion::Supera) return 0;
    return e.todosSeis ? 1 : 0;
}

// D0015 — El objetivo pifio la salvacion: el estado de la carta, con el doble de rondas.
inline bool efectoDoble(const Enfrentamiento& e) { return salvacion(e) == Salvacion::Pifia; }

// El dano ya tirado de una carta de magia, segun la salvacion.
inline int danoMagico(int tirado, Salvacion s) {
    switch (s) {
        case Salvacion::Nada: return 0;
        case Salvacion::Supera: return std::max(0, tirado) / 2;
        case Salvacion::Falla:
        case Salvacion::Pifia: return std::max(0, tirado);
    }
    return 0;
}

}  // namespace RPG::dano
