// AV16-10 — Transiciones suaves entre suelos (dual-grid). Sin OpenGL.
//
// Los suelos se dibujan como siempre, uno por casilla. Encima, una segunda cuadricula
// desplazada media casilla tiene una celda en cada esquina donde se juntan cuatro suelos:
// mira de que material es cada una de las cuatro y dibuja, para cada material que no sea
// el de abajo, uno de 15 recortes con el borde difuminado (assets/textures/transiciones.png,
// hecho por tools/gen_transiciones.py).
//
// Aqui solo la decision: que capas lleva una celda. El pintado esta en IsometricRenderer.
#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"

namespace Game {

class Transiciones {
public:
    // Un recorte: la fila del material en la hoja (su rango) y la mascara 1..14 de esquinas.
    struct Capa {
        int fila = 0;
        int mascara = 0;
    };

    // Bits de la mascara, en el orden de las esquinas de la celda.
    static constexpr int kArriba = 1, kDerecha = 2, kIzquierda = 4, kAbajo = 8;

    // Lee <carpetaAssets>/textures/transiciones.json. Ok(false) si no existe: sin transiciones.
    Result<bool> carga(const std::string& carpetaAssets);
    bool hay() const { return !m_rango.empty(); }

    int filas() const { return static_cast<int>(m_rango.size()); }
    int columnas() const { return m_columnas; }
    const std::string& imagen() const { return m_imagen; }
    int celdaAncho() const { return m_ancho; }
    int celdaAlto() const { return m_alto; }

    // El rango de un suelo (-1: no tiene transiciones; los interiores, los edificios...).
    int rango(int gid) const;

    // Las capas de una celda, de abajo arriba. Orden de las esquinas: arriba (x,y), derecha
    // (x+1,y), izquierda (x,y+1), abajo (x+1,y+1). Vacio si una esquina no es un suelo con
    // transiciones, o si las cuatro son el mismo.
    std::vector<Capa> capas(const std::array<int, 4>& gids) const;

    // Region de la hoja para el atlas: un id por (fila, mascara).
    static int idRegion(int fila, int mascara) { return fila * 16 + mascara; }

    // Para pruebas: pone la tabla a mano (gid -> rango).
    void pon(const std::map<int, int>& rangos, int columnas = 15) {
        m_rango = rangos;
        m_columnas = columnas;
    }

private:
    std::map<int, int> m_rango;
    int m_columnas = 15, m_ancho = 128, m_alto = 64;
    std::string m_imagen;
};

}  // namespace Game
