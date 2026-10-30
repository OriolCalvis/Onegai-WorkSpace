#pragma once

// U3-05.E07b -- Carga del clip «idle» del cuerpo base (D3). GL-free.
// assets/animaciones/idle_cuerpo_base.clips.json: clip «idle», sprites 1..12, 220 ms cada uno
// (el ritmo que tenia AnimatedEntity con frameTime 0.22 s), ciclico, sin eventos.

#include <optional>
#include <string>

#include "Game/ClipAnimacion.h"

namespace Game {

inline constexpr const char* kRutaIdleCuerpoBase = "assets/animaciones/idle_cuerpo_base.clips.json";

// Lee el fichero y devuelve el clip de id `idClip`; ante cualquier fallo escribe el motivo por
// std::cerr y devuelve nullopt (nunca lanza ni rompe el nivel).
std::optional<ClipAnimacion> cargaClipDeFichero(const std::string& ruta, const std::string& idClip);

// Idle del cuerpo base: se carga una sola vez (tambien el fallo) y se comparte. nullptr si fallo.
const ClipAnimacion* idleCuerpoBase();

}  // namespace Game
