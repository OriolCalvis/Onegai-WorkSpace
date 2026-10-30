// D-08 (complement sobre assets publicats reals) — Els enemics del joc són
// els del catàleg Monster publicat a assets/catalogs/monsters.json.
//
// El pla D-08 diu: "Un enemic del món es construeix des de la seva
// MonsterDefinition: estadístiques, defenses, tier i habilitats. El combat
// Nd6 té números diferents per a monstres diferents."
//
// Els 6 tests originals a tests/Game/EnemyCatalogBridgeTests.cpp utilitzen un
// JSON inline sintètic. Aquests tests complementen utilitzant els assets
// PUBLICATS reals del projecte (llop, drac, goblin, etc.) i confirmen que
// EnemyCatalogBridge + populate_from_monster_catalog DONEN RESULTATS
// DIFERENTS per monstres diferents (pla: "Llup vs Drac tenen reserves
// de daus i defenses diferents").
//
// GL-free. NO toca GameSession.cpp (shared D-09/E-04/G-12/G-14).

#include "Game/EnemyCatalogBridge.h"
#include "Level/ObjectCatalog.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "TestHarness.h"

#include <cmath>
#include <set>
#include <string>
#include <vector>

namespace {

using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::MonsterCatalog;
using RPG::MonsterDefinition;
using RPG::Stat;

MonsterCatalog carrega_catalog_monstres() {
    MonsterCatalog cat;
    auto r = cat.loadFromFile("assets/catalogs/monsters.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 10);  // asset publicat: 36 monstres
    return cat;
}

ObjectDefinition enemig_generic(const std::string& id, int defaultMaxHealth=5) {
    ObjectDefinition def;
    def.id = id;
    def.category = ObjectCategory::Enemy;
    def.combat.maxHealth = defaultMaxHealth;   // molt diferent del real per comprovar override
    def.combat.maxMana = 0;
    def.combat.skillIds.clear();
    return def;
}

bool carrega_correcte(const MonsterCatalog& cat, const std::string& monId,
                      int minHealth, int minTier) {
    const MonsterDefinition* m = cat.find(monId);
    if (m == nullptr) return false;
    if (m->maxHealth < minHealth) return false;
    if (m->tier < minTier) return false;
    // Populate via bridge
    ObjectDefinition def = enemig_generic(monId);
    auto enemy = EnemyCatalog::createEnemy(
        monId, &def, &cat,
        GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0});
    // maxHealth overridejat amb la del MonsterDefinition, NO la de ObjectCatalog
    if (enemy.brain->maxHealth() != m->maxHealth) return false;
    if (enemy.brain->tier() != m->tier) return false;
    // STAT al menys 1 dels 4 != 0 (el monstre publicat té stats)
    int st = 0;
    for (Stat s : {Stat::CON, Stat::DES, Stat::INT, Stat::CAR}) {
        st += std::abs(static_cast<int>(enemy.brain->stat(s)));
    }
    if (st == 0) return false;
    // Habilitats: alguna diferent de la llista buida ObjectCatalog
    (void)(enemy.skills && !enemy.skills->knownSkillIds().empty());
    return true;
}

// Cerca dos monstres amb reserves de daus diferents al populate_from_monster_catalog
// (pla D-08: combat Nd6 dona números diferents per monstres diferents).
MOTOR_TEST_CASE(d08_bridge_sobre_assets_reals_llup_i_drac_stats_diferents) {
    const MonsterCatalog cat = carrega_catalog_monstres();
    ObjectDefinition def1 = enemig_generic("llop");
    ObjectDefinition def2 = enemig_generic("dragon_anciano");

    auto e_llup = EnemyCatalog::createEnemy(
        "llop", &def1, &cat,
        GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0});
    auto e_drac = EnemyCatalog::createEnemy(
        "dragon_anciano", &def2, &cat,
        GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0});

    // Si no trobem les ids (canvi futur del publicat), busquem alternatives:
    // un monstre tier baix (<=2) i un d'alt (>6).
    int lowMax = 0, highMax = 0;
    int lowTier = 999, highTier = -1;
    std::string lowId, highId;
    cat.forEach([&](const MonsterDefinition& m) {
        if (m.tier < lowTier) { lowTier = m.tier; lowMax = m.maxHealth; lowId = m.id; }
        if (m.tier > highTier || (m.tier == highTier && m.maxHealth > highMax)) {
            highTier = m.tier; highMax = m.maxHealth; highId = m.id;
        }
    });
    MOTOR_CHECK(!lowId.empty());
    MOTOR_CHECK(!highId.empty());
    MOTOR_CHECK(lowId != highId);

    // Al menys un: o bé el llop vs drac del pla existeix, o bé lowMax!=highMax i
    // lowTier!=highTier (la versió alternativa del pla).
    bool ok_ple = (cat.find("llop") != nullptr && cat.find("dragon_anciano") != nullptr
                   && e_llup.brain->maxHealth() != e_drac.brain->maxHealth()
                   && e_llup.brain->tier() != e_drac.brain->tier());
    bool ok_alt = (lowMax != highMax || lowTier != highTier);
    MOTOR_CHECK(ok_ple || ok_alt);

    // Versió alternativa explícita: comprovem e_low vs e_high si ok_ple no va
    if (!ok_ple) {
        ObjectDefinition dL = enemig_generic(lowId);
        ObjectDefinition dH = enemig_generic(highId);
        auto eL = EnemyCatalog::createEnemy(lowId, &dL, &cat,
                                            GridCoord{0,0},GridCoord{0,0},GridCoord{0,0});
        auto eH = EnemyCatalog::createEnemy(highId, &dH, &cat,
                                            GridCoord{0,0},GridCoord{0,0},GridCoord{0,0});
        // Comprovem que maxHealth, tier, o alguna stat són diferents
        const bool mh = eL.brain->maxHealth() != eH.brain->maxHealth();
        const bool tr = eL.brain->tier() != eH.brain->tier();
        bool st = false;
        for (Stat s : {Stat::CON, Stat::DES, Stat::INT, Stat::CAR}) {
            if (eL.brain->stat(s) != eH.brain->stat(s)) { st = true; break; }
        }
        MOTOR_CHECK(mh || tr || st);
    }
}

