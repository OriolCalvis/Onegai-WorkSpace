// demo_senal_ceniza — "La Senal de la Ceniza", el espejo oriental de la
// campana de Boundington.
//
// Ano 2000 b.f.: en el oeste los Perdidos de Perishton intentan liberar a
// Gurkazaal sin que nadie lo vea; en el este Choubar firma el tributo de
// los Sagas. El jugador mueve la palabra entre Guskedor (el Gran Consejo
// orco), Venordemn (las Cortes tiefling), Qethatos (la universidad
// teatral) y Klimnebra (la capital que ya firma la guerra).
//
// INVARIANTE CANONICO, igual que la Matanza en Boundington: la Primera
// Cruzada oriental OCURRE siempre (EV-2000-CRUZADA01E). El jugador solo
// decide si el este entra en ella ciego, medio despierto o con los ojos
// abiertos. Este demo juega las tres rutas mas la de criticos, y comprueba
// que los tres cierres existen y son excluyentes.
//
// Las tiradas Nd6 se fuerzan con ScriptedRng (rewind por tirada, patron de
// demo_prologo). El valor de stat por ruta simula el heroe que la haria:
// la ruta de criticos usa pools de 4 dados (un heroe carismatico).
//
// GL-free: corre en CI.
#include <cstdio>
#include <set>
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
using RPG::NarrativeEngine;
using RPG::NarrativeResult;
using RPG::NarrativeState;
using RPG::SkillDefinition;

namespace {

const char* kRuta = "assets/adventures/este_norte_senal_ceniza.json";

bool suelta(const RPG::NarrativeResult& r) {
    if (!r.fired) return false;
    if (!r.speaker.empty()) std::printf("      %s:\n", r.speaker.c_str());
    for (const auto& l : r.lines) std::printf("        %s\n", l.c_str());
    return true;
}

// Resuelve los skillCheck de un result con rewind por tirada (patron
// demo_prologo). valorStat = tamano del pool del heroe de esta ruta.
int resolver(const SkillCatalog& cat, int valorStat, const NarrativeResult& r,
             ScriptedRng& rng, NarrativeState& s) {
    int n = 0;
    for (const auto& c : r.skillChecks) {
        const SkillDefinition* def = cat.find(c.skillId);
        require(def != nullptr);
        // SIN rewind: cada tirada consume sus dados del guion en orden,
        // asi una ruta puede exigir resultados DISTINTOS por tirada.
        auto pool = RPG::DicePoolEngine::roll_pool(valorStat, RPG::DiceMod::NORMAL, rng);
        auto out = RPG::DicePoolEngine::resolve_against_cd(pool, c.cd);
        switch (out.degree) {
            case RPG::Degree::BOTCH:    s.setFlag(c.flagBotch);    break;
            case RPG::Degree::PARTIAL:  s.setFlag(c.flagPartial);  break;
            case RPG::Degree::SUCCESS:  s.setFlag(c.flagSuccess);  break;
            case RPG::Degree::CRITICAL: s.setFlag(c.flagCritical); break;
        }
        ++n;
    }
    return n;
}

// Un paso de "talk con posible tirada + tick del resultado".
void paso(NarrativeEngine& e, NarrativeState& s, const SkillCatalog& cat,
          const char* npc, ScriptedRng& rng, int stat,
          std::set<std::string>* vistos) {
    NarrativeResult r = e.talkTo(npc, s);
    if (r.fired) {
        suelta(r);
        if (vistos) vistos->insert(r.beatId);
        if (!r.skillChecks.empty()) resolver(cat, stat, r, rng, s);
        NarrativeResult rel = e.tick(s);   // el beat de resultado es auto
        if (rel.fired) {
            std::printf("      (resultado de la tirada)\n");
            if (vistos) vistos->insert(rel.beatId);
        }
    }
}

void entrar(NarrativeEngine& e, NarrativeState& s, const char* nivel,
            std::set<std::string>* vistos) {
    NarrativeResult r = e.enterLevel(nivel, s);
    if (r.fired && vistos) vistos->insert(r.beatId);
    suelta(r);
}

}  // namespace

