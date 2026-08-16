// La Plaza de las Razas, recorrida de punta a punta sin abrir una ventana.
//
// Tarea P0 del PARALLEL_DEVELOPMENT_BOARD. La plaza no tiene combate ni
// oro: su unica recompensa es informacion, asi que lo que hay que
// comprobar es que se pueda hablar con los diez y que el cierre solo
// llegue cuando se ha hablado con TODOS. Un beat de cierre que se dispara
// antes de tiempo se lleva por delante el unico efecto del nivel.
//
// GL-free: NarrativeEngine no sabe nada de ventanas (ver su header).
#include <cstdio>
#include <string>
#include <vector>

#include "Check.h"
#include "RPG/NarrativeEngine.h"

using RPG::AdventureScript;
using RPG::NarrativeEngine;
using RPG::NarrativeResult;
using RPG::NarrativeState;

int main() {
    std::printf("=== Plaza de las Razas ===\n\n");

    auto carga = AdventureScript::loadFromFile("assets/adventures/plaza_de_las_razas.json");
    require(carga);
    const AdventureScript guion = carga.value();
    std::printf("  %d beats cargados\n", (int)guion.beats.size());
    require(guion.beats.size() == 14);

    NarrativeState estado;
    NarrativeEngine motor;
    motor.setAdventure(&guion);

    // --- Llegar ---
    NarrativeResult r = motor.enterLevel("assets/levels/plaza_de_las_razas.json", estado);
    require(!r.lines.empty());
    require(estado.hasFlag("plaza_vista"));
    std::printf("  llegada: \"%s\"\n", r.lines.front().substr(0, 62).c_str());

    // Volver a entrar NO repite la llegada: el jugador entra y sale de la
    // plaza varias veces y no puede oir lo mismo cada vez.
    r = motor.enterLevel("assets/levels/plaza_de_las_razas.json", estado);
    require(r.lines.empty());
    std::printf("  volver a entrar no repite el texto. OK\n");

    const std::vector<std::string> razas = {
        "aarakocra_de_las_montanas",
        "alquimistas_de_aegroum",
        "drow_explorador",
        "enanos_forjadores",
        "hombres_lagarto_de_la_selva",
        "medianos_urbanos",
        "minotauros_del_laberinto",
        "nagas_arcanos",
        "orcos_del_consejo",
        "race_elf_canopy",
        "tieflings_de_la_sombra",
        "yokai_de_los_vientos"};
    require(razas.size() == 12);

    // --- Hablar con nueve: el cierre NO puede llegar todavia ---
    int hablados = 0;
    for (std::size_t i = 0; i + 1 < razas.size(); ++i) {
        r = motor.talkTo("npc_race_" + razas[i], estado);
        require(!r.lines.empty());                       // cada uno dice lo suyo
        require(estado.hasFlag("plaza_hablado_" + razas[i]));
        ++hablados;
        require(!estado.hasFlag("plaza_completa"));      // aun no
    }
    NarrativeResult a = motor.tick(estado);
    require(a.lines.empty());
    require(!estado.hasFlag("plaza_completa"));
    std::printf("  con %d de 12, el cierre no se dispara. OK\n", hablados);

    // --- El duodecimo abre el cierre ---
    r = motor.talkTo("npc_race_" + razas.back(), estado);
    require(!r.lines.empty());
    a = motor.tick(estado);
    require(!a.lines.empty());
    require(estado.hasFlag("plaza_completa"));
    std::printf("  con los 12: \"%s\"\n", a.lines.back().substr(0, 62).c_str());

    // Y no se repite.
    a = motor.tick(estado);
    require(a.lines.empty());

    // Hablar de nuevo con uno ya hablado tampoco repite.
    r = motor.talkTo("npc_race_" + razas.front(), estado);
    require(r.lines.empty());
    std::printf("  nada se repite al insistir. OK\n");

    std::printf("\n  12 razas, 14 beats, todos alcanzables\n");
    std::printf("todas las comprobaciones han pasado\n");
    return 0;
}
