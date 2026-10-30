#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Map/MapDocument.h"
#include "Map/RegionPresence.h"
#include "RPG/EventInterpreter.h"
#include "RPG/SessionCommandHost.h"

// U3-02.E06b — Despacho de los triggers de REGION (RegionEnter/Stay/Exit) al
// intérprete de eventos. Contrato: planificacion/pla_unity/descomposicio/
// contratos/U3-02_regiones.md §2.4.
//
// POR QUE NO ESTA EN MapEventRunner: el runner ya incumple la ley 1b y no se
// amplia. Este despachador usa SOLO API publica: la misma regla de seleccion
// de pagina que el runner (`ActivePageWithInventory`, E-09 —una sola regla,
// no una copia—) y `EventInterpreter::start`.
//
// COMO ELIGE LA PAGINA: para cada trigger, y en orden de capa y de evento, se
// toma la PAGINA ACTIVA del evento (la que gana por condiciones). Arranca solo
// si esa pagina tiene el trigger de region equivalente al `kind` y escucha
// exactamente esa `RegionRef` (regionLayerId + regionId, D1 = b). Una pagina
// activa que no es de region, o que escucha otra region, no cuenta: no se
// busca "otra pagina del mismo evento que si case", igual que el runner no
// busca otra pagina cuando la activa no es Touch. Una condicion no evaluable
// NO es falsa: se reporta (`NotEvaluable`) y no se arranca.
//
// EXCLUSION CON ESCENAS BLOQUEANTES: nunca arranca una segunda escena que
// bloquee. Antes de CADA arranque consulta `blocksPlayer()`: si algo bloquea
// (una escena viva, o una que acaba de arrancar este mismo despacho), el
// trigger NO arranca y queda en `skipped` con motivo `BlockedByScene`. El
// trigger omitido NO se reencola: la presencia ya avanzo, asi que un Enter
// omitido se pierde (decision de quien llama si lo reintenta o lo descarta).
//
// ORDEN RESPECTO A MapEventRunner::update (documentado, no forzado): llamar
// DESPUES de `update()` en el mismo frame. Asi, un Autorun que arranca en
// `update()` ya bloquea y el trigger de region cede ante el. En el orden
// inverso la escena de region arrancaria primero y `update()` arrancaria el
// Autorun ENCIMA, porque `dispararAutomaticos()` no mira `blocksPlayer()`
// (hueco del runner, contrato §1.4, no corregido aqui).
namespace RPG {

enum class RegionSkipReason {
    BlockedByScene,   // otra escena bloqueante viva: no se arranca una segunda
    AlreadyRunning,   // ese evento ya se esta ejecutando
    NotEvaluable,     // la condicion de la pagina no se pudo evaluar
    StartFailed       // el interprete rechazo el arranque
};

struct RegionStarted {
    std::string eventId;
    std::size_t pageIndex = 0;
    Regiones::RegionTrigger trigger;
};

struct RegionSkipped {
    std::string eventId;
    Regiones::RegionTrigger trigger;
    RegionSkipReason reason = RegionSkipReason::BlockedByScene;
    std::string detail;
};

struct RegionDispatchResult {
    std::vector<RegionStarted> started;   // en el orden de despacho
    std::vector<RegionSkipped> skipped;
};

// `triggers` ya viene ordenado (orderRegionEvents) y se despacha tal cual, en
// ese orden. Error solo si el mapa no es utilizable por argumentos; los
// fallos por evento van en `skipped`.
Result<RegionDispatchResult> dispatchRegionTriggers(
    const std::vector<Regiones::RegionTrigger>& triggers, const MapDocument& map,
    const NarrativeState& narrative, const std::vector<std::string>& inventory,
    EventInterpreter& interpreter);

}  // namespace RPG
