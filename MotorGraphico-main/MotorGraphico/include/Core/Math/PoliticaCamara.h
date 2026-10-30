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

}  // namespace PoliticaCamara
