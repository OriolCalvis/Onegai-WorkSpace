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

// Un jugador: un estado para toda la campana y una forma de contestar.
struct Jugador {
    const Guiones& g;
    NarrativeEngine e;
    NarrativeState s;  // <- UNO SOLO para las cuatro aventuras
    const AdventureScript* guion = nullptr;
    Suerte suerte = Suerte::Buena;
    bool ruido = false;
    bool cinesAlEntrar = false;
    std::vector<std::string> zonasPrimero;  // cinematicas con zona a las que va antes que a las demas (pisaUnaZona)
    std::vector<std::string> zonasEvita;    // ... y a las que no va nunca
    bool enCine = false;  // dentro de cinematica(): no se encadenan otras desde drena()
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
        for (int i = 0; i < 150; ++i) {
            if (s.eleccion().activa()) {
                contesta();
                continue;
            }
            // En el juego se vuelve a mirar los beats cuando el estado cambia, no en bucle: un beat
            // auto sin guarda (b3_campanas) saldria para siempre. Desde el dia de Gorran, si un beat
            // salta y no cambia ninguna bandera ni variable (ni deja una eleccion), se para.
            const bool vigila = cinesAlEntrar;
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

    NarrativeResult habla(const std::string& npc) {
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
            return cinematica(GridCoord{(c.zonaDesde.x + c.zonaHasta.x) / 2, (c.zonaDesde.y + c.zonaHasta.y) / 2});
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
    // `cuantos` es el minimo que tiene que haber cerrado; `tiradasBuenas` hace que las tiradas de
    // los lios salgan bien aunque el jugador tenga mala suerte para lo que viene (las escenas que
    // prueban el pedestal con mala suerte no son las que prueban los lios).
    int losLiosDeLaAldea(int cuantos = 3, bool tiradasBuenas = true) {
        const std::vector<std::string> antes = prefiere;
        const Suerte suerteAntes = suerte;
        if (tiradasBuenas && !buena()) suerte = Suerte::Buena;
        prefiere.insert(prefiere.begin(),
                        {"«Me quedo. Nadie se la lleva.»", "Hablarle a la Garsa [Persuasión]", "Dejar que se la lleven",
                         "Apuntalar con las vigas del taller", "Sacar a los críos en silencio [Sigilo]",
                         "Abrir una sin romper el sello [Investigación]", "Hundirlas en el río [Atletismo]"});
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
        entra("mina_gorran");
        habla("minera_comedor");  // bgb_minera: los mineros atrapados (bgb_gancho)
        entra("galerias_gorran");
        habla("carpintero_galeria");
        entra("mina_gorran");
        pisaLaZona("bgb_golpes");
        suma("bgb_cerrada", "Veta Honda");
        // A — Cal Serra: con el reloj en 2 llegan los mercenarios (bga_llegan -> bga_entran): la Garsa.
        entra("aldea_serra_p0");
        habla("serra");
        suma("bga_cerrada", "Cal Serra");
        // C — Las Galerias: la carga gris (abrir una caja, el asalto del transbordo).
        entra("galerias_gorran");
        habla("aduanero_galeria");
        habla("cajas_cera_gris");
        pisaLaZona("bgc_asalto");
        suma("bgc_cerrada", "Galerias");
        // D — la mina vieja: Tinet y Mia (bgd_rapto, reloj 2); el diacono enciende las velas.
        entra("mina2_gorran");
        pisaLaZona("bgd_velas");
        suma("bgd_cerrada", "mina vieja");
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
    void pasaElDiaDeGorranJugando() {
        const std::vector<std::string> antes = prefiere;
        prefiere.insert(prefiere.end(), {"Aceptar el encargo", "Pagar el paso (5)", "«Vengo a ver al jefe.» [Persuasión]",
                                         "Dejar que estalle", "Suelta a los presos y estamos en paz.",
                                         "Subir a dormir"});
        fase("dia_gorran");
        drena();  // bg_amanece
        entra("interior_taberna");
        habla("skilla");  // b2_rosita_encargo
        entra("antecova_gorran");  // b2_cueva_llegada: pagar el paso -> bnd2_aldea_dentro
        losLiosDeLaAldea();  // -> b2_subasta_aviso -> bnd2_subasta_ya
        entra("recinto_gorran");  // b2_porton
        require(pisaLaZona("recinto_subasta"));  // b2_subasta(_cojo_pobre): «Dejar que estalle»
        require(s.hasFlag("bnd2_subasta_vista"));
        entra("interior_taberna");
        habla("skilla");  // bg_skilla_hora: «Subir a dormir» -> bndg_a_dormir -> bg_cierre
        require(s.hasFlag("bndg_dia_cerrado"));
        prefiere = antes;
        fase("dia2");
        drena();  // b2_amanece: bnd2_encargo
        require(s.hasFlag("bnd2_encargo"));
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
            const GridCoord centro{(c.zonaDesde.x + c.zonaHasta.x) / 2, (c.zonaDesde.y + c.zonaHasta.y) / 2};
            if (cat.laQueEmpieza(s.lugar(), centro, tiene) != &c) continue;
            return cinematica(centro);
        }
        return false;
    }

    // El Ocaso desde la base hasta la huida: la salida elegida (zona de ciudad_oeste), la ciudad
    // del Ocaso, Las Afueras y la huida. En cada sitio pisa las zonas que van saliendo.
    void ocasoRecorrido(const std::string& ruta = "base_ruta_oeste") {
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
        habla("venides_base");
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
        const bool yaEnCine = enCine;
        enCine = true;
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
