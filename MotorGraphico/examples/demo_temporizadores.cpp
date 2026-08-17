// Test GL-free de los temporizadores de mision (F1, bitacora 2.18).
// Tiempo NARRATIVO: solo avanza con dt activo (el orquestador pasa 0 en
// combate/dialogo/pausa/AFK), puede expirar por tiempo o por salir de la
// zona, avisa por umbrales y las consecuencias las disparan los beats
// Auto de siempre. Sin ventana, sin assert(): require() de Check.h.

#include <cstdio>
#include <string>

#include "Check.h"
#include "RPG/NarrativeEngine.h"

using namespace RPG;

namespace {

const char* kAventura = R"({
  "id": "demo_temporizadores",
  "name": "Demo temporizadores F1",
  "timers": [
    {
      "id": "rescate",
      "duracion": 10.0,
      "arrancaPor": "aviso_secuestro",
      "flagExpira": "llegaste_tarde",
      "avisos": [
        { "fraccion": 0.5, "flag": "gritos_lejanos" },
        { "fraccion": 0.1, "flag": "ultimos_segundos" }
      ],
      "presentacion": "contador"
    },
    {
      "id": "casa_en_llamas",
      "duracion": 60.0,
      "expiraPor": ["tiempo", "salir_de_zona"],
      "zona": "nivel_casa",
      "flagExpira": "casa_quemada",
      "presentacion": "secreto"
    }
  ],
  "beats": [
    {
      "id": "arrancar_casa",
      "trigger": { "type": "talk", "target": "vecina" },
      "effects": [ { "type": "startTimer", "arg": "casa_en_llamas" } ]
    },
    {
      "id": "casa_arde",
      "trigger": { "type": "auto" },
      "requires": { "allFlags": ["casa_quemada"] },
      "once": true,
      "lines": ["La casa se derrumba tras de ti."]
    },
    {
      "id": "tarde",
      "trigger": { "type": "auto" },
      "requires": { "allFlags": ["llegaste_tarde"] },
      "once": true,
      "lines": ["Ya no queda nadie a quien salvar."]
    }
  ]
})";

// Copia por valor: timers() devuelve un vector temporal y devolver un
// puntero a sus elementos seria dangling. id vacio = no encontrado.
TimerState busca(const NarrativeState& state, const std::string& id) {
    for (const TimerState& t : state.timers()) {
        if (t.id == id) return t;
    }
    return TimerState{};
}

void testCarga() {
    Result<AdventureScript> r = AdventureScript::loadFromString(kAventura);
    require(r.isOk());
    const AdventureScript& adv = r.value();
    require(adv.timers.size() == 2);

    const TimerDefinition& rescate = adv.timers[0];
    require(rescate.id == "rescate");
    require(rescate.duracion > 9.9f && rescate.duracion < 10.1f);
    require(rescate.arrancaPor == "aviso_secuestro");
    require(rescate.expiraPorTiempo == true);
    require(rescate.expiraPorSalirDeZona == false);
    require(rescate.avisos.size() == 2);
    require(rescate.presentacion == TimerDefinition::Presentacion::Contador);

    const TimerDefinition& casa = adv.timers[1];
    require(casa.expiraPorTiempo == true);
    require(casa.expiraPorSalirDeZona == true);
    require(casa.zona == "nivel_casa");
    require(casa.presentacion == TimerDefinition::Presentacion::Secreto);

    std::printf("[TIMERS] carga y campos correctos.\n");
}

void testErroresDeCarga() {
    // Sin flagExpira: nadie se enteraria de que expiro.
    require(!AdventureScript::loadFromString(R"({
        "id": "x", "timers": [ { "id": "t", "duracion": 5 } ], "beats": []
    })").isOk());
    // salir_de_zona sin zona declarada.
    require(!AdventureScript::loadFromString(R"({
        "id": "x", "timers": [ { "id": "t", "expiraPor": ["salir_de_zona"],
                                  "flagExpira": "f" } ], "beats": []
    })").isOk());
    // Expira por tiempo pero duracion 0.
    require(!AdventureScript::loadFromString(R"({
        "id": "x", "timers": [ { "id": "t", "duracion": 0,
                                  "flagExpira": "f" } ], "beats": []
    })").isOk());
    // Aviso con fraccion fuera de (0, 1].
    require(!AdventureScript::loadFromString(R"({
        "id": "x", "timers": [ { "id": "t", "duracion": 5, "flagExpira": "f",
                                  "avisos": [ { "fraccion": 0, "flag": "a" } ] } ],
        "beats": []
    })").isOk());
    // Presentacion desconocida.
    require(!AdventureScript::loadFromString(R"({
        "id": "x", "timers": [ { "id": "t", "duracion": 5, "flagExpira": "f",
                                  "presentacion": "reloj_de_arena" } ], "beats": []
    })").isOk());
    std::printf("[TIMERS] errores de carga rechazados (todo-o-nada).\n");
}

