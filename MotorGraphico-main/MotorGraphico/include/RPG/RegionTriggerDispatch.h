#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Map/MapDocument.h"
#include "Map/RegionPresence.h"
#include "RPG/EventInterpreter.h"
#include "RPG/MapEventRunner.h"
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
// PERDIDA DE TRIGGERS (contractual): un trigger omitido por escena viva NO
// se reencola ni se entrega despues. Los triggers de region son por
// prioridad (D3), no de entrega garantizada; la presencia ya avanzo.
//
// NOTA StartFailed: es defensivo. Con el id no vacio (validado), el indice
// de pagina tomado de ActivePageWithInventory y `isRunning` mirado antes,
// `EventInterpreter::start` (src/RPG/EventInterpreter.cpp:33-45) no tiene
// otra via de fallo, asi que no hay prueba que lo alcance.
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
    StartFailed,      // el interprete rechazo el arranque
    DeferredToAutomatics  // el frame EMPEZO bloqueado: ver updateEventsAndRegions
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

// Un frame completo: `runner.update(frame)` y despues el despacho. Si al
// empezar hay una escena bloqueante viva, el runner se alimenta SIN mapa
// (setMap(nullptr) y se restaura): avanza el interprete y registra la casilla
// pero no dispara automaticos. Efecto CONTRACTUAL: Autorun y tambien Parallel
// nuevos esperan a que la escena termine y no se pierden (coherente con
// MapEventRunner.h:69-71: mientras algo bloquee no arranca NADA nuevo). El
// mapa se restaura con una guarda RAII, tambien si `update()` lanza.
// SIN INANICION: si el frame empezo bloqueado NO se despachan sus triggers de
// region; van a `skipped` (un registro por trigger, eventId vacio) con motivo
// `DeferredToAutomatics`. Sin esto, un Stay con intervalo menor que su escena
// arrancaria otro justo en el frame en que la escena termina y los automaticos
// pendientes no arrancarian nunca. El frame siguiente empieza sin bloqueo,
// evalua primero los automaticos (`runner.update` con mapa) y despues
// despacha: un Autorun pendiente SIEMPRE gana a un trigger de region en el
// frame en que termina una escena.
// PERDIDA DE Enter/Exit (contractual, D8 abierta): el diferido NO distingue
// si hay automaticos pendientes. Un Enter/Exit (disparo unico) que cae en un
// frame que empezo bloqueado se DESCARTA con `DeferredToAutomatics` aunque
// no haya ningun automatico pendiente, y no se reintenta (la presencia ya
// avanzo). Solo el Stay se regenera mientras dure la presencia en la region.
// Las entradas diferidas llevan `eventId` vacio: no se miro que evento las
// habria arrancado. Sin mapa en el runner no hay despacho.
Result<RegionDispatchResult> updateEventsAndRegions(
    MapEventRunner& runner, EventInterpreter& interpreter, const NarrativeState& narrative,
    const std::vector<std::string>& inventory, const MapEventFrame& frame,
    const std::vector<Regiones::RegionTrigger>& triggers);

}  // namespace RPG
