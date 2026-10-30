// AV13-07 — La ruta por casillas y hasta donde se llega con un presupuesto.
//
// Para moverse en el combate por casillas (COMBATE_BG3.md §3: «la ruta se ve
// antes de andar: las casillas que alcanzas, la ruta con lo que gasta») y para
// el clic de raton fuera de el (AV7-08). Cuatro vecinos, como tryMovePlayer.
// Dijkstra dentro de un recuadro (los tableros son de 11x11: no hace falta mas);
// el coste de cada casilla lo da un callback (1 por defecto; 2 en terreno
// dificil, cuando lo haya). GL-free y sin el nivel dentro.
//
// Desempate fijo (para que la misma pregunta de siempre la misma ruta): a igual
// coste, gana la casilla que se visito antes, y los vecinos se miran en el
// orden derecha, abajo, izquierda, arriba.
#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "Core/Math/GridCoord.h"

namespace Game {

class RutaCasillas {
public:
    // true si se puede pisar (y no hay nadie). La casilla de salida no se mira.
    using Pisa = std::function<bool(int x, int y)>;
    // Lo que cuesta entrar en la casilla (>= 1). Vacio: todo cuesta 1.
    using Coste = std::function<int(int x, int y)>;

    // Lo que se SUMA al entrar de una casilla a otra vecina (>= 0): la subida de
    // un nivel de altura (AV19-02) cuesta el doble, es decir, +1.
    using Extra = std::function<int(GridCoord de, GridCoord a)>;

    struct Alcanzable {
        GridCoord casilla;
        int coste = 0;
    };

    RutaCasillas(GridCoord origen, int ancho, int alto, Pisa pisa, Coste coste = {});

    void ponExtra(Extra extra) { m_extra = std::move(extra); }

    // Las casillas desde `desde` (sin ella) hasta `hasta` (incluida). Vacia si
    // no se llega o si `hasta` == `desde`.
    std::vector<GridCoord> ruta(GridCoord desde, GridCoord hasta) const;
    // Las casillas a las que se llega gastando como mucho `presupuesto` (sin la
    // de salida), con lo que cuesta llegar a cada una.
    std::vector<Alcanzable> alcanzables(GridCoord desde, int presupuesto) const;
    // Lo que cuesta ir de `desde` a `hasta` (-1 si no se llega).
    int coste(GridCoord desde, GridCoord hasta) const;

private:
    struct Mapa;  // costes y de donde se viene, por casilla del recuadro
    Mapa explora(GridCoord desde, int presupuesto) const;
    bool dentro(GridCoord c) const;
    int indice(GridCoord c) const;

    GridCoord m_origen;
    int m_ancho, m_alto;
    Pisa m_pisa;
    Coste m_coste;
    Extra m_extra;
};

}  // namespace Game
