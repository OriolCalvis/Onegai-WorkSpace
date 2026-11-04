#pragma once

// U3-06.E05 -- Entradas de la maquina de animacion para un PNJ (pura, sin GL). El productor
// (EstadoPnj, D-T5) calcula todo lo que depende de su reloj y su rumbo; la maquina
// assets/animaciones/pnj_basico.estados.json solo elige idle/walk por el rumbo.
// Mismas guardas, mismo orden y misma aritmetica float que el oraculo RutinasPnj::animacion
// (que se mantiene): andando() manda; luego la actividad segun el movimiento; luego el defecto.
// Emite solo "andando" (0/1) y "rumbo" (0..3), siempre finitos. Contrato:
// planificacion/pla_unity/descomposicio/contratos/U3-06_estados.md (E05).

#include "Game/RutinasPnj.h"
#include "Game/SeleccionAnimacion.h"

namespace Game {

EntradasAnimacion entradasDePnj(const EstadoPnj& e, bool reducidas);

}  // namespace Game
