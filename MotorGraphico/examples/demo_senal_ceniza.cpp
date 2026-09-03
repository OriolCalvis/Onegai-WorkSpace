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
// Cada guion contiene los dados del jugador y del rival, en ese orden.
// GameSession resuelve las tiradas. El stat por ruta simula el heroe:
// la ruta de criticos usa pools de 4 dados (un heroe carismatico).
//
// GL-free: corre en CI.
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "Check.h"
#include "ScriptedRng.h"
#include "NarrativeCheckSession.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "RPG/NarrativeEngine.h"

using RPG::AdventureScript;
using Nd6SkillCatalog = RPG::Catalogs::SkillCatalog;
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

// Resuelve los skillCheck mediante la sesion real, sin rewind: cada check
// consume sus dos pools en orden. valorStat = dados del heroe de esta ruta.
int resolver(const Nd6SkillCatalog& cat, int valorStat, const NarrativeResult& r,
             ScriptedRng& rng, NarrativeState& s) {
    int n = 0;
    for (const auto& c : r.skillChecks) {
        demo::resolveNarrativeCheck(cat, valorStat, c, rng, s);
        ++n;
    }
    return n;
}

// Un paso de "talk con posible tirada + tick del resultado".
void paso(NarrativeEngine& e, NarrativeState& s, const Nd6SkillCatalog& cat,
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

    Nd6SkillCatalog cat;
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
        // SUCCESS en los cinco checks: 1.5 exitos del heroe vs 0.5 del rival.
        ScriptedRng rng({
            6, 5,       5, 2, 2,     // arcano: 2 vs 3 dados
            6, 5, 1,    5, 2,        // percepcion: 3 vs 2
            6, 5,       5, 2,        // teatro: 2 vs 2
            6, 5,       5, 2, 2,     // consejo: 2 vs 3
            6, 5, 1,    5, 2, 2, 2   // capitan: 3 vs 4
        });
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
        require(rng.consumed() == 26);
        std::printf("  -> cierre OJOS ABIERTOS: OK\n\n");
    }

    // -----------------------------------------------------------------
    // RUTA 2 — medio despierto: pruebas pero el Consejo solo duda.
    // -----------------------------------------------------------------
    std::printf("[RUTA 2: medio despierto]\n");
    {
        NarrativeEngine e; e.setAdventure(&guion);
        NarrativeState s;
        // Empate en arcano y consejo; exito en percepcion; pifia en teatro.
        ScriptedRng rng({
            5, 5,       5, 5, 2,     // 1 vs 1: PARTIAL
            6, 5, 1,    5, 2,        // 1.5 vs 0.5: SUCCESS
            1, 1,       5, 2,        // pifia propia: BOTCH
            5, 5,       5, 5, 2      // 1 vs 1: PARTIAL
        });
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
        require(rng.consumed() == 19);
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
        ScriptedRng rng({
            5, 1, 1,    5, 2,        // percepcion: empate a 0.5
            1, 1,       5, 2, 2, 2   // capitan: pifia propia
        });
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
        require(rng.consumed() == 11);
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
        // Todo-seises del heroe supera al rival en los cinco checks.
        ScriptedRng rng({
            6, 6, 6, 6,    5, 2, 2,
            6, 6, 6, 6,    5, 2,
            6, 6, 6, 6,    5, 2,
            6, 6, 6, 6,    5, 2, 2,
            6, 6, 6, 6,    5, 2, 2, 2
        });
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
        require(rng.consumed() == 34);
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
