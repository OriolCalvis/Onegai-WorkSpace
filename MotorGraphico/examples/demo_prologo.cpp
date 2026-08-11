// demo_prologo — el prologo onirico de Boundington (Cap. 1 y 2 del canon).
//
// Reproduce la aventura boundington_prologo.json de punta a cabo, forzando
// SUCCESS en las dos tiradas Nd6 (Percepcion y Conocimiento Arcano) con un
// ScriptedRng, y comprueba que el cierre deja bnd_prologo_terminado
// encendido. Valida ademas que el contenido no tiene beats muertos: en la
// ruta SUCCESS, todos los beats "principales" (los que no son las ramas
// BOTCH/PARTIAL/CRITICAL de las dos tiradas) disparan.
//
// GL-free: corre en CI. Mismo molde que demo_boundington, pero aqui la
// cadena es toda Auto (un beat por tick) y la narrativa pide tiradas Nd6
// que este demo resuelve con DicePoolEngine, igual que haria GameSession.

#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "Check.h"
#include "ScriptedRng.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/NarrativeEngine.h"

using RPG::AdventureScript;
using RPG::Catalogs::SkillCatalog;
using RPG::NarrativeEngine;
using RPG::NarrativeResult;
using RPG::NarrativeState;
using RPG::SkillDefinition;

namespace {

const char* kRuta = "assets/adventures/boundington_prologo.json";

// Los dos skillCheck del prologo: Percepcion (CD 1.0) y Conocimiento Arcano
// (CD 1.5). Los identificamos por el skillId, que es estable en el JSON.
bool esCheckDe(const RPG::NarrativeResult& r, const char* skillId) {
    for (const auto& c : r.skillChecks) {
        if (c.skillId == skillId) return true;
    }
    return false;
}

// Resuelve los skillCheck de un NarrativeResult con DicePoolEngine, igual
// que GameSession::resolveSkillChecks, y enciende la flag del grado en el
// estado. Usa el valor de stat indicado como tamano de pool (en una partida
// real sale de ICombatant::stat; aqui lo fijamos para poder guionizar el
// resultado via ScriptedRng). Devuelve cuantos checks resolvio.
//
// OJO con el RNG: se hace rewind() entre checks para que cada tirada arran
//que desde el mismo guion. Si no, un pool largo (stat 3 = 3 dados) dejaria
// el RNG avanzado y la siguiente tirada leeria valores distintos a los
// previstos. En una partida real esto no importa (cada tirada es azar);
// aqui si, porque queremos forzar un grado concreto.
int resolverChecks(const SkillCatalog& cat, int valorStat,
                   const RPG::NarrativeResult& r, ScriptedRng& rng,
                   NarrativeState& state) {
    int n = 0;
    for (const auto& c : r.skillChecks) {
        const SkillDefinition* def = cat.find(c.skillId);
        require(def != nullptr);
        rng.rewind();
        const RPG::PoolResult pool =
            RPG::DicePoolEngine::roll_pool(valorStat, RPG::DiceMod::NORMAL, rng);
        const RPG::CheckOutcome out = RPG::DicePoolEngine::resolve_against_cd(pool, c.cd);
        switch (out.degree) {
            case RPG::Degree::BOTCH:    state.setFlag(c.flagBotch);    break;
            case RPG::Degree::PARTIAL:  state.setFlag(c.flagPartial);  break;
            case RPG::Degree::SUCCESS:  state.setFlag(c.flagSuccess);  break;
            case RPG::Degree::CRITICAL: state.setFlag(c.flagCritical); break;
        }
        ++n;
    }
    return n;
}

// Imprime lo que devuelve un disparo y dice si hubo beat.
bool suelta(const RPG::NarrativeResult& r, const char* que) {
    if (!r.fired) return false;
    std::printf("    [%s]\n", que);
    if (!r.speaker.empty()) std::printf("      %s:\n", r.speaker.c_str());
    for (const auto& l : r.lines) std::printf("        %s\n", l.c_str());
    return true;
}

}  // namespace

