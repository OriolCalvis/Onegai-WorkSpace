// demo_boundington — el primer dia en Boundington, 1981 b.f.
//
// Juega la aventura tres veces, por tres caminos distintos, y comprueba que
// los tres desenlaces se alcanzan. Es la primera aventura del motor con final
// ramificado: el jugador no puede evitar la Matanza de Boundington -- ocurre
// en EV-1981-BOUNDINGTON y es canon -- pero decide cuanta gente sobrevive.
//
// La regla de los tres cierres sale tal cual del documento canonico:
//   pruebas suficientes  -> Venides reune 20 hombres  (Punto de los Creadores)
//   algo, pero no basta  -> Venides reune 10 hombres  (Punto de Sho-Noco)
//   nada                 -> Venides estara solo
//
// GL-free: no abre ventana. Corre en CI.

#include <cstdio>
#include <string>
#include <vector>

#include "Check.h"
#include "RPG/NarrativeEngine.h"

using RPG::AdventureScript;
using RPG::NarrativeEngine;
using RPG::NarrativeState;

namespace {

const char* kRuta = "assets/adventures/boundington_primer_dia.json";

// Imprime lo que devuelve un disparo y dice si hubo beat.
bool suelta(const RPG::NarrativeResult& r, const char* que) {
    if (!r.fired) return false;
    std::printf("    [%s]\n", que);
    if (!r.speaker.empty()) std::printf("      %s:\n", r.speaker.c_str());
    for (const auto& l : r.lines) std::printf("        %s\n", l.c_str());
    return true;
}

int objetivosHechos(const NarrativeEngine& e, const NarrativeState& s) {
    int n = 0;
    for (const auto& o : e.objectives(s)) {
        if (o.done) ++n;
    }
    return n;
}

}  // namespace

