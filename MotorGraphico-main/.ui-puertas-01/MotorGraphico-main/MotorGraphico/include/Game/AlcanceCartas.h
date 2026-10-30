#pragma once
// AV13-13 — El alcance de las cartas sobre el tablero (COMBATE_BG3.md §5).
//
// La carta dice hasta donde llega (D-02, la clave `range` del asset); aqui se
// pasa a casillas (1 casilla = 1,5 m) y se mira a quien llega cada una:
//
//   sobre ti   0
//   al lado    1            (cuerpo a cuerpo; tambien Atacar)
//   corto      4  (6 m)
//   medio      8  (12 m)
//   largo     12  (18 m: el tablero entero, como los conjuros de BG3)
//
// Mas alla de 1 casilla hace falta verle (linea de vision del tablero). Las
// cifras de corto/medio/largo son una propuesta: viven solo aqui.
//
// Una carta sin dato (UNKNOWN) se toma como corto, o sobre ti si es SELF,
// hasta que AV13-09 lo rellene aguas arriba.

#include "Game/BarraCombate.h"
#include "Game/TableroCombate.h"
#include "RPG/Definitions/SkillDefinition.h"

class BattleState;

namespace Game {

int casillasDeAlcance(RPG::SkillRange rango, RPG::SkillTarget objetivo);

// Con tablero: rellena el alcance y quien esta a tiro de cada carta de la vista
// (las basicas y la mano), y bloquea, con su motivo, la que no llega a nadie a
// quien pueda apuntar. Sin tablero no toca nada.
void ponAlcances(VistaBarra& v, const BattleState& batalla);

// Las casillas del tablero a las que llega `quien` con `casillas` (pisables, sin la
// suya; mas alla de 1, viendolas): lo que se pinta en el suelo al senalar una carta.
std::vector<GridCoord> casillasATiro(const TableroCombate& t, const std::string& quien, int casillas);

// AV13-14 — Lo que corta un area: una casilla que no se pisa y donde no hay nadie (un muro, un mueble).
bool esMuro(const TableroCombate& t, GridCoord c);

}  // namespace Game
