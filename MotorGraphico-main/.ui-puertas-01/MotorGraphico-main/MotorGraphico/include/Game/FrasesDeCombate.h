#pragma once
// EH-16 (evaluación heurística, 14/10) — El resultado de las tiradas, en palabras.
//
// Antes el registro decía «Empate, 0 contra 0: roce, 1 de dano»: incomprensible para quien no
// conoce el sistema. Ahora dice qué ha pasado y, entre paréntesis, los números:
//   «Venides ataca a Lobo: el ataque (1,5) supera la defensa (0). 7 de daño (2d6).»
// GL-free; lo usan las tiradas de la mesa (CartasTiradas.cpp, CartasJugar.cpp). Pruebas:
// tests/Game/FrasesDeCombateTests.cpp.

#include <string>

#include "RPG/DicePoolEngine.h"

namespace Game::frases {

// Los éxitos como se leen: «1», «1,5».
std::string numero(float exitos);
// Una tirada contra otra: «el ataque (1,5) supera la defensa (0)», «crítico: …», «empatan …»,
// «la defensa (1) para el ataque (0)», «pifia: el ataque falla del todo».
std::string enfrentada(RPG::Degree grado, float ataque, float defensa, bool pifia);
// Sin nadie enfrente (una cura): «acierto (2 éxitos)».
std::string sinRival(RPG::Degree grado, float exitos, bool pifia);
// Quien no puede defenderse: «no se defiende», «no se defiende: crítico».
std::string sinDefensa(RPG::Degree grado);
// Lo que hace: «7 de daño (2d6)», «1 de daño (solo roza)», «Sin daño».
std::string dano(int dano, const std::string& como);

}  // namespace Game::frases
