// demo_narrativa_nd6 — tiradas Nd6 disparadas desde un beat de narrativa.
//
// Valida el puente NarrativeEngine -> DicePoolEngine que abre la fase de
// motor Nd6 en narrativa (ver FORMATO_AVENTURAS.md, effect "skillCheck"):
//
//   1. El parser de aventuras acepta un effect skillCheck y lo empaqueta
//      en NarrativeResult.skillChecks (diferido, como grantGold/log).
//   2. Al resolverlo contra DicePoolEngine, el grado Nd6 (BOTCH/PARTIAL/
//      SUCCESS/CRITICAL) enciende la flag correcta de las cuatro.
//
// Cada peticion se resuelve mediante GameSession con un jugador de
// prueba y RNG guionizado. El grado y la flag salen del codigo de partida.
//
// GL-free: corre en CI.

#include <cstdio>
#include <string>
#include <vector>

#include "Check.h"
#include "ScriptedRng.h"
#include "NarrativeCheckSession.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/NarrativeEngine.h"

using RPG::AdventureScript;
using Nd6SkillCatalog = RPG::Catalogs::SkillCatalog;
using RPG::NarrativeBeat;
using RPG::NarrativeEffect;
using RPG::NarrativeEngine;
using RPG::NarrativeResult;
using RPG::NarrativeState;
using RPG::SkillCheckRequest;
using RPG::SkillDefinition;

namespace {

// El rival tira dos dados, despues de los tres del jugador.
constexpr int kRivalDice = 2;

// Una mini-aventura de un solo beat con un skillCheck de Percepcion.
// La construimos en C++ (no hace falta JSON en disco) para que el test
// sea autocontenido; el parser se valida aparte cargando el JSON real
// del catalogo de skills.
AdventureScript aventuraDemo() {
    AdventureScript adv;
    adv.id = "adv_demo_nd6";
    adv.name = "Demo Nd6 en narrativa";

    NarrativeBeat beat;
    beat.id = "beat_percepcion";
    beat.trigger.type = RPG::TriggerType::Enter;
    beat.trigger.target = "sala";
    beat.speaker = "";
    beat.lines = {"Algo se mueve en la penumbra. ¿Lo ves?"};

    NarrativeEffect eff;
    eff.type = RPG::EffectType::SkillCheck;
    eff.skillCheck.skillId = "percepcion";
    eff.skillCheck.rivalDice = kRivalDice;
    eff.skillCheck.flagBotch = "percibe_nada";
    eff.skillCheck.flagPartial = "percibe_parcial";
    eff.skillCheck.flagSuccess = "percibe_exito";
    eff.skillCheck.flagCritical = "percibe_critico";
    beat.effects.push_back(eff);

    adv.beats.push_back(std::move(beat));
    return adv;
}

// Carga el catalogo Nd6 real de disco y comprueba que "percepcion" esta.
// Si esta parte falla, el parser de skills.json o el gen_skills_utilitarias
// tienen un problema -- no el puente narrativo.
Nd6SkillCatalog catalogoConPercepcion() {
    Nd6SkillCatalog cat;
    auto r = cat.loadFromFile("assets/catalogs/skills.json");
    require(r.isOk());
    require(cat.find("percepcion") != nullptr);
    require(cat.find("sigilo") != nullptr);
    require(cat.find("conocimiento_arcano") != nullptr);
    return cat;
}

}  // namespace

int main() {
    std::printf("=== demo_narrativa_nd6: tiradas Nd6 desde un beat ===\n\n");

    // --- 1. El beat empaqueta el skillCheck en NarrativeResult --------
    AdventureScript adv = aventuraDemo();
    NarrativeEngine engine;
    engine.setAdventure(&adv);
    NarrativeState state;

    NarrativeResult r = engine.enterLevel("sala", state);
    require(r.fired);
    require(r.skillChecks.size() == 1);
    require(r.skillChecks[0].skillId == "percepcion");
    require(r.skillChecks[0].rivalDice == kRivalDice);
    std::printf("[1] El beat empaqueta el skillCheck: OK (skillId=%s, rivalDice=%d)\n",
                r.skillChecks[0].skillId.c_str(), r.skillChecks[0].rivalDice);

    // --- 2. Las 9 skills utilitarias cargan del catalogo real --------
    Nd6SkillCatalog cat = catalogoConPercepcion();
    std::printf("[2] Catalogo Nd6 carga y tiene percepcion/sigilo/etc.: OK\n");

    const SkillCheckRequest req = r.skillChecks[0];

    // --- 3. Los cuatro grados encienden las cuatro flags -------------
    // Cada guion contiene 3 dados del jugador y 2 del rival.
    struct Caso { const char* nombre; std::vector<int> tirada; const char* flag; };
    const Caso casos[] = {
        {"BOTCH",    {1, 1, 1, 6, 2}, "percibe_nada"},
        {"PARTIAL",  {5, 1, 1, 5, 2}, "percibe_parcial"},
        {"SUCCESS",  {6, 1, 1, 5, 2}, "percibe_exito"},
        {"CRITICAL", {6, 6, 6, 6, 2}, "percibe_critico"},
    };

    std::printf("\n[3] Resolucion Nd6 -> flag (jugador DES 3, rival %dd6):\n", kRivalDice);
    for (const Caso& c : casos) {
        NarrativeState s;  // fresco por caso: una sola flag debe quedar encendida
        ScriptedRng rng(c.tirada);
        demo::resolveNarrativeCheck(cat, /*dados=*/3, req, rng, s);
        require(rng.consumed() == 5);
        require(s.hasFlag(c.flag));
        // Las otras tres NO deben quedar encendidas.
        for (const Caso& otra : casos) {
            if (otra.flag != c.flag) require(!s.hasFlag(otra.flag));
        }
        std::printf("    tirada {%d,%d,%d} vs {%d,%d} -> %s -> flag \"%s\": OK\n",
                    c.tirada[0], c.tirada[1], c.tirada[2], c.tirada[3], c.tirada[4], c.nombre, c.flag);
    }

    // El contenido antiguo sigue importandose como dados de un rival.
    // Contar las llamadas al RNG distingue una enfrentada real de una CD fija.
    {
        auto legacy = req;
        legacy.rivalDice = 0;
        legacy.cd = 1.5f;
        NarrativeState s;
        ScriptedRng rng({6, 5, 2, 5, 2, 2});
        demo::resolveNarrativeCheck(cat, 3, legacy, rng, s);
        require(rng.consumed() == 6);
        require(s.hasFlag(req.flagSuccess));
    }
    // rivalDice explicito manda aunque el request conserve un campo cd legacy.
    {
        auto explicitRival = req;
        explicitRival.rivalDice = 1;
        explicitRival.cd = 2.5f;
        NarrativeState s;
        ScriptedRng rng({6, 5, 2, 5});
        demo::resolveNarrativeCheck(cat, 3, explicitRival, rng, s);
        require(rng.consumed() == 4);
        require(s.hasFlag(req.flagSuccess));
    }
    // Cero dados contra cero es empate, sin consumir azar ni inventar pifia.
    {
        auto empty = req;
        empty.rivalDice = 0;
        empty.cd = 0.0f;
        NarrativeState s;
        ScriptedRng rng({});
        demo::resolveNarrativeCheck(cat, 0, empty, rng, s);
        require(rng.consumed() == 0);
        require(s.hasFlag(req.flagPartial));
    }
    std::printf("[4] Compatibilidad legacy, rival explicito y pools vacios: OK\n");

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
