// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-07  (fase C, 2 j) - Les aventures autònomes disparen els seus beats
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// C-07 — La prova de regressió que faltava: les aventures autònomes
// disparen els seus beats.
//
// El bug (03/09): `Application::loadLevel` demanava
// `m_narrativaLista && m_narrativeCampaignTransitions` per cridar
// `setNarrative()`. Amb `--precuela` (o qualsevol aventura passada per
// `--aventura`) la bandera de campanya queda a false, i per tant
// l'aventura es carregava sencera per no fer absolutament res: ni un
// beat. L'altra cara del mateix encreuament: `update()` NO comprovava la
// bandera de campanya abans d'encadenar la cronologia de Boundington
// (pròleg -> Dia 1 -> Dia 2 -> Ocaso), o sigui que una aventura autònoma
// que encengués una flag de tancament de capítol es menjava salts que no
// són seus.
//
// Per què aquest fitxer i no `ApplicationTests.cpp`: la guarda vivia dins
// d'`Application`, que és GL i enllaça `motor_core`; els tests que
// l'enllacen no s'executen sense finestra i el paquet quedava obert per
// la regla 2 del punt 6 del pla. La decisió està extreta a
// `Application::narrativeWiring()`, que és pura (`static constexpr`, no
// toca cap membre ni cap crida GL) i és l'única font de veritat dels dos
// `if` d'`Application.cpp`. Això permet provar-la sense context GL, i
// muntar la resta del cablejat amb el `GameSession` i el
// `NarrativeEngine` de veritat, que ja són GL-free.
//
// Comprovat: si es torna la decisió a la condició antiga, aquest fitxer
// falla (vegeu l'informe del paquet).

#include "TestHarness.h"

#include "Engine/Application.h"
#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "RPG/NarrativeEngine.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

using Wiring = Application::NarrativeWiring;

// Ruta del nivell on entra el jugador. És alhora el `target` del beat
// "enter" de la precuela: `loadLevel` passa `levelPath` tal qual a
// `GameSession::enterLevelNarrative`.
const char* const kNivell = "assets/levels/test_level.json";

class TestBody final : public ICombatant {
public:
    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::min(m_maxHealth, m_health + amount); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }
    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "c07_body"; }

private:
    int m_health = 40;
    int m_maxHealth = 40;
};

LevelDefinition makeLevel() {
    LevelDefinition level;
    level.name = "Cami de Venides";
    level.playerStart = GridCoord{0, 0};
    return level;
}

// Una aventura AUTÒNOMA, del tipus que arriba per `--precuela`: no és cap
// dels quatre capítols de la campanya, té el seu propi beat d'entrada i
// —això és el que fa el cas realista— acaba encenent
// `bnd_prologo_terminado`, perquè una precuela de Boundington
// naturalment enllaça amb el pròleg. Si la bandera de campanya no
// governés les transicions de la cronologia, aquesta flag segrestaria el
// salt al Dia 1.
RPG::AdventureScript precuelaAutonoma() {
    const auto r = RPG::AdventureScript::loadFromString(R"({
        "id": "adv_precuela_c07",
        "name": "Precuela autonoma",
        "description": "Aventura fora de la cronologia, carregada amb --precuela.",
        "beats": [
            {"id": "pre_arribada",
             "trigger": {"type": "enter", "target": "assets/levels/test_level.json"},
             "speaker": "Narrador",
             "lines": ["El cami cap a Boundington es llarg.", "Ningu t'espera."],
             "effects": [
                {"type": "setFlag", "arg": "pre_arribada_feta"},
                {"type": "setFlag", "arg": "bnd_prologo_terminado"},
                {"type": "grantGold", "value": 5}
             ]}
        ]
    })");
    MOTOR_CHECK(r.isOk());
    return r.value();
}

// El director de capítols d'`Application::update()`, reduït al que
// decideix: la porta és `narrativeWiring(...).chainCampaignTimeline` —la
// decisió compartida de veritat— i darrere hi ha la mateixa cascada de
// flags que hi ha a `Application::update()`. Retorna quin salt de capítol
// demanaria aquest frame.
enum class Salt { Cap, Dia1, DiaGorran, Dia2, Ocaso };

