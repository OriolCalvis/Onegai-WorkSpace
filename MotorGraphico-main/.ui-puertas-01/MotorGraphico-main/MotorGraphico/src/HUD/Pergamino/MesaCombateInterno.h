// UI-BG3-01 — Lo que comparten los ficheros del combate en pergamino
// (MesaCombateHud.cpp: bandeja, carta grande, turno y aviso;
// MesaCombateRetales.cpp: orden de turno, placa del enemigo, registro,
// emergente y retratos de enemigos; MesaCombateEstados.cpp, H2-03: los
// estados). Cabecera interna.
#pragma once

#include <string>
#include <vector>

#include "Game/BarraCombate.h"
#include "HUD/Pergamino/Lienzo.h"
#include "HUD/Pergamino/MesaCombateHud.h"
#include "HUD/Pergamino/Pincel.h"
#include "HUD/Pergamino/Tema.h"

namespace Pergamino::combate {

struct Ctx {
    Lienzo& l;
    const Pincel& p;
    Retratista& r;
};

// Lineas centradas en [x, x + ancho], partidas solo por los espacios.
int centrado(const Ctx& k, Letra letra, const std::string& s, float x, float y, float ancho, float alto,
             int maxLineas, const Vector4& color);
std::string conMayuscula(std::string s);
void tipoDeCarta(RPG::ActionType t, const char*& glifo, Vector4& color, const char*& rotulo);
const char* iconoDeCarta(const Pincel& p, const Game::CartaBarra& c);

// MesaCombateHud.cpp
void pintaBandeja(const Ctx& k, const Game::VistaBarra& v, const Game::BarraCombate& b, const std::string& rival);
void pintaAvisoReaccion(const Ctx& k, const Game::BarraCombate& b);
void pintaTurnoRival(const Ctx& k, const DatosCombate& d);
// MesaCombateRetales.cpp
void pintaOrden(const Ctx& k, const DatosCombate& d);
void pintaPlaca(const Ctx& k, const DatosCombate& d);
void pintaRegistro(const Ctx& k, const DatosCombate& d);
// El emergente va encima de todo, con su velo de vista entera; (dx, dy) es
// donde ha quedado el resto de la pantalla en una vista mayor.
void pintaEmergente(const Ctx& k, const Game::BarraCombate& b, float dx, float dy, int anchoVista, int altoVista);
// MesaCombateEstados.cpp (H2-03): la ficha de un estado (lado 24 en la placa,
// 20 bajo el retrato del heroe) con su zona para el raton (-1 = sin zona).
const char* iconoDeEstado(const Pincel& p, const Game::EstadoVisible& e);
void pintaFicha(const Ctx& k, const Game::EstadoVisible& e, float x, float y, float lado, int zona);
// `soloFichas`: solo las fichas en una fila (la placa compacta de un grupo).
float altoEstadosPlaca(const std::vector<Game::EstadoVisible>& estados, bool soloFichas = false);
void pintaEstadosPlaca(const Ctx& k, const std::vector<Game::EstadoVisible>& estados, int quien, float x,
                       float y, float ancho, bool soloFichas = false);
void pintaEstadosHeroe(const Ctx& k, const std::vector<Game::EstadoVisible>& estados, float x, float y);
// El emergente del estado bajo el raton, junto a su ficha (las zonas desde
// `desdeZona` son las de este combate).
void pintaEmergenteEstado(const Ctx& k, const DatosCombate& d, std::size_t desdeZona);

}  // namespace Pergamino::combate
