// U3-02.E06b — El contrato y las decisiones estan en
// include/RPG/RegionTriggerDispatch.h; aqui solo esta el como.

#include "RPG/RegionTriggerDispatch.h"

#include <utility>
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

// ¿Alguna pagina del evento escucha este trigger? Filtro previo: solo esos
// eventos se evaluan, y por tanto solo ellos pueden reportar NotEvaluable.
bool escuchaTrigger(const MapEventDocument& event, const Regiones::RegionTrigger& trigger) {
    for (const MapEventPage& p : event.pages) {
        if (p.trigger == triggerDe(trigger.kind) && p.regionLayerId == trigger.ref.capa &&
            p.regionId == trigger.ref.region) {
            return true;
        }
    }
    return false;
}

void despacharAEvento(RegionDispatchResult& out, const MapEventDocument& event,
                      const Regiones::RegionTrigger& trigger, const NarrativeState& narrative,
                      const std::vector<std::string>& inventory, EventInterpreter& interpreter) {
    if (!escuchaTrigger(event, trigger)) {
        return;
    }
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

// Quita el mapa al runner y lo devuelve al salir del ambito, tambien si
// `update()` lanza (el anfitrion de comandos no es noexcept).
class MapaSuspendido {
public:
    MapaSuspendido(MapEventRunner& runner, bool suspender)
        : m_runner(runner), m_mapa(runner.map()), m_activo(suspender) {
        if (m_activo) {
            m_runner.setMap(nullptr);
        }
    }
    ~MapaSuspendido() {
        if (m_activo) {
            m_runner.setMap(m_mapa);
        }
    }
    MapaSuspendido(const MapaSuspendido&) = delete;
    MapaSuspendido& operator=(const MapaSuspendido&) = delete;

private:
    MapEventRunner& m_runner;
    const MapDocument* m_mapa;
    bool m_activo;
};

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

Result<RegionDispatchResult> updateEventsAndRegions(
    MapEventRunner& runner, EventInterpreter& interpreter, const NarrativeState& narrative,
    const std::vector<std::string>& inventory, const MapEventFrame& frame,
    const std::vector<Regiones::RegionTrigger>& triggers) {
    const MapDocument* mapa = runner.map();
    // Con una escena bloqueante viva el runner no debe ver el mapa: sin mapa
    // `update()` solo limpia, registra la casilla y avanza el interprete, y
    // `dispararAutomaticos()` (que no mira blocksPlayer) no puede arrancar
    // un Autorun encima. No se pierde nada: ni Touch ni Interact arrancan con
    // algo bloqueando, y los automaticos (Autorun Y Parallel) no se registran,
    // asi que arrancan cuando la escena termina.
    const bool bloqueado = interpreter.blocksPlayer();
    {
        const MapaSuspendido guarda(runner, bloqueado);
        runner.update(frame);
    }
    if (mapa == nullptr) {
        return Result<RegionDispatchResult>::Ok(RegionDispatchResult{});
    }
    if (bloqueado) {
        // Ver la cabecera (SIN INANICION): los automaticos pendientes van
        // primero, en el frame siguiente.
        RegionDispatchResult diferido;
        for (const Regiones::RegionTrigger& trigger : triggers) {
            diferido.skipped.push_back(RegionSkipped{
                std::string(), trigger, RegionSkipReason::DeferredToAutomatics,
                "el frame empezo con una escena bloqueante"});
        }
        return Result<RegionDispatchResult>::Ok(std::move(diferido));
    }
    return dispatchRegionTriggers(triggers, *mapa, narrative, inventory, interpreter);
}

}  // namespace RPG
