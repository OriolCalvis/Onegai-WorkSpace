// UI-BG3-01 — Lo que comparten los tres ficheros del cuaderno de creacion
// (PaginasCreacion.cpp: marco, bloques comunes y rejillas;
// PaginasCreacionHojas.cpp: Raza, Clase, Origen y Deidad;
// PaginasCreacionHistoria.cpp: Aspecto, Historia y Nombre). Cabecera
// interna: no la incluye nadie de fuera.
//
// Las medidas son las de las maquetas C02 en una vista de 1280x720; en una
// vista mayor, pintaCreacion desplaza todo al centro.
#pragma once

#include <string>
#include <vector>

#include "Game/CuadernoCreacion.h"
#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/PaginasCreacion.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino::creacion {

using Cuaderno = Game::CuadernoCreacion;
using Hoja = Cuaderno::Hoja;
using ZonaC = Cuaderno::Zona;

// La hoja de papel dentro del cuero y sus tres columnas.
constexpr float kHojaX = 38.0f, kHojaY = 100.0f, kHojaW = 1204.0f, kHojaH = 530.0f;
constexpr float kIzqX = 62.0f;                         // rejilla, matriz o campo
constexpr float kTeselaW = 104.0f, kTeselaH = 100.0f;  // una tesela de la rejilla
constexpr float kHueco = 9.0f;
constexpr float kCentroX = 650.0f, kCentroW = 256.0f;  // el personaje y sus atributos
constexpr float kFichaX = 928.0f, kFichaW = 290.0f;    // la ficha de lo senalado

struct Contexto {
    Lienzo& l;
    const Pincel& p;
    const Cuaderno& c;
    Retratista& r;
};

inline int tipo(ZonaC z) { return static_cast<int>(z); }

// --- Texto ---
// «COLOR DE LA BARBA» -> «Color de la barba» (las etiquetas del editor
// vienen en mayusculas, de cuando la fuente del HUD no tenia minusculas).
std::string frase(const std::string& s);
// Lineas centradas en [x, x + ancho]; como mucho `maxLineas` (la ultima con
// puntos suspensivos). Devuelve cuantas ha pintado.
int centrado(const Contexto& k, Letra letra, const std::string& s, float x, float y, float ancho, float alto,
             int maxLineas, const Vector4& color);

// --- Piezas ---
void boton(const Contexto& k, const Rect& r, const std::string& texto, bool primario);
// Una etiqueta en caja (filtro de papel, sugerencia de nombre). activo: tinta
// de fondo y letra clara.
float chip(const Contexto& k, const std::string& texto, float x, float y, bool activo);
// Rombo de trazos bajo los pies del personaje.
void sombraRombo(const Contexto& k, float cx, float cy, float semiAncho, float semiAlto);
// El personaje de la vista previa a escala `escala` con los pies en
// (cx, yPies), andando y girando (o quieto mirando a c.rumbo()).
void pintaVista(const Contexto& k, const Cuaderno::Vista& v, float cx, float yPies, int escala, int orientacion = -1);
// «AL EMPEZAR» o «CON EL KIT PUESTO»: las cuatro cajas de atributo.
void pintaAtributos(const Contexto& k, const Cuaderno::Vista& v, float x, float y);
// La columna del centro: personaje, girar y atributos (Raza, Origen, Deidad).
void pintaCentro(const Contexto& k, const Cuaderno::Vista& v, float yPies, float yAtributos);

// --- Rejillas (Raza, Clase, Origen, Deidad) ---
// Pinta las filas visibles de ids() desde (x, y) y deja sus zonas. `retratos`
// = la tesela lleva el personaje (Raza y Clase); si no, el icono `icono`.
void pintaRejilla(const Contexto& k, float x, float y, bool retratos, const char* icono);
// «1–20 de 43 · rueda del raton o S para ver mas».
void pintaCuenta(const Contexto& k, float x, float y, const char* que);

// --- Ficha de la derecha ---
// Nombre grande (hasta dos lineas); devuelve la y de debajo.
float fichaTitulo(const Contexto& k, const std::string& nombre, float y);
// Rotulo «RASGO ------» y un parrafo; devuelve la y de debajo.
float fichaBloque(const Contexto& k, const std::string& rotulo, const std::string& titulo, const std::string& texto,
                  float y, int maxLineas);

// --- Hojas ---
void pintaRaza(const Contexto& k);
void pintaClase(const Contexto& k);
void pintaOrigen(const Contexto& k);
void pintaDeidad(const Contexto& k);
void pintaAspecto(const Contexto& k);
void pintaHistoria(const Contexto& k);
void pintaNombre(const Contexto& k);

}  // namespace Pergamino::creacion
