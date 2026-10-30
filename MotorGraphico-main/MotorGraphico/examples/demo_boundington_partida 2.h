// demo_boundington_partida.h — el jugador de las demos de Boundington.
//
// DIALOGOS-01 (13/10/2026): con los dialogos nuevos casi todo es una CHARLA
// (temas que se preguntan en el orden que se quiera, paginas y una salida), y
// una partida ya no puede ser una lista de clics. Es un JUGADOR que, cada vez
// que tiene que contestar:
//   1. coge la primera respuesta de SU lista que este en pantalla (y que no
//      haya usado ya en esa misma pantalla);
//   2. si no hay ninguna, pasa de pagina («Hablar de otra cosa»);
//   3. si ya ha pasado por todas, se despide (la salida de la charla) o, si
//      no la hay, coge la ultima.
// Asi una ruta es «que haces» (donde vas, con quien hablas) y «que dices» (lo
// que prefieres contestar), y aguanta que se anadan temas a una charla.
//
// Lo usan demo_campana_boundington (las cuatro aventuras seguidas, con
// nombre) y demo_boundington (las escenas, una a una). Las tiradas Nd6 y los
// combates se resuelven segun la suerte de la ruta: lo que se prueba es la
// ramificacion, no el azar. La cobertura de todos los beats, los atascos y la
// moral los mide explorar_boundington, que juega al azar.
#pragma once

#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "Core/Check.h"
#include "Game/Cinematica.h"  // PROLOGO-02
#include "RPG/NarrativeEngine.h"