Salt directorDeCampanya(const Wiring& wiring, const RPG::NarrativeState& state) {
    if (!wiring.chainCampaignTimeline) {
        return Salt::Cap;
    }
    if (state.hasFlag("bnd_prologo_terminado") && !state.hasFlag("bnd_dia1_empezado")) {
        return Salt::Dia1;
    }
    if (state.hasFlag("bnd_dia_cerrado") && !state.hasFlag("bnd_diag_empezado")) {
        return Salt::DiaGorran;
    }
    if (state.hasFlag("bndg_dia_cerrado") && !state.hasFlag("bnd_dia2_empezado")) {
        return Salt::Dia2;
    }
    if (state.hasFlag("bnd2_dia_cerrado") && !state.hasFlag("bnd_ocaso_empezado")) {
        return Salt::Ocaso;
    }
    return Salt::Cap;
}

// El cablejat d'`Application::loadLevel`, amb la MATEIXA guarda: si la
// decisió diu que cal enganxar-la, la narrativa va a la sessió nova i es
// dispara el beat "enter" del nivell que acaba de carregar.
bool muntaSessio(GameSession& session, const Wiring& wiring, const RPG::NarrativeEngine& engine,
                 RPG::NarrativeState& state) {
    if (!wiring.attachToSession) {
        return false;
    }
    session.setNarrative(&engine, &state);
    return session.enterLevelNarrative(kNivell);
}

struct Fixture {
    ObjectCatalog catalog;
    SkillCatalog skills;
    TestBody body;
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;

    GameSession nouSession() {
        return GameSession(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills,
                           &inventory);
    }
};

}  // namespace

// ---------------------------------------------------------------------
// La decisió, nua. Les dues banderes responen a DUES preguntes, i el bug
// va ser encreuar-les: aquesta és la taula que ho fixa.
// ---------------------------------------------------------------------
MOTOR_TEST_CASE(c07_la_bandera_de_campanya_no_decideix_si_la_narrativa_existeix) {
    // Sense aventura carregada no hi ha narrativa ni cronologia, digui el
    // que digui l'altra bandera.
    MOTOR_CHECK(!Application::narrativeWiring(false, false).attachToSession);
    MOTOR_CHECK(!Application::narrativeWiring(false, false).chainCampaignTimeline);
    MOTOR_CHECK(!Application::narrativeWiring(false, true).attachToSession);
    MOTOR_CHECK(!Application::narrativeWiring(false, true).chainCampaignTimeline);

    // Aventura autònoma (--precuela / --aventura): S'ENGANXA a la sessió
    // —aquesta és la cara del bug— i NO encadena la cronologia.
    MOTOR_CHECK(Application::narrativeWiring(true, false).attachToSession);
    MOTOR_CHECK(!Application::narrativeWiring(true, false).chainCampaignTimeline);

    // La campanya de Boundington: s'enganxa i a més encadena capítols.
    MOTOR_CHECK(Application::narrativeWiring(true, true).attachToSession);
    MOTOR_CHECK(Application::narrativeWiring(true, true).chainCampaignTimeline);
}

// ---------------------------------------------------------------------
// Cara 1 del bug: l'aventura autònoma s'enganxa i dispara el primer beat.
// Amb la condició antiga (`lista && campanya`) no s'arribava mai a
// setNarrative() i aquest cas es queda sense diàleg, sense flag i sense
// or.
// ---------------------------------------------------------------------
MOTOR_TEST_CASE(c07_una_aventura_autonoma_dispara_el_seu_primer_beat) {
    Fixture f;
    const RPG::AdventureScript guio = precuelaAutonoma();
    RPG::NarrativeEngine engine;
    engine.setAdventure(&guio);
    RPG::NarrativeState state;

    GameSession session = f.nouSession();
    const Wiring wiring = Application::narrativeWiring(/*narrativaLista=*/true,
                                                       /*campanya=*/false);

    MOTOR_CHECK(muntaSessio(session, wiring, engine, state));

    // El beat "enter" ha disparat de veritat: diàleg obert amb les seves
    // línies, flags enceses i l'efecte d'or aplicat.
    MOTOR_CHECK(session.mode() == GameMode::Dialogue);
    MOTOR_CHECK(session.dialogueSpeaker() == "Narrador");
    MOTOR_CHECK(session.dialogueLines().size() == 2);
    MOTOR_CHECK(state.hasFlag("pre_arribada_feta"));
    MOTOR_CHECK(session.gold() == 5);
    MOTOR_CHECK(session.narrativeState() == &state);
}

