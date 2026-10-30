// UI-BG3-01 — El combate en pergamino: la barra de cartas y lo que la rodea.
//
// Maquetas aprobadas C03a-C03c (Oriol, 05/10: el HUD de combate era «una
// lista de la compra, no un HUD de un videojuego»):
//   - abajo, la bandeja de cuero con las basicas (Atacar 1, Pocion 2, Huir 3)
//     y la mano (4-0), las de creador apartadas en su hueco y «F Pasar»;
//   - a la izquierda, la carta senalada en grande (tipo, que hace, a quien,
//     como se defiende el rival, si cierra el turno) y, encima de la bandeja,
//     lo que queda del turno: accion, movimiento, canaliza y reaccion;
//   - arriba, el orden de turno con la ronda, la placa del enemigo con su
//     vida y sus estados y el registro (L lo esconde);
//   - el emergente para elegir (la estacion del Bardo, la opcion de una carta)
//     y el aviso de la reaccion que salta en el turno enemigo.
// Lo que responde al raton queda como Zona con los tipos de
// Game::BarraCombate::Zona. Sin OpenGL.
#pragma once

#include <string>
#include <vector>

#include "Game/BarraCombate.h"
#include "Game/CuadernoCreacion.h"
#include "Game/OrdenDeTurno.h"
#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/PaginasCreacion.h"

namespace Game {
class RegistroSprites;
}

namespace Pergamino {

class Pincel;

struct EnemigoEnMesa {
    std::string nombre;
    std::string objectId;  // para su retrato (RegistroSprites)
    int pv = 0, pvMax = 0;
    bool vivo = true;
    // Sus estados vienen en la vista (VistaBarra::estadosEnemigos, H2-03).
};

// ACOMPANANTES-01 — Quien va con el heroe (Venides, Xila, una invocacion):
// sale en el orden de turno detras de el, con su vida o, si cura, las curas
// que le quedan.
struct AliadoEnMesa {
    std::string nombre;
    std::string objectId;  // su retrato: el id del PNJ
    int pv = 0, pvMax = 0;
    int curas = -1;        // las que le quedan (-1 = no cura)
    bool vivo = true;
};

// Los aliados de la batalla menos el heroe, en su orden.
std::vector<AliadoEnMesa> aliadosEnMesa(const BattleState& batalla);

struct DatosCombate {
    const Game::VistaBarra* vista = nullptr;
    const Game::BarraCombate* barra = nullptr;
    std::string heroe;
    const Game::CuadernoCreacion::Vista* retratoHeroe = nullptr;  // su aspecto, si lo hay
    std::vector<AliadoEnMesa> aliados;  // ACOMPANANTES-01
    std::vector<EnemigoEnMesa> enemigos;
    std::vector<std::string> registro;  // el log del combate, el ultimo al final
    const Game::RegistroSprites* sprites = nullptr;
    // AV13-11 — El orden de la iniciativa (vacio: el de siempre) y a quien le toca.
    std::vector<Game::Puesto> orden;
    std::size_t enTurno = 0;
};

void pintaCombate(Lienzo& l, const Pincel& p, const DatosCombate& d, Retratista& retratos, int anchoVista,
                  int altoVista);

}  // namespace Pergamino
