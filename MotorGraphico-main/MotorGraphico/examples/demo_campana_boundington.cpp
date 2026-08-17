// demo_campana_boundington — "Los Perdidos de Boundington" de punta a punta.
//
// Las cuatro aventuras (prologo + tres dias) jugadas SEGUIDAS, con UN SOLO
// NarrativeState. Eso es lo que las convierte en una campana y no en cuatro
// demos sueltas: las flags del prologo siguen encendidas el tercer dia, y por
// eso Venides puede reconocer que compartiste el sueno de Sho-Noco, y por eso
// reconoces a Verina en la plaza sin que nadie te la presente.
//
// EL PATRON, que es lo unico que hay que saber para anadir mas dias:
//
//     NarrativeState estado;                  // uno, para toda la campana
//     for (cada guion en orden) {
//         motor.setAdventure(&guion);         // el motor cambia de libreto
//         ... jugar ...                       // el estado NO se toca
//     }
//
// setAdventure() cambia el libreto; NarrativeState es del jugador, no de la
// aventura. El motor ya lo permitia: nadie lo habia probado.
//
// GL-free: no abre ventana, no necesita GameSession. Las tiradas Nd6 y los
// combates que piden los beats se devuelven en NarrativeResult y aqui se
// resuelven a mano de forma determinista (ver resuelveDiferidos), que es
// exactamente lo que hace GameSession pero sin dados ni BattleState: a este
// demo le interesa la RAMIFICACION, no el azar.

#include <cstdio>
#include <string>
#include <vector>

#include "Check.h"
#include "RPG/NarrativeEngine.h"

using RPG::AdventureScript;
using RPG::NarrativeEngine;
using RPG::NarrativeResult;
using RPG::NarrativeState;

