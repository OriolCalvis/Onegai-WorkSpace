// U4-07.E04 — Una particula del emisor generico y su nacimiento con semilla explicita. GL-free.
//
// `crearParticula` es una funcion pura del generador que recibe: misma secuencia de azar -> misma particula,
// bit a bit. Nada `static`, ni rand(), ni random_device. El generador de CADA emisor lo construye
// `crearAzarEmisor` y lo guarda el Emisor (E05) en un unique_ptr: Xoroshiro128p no es asignable (R17).
//
// Orden FIJO de los 9 sorteos (siempre se consumen los 9, aunque no haga falta alguno):
//   1 vida  2 velocidad  3 angulo  4 variante  5 otraForma  6 ang0  7 radio0  8 oscura  9 lado
// Cada sorteo es `uniforme01`. Cambiar el orden o saltarse uno desplaza TODAS las particulas siguientes.
#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>

#include "Game/Emisor/DefinicionEmisor.h"
#include "Game/Personajes/FamiliasDeParticulas.h"
#include "RPG/RandomEngine.h"

namespace Game::Emisor {

constexpr int kSorteosPorParticula = 9;     // los que consume crearParticula, siempre
constexpr int kSorteosDescartados = 4;      // calentamiento del generador del emisor (R3)

struct Particula {
    float x = 0.0f, y = 0.0f;         // px del espacio del grupo, relativo al origen del emisor (nace en 0,0)
    float vx0 = 0.0f, vy0 = 0.0f;     // px/s: velocidad INICIAL del emisor (velocidad+angulo+abanico); no se integra
    float edad = 0.0f, vida = 0.0f;   // s; viva mientras edad < vida
    float ang0 = 0.0f, radio0 = 0.0f; // rad en [0,2pi) / px en [6,10): parametros fijos de Orbita/Converge/Expande
    float lado = 0.0f;                // [-1,1): 9.o sorteo; el signo es el sentido de la deriva
    std::uint16_t variante = 0;       // indice en TipoDeParticula::colores (0 si esta vacio)
    std::uint16_t k = 0;              // indice de nacimiento; lo asigna E05 (aqui siempre 0)
    std::uint8_t  otraForma = 0;      // 0/1: usa TipoDeParticula::otraForma
    std::uint8_t  oscura = 0;         // 0/1: nace del tono oscuro que tapa (contraste)
    std::uint16_t reservado = 0;      // rellena la estructura a 44 bytes sin huecos; siempre 0
};
// Sin relleno: el hash de bits de E08 (FNV-1a sobre la estructura) y la comparacion bit a bit son fiables.
static_assert(sizeof(Particula) == 44, "Particula no debe tener relleno (9 float + 4 u16 + 2 u8)");

// splitmix64 (una vuelta, con el incremento 0x9E3779B97F4A7C15): semillas consecutivas dan salidas muy distintas.
std::uint64_t mezclarSemilla(std::uint64_t semilla);

// [0,1) en pasos de 1e-6 (roll_in_range(0, 999999) / 1e6); nunca 1. Consume UN sorteo.
double uniforme01(RPG::RandomEngine& r);

// El generador de un emisor: Xoroshiro128p(mezclarSemilla(semilla)) con kSorteosDescartados sorteos tirados.
// Un generador por emisor, nunca compartido.
std::unique_ptr<RPG::Xoroshiro128p> crearAzarEmisor(std::uint64_t semilla);

// Nace una particula (edad 0, k 0, x=y=0). Consume SIEMPRE kSorteosPorParticula sorteos.
//   vida     = lerp(vidaMin, vidaMax, u1)
//   rapidez  = lerp(velocidadMin, velocidadMax, u2)
//   theta    = radianes(anguloCentro + (u3 - 0.5) * abanico); vx0 = rapidez*cos, vy0 = rapidez*sin
//   variante = floor(u4 * colores.size())  (0 si no hay colores)
//   otraForma= u5 < tipo.conOtraForma
//   ang0     = u6 * 2pi;  radio0 = 6 + 4*u7 (como Converge de BrilloEquipo)
//   oscura   = u8 < d.contraste;  lado = u9*2 - 1
Particula crearParticula(const DefinicionEmisor& d, const TipoDeParticula& tipo, RPG::RandomEngine& r);

}  // namespace Game::Emisor