int main() {
    // ---------------------------------------------------------------
    // Ruta 1 — el que lo pregunta todo. Punto de los Creadores.
    // ---------------------------------------------------------------
    std::printf("=== RUTA 1: el forastero que pregunta ===\n");
    {
        auto cargada = AdventureScript::loadFromFile(kRuta);
        require(cargada);
        const AdventureScript guion = cargada.value();
        NarrativeEngine e;
        e.setAdventure(&guion);
        NarrativeState s;

        require(suelta(e.enterLevel("assets/levels/ciudad_centro.json", s), "llegada"));
        require(suelta(e.talkTo("skilla", s), "Skilla: la cama"));
        require(suelta(e.talkTo("skilla", s), "Skilla: la Furia de Alaro"));
        require(suelta(e.talkTo("aigren", s), "Aigren: el hierro"));
        require(suelta(e.talkTo("luisarda", s), "Luisarda: las antorchas"));
        require(suelta(e.talkTo("xila", s), "Xila: la iglesia sin techo"));
        require(suelta(e.enterLevel("assets/levels/interior_iglesia.json", s), "la prueba"));
        require(s.hasFlag("bnd_pruebas_suficientes"));

        require(suelta(e.talkTo("venides", s), "Venides: con pruebas"));
        require(s.hasFlag("bnd_venides_20"));

        require(suelta(e.talkTo("skilla", s), "Skilla: a dormir"));
        require(suelta(e.tick(s), "CIERRE"));
        require(s.hasFlag("bnd_dia_cerrado"));
        std::printf("    objetivos cumplidos: %d de 4\n\n", objetivosHechos(e, s));
        require(objetivosHechos(e, s) == 4);
    }

    // ---------------------------------------------------------------
    // Ruta 2 — el que oye algo pero no lo prueba. Punto de Sho-Noco.
    // ---------------------------------------------------------------
    std::printf("=== RUTA 2: el forastero que oye rumores ===\n");
    {
        auto cargada = AdventureScript::loadFromFile(kRuta);
        require(cargada);
        const AdventureScript guion = cargada.value();
        NarrativeEngine e;
        e.setAdventure(&guion);
        NarrativeState s;

        require(suelta(e.enterLevel("assets/levels/ciudad_centro.json", s), "llegada"));
        require(suelta(e.talkTo("skilla", s), "Skilla: la cama"));
        require(suelta(e.talkTo("skilla", s), "Skilla: la Furia de Alaro"));
        require(suelta(e.talkTo("parroquiano_humilde", s), "el parroquiano"));
        require(suelta(e.talkTo("sectario_perdido", s), "un sectario"));
        require(!s.hasFlag("bnd_pruebas_suficientes"));   // rumores no son pruebas

        require(suelta(e.talkTo("venides", s), "Venides: solo rumores"));
        require(s.hasFlag("bnd_venides_10"));
        require(!s.hasFlag("bnd_venides_20"));

        require(suelta(e.talkTo("skilla", s), "Skilla: a dormir"));
        require(suelta(e.tick(s), "CIERRE"));
        require(s.hasFlag("bnd_dia_cerrado"));
        std::printf("\n");
    }

    // ---------------------------------------------------------------
    // Ruta 3 — el que se va a dormir. Venides estara solo.
    // ---------------------------------------------------------------
    std::printf("=== RUTA 3: el forastero que se acuesta temprano ===\n");
    {
        auto cargada = AdventureScript::loadFromFile(kRuta);
        require(cargada);
        const AdventureScript guion = cargada.value();
        NarrativeEngine e;
        e.setAdventure(&guion);
        NarrativeState s;

        require(suelta(e.enterLevel("assets/levels/ciudad_centro.json", s), "llegada"));
        require(suelta(e.talkTo("skilla", s), "Skilla: la cama"));
        require(suelta(e.talkTo("skilla", s), "Skilla: la Furia de Alaro"));
        require(suelta(e.talkTo("skilla", s), "Skilla: a dormir"));
        require(suelta(e.tick(s), "CIERRE"));

        require(s.hasFlag("bnd_dia_cerrado"));
        require(!s.hasFlag("bnd_aviso_dado"));
        require(!s.hasFlag("bnd_venides_20"));
        require(!s.hasFlag("bnd_venides_10"));
        std::printf("\n");
    }

    // ---------------------------------------------------------------
    // Un beat que no se dispara nunca es contenido muerto: se escribio,
    // se reviso y no lo ve nadie. Vale la pena comprobarlo aqui, que es
    // barato, en vez de descubrirlo dentro de un ano.
    // ---------------------------------------------------------------
    std::printf("=== cobertura: ningun beat muerto ===\n");
    {
        auto cargada = AdventureScript::loadFromFile(kRuta);
        require(cargada);
        const AdventureScript guion = cargada.value();
        NarrativeEngine e;
        e.setAdventure(&guion);
        NarrativeState s;
        std::vector<std::string> vistos;
        auto anota = [&](const RPG::NarrativeResult& r) {
            if (r.fired && !r.beatId.empty()) vistos.push_back(r.beatId);
        };

        // recorrido exhaustivo: todo con todos, dos veces, y cierre
        anota(e.enterLevel("assets/levels/ciudad_centro.json", s));
        anota(e.enterLevel("assets/levels/interior_castillo.json", s));
        for (int vuelta = 0; vuelta < 2; ++vuelta) {
            for (const char* npc : {"skilla", "parroquiano_humilde", "aigren",
                                    "luisarda", "xila", "sectario_perdido"}) {
                anota(e.talkTo(npc, s));
            }
        }
        anota(e.enterLevel("assets/levels/interior_iglesia.json", s));
        anota(e.talkTo("venides", s));
        anota(e.talkTo("skilla", s));
        anota(e.tick(s));

        std::printf("    beats disparados en el recorrido largo: %d\n",
                    static_cast<int>(vistos.size()));
        for (const auto& b : vistos) std::printf("      %s\n", b.c_str());
        // 13 de los 16: los otros tres son los cierres alternativos, que por
        // definicion se excluyen entre si (una partida solo ve uno).
        require(vistos.size() >= 13);
    }

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
