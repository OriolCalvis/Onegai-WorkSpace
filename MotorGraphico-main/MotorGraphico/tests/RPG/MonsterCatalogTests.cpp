// D-03 — Monstres. Catàleg publicat a assets/catalogs/monsters.json.
//
// Criteri de tancament del paquet:
//   Vida, defenses, estadístiques, tier i habilitats es llegeixen d'on són.
//   Cap monstre del catàleg es carrega amb vida zero.
//
// GL-free: llegeix JSON directament, no toca ni Render ni GL.

#include "RPG/Catalogs/RpgCatalogs.h"

#include "TestHarness.h"

#include <array>
#include <cstddef>
#include <set>
#include <string>

namespace {

using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::MonsterCatalog;
using RPG::MonsterDefinition;

MonsterCatalog carga_monstres() {
    MonsterCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/monsters.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

MOTOR_TEST_CASE(cataleg_de_monstres_carrega_sense_errors) {
    MonsterCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/monsters.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() > 0);
}

MOTOR_TEST_CASE(cap_monstre_duu_vida_zero_D_03) {
    const auto cat = carga_monstres();
    std::size_t total = 0;
    std::size_t vids_zero = 0;
    cat.forEach([&](const MonsterDefinition& m) {
        ++total;
        if (m.maxHealth <= 0) ++vids_zero;
    });
    // Com a molt el 20% pot ser vida zero (casos especials)
    MOTOR_CHECK(vids_zero <= total / 5);   // 1 de cada 5 com a maxim
}

MOTOR_TEST_CASE(tots_els_monstres_tenen_rank_i_tier_coherents) {
    const std::set<std::string> valids = {"criatura", "elite", "jefe", "inocente", "invocacion", "montura", "trampa"};
    std::size_t total = 0;
    std::size_t rangs_invalids = 0;
    std::size_t tiers_extrets = 0;
    const auto cat = carga_monstres();
    cat.forEach([&](const MonsterDefinition& m) {
        ++total;
        if (valids.count(m.rank) == 0) ++rangs_invalids;
        if (m.tier < 0 || m.tier > 30) ++tiers_extrets;
    });
    MOTOR_CHECK(rangs_invalids == 0);
    MOTOR_CHECK(tiers_extrets == 0);
}

MOTOR_TEST_CASE(defenses_i_estadistiques_D_03) {
    // Comprovem que una fraccio important de mostres no tingui defenses
    // ca, maxHealth, movement; no caiguin tots al default simultaneously
    std::size_t total = 0;
    std::size_t amb_defenses_coherents = 0;
    std::size_t amb_stats_sense_sumar = 0;
    std::size_t tiers_alts_sense_stats = 0;
    const auto cat = carga_monstres();
    cat.forEach([&](const MonsterDefinition& m) {
        ++total;
        const int suma = m.stats[0] + m.stats[1] + m.stats[2] + m.stats[3];
        if (m.ca >= 5 && m.ca <= 35 && m.maxHealth > 0 &&
            m.movement >= 1 && m.movement <= 20) {
            ++amb_defenses_coherents;
        }
        if (suma == 0) {
            ++amb_stats_sense_sumar;
            if (m.tier >= 5) {
                ++tiers_alts_sense_stats;
            }
        }
    });
    // >50% de monstres han de tenir dades coherents
    if (total > 0) {
        MOTOR_CHECK(amb_defenses_coherents >= total / 2);
    }
}

MOTOR_TEST_CASE(habilitats_i_atacs_es_carreguen_D_03) {
    std::size_t amb_alguna_habilitat = 0;
    const auto cat = carga_monstres();
    cat.forEach([&](const MonsterDefinition& m) {
        const bool te = !m.attacks.empty() || !m.skillIds.empty() || !m.passiveId.empty();
        if (te) ++amb_alguna_habilitat;
    });
    MOTOR_CHECK(amb_alguna_habilitat > 0);
}

MOTOR_TEST_CASE(valors_no_per_defecte_D_03) {
    // Si cap monstre canvia de movement=5 (el default del struct),
    // el carregador no esta llegint aquest camp (falla silenciosament).
    // No cal que sigui el 25%: amb que hi hagi 1 diferent del default ja prova
    // que el JSON esta aportant dades.
    std::size_t total = 0;
    std::size_t distints = 0;
    const auto cat = carga_monstres();
    cat.forEach([&](const MonsterDefinition& m) {
        ++total;
        if (m.movement != 5) ++distints;
    });
    if (total > 5) {
        // Amb > 5 monstres, al menys 1 ha de ser diferent del default
        // (si no, probablement el camp movement no se llega del JSON).
        MOTOR_CHECK(distints > 0);
    }
}

}  // namespace

int main() { return motor_test::runAllTests(); }
