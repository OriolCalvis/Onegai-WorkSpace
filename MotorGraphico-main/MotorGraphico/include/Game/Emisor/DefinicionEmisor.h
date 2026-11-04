// U4-07.E02 — La definicion de un emisor de particulas generico, leida de JSON.
//
// Un emisor echa particulas de una familia de Game/Personajes/FamiliasDeParticulas (las 40) con una tasa,
// una vida, una velocidad y un abanico. Aqui solo vive el DTO y su lector: nada avanza el tiempo ni sortea
// (eso es E03..E05). GL-free.
//
// Unidades: tiempo en segundos; posicion en px y velocidad en px/s del espacio del GrupoDeDibujo del emisor;
// angulos en GRADOS en el JSON (0 = +x, y de pantalla hacia abajo; -90 = arriba); tasa en particulas/s.
//
// Claves JSON (todas en minuscula y sin tildes; una clave desconocida es un Error, no se ignora):
//   familia (obligatoria), tasa, duracion, cortar, vida [min,max] (obligatoria), velocidad [min,max],
//   angulo, abanico, maximo (obligatoria), semilla, opacidad, contraste, color ("rrggbb").
#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "Core/Errors/Result.h"
#include "Core/Json/JsonValue.h"

namespace Game::Emisor {

// Limites de la definicion (provisionales, D-O2; los valida E08) y del presupuesto generico.
constexpr double kMaxTasa = 500.0;            // particulas/s
constexpr double kMaxVida = 10.0;             // s de vida de una particula
constexpr double kMaxDuracion = 60.0;         // s de vida del emisor
constexpr double kMaxVelocidad = 2000.0;      // px/s del espacio del grupo
constexpr int kMaxPorEmisor = 256;            // aforo de un emisor
constexpr int kMaxEmisores = 64;              // RESERVADO: sin consumidor en U4-07
constexpr int kMaxParticulasGlobal = 2048;    // vivas entre todos los emisores
constexpr int kMaxQuadsPorFotograma = 8192;   // cuerpo + contorno de todos los destellos
// 2^53 - 1: el mayor entero que un double JSON distingue de su vecino. JsonValue convierte a double ANTES de
// validar, asi que un 2^53+1 en el fichero llega ya redondeado a 2^53: por eso se rechaza todo x >= 2^53.
constexpr double kMaxSemilla = 9007199254740991.0;

struct DefinicionEmisor {
    std::string familia;          // material: una de familiasDeParticulas() (las 40)
    double tasa = 0.0;            // particulas/s, 0..kMaxTasa
    double duracion = 0.0;        // s de vida del EMISOR; 0 = continuo; 0..kMaxDuracion
    bool   cortar = false;        // true: al llegar a `duracion` se corta TODO; false: las vivas se agotan
    double vidaMin = 0.0, vidaMax = 0.0;            // s, (0..kMaxVida], vidaMin <= vidaMax
    double velocidadMin = 0.0, velocidadMax = 0.0;  // px/s, 0..kMaxVelocidad
    double anguloCentro = 0.0, abanico = 360.0;     // grados; abanico en (0..360]
    int    maximo = 0;            // aforo del emisor, 1..kMaxPorEmisor
    std::uint64_t semilla = 0;    // 0..2^53-1
    float opacidad = 1.0f;        // (0..2]; alfa = min(1, vivo*opacidad)
    float contraste = 0.0f;       // [0,1]: parte que nace oscura y tapa
    std::array<std::uint8_t, 3> colorBase{255, 240, 210};  // si tipo.colores esta vacio
};

// Lee y valida un emisor. Cualquier fallo (tipo, rango, familia inexistente, clave desconocida) es un Error
// "<ruta>: <motivo>"; nunca un valor por defecto silencioso.
Result<DefinicionEmisor> leerEmisor(const JsonValue& json);

}  // namespace Game::Emisor