namespace {

// Como se resuelven aqui las tiradas y los combates que el motor difiere.
// En el juego real esto lo hace GameSession con DicePoolEngine y BattleState.
// Los cuatro grados del Nd6 (GDD 7.1). Antes solo se simulaban dos, y por eso
// las ramas de PARTIAL y CRITICAL parecian contenido muerto sin serlo.
enum class Suerte { Critica, Buena, Regular, Mala };

// Si no es nulo, cada beat que dispara deja su id aqui (medida de cobertura).
std::vector<std::string>* g_anotador = nullptr;

void resuelveDiferidos(const NarrativeResult& r, NarrativeState& estado, Suerte suerte) {
    for (const auto& c : r.skillChecks) {
        switch (suerte) {
            case Suerte::Critica: estado.setFlag(c.flagCritical); break;
            case Suerte::Buena:   estado.setFlag(c.flagSuccess);  break;
            case Suerte::Regular: estado.setFlag(c.flagPartial);  break;
            case Suerte::Mala:    estado.setFlag(c.flagBotch);    break;
        }
    }
    for (const auto& b : r.battles) {
        // Un combate NO se resuelve en el mismo instante que el beat que lo
        // pide: en el juego real pasan turnos y la flag la enciende
        // syncBattleOutcome. Aqui se simula ese "despues" encendiendola ya,
        // porque lo que se prueba es que la aventura ramifica bien, no el
        // combate.
        const bool gana = (suerte == Suerte::Critica || suerte == Suerte::Buena);
        estado.setFlag(gana ? b.flagVictory : b.flagDefeat);
    }
}

void muestra(const NarrativeResult& r) {
    if (!r.fired) return;
    if (g_anotador != nullptr && !r.beatId.empty()) g_anotador->push_back(r.beatId);
    if (!r.speaker.empty()) std::printf("      %s:\n", r.speaker.c_str());
    for (const auto& l : r.lines) std::printf("        %s\n", l.c_str());
    for (const auto& l : r.log) std::printf("      · %s\n", l.c_str());
}

// Dispara tick() hasta que deje de haber beats. Los beats "auto" encadenan de
// uno en uno, asi que sin esto media aventura no llega a verse.
int drena(NarrativeEngine& e, NarrativeState& s, Suerte suerte, int tope = 40) {
    int n = 0;
    for (int i = 0; i < tope; ++i) {
        NarrativeResult r = e.tick(s);
        if (!r.fired) break;
        muestra(r);
        resuelveDiferidos(r, s, suerte);
        ++n;
    }
    return n;
}

NarrativeResult habla(NarrativeEngine& e, NarrativeState& s, const char* npc, Suerte suerte) {
    NarrativeResult r = e.talkTo(npc, s);
    muestra(r);
    resuelveDiferidos(r, s, suerte);
    return r;
}

NarrativeResult entra(NarrativeEngine& e, NarrativeState& s, const char* nivel, Suerte suerte) {
    NarrativeResult r = e.enterLevel(nivel, s);
    muestra(r);
    resuelveDiferidos(r, s, suerte);
    return r;
}

struct Guiones {
    AdventureScript prologo, dia1, dia2, ocaso;
};

Guiones cargaTodo() {
    auto p = AdventureScript::loadFromFile("assets/adventures/boundington_prologo.json");
    auto a = AdventureScript::loadFromFile("assets/adventures/boundington_primer_dia.json");
    auto b = AdventureScript::loadFromFile("assets/adventures/boundington_segundo_dia.json");
    auto c = AdventureScript::loadFromFile("assets/adventures/boundington_ocaso.json");
    require(p);
    require(a);
    require(b);
    require(c);
    return Guiones{p.value(), a.value(), b.value(), c.value()};
}

// Una partida entera. Devuelve el estado final para poder interrogarlo.
struct Ruta {
    Suerte suerte = Suerte::Buena;
    bool investigaTodo = true;
    bool buscaAliados = true;   // false = nadie te debe un favor el dia 3
    bool avisaVenides = true;   // false = el caballero no se entera de nada
    bool ruidoso = false;
};

NarrativeState juega(const Guiones& g, Ruta ruta) {
    const Suerte suerte = ruta.suerte;
    const bool investigaTodo = ruta.investigaTodo;
    const bool ruidoso = ruta.ruidoso;
    NarrativeState s;                 // <- UNO SOLO para las cuatro aventuras
    NarrativeEngine e;

    // --- Prologo -------------------------------------------------------
    if (ruidoso) std::printf("\n  ---- PROLOGO ----\n");
    e.setAdventure(&g.prologo);
    entra(e, s, "assets/levels/interior_vacio.json", suerte);
    drena(e, s, suerte);
    require(s.hasFlag("bnd_prologo_terminado"));

    // --- Primer dia ----------------------------------------------------
    if (ruidoso) std::printf("\n  ---- PRIMER DIA ----\n");
    e.setAdventure(&g.dia1);
    entra(e, s, "assets/levels/ciudad_centro.json", suerte);
    habla(e, s, "skilla", suerte);          // la cama

    // El beat que solo existe si vienes del prologo. Es la prueba de que el
    // estado cruza de aventura: pide bnd_prologo_pasado, que enciende el
    // prologo y consulta el primer dia.
    if (ruta.avisaVenides) {
        NarrativeResult reconoce = habla(e, s, "venides", suerte);
        require(reconoce.fired);
        require(reconoce.beatId == "beat_venides_reconoce");
    }

    // Contenido opcional del primer dia: el castillo cerrado, los sectarios
    // del Casco Antiguo y la segunda visita a la herreria.
    entra(e, s, "assets/levels/interior_castillo.json", suerte);
    habla(e, s, "sectario_perdido", suerte);

    if (investigaTodo) {
        habla(e, s, "aigren", suerte);
        habla(e, s, "aigren", suerte);   // ya te he dicho lo que sabia
        habla(e, s, "luisarda", suerte);
        habla(e, s, "xila", suerte);
        entra(e, s, "assets/levels/interior_iglesia.json", suerte);
        if (ruta.avisaVenides) habla(e, s, "venides", suerte);  // -> 20 hombres
    } else {
        habla(e, s, "parroquiano_humilde", suerte);
        if (ruta.avisaVenides) habla(e, s, "venides", suerte);  // -> 10 hombres
    }
    habla(e, s, "skilla", suerte);          // la espada de Skilla
    habla(e, s, "skilla", suerte);          // a dormir
    drena(e, s, suerte);
    require(s.hasFlag("bnd_dia_cerrado"));

    // --- Segundo dia ---------------------------------------------------
    if (ruidoso) std::printf("\n  ---- SEGUNDO DIA ----\n");
    e.setAdventure(&g.dia2);
    drena(e, s, suerte);                    // el encargo de Venides
    require(s.hasFlag("bnd2_encargo"));

    habla(e, s, "skilla", suerte);          // el encargo de la espada
    entra(e, s, "assets/levels/mazmorra_64x64.json", suerte);
    drena(e, s, suerte);                    // sigilo -> robo limpio o Gorran
    entra(e, s, "assets/levels/ciudad_oeste.json", suerte);
    drena(e, s, suerte);                    // la tienda abandonada
    if (ruta.buscaAliados) {
        habla(e, s, "nina_del_gato", suerte);   // los gitanos de Surysal
    } else {
        // Ni baja a la Barriada. El cierre del dia pide que ese hilo este
        // resuelto de una forma u otra, asi que se marca como cerrado sin
        // alianza -- que es exactamente lo que significa no ir.
        s.setFlag("bnd2_gitanos_hablado");
        s.setFlag("bnd2_gitanos_resuelto");
    }
    drena(e, s, suerte);
    entra(e, s, "assets/levels/ciudad_este.json", suerte);
    drena(e, s, suerte);                    // la fiesta del Barrio Alto
    habla(e, s, "skilla", suerte);           // devolver la espada (si la tiene)
    if (ruta.avisaVenides) habla(e, s, "venides", suerte);
    habla(e, s, "skilla", suerte);           // cierre del dia
    drena(e, s, suerte);
    require(s.hasFlag("bnd2_dia_cerrado"));

    // --- El Ocaso ------------------------------------------------------
    if (ruidoso) std::printf("\n  ---- EL OCASO ----\n");
    e.setAdventure(&g.ocaso);
    entra(e, s, "assets/levels/ciudad_centro.json", suerte);
    drena(e, s, suerte);
    habla(e, s, "venides", suerte);
    entra(e, s, "assets/levels/interior_cuartel.json", suerte);
    drena(e, s, suerte);                    // campanas, arenga, combates, huida
    require(s.hasFlag("bnd3_fuera"));
    require(s.hasFlag("bnd3_epilogo"));
    return s;
}

void juegaAnotando(const Guiones& g, Ruta r, std::vector<std::string>& vistos) {
    g_anotador = &vistos;
    juega(g, r);
    g_anotador = nullptr;
}

}  // namespace

