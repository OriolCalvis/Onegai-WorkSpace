// UI-BG3-01 — El cuaderno de creacion de personaje, pintado en pergamino.
//
// Maquetas aprobadas C02a-C02d (Oriol, 05/10: «m'encanta»): cuero de borde a
// borde con «NUEVA PARTIDA» y el paso, siete pestanas (Q y E en sus teclas),
// la hoja con el contenido de cada pestana y el pie con las teclas y el
// boton CONTINUAR. Raza y Clase son rejillas de retratos de cinco columnas;
// Origen y Deidad, rejillas de iconos; Historia, una matriz de 7 x 5
// tarjetas; Aspecto, las filas del editor con sus flechas; Nombre, el campo,
// las sugerencias, el resumen y «Empezar la aventura». En el centro va el
// personaje (andando y girando) con sus atributos; a la derecha, la ficha de
// lo senalado.
//
// Todo lo que responde al raton queda como Zona en el lienzo, con los tipos
// de Game::CuadernoCreacion::Zona. Sin OpenGL: los retratos son celdas del
// AtlasRetratos que se rellenan montando personajes por capas.
#pragma once

#include "Game/CuadernoCreacion.h"
#include "Game/Personajes/MontajePersonajes.h"  // Oriol (24/10): Game::Reparto
#include "HUD/Pergamino/AtlasRetratos.h"
#include "HUD/Pergamino/Lienzo.h"

#include <string>

namespace Game {
class MontajePersonajes;
class RegistroSprites;
}

namespace Pergamino {

class Pincel;

// Pone los retratos de personajes en el atlas: monta la hoja de 12 frames
// por capas (Game::MontajePersonajes) y guarda el frame pedido. Sin montaje
// no hay retratos (las teselas quedan con su nombre).
class Retratista {
public:
    Retratista(AtlasRetratos& atlas, Game::MontajePersonajes* montaje) : m_atlas(atlas), m_montaje(montaje) {}
    // La region del atlas con el frame `frame` (0..11) de esa vista; Rect
    // vacio si no se puede. PERF-2: con `prioritario` false (las rejillas) no
    // monta si ya se han montado AtlasRetratos::kMontajesPorFotograma en este
    // fotograma: devuelve Rect vacio y se rellena en los siguientes. El
    // personaje central (prioritario) monta siempre y no cuenta en el limite.
    Rect frame(const Game::CuadernoCreacion::Vista& v, int frame, bool prioritario = true);
    // Oriol (24/10): el reparto de las hojas que monta (20 frames o 40 en diagonal).
    Game::Reparto reparto() const;
    std::string aviso(const Game::CuadernoCreacion::Vista& v, int frame) const;
    // UI-BG3-01 — El retrato de un enemigo (su frame de frente, quieto) desde
    // la hoja que le da el registro de sprites (MesaCombateRetales.cpp).
    Rect enemigo(const Game::RegistroSprites& registro, const std::string& objectId);

private:
    AtlasRetratos& m_atlas;
    Game::MontajePersonajes* m_montaje;
};

// La pantalla entera, en la capa alta (tapa el mundo).
void pintaCreacion(Lienzo& l, const Pincel& p, const Game::CuadernoCreacion& c, Retratista& retratos,
                   int anchoVista, int altoVista);

}  // namespace Pergamino
