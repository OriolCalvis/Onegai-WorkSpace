#pragma once

#include <cstdint>

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
//  Rango de la caja para el centro: por eje, mitad = vista/(2*zoom) en mundo; si (max-min) <=
//  2*mitad el eje se CENTRA en (min+max)/2 (rango de un punto); si no, [min+mitad, max-mitad].
//  Con usaRombo el resultado es la PROYECCION EUCLIDEA del centro sobre el poligono convexo
//  caja-rombo (rombo recortado contra el rectangulo del rango, Sutherland-Hodgman): el propio centro
//  si ya esta dentro (tolerancia 1e-3 px), si no el punto mas cercano de sus aristas. Es 1-Lipschitz
//  y valida igual con rango 2D, casi colapsado (poligono degenerado a un segmento fino) o colapsado
//  (segmento/punto): sin umbrales, sin ancla y sin saltos al barrer la sacudida o el zoom.
//  Si caja y rombo no se cortan (interseccion vacia) gana el rombo: el resultado es el punto del
//  rombo mas cercano al RECTANGULO del rango (no al centro dado, para ser continuo en la frontera
//  en que la interseccion se vacia al cambiar el zoom); en ese caso la caja no se respeta.
//  Sin usaRombo solo se aplica la caja (clamp por eje).
//  Resultado: SIEMPRE sobre el rombo; en el rango de la caja siempre que la interseccion no sea
//  vacia; idempotente (el resultado ya esta dentro con tolerancia).
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

// Sacudida de camara (E04). Amplitud en pixeles de PANTALLA (no depende del zoom; quien la aplica
// divide por zoom, ver E05). Sin estado: el desplazamiento es una funcion pura de (estado, t, semilla).
struct EstadoSacudida {
    float amplitudPx = 0.0f;  // amplitud maxima por eje X, en pixeles de pantalla
    float duracion = 0.0f;    // segundos
};

// Desplazamiento de vista en pixeles de PANTALLA en el instante `transcurrido` (segundos desde el
// disparo). Determinista: ruido por hash splitmix64 de (semilla, floor(transcurrido*60)), sin rand()
// ni reloj; la misma entrada da el mismo resultado bit a bit y no es acumulativa.
//  - Envolvente: (1 - t/duracion)^2 (decrece hasta 0 en t = duracion); eje X en [-1,1], eje Y por 0,6.
//    |x| <= amplitud*factor*envolvente, |y| <= 0,6 de eso.
//  - factorMovimiento en [0,1] escala linealmente (D-O4: 0 = apagada => (0,0) exacto).
//  - Devuelve (0,0) exacto si transcurrido < 0 o >= duracion, o amplitud <= 0, o duracion <= 0,
//    o factorMovimiento == 0.
//  - Error: amplitud, duracion o transcurrido no finitos; transcurrido*60 >= 2^63 (indice de fotograma
//    no representable); factorMovimiento no finito o fuera de [0,1].
Result<Vector2> desplazamientoSacudida(const EstadoSacudida& s, double transcurrido,
                                       std::uint32_t semilla, float factorMovimiento = 1.0f);

// Pose compuesta por fotograma (E05). base = posicion logica recortada a los limites (sin sacudida);
// vista = lo que se lee para proyectar mundo->pantalla (base + sacudida/zoom, recortada otra vez).
struct PoseCamara {
    Vector2 base;
    Vector2 vista;
    float zoom = 1.0f;
};

// Regla D-T6: true = la vista final (con sacudida) se recorta a los limites (nunca vacio por
// sacudida; en un borde el temblor es unilateral); false = sacudida libre.
struct PoliticaPose {
    bool sacudidaDentroDeLimites = true;
};

// Compone la pose de un fotograma. posicionLogica y zoom son los ACTUALES de Camera (zoom
// interpolado, D-T12). sacudidaPantalla llega en pixeles de PANTALLA (salida de E04); aqui es el
// UNICO sitio donde se divide por zoom para pasar a mundo (D-T5). La posicion logica no se modifica.
//  1. base = limitarAlMapa(posicionLogica, zoom, vista, *limites)  (sin limites: base = posicionLogica).
//  2. vista = base + sacudidaPantalla/zoom (double, float una sola vez).
//  3. Con limites y sacudidaDentroDeLimites: vista = limitarAlMapa(vista, ...) sobre la pose FINAL.
//  Con sacudida (0,0) vista == base bit a bit. limites == nullptr (titulo, D-O6): base + sacudida.
//  - Error: zoom no finito o <= 0; posicionLogica o sacudidaPantalla no finitas; con limites,
//    los errores de limitarAlMapa (vista <= 0, limites no finitos...).
Result<PoseCamara> componerPose(const Vector2& posicionLogica, float zoom,
                                const Vector2& sacudidaPantalla, const TamanoVista& vista,
                                const LimitesMapa* limites, const PoliticaPose& politica = {});

}  // namespace PoliticaCamara