namespace demo_bnd {

using RPG::AdventureScript;
using RPG::BattleRequest;
using RPG::EffectType;
using RPG::NarrativeBeat;
using RPG::NarrativeEffect;
using RPG::NarrativeEngine;
using RPG::NarrativeResult;
using RPG::NarrativeState;
using RPG::OpcionDeDialogo;
using RPG::SkillCheckRequest;
using RPG::TablaAleatoriaRequest;

// Los cuatro grados del Nd6 (GDD 7.1). Con Critica/Buena se ganan los
// combates; con Regular/Mala, se pierden.
enum class Suerte { Critica, Buena, Regular, Mala };

inline const char* nombre(Suerte s) {
    switch (s) {
        case Suerte::Critica: return "critica";
        case Suerte::Buena: return "buena";
        case Suerte::Regular: return "regular";
        case Suerte::Mala: return "mala";
    }
    return "?";
}

struct Guiones {
    AdventureScript prologo, dia1, dia2, ocaso;
    const AdventureScript& por(const std::string& n) const {
        if (n == "prologo") return prologo;
        if (n == "dia1") return dia1;
        if (n == "dia2") return dia2;
        require(n == "ocaso");
        return ocaso;
    }
};

inline Guiones cargaTodo() {
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

// La despedida de una charla: la marca el guion ("despedida"); en un beat
// que no es charla, la opcion que solo toca la contabilidad de la charla.
inline bool deLibro(const std::string& n) {
    for (const char* fin : {"_en_charla", "_pag", "_abierta", "_visto"}) {
        const std::string f(fin);
        if (n.size() >= f.size() && n.compare(n.size() - f.size(), f.size(), f) == 0) return true;
    }
    return n.find("_n_") != std::string::npos;
}

inline bool esSalida(const OpcionDeDialogo& o) {
    if (o.despedida) return true;
    if (o.sigue) return false;
    for (const NarrativeEffect& e : o.efectos) {
        if (e.type == EffectType::Log) continue;
        const bool libro = e.type == EffectType::SetFlag || e.type == EffectType::ClearFlag ||
                           e.type == EffectType::SetVariable || e.type == EffectType::AddVariable;
        if (!libro || !deLibro(e.arg)) return false;
    }
    return true;
}

// Un jugador: un estado para toda la campana y una forma de contestar.
struct Jugador {
    const Guiones& g;
    NarrativeEngine e;
    NarrativeState s;  // <- UNO SOLO para las cuatro aventuras
    const AdventureScript* guion = nullptr;
    Suerte suerte = Suerte::Buena;
    bool ruido = false;
    std::vector<std::string> prefiere;    // lo que contesta, por orden de gusto
    std::set<std::string> usadas;         // "beat|texto": no repite en la misma pantalla
    std::set<std::string> pierde;         // flagVictory de combates que pierde (una vez cada uno)
    std::vector<std::string>* anotador = nullptr;  // cobertura: cada beat que dispara
    std::vector<std::string> vistos;               // los de esta partida

    Jugador(const Guiones& guiones, Suerte su, const std::vector<std::string>& flagsHeroe = {}, bool r = false)
        : g(guiones), suerte(su), ruido(r) {
        // La creacion de personaje deja estas flags y textos (HeroeEnLaHistoria);
        // aqui se ponen a mano.
        for (const std::string& f : flagsHeroe) s.setFlag(f);
        s.ponTextosHeroe({{"nombre", "Ilse"}, {"clase", "Berserker"}, {"origen", "Mes de la Cosecha"},
                          {"deidad", "Vida"}, {"pueblo", "Huma de Aegroum"}});
    }

    bool buena() const { return suerte == Suerte::Critica || suerte == Suerte::Buena; }

    void fase(const std::string& n) {
        guion = &g.por(n);
        e.setAdventure(guion);  // el motor cambia de libreto; el estado NO se toca
        if (ruido) std::printf("\n  ---- %s ----\n", n.c_str());
    }

    const NarrativeBeat* beat(const std::string& id) const {
        for (const NarrativeBeat& b : guion->beats)
            if (b.id == id) return &b;
        return nullptr;
    }

    void procesa(const NarrativeResult& r) {
        if (!r.fired) return;
        if (!r.beatId.empty()) {
            vistos.push_back(r.beatId);
            if (anotador != nullptr) anotador->push_back(r.beatId);
        }
        if (ruido) {
            if (!r.speaker.empty()) std::printf("      %s:\n", r.speaker.c_str());
            for (const auto& l : r.lines) std::printf("        %s\n", l.c_str());
            for (const auto& l : r.log) std::printf("      · %s\n", l.c_str());
        }
        for (const SkillCheckRequest& c : r.skillChecks) {
            std::string f;
            switch (suerte) {
                case Suerte::Critica: f = c.flagCritical; break;
                case Suerte::Buena: f = c.flagSuccess; break;
                case Suerte::Regular: f = c.flagPartial; break;
                case Suerte::Mala: f = c.flagBotch; break;
            }
            // Las tiradas «de exitos» solo tienen dos salidas.
            if (f.empty()) f = buena() ? c.flagSuccess : c.flagBotch;
            if (!f.empty()) s.setFlag(f);
        }
        for (const BattleRequest& b : r.battles) {
            // ROSITA-01 — Los refuerzos llegan a mitad si se cumple su requires
            // (Game::llegaRefuerzos); un combate que se pierde a proposito
            // («pierde») es el de un golpe que te tumba sin darles tiempo.
            for (const BattleRequest::Refuerzo& rf : b.refuerzos) {
                if (pierde.count(b.flagVictory) != 0 || !rf.requiere.isSatisfiedBy(s)) continue;
                for (const std::string& f : rf.flags) s.setFlag(f);
                if (ruido) std::printf("      [refuerzos]\n");
            }
            // Un combate NO se resuelve en el mismo instante que el beat que lo
            // pide: en el juego real pasan turnos y la flag la enciende
            // syncBattleOutcome. Aqui se simula ese "despues" encendiendola ya.
            const bool gana = buena() && pierde.count(b.flagVictory) == 0;
            pierde.erase(b.flagVictory);
            s.setFlag(gana || b.flagDefeat.empty() ? b.flagVictory : b.flagDefeat);
            if (ruido) std::printf("      [combate: %s]\n", gana ? "victoria" : "derrota");
        }
        for (const TablaAleatoriaRequest& t : r.tablas)
            if (!t.flags.empty()) s.setFlag(buena() ? t.flags.front() : t.flags.back());
    }

    void contesta() {
        const std::vector<std::string> textos = s.eleccion().textos;
        const std::string id = s.eleccion().beatId;
        auto libre = [&](std::size_t i) { return usadas.count(id + "|" + textos[i]) == 0; };
        int k = -1;
        for (std::size_t p = 0; p < prefiere.size() && k < 0; ++p)
            for (std::size_t i = 0; i < textos.size() && k < 0; ++i)
                if (textos[i] == prefiere[p] && libre(i)) k = static_cast<int>(i);
        for (std::size_t i = 0; i < textos.size() && k < 0; ++i)
            if (textos[i] == "Hablar de otra cosa" && libre(i)) k = static_cast<int>(i);
        if (k < 0) {
            const NarrativeBeat* b = beat(id);
            k = static_cast<int>(textos.size()) - 1;
            if (b != nullptr)
                for (std::size_t i = 0; i < textos.size(); ++i)
                    if (esSalida(b->opciones[static_cast<std::size_t>(s.eleccion().indices[i])])) k = static_cast<int>(i);
        }
        const std::string elegida = textos[static_cast<std::size_t>(k)];
        usadas.insert(id + "|" + elegida);
        if (ruido) std::printf("    > %s\n", elegida.c_str());
        procesa(e.elige(k, s));
    }

    // Lo que pasa sin que el jugador haga nada: contestar lo que se pregunte,
    // los beats automaticos y la charla que sigue (DIALOGOS-01).
    void drena() {
        for (int i = 0; i < 150; ++i) {
            if (s.eleccion().activa()) {
                contesta();
                continue;
            }
            NarrativeResult r = e.tick(s);
            if (!r.fired) r = e.sigue(s);
            if (!r.fired) return;
            procesa(r);
        }
        std::printf("    !! drena: 150 pasos seguidos sin parar\n");
        require(false);
    }

    void entra(const std::string& nivel) {
        const std::string ruta = "assets/levels/" + nivel + ".json";
        s.ponLugar(ruta);  // DIALOGOS-01: requires.lugar
        if (ruido) std::printf("\n  [entras en %s]\n", nivel.c_str());
        procesa(e.enterLevel(ruta, s));
        drena();
    }

    NarrativeResult habla(const std::string& npc) {
        if (ruido) std::printf("\n  [hablas con %s]\n", npc.c_str());
        NarrativeResult r = e.talkTo(npc, s);
        procesa(r);
        drena();
        return r;
    }

    // Rosita lleva la hora: «Esperar a que anochezca».
    void anochece() {
        prefiere.insert(prefiere.begin(), "Esperar a que anochezca");
        habla("skilla");
        prefiere.erase(prefiere.begin());
    }

    // PROLOGO-02 — Una cinematica (assets/cinematicas/cinematicas.json): lo
    // que se ve (quien anda, los fundidos) se salta; lo que pide a la historia
    // (sus beats, sus flags, sus niveles), no. `donde`: la celda del jugador,
    // para las que empiezan en una zona. false si no empieza ninguna.
    bool cinematica(const GridCoord& donde) {
        static const Game::CatalogoCinematicas cat = [] {
            auto r = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
            require(r);
            return r.value();
        }();
        const Game::Cinematica* c = cat.laQueEmpieza(s.lugar(), donde, [this](const std::string& f) { return s.hasFlag(f); });
        if (c == nullptr) return false;
        if (ruido) std::printf("\n  [cinematica %s]\n", c->id.c_str());
        for (const Game::PasoCinematica& p : c->pasos) {
            if (p.tipo == Game::PasoCinematica::Tipo::Beat) {
                s.setFlag(p.texto);
                drena();
            } else if (p.tipo == Game::PasoCinematica::Tipo::Flag) {
                s.setFlag(p.texto);
            } else if (p.tipo == Game::PasoCinematica::Tipo::Nivel) {
                std::string n = p.texto.substr(p.texto.rfind('/') + 1);
                entra(n.substr(0, n.size() - 5));  // sin «.json»
            }
        }
        s.setFlag(c->vista());
        for (const std::string& f : c->alTerminar) s.setFlag(f);
        drena();
        return true;
    }

    // El prologo (PROLOGO-02): el prado del rio, el cofre (el juego enciende
    // estas flags al abrirlo y ponerse el kit), el barquero que baja por el rio
    // al llegar a la orilla, la charla con Sho-Noco («No pienso subir ahi», y
    // luego «Subir a la barca») y los cuatro recuerdos de Venides.
    void prologo() {
        fase("prologo");
        entra("prado_del_rio");
        for (const char* f : {"kit_basico_recogido", "kit_basico_puesto", "bnd_prologo_kit_listo"}) s.setFlag(f);
        drena();
        prefiere.insert(prefiere.begin(), {"No pienso subir ahí.", "Subir a la barca"});
        cinematica(GridCoord{12, 9});  // la orilla
        habla("sho_noco");
        prefiere.erase(prefiere.begin(), prefiere.begin() + 2);
        while (cinematica(GridCoord{-1, -1})) {
        }
        require(s.hasFlag("bnd_prologo_terminado"));
    }

    bool visto(const std::string& id) const {
        for (const std::string& v : vistos)
            if (v == id) return true;
        return false;
    }
};

// Una escena con nombre: lo que se comprueba por el camino se apunta, y al
// final se dice todo lo que fallo (y entonces la demo falla).
struct Escena {
    Jugador j;
    std::string titulo;
    std::vector<std::string> fallos;

    Escena(const Guiones& g, const char* t, Suerte su, const std::vector<std::string>& heroe,
           const std::vector<std::string>& dice, std::vector<std::string>* anotador)
        : j(g, su, heroe), titulo(t) {
        j.prefiere = dice;
        j.anotador = anotador;
    }
    void ve(std::initializer_list<const char*> fs) {
        for (const char* f : fs)
            if (!j.s.hasFlag(f)) fallos.push_back(std::string("falta ") + f);
    }
    void nove(std::initializer_list<const char*> fs) {
        for (const char* f : fs)
            if (j.s.hasFlag(f)) fallos.push_back(std::string("sobra ") + f);
    }
    void visto(std::initializer_list<const char*> ids) {
        for (const char* id : ids)
            if (!j.visto(id)) fallos.push_back(std::string("no se vio ") + id);
    }
    void min(const char* var, int n) {
        if (j.s.variable(var) < n)
            fallos.push_back(std::string(var) + "=" + std::to_string(j.s.variable(var)) + " < " + std::to_string(n));
    }
    void pon(std::initializer_list<const char*> fs) {
        for (const char* f : fs) j.s.setFlag(f);
    }
    void var(const char* v, int n) { j.s.setVariable(v, n); }
    void di(std::initializer_list<const char*> ts) {
        j.prefiere.insert(j.prefiere.begin(), ts.begin(), ts.end());
    }
    void fin() {
        std::printf("  %s %s\n", fallos.empty() ? "OK " : "MAL", titulo.c_str());
        for (const std::string& f : fallos) std::printf("        %s\n", f.c_str());
        require(fallos.empty());
    }
};

}  // namespace demo_bnd
