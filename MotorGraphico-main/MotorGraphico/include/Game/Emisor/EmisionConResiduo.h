// U4-07.E03 — Cuantas particulas nacen en un paso, con residuo fraccional entre pasos. GL-free.
//
// La tasa (particulas/s) no da enteros por tick: lo que sobra de un paso (`residuo`, en [0,1)) se suma al
// siguiente, de modo que el total de nacimientos no depende de como se trocea el tiempo. El movimiento
// reducido (D-O3) NO tiene parametro propio: el llamador pasa `tasa * escalaTasa` (0,25). Quien emite solo
// hasta `duracion` (E05) tambien recorta `dt` ANTES de llamar; aqui solo se acota el catch-up.
#pragma once

#include "Core/Errors/Result.h"

namespace Game::Emisor {

// Particulas fraccionarias pendientes entre pasos, siempre en [0,1).
struct EstadoEmision { double residuo = 0.0; };

struct PasoEmision {
    EstadoEmision estado;       // residuo tras el paso
    int cuantas = 0;            // nacimientos de este paso
    bool recortado = false;     // dt > kMaxPasoEmision: el tiempo sobrante se descarto
};

// UNICA definicion (E05 la usa, no la redefine). Tras una pausa larga de 5 s no sale una rafaga.
constexpr double kMaxPasoEmision = 0.25;   // s: tope de catch-up por llamada
// 60 pasos de 1/60 a tasa 10 acumulan 9,999999999999998 y deben dar 10 (patron de kToleranciaFinTween).
constexpr double kTolEmision = 1e-9;

// dt en s, tasa en particulas/s. Error si dt o tasa no son finitos o son negativos, tasa > kMaxTasa o el
// residuo de entrada esta fuera de [0,1). dt==0 o tasa==0 -> cuantas 0 y residuo intacto (pausa, D-O6).
// dt > kMaxPasoEmision se acota y marca `recortado` (tambien con tasa 0).
Result<PasoEmision> emisiones(const EstadoEmision& e, double dt, double tasa);

}  // namespace Game::Emisor
