// AV13-08 — Geometria pura de las areas que se previsualizan en el combate.
// No conoce bandos, ocupantes ni GL: devuelve todas las casillas afectadas.
#pragma once

#include <functional>
#include <vector>

#include "Core/Math/GridCoord.h"

namespace Game {

struct PlantillaArea {
    enum class Forma { Circulo, Cono, Linea, Cuadrado };

    Forma forma = Forma::Circulo;
    // Radio euclideo del circulo/cono; numero de casillas de la linea;
    // lado del cuadrado. Negativo: area vacia. Circulo de radio cero: origen.
    int tamano = 1;
};

// Orden de lectura (y, luego x), sin repetidas; no recorta al nivel.
// Circulo/cuadrado incluyen el origen. Cono/linea lo excluyen.
// Cono: 90 grados, bordes incluidos; el signo de haciaDonde-origen elige
// una de las ocho direcciones. Linea: Bresenham de N pasos hacia haciaDonde,
// prolongada si el destino esta cerca. Sin direccion: cono/linea vacios.
// Cuadrado par: el centro geometrico queda media casilla arriba/izquierda
// del origen (inicio = origen - lado/2). Lado cero: vacio.
// bloquea opcional: misma vista que TableroCombate::seVen, sin consultar
// los extremos. Un muro puede estar en el area, pero oculta lo que hay detras.
std::vector<GridCoord> casillas(const PlantillaArea& plantilla, GridCoord origen,
                                GridCoord haciaDonde,
                                const std::function<bool(int x, int y)>& bloquea = {});

}  // namespace Game
