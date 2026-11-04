// H2-01 — El dado se ve: las tiradas pintadas en el pergamino.
//
// JUGABILIDAD_FASE_H.md, H2-01: «Al hablar, al atacar y al usar una habilidad
// se ve la tirada y su grado. La pifia se distingue a simple vista». Cada dado
// es una cara de d6 con sus puntos. Los que cuentan llevan color (Ed. 3): el 6
// es un éxito (verde) y el 5, medio (ámbar); con ventaja el 4 también vale
// medio, y con desventaja solo cuenta el 6. Los 1 de una pifia van en rojo, y
// el rótulo de la pifia también.
//
// Dos sitios:
//   - el combate (MesaCombateHud): el panel de la última tirada, bajo el
//     registro, con los dados de quien tira y los del rival;
//   - el diálogo (DialogoHoja): una tira encima de la hoja cuando el beat
//     pide una prueba («Tirada de Percepción contra el vacío»).
// Sin OpenGL.
#pragma once

#include <string>

#include "HUD/Pergamino/Lienzo.h"
#include "RPG/TiradaVisible.h"

namespace Pergamino {

class Pincel;

namespace dados {

// Lo que vale cada dado en el recuento: 0 nada, 1 medio éxito, 2 éxito,
// -1 un 1 de una pifia.
enum Cuenta : int { kNada = 0, kMedio = 1, kExito = 2, kPifia = -1 };
Cuenta cuentaDe(const RPG::PoolResult& r, int valor);

// Una cara de d6 de `lado` px (21 en el HUD) con sus puntos.
void cara(Lienzo& l, int valor, float x, float y, float lado, Cuenta cuenta);
// La fila de dados de una tirada, de izquierda a derecha, sin pasar de
// `anchoMax` (los dados se encogen si no caben, hasta 10 px). Devuelve el
// ancho usado, que es el que da anchoFila.
float fila(Lienzo& l, const RPG::PoolResult& r, float x, float y, float anchoMax, float ladoMax = 21.0f);
float anchoFila(std::size_t dados, float anchoMax, float ladoMax = 21.0f);

// «ÉXITO», «PARCIAL», «FALLO», «CRÍTICO» o «¡PIFIA!», y su color.
const char* rotulo(const RPG::TiradaVisible& t);
Vector4 color(const RPG::TiradaVisible& t);
// «1,5»: los éxitos con coma, como en el registro.
std::string exitos(float n);

// Oriol (22/10): «falta la animación de lanzar dados». Los dados de `r` dando vueltas en el instante
// `t` (segundos desde que salen): cada cara cambia unas 14 veces por segundo; nada de pifias ni éxitos.
RPG::PoolResult rodando(const RPG::PoolResult& r, float t);
// Lo que dura la vuelta antes de que se vea el resultado.
inline constexpr float kRodar = 0.7f;

}  // namespace dados

// El panel de la última tirada del combate, de 280 de ancho, con la esquina
// arriba a la izquierda en (x, y). `brillo` (0..1) la resalta cuando es nueva.
// Con `en` > 1 la acción tiró varias veces y la cabecera dice cuál es
// («2/3»). Devuelve el alto.
// `rodar` >= 0: los dados aun ruedan (segundos desde que salieron, ver dados::rodando): sin grado ni
// marco hasta que paran. Por defecto (-1), quietos.
float pintaTiradaCombate(Lienzo& l, const Pincel& p, const RPG::TiradaVisible& t, float x, float y,
                         float brillo, int de = 0, int en = 0, float rodar = -1.0f);
// La tira de la prueba de un beat, apoyada encima de la hoja del diálogo con
// su borde derecho en `derecha` y su pie en `abajo`.
void pintaTiradaDialogo(Lienzo& l, const Pincel& p, const RPG::TiradaVisible& t, float derecha, float abajo,
                        float rodar = -1.0f);

}  // namespace Pergamino
