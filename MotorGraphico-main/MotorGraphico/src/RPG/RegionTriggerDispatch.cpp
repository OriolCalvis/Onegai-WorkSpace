// U3-02.E06b — El contrato y las decisiones estan en
// include/RPG/RegionTriggerDispatch.h; aqui solo esta el como.

#include "RPG/RegionTriggerDispatch.h"

#include <variant>

namespace RPG {
namespace {

EventTrigger triggerDe(Regiones::RegionTrigger::Kind kind) {
    switch (kind) {
        case Regiones::RegionTrigger::Kind::Enter:
            return EventTrigger::RegionEnter;
        case Regiones::RegionTrigger::Kind::Stay:
            return EventTrigger::RegionStay;
        case Regiones::RegionTrigger::Kind::Exit:
            return EventTrigger::RegionExit;
    }
    return EventTrigger::RegionEnter;
}

void omitir(RegionDispatchResult& out, const MapEventDocument& event,
            const Regiones::RegionTrigger& trigger, RegionSkipReason reason,
            std::string detail) {
    out.skipped.push_back(RegionSkipped{event.id.value(), trigger, reason, std::move(detail)});
}

void despacharAEvento(RegionDispatchResult& out, const MapEventDocument& event,
                      const Regiones::RegionTrigger& trigger, const NarrativeState& narrative,
                      const std::vector<std::string>& inventory, EventInterpreter& interpreter) {
    const ActivePageResult activa = ActivePageWithInventory(event, narrative, inventory);
    if (!activa.trustworthy()) {
        omitir(out, event, trigger, RegionSkipReason::NotEvaluable, activa.diagnostic);
        return;
    }
    const MapEventPage* pagina = activa.page;
    if (pagina == nullptr || pagina->trigger != triggerDe(trigger.kind) ||
        pagina->regionLayerId != trigger.ref.capa || pagina->regionId != trigger.ref.region) {
        return;
    }
    // Ley de una sola escena bloqueante: se mira ANTES de cada arranque, de
    // modo que lo arrancado antes en este mismo despacho tambien cuenta.
    if (interpreter.blocksPlayer()) {
        omitir(out, event, trigger, RegionSkipReason::BlockedByScene,
               "hay una escena bloqueante viva");
        return;
    }
    if (interpreter.isRunning(event.id.value())) {
        omitir(out, event, trigger, RegionSkipReason::AlreadyRunning, "el evento ya se ejecuta");
        return;
    }
    const Result<bool> arranque = interpreter.start(event, activa.index);
    if (!arranque.isOk()) {
        omitir(out, event, trigger, RegionSkipReason::StartFailed, arranque.errorMessage());
        return;
    }
    out.started.push_back(RegionStarted{event.id.value(), activa.index, trigger});
}

}  // namespace

Result<RegionDispatchResult> dispatchRegionTriggers(
    const std::vector<Regiones::RegionTrigger>& triggers, const MapDocument& map,
    const NarrativeState& narrative, const std::vector<std::string>& inventory,
    EventInterpreter& interpreter) {
    RegionDispatchResult out;
    for (const Regiones::RegionTrigger& trigger : triggers) {
        for (const MapLayer& capa : map.layers()) {
            const EventLayerDocument* eventos = std::get_if<EventLayerDocument>(&capa.data());
            if (eventos == nullptr) {
                continue;
            }
            for (const MapEventDocument& event : eventos->events()) {
                despacharAEvento(out, event, trigger, narrative, inventory, interpreter);
            }
        }
    }
    return Result<RegionDispatchResult>::Ok(std::move(out));
}

}  // namespace RPG