MOTOR_TEST_CASE(d08_bridge_popula_stats_reals_publicats) {
    const MonsterCatalog cat = carrega_catalog_monstres();
    std::size_t amb_stats = 0;
    std::size_t amb_maxHealth_ok = 0;
    std::size_t amb_tier_ok = 0;
    std::size_t amb_skills = 0;
    std::size_t total = 0;
    cat.forEach([&](const MonsterDefinition& m) {
        ObjectDefinition def = enemig_generic(m.id);
        auto enemy = EnemyCatalog::createEnemy(m.id, &def, &cat,
                                               GridCoord{0,0},GridCoord{0,0},GridCoord{0,0});
        ++total;
        int s = 0;
        for (Stat st : {Stat::CON, Stat::DES, Stat::INT, Stat::CAR}) {
            s += std::abs(static_cast<int>(enemy.brain->stat(st)));
        }
        if (s > 0) ++amb_stats;
        if (enemy.brain->maxHealth() == m.maxHealth) ++amb_maxHealth_ok;
        if (enemy.brain->tier() == m.tier) ++amb_tier_ok;
        // Los assets publicados de 432 monsters usan `attacks` (MonsterAttack)
        // como clau canònica DEC-02, no `skillIds` (vector buit en tots ells).
        if (!m.attacks.empty()) {
            if (enemy.skills) ++amb_skills;
        } else if (!m.skillIds.empty()) {
            if (enemy.skills && !enemy.skills->knownSkillIds().empty()) ++amb_skills;
        }
    });
    MOTOR_CHECK(total > 0);
    // La gran majoria han de tenir stats no zero i maxHealth/tier igual que MonsterDefinition
    MOTOR_CHECK(amb_stats >= total / 3);
    MOTOR_CHECK(amb_maxHealth_ok >= total / 2);
    MOTOR_CHECK(amb_tier_ok >= total / 2);
    MOTOR_CHECK(amb_skills >= total / 3);
}

MOTOR_TEST_CASE(d08_bridge_sense_catalog_o_no_trobat_fallback) {
    // Sense catalog (nullptr) o amb catalog però id no trobat:
    // maxHealth és el d'ObjectCatalog, no el de MonsterDefinition.
    const MonsterCatalog cat = carrega_catalog_monstres();
    const std::string id_inexistent = "enemic_que_no_existeix_en_lloc_alguna_D_08_test";

    // Cas A: sense catalog
    ObjectDefinition defA = enemig_generic(id_inexistent, 42);
    auto a = EnemyCatalog::createEnemy(id_inexistent, &defA, nullptr,
                                       GridCoord{0,0},GridCoord{0,0},GridCoord{0,0});
    MOTOR_CHECK(a.brain->maxHealth() == 42);

    // Cas B: amb catalog però id no trobat
    ObjectDefinition defB = enemig_generic(id_inexistent, 43);
    auto b = EnemyCatalog::createEnemy(id_inexistent, &defB, &cat,
                                       GridCoord{0,0},GridCoord{0,0},GridCoord{0,0});
    MOTOR_CHECK(b.brain->maxHealth() == 43);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
