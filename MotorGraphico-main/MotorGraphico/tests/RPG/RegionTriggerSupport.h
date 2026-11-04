#pragma once

// U3-02.E06b — Soporte compartido de las pruebas de RegionTriggerDispatch y de
// su orquestacion entre frames: constructores de documentos y la escena.

#include "Level/ObjectCatalog.h"
#include "Map/MapDocument.h"
#include "RPG/EventCommand.h"
#include "RPG/EventInterpreter.h"
#include "RPG/MapEventRunner.h"
#include "RPG/SessionCommandHost.h"
#include "Render/ICombatant.h"
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "RPG/RegionTriggerDispatch.h"
#include "TestHarness.h"

namespace region_test {


using namespace RPG;
using Kind = Regiones::RegionTrigger::Kind;

class Cuerpo : public ICombatant {
public:
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "jugador"; }
};

inline LevelDefinition nivelVacio() {
    LevelDefinition l;
    l.name = "Prueba";
    l.playerStart = GridCoord{0, 0};
    return l;
}

inline ids::EventId evt(const std::string& v) { return ids::EventId::parse(v).value(); }
inline ids::LayerId lyr(const std::string& v) { return ids::LayerId::parse(v).value(); }
inline ids::RegionId reg(const std::string& v) { return ids::RegionId::parse(v).value(); }

inline MapEventCommand espera() {
    MapEventCommand c;
    c.type = "control.esperar";
    c.params = {{"ms", "1000"}};
    return c;
}

inline MapEventPage pagina(const std::string& id, EventTrigger t, const std::string& capa = "",
                    const std::string& region = "") {
    MapEventPage p;
    p.id = evt(id);
    p.name = id;
    p.trigger = t;
    if (!capa.empty()) {
        p.regionLayerId = lyr(capa);
        p.regionId = reg(region);
    }
    p.commands = {espera()};
    return p;
}

inline MapEventDocument evento(const std::string& id, GridCoord donde, MapEventPage p) {
    MapEventDocument e;
    e.id = evt(id);
    e.name = id;
    e.position = donde;
    e.pages = {std::move(p)};
    return e;
}

inline MapEventDocument eventoRegion(const std::string& id, int x, EventTrigger t,
                              const std::string& region = "zona", const std::string& capa = "reg") {
    return evento(id, GridCoord{x, 0}, pagina(id + "_p0", t, capa, region));
}

// Mapa con dos capas Region ("reg", "reg2", ambas con las regiones "zona" y "otra") y la
// capa de eventos con los eventos dados.
inline MapDocument mapaCon(std::vector<MapEventDocument> eventos) {
    MapDocument mapa(ids::MapId::parse("prueba").value(), "Prueba", 16, 12, 64, 32);
    int orden = 0;
    for (const char* capa : {"reg", "reg2"}) {
        MapLayerMeta meta;
        meta.order = orden++;
        meta.id = lyr(capa);
        meta.name = capa;
        MOTOR_CHECK(mapa.addLayer(MapLayer::regions(meta, RegionLayerDocument())).isOk());
        MapRegionDocument r;
        r.id = reg("zona");
        r.name = "zona";
        r.bounds = GridBounds{0, 3, 0, 3};
        MOTOR_CHECK(mapa.addRegion(meta.id, r).isOk());
        r.id = reg("otra");
        r.name = "otra";
        r.bounds = GridBounds{8, 9, 8, 9};
        MOTOR_CHECK(mapa.addRegion(meta.id, r).isOk());
    }
    MapLayerMeta meta;
    meta.id = lyr("eventos");
    meta.order = orden;
    meta.name = "Eventos";
    MOTOR_CHECK(mapa.addLayer(MapLayer::events(meta, EventLayerDocument())).isOk());
    for (MapEventDocument& e : eventos) {
        const Result<bool> alta = mapa.addEvent(meta.id, e);
        if (!alta.isOk()) {
            MOTOR_CHECK_FAIL(alta.errorMessage());
        }
    }
    return mapa;
}

inline Regiones::RegionTrigger disp(Kind k, const std::string& region = "zona",
                             const std::string& capa = "reg") {
    return Regiones::RegionTrigger{k, Regiones::RegionRef{lyr(capa), reg(region)}};
}

struct Escena {
    ObjectCatalog catalogo;
    SkillCatalog habilidades;
    Cuerpo cuerpo;
    SkillSet skills{10};
    std::vector<std::string> inventario;
    GameSession sesion{nullptr, &catalogo, &habilidades, nivelVacio(), &cuerpo, &skills,
                       &inventario};
    NarrativeState estado;
    SessionCommandHost partida{sesion, inventario};
    NarrativeCommandHost host{estado, &partida};
    EventInterpreter interprete{host};
    MapEventRunner runner{estado, inventario, interprete};

    RegionDispatchResult despachar(const MapDocument& mapa,
                                   const std::vector<Regiones::RegionTrigger>& t) {
        const Result<RegionDispatchResult> r =
            dispatchRegionTriggers(t, mapa, estado, inventario, interprete);
        MOTOR_CHECK(r.isOk());
        return r.value();
    }
};

}  // namespace region_test
