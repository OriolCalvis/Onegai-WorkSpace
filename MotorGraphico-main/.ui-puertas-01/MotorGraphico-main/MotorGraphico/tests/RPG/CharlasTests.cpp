// DIALOGOS-01 — Las charlas a lo D&D («Boundington: los diálogos»): lo que el
// motor narrativo pone de su parte.
//
//   - requires.lugar: un beat que solo salta en un nivel (el mismo PNJ dice
//     otra cosa en otro sitio). Sin lugar conocido, no se mira.
//   - "sigue": despues de una opcion la charla sigue (se vuelve a disparar el
//     mismo trigger), y un combate la corta.
//   - Paginas ("pagina", "pasaPagina", "despedida"): una pagina sin temas se
//     salta sola hasta la siguiente que tenga algo; si no queda nada en
//     ninguna, se queda sin «Hablar de otra cosa».
//   - «Eliges: ...» sin punto doble.
//   - Y se escribe de vuelta igual.
// GL-free.

#include "TestHarness.h"

#include "RPG/AdventureSerializer.h"
#include "RPG/NarrativeEngine.h"

#include <string>
#include <vector>

namespace {

// Una charla con dos paginas (un tema en cada una), y un PNJ que en la
// herreria dice otra cosa.
const char* kCharla = R"({
  "id": "prueba_charlas",
  "beats": [
    {"id": "en_la_herreria", "trigger": {"type": "talk", "target": "edul"},
     "requires": {"lugar": "assets/levels/interior_herreria.json"},
     "speaker": "Edul", "lines": ["Aqui no. Aqui trabajo."]},
    {"id": "abre", "trigger": {"type": "talk", "target": "edul"}, "requires": {"notFlags": ["visto"]},
     "speaker": "Edul", "lines": ["¿Qué quieres?"], "effects": [{"type": "setFlag", "arg": "visto"}],
     "opciones": [
       {"texto": "¿Forjas?", "requires": {"notFlags": ["t_a"]}, "lines": ["Forjaba."],
        "effects": [{"type": "setFlag", "arg": "t_a"}, {"type": "setFlag", "arg": "en"}], "sigue": true},
       {"texto": "Hablar de otra cosa", "effects": [{"type": "setVariable", "arg": "pag", "value": 1},
        {"type": "setFlag", "arg": "en"}], "sigue": true, "pasaPagina": true},
       {"texto": "Irte", "effects": [{"type": "setVariable", "arg": "pag", "value": 0},
        {"type": "clearFlag", "arg": "en"}], "despedida": true}
     ]},
    {"id": "s0", "trigger": {"type": "talk", "target": "edul"}, "pagina": true,
     "requires": {"allFlags": ["visto", "en"], "variables": {"pag": 0}}, "speaker": "Edul", "lines": ["…"],
     "opciones": [
       {"texto": "¿Forjas?", "requires": {"notFlags": ["t_a"]}, "lines": ["Forjaba."],
        "effects": [{"type": "setFlag", "arg": "t_a"}, {"type": "setFlag", "arg": "en"}], "sigue": true},
       {"texto": "Hablar de otra cosa", "effects": [{"type": "setVariable", "arg": "pag", "value": 1},
        {"type": "setFlag", "arg": "en"}], "sigue": true, "pasaPagina": true},
       {"texto": "Irte", "effects": [{"type": "setVariable", "arg": "pag", "value": 0},
        {"type": "clearFlag", "arg": "en"}], "despedida": true}
     ]},
    {"id": "s1", "trigger": {"type": "talk", "target": "edul"}, "pagina": true,
     "requires": {"allFlags": ["visto", "en"], "variables": {"pag": 1}}, "speaker": "Edul", "lines": ["…"],
     "opciones": [
       {"texto": "¿Y tu hermana?", "requires": {"notFlags": ["t_b"]}, "lines": ["Lee."],
        "effects": [{"type": "setFlag", "arg": "t_b"}, {"type": "setFlag", "arg": "en"}], "sigue": true},
       {"texto": "Pelear", "requires": {"allFlags": ["t_b"], "notFlags": ["pelea"]},
        "effects": [{"type": "setFlag", "arg": "pelea"},
                    {"type": "startBattle", "startBattle": {"monsterId": "edul_furioso",
                     "flagVictory": "gana", "flagDefeat": "pierde"}}], "sigue": true},
       {"texto": "Hablar de otra cosa", "effects": [{"type": "setVariable", "arg": "pag", "value": 0},
        {"type": "setFlag", "arg": "en"}], "sigue": true, "pasaPagina": true},
       {"texto": "Irte", "effects": [{"type": "setVariable", "arg": "pag", "value": 0},
        {"type": "clearFlag", "arg": "en"}], "despedida": true}
     ]}
  ]
})";

