// C2-04d — Donde cae un bocadillo sobre la cabeza de un actor, sin GL: de los pies en
// el mundo (px) a la esquina del retal en pantalla. La formula es la de
// BocadillosPnj.cpp, extraida para que la compartan los PNJ y las emociones y se pueda probar.
#pragma once

#include <cmath>

namespace Game {

struct PosicionBocadillo {
    float x = 0.0f, pies = 0.0f;  // pies del actor en pantalla
    float x0 = 0.0f, y0 = 0.0f;   // esquina superior izquierda del retal
    bool visible = false;         // false: el retal cae entero fuera de la vista
};

// `piesX/piesY`: los pies en el mundo; `camara*`/`zoom`: la camara; `ancho/alto`: la vista;
// `w/h`: el tamano del retal. El retal queda 58 px de mundo (por el zoom) + 8 px sobre los pies.
inline PosicionBocadillo posicionBocadillo(float piesX, float piesY, float camaraX, float camaraY, float zoom,
                                           int anchoVista, int altoVista, float w, float h) {
    PosicionBocadillo r;
    r.x = (piesX - camaraX) * zoom + static_cast<float>(anchoVista) * 0.5f;
    r.pies = (piesY - camaraY) * zoom + static_cast<float>(altoVista) * 0.5f;
    r.x0 = std::round(r.x - w * 0.5f);
    r.y0 = std::round(r.pies - 58.0f * zoom - h - 8.0f);
    r.visible = !(r.x0 + w < 0.0f || r.x0 > static_cast<float>(anchoVista) || r.y0 + h < 0.0f ||
                  r.y0 > static_cast<float>(altoVista));
    return r;
}

}  // namespace Game