void testRelojDelRescate() {
    AdventureScript adv = AdventureScript::loadFromString(kAventura).value();
    NarrativeEngine engine;
    engine.setAdventure(&adv);
    NarrativeState state;

    // Arranca por flag; sin la flag, el reloj no esta en marcha.
    engine.tickTimers(state, 1.0f, "nivel_calle");
    require(busca(state, "rescate").activo == false);

    state.setFlag("aviso_secuestro");
    engine.tickTimers(state, 0.0f, "nivel_calle");
    require(busca(state, "rescate").id == "rescate");
    require(busca(state, "rescate").activo == true);

    // dt activo descuenta; dt 0 (combate/dialogo/pausa/AFK) no.
    engine.tickTimers(state, 4.0f, "nivel_calle");
    require(busca(state, "rescate").restante > 5.9f &&
            busca(state, "rescate").restante < 6.1f);
    require(state.hasFlag("gritos_lejanos") == false);  // umbral: 5.0

    engine.tickTimers(state, 0.0f, "nivel_calle");      // combate: congelado
    require(busca(state, "rescate").restante > 5.9f);

    engine.tickTimers(state, 1.5f, "nivel_calle");      // restante 4.5 <= 5
    require(state.hasFlag("gritos_lejanos") == true);

    engine.tickTimers(state, 3.6f, "nivel_calle");      // restante 0.9 <= 1.0
    require(state.hasFlag("ultimos_segundos") == true);

    engine.tickTimers(state, 1.0f, "nivel_calle");      // restante 0: tarde
    require(state.hasFlag("llegaste_tarde") == true);
    require(busca(state, "rescate").activo == false);
    require(busca(state, "rescate").expirado == true);

    // La consecuencia la dispara el beat Auto de siempre.
    NarrativeResult fin = engine.tick(state);
    require(fin.fired == true);
    require(fin.beatId == "tarde");

    // Un timer expirado no se reinicia solo.
    engine.tickTimers(state, 5.0f, "nivel_calle");
    require(busca(state, "rescate").expirado == true);

    std::printf("[TIMERS] reloj, avisos por umbral, pausa con dt 0 y expiracion.\n");
}

void testSalirDeLaZona() {
    AdventureScript adv = AdventureScript::loadFromString(kAventura).value();
    NarrativeEngine engine;
    engine.setAdventure(&adv);
    NarrativeState state;

    // Arranca por efecto startTimer al hablar con la vecina.
    NarrativeResult charla = engine.talkTo("vecina", state);
    require(charla.fired == true);
    require(busca(state, "casa_en_llamas").id == "casa_en_llamas");
    require(busca(state, "casa_en_llamas").activo == true);

    // Dentro de la zona, con dt 0, no pasa nada.
    engine.tickTimers(state, 0.0f, "nivel_casa");
    require(busca(state, "casa_en_llamas").activo == true);

    // SALIR de la zona lo expira aunque quedara tiempo en el reloj:
    // la casa arde igual (el ejemplo del encargo).
    engine.tickTimers(state, 0.0f, "nivel_calle");
    require(state.hasFlag("casa_quemada") == true);
    require(busca(state, "casa_en_llamas").expirado == true);

    NarrativeResult derrumbe = engine.tick(state);
    require(derrumbe.fired == true);
    require(derrumbe.beatId == "casa_arde");

    std::printf("[TIMERS] expiracion por salir de la zona + consecuencia.\n");
}

}  // namespace

int main() {
    testCarga();
    testErroresDeCarga();
    testRelojDelRescate();
    testSalirDeLaZona();
    std::printf("demo_temporizadores: todas las comprobaciones han pasado\n");
    return 0;
}
