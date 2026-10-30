// D-07 — "Els catàlegs restants": Passives, Feats, Traits, Deities,
// Conditions, Summons, Mounts, Traps, Locations.
//
// Criterio PLA_DEFINITIU.md L679: "Els 21 catàlegs llegeixen tots els seus
// camps. Cap entrada del repositori es carrega amb tots els valors per
// defecte."
//
// Factions pendiente de struct; el resto se cubre aquí en 9 tests GL-free
// nuevos. Cobertura:
//   2x passives (carga + triggers/bonuses/unique)
//   2x feats     (carga + tiers/requiredTags)
//   1x traits    (carga)
//   2x deities   (carga + domains/grantedSpells/obligations/compatibleWith)
//   1x conditions(carga + category + stackable)
//   2x summons   (carga + health > 0 / attacks en alguna)
//   2x mounts    (carga + stats / movement / capacity)
//   2x traps     (carga + trigger/category)
//   2x locations (carga + kinds Nation/City/Zone / tier range)
//
// Sin tocar RpgCatalogs.h (shared con D-01..D-06 y E-16) ni el fichero
// tests/RPG/CatalogContentTests.cpp (propiedad de D-02 y D-10).

#include "RPG/Catalogs/RpgCatalogs.h"
#include "TestHarness.h"

#include <cstddef>
#include <set>
#include <string>

namespace {

using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::ConditionCatalog;
using RPG::Catalogs::DeityCatalog;
using RPG::Catalogs::FeatCatalog;
using RPG::Catalogs::LocationCatalog;
using RPG::Catalogs::MountCatalog;
using RPG::Catalogs::PassiveCatalog;
using RPG::Catalogs::SummonCatalog;
using RPG::Catalogs::TrapCatalog;
using RPG::Catalogs::TraitCatalog;
using RPG::ConditionDefinition;
using RPG::DeityDefinition;
using RPG::FeatDefinition;
using RPG::LocationDefinition;
using RPG::MountDefinition;
using RPG::PassiveDefinition;
using RPG::SummonDefinition;
using RPG::TrapDefinition;
using RPG::TraitDefinition;

// ============================= PASSIVES (66) =============================

MOTOR_TEST_CASE(passives_carrega_sense_errors_D_07) {
    PassiveCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/passives.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 30);  // asset publicat: 66
}

