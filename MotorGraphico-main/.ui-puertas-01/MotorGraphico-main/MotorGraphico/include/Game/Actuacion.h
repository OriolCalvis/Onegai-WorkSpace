// ACTUAR-01 — Actuar en la calle.
//
// Oriol (08/10/2026): «podamos hacer una performance en la calle y depende de
// nuestra competencia musical y tiradas, atraer a la gente a nuestro
// espectaculo».
//
// Un sitio para actuar es un objeto del catalogo con su bloque "actuacion"
// (PerformanceData, en Level/ObjectCatalog.h), como el cartel de negocio. Al
// interactuar con el (GameSession::interact), el heroe actua: una tirada
// enfrentada de Interpretacion (CAR, la de la carta «interpretacion») contra
// la indiferencia de los que pasan (los dados del sitio). Lo que lleva en la
// mano principal suma dados si sirve para actuar (la etiqueta «Instrumento» o
// «Malabares»): +1 si es comun, +2 si es raro y +3 si es epico o mejor.
//
// El grado dice cuanta gente se para y cuanto echa:
//   perder (o pifiar)  -> nadie se para (con la pifia, ademas, se rien);
//   empate (parcial)   -> un cuarto de los que pasan: 1 de oro cada uno;
//   exito              -> la mitad: 2 de oro cada uno;
//   critico            -> todos: 3 de oro cada uno.
// (y, del parcial para arriba, unos pocos mas, al azar).
//
// PO-49: con la cuenta de dias en marcha, se actua una vez por sitio y DIA de la
// historia. Sin dias (un nivel suelto) vale lo de abajo.
// Luego, los de ese sitio ya te han visto: hasta que no andes kPasosDeDescanso
// pasos (el tiempo del juego son los pasos del jugador, ver SessionData), alli
// no se vuelve a actuar. El reloj de pasos y el ultimo paso en que se actuo en
// cada sitio son variables del estado narrativo: se guardan con la partida.
//
// GL-free. Lo llama GameSession (interact y movePlayer); GameSession.h esta en
// su techo de lineas, asi que todo vive aqui.
#pragma once

#include <string>
#include <vector>

#include "RPG/DicePoolEngine.h"

struct SessionData;
struct ObjectDefinition;
struct PerformanceData;

namespace RPG {
class NarrativeState;
class RandomEngine;
struct EquipmentDefinition;
}  // namespace RPG

namespace Game {

// Los pasos que hay que andar para volver a actuar en el mismo sitio.
constexpr int kPasosDeDescanso = 120;

// Con que actua: lo de la mano principal, si sirve para actuar.
struct ConQueActuas {
    std::string nombre;  // con su posesivo y en minusculas: «tu lira»; vacio = a capela
    int dadosExtra = 0;
};
ConQueActuas conQueActuas(const RPG::EquipmentDefinition* manoPrincipal);

// Lo que pasa al actuar.
struct Actuacion {
    bool descansando = false;  // aqui ya te han visto: ni tirada ni oro
    int pasosQueFaltan = 0;    // si descansando, cuanto queda
    int dados = 0;             // los del heroe: su CAR y lo que suma la mano
    int rivalDice = 0;
    RPG::PoolResult tirada;
    RPG::PoolResult rival;
    RPG::Degree grado = RPG::Degree::BOTCH;
    int publico = 0;  // cuantos se paran a mirar
    int oro = 0;      // lo que echan
    std::vector<std::string> lineas;
};

// La tirada y lo que pasa, sin tocar nada (la prueban los tests).
Actuacion actua(const PerformanceData& sitio, int carisma, const ConQueActuas& con, RPG::RandomEngine& rng);

// El reloj: un paso mas (GameSession::movePlayer).
void cuentaUnPaso(RPG::NarrativeState& estado);
// PO-49: ¿ya actuaste en este sitio en el dia de la historia en que estamos?
bool yaActuasteHoy(const RPG::NarrativeState& estado, const std::string& sitioId);
// Los pasos que le faltan a un sitio para volver a actuar en el (0 = ya se puede).
int pasosQueFaltan(const RPG::NarrativeState& estado, const std::string& sitioId);

// Todo junto, desde la sesion: la carisma de la ficha, lo que lleva en la
// mano, la tirada, el descanso del sitio, la foto de la tirada para la hoja
// del dialogo y el dialogo puesto. El oro lo suma quien llama (addGold).
Actuacion actuaEn(SessionData& sesion, const ObjectDefinition& sitio);

}  // namespace Game
