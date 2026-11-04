// C2-04c — Qué casillas arden y con qué semilla, sin GL (lo dibuja Render/FuegoNivel).
#pragma once

#include <cstdint>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/EscenaViva.h"

namespace Game {

// Las casillas con fuego encendido, en orden estable (por y y luego por x).
std::vector<GridCoord> celdasAEncender(const EscenaViva& escena);

// La semilla de las llamas de una casilla: siempre la misma para la misma y distinta
// para casillas distintas (dos casillas contiguas no arden igual).
std::uint32_t semillaDe(const GridCoord& celda);

}  // namespace Game