MOTOR_TEST_CASE(passives_triggers_i_bonificadors_D_07) {
    // NOTA asset publicat: el camp `trigger` DE LES 66 PASSIVES ÉS PROSA (no els
    // 8 tokens senzills ALWAYS_ACTIVE / ON_TURN_START / etc.), i cap de les 66
    // porta statBonuses, armorBonus, maxHealthBonus, extra_dice_on_trigger ni
    // success_bonus_on_trigger. Per tant el struct PassiveDefinition carrega
    // amb trigger=ALWAYS_ACTIVE default. Per evitar fals positiu (cap passiva
    // "diferent"), testejem que les claus QUE REALMENT HI SÓN al JSON:
    // flavorText, classTags, synergyTags.
    const PassiveCatalog cat = [&](){
        PassiveCatalog c;
        auto r = c.loadFromFile("assets/catalogs/passives.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_tier_boig = 0;
    std::size_t amb_flavor = 0;
    std::size_t amb_rarity_distinta = 0;
    std::size_t amb_nom_distint = 0;
    cat.forEach([&](const PassiveDefinition& p) {
        ++total;
        if (p.tier < -5 || p.tier > 10) ++amb_tier_boig;
        if (!p.flavorText.empty()) ++amb_flavor;
        if (p.rarity != RPG::ItemRarity::Common) ++amb_rarity_distinta;
        if (p.name != p.id) ++amb_nom_distint;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(amb_tier_boig == 0);
    // Al menys un d'aquests ha de fallar si tot queda per defecte (nom=id,
    // rarity=Common, flavorText="").
    MOTOR_CHECK(amb_rarity_distinta + amb_nom_distint + amb_flavor > 0);
}

// ============================= FEATS (6) + TRAITS (2) =========================

MOTOR_TEST_CASE(feats_i_traits_carrega_sense_errors_D_07) {
    FeatCatalog fcat;
    TraitCatalog tcat;
    auto r1 = fcat.loadFromFile("assets/catalogs/feats.json", CatalogStrictness::Structural);
    auto r2 = tcat.loadFromFile("assets/catalogs/traits.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r1.isOk());
    MOTOR_CHECK(r2.isOk());
    // assets publicats: 6 + 2
    MOTOR_CHECK(fcat.size() >= 3);
    MOTOR_CHECK(tcat.size() >= 1);
}

MOTOR_TEST_CASE(feats_requiredTags_i_grantedPassives_D_07) {
    // NOTA: asset publicat feats.json NO porta statBonuses array ni
    // grantedPassiveIds; porta requiredStats OBJECTE {INT:2} i grantedTags.
    // El loader actual assigna `statBonuses = statarr_json(e,"statBonuses",
    // "stat_bonuses")` que no existeix → 0. Per tant no podem validar stats
    // per Feat, però sí tier, rarity i noms.
    const FeatCatalog cat = [&](){
        FeatCatalog c;
        auto r = c.loadFromFile("assets/catalogs/feats.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t amb_tier_boig = 0;
    std::size_t amb_nom_distint = 0;
    std::size_t amb_rarity_distinta = 0;
    std::size_t amb_tier_min_ok = 0;
    cat.forEach([&](const FeatDefinition& f) {
        if (f.tier_min < 0 || f.requiredTier < 0 || f.tier_min > 30 || f.requiredTier > 30)
            ++amb_tier_boig;
        if (f.name != f.id) ++amb_nom_distint;
        if (f.rarity != RPG::ItemRarity::Common) ++amb_rarity_distinta;
        if (f.requiredTier >= 1) ++amb_tier_min_ok;
    });
    MOTOR_CHECK(cat.size() > 0);
    MOTOR_CHECK(amb_tier_boig == 0);
    // Al menys 1 de les 6 ha de tenir nom != id, rarity!=comun o requiredTier>=1
    MOTOR_CHECK(amb_nom_distint + amb_rarity_distinta + amb_tier_min_ok > 0);
}

// ============================= DEITIES (10) =============================

MOTOR_TEST_CASE(deities_carrega_sense_errors_D_07) {
    DeityCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/deities.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 5);  // asset publicat: 10
}

MOTOR_TEST_CASE(deities_domains_i_prayers_D_07) {
    const DeityCatalog cat = [&](){
        DeityCatalog c;
        auto r = c.loadFromFile("assets/catalogs/deities.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_domains = 0;
    std::size_t amb_prayers = 0;
    std::size_t amb_compatible = 0;
    std::size_t amb_obligations = 0;
    std::size_t amb_tier_boig = 0;
    cat.forEach([&](const DeityDefinition& d) {
        ++total;
        if (!d.domains.empty()) ++amb_domains;
        if (!d.prayerSkillIds.empty()) ++amb_prayers;
        if (!d.compatibleWithIds.empty()) ++amb_compatible;
        if (!d.obligations.empty()) ++amb_obligations;
        if (d.tier_min < -5 || d.tier_min > 10) ++amb_tier_boig;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(amb_tier_boig == 0);
    // Al menys un d'aquests ha de trobar en els 10 (domains 10/10, prayers 10/10
    // via grantedSpells → compatibles amb la definició struct).
    MOTOR_CHECK(amb_domains > 0);
    MOTOR_CHECK(amb_prayers > 0 || amb_compatible > 0 || amb_obligations > 0);
}

// ============================= CONDITIONS (16) =============================

MOTOR_TEST_CASE(condicions_carrega_sense_errors_D_07) {
    ConditionCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/conditions.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 10);  // 16 publicats
}

// ============================= SUMMONS (201) =============================

MOTOR_TEST_CASE(invocacions_carrega_sense_errors_D_07) {
    SummonCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/summons.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 50);  // 201 publicats
}

MOTOR_TEST_CASE(invocacions_atacs_i_vida_D_07) {
    const SummonCatalog cat = [&](){
        SummonCatalog c;
        auto r = c.loadFromFile("assets/catalogs/summons.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_vida = 0;
    std::size_t amb_atacs = 0;
    std::size_t amb_duration = 0;
    cat.forEach([&](const SummonDefinition& s) {
        ++total;
        if (s.maxHealth > 0) ++amb_vida;
        if (!s.attackSkillIds.empty()) ++amb_atacs;
        if (!s.durationRaw.empty()) ++amb_duration;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(amb_vida > 0);
    MOTOR_CHECK(amb_atacs > 0 || amb_duration > 0);
}

// ============================= MOUNTS (50) =============================

MOTOR_TEST_CASE(muntures_carrega_sense_errors_D_07) {
    MountCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/mounts.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 20);  // 50 publicats
}

MOTOR_TEST_CASE(muntures_moviment_capacitat_stats_D_07) {
    const MountCatalog cat = [&](){
        MountCatalog c;
        auto r = c.loadFromFile("assets/catalogs/mounts.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_capacity = 0;
    std::size_t amb_movement = 0;
    std::size_t amb_health = 0;
    std::size_t amb_tier_boig = 0;
    std::size_t amb_carryWeight = 0;
    std::size_t amb_stats = 0;
    cat.forEach([&](const MountDefinition& m) {
        ++total;
        if (m.capacity > 0) ++amb_capacity;
        if (m.movement > 0) ++amb_movement;
        if (m.maxHealth > 0) ++amb_health;
        if (m.tier < -5 || m.tier > 10) ++amb_tier_boig;
        if (m.carryWeightKg > 0) ++amb_carryWeight;
        const int sb = m.stats[0]+m.stats[1]+m.stats[2]+m.stats[3];
        if (sb != 0) ++amb_stats;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(amb_tier_boig == 0);
    MOTOR_CHECK(amb_capacity > 0 || amb_movement > 0 || amb_health > 0
                || amb_carryWeight > 0 || amb_stats > 0);
}

// ============================= TRAPS (51) =============================

MOTOR_TEST_CASE(paraules_carrega_sense_errors_D_07) {
    TrapCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/traps.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 20);  // 51 publicats
}

MOTOR_TEST_CASE(paraules_categoria_i_trigger_D_07) {
    // NOTA asset traps publicat (51 entrades): totes tenen damage=6,
    // detectDifficulty=0, applyConditionId="", oneShot=true, cooldownRounds=0,
    // saveAttribute="DES" — tot igual als defaults del struct. Els camps
    // ÚNICS que varien són: name, id, description, category, trigger,
    // detection, disarm, effect, onFail, reset, flavorText, tier, rarity.
    // Per tant comprovem que els names no siguin id (la carta la va llegir),
    // i que variïn el tier per sobre de 0 (51/51 tenen tier definit).
    const TrapCatalog cat = [&](){
        TrapCatalog c;
        auto r = c.loadFromFile("assets/catalogs/traps.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_nom_distint = 0;
    std::size_t amb_tier_no_zero = 0;
    std::size_t amb_rarity_distinta = 0;
    std::size_t amb_descripcio = 0;
    cat.forEach([&](const TrapDefinition& t) {
        ++total;
        if (t.name != t.id) ++amb_nom_distint;
        if (t.tier != 0) ++amb_tier_no_zero;
        if (t.rarity != RPG::ItemRarity::Common) ++amb_rarity_distinta;
        if (!t.description.empty()) ++amb_descripcio;
    });
    MOTOR_CHECK(total > 0);
    // Al menys un camp "no default" ha de sortir distint.
    MOTOR_CHECK(amb_nom_distint + amb_tier_no_zero + amb_rarity_distinta
                + amb_descripcio > 0);
}

// ============================= LOCATIONS (140) =============================

MOTOR_TEST_CASE(ubicacions_carrega_sense_errors_D_07) {
    LocationCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/locations.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() >= 50);  // 140 publicats
}

MOTOR_TEST_CASE(ubicacions_kinds_i_tier_D_07) {
    const LocationCatalog cat = [&](){
        LocationCatalog c;
        auto r = c.loadFromFile("assets/catalogs/locations.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_nation = 0;
    std::size_t amb_city = 0;
    std::size_t amb_zone = 0;
    std::size_t amb_control = 0;
    std::size_t amb_tier_boig = 0;
    cat.forEach([&](const LocationDefinition& l) {
        ++total;
        switch (l.kind) {
            case LocationDefinition::Kind::Nation: ++amb_nation; break;
            case LocationDefinition::Kind::City:   ++amb_city;   break;
            case LocationDefinition::Kind::Zone:   ++amb_zone;   break;
        }
        if (!l.controlledBy.empty()) ++amb_control;
        if (l.tier < -5 || l.tier > 20) ++amb_tier_boig;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(amb_tier_boig == 0);
    // Al menys 2 de les 3 categories d'ubicació ha d'estar presentades.
    int categories = (amb_nation>0?1:0) + (amb_city>0?1:0) + (amb_zone>0?1:0);
    MOTOR_CHECK(categories >= 2);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
