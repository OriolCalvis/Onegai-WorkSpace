#pragma once
// D0027, D0030 — La foto del inicio de un combate (de su primera oleada): lo que hace falta para que, si el
// heroe muere, el combate vuelva a empezar tal como empezo. GL-free; la toma y la devuelve
// GameSession (GameSessionRevivir.cpp).

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "Core/Math/GridCoord.h"
#include "Game/Cicatrices.h"
#include "Game/SaveGame.h"
#include "RPG/NarrativeEngine.h"
#include "RPG/Narrative/AdventureScript.h"
#include "World/World.h"

class ICombatant;
namespace RPG { class RandomEngine; }

namespace Game {

// CAER-01 — Sube la vida del heroe a un cuarto del maximo (1 como poco) y devuelve la que tiene.
int levantaHeroe(ICombatant& heroe);

// D0030 — Al huir se tira un dado por lo que se cae en la carrera: con un 1, el 30 % del oro; con un 2,
// el 20 %; con un 3, el 10 %; con 4 o mas, nada. Sin oro no hay tirada. Devuelve la linea del registro.
std::string pagaLaHuida(int& oro, RPG::RandomEngine& dado);

struct FotoCombate {
    // Quienes pelean y a quien se llevaba a su lado: con esto se vuelve a montar el mismo combate.
    std::vector<Engine::EntityHandle> handles;
    std::vector<RPG::AcompananteDef> aLado;

    // El heroe: vida, mana, mochila, oro y casilla.
    int vida = 0;
    int mana = 0;
    std::vector<std::string> mochila;
    int oro = 0;
    GridCoord casilla;

    // Cada enemigo: vida, mana y casilla (los handles de arriba, en el mismo orden).
    struct Enemigo {
        int vida = 0;
        int mana = 0;
        GridCoord casilla;
    };
    std::vector<Enemigo> enemigos;

    // La historia: el pedido en curso (con los refuerzos que le quedaban) y las flags de entonces
    // (un refuerzo enciende las suyas a mitad del combate). La cola de oleadas no se toca mientras
    // se pelea, asi que basta con apuntarla al ganar la primera oleada (cola). Las variables, porque
    // al cerrar una oleada se apuntan las curas gastadas de los acompanantes.
    std::optional<RPG::BattleRequest> pedido;
    std::vector<std::string> flags;
    std::map<std::string, int> variables;
    std::vector<RPG::BattleRequest> cola;
    SaveGameData guardado;  // D0033: la partida tal como estaba; es lo que guarda F5 en combate
    bool oleadaGanada = false;  // ya se gano alguna: sus enemigos no existen y hay que volver a invocarlos
};

// Lo de la sesion que tiene que ver con morir en combate: la foto del inicio del combate, cuantas
// veces ha caido el heroe en esta sesion y sus cicatrices (con el fichero donde se guardan).
struct Muertes {
    std::optional<FotoCombate> foto;
    bool sigueLaOleada = false;  // la oleada que empieza sigue a una ganada: la foto no cambia
    int n = 0;
    Cicatrices cicatrices;
    std::string ruta;
};

}  // namespace Game
