// AV15-05a (EH-22, EH-23, EH-29, EH-30) — Lo caro e irreversible se confirma.
//
// Una sola pregunta para todo el juego, la del papiro de siempre: un título
// en pixel («¿SALIR DEL JUEGO?»), una línea que dice qué se pierde o qué
// cuesta, y dos filas, «No» primero y con el foco (lo que no se puede
// deshacer no se hace con un ENTER de más). Aquí solo están los textos y las
// reglas de cuándo preguntar; lo pintan el papiro de la pausa y la hoja del
// negocio (HUD/Pergamino/HojaNegocio). GL-free.
//
// Cuándo NO se pregunta: cuando no hay nada que perder. Salir, volver al
// título o cargar no preguntan si la partida está igual que la guardada;
// guardar no pregunta si no hay partida que sobrescribir (o es la misma).
#pragma once

#include <string>

#include "Game/PauseMenu.h"

struct SaveGameData;

namespace Game {

struct Confirmacion {
    std::string titulo;   // pixel x3, en mayúsculas: «¿SALIR DEL JUEGO?»
    std::string detalle;  // una línea: «Perderás lo jugado desde las 12:31.»
    std::string no;       // «No, seguir jugando» (fila 0, con el foco)
    std::string si;       // «Sí, salir sin guardar» (fila 1)
    std::string notaSi;   // bajo el «Sí», en cursiva: lo que cuesta (la hoja del negocio)
    int foco = 0;         // 0 = No, 1 = Sí

    bool vacia() const { return titulo.empty(); }
    bool siElegido() const { return foco == 1; }
    void mueve() { foco = foco == 0 ? 1 : 0; }
};

// La partida guardada con la que se compara la de ahora.
struct UltimaPartida {
    bool hay = false;         // hay una partida que cargar o sobrescribir
    std::string savedAt;      // su fecha («2026-10-14 12:31», como SaveGameData::savedAt)
    bool sinCambios = false;  // la de ahora es igual: no hay nada que perder
};

// Cuándo es una fecha de guardado, dicha para el jugador: «las 12:31» si es
// de hoy, «el 14/09 a las 06:58» si no. `hoy` como NowStamp().
std::string cuandoSeGuardo(const std::string& savedAt, const std::string& hoy);

// La pregunta antes de una acción de la pausa (vacía = no hace falta
// preguntar): Salir, Volver al título, Cargar y Guardar.
Confirmacion preguntaDePausa(PauseAction accion, const UltimaPartida& ultima, const std::string& hoy);

// Las del negocio: comprarlo y revenderlo.
Confirmacion preguntaComprar(const std::string& negocio, int precio, int oro, int renta);
Confirmacion preguntaRevender(const std::string& negocio, int pagaste, int recibes);

// Dos partidas son la misma si lo son en todo menos en la fecha de guardado.
bool mismaPartida(SaveGameData a, SaveGameData b);

}  // namespace Game
