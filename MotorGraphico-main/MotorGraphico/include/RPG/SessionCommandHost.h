#pragma once

#include <string>
#include <vector>

#include "Game/GameSession.h"
#include "RPG/EventCommand.h"

// E-04 (segunda rebanada) — Las familias que necesitan la partida.
//
// POR QUÉ ES UN FICHERO NUEVO Y NO UN CAMBIO EN GameSession. El punto de
// costura lo dejó preparado la primera rebanada: NarrativeCommandHost
// resuelve las familias de estado narrativo y delega el resto en un
// `fallback`. Esto ES ese fallback. No hace falta tocar
// src/Game/GameSession.cpp, que comparten seis paquetes (C-04, C-17,
// D-08, E-04, G-12, G-14) y que es de los sitios donde dos agentes se
// pisan sin enterarse — ver planificacion/MAPA_DEL_CODIGO.md.
//
// Todo lo que hace falta ya es público en GameSession: addGold(), gold(),
// y el inventario, que el propio llamante le pasó por puntero al
// construirla y por tanto ya tiene.
//
// CAMBIAR DE MAPA NO SE HACE AQUÍ, Y NO ES UNA LIMITACIÓN. GameSession no
// carga niveles a propósito —no hace I/O ni conoce rutas de assets— y
// expone la transición pendiente para que quien orquesta la ejecute. Este
// anfitrión hace exactamente lo mismo: anota la transición pedida y la
// expone. La sesión decide, el de fuera orquesta; es la división que ya
// usan LevelTransition y BattleState.

namespace RPG {

class SessionCommandHost : public EventHost {
public:
    // El inventario es el MISMO vector que se le pasó a la sesión al
    // construirla: el llamante ya lo tiene, así que no hace falta abrir
    // un accesor nuevo en GameSession para llegar a él.
    SessionCommandHost(GameSession& session, std::vector<std::string>& inventory)
        : m_session(session), m_inventory(inventory) {}

    CommandResult execute(const MapEventCommand& command) override;
    bool waitResolved(const MapEventCommand& command) override;

    // Desenlace del ultimo combate lanzado por un evento. Se conserva
    // despues de que la sesion cierre el combate, porque `control.segun_
    // desenlace` lo lee en el comando SIGUIENTE, cuando battle() ya es
    // nullptr.
    BattleOutcome lastBattleOutcome() const { return m_lastOutcome; }

    // La transición que ha pedido un `mapa.ir`, si la hay. Quien lleva el
    // bucle la lee, reconstruye la sesión sobre el nivel nuevo y la
    // limpia.
    const LevelTransition& requestedTransition() const { return m_transition; }
    void clearRequestedTransition() { m_transition = LevelTransition{}; }

private:
    GameSession& m_session;
    std::vector<std::string>& m_inventory;
    LevelTransition m_transition;
    BattleOutcome m_lastOutcome = BattleOutcome::InProgress;
};

// La condición de inventario que la primera rebanada no podía evaluar.
//
// Sigue devolviendo Error cuando no hay inventario que mirar, en vez de
// falso: «no tiene la llave» y «no sé mirar el inventario desde aquí» son
// cosas distintas, y confundirlas convierte una pieza que falta en una
// puerta que nunca se abre y que nadie sabe por qué.
Result<bool> EvaluateConditionWithInventory(const MapEventCondition& condition,
                                            const NarrativeState& state,
                                            const std::vector<std::string>& inventory);

// Solo la parte de inventario, sin estado narrativo de por medio. Existe
// aparte porque hay un sitio —el `control.si` que el anfitrión narrativo
// delega— donde la condición SIEMPRE es de objeto y no hay estado que
// pasar: inventarse uno vacío para cumplir la firma seria una trampa
// esperando a que alguien delegue una bandera por error.
Result<bool> EvaluateItemCondition(const MapEventCondition& condition,
                                   const std::vector<std::string>& inventory);

// E-09 — Página activa de un evento mirando TAMBIÉN el inventario.
//
// Es la que usa el juego: una puerta que pide la llave tiene su página
// «abierta» condicionada a llevarla, y sin inventario esa condición no se
// puede evaluar. La regla (gana la última que se cumple) y la combinación
// AND/OR viven en E-04; aquí solo se cambia el evaluador, para que no
// haya dos versiones de la misma regla.
ActivePageResult ActivePageWithInventory(const MapEventDocument& event,
                                         const NarrativeState& state,
                                         const std::vector<std::string>& inventory);

}  // namespace RPG
