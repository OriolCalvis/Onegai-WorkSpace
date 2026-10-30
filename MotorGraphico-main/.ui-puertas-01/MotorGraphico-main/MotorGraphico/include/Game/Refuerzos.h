// ROSITA-01 — Los que llegan a mitad del combate (RPG::BattleRequest::Refuerzo).
//
// Oriol (13/10, «Boundington: los diálogos»): la antigua compañía de Rosita
// llega «cuando Sandes esté a 1/4 de vida, con las habilidades de Venides y
// Xila gastadas y a punto de morir». Antes llegaban al PERDER el combate (y se
// repetía con ellos); ahora llegan dentro del mismo combate.
//
// Cada refuerzo llega una vez, la primera vez que se cumplen las dos cosas:
//  - su "requires" (como el de un beat: flags, variables...);
//  - el enemigo principal (el primero del grupo, monsterId) ha bajado a
//    `vidaEnemigo` de su vida, o el heroe a `vidaHeroe` (lo primero que pase).
// Al llegar se encienden sus flags, sus lineas van al registro del combate y
// sus acompañantes (cada uno si su siFlag está encendida) se ponen al final
// de tu bando, con las curas que les queden en la aventura. Sin mesa de
// cartas los acompañantes no vienen (como en startBattle), pero las flags y
// las lineas, sí.
//
// GameSession lo llama tras cada accion mientras el combate sigue
// (syncBattleOutcome). Sin GL: se prueba en tests/Game/AcompanantesTests.cpp.
#pragma once

#include "RPG/Narrative/NarrativeBeat.h"

class BattleState;

namespace RPG {
class NarrativeState;
struct AdventureScript;
}  // namespace RPG

namespace Game {

// ¿Le toca ya? (su "requires" y la vida del enemigo principal o del heroe).
bool tocaRefuerzo(const BattleState& batalla, const RPG::BattleRequest::Refuerzo& refuerzo,
                  const RPG::NarrativeState& trama);

// Los que tocan llegan y se quitan de `pedido.refuerzos` (llegan una vez).
// `aventura`: de donde salen las fichas de los acompañantes y sus curas
// (nullptr: no viene nadie, pero se encienden las flags y se anotan las
// lineas). Devuelve cuantos refuerzos han llegado.
int llegaRefuerzos(BattleState& batalla, RPG::BattleRequest& pedido, RPG::NarrativeState& trama,
                   const RPG::AdventureScript* aventura);

}  // namespace Game