int main() {
    std::printf("=== demo_prologo: Los Perdidos de Boundington (Cap. 1-2) ===\n\n");

    auto cargada = AdventureScript::loadFromFile(kRuta);
    require(cargada.isOk());
    const AdventureScript guion = cargada.value();
    std::printf("[carga] %s: %zu beats, %zu objetivos\n\n",
                guion.id.c_str(), guion.beats.size(), guion.objectives.size());

    // Catalogo Nd6: necesita las skills utilitarias (percepcion,
    // conocimiento_arcano) que anyade tools/gen_skills_utilitarias.py.
    SkillCatalog cat;
    auto rc = cat.loadFromFile("assets/catalogs/skills.json");
    require(rc.isOk());
    require(cat.find("percepcion") != nullptr);
    require(cat.find("conocimiento_arcano") != nullptr);

    NarrativeEngine e;
    e.setAdventure(&guion);
    NarrativeState s;

    std::set<std::string> vistos;  // beats que dispararon (para cobertura)

    // RNG guionizado para forzar SUCCESS en las dos tiradas. Con stat 3 y
    // tirada {6,1,1} = 1.0 exitos: SUCCESS contra CD 1.0 (>=1.0, <cd+1) y
    // tambien SUCCESS contra CD 1.5 seria PARTIAL (0.5<1.0)... asi que
    // para que Conocimiento Arcano (CD 1.5) salga SUCCESS hace falta
    // >=1.5 exitos: {6,5,1} = 1.5 exitos. Como resolverChecks hace rewind
    // por check, la tirada se reinicia para cada skill, asi que el mismo
    // guion sirve para las dos. {6,5,1} = 1.5 exitos: SUCCESS contra CD
    // 1.0 (1.5>=1.0 y <2.0) Y SUCCESS contra CD 1.5 (1.5>=1.5 y <2.5).
    // No es CRITICAL porque no todos los dados son 6.
    ScriptedRng rng({6, 5, 1});

    // Beat de entrada: el "enter" en el nivel del vacio. Lo contamos en
    // "vistos" aunque se dispare por enterLevel y no por tick.
    const NarrativeResult entrada = e.enterLevel("assets/levels/interior_vacio.json", s);
    require(suelta(entrada, "caida"));
    vistos.insert(entrada.beatId);
    require(s.hasFlag("bnd_prologo_empezado"));

    // A partir de aqui toda la cadena es Auto: un beat por tick. Cuando un
    // tick devuelve skillChecks, los resolvemos antes del siguiente tick
    // (igual que GameSession: applyNarrative resuelve y luego closeInteraction
    // vuelve a hacer tick). El bucle termina cuando ningun beat nuevo
    // dispara en un tick.
    std::printf("\n[relato — cadena auto, un beat por tick]\n");
    int ticksSinNovedad = 0;
    int ticks = 0;
    // Seguridad: el prologo tiene 18 beats; 40 ticks sobran.
    while (ticksSinNovedad < 2 && ticks < 60) {
        NarrativeResult r = e.tick(s);
        if (!r.fired) {
            ++ticksSinNovedad;
            ++ticks;
            continue;
        }
        ticksSinNovedad = 0;
        vistos.insert(r.beatId);
        suelta(r, r.beatId.c_str());

        // Si este beat pidio tiradas, las resolvemos con el RNG antes de
        // seguir: la flag del grado debe quedar encendida para que el beat
        // "de resultado" encaje en el siguiente tick.
        if (!r.skillChecks.empty()) {
            const int resueltos = resolverChecks(cat, /*valorStat=*/3, r, rng, s);
            require(resueltos >= 1);
            std::printf("      (tirada Nd6 resuelta: %d check(s))\n", resueltos);
        }
        ++ticks;
    }

    // --- Comprobaciones finales ---
    std::printf("\n[comprobaciones]\n");

    require(s.hasFlag("bnd_prologo_terminado"));
    std::printf("    cierre bnd_prologo_terminado encendido: OK\n");

    require(s.hasFlag("bnd_prologo_shonoco"));
    require(s.hasFlag("bnd_prologo_pasado"));
    std::printf("    flags de contexto (shonoco, pasado): OK\n");

    // Cobertura: los beats "principales" de la cadena deben haber disparado.
    // Las ramas BOTCH/PARTIAL/CRITICAL de cada tirada no disparan en la ruta
    // SUCCESS (por definicion, son mutuamente excluyentes), asi que no se
    // exigen.
    const char* esperados[] = {
        "prologo_caida", "prologo_presentacion", "prologo_percepcion",
        "prologo_percepcion_exito", "prologo_barca", "prologo_arcano",
        "prologo_arcano_exito", "prologo_subida", "prologo_taberna",
        "prologo_giro", "prologo_despertar",
    };
    int faltan = 0;
    for (const char* id : esperados) {
        if (vistos.find(id) == vistos.end()) {
            std::printf("    BEAT FALTANTE: %s\n", id);
            ++faltan;
        }
    }
    require(faltan == 0);
    std::printf("    cobertura: los %zu beats principales de la ruta SUCCESS "
                "dispararon (sin beats muertos)\n",
                sizeof(esperados) / sizeof(esperados[0]));

    // Y las ramas excluidas NO deben haber disparado.
    require(!s.hasFlag("bnd_prologo_percibe_botch"));
    require(!s.hasFlag("bnd_prologo_arcano_botch"));
    std::printf("    ramas excluidas (botch) no dispararon: OK\n");

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
