// H2-03 — Los estados de cada combatiente, como se ensenan.
//
// JUGABILIDAD_FASE_H.md, H2-03: «Un enemigo envenenado muestra el icono y el
// contador, y ambos bajan por ronda». Los estados ya caducaban de verdad (las
// rondas de la mesa de cartas en CartasEnCombate::finDeRonda y las
// condiciones de la ficha en BattleState::tickRound, H0-06), pero la placa
// del enemigo solo decia «Cancion del Otono · 1» y el heroe no ensenaba los
// suyos.
//
// Un combatiente lleva estados de dos sitios: los de la mesa (lo que deja una
// carta, o una condicion sobre quien no tiene ficha) y las condiciones de su
// ficha (D-09: duran mas alla del combate). Aqui se juntan, cada uno con su
// icono del atlas (el de la condicion, o el de lo que hace: el dano por turno
// sangra, la marca marca...), las rondas que le quedan y si va a favor o en
// contra de quien lo lleva. El pergamino lo pinta (MesaCombateHud). Sin GL.
//
// La pieza de texto de antes (HUD/ConditionWidget, solo condiciones de la
// ficha) sigue como estaba; el combate en pergamino usa esta.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

class BattleState;

namespace Game {

struct EstadoVisible {
    enum class Signo { Contra, Neutro, AFavor };
    std::string id;           // condicion («envenenado») o carta («cancion_del_otono»)
    std::string nombre;       // «Envenenado», «Cancion del Otono»
    std::string descripcion;  // lo que hace, para el emergente
    std::string icono;        // id en assets/textures/iconos.json
    int rondas = -1;          // las que le quedan; -1 = sin cuenta (hasta el final, o hasta curarse)
    bool hastaCurarse = false;
    int pila = 1;             // acumulaciones (Fatiga x3)
    Signo signo = Signo::Contra;
};

// Los del aliado o enemigo `i` de la batalla: primero los de la mesa, luego
// las condiciones de su ficha. Vacio si no esta o no lleva ninguno.
std::vector<EstadoVisible> estadosDe(const BattleState& batalla, bool aliado, std::size_t i);

// «2 rondas», «1 ronda», «hasta curarse» o «todo el combate».
std::string duracionDe(const EstadoVisible& e);

}  // namespace Game
