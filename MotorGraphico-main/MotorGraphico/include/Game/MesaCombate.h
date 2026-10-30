// H2-06 — Lo que BattleState le pregunta a «la mesa» donde el heroe juega sus
// cartas de clase.
//
// BattleState resuelve el combate de siempre (ataque basico, habilidades del
// catalogo legado, pociones, huir). Las cartas de clase hacen cosas que ese
// camino no sabe hacer: estados que dan ventaja o desventaja, escudos,
// reacciones que bloquean un golpe, invocaciones... Cuando el heroe tiene
// ficha con cartas, GameSession le pone a la batalla una mesa
// (Game::CartasEnCombate) y BattleState le cede los turnos: si la mesa dice
// que ha resuelto la accion, BattleState solo comprueba el desenlace.
//
// Es una interfaz pura a proposito: BattleState.cpp se compila suelto en
// test_g13_combat_commands y no puede depender de la implementacion.
#pragma once

#include <cstddef>
#include "Core/Math/GridCoord.h"
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/TiradaVisible.h"

struct BattleAction;
struct BattleParticipant;
class SkillCatalog;
namespace RPG {
class RandomEngine;
}

namespace Game {

// Lo que la mesa toca de la batalla, por referencia: los dos bandos, el log,
// el dado de la batalla y los catalogos que la batalla ya tenia.
struct ContextoBatalla {
    std::vector<BattleParticipant>& aliados;
    std::vector<BattleParticipant>& enemigos;
    std::vector<std::string>& log;
    RPG::RandomEngine& rng;
    SkillCatalog* habilidadesLegado;
    const RPG::Catalogs::ConditionCatalog* condiciones;
    // AV13-12 — Con tablero: si el enemigo e llega al aliado a (al lado, o a tiro si lanza).
    // Vacio (sin tablero, o en las pruebas de siempre): llega a todos.
    std::function<bool(std::size_t e, std::size_t a)> llega = {};
    // AV13-29 — Con tablero: si el aliado a tiene al enemigo e al lado (para su golpe cuerpo a
    // cuerpo). Vacio: a todos.
    std::function<bool(std::size_t a, std::size_t e)> alLado = {};
    // AV13-14 — Con tablero: la casilla de cada uno (aliado o enemigo, por indice), para las areas.
    std::function<std::optional<GridCoord>(bool aliado, std::size_t i)> casillaDe = {};
    std::function<bool(int x, int y)> muro = {};  // AV13-14: lo que corta un area (sin tablero, nada)
};

class IMesaCombate {
public:
    virtual ~IMesaCombate() = default;

    // La accion del aliado. true = resuelta aqui; false = sigue BattleState
    // (pocion y huir, o un aliado que no es el heroe).
    virtual bool resolverAliado(ContextoBatalla& c, std::size_t aliado, const BattleAction& a) = 0;
    // El turno de un enemigo (vivo). true = resuelto aqui.
    virtual bool resolverEnemigo(ContextoBatalla& c, std::size_t enemigo) = 0;
    // AV14-01 — Sus tiradas recientes (la mas nueva al final), para dar su grado a
    // cada golpe. nullptr: la mesa no tira a la vista.
    virtual const std::vector<RPG::TiradaVisible>* tiradasVisibles() const { return nullptr; }
    // AV13-29 — Los acompanantes con su turno: si al aliado lo juega el jugador (el heroe, siempre),
    // y el turno de un acompanante que va solo (su IA de siempre). false: la mesa no sabe de el.
    virtual bool juegaElJugador(const ContextoBatalla& c, std::size_t aliado) const { (void)c; return aliado == 0; }
    virtual bool actuaSolo(ContextoBatalla& c, std::size_t aliado) { (void)c; (void)aliado; return false; }
    // Fin de ronda: dano por turno, duraciones, invocaciones que se acaban.
    virtual void finDeRonda(ContextoBatalla& c) = 0;

    // --- Para el menu de combate (Application) ---
    // Las filas que van detras de Atacar / Pocion / Huir.
    virtual std::vector<std::string> filas() const = 0;
    // La accion que corresponde a una de esas filas (UseSkill con un id que
    // solo entiende la mesa). false si la fila no existe o esta bloqueada.
    virtual bool accionDeFila(std::size_t fila, BattleAction& accion) const = 0;
    // Texto de la carta de la fila (lo que dice y por que no se puede jugar).
    virtual std::string detalle(std::size_t fila) const = 0;
    // true si la ultima accion del heroe no cerro su turno (una accion libre,
    // una reaccion preparada, una canalizacion o un turno extra).
    virtual bool turnoAbierto() const = 0;
};

}  // namespace Game
