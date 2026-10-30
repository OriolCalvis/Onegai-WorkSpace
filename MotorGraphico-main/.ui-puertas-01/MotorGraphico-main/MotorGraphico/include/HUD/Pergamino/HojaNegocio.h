// AV15-05a (EH-29, EH-30, EH-31) — La hoja del negocio en el pergamino.
//
// Sustituye al panel oscuro de antes: un papiro con el negocio (lo que
// renta, lo que tienes), sus filas (Game::NegocioEstado: cada una con lo que
// hace o por qué no se puede), lo que acaba de pasar y el pie de teclas. Si
// hay una pregunta (comprar o revender), va encima, en su propio papiro, con
// «No» primero y con el foco. Solo pinta: no decide nada.
#pragma once

#include "HUD/Pergamino/Lienzo.h"

class GameSession;

namespace Game {
class NegocioEstado;
}

namespace Pergamino {

class Pincel;

void pintaNegocio(Lienzo& l, const Pincel& p, const GameSession& sesion, const Game::NegocioEstado& negocio,
                  int anchoVista, int altoVista);

}  // namespace Pergamino
