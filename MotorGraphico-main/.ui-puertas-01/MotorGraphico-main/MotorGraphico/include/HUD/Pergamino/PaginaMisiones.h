// AV14-22 (D0016) — La hoja de Misiones del cuaderno, como datos.
//
// Sigue USABILIDAD_UOC.md: en cinco segundos el jugador tiene que saber que
// le toca. La lista empieza por la mision activa y su detalle abre con
// «Ahora: …». Debajo de cada mision empezada salen, sangrados, sus
// apartados (con su cuenta) y cada decision que ya tomaste; las misiones
// que aun no han empezado son una sola fila apagada (no se cuentan los
// capitulos antes de jugarlos). Al elegir:
//   - una mision: lo que toca, sus apartados con su cuenta y el resumen;
//   - un apartado: sus objetivos (hechos, por hacer, opcionales, contadores);
//   - una decision: lo que elegiste, lo que paso y lo que ha pasado despues.
// GL-free: rellena una PaginaCuaderno, la pinta pintaPagina().
#pragma once

#include <cstddef>
#include <vector>

#include "HUD/Pergamino/PaginaCuaderno.h"

namespace Game {
struct DiarioDatos;
}  // namespace Game

namespace Pergamino {

struct FilaDiario {
    enum class Tipo { Mision, Apartado, Decision };
    Tipo tipo = Tipo::Mision;
    std::size_t mision = 0;  // indice en DiarioDatos::misiones
    std::size_t indice = 0;  // apartado o decision dentro de la mision
};

// Las filas de la lista, en orden (una por mision, y las de sus apartados y
// decisiones si la mision ha empezado).
std::vector<FilaDiario> filasDelDiario(const Game::DiarioDatos& diario);

// La hoja con la fila `elegida` (se recorta a la ultima) en el detalle.
// El diario no puede venir vacio: la hoja vacia la decide quien llama.
PaginaCuaderno paginaMisiones(const Game::DiarioDatos& diario, std::size_t elegida);

}  // namespace Pergamino
