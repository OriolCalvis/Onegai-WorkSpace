// PERGAMINO-01 — El papiro: menus de una sola pagina (pausa, opciones y
// derrota).
//
// Material «papiro» del tema: hoja con rodillos de madera y remates de
// laton, sin animacion de pagina (§13.1). La maqueta aprobada es P05a
// (pausa con el guardado confirmado junto a la accion, MODELO_VISUAL.md
// E17). Mismas reglas que el resto: un solo foco (la cinta carmin), el
// mundo detras de un velo, verbos concretos en el pie.
//
// Datos y dibujo, sin OpenGL. Que entradas hay y cual tiene el foco lo
// deciden los modelos del juego (PauseMenu, OptionsState, GameOverScreen);
// aqui solo se les pone nombre en castellano y se pintan.
#pragma once

#include <string>
#include <vector>

#include "HUD/Pergamino/Lienzo.h"

namespace Pergamino {

class Pincel;

struct EntradaPapiro {
    std::string texto;
    bool separadorAntes = false;  // filete entre grupos (Volver al titulo / Salir)
    // Fila de ajuste (opciones): valor a la derecha. Si deslizador >= 0, se
    // pinta una pista de 11 marcas con ese valor (0..10).
    std::string valor;
    int deslizador = -1;
    // UI-BG3-01 — Una entrada apagada se lee en tinta desvaida y lleva su
    // motivo debajo, en cursiva («Continuar · No hay ninguna partida
    // guardada»): se puede senalar, no ejecutar.
    bool apagada = false;
    std::string motivo;
};

// UI-BG3-01 — Lo que responde al raton en un papiro: cada entrada (indice =
// su posicion en DatosPapiro::entradas).
constexpr int kZonaEntradaPapiro = 1;

struct AyudaPapiro {
    std::vector<std::string> teclas;
    std::string verbo;
};

struct DatosPapiro {
    float ancho = 480.0f;           // P05a; las opciones lo piden mas ancho (su pie)
    std::string titulo;             // pixel x3: EN PAUSA, OPCIONES, HAS CAÍDO
    std::string subtitulo;          // una linea bajo el titulo (derrota)
    std::vector<EntradaPapiro> entradas;
    int foco = 0;
    // Linea de estado: el resultado de la ultima accion de esta ventana
    // (Partida guardada · zona · hora), con su sello.
    std::string estado, estadoDetalle;
    bool estadoError = false;
    std::vector<AyudaPapiro> pie;
    // UI-BG3-01 — El titulo del juego pone el papiro mas abajo y sin velo
    // (el nombre del juego va encima, sobre la ciudad). arriba < 0 = centrado.
    float arriba = -1.0f;
    bool velo = true;
    std::string notaPie;  // una linea pequena bajo las teclas («o haz clic con el raton»)
};

// Alto del papiro para estas entradas (el ancho va en los datos). Con el
// pincel cuenta tambien las filas de mas del pie cuando no cabe en una.
float altoPapiro(const DatosPapiro& d, const Pincel* p = nullptr);
// Velo sobre el mundo + papiro centrado en la vista.
void pintaPapiro(Lienzo& l, const Pincel& p, const DatosPapiro& d, int anchoVista, int altoVista);

}  // namespace Pergamino
