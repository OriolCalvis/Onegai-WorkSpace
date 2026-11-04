// U4-07.E05 — Posicion de una particula en FORMA CERRADA de su edad (portada de BrilloEquipo.cpp:288-353).
//
// Funcion pura de (tipo, edad, vida, k, lado, vx0/vy0, ang0, radio0): no depende de como se trocee el tiempo.
// Posicion relativa al origen del emisor, en px del espacio del grupo. t = edad (s).
//   Sube/Cae/Chispazo: (vx0,vy0)*t + (vx*sin(3t+k)*0.4 + lado*vx*t*0.5, vy*t)
//   Deriva:            (vx0,vy0)*t + (sentido*vx*t + sin(5t+k)*0.6, vy*t)        sentido = lado<0 ? -1 : 1
//   Salpica:           (vx0,vy0)*t + (lado*vx*t, vy*t + 0.5*g*t*t)   g = 2|vy|/vida de la PARTICULA
//   Converge:          radio0*(1-t/vida)*(cos ang0, sin ang0)             (ignora vx0/vy0)
//   Orbita:            ang = ang0 + t*vx*0.5; r = (radio0-6)*0.75+5; (r cos, r*0.5*sin + vy*t)  (ignora vx0/vy0)
//   Expande:           (0,0): el aro de 10 puntos de radio 1+6*(t/vida) lo dibuja E06 (ignora vx0/vy0)
// `radio0` nace en [6,10) (E04); Orbita lo reescala a [5,8) como BrilloEquipo (5+3u).
#pragma once

#include "Game/Emisor/Particula.h"

namespace Game::Emisor {

constexpr float kEdadRayo = 0.12f;   // s: rayo y chispazo solo se ven hasta esta edad (BrilloEquipo.cpp:269)

// Recalcula p.x, p.y desde p.edad y los datos de nacimiento. No toca la edad.
void colocaParticula(const TipoDeParticula& tipo, Particula& p);

// true: la particula sigue viva pero no se proyecta (forma Rayo o movimiento Chispazo con edad > 0,12).
bool estaOculta(const TipoDeParticula& tipo, const Particula& p);

}  // namespace Game::Emisor
