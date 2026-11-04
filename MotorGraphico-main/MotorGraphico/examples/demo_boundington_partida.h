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

#include <algorithm>
#include <cstdio>
#include <cstdlib>  // getenv (BND_RUIDO, solo para diagnosticar)
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Core/Check.h"
#include "Game/Cinematica.h"  // PROLOGO-02
#include "RPG/NarrativeEngine.h"
#include "RPG/Narrative/EventosDelMundo.h"  // H4: el Ocaso se pone en marcha con eventos del mundo

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
// combates; con Regular/Mala, se pierden (D0027: perder = huir; morir solo reinicia el combate).
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
    // GORRAN-DIA2 (22/10): el dia 2 es la montaña de Gorran (boundington_dia_gorran.json,
    // flags bndg_*/bnd2_*) y el antiguo dia 2 (boundington_segundo_dia.json) es ya el DIA 3.
    // Los nombres "dia2" de las demos se quedan para este ultimo, como en Application.cpp (m_dia2).
    AdventureScript prologo, dia1, gorran, dia2, ocaso;
    const AdventureScript& por(const std::string& n) const {
        if (n == "prologo") return prologo;
        if (n == "dia1") return dia1;
        if (n == "dia_gorran") return gorran;
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
    auto d = AdventureScript::loadFromFile("assets/adventures/boundington_dia_gorran.json");
    require(p);
    require(a);
    require(b);
    require(c);
    require(d);
    return Guiones{p.value(), a.value(), d.value(), b.value(), c.value()};
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

// E-MATRIZ-01b — Lo que el motor narrativo NO aplica por si mismo y que en el juego es de GameSession::applyNarrative:
// el oro, los niveles y hitos, los actos (la moral) y los objetos que da un beat. El arnes lo APUNTA, en orden, junto
// al NarrativeState; no lo aplica al jugar (ningun guion lee oro, moral ni nivel, y asi ninguna demo cambia de camino).
// Quien quiera las cuentas (fabricar_guardados.h: aplicaLasCuentas) las vuelca en una sesion con el heroe, usando lo que
// el juego usa (moral::apuntaActos / asientaNoche, CharacterSheet::grantLevelPoints): applyNarrative es privada.
struct Cuentas {
    static constexpr int kBolsaInicial = 150;  // la del juego al empezar (Application.cpp: addGold(150) tras cargar el primer nivel)
    struct Paso {
        std::string beat;
        int oro = 0, puntos = 0, hitos = 0;
        std::vector<std::string> actos, objetos, descansos;
    };
    std::vector<Paso> pasos;

    void anota(const NarrativeResult& r) {
        if (r.goldDelta == 0 && r.levelPoints == 0 && r.milestones == 0 && r.actos.empty() && r.objetos.empty() && r.descansos.empty())
            return;
        pasos.push_back({r.beatId, r.goldDelta, r.levelPoints, r.milestones, r.actos, r.objetos, r.descansos});
    }
};

// Un jugador: un estado para toda la campana y una forma de contestar.
struct Jugador {
    const Guiones& g;
    NarrativeEngine e;
    NarrativeState s;  // <- UNO SOLO para las cuatro aventuras
    Cuentas cuentas;   // E-MATRIZ-01b: oro, niveles, actos y objetos (al lado del estado; ver Cuentas)
    const AdventureScript* guion = nullptr;
    Suerte suerte = Suerte::Buena;
    bool ruido = false;
    bool cinesAlEntrar = false;
    bool vigilaSiempre = false;  // E-MISIONES-01b: como en el juego, un beat que salta sin cambiar nada no se repite en bucle (ver drena); lo pide quien muta el guion
    std::vector<std::string> zonasPrimero;  // cinematicas con zona a las que va antes que a las demas (pisaUnaZona)
    std::vector<std::string> zonasEvita;    // ... y a las que no va nunca
    // E-DISPARADORES-3: la salida que elige en el dialogo de la base (b3_por_donde): base_ruta_oeste / _norte / _quedarse.
    std::string rutaOcaso = "base_ruta_oeste";
    bool enCine = false;  // dentro de cinematica(): no se encadenan otras desde drena()
    std::vector<std::string> prefiere;    // lo que contesta, por orden de gusto
    std::set<std::string> usadas;         // "beat|texto": no repite en la misma pantalla
    std::set<std::string> pierde;         // flagVictory de combates que «pierde» (una vez cada uno): D0027, es huir
    int ineludiblesGanados = 0;           // combates ineludibles cuyo escenario pedia perder: se ganan (no se huye)
    std::map<std::string, Suerte> tiradas;  // E-MATRIZ-01b: la suerte que sale en UNA tirada (por su flagBotch); las demas, `suerte`
    std::vector<std::string>* anotador = nullptr;  // cobertura: cada beat que dispara
    std::vector<std::string> vistos;               // los de esta partida
    // E-DIA1-CONSEC: lo que el jugador ha leido (lineas de beats y de respuestas, con {nombre} ya puesto), el dado rival de cada
    // tirada pedida ("beat/habilidad") y los combates de los que huye (por su flagVictory: sale su flagHuida). Las demos no lo miran.
    std::vector<std::string> dicho;
    // E-MISIONES-01b: traza de lo que ve el jugador (beat y lineas, menu y opcion elegida) y de cuando se enciende una de las flags de `trazaFlags`.
    // Sirve para escribir los guiones de captura de los cierres (tools/guiones/q1_*_cierre.txt). Las demos no la piden.
    std::vector<std::string>* traza = nullptr;
    std::vector<std::string> trazaFlags;
    std::set<std::string> trazaVistas;
    void trazaCierre() {
        if (traza == nullptr) return;
        for (const std::string& f : trazaFlags)
            if (s.hasFlag(f) && trazaVistas.insert(f).second) traza->push_back("CIERRE " + f);
    }
    std::vector<std::pair<std::string, int>> dados;
    std::set<std::string> huyen;
    bool dijo(const std::string& trozo) const {
        for (const std::string& l : dicho)
            if (l.find(trozo) != std::string::npos) return true;
        return false;
    }

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
        // Desde el dia de Gorran, al entrar en un nivel arrancan solas las cinematicas sin zona
        // (castillo_venides, taberna_velas, plaza_sermon...) como en el juego.
        cinesAlEntrar = n == "dia_gorran" || n == "dia2" || n == "ocaso";
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
        cuentas.anota(r);
        for (const std::string& l : r.lines) dicho.push_back(l);
        if (traza != nullptr) traza->push_back("BEAT " + r.beatId + " [" + r.speaker + "] lineas=" + std::to_string(r.lines.size()) + (r.lines.empty() ? std::string() : " «" + r.lines.front().substr(0, 60) + "»"));
        for (const SkillCheckRequest& c : r.skillChecks) dados.emplace_back(r.beatId + "/" + c.skillId, c.rivalDice);
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
            const auto forzada = tiradas.find(c.flagBotch);
            const Suerte su = forzada != tiradas.end() ? forzada->second : suerte;
            switch (su) {
                case Suerte::Critica: f = c.flagCritical; break;
                case Suerte::Buena: f = c.flagSuccess; break;
                case Suerte::Regular: f = c.flagPartial; break;
                case Suerte::Mala: f = c.flagBotch; break;
            }
            // Las tiradas «de exitos» solo tienen dos salidas.
            if (f.empty()) f = (su == Suerte::Critica || su == Suerte::Buena) ? c.flagSuccess : c.flagBotch;
            if (!f.empty()) s.setFlag(f);
        }
        for (const BattleRequest& b : r.battles) {
            // ROSITA-01 — Los refuerzos llegan a mitad si se cumple su requires
            // (Game::llegaRefuerzos); un combate que se pierde a proposito
            // («pierde») es el de un golpe que te tumba sin darles tiempo.
            for (const BattleRequest::Refuerzo& rf : b.refuerzos) {
                if (pierde.count(b.flagVictory) != 0 || huyen.count(b.flagVictory) != 0 || !rf.requiere.isSatisfiedBy(s)) continue;
                for (const std::string& f : rf.flags) s.setFlag(f);
                if (ruido) std::printf("      [refuerzos]\n");
            }
            // Un combate NO se resuelve en el mismo instante que el beat que lo
            // pide: en el juego real pasan turnos y la flag la enciende
            // syncBattleOutcome. Aqui se simula ese "despues" encendiendola ya.
            // D0027 (E-REVIVIR-01): morir NO enciende flagDefeat (el combate vuelve a empezar); la rama de
            // «perder» solo se alcanza huyendo, que enciende flagHuida o, sin ella, flagDefeat (como
            // GameSession::syncBattleOutcome). De un combate ineludible no se huye: se acaba ganando.
            const bool huye = huyen.count(b.flagVictory) != 0 && !b.flagHuida.empty();  // E-DIA1-CONSEC
            const bool gana = !huye && buena() && pierde.count(b.flagVictory) == 0;
            pierde.erase(b.flagVictory);
            huyen.erase(b.flagVictory);
            // D0027 §6: en una pelea sinMatar (nadie muere) perder sigue siendo perder: te levantas y flagDefeat.
            if (b.sinMatar) {
                const bool pierdeSinMatar = !gana;
                s.setFlag(!pierdeSinMatar || b.flagDefeat.empty() ? b.flagVictory : (huye ? b.flagHuida : b.flagDefeat));
                if (ruido) std::printf("      [combate: %s]\n", pierdeSinMatar ? "derrota (sin matar)" : "victoria");
                continue;
            }
            const bool seVa = !gana && !b.ineludible;
            const std::string& salida = !seVa ? b.flagVictory : !b.flagHuida.empty() ? b.flagHuida : b.flagDefeat.empty() ? b.flagVictory : b.flagDefeat;
            if (!gana && b.ineludible) ++ineludiblesGanados;  // la rama «perdida» de un ineludible no existe
            s.setFlag(salida);
            if (ruido) std::printf("      [combate: %s]\n", seVa ? "huida" : "victoria");
        }
        for (const TablaAleatoriaRequest& t : r.tablas)
            if (!t.flags.empty()) s.setFlag(buena() ? t.flags.front() : t.flags.back());
        trazaCierre();
    }

    void contesta() {
        const std::vector<std::string> textos = s.eleccion().textos;
        const std::string id = s.eleccion().beatId;
        auto libre = [&](std::size_t i) { return usadas.count(id + "|" + textos[i]) == 0; };
        int k = -1;
        if (id.rfind("b3_por_donde", 0) == 0) {  // la salida: la opcion de rutaOcaso
            const char* texto = rutaOcaso == "base_ruta_norte" ? "Ir por el paso norte" : rutaOcaso == "base_ruta_quedarse" ? "Mantener la puerta" : "Salir por el oeste";
            for (std::size_t i = 0; i < textos.size() && k < 0; ++i)
                if (textos[i] == texto) k = static_cast<int>(i);
        }
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
        if (traza != nullptr) {
            std::string menu;
            for (std::size_t i = 0; i < textos.size(); ++i) menu += (menu.empty() ? "" : " | ") + std::to_string(i + 1) + ". " + textos[i];
            traza->push_back("MENU " + id + " -> " + std::to_string(k + 1) + "  {" + menu + "}");
        }
        if (ruido) std::printf("    > %s\n", elegida.c_str());
        procesa(e.elige(k, s));
    }

    // C8-03c: GameSession::avaluaEventosDelMundo() una vez por fotograma. Aqui, cada vez que la
    // historia se queda quieta. true si algo salto (y entonces hay que volver a mirar los beats).
    bool evaluaEventos() {
        static const RPG::eventos::Catalogo cat = [] {
            auto r = RPG::eventos::cargaCatalogo("assets/eventos/boundington.json");
            require(r);
            return r.value();
        }();
        return !RPG::eventos::evalua(cat, s).empty();
    }

    // Lo que pasa sin que el jugador haga nada: contestar lo que se pregunte,
    // los beats automaticos y la charla que sigue (DIALOGOS-01).
    void drena() {
        std::string ultimoAuto;
        for (int i = 0; i < 150; ++i) {
            if (s.eleccion().activa()) {
                contesta();
                continue;
            }
            // En el juego se vuelve a mirar los beats cuando el estado cambia, no en bucle: un beat
            // auto sin guarda (b3_campanas) saldria para siempre. Desde el dia de Gorran, si un beat
            // salta y no cambia ninguna bandera ni variable (ni deja una eleccion), se para.
            const bool vigila = cinesAlEntrar || vigilaSiempre;
            const auto banderasAntes = vigila ? s.flags() : std::vector<std::string>{};
            const auto variablesAntes = vigila ? s.variables() : std::map<std::string, int>{};
            NarrativeResult r = e.tick(s);
            if (!r.fired) r = e.sigue(s);
            if (!r.fired) {
                // Como el juego, fotograma a fotograma: primero el mundo (eventos), luego lo que
                // arranca solo en este nivel (cinematicas sin zona). Solo desde el dia de Gorran.
                if (cinesAlEntrar && evaluaEventos()) continue;
                if (cinesAlEntrar && !enCine && cinematica(GridCoord{-1, -1})) continue;
                return;
            }
            procesa(r);
            if (vigila && !s.eleccion().activa() && s.flags() == banderasAntes && s.variables() == variablesAntes)
                return;
            // Un guion mutado puede dejar un auto que se repite cambiando un contador (addVariable) sin llegar a su flag: el mismo beat
            // dos veces seguidas (aunque se conteste una eleccion entre medias) ya es un bucle (E-MISIONES-01b).
            if (vigilaSiempre && !r.beatId.empty() && r.beatId == ultimoAuto) return;
            ultimoAuto = r.beatId;
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
        empiezaElDesayuno(nivel);
        cinesSinZona();
        // El Ocaso empieza en la plaza de ciudad_centro: el sermon de Sandes (plaza_sermon) salta al
        // pisar su zona [49,49,61,58]; el jugador va por ella.
        if (guion == &g.ocaso && nivel == "ciudad_centro" && !enCine) cinematica(GridCoord{54, 54});
    }

    // H-DIA1-1: bajar a la taberna ya no empieza el desayuno. Empieza al pasar
    // junto a Venides (cinematica venides_llama, que lo pide) y la cinematica
    // venides_te_sienta te lleva a su mesa; de ahi salta b1_desayuno. Aqui el
    // jugador hace ese camino: llega a la celda de su derecha (5, 7).
    void empiezaElDesayuno(const std::string& nivel) {
        if (nivel != "interior_taberna" || !s.hasFlag("bnd0_dormido")) return;
        if (s.hasFlag("bnd1_desayuno_visto") || s.hasFlag("bnd1_desayuno_hecho")) return;
        for (int vez = 0; vez < 2 && !s.hasFlag("bnd1_desayuno_llamado"); ++vez)
            if (!cinematica(GridCoord{5, 7})) break;
    }

    // D0024.3: el jugador no habla con quien no ve. La escena (cinematicas.json, «apariciones»: visibleSi/ocultoSi) la lee el mismo
    // CatalogoCinematicas::visible que usa el juego (ApplicationCinematica.cpp), con las flags del estado y el nivel en que esta.
    // Un objeto sin regla (o un npc que no es un objeto de la escena) cuenta como visible.
    bool seVe(const std::string& objeto) const {
        static const Game::CatalogoCinematicas cat = [] {
            auto r = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
            require(r);
            return r.value();
        }();
        return cat.visible(objeto, s.lugar(), [&](const std::string& f) { return s.hasFlag(f); });
    }

    NarrativeResult habla(const std::string& npc) {
        if (!seVe(npc)) {
            // E-MATRIZ-03 A0: sin lista de pendientes. Hablar con quien esta oculto es un fallo del camino (los dos casos que
            // quedaban, seguidor_brinda y venides_base, se resolvieron en el jugador: anochece() y ocasoBase()).
            std::printf("    !! habla: «%s» esta oculto en %s con estas flags (ocultoSi/visibleSi de cinematicas.json): el jugador no lo ve ni puede hablarle\n",
                        npc.c_str(), s.lugar().c_str());
            if (std::getenv("BND_OCULTO_AVISA") == nullptr) require(false);  // para diagnosticar: lista todos los que se tocan sin abortar
        }
        if (ruido) std::printf("\n  [hablas con %s]\n", npc.c_str());
        NarrativeResult r = e.talkTo(npc, s);
        procesa(r);
        drena();
        subeConRosita();
        apagaLasVelas();
        cinesSinZona();
        return r;
    }

    // Las cinematicas sin zona del nivel en que estas arrancan solas en cuanto se cumplen sus
    // condiciones (al entrar y despues de hablar), solo desde el dia de Gorran (ver fase()).
    void cinesSinZona() {
        if (!cinesAlEntrar || enCine) return;
        for (int vez = 0; vez < 4 && cinematica(GridCoord{-1, -1}); ++vez) {
        }
    }

    // Dia 3 (boundington_segundo_dia): «Subir a dormir» (bnd2_a_dormir) lanza la cinematica
    // taberna_velas (Rosita apaga las velas: bnd2_velas_apagadas) y entonces cierra el dia
    // (b2_cierre*, que exigen las velas apagadas y estar en la taberna).
    void apagaLasVelas() {
        if (!s.hasFlag("bnd2_a_dormir") || s.hasFlag("bnd2_velas_apagadas") || s.hasFlag("bnd2_dia_cerrado")) return;
        if (s.lugar().find("interior_taberna.json") == std::string::npos) return;
        cinematica(GridCoord{-1, -1});
    }

    // G4-02: «Subir a dormir» (bnd0_a_dormir) ya no duerme al instante: salta
    // la cinematica taberna_rosita_puja (enciende bnd0_rosita_te_sube y deja al
    // jugador arriba) y, al amanecer, se baja a la taberna, donde empieza el
    // desayuno (b1_desayuno: enter interior_taberna).
    void subeConRosita() {
        if (!s.hasFlag("bnd0_a_dormir") || s.hasFlag("bnd0_dormido") || s.hasFlag("bnd0_rosita_te_sube")) return;
        if (!cinematica(GridCoord{-1, -1})) return;
        if (s.hasFlag("bnd0_dormido")) entra("interior_taberna");
    }

    // Oriol (22/10): la noche del dia 1 acaba arriba, en la cama. Con la noche
    // hecha, subir al cuarto (enter interior_taberna_p1) enciende bnd1_en_cuarto,
    // que exigen los cierre_* del dia 1.
    void subeAlCuarto() { entra("interior_taberna_p1"); }

    // «Subir a dormir» como lo hace el jugador: habla con Rosita y sube al cuarto.
    void duerme() {
        if (!s.hasFlag("bnd_noche")) anochece();  // el cuarto se sube de noche
        habla("skilla");
        subeAlCuarto();
    }

    // GORRAN-DIA2: el dia 2 entero es la montaña de Gorran (dia_gorran). Esto NO lo juega:
    // es el atajo de las escenas que prueban el DIA 3 (boundington_segundo_dia, fase "dia2"),
    // que solo necesitan que el dia de Gorran haya acabado (bndg_dia_cerrado, que exige
    // b2_amanece). Amanece, Rosita da el encargo (bnd2_rosita_encargo_visto) y, con la
    // subasta ya pasada (bnd2_subasta_vista: antes bg_skilla_hora_pronto no te deja
    // acostarte), el jugador sube a dormir: bg_cierre cierra el dia y amanece el tercero.
    // Para jugarlo de verdad (lios, subasta, dormir): pasaElDiaDeGorranJugando(). Cambiar este atajo
    // por ella rompe "DIA 3: la fiesta, por la puerta y hablando" (ver GORRAN_LIOS.md).
    void pasaElDiaDeGorran() {
        fase("dia_gorran");
        drena();  // bg_amanece
        entra("interior_taberna");
        habla("skilla");  // b2_rosita_encargo (la primera vez)
        s.setFlag("bnd2_subasta_vista");
        s.setFlag("bndg_a_dormir");  // «Subir a dormir» de bg_skilla_hora
        entra("interior_taberna");   // bg_cierre exige estar en la taberna
        require(s.hasFlag("bndg_dia_cerrado"));
        fase("dia2");
        drena();  // b2_amanece: bnd2_encargo
        require(s.hasFlag("bnd2_encargo"));
    }

    // La cinematica con zona `id`, pisada por el jugador (el centro de su zona) en el nivel en que esta.
    // false si no esta en ese nivel o no se cumplen sus condiciones (no se juega nada).
    bool pisaLaZona(const std::string& id) {
        static const Game::CatalogoCinematicas cat = [] {
            auto r = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
            require(r);
            return r.value();
        }();
        for (const Game::Cinematica& c : cat.lista) {
            if (c.id != id || !c.hayZona || c.nivel != s.lugar()) continue;
            return cinematica(c.puntoDeEntrada());
        }
        return false;
    }

    // GORRAN-LIOS (D0022 3.2): los cuatro «lios de la aldea» de Gorran, jugados como el jugador
    // (hablar, pisar la zona de la cinematica, elegir en el dialogo), no dados por hechos: cada uno
    // se cierra con su propio beat (bgb_/bga_/bgd_/bgc_cerrada) y es ese beat el que suma a
    // gorran_misiones. Va por orden de reloj (aldea_tiempo; solo avanza con lo que haces): la Veta
    // Honda (vigas del taller y apuntalar), Cal Serra (la Garsa contra los mercenarios), la mina
    // vieja (sacar a los ninos en silencio) y las Galerias (hundir la carga gris). Al tercero
    // cerrado suena el cuerno (b2_subasta_aviso -> bnd2_subasta_ya). Los viajes entre niveles no
    // se andan (entra() es un salto, como en el resto de ayudas). Con mala suerte algunos lios se
    // cierran peor (o por tiempo): `cuantos` es el minimo de lios que tienen que haber cerrado.
    // GORRAN-FIX: `orden` son las letras de los lios en el orden en que se juegan (B Veta Honda, A Cal Serra,
    // C Galerias, D mina vieja). El reloj manda: con "BADC" la mina vieja se hace con el reloj en 3 y se salva
    // a Mia; con "BACD" se llega con el reloj en 5 (bgd_mia_perdida) y solo se salva a Tinet (D0023.2).
    // `cuantos` es el minimo que tiene que haber cerrado; `tiradasBuenas` hace que las tiradas de
    // los lios salgan bien aunque el jugador tenga mala suerte para lo que viene (las escenas que
    // prueban el pedestal con mala suerte no son las que prueban los lios).
    int losLiosDeLaAldea(int cuantos = 3, bool tiradasBuenas = true, const std::string& orden = "BADC",
                         const std::vector<std::string>& primero = {}) {
        const std::vector<std::string> antes = prefiere;
        const Suerte suerteAntes = suerte;
        if (tiradasBuenas && !buena()) suerte = Suerte::Buena;
        prefiere.insert(prefiere.begin(),
                        {"«Me quedo. Nadie se la lleva.»", "Hablarle a la Garsa [Persuasión]", "Dejar que se la lleven",
                         "Apuntalar con las vigas del taller", "Sacar a los críos en silencio [Sigilo]",
                         "Abrir una sin romper el sello [Investigación]", "Hundirlas en el río [Atletismo]"});
        prefiere.insert(prefiere.begin(), primero.begin(), primero.end());  // E-MATRIZ-02: las decisiones propias van antes que las de siempre
        entra("cueva_gorran");  // la plaza de la aldea: bg_aldea_problemas
        // Cada lio tiene que sumar exactamente 1 a gorran_misiones (su beat de cierre, una sola vez).
        int misiones = s.variable("gorran_misiones");
        const auto suma = [&](const char* cerrada, const char* quien) {
            if (!s.hasFlag(cerrada)) return;  // ese lio no se ha cerrado: no suma
            misiones += 1;
            if (s.variable("gorran_misiones") != std::min(misiones, 4))
                std::printf("    !! %s: gorran_misiones=%d, esperado %d\n", quien, s.variable("gorran_misiones"), misiones);
            require(s.variable("gorran_misiones") == std::min(misiones, 4));
        };
        // B — La Veta Honda: la minera cuenta lo del derrumbe, cuatro vigas del taller (+1 de reloj) y
        // los golpes al otro lado (+1): apuntalar.
        const auto lioB = [&] {
            entra("mina_gorran");
            habla("minera_comedor");  // bgb_minera: los mineros atrapados (bgb_gancho)
            entra("galerias_gorran");
            habla("carpintero_galeria");
            entra("mina_gorran");
            pisaLaZona("bgb_golpes");
            suma("bgb_cerrada", "Veta Honda");
        };
        // A — Cal Serra: con el reloj en 2 llegan los mercenarios (bga_llegan -> bga_entran): la Garsa.
        const auto lioA = [&] {
            entra("aldea_serra_p0");
            habla("serra");
            suma("bga_cerrada", "Cal Serra");
        };
        // C — Las Galerias: la carga gris (abrir una caja, el asalto del transbordo).
        const auto lioC = [&] {
            entra("galerias_gorran");
            habla("aduanero_galeria");
            habla("cajas_cera_gris");
            pisaLaZona("bgc_asalto");
            suma("bgc_cerrada", "Galerias");
        };
        // D — la mina vieja: Tinet y Mia (bgd_rapto, reloj 2); el diacono enciende las velas. Con el reloj
        // en 5 o mas Mia ya no esta (bgd_mia_perdida, D0023.2): el cierre salva solo a Tinet.
        const auto lioD = [&] {
            entra("mina2_gorran");
            pisaLaZona("bgd_velas");
            suma("bgd_cerrada", "mina vieja");
        };
        for (const char c : orden) {
            if (c == 'B') lioB();
            if (c == 'A') lioA();
            if (c == 'C') lioC();
            if (c == 'D') lioD();
        }
        prefiere = antes;
        suerte = suerteAntes;
        int cerrados = 0;
        for (const char* f : {"bga_cerrada", "bgb_cerrada", "bgc_cerrada", "bgd_cerrada"}) cerrados += s.hasFlag(f) ? 1 : 0;
        if (ruido) std::printf("      [lios cerrados: %d, gorran_misiones=%d]\n", cerrados, s.variable("gorran_misiones"));
        require(cerrados >= cuantos);
        require(s.variable("gorran_misiones") == cerrados);  // por merito: lo cuenta cada beat de cierre
        return cerrados;
    }

    // GORRAN-LIOS: el dia 2 de Gorran SIN atajos, hasta el dia 3. El encargo de Rosita, la Gola Fosca
    // (se paga), los cuatro lios de la aldea (losLiosDeLaAldea), el cuerno, el porton del recinto y
    // la subasta de la Furia (cinematica recinto_subasta) y, con ella pasada, «Subir a dormir»
    // (bg_skilla_hora: bndg_a_dormir -> bg_cierre / bg_cierre_furia). Amanece el tercero (b2_amanece:
    // bnd2_encargo). La espada no se roba del pedestal (eso es laEspadaDeRosita): en la subasta
    // «Dejar que estalle» acaba en pelea con Gorran, y vencido «Suelta a los presos y estamos en paz.».
    // E-MATRIZ-02: las decisiones del dia son parametros (DiaDeGorran); con los de por defecto es lo de siempre.
    struct DiaDeGorran {
        std::string lios = "BADC";                     // letras de los lios que se juegan, en orden (B Veta Honda, A Cal Serra, C Galerias, D mina vieja)
        bool aval = false;                             // vuelve con Serra tras salvar a Iris y ella le avala ante el recinto (bga_despues_bien: bnd2_aval_serra)
        bool tregua = false;                           // D0024: en la subasta, con el aval de Serra, «Proponerle una tregua» (b2_subasta_aval) y luego lo que diga `subasta`
        std::vector<std::string> subasta = {"Dejar que estalle"};  // lo que elige en la subasta, por orden de gusto
        std::string final = "Suelta a los presos y estamos en paz.";  // vencido Gorran: esta o «Acabar con él (no volverá a ayudarte)»
        std::vector<std::string> antes;                // respuestas que van por delante de las demas (p. ej. «Unirte: «Yo enciendo la vela.»»)
        bool trato = false;                            // en la casa del Cojo se lleva el trato (b2_libros_cojo) y se lo lee a Gorran (b2_gorran_trato) en vez de la subasta
        std::vector<std::pair<std::string, Suerte>> tiradas;  // tiradas (por su flagBotch) que salen con esta suerte aunque las demas salgan bien
        bool exige = true;                             // false (las mutaciones): lo que no sale no aborta, la prueba lo mira despues
        std::string parada;                            // E-MATRIZ-02 (capturas): "subasta" | "balance" | "dormir" = se para JUSTO ANTES de ese momento
        bool amaneceElTercero = true;                  // false = se queda en el cierre del dia (bndg_dia_cerrado): el guardado de fin de dia
        int cuantos = 3;                               // minimo de lios cerrados
    };

    void juegaElDiaDeGorran(const DiaDeGorran& d) {
        const std::vector<std::string> antes = prefiere;
        prefiere.insert(prefiere.begin(), d.antes.begin(), d.antes.end());
        for (const auto& t : d.tiradas) tiradas[t.first] = t.second;
        if (d.trato) prefiere.push_back("Llevarte el trato");
        if (d.trato) prefiere.push_back("Enseñarle el trato y leérselo");
        if (d.tregua) prefiere.push_back("Proponerle una tregua");
        prefiere.insert(prefiere.end(), d.subasta.begin(), d.subasta.end());
        prefiere.insert(prefiere.end(), {"Aceptar el encargo", "Pagar el paso (5)", "«Vengo a ver al jefe.» [Persuasión]"});
        prefiere.push_back(d.final);
        prefiere.insert(prefiere.end(), {"Suelta a los presos y estamos en paz.", "Subir a dormir"});
        fase("dia_gorran");
        drena();  // bg_amanece
        entra("interior_taberna");
        habla("skilla");  // b2_rosita_encargo
        entra("antecova_gorran");  // b2_cueva_llegada: pagar el paso -> bnd2_aldea_dentro
        losLiosDeLaAldea(d.cuantos, true, d.lios, d.antes);  // -> b2_subasta_aviso -> bnd2_subasta_ya
        if (d.aval) {  // Serra, ya con Iris a salvo (bga_despues_bien), avala al heroe ante el recinto: bnd2_aval_serra
            entra("aldea_serra_p0");
            habla("serra");
            require(!d.exige || s.hasFlag("bnd2_aval_serra"));
        }
        if (d.trato) {  // el trato del Cojo con Gorran, de su casa
            entra("aldea_cojo_p1");
            habla("libros_cojo");
            require(!d.exige || s.hasFlag("bnd2_trato_venta"));
        }
        entra("recinto_gorran");  // b2_porton
        // D0024.3: Gorran esta oculto desde el cuerno (bnd2_subasta_ya): b2_gorran_trato (talk) no se alcanza jugando. El trato que
        // se lleva del Cojo se lee en el calabozo (b2_calabozo): una farsa que sale mal (sospecha 3) te lleva atado a Gorran.
        if (d.parada == "subasta") {
            prefiere = antes;
            return;
        }
        require(pisaLaZona("recinto_subasta") || !d.exige);  // b2_subasta(_cojo_pobre): la eleccion de `subasta`
        require(!d.exige || s.hasFlag("bnd2_subasta_vista"));
        require(!d.exige || !d.tregua || s.hasFlag("bnd2_gorran_tregua"));  // D0024: la tregua se pacta en la subasta
        if (d.parada == "balance") {  // aun no se ha bajado a la taberna: al entrar sale el hito y el balance de la aldea
            prefiere = antes;
            return;
        }
        entra("interior_taberna");
        if (d.parada == "dormir") {
            prefiere = antes;
            return;
        }
        habla("skilla");  // bg_skilla_hora: «Subir a dormir» -> bndg_a_dormir -> bg_cierre
        require(!d.exige || s.hasFlag("bndg_dia_cerrado"));
        prefiere = antes;
        if (!d.amaneceElTercero) return;
        fase("dia2");
        drena();  // b2_amanece: bnd2_encargo
        require(s.hasFlag("bnd2_encargo"));
    }

    void pasaElDiaDeGorranJugando(const std::string& ordenLios = "BADC") {
        DiaDeGorran d;
        d.lios = ordenLios;
        juegaElDiaDeGorran(d);
    }

    // GORRAN-DIA2 — La espada de Rosita (dia_gorran): el encargo de Rosita, el paso por la Gola
    // Fosca (se paga), los lios de la aldea (losLiosDeLaAldea: la subasta exige tres cerrados,
    // gorran_misiones >= 3, b2_subasta_aviso) y el pedestal del recinto cuando suena el cuerno
    // de la subasta.
    void laEspadaDeRosita() {
        entra("interior_taberna");
        habla("skilla");  // b2_rosita_encargo -> bnd2_skilla_encargo
        entra("antecova_gorran");  // b2_cueva_llegada: pagar el paso -> bnd2_aldea_dentro
        losLiosDeLaAldea();  // -> b2_subasta_aviso -> bnd2_subasta_ya
        entra("recinto_gorran");
        habla("pedestal_furia");  // b2_pedestal -> «Cogerla sin que te oigan [Sigilo]»
    }

    // El jugador pisa, en el nivel en que esta, la zona de la primera cinematica que arranca ahi
    // (el centro de su zona). `ruta` es la salida que elige en la base (base_ruta_oeste / _norte /
    // _quedarse): las otras zonas se esquivan. false si no hay ninguna que pisar.
    bool pisaUnaZona(const std::string& ruta = "base_ruta_oeste") {
        static const Game::CatalogoCinematicas cat = [] {
            auto r = Game::CatalogoCinematicas::carga("assets/cinematicas/cinematicas.json");
            require(r);
            return r.value();
        }();
        const auto tiene = [this](const std::string& f) { return s.hasFlag(f); };
        // El orden lo pone el jugador: las que prefiere ir primero, y despues las del catalogo.
        std::vector<const Game::Cinematica*> orden;
        for (const std::string& id : zonasPrimero)
            for (const Game::Cinematica& c : cat.lista)
                if (c.id == id) orden.push_back(&c);
        for (const Game::Cinematica& c : cat.lista) orden.push_back(&c);
        for (const Game::Cinematica* pc : orden) {
            const Game::Cinematica& c = *pc;
            if (!c.hayZona || c.nivel != s.lugar()) continue;
            if (c.id.rfind("base_ruta_", 0) == 0 && c.id != ruta) continue;
            if (std::find(zonasEvita.begin(), zonasEvita.end(), c.id) != zonasEvita.end()) continue;
            const GridCoord centro = c.puntoDeEntrada();
            if (cat.laQueEmpieza(s.lugar(), centro, tiene) != &c) continue;
            return cinematica(centro);
        }
        return false;
    }

    // El Ocaso desde la base hasta la huida: la salida elegida (zona de ciudad_oeste), la ciudad
    // del Ocaso, Las Afueras y la huida. En cada sitio pisa las zonas que van saliendo.
    void ocasoRecorrido(const std::string& ruta = "base_ruta_oeste") {
        if (!s.hasFlag("bnd3_ruta")) require(s.hasFlag("bnd3_elige_" + ruta.substr(10)));  // la salida se elige en el dialogo (b3_por_donde)
        for (const char* nivel : {"ciudad_oeste", "afueras_boundington_ocaso", "huida_boundington"}) {
            entra(nivel);
            for (int vez = 0; vez < 12 && pisaUnaZona(ruta); ++vez) {
            }
        }
    }

    // El Ocaso (boundington_ocaso), el principio: la plaza (sermon de Sandes: «Correr a avisar a
    // Venides»), la base y Venides (b3_avisar_venides -> evento comienza_ocaso -> campanas, arenga).
    void ocasoBase() {
        entra("ciudad_centro");
        entra("interior_cuartel");
        // E-MATRIZ-03 A0: con bnd2_venides_sin_aviso (callaste el informe) Venides no esta en la base (cinematicas.json, «venides_base»):
        // no se le habla; las campanas suenan solas al ponerse el sol y b3_venides_retenido cuenta que salio solo.
        if (seVe("venides_base")) habla("venides_base");
        if (!s.hasFlag("bnd3_campanas")) {  // nadie ha avisado: suenan solas al ponerse el sol (evento ocaso_puesta)
            entra("interior_taberna");
            anochece();
            entra("interior_cuartel");
        }
    }

    // Cierra el DIA 3 (boundington_segundo_dia): anochece si hace falta y «Subir a dormir» con
    // Rosita (b2_skilla_hora pide franja >= 1); apagaLasVelas() hace el resto (bnd2_dia_cerrado).
    void duermeDia3() {
        if (s.variable("franja") < 2) anochece();
        prefiere.insert(prefiere.begin(), "Subir a dormir");
        habla("skilla");
        prefiere.erase(prefiere.begin());
    }

    // Rosita lleva la hora: «Esperar a que anochezca».
    // E-MATRIZ-03 A0: si ya es de noche (franja >= 2: p. ej. tras la Barriada) no hay nada que esperar; hablar con Rosita
    // aqui ofrece «Subir a dormir» y el jugador se acostaria antes de la fiesta (b2_fiesta_puerta exige no bnd2_a_dormir).
    void anochece() {
        if (guion == &g.dia2 && s.variable("franja") >= 2) return;  // (solo el dia 3 lleva la franja)
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
        for (const std::string& f : cat.rearmadas(s.lugar(), donde, [this](const std::string& f) { return s.hasFlag(f); }))
            s.clearFlag(f);  // E5 (D0026.4): salir de una zona `once: false` la rearma
        const Game::Cinematica* c = cat.laQueEmpieza(s.lugar(), donde, [this](const std::string& f) { return s.hasFlag(f); });
        if (c == nullptr) return false;
        const bool yaEnCine = enCine;
        enCine = true;
        if (ruido) std::printf("\n  [cinematica %s]\n", c->id.c_str());
        for (const Game::PasoCinematica& p : cat.pasosEfectivos(*c)) {  // E-DISPARADORES-1: con disparador, lleva + accion
            if (p.tipo == Game::PasoCinematica::Tipo::Beat) {
                s.setFlag(p.texto);
                drena();
            } else if (p.tipo == Game::PasoCinematica::Tipo::Flag) {
                s.setFlag(p.texto);
            } else if (p.tipo == Game::PasoCinematica::Tipo::Combate) {  // E4: como un startBattle; huir cuenta como perder (D0027: morir no la enciende)
                const bool gana = !huyen.count(p.flagGana) && !pierde.count(p.flagGana) && buena();
                pierde.erase(p.flagGana);
                huyen.erase(p.flagGana);
                s.setFlag(gana ? p.flagGana : p.flagPierde);
                if (ruido) std::printf("      [combate %s: %s]\n", p.texto.c_str(), gana ? "victoria" : "derrota");
            } else if (p.tipo == Game::PasoCinematica::Tipo::Nivel) {
                std::string n = p.texto.substr(p.texto.rfind('/') + 1);
                entra(n.substr(0, n.size() - 5));  // sin «.json»
            }
        }
        s.setFlag(c->vista());
        for (const std::string& f : c->alTerminar) s.setFlag(f);
        drena();
        enCine = yaEnCine;
        // FANTASMA-01: en los recuerdos, al acabar, andas libre; para seguir, se
        // habla con Sho-Noco (mas de una vez si antes eliges «Mirar un poco mas»).
        bool libre = false;
        for (const std::string& f : c->alTerminar)
            libre = libre || (f.size() > 6 && f.compare(f.size() - 6, 6, "_libre") == 0);
        for (int vez = 0; libre && vez < 4; ++vez) {
            habla("sho_noco");
            if (cat.laQueEmpieza(s.lugar(), GridCoord{-1, -1}, [this](const std::string& f) { return s.hasFlag(f); }) != nullptr)
                break;
        }
        if (!yaEnCine) cinesSinZona();  // las que arrancan solas al acabar esta
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
        // Diagnostico: BND_RUIDO="trozo del titulo" cuenta la partida de esa escena paso a paso.
        const char* ruido = std::getenv("BND_RUIDO");
        if (ruido != nullptr && titulo.find(ruido) != std::string::npos) j.ruido = true;
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
    void igual(const char* var, int n) {
        if (j.s.variable(var) != n)
            fallos.push_back(std::string(var) + "=" + std::to_string(j.s.variable(var)) + " != " + std::to_string(n));
    }
    void noVisto(std::initializer_list<const char*> ids) {
        for (const char* id : ids)
            if (j.visto(id)) fallos.push_back(std::string("se vio ") + id);
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
