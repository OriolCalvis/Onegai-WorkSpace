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

// Contorno real del mapa: paralelogramo (rombo si columnas == filas) formado por las casillas,
// en pixeles de MUNDO. Origen y geometria de IsometricRenderer/MapFraming: el rectangulo tw x th
// de la casilla (gx,gy) arranca en IsoMath::gridToScreen(gx,gy) y su rombo tiene la esquina
// superior en (+tw/2, 0) de ese origen. Vertices: superior = esquina sup. de la casilla (0,0);
// derecho = esquina dcha. de (columnas-1,0); inferior = esquina inf. de (columnas-1,filas-1);
// izquierdo = esquina izq. de (0,filas-1). Cualquier punto del contorno cae en el rombo de alguna
// casilla; ESE es el punto que cuenta como «sobre terreno» (se usa el rombo entero de la casilla).
struct RomboMapa {
    Vector2 superior;
    Vector2 derecho;
    Vector2 inferior;
    Vector2 izquierdo;
};

// Caja del mapa en pixeles de MUNDO iso (origen = esquina superior del rombo de la casilla 0,0).
// Si usaRombo es false solo se aplica la caja (mapas sinteticos / pruebas).
struct LimitesMapa {
    float izquierda = 0.0f;
    float derecha = 0.0f;
    float arriba = 0.0f;
    float abajo = 0.0f;
    bool usaRombo = false;
    RomboMapa rombo;
};

// Vertices del contorno del mapa (funcion pura, calculo en double, float una sola vez).
//  - Error: columnas, filas, anchoTile o altoTile <= 0.
Result<RomboMapa> romboDeMapa(int columnas, int filas, int anchoTile, int altoTile);

// Caja que contiene el rombo del mapa + el propio rombo (usaRombo = true). Caja con la misma
// geometria que Editor::fitMap (MapFraming.cpp:17-19), duplicada a proposito (D-T10).
// margenSuperiorPx (>= 0, defecto 0) sube el borde superior de la CAJA (arriba = -margen) para
// sprites altos / alturas de terreno (D-T7); el rombo no cambia.
//  izquierda = -(filas-1)*tw/2; derecha = (columnas-1)*tw/2 + tw; arriba = -margen;
//  abajo = (columnas+filas)*th/2.
//  - Error: columnas, filas, anchoTile o altoTile <= 0; margen negativo o no finito.
Result<LimitesMapa> limitesDeMapa(int columnas, int filas, int anchoTile, int altoTile,
                                  float margenSuperiorPx = 0.0f);

// Limita el CENTRO de la vista (D-O3 precisada por Oriol para isometrico: «el centro de la
// pantalla siempre sobre terreno»). Orden:
//  1. Caja: por eje, mitad = vista/(2*zoom) en mundo; si (max-min) <= 2*mitad el eje se CENTRA
//     en (min+max)/2; si no, clamp(centro, min+mitad, max-mitad). Garantiza vista dentro de la
//     caja cuando cabe.
//  2. Rombo (si usaRombo): el centro se lleva al punto MAS CERCANO (euclideo) del contorno. Si ese
//     punto saca la vista de la caja (eje estrecho), se acerca al centro de la caja por el
//     segmento hasta el limite de la caja: el resultado esta SIEMPRE en caja (paso 1) y rombo.
//  Un centro que ya cumple ambas cosas (rombo con tolerancia 1e-3 px) se devuelve IDENTICO. En
//  bordes y esquinas puede verse algo de vacio; nunca una pantalla sin terreno en el centro.
//  Precision: todo en double; el resultado limitado se redondea a float una vez, asi que el
//  error es <= medio ulp de float (~2,4e-4 px de mundo con |coord| <= 4096, ~1,2e-4 con 2048).
//  La tolerancia de comparacion fijada en las pruebas es 1e-3 px de mundo ABSOLUTA, valida para
//  |coord| <= 4096; para mapas mayores usar tolerancia relativa ~ 2^-23 * |coord|.
//  - Error: zoom no finito o <= 0; vista con ancho o alto <= 0; centro no finito; limites o
//    rombo no finitos; caja invertida (derecha < izquierda o abajo < arriba).
Result<Vector2> limitarAlMapa(const Vector2& centro, float zoom, const TamanoVista& vista,
                              const LimitesMapa& limites);

}  // namespace PoliticaCamara
