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
// Lo hace SIN GameSession (que pediria TileMap/catalogos/ICombatant):
// replica exactamente el resolveSkillChecks de GameSession, que a su vez
// no es mas que roll_pool + resolve_against_cd del mismo DicePoolEngine
// que usa el combate. Asi narrativa y combate comparten reglas Nd6 (GDD
// 7.1): un arma que hace 8 de dano son 8 dados, y una Percepcion DC 1.0
// son N dados del castingStat; la regla de exitos (6=1, 5=medio, 1-4=0)
// es la misma en los dos sitios.
//
// GL-free: corre en CI.

#include <cstdio>
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
using RPG::NarrativeBeat;
using RPG::NarrativeEffect;
using RPG::NarrativeEngine;
using RPG::NarrativeResult;
using RPG::NarrativeState;
using RPG::SkillCheckRequest;
using RPG::SkillDefinition;

namespace {

// CD 1.0 para todas las tiradas: con ella los cuatro grados se distinguen
// con pools pequenos y faciles de guionizar:
//   0 exitos        -> BOTCH
//   0.5 exitos       -> PARTIAL   (< 1.0)
//   1.0 exitos       -> SUCCESS   (>= 1.0)
//   >= 2.0 exitos    -> CRITICAL  (>= cd+1)
constexpr float kCd = 1.0f;

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
    eff.skillCheck.cd = kCd;
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
SkillCatalog catalogoConPercepcion() {
    SkillCatalog cat;
    auto r = cat.loadFromFile("assets/catalogs/skills.json");
    require(r.isOk());
    require(cat.find("percepcion") != nullptr);
    require(cat.find("sigilo") != nullptr);
    require(cat.find("conocimiento_arcano") != nullptr);
    return cat;
}

// Replica exacta de GameSession::resolveSkillChecks: dado un catalogo, el
// VALOR del stat del jugador (0..8 en Onegai, no el enum Stat) y un RNG,
// resuelve una peticion y enciende la flag del grado en "state". Devuelve
// el grado para que el test pueda assertarlo.
//
// OJO: el tamano del pool es el VALOR del stat (int), no Stat::DES. En
// GameSession sale de m_playerBody->stat(def->casting_stat), que devuelve
// el rango 0..8; aqui lo pasamos directamente porque no hay ICombatant.
int gradoTrasTirada(const SkillCatalog& cat, int valorStat,
                    const SkillCheckRequest& req, ScriptedRng& rng,
                    NarrativeState& state) {
    const SkillDefinition* def = cat.find(req.skillId);
    require(def != nullptr);

    // N dados = valor del stat del jugador segun la skill. Igual que en
    // combate (ApplyBasicAttackNd6): la pool nace del stat, no de un
    // numero fijo.
    const int dados = valorStat;
    const RPG::PoolResult pool =
        RPG::DicePoolEngine::roll_pool(dados, RPG::DiceMod::NORMAL, rng);
    const RPG::CheckOutcome out = RPG::DicePoolEngine::resolve_against_cd(pool, req.cd);

    switch (out.degree) {
        case RPG::Degree::BOTCH:    state.setFlag(req.flagBotch);    break;
        case RPG::Degree::PARTIAL:  state.setFlag(req.flagPartial);  break;
        case RPG::Degree::SUCCESS:  state.setFlag(req.flagSuccess);  break;
        case RPG::Degree::CRITICAL: state.setFlag(req.flagCritical); break;
    }
    return static_cast<int>(out.degree);
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
    std::printf("[1] El beat empaqueta el skillCheck: OK (skillId=%s, cd=%.1f)\n",
                r.skillChecks[0].skillId.c_str(), r.skillChecks[0].cd);

    // --- 2. Las 9 skills utilitarias cargan del catalogo real --------
    SkillCatalog cat = catalogoConPercepcion();
    std::printf("[2] Catalogo Nd6 carga y tiene percepcion/sigilo/etc.: OK\n");

    const SkillCheckRequest req = r.skillChecks[0];

    // --- 3. Los cuatro grados encienden las cuatro flags -------------
    // Jugador con DES 3 (pool de 3 dados). Tiradas guionizadas para forzar
    // cada grado contra CD 1.0.
    //   BOTCH:    {1,1,1}        -> 0 exitos
    //   PARTIAL:  {5,1,1}        -> 0.5 exitos (< 1.0)
    //   SUCCESS:  {6,1,1}        -> 1.0 exitos (>= 1.0)
    //   CRITICAL: {6,6,1}        -> 2.0 exitos (>= cd+1 = 2.0)
    struct Caso { const char* nombre; std::vector<int> tirada; int grado; const char* flag; };
    const Caso casos[] = {
        {"BOTCH",    {1, 1, 1}, 0, "percibe_nada"},
        {"PARTIAL",  {5, 1, 1}, 1, "percibe_parcial"},
        {"SUCCESS",  {6, 1, 1}, 2, "percibe_exito"},
        {"CRITICAL", {6, 6, 1}, 3, "percibe_critico"},
    };

    std::printf("\n[3] Resolucion Nd6 -> flag (jugador DES 3, CD %.1f):\n", kCd);
    for (const Caso& c : casos) {
        NarrativeState s;  // fresco por caso: una sola flag debe quedar encendida
        ScriptedRng rng(c.tirada);
        const int grado = gradoTrasTirada(cat, /*valorStat=*/3, req, rng, s);
        require(grado == c.grado);
        require(s.hasFlag(c.flag));
        // Las otras tres NO deben quedar encendidas.
        for (const Caso& otra : casos) {
            if (otra.flag != c.flag) require(!s.hasFlag(otra.flag));
        }
        std::printf("    tirada {%d,%d,%d} -> %s -> flag \"%s\": OK\n",
                    c.tirada[0], c.tirada[1], c.tirada[2], c.nombre, c.flag);
    }

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
