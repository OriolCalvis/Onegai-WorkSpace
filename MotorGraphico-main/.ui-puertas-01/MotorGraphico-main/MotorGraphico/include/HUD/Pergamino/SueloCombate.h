// AV13-10 — El borde del tablero del combate, en tinta sobre el suelo.
//
// Oriol (14/10, D0014): «moverse por casillas en combate, como en un tactics,
// con las areas visibles». El combate se juega en un recuadro de casillas del
// nivel (Game::TableroCombate); aqui se decide QUE trazos lleva el suelo: las
// aristas de las casillas del recuadro que dan fuera de el. Donde el borde
// cae sobre una pared o un mueble no se pinta (la linea se corta, como la
// pintaria alguien con una tiza). Lo dibuja Render/PergaminoGL en el pase de
// sombras del mundo, bajo los sprites, igual que el rombo de MarcaSuelo.
//
// Las aristas van en coordenadas de ESQUINA de la rejilla: la casilla (x,y)
// tiene sus vertices en (x,y) arriba, (x+1,y) a la derecha, (x+1,y+1) abajo y
// (x,y+1) a la izquierda (IsoMath::gridToScreen las lleva al mundo). GL-free.
#pragma once

#include <functional>
#include <vector>

#include "Core/Math/GridCoord.h"

namespace Pergamino {

struct AristaSuelo {
    GridCoord desde, hasta;  // esquinas de la rejilla
};

struct SueloCombate {
    bool visible = false;
    GridCoord origen{};
    int ancho = 0, alto = 0;
    std::vector<bool> pisable;  // ancho * alto, por filas: se pinta el borde de las pisables
    float anchoBaldosa = 64.0f, altoBaldosa = 32.0f;
    float pies = 0.9f;  // como MarcaSuelo: el centro del rombo, en fraccion del alto
    // AV13-12: en tu turno, las casillas a las que llegas (tinta muy clara) y la
    // ruta hasta la que senala el raton (carmin; la ultima, mas fuerte).
    std::vector<GridCoord> alcance;
    std::vector<GridCoord> ruta;
    // AV13-13: hasta donde llega la carta senalada (un filete carmin en cada casilla a tiro).
    std::vector<GridCoord> tiro;
    // AV13-14: el area de la carta senalada, apuntada (carmin fuerte: lo que pilla, aliados incluidos).
    std::vector<GridCoord> area;

    // Rellena el recuadro desde una funcion de «se pisa».
    void pon(GridCoord o, int w, int h, const std::function<bool(int x, int y)>& sePisa);
    void quita() {
        visible = false;
        alcance.clear();
        ruta.clear();
        tiro.clear();
        area.clear();
    }
    bool esPisable(int x, int y) const;
    // Las aristas del borde: las de cada casilla pisable del recuadro que dan
    // fuera de el, en orden (por filas) para que salgan siempre igual.
    std::vector<AristaSuelo> aristas() const;
};

}  // namespace Pergamino
