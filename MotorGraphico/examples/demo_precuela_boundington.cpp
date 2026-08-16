// Prueba de contenido de la precuela de Boundington. Mantiene trazable la
// entrada de Obsidian y valida las dos consecuencias del bosque sin abrir GL.
#include <cstdio>

#include "Check.h"
#include "RPG/NarrativeEngine.h"

namespace {
constexpr const char* kRuta = "assets/adventures/boundington_precuela_taberna.json";
void requireBeat(const RPG::NarrativeResult& result) { require(result.fired); }
void completaHilos(RPG::NarrativeEngine& engine, RPG::NarrativeState& state) {
    requireBeat(engine.enterLevel("assets/levels/interior_taberna.json", state));
    requireBeat(engine.talkTo("luisarda", state));
    requireBeat(engine.enterLevel("assets/levels/ciudad_oeste.json", state));
    requireBeat(engine.talkTo("griffin", state));
    requireBeat(engine.enterLevel("assets/levels/interior_banos.json", state));
}
}  // namespace

int main() {
    auto loaded = RPG::AdventureScript::loadFromFile(kRuta);
    require(loaded.isOk());
    const RPG::AdventureScript script = loaded.value();
    {
        RPG::NarrativeEngine engine; RPG::NarrativeState state; engine.setAdventure(&script);
        completaHilos(engine, state);
        requireBeat(engine.talkTo("ben_kafka", state));
        requireBeat(engine.enterLevel("assets/levels/mazmorra_64x64.json", state));
        requireBeat(engine.talkTo("ben_kafka", state));
        requireBeat(engine.tick(state));
        require(state.hasFlag("pre_bnd_ninos_rescatados"));
        require(!state.hasFlag("pre_bnd_ninos_sacrificados"));
        require(state.hasFlag("pre_bnd_precuela_cerrada"));
    }
    {
        RPG::NarrativeEngine engine; RPG::NarrativeState state; engine.setAdventure(&script);
        completaHilos(engine, state);
        requireBeat(engine.talkTo("ben_kafka", state));
        requireBeat(engine.enterLevel("assets/levels/mazmorra_64x64.json", state));
        state.setFlag("pre_bnd_forzar_tragedia");
        requireBeat(engine.tick(state));
        requireBeat(engine.tick(state));
        require(state.hasFlag("pre_bnd_ninos_sacrificados"));
        require(!state.hasFlag("pre_bnd_ninos_rescatados"));
        require(state.hasFlag("pre_bnd_precuela_cerrada"));
    }
    std::printf("precuela de Boundington: rutas de rescate y tragedia verificadas\n");
    return 0;
}