RPG::AdventureScript carga() {
    auto r = RPG::AdventureScript::loadFromString(kCharla);
    MOTOR_CHECK(r.isOk());
    return r.isOk() ? r.value() : RPG::AdventureScript{};
}

int elige(const RPG::NarrativeEngine& e, RPG::NarrativeState& s, const std::string& texto) {
    const std::vector<std::string> t = s.eleccion().textos;
    for (std::size_t i = 0; i < t.size(); ++i) {
        if (t[i] != texto) continue;
        e.elige(static_cast<int>(i), s);
        return static_cast<int>(i);
    }
    MOTOR_CHECK(false && "no esta esa opcion");
    return -1;
}

}  // namespace

MOTOR_TEST_CASE(el_lugar_decide_que_beat_salta) {
    const RPG::AdventureScript a = carga();
    RPG::NarrativeEngine e;
    e.setAdventure(&a);
    RPG::NarrativeState s;
    s.ponLugar("assets/levels/interior_herreria.json");
    MOTOR_CHECK(e.talkTo("edul", s).beatId == "en_la_herreria");
    s.ponLugar("assets/levels/ciudad_oeste.json");
    MOTOR_CHECK(e.talkTo("edul", s).beatId == "abre");
    // Sin lugar conocido (pruebas antiguas, demos sin niveles) no se mira.
    RPG::NarrativeState sin;
    MOTOR_CHECK(e.talkTo("edul", sin).beatId == "en_la_herreria");
}

MOTOR_TEST_CASE(la_charla_sigue_y_salta_las_paginas_vacias) {
    const RPG::AdventureScript a = carga();
    RPG::NarrativeEngine e;
    e.setAdventure(&a);
    RPG::NarrativeState s;
    s.ponLugar("assets/levels/ciudad_oeste.json");
    MOTOR_CHECK(e.talkTo("edul", s).beatId == "abre");
    elige(e, s, "¿Forjas?");
    MOTOR_CHECK(s.haySigue());
    // La pagina 0 ya no tiene temas (¿Forjas? esta preguntado): se salta sola
    // a la 1, sin decir nada de la 0.
    const RPG::NarrativeResult r = e.sigue(s);
    MOTOR_CHECK(r.fired && r.beatId == "s1");
    MOTOR_CHECK(!s.haySigue());
    MOTOR_CHECK(s.variable("pag") == 1);
    MOTOR_CHECK((s.eleccion().textos == std::vector<std::string>{"¿Y tu hermana?", "Hablar de otra cosa", "Irte"}));
    elige(e, s, "¿Y tu hermana?");
    // Un combate corta la charla: lo que queda se habla despues.
    const RPG::NarrativeResult p = e.sigue(s);
    MOTOR_CHECK(p.beatId == "s1");
    elige(e, s, "Pelear");
    MOTOR_CHECK(!s.haySigue());
    // Ya no queda nada en ninguna pagina: se queda en esta, sin pasar pagina.
    s.ponSigue(static_cast<int>(RPG::TriggerType::Talk), "edul");
    const RPG::NarrativeResult fin = e.sigue(s);
    MOTOR_CHECK(fin.beatId == "s1");
    MOTOR_CHECK((s.eleccion().textos == std::vector<std::string>{"Irte"}));
}

MOTOR_TEST_CASE(eliges_sin_punto_doble) {
    const RPG::AdventureScript a = carga();
    RPG::NarrativeEngine e;
    e.setAdventure(&a);
    RPG::NarrativeState s;
    s.ponLugar("x");
    e.talkTo("edul", s);
    const RPG::NarrativeResult r = e.elige(0, s);  // «¿Forjas?»
    MOTOR_CHECK(!r.log.empty() && r.log[0] == "Eliges: ¿Forjas?");
}

MOTOR_TEST_CASE(se_escriben_de_vuelta_igual) {
    const RPG::AdventureScript a = carga();
    const std::string una = RPG::serializeAdventure(a);
    auto otra = RPG::AdventureScript::loadFromString(una);
    MOTOR_CHECK(otra.isOk());
    if (!otra.isOk()) return;
    MOTOR_CHECK(RPG::serializeAdventure(otra.value()) == una);
    MOTOR_CHECK(otra.value().beats[2].pagina && !otra.value().beats[1].pagina);
    MOTOR_CHECK(otra.value().beats[1].opciones[1].pasaPagina);
    MOTOR_CHECK(otra.value().beats[1].opciones[2].despedida);
    MOTOR_CHECK(otra.value().beats[1].opciones[0].sigue);
    MOTOR_CHECK(otra.value().beats[0].requirements.lugares.size() == 1);
}

int main() { return motor_test::runAllTests(); }
