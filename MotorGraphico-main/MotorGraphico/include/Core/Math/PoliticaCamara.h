#pragma once

#include "Core/Errors/Result.h"
#include "Core/Math/Vector2.h"

// U4-01 — Politica pura de camara (sin GL). Sala limpia: matematica estandar.
// Unidades: zona muerta en pixeles de PANTALLA (invariante al zoom); centro y actor en
// pixeles de MUNDO iso. Calculo interno en double, resultado float una sola vez (D-T3).
namespace PoliticaCamara {

// Tamano de la vista en pixeles de pantalla.
struct TamanoVista {
    int ancho = 0;
    int alto = 0;
};

// Semi-dimensiones de la zona muerta, en pixeles de pantalla. {0,0} = centrar siempre
// (comportamiento actual del juego; el valor de producto D-O1 lo fija E06, no esta politica).
struct ZonaMuerta {
    float semiAnchoPx = 0.0f;
    float semiAltoPx = 0.0f;
};

// Seguimiento con zona muerta. centroActual = destino vigente de la camara (mundo);
// actorMundo = pixel de mundo del actor. La zona equivale a semi/zoom en mundo.
//  - Por eje: si |actor - centro| <= semi/zoom + 1e-4 se devuelve ese eje IDENTICO (sin
//    jitter en la frontera); si no, el centro se empuja lo minimo para dejar al actor
//    SOBRE el borde de la zona.
//  - Error: zoom no finito o <= 0; zona negativa o no finita; centro o actor no finitos.
Result<Vector2> seguirConZonaMuerta(const Vector2& centroActual, float zoom,
                                    const Vector2& actorMundo, const ZonaMuerta& zona);

// Caja del mapa en pixeles de MUNDO iso (origen = esquina superior del rombo de la casilla 0,0).
struct LimitesMapa {
    float izquierda = 0.0f;
    float derecha = 0.0f;
    float arriba = 0.0f;
    float abajo = 0.0f;
};

// Caja que contiene el rombo del mapa; misma geometria que Editor::fitMap (MapFraming.cpp:17-19),
// duplicada a proposito para no arrastrar el editor (D-T10). margenSuperiorPx (>= 0, defecto 0)
// sube el borde superior (arriba = -margen) para sprites altos / alturas de terreno (D-T7).
//  izquierda = -(filas-1)*tw/2; derecha = (columnas-1)*tw/2 + tw; arriba = -margen;
//  abajo = (columnas+filas)*th/2. Calculo en double, float una sola vez.
//  - Error: columnas, filas, anchoTile o altoTile <= 0; margen negativo o no finito.
Result<LimitesMapa> limitesDeMapa(int columnas, int filas, int anchoTile, int altoTile,
                                  float margenSuperiorPx = 0.0f);

// Limita el CENTRO de la vista para que la vista nunca salga de la caja (D-O3: nunca se ve fuera
// del mapa). Por eje: mitad = vista/(2*zoom) en mundo; si (max-min) <= 2*mitad (el mapa cabe) el
// eje se CENTRA en (min+max)/2; si no, clamp(centro, min+mitad, max-mitad). Un centro ya valido
// se devuelve IDENTICO (sin redondeo). Es una caja rectangular: no recorta las esquinas vacias
// del rombo.
//  Precision: todo en double; el resultado limitado se redondea a float una vez, asi que el
//  error es <= medio ulp de float (~2.4e-4 px de mundo con |coord| <= 4096, ~1.2e-4 con 2048).
//  La tolerancia de comparacion fijada en las pruebas es 1e-3 px de mundo ABSOLUTA, valida para
//  |coord| <= 4096; para mapas mayores usar tolerancia relativa ~ 2^-23 * |coord|.
//  - Error: zoom no finito o <= 0; vista con ancho o alto <= 0; centro no finito; limites no
//    finitos o invertidos (derecha < izquierda o abajo < arriba).
Result<Vector2> limitarAlMapa(const Vector2& centro, float zoom, const TamanoVista& vista,
                              const LimitesMapa& limites);

}  // namespace PoliticaCamara