// La campanya no perd res pel camí: segueix enganxant-se igual.
MOTOR_TEST_CASE(c07_la_campanya_segueix_enganxant_se_a_la_sessio) {
    Fixture f;
    const RPG::AdventureScript guio = precuelaAutonoma();
    RPG::NarrativeEngine engine;
    engine.setAdventure(&guio);
    RPG::NarrativeState state;

    GameSession session = f.nouSession();
    MOTOR_CHECK(muntaSessio(session, Application::narrativeWiring(true, true), engine, state));
    MOTOR_CHECK(session.mode() == GameMode::Dialogue);
    MOTOR_CHECK(state.hasFlag("pre_arribada_feta"));
}

// Sense aventura carregada (init() no va poder llegir els JSON) la sessió
// es queda sense narrativa i no peta: el motor gràfic segueix servint.
MOTOR_TEST_CASE(c07_sense_aventura_la_sessio_es_queda_sense_narrativa) {
    Fixture f;
    const RPG::AdventureScript guio = precuelaAutonoma();
    RPG::NarrativeEngine engine;
    engine.setAdventure(&guio);
    RPG::NarrativeState state;

    GameSession session = f.nouSession();
    MOTOR_CHECK(!muntaSessio(session, Application::narrativeWiring(false, false), engine, state));
    MOTOR_CHECK(session.narrativeState() == nullptr);
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
    MOTOR_CHECK(!state.hasFlag("pre_arribada_feta"));
}

// ---------------------------------------------------------------------
// Cara 2 del bug: l'aventura autònoma NO encadena la cronologia de
// Boundington. La precuela encén `bnd_prologo_terminado` com a part del
// seu propi final; si la bandera de campanya no governés aquest bloc
// (com passava abans), el frame següent es plantaria a ciutat_centro
// carregant el Dia 1.
// ---------------------------------------------------------------------
MOTOR_TEST_CASE(c07_una_aventura_autonoma_no_encadena_la_cronologia_de_boundington) {
    Fixture f;
    const RPG::AdventureScript guio = precuelaAutonoma();
    RPG::NarrativeEngine engine;
    engine.setAdventure(&guio);
    RPG::NarrativeState state;

    GameSession session = f.nouSession();
    const Wiring wiring = Application::narrativeWiring(true, false);
    MOTOR_CHECK(muntaSessio(session, wiring, engine, state));

    // El beat de la precuela ha encès la flag de tancament del pròleg...
    MOTOR_CHECK(state.hasFlag("bnd_prologo_terminado"));
    // ...i tot i així el director no demana cap salt de capítol.
    MOTOR_CHECK(directorDeCampanya(wiring, state) == Salt::Cap);

    // Ni amb les altres dues portes de la cronologia enceses.
    state.setFlag("bnd_dia_cerrado");
    state.setFlag("bnd2_dia_cerrado");
    MOTOR_CHECK(directorDeCampanya(wiring, state) == Salt::Cap);
}

// I el contrapunt: amb la campanya de veritat, la cronologia sí encadena.
// Sense aquest cas, «no encadena mai» passaria el test anterior.
MOTOR_TEST_CASE(c07_la_campanya_si_encadena_els_seus_capitols) {
    const Wiring wiring = Application::narrativeWiring(true, true);
    RPG::NarrativeState state;

    MOTOR_CHECK(directorDeCampanya(wiring, state) == Salt::Cap);

    state.setFlag("bnd_prologo_terminado");
    MOTOR_CHECK(directorDeCampanya(wiring, state) == Salt::Dia1);

    state.setFlag("bnd_dia1_empezado");
    state.setFlag("bnd_dia_cerrado");
    MOTOR_CHECK(directorDeCampanya(wiring, state) == Salt::DiaGorran);

    state.setFlag("bnd_diag_empezado");
    state.setFlag("bndg_dia_cerrado");
    MOTOR_CHECK(directorDeCampanya(wiring, state) == Salt::Dia2);

    state.setFlag("bnd_dia2_empezado");
    state.setFlag("bnd2_dia_cerrado");
    MOTOR_CHECK(directorDeCampanya(wiring, state) == Salt::Ocaso);

    state.setFlag("bnd_ocaso_empezado");
    MOTOR_CHECK(directorDeCampanya(wiring, state) == Salt::Cap);
}

int main() { return motor_test::runAllTests(); }
