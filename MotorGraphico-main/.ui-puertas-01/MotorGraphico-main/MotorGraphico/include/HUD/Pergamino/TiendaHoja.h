// TIENDA-02 — La tienda en pergamino.
//
// Era el panel oscuro de antes del tema (TIENDA-01): una sola lista con lo
// que se compra y, debajo, lo que se vende, y el resultado en el registro.
// Ahora es la plantilla T2 del modelo visual (lista y detalle, M06) en el
// material del tema que le toca (S3: «Hoja: diálogos, fichas y tienda»):
//
//   - cabecera: TIENDA en píxel x3, el tendero en Lora y tu oro;
//   - pestañas Comprar y Vender (E29: la venta ya no se mete en la lista);
//   - la página de la pestaña: la lista con el precio y el detalle de la
//     fila con foco, con su botón («COMPRAR 1 · 30 ORO», «VENDER 1 ·
//     RECIBES 15», E30) y, si no llega el oro, por qué;
//   - la línea de estado sobre el pie, con lo que acaba de pasar (E18);
//   - el pie con las teclas.
//
// Datos y dibujo, sin OpenGL ni estado: el modelo es Game::TiendaEstado y
// la partida, GameSession. Lo dibuja Render/PergaminoGL, como el resto.
#pragma once

#include <string>

#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/PaginaCuaderno.h"

class GameSession;

namespace Game {
class TiendaEstado;
}

namespace Pergamino {

class Pincel;

// Lo que responde al ratón en la tienda (además de las filas y el botón de
// la página, kZonaFilaPagina y kZonaBotonPagina). Índice: 0 Comprar, 1 Vender.
constexpr int kZonaPestanaTienda = 1400;

struct DatosTienda {
    std::string tendero;
    int oro = 0;
    int pestana = 0;  // 0 Comprar, 1 Vender
    PaginaCuaderno pagina;
    std::string estado;  // la última compra o venta («Comprado: Poción · –30 de oro · ahora tienes 3»)
    // AV14-23: lo que la facción del tendero piensa de ti y lo que cambia el
    // precio («Aegroum: te aprecian (+12). Pagas un 9 % menos.»). Vacía = nada.
    std::string reputacion;
    bool estadoError = false;
};

// La hoja de la tienda, a partir del modelo y de la partida.
DatosTienda datosTienda(const Game::TiendaEstado& tienda, const GameSession& sesion);

// Dónde cae la hoja en la vista: 940 x 600, centrada.
Rect rectTienda(int anchoVista, int altoVista);

// El velo sobre el mundo, la hoja, la cabecera, las pestañas, la página de
// la pestaña activa, la línea de estado y el pie.
void pintaTienda(Lienzo& l, const Pincel& p, const DatosTienda& d, int anchoVista, int altoVista);

}  // namespace Pergamino
