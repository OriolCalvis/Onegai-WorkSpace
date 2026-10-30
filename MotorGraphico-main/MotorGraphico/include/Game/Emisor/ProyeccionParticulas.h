// U4-07.E06 — Proyeccion de un Emisor a `Game::Destello` para `pintaDestellos` (sin GL nuevo, D-T2). GL-free.
//
// Las coordenadas de un emisor son px de frame de su GrupoDeDibujo: mundo = esquina + (round(x)+dx) * tam/64.
// Alfa final = alfaMax * (a) * entradaF * salidaF:
//   (a) por vida de la particula: min(1, vivo*opacidad), vivo = 1-edad/vida (BrilloEquipo.cpp:274-277);
//   entradaF = min(1, tiempo/entrada) (siempre);  salidaF = min(1, (duracion-tiempo)/salida) SOLO con cortar=true
//   y duracion>0. Con cortar=false no hay envolvente de salida. Con cortar y tiempo>=duracion: corte duro (nada).
// `Destello` no lleva escala: `EstiloEmisor::escala` la aplica el adaptador (E07) al `GrupoDeDibujo::tam`; la
// proyeccion NO la usa (solo cambia el tamano, nunca posicion, alfa ni color).
#pragma once

#include <cstddef>
#include <vector>

#include "Core/Math/Vector2.h"
#include "Game/Emisor/Emisor.h"
#include "Game/Personajes/BrilloEquipo.h"

namespace Game::Emisor {

// Esquina de un pixel de forma. std::round (mitades lejos de cero) ANTES de escalar, como PintaDestellos.cpp.
Vector2 mundoDePixel(const GrupoDeDibujo& g, float xFrame, float yFrame, int dx, int dy);

struct EstiloEmisor {
    float alfaMax = 1.0f;     // 0..1
    float entrada = 0.08f;    // s: sube 0 -> 1 durante los primeros `entrada` s de Emisor::tiempo
    float salida = 0.4f;      // s: SOLO con cortar=true: baja 1 -> 0 durante los ultimos `salida` s
    float escala = 1.0f;      // 1 = un pixel de frame por pixel de forma (lo aplica el adaptador, ver arriba)
};

struct ResumenProyeccion {
    std::size_t destellos = 0;    // anadidos a `out` por esta llamada
    std::size_t quads = 0;        // cuerpo + contorno de lo anadido (solo con alfa>0)
    std::size_t descartados = 0;  // DESTELLOS (no quads) que no entran por quadsMax
};

// SUMA de pixelesDeForma(f).size() + contornoDeForma(f).size() de cada destello con alfa>0 (lo que pinta pintaDestellos).
std::size_t quadsDeDestellos(const std::vector<Game::Destello>& d);

// Anade a `out` (que NO se limpia: se acumula entre emisores del mismo grupo) los destellos de las particulas
// vivas y visibles, en orden de nacimiento, hasta `quadsMax` quads; el resto se cuenta en `descartados`.
// `Expande` emite 10 `Punto` por particula (aro de radio 1+6*(t/vida), y aplastada a 0,6).
ResumenProyeccion proyectarParticulas(const Emisor& e, const EstiloEmisor& estilo, std::size_t quadsMax,
                                      std::vector<Game::Destello>& out);

}  // namespace Game::Emisor
