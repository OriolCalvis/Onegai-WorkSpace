#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Game/MenuLogic.h"
#include "Game/SessionTipos.h"      // LevelTransition
#include "Level/LevelDefinition.h"
#include "Level/WorldTopology.h"

namespace Game {

// Un destino de viaje: la puerta que se toma, cómo se llama el sitio al que
// lleva y dónde aterriza el jugador. `label` es el nombre que el autor le dio
// al nivel destino (WorldNode::name); la pieza no lo reescribe ni lo recorta,
// truncar es cosa de quien pinta (HudTextWidgets ya lo hace).
struct TravelDestination {
    std::string objectId;       // la puerta en el nivel actual
    GridCoord doorPosition;     // dónde está la puerta (para señalarla en el mapa)
    std::string label;          // nombre canónico del destino
    std::string levelPath;      // nivel destino, como lo declara la puerta
    GridCoord entryPosition;    // targetPosition de la puerta
    bool hasEntryPosition = false;
    // true si desde el destino se puede volver al nivel actual siguiendo el
    // atlas. false = el viaje encierra al jugador y confirm() no lo toma.
    bool hasReturn = false;
};

// H3-01 (Fase H) — viajar entre naciones: el atlas conectado a LevelTransition.
//
// EL HUECO. El mundo YA es un solo grafo: `tools/validar_mundo_unido.py`
// cuenta 93 niveles y 191 puertas, todo se alcanza y de todo se vuelve, y
// `Application::applyPendingTransition()` (ApplicationBucle.cpp) ejecuta una
// `LevelTransition` en cuanto `GameSession` la expone. Lo que no existía era
// cómo se CRUZA el mar: el mapamundi es un archipiélago y 13 de sus 14
// desembarcos no son alcanzables andando desde `playerStart` — por eso
// `tools/conectividad.py` los reporta como AVISO aplazado a esta tarea desde
// el 19/08. Sin esta pieza el jugador llega al mapamundi y solo puede
// desembarcar en la masa de la que vino.
//
// QUÉ HACE. Convierte las puertas del nivel actual en una lista de destinos
// navegables y, al confirmar, en la `LevelTransition` que el orquestador ya
// sabe ejecutar. Usa `WorldTopology` —que hasta hoy solo consumían
// `demo_mundo` y sus tests— para dos cosas que los datos del nivel no dicen:
// el nombre del destino y si desde él se puede volver.
//
// QUÉ NO HACE. No posee nada (atlas y nivel son referencias de quien lleva la
// partida), no hace I/O, no conoce GL, no decide CUÁNDO se abre el menú de
// viaje ni carga el nivel destino: misma división que `LevelTransition` y que
// `MapEventRunner` — la pieza decide, el de fuera orquesta. Abrirlo es una
// acción del jugador en un mapa de viaje; en un nivel corriente las puertas ya
// se usan andando, y ofrecerlas aquí sería viaje rápido, que es otra decisión.
class WorldTravelState {
public:
    // `levelPath` es la ruta con la que el atlas conoce el nivel actual (la
    // misma que se pasó a WorldTopology::load). Error si el nivel no está en
    // el atlas o si no tiene ninguna salida que el atlas conozca: un menú de
    // viaje vacío no es un menú, es un sitio donde esta acción no aplica.
    static Result<WorldTravelState> open(const WorldTopology& atlas,
                                         const LevelDefinition& level,
                                         const std::string& levelPath);

    // --- Navegación (MenuLogic de producción: wraparound + clamp) ---
    void moveUp();
    void moveDown();
    std::size_t selected() const;

    // open() garantiza que no está vacío: un estado sin destinos es un Error,
    // no un menú (mismo contrato que OptionsState::selectedEntry()).
    const std::vector<TravelDestination>& destinations() const { return m_destinations; }
    const TravelDestination& selectedDestination() const;

    // --- Para el HUD: mismo contrato que OptionsState/PauseMenu ---
    std::vector<std::string> labels() const;
    // `label`, o `label (sin retorno)` cuando el viaje encerraría al jugador.
    std::vector<std::string> displayLines() const;

    // --- El viaje ---
    // LevelTransition con pending=false (es decir: nada) si el destino elegido
    // no tiene vuelta conocida. El destino problemático no se oculta: se ve en
    // displayLines(), se explica en diagnostic() y no se puede tomar.
    LevelTransition confirm() const;

    // --- Diagnóstico ---
    const std::string& levelPath() const { return m_levelPath; }
    // Todo lo que no se ofrece y todo lo que se ofrece con reparos: destinos
    // que el atlas no conoce y destinos sin retorno. No incluye los objetos
    // del nivel que no son puerta: sin el ObjectCatalog no hay forma de
    // distinguir un adorno de una puerta mal declarada, y un diagnóstico con
    // los props de la ciudad dentro no lo lee nadie.
    const std::vector<std::string>& diagnostic() const { return m_diagnostic; }

private:
    std::string m_levelPath;
    std::vector<TravelDestination> m_destinations;
    std::vector<std::string> m_diagnostic;
    MenuLogic m_nav;
};

}  // namespace Game