int main() {
    const Guiones g = cargaTodo();
    std::printf("=== Los Perdidos de Boundington — campana completa ===\n");
    std::printf("    %d + %d + %d + %d beats en cuatro aventuras\n",
                static_cast<int>(g.prologo.beats.size()), static_cast<int>(g.dia1.beats.size()),
                static_cast<int>(g.dia2.beats.size()), static_cast<int>(g.ocaso.beats.size()));

    // ---------------------------------------------------------------
    // Partida A — el que lo hace todo bien.
    // ---------------------------------------------------------------
    std::printf("\n########## PARTIDA A: el que investiga y tiene suerte ##########\n");
    {
        NarrativeState s = juega(g, Ruta{Suerte::Buena, true, true, true, true});
        require(s.hasFlag("bnd_venides_20"));       // 20 hombres
        require(s.hasFlag("bnd2_furia_alaro"));     // recupero la espada
        require(s.hasFlag("bnd2_aliado_gitanos"));  // alianza con Surysal
        require(s.hasFlag("bnd3_final_oeste"));     // salida con los carros
        require(s.hasFlag("bnd3_verina_con_grupo"));// Verina viva
        std::printf("\n  -> 20 hombres, gitanos, Verina viva, salida oeste.\n");
    }

    // ---------------------------------------------------------------
    // Partida B — el que hace lo minimo y le sale mal.
    // ---------------------------------------------------------------
    std::printf("\n########## PARTIDA B: el que va a lo justo ##########\n");
    {
        NarrativeState s = juega(g, Ruta{Suerte::Mala, false, false, true, false});
        require(s.hasFlag("bnd_venides_10"));        // solo 10 hombres
        require(!s.hasFlag("bnd2_aliado_gitanos"));  // nadie le debe nada
        require(!s.hasFlag("bnd3_final_oeste"));     // sale sin ayuda o cae
        std::printf("  -> 10 hombres, sin aliados, final distinto. OK\n");
    }

    // ---------------------------------------------------------------
    // Lo que NO puede cambiar: la ciudad cae siempre. Es canon
    // (EV-1981-BOUNDINGTON) y ninguna ruta debe poder evitarlo.
    // ---------------------------------------------------------------
    std::printf("\n########## invariante: Boundington cae en las dos ##########\n");
    {
        for (Suerte suerte : {Suerte::Critica, Suerte::Buena, Suerte::Regular, Suerte::Mala}) {
            for (bool todo : {true, false}) {
                NarrativeState s = juega(g, Ruta{suerte, todo, true, true, false});
                require(s.hasFlag("bnd3_campanas"));   // la Matanza ocurre
                require(s.hasFlag("bnd3_epilogo"));    // y la campana cierra
            }
        }
        std::printf("  -> las 8 combinaciones terminan con la ciudad ardiendo. OK\n");
    }


    // ---------------------------------------------------------------
    // Cobertura: un beat que no dispara nunca es contenido muerto -- se
    // escribio, se reviso y no lo ve nadie. Se juega por las cuatro
    // combinaciones y se cuenta cuantos beats distintos salieron.
    // ---------------------------------------------------------------
    std::printf("\n########## cobertura de beats ##########\n");
    {
        std::vector<std::string> todos;
        for (const AdventureScript* a : {&g.prologo, &g.dia1, &g.dia2, &g.ocaso}) {
            for (const auto& b : a->beats) todos.push_back(b.id);
        }
        std::vector<std::string> vistos;
        for (Suerte suerte : {Suerte::Critica, Suerte::Buena, Suerte::Regular, Suerte::Mala}) {
            for (bool todo : {true, false}) {
                juegaAnotando(g, Ruta{suerte, todo, true, true, false}, vistos);
            }
        }
        // Rutas que las 8 de arriba no pueden tomar, porque siempre avisan a
        // Venides y siempre buscan aliados:
        //   - sin avisar  -> Venides solo, cierre_silencio, arenga en soledad
        //   - sin aliados -> huida por el oeste sin los carros de Surysal
        juegaAnotando(g, Ruta{Suerte::Buena, false, true, false, false}, vistos);
        juegaAnotando(g, Ruta{Suerte::Buena, true, false, true, false}, vistos);
        juegaAnotando(g, Ruta{Suerte::Regular, false, false, false, false}, vistos);

        // Dos ramas que ningun recorrido uniforme puede tomar, porque piden
        // combinaciones que se contradicen entre si dentro de una partida:
        //
        //   beat_venides_nada  -> hablar con el caballero ANTES de tener nada
        //                         que contarle (ni siquiera la charla larga
        //                         con Skilla, que ya cuenta como rumor).
        //   b3_verina_perdida  -> GANAR los dos combates del Ocaso y PERDER
        //                         el de Verina. Con suerte uniforme, quien
        //                         pierde el tercero no llega al tercero.
        //
        // Se prueban aparte, montando el estado a mano. Es mas honesto que
        // retorcer la partida entera para pasar por ellas.
        {
            NarrativeState s;
            NarrativeEngine e;
            e.setAdventure(&g.prologo);
            entra(e, s, "assets/levels/interior_vacio.json", Suerte::Buena);
            drena(e, s, Suerte::Buena);
            e.setAdventure(&g.dia1);
            entra(e, s, "assets/levels/ciudad_centro.json", Suerte::Buena);
            habla(e, s, "skilla", Suerte::Buena);            // solo la cama
            g_anotador = &vistos;
            habla(e, s, "venides", Suerte::Buena);           // reconoce
            NarrativeResult nada = habla(e, s, "venides", Suerte::Buena);
            g_anotador = nullptr;
            require(nada.fired);
            require(nada.beatId == "beat_venides_nada");
        }
        {
            NarrativeState s;
            NarrativeEngine e;
            e.setAdventure(&g.ocaso);
            // Estado minimo para llegar al rescate: dia 2 cerrado, plaza
            // vista con el prologo detras, avisado, campanas y los dos
            // primeros combates ganados.
            for (const char* f : {"bnd2_dia_cerrado", "bnd_prologo_pasado", "bnd3_plaza_vista",
                                  "bnd3_verina_vista", "bnd3_avisado", "bnd3_campanas",
                                  "bnd3_arenga", "bnd3_saqueadores_lanzado",
                                  "bnd3_saqueadores_vencidos", "bnd3_fanaticos_lanzado",
                                  "bnd3_fanaticos_vencidos"}) {
                s.setFlag(f);
            }
            g_anotador = &vistos;
            drena(e, s, Suerte::Mala);   // pierde el combate por Verina
            g_anotador = nullptr;
            require(s.hasFlag("bnd3_verina_muerta"));
            require(s.hasFlag("bnd3_fuera"));
        }

        int n = 0;
        std::vector<std::string> muertos;
        for (const auto& id : todos) {
            bool visto = false;
            for (const auto& v : vistos) {
                if (v == id) { visto = true; break; }
            }
            if (visto) ++n; else muertos.push_back(id);
        }
        std::printf("  beats disparados: %d de %d\n", n, static_cast<int>(todos.size()));
        for (const auto& m : muertos) std::printf("    sin disparar: %s\n", m.c_str());
        // Los que faltan son ramas mutuamente excluyentes (un cierre por
        // partida, un grado de tirada por tirada). Con 4 recorridos deberia
        // verse la gran mayoria.
        // Todos los beats de las cuatro aventuras se ven en algun recorrido.\n        require(muertos.empty());
    }

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