int main() {
    std::printf("=== demo_senal_ceniza: el este, ano 2000 b.f. ===\n\n");

    auto cargada = AdventureScript::loadFromFile(kRuta);
    require(cargada.isOk());
    const AdventureScript guion = cargada.value();
    std::printf("[carga] %s: %zu beats, %zu objetivos\n\n",
                guion.id.c_str(), guion.beats.size(), guion.objectives.size());

    SkillCatalog cat;
    require(cat.loadFromFile("assets/catalogs/skills.json").isOk());

    std::set<std::string> vistos;

    // -----------------------------------------------------------------
    // RUTA 1 — ojos abiertos: pruebas en las dos orillas y el Consejo
    // escucha. Persuasion firme tambien ante el Capitan.
    // -----------------------------------------------------------------
    std::printf("[RUTA 1: ojos abiertos]\n");
    {
        NarrativeEngine e; e.setAdventure(&guion);
        NarrativeState s;
        // arcano{6,5} perc{6,5,1} teatro{6,5} persu{6,5} capit{6,6,5}
// (capit con 3 dados: {6,6} serian todo-seises = critico automatico)
        ScriptedRng rng({6, 5, 6, 5, 1, 6, 5, 6, 5, 6, 6, 5});
        entrar(e, s, "assets/levels/ciudad_en_guskedor.json", &vistos);
        paso(e, s, cat, "en_enviada_neblinia", rng, 3, &vistos);
        paso(e, s, cat, "en_anciana_consejo", rng, 2, &vistos);
        entrar(e, s, "assets/levels/ciudad_en_venordemn.json", &vistos);
        paso(e, s, cat, "en_archivera_espejos", rng, 2, &vistos);   // arcano ok
        entrar(e, s, "assets/levels/ciudad_en_qethatos.json", &vistos);
        paso(e, s, cat, "en_cronista_qethatos", rng, 3, &vistos);   // percepcion ok
        paso(e, s, cat, "en_cronista_qethatos", rng, 2, &vistos);   // teatro bien
        paso(e, s, cat, "en_anciana_consejo", rng, 2, &vistos);     // persuasion ok
        entrar(e, s, "assets/levels/ciudad_en_klimnebra.json", &vistos);
        paso(e, s, cat, "en_capitan_ventobelico", rng, 3, &vistos); // persuasion firme
        NarrativeResult n = e.tick(s);                               // la noticia
        if (n.fired) { vistos.insert(n.beatId); suelta(n); }
        NarrativeResult c = e.tick(s);                               // el cierre
        if (c.fired) { vistos.insert(c.beatId); suelta(c); }
        require(s.hasFlag("en_sc_cerrado"));
        require(s.hasFlag("en_sc_consejo_sabe"));
        require(s.hasFlag("en_sc_choubar_aviso"));
        require(!s.hasFlag("en_sc_consejo_sella"));
        std::printf("  -> cierre OJOS ABIERTOS: OK\n\n");
    }

    // -----------------------------------------------------------------
    // RUTA 2 — medio despierto: pruebas pero el Consejo solo duda.
    // -----------------------------------------------------------------
    std::printf("[RUTA 2: medio despierto]\n");
    {
        NarrativeEngine e; e.setAdventure(&guion);
        NarrativeState s;
        // arcano{5,5}=PARCIAL perc{6,5,1} teatro{1,1} persu{5,5}
        ScriptedRng rng({5, 5, 6, 5, 1, 1, 1, 5, 5});
        entrar(e, s, "assets/levels/ciudad_en_guskedor.json", &vistos);
        paso(e, s, cat, "en_enviada_neblinia", rng, 3, &vistos);
        paso(e, s, cat, "en_anciana_consejo", rng, 2, &vistos);
        entrar(e, s, "assets/levels/ciudad_en_venordemn.json", &vistos);
        paso(e, s, cat, "en_archivera_espejos", rng, 2, &vistos);   // arcano PARCIAL
        require(s.hasFlag("en_sc_cortes_roza"));
        entrar(e, s, "assets/levels/ciudad_en_qethatos.json", &vistos);
        paso(e, s, cat, "en_cronista_qethatos", rng, 3, &vistos);
        paso(e, s, cat, "en_cronista_qethatos", rng, 2, &vistos);   // teatro flop
        paso(e, s, cat, "en_anciana_consejo", rng, 2, &vistos);     // persuasion parcial
        NarrativeResult n = e.tick(s);
        if (n.fired) { vistos.insert(n.beatId); suelta(n); }
        NarrativeResult c = e.tick(s);
        if (c.fired) { vistos.insert(c.beatId); suelta(c); }
        require(s.hasFlag("en_sc_cerrado"));
        require(s.hasFlag("en_sc_consejo_duda"));
        require(!s.hasFlag("en_sc_consejo_sabe"));
        std::printf("  -> cierre MEDIO DESPIERTO: OK\n\n");
    }

    // -----------------------------------------------------------------
    // RUTA 3 — ciego: nadie escucha nada. La guerra pilla al este mirando
    // a la Cordillera.
    // -----------------------------------------------------------------
    std::printf("[RUTA 3: ciego]\n");
    {
        NarrativeEngine e; e.setAdventure(&guion);
        NarrativeState s;
        // capit{1,1}=BOTCH perc{5,1,1}=PARCIAL
        ScriptedRng rng({5, 1, 1, 1, 1});
        entrar(e, s, "assets/levels/ciudad_en_guskedor.json", &vistos);
        paso(e, s, cat, "en_enviada_neblinia", rng, 3, &vistos);
        paso(e, s, cat, "en_enviada_neblinia", rng, 3, &vistos);   // repite
        paso(e, s, cat, "en_anciana_consejo", rng, 2, &vistos);
        paso(e, s, cat, "en_anciana_consejo", rng, 2, &vistos);    // repite
        entrar(e, s, "assets/levels/ciudad_en_qethatos.json", &vistos);
        paso(e, s, cat, "en_cronista_qethatos", rng, 3, &vistos);  // percepcion PARCIAL
        require(s.hasFlag("en_sc_archivo_roza"));
        entrar(e, s, "assets/levels/ciudad_en_klimnebra.json", &vistos);
        paso(e, s, cat, "en_capitan_ventobelico", rng, 2, &vistos); // portazo
        NarrativeResult n = e.tick(s);
        if (n.fired) { vistos.insert(n.beatId); suelta(n); }
        NarrativeResult c = e.tick(s);
        if (c.fired) { vistos.insert(c.beatId); suelta(c); }
        require(s.hasFlag("en_sc_cerrado"));
        require(!s.hasFlag("en_sc_consejo_sabe"));
        require(!s.hasFlag("en_sc_consejo_duda"));
        require(!s.hasFlag("en_sc_choubar_aviso"));
        std::printf("  -> cierre CIEGO: OK\n\n");
    }

    // -----------------------------------------------------------------
    // RUTA 4 — criticos: heroe de 4 dados. El Consejo se levanta y sella;
    // Qethatos estrena obra; el Capitan anota en el propio tributo.
    // -----------------------------------------------------------------
    std::printf("[RUTA 4: criticos]\n");
    {
        NarrativeEngine e; e.setAdventure(&guion);
        NarrativeState s;
        // 4 tiradas de heroe de 4 dados, todas criticas salvo el capit
        ScriptedRng rng({6, 6, 5, 1, 6, 6, 5, 1, 6, 6, 5, 1, 6, 6, 5, 1,
                      6, 6, 5, 1});
        entrar(e, s, "assets/levels/ciudad_en_guskedor.json", &vistos);
        paso(e, s, cat, "en_enviada_neblinia", rng, 4, &vistos);
        paso(e, s, cat, "en_anciana_consejo", rng, 4, &vistos);
        entrar(e, s, "assets/levels/ciudad_en_venordemn.json", &vistos);
        paso(e, s, cat, "en_archivera_espejos", rng, 4, &vistos);   // ve claro
        entrar(e, s, "assets/levels/ciudad_en_qethatos.json", &vistos);
        paso(e, s, cat, "en_cronista_qethatos", rng, 4, &vistos);   // halla claro
        paso(e, s, cat, "en_cronista_qethatos", rng, 4, &vistos);   // obra maestra
        paso(e, s, cat, "en_anciana_consejo", rng, 4, &vistos);     // sella
        entrar(e, s, "assets/levels/ciudad_en_klimnebra.json", &vistos);
        paso(e, s, cat, "en_capitan_ventobelico", rng, 4, &vistos); // aviso firme
        NarrativeResult n = e.tick(s);
        if (n.fired) { vistos.insert(n.beatId); suelta(n); }
        NarrativeResult c = e.tick(s);
        if (c.fired) { vistos.insert(c.beatId); suelta(c); }
        require(s.hasFlag("en_sc_consejo_sella"));
        require(s.hasFlag("en_sc_cortes_ve_claro"));
        require(s.hasFlag("en_sc_archivo_halla_claro"));
        require(s.hasFlag("en_sc_teatro_obra"));
        require(s.hasFlag("en_sc_choubar_aviso"));
        require(s.hasFlag("en_sc_cerrado"));
        std::printf("  -> cierre OJOS ABIERTOS + SELLO: OK\n\n");
    }

    // -----------------------------------------------------------------
    // Cobertura: todo beat que no sea variante excluyente tiene que haber
    // disparado en alguna ruta.
    // -----------------------------------------------------------------
    std::printf("[cobertura]\n");
    const char* esperados[] = {
        "beat_llegada_guskedor", "beat_enviada", "beat_enviada_repite",
        "beat_anciana_primera", "beat_anciana_repite", "beat_persuadir_consejo",
        "beat_consejo_duda", "beat_consejo_sabe", "beat_consejo_sella_relato",
        "beat_venordemn_llegada", "beat_archivera",
        "beat_cortes_roza_relato", "beat_cortes_ve_relato",
        "beat_cortes_claro_relato",
        "beat_qethatos_llegada", "beat_cronista_primera",
        "beat_archivo_roza_relato", "beat_archivo_halla_relato",
        "beat_archivo_claro_relato",
        "beat_teatro", "beat_teatro_flop_relato", "beat_teatro_bien_relato",
        "beat_teatro_obra_relato",
        "beat_klimnebra_llegada", "beat_capitan_primera",
        "beat_choubar_porton_relato", "beat_choubar_aviso_relato",
        "beat_noticia",
        "cierre_ojos", "cierre_parcial", "cierre_ciego",
    };
    int faltan = 0;
    for (const char* id : esperados) {
        if (vistos.find(id) == vistos.end()) {
            std::printf("    BEAT FALTANTE: %s\n", id);
            ++faltan;
        }
    }
    require(faltan == 0);
    std::printf("    %zu/%zu beats esperados disparados "
                "(las variantes no jugadas se excluyen por definicion)\n",
                vistos.size(), sizeof(esperados) / sizeof(esperados[0]));

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
