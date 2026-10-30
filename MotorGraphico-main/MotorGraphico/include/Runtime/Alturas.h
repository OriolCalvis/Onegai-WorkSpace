// AV19-02 — La altura del terreno, en «niveles naturales» (Oriol: cuevas y rutas de montaña; la ciudad
// no la necesita). Cada casilla tiene un nivel entero 0..kMax; se dibuja kPasoPx píxeles más arriba por
// nivel y se puede pasar de una casilla a otra si la diferencia es de UN nivel como mucho (una rampa
// o unas escaleras son casillas con niveles seguidos: 0,1,2...). Dos niveles de diferencia o más es un
// acantilado: no se pasa.
//
// GL-free y solo cabecera: la usan RuntimeTileMap (guarda la rejilla), el cargador de niveles, la sesión
// (movimiento), el dibujo y las pruebas.
//
// En el JSON del nivel:
//   "alturas": [
//     { "x": 10, "y": 4, "w": 6, "h": 5, "nivel": 2 },                        // una meseta a nivel 2
//     { "x": 10, "y": 9, "w": 6, "h": 3, "nivel": 0, "hasta": 2, "eje": "y" } // rampa: sube de 0 a 2 a lo largo de y
//   ]
// Las regiones se pintan en orden, una sobre otra; lo no pintado es nivel 0. Con «hasta», el nivel va de
// «nivel» (primera fila/columna) a «hasta» (última) repartido por igual, redondeado.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"

struct AlturaRegion {
    int x = 0, y = 0, w = 1, h = 1;
    int nivel = 0;
    bool conRampa = false;  // «hasta» presente
    int hasta = 0;
    char eje = 'y';  // 'x' o 'y': a lo largo de qué eje va la rampa
};

class Alturas {
public:
    static constexpr int kMax = 9;
    // Lo que sube el dibujo por nivel: media baldosa (tileHeight 32 → 16 px).
    static constexpr float kPasoPx = 16.0f;

    // Todo a nivel 0.
    void reinicia(int ancho, int alto) {
        m_ancho = std::max(0, ancho);
        m_alto = std::max(0, alto);
        m_niveles.assign(static_cast<std::size_t>(m_ancho) * static_cast<std::size_t>(m_alto), 0);
        m_hay = false;
    }
    // ¿Hay alguna casilla por encima del nivel 0? (los mapas planos se saltan todo el dibujo en altura)
    bool hay() const { return m_hay; }
    int ancho() const { return m_ancho; }
    int alto() const { return m_alto; }

    // El nivel de una casilla; fuera del mapa, 0.
    int en(int x, int y) const {
        if (x < 0 || y < 0 || x >= m_ancho || y >= m_alto) return 0;
        return m_niveles[static_cast<std::size_t>(y) * static_cast<std::size_t>(m_ancho) + static_cast<std::size_t>(x)];
    }
    int en(const GridCoord& c) const { return en(c.x, c.y); }

    void pon(int x, int y, int nivel) {
        if (x < 0 || y < 0 || x >= m_ancho || y >= m_alto) return;
        nivel = std::max(0, std::min(nivel, kMax));
        m_niveles[static_cast<std::size_t>(y) * static_cast<std::size_t>(m_ancho) + static_cast<std::size_t>(x)] =
            static_cast<std::int8_t>(nivel);
        if (nivel > 0) m_hay = true;
    }

    // Pinta las regiones, en orden, sobre una rejilla a cero.
    void construye(const std::vector<AlturaRegion>& regiones, int ancho, int alto) {
        reinicia(ancho, alto);
        for (const AlturaRegion& r : regiones) {
            for (int j = 0; j < r.h; ++j) {
                for (int i = 0; i < r.w; ++i) {
                    int n = r.nivel;
                    if (r.conRampa) {
                        const int largo = r.eje == 'x' ? r.w : r.h;
                        const int paso = r.eje == 'x' ? i : j;
                        const double t = largo > 1 ? static_cast<double>(paso) / (largo - 1) : 1.0;
                        n = static_cast<int>(std::lround(r.nivel + (r.hasta - r.nivel) * t));
                    }
                    pon(r.x + i, r.y + j, n);
                }
            }
        }
    }

    // La regla de paso: un nivel de diferencia como mucho.
    static bool sePuedePasar(int nivelDe, int nivelA) { return std::abs(nivelDe - nivelA) <= 1; }
    bool sePuedePasar(const GridCoord& de, const GridCoord& a) const { return sePuedePasar(en(de), en(a)); }

    // Cuánto sube el dibujo de una casilla (negativo: hacia arriba en pantalla). Para los niveles.
    static float desfasePx(int nivel) { return -static_cast<float>(nivel) * kPasoPx; }
    float desfasePx(const GridCoord& c) const { return desfasePx(en(c)); }

private:
    int m_ancho = 0, m_alto = 0;
    bool m_hay = false;
    std::vector<std::int8_t> m_niveles;
};
