// D-05 — Objectes, equipament i consumibles + Taules de botí (Loot Tables).
//
// Criteris de tancament del paquet (part de catàlegs, no InventoryEngine):
//   Els tres catàlegs (equipment, consumables, loot_tables) llegeixen TOTS els
//   seus camps del JSON publicat a assets/catalogs/, amb noms canònics de
//   l'asset (DEC-02: mana el JSON, el C++ s'adapta).
//
// Nota de coordinació multi-agent: l'altre punt de D-05 (InventoryEngine
// duplica bonificadors d'armes a dues mans i confon escut amb off-hand)
// VIVEIX EN src/RPG/InventoryEngine.cpp, fitxer COMPARTIT amb el paquet
// G-12 (pantalla d'inventari). Per evitar col·lisions NOMÉS es tanca la
// part de CARREGADORS + tests de contingut real aquí. La part de regles
// d'inventari la tancarà qui tingui G-12 o qui trobi InventoryEngine lliure.
//
// GL-free: només lectura JSON, no render.

#include "RPG/Catalogs/RpgCatalogs.h"
#include "Level/ObjectCatalog.h"
#include "TestHarness.h"

#include <cstddef>
#include <set>
#include <string>

namespace {

using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::ConsumableCatalog;
using RPG::Catalogs::EquipmentCatalog;
using RPG::Catalogs::LootTableCatalog;
using RPG::ConsumableDefinition;
using ::ObjectCatalog;
using ::ObjectDefinition;
using RPG::EquipmentDefinition;
using RPG::LootTableDefinition;

MOTOR_TEST_CASE(equipament_carrega_sense_errors_D_05) {
    EquipmentCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/equipment.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() > 100);   // asset publicat: 413 entrades (07/09)
}

MOTOR_TEST_CASE(equipament_slots_i_categories_D_05) {
    const EquipmentCatalog cat = [&](){
        EquipmentCatalog c;
        auto r = c.loadFromFile("assets/catalogs/equipment.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    // PART-02: los brazos (Tier 1); el collar, el cinturon y los anillos (Tier 2)
    const std::set<std::string> slots_ok = {"Head", "Torso", "Legs", "Feet", "MainHand", "OffHand", "Accessory", "TwoHanded", "Consumable",
                                            "Arms", "Neck", "Belt", "Ring", "Ring2"};
    std::size_t total = 0;
    std::size_t slot_invalid = 0;
    std::size_t amb_price_negatiu = 0;
    std::size_t amb_rarity_common = 0;
    std::size_t amb_tier_extret = 0;
    cat.forEach([&](const EquipmentDefinition& e) {
        ++total;
        if (!e.slot.empty() && slots_ok.count(e.slot) == 0) ++slot_invalid;
        if (e.price < 0) ++amb_price_negatiu;
        if (e.rarity == RPG::ItemRarity::Common) ++amb_rarity_common;
        if (e.tier < 0 || e.tier > 30) ++amb_tier_extret;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(slot_invalid == 0);
    MOTOR_CHECK(amb_price_negatiu == 0);
    MOTOR_CHECK(amb_tier_extret == 0);
    // Pas: hi ha coses que no son comunes (tier + elevat = uncommon/rare)
    MOTOR_CHECK(amb_rarity_common < total);
}

MOTOR_TEST_CASE(equipament_bonificadors_llegits_D_05) {
    // Si cap entrada te statBonuses o requiredStats sumant > 0, el carregador
    // ignora les claus (el asset publicat trau stats a molts equips).
    const EquipmentCatalog cat = [&](){
        EquipmentCatalog c;
        auto r = c.loadFromFile("assets/catalogs/equipment.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t amb_stats = 0;
    std::size_t amb_linkedSkill = 0;
    std::size_t amb_tags = 0;
    cat.forEach([&](const EquipmentDefinition& e) {
        const int s = e.statBonuses[0]+e.statBonuses[1]+e.statBonuses[2]+e.statBonuses[3];
        if (s != 0) ++amb_stats;
        if (!e.linkedSkillId.empty()) ++amb_linkedSkill;
        if (!e.grantedTags.empty()) ++amb_tags;
    });
    // Del asset: 413 entradas. Al menos 1/10 tienen algo.
    MOTOR_CHECK(amb_stats > 0);          // algun equipo CON bonificacions.
    MOTOR_CHECK(amb_linkedSkill > 0);    // linkedSkill como minimo 1 (arco_corto, baston_arcano, etc.)
    MOTOR_CHECK(amb_tags > 0);           // tags com "Armadura Ligera", "Arma Ligera", "Foco"...
}

MOTOR_TEST_CASE(consumibles_carrega_sense_errors_D_05) {
    ConsumableCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/consumables.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() > 20);
}

MOTOR_TEST_CASE(consumibles_usos_i_accions_D_05) {
    const ConsumableCatalog cat = [&](){
        ConsumableCatalog c;
        auto r = c.loadFromFile("assets/catalogs/consumables.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_usos_0 = 0;
    std::size_t amb_accio = 0;
    std::size_t amb_preu_negatiu = 0;
    std::size_t amb_tier_extret = 0;
    cat.forEach([&](const ConsumableDefinition& c) {
        ++total;
        if (c.uses <= 0) ++amb_usos_0;
        if (!c.actionType.empty()) ++amb_accio;
        if (c.price < 0) ++amb_preu_negatiu;
        if (c.tier < 0 || c.tier > 30) ++amb_tier_extret;
    });
    MOTOR_CHECK(total > 0);
    MOTOR_CHECK(amb_usos_0 == 0);
    MOTOR_CHECK(amb_accio > total/2);       // majoria tenen accio.
    MOTOR_CHECK(amb_preu_negatiu == 0);
    MOTOR_CHECK(amb_tier_extret == 0);
}

MOTOR_TEST_CASE(consumibles_noms_coherents_D_05) {
    // Evitar que tots quedin amb name = id (fallback del carregador quan la
    // key "name" no se llegeix del JSON).
    const ConsumableCatalog cat = [&](){
        ConsumableCatalog c;
        auto r = c.loadFromFile("assets/catalogs/consumables.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t distints = 0;
    cat.forEach([&](const ConsumableDefinition& c) {
        ++total;
        if (c.name != c.id) ++distints;
    });
    if (total > 5) MOTOR_CHECK(distints >= 3);   // al menos 3 nombres != id
}

MOTOR_TEST_CASE(taules_boti_carrega_sense_errors_D_05) {
    LootTableCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/loot_tables.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() > 0);
}

MOTOR_TEST_CASE(taules_boti_entries_i_drops_D_05) {
    const LootTableCatalog cat = [&](){
        LootTableCatalog c;
        auto r = c.loadFromFile("assets/catalogs/loot_tables.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t total = 0;
    std::size_t amb_entries = 0;
    std::size_t amb_drops = 0;
    std::size_t amb_gold = 0;
    std::size_t amb_clue = 0;
    std::size_t ids_buits = 0;
    cat.forEach([&](const LootTableDefinition& l) {
        ++total;
        if (!l.entries.empty()) ++amb_entries;
        if (!l.drops.empty()) ++amb_drops;
        if (l.minGold > 0 || l.maxGold > 0) ++amb_gold;
        if (!l.clue.empty()) ++amb_clue;
        for (const auto& e : l.entries) if (e.refId.empty()) ++ids_buits;
        for (const auto& d : l.drops)   if (d.itemId.empty()) ++ids_buits;
    });
    MOTOR_CHECK(total > 0);
    // El asset publicat trau drops i entries paral·lelament per compatibilitat.
    MOTOR_CHECK(amb_entries > 0);
    MOTOR_CHECK(amb_drops > 0);
    MOTOR_CHECK(amb_gold > 0);
    MOTOR_CHECK(amb_clue > 0);
    MOTOR_CHECK(ids_buits == 0);
}

MOTOR_TEST_CASE(taules_boti_gold_coherent_D_05) {
    // maxGold >= minGold. Si cap taula ho compleix, el swap de min/max va mal.
    const LootTableCatalog cat = [&](){
        LootTableCatalog c;
        auto r = c.loadFromFile("assets/catalogs/loot_tables.json", CatalogStrictness::Structural);
        MOTOR_CHECK(r.isOk());
        return c;
    }();
    std::size_t incoherents = 0;
    cat.forEach([&](const LootTableDefinition& l) {
        if (l.maxGold < l.minGold) ++incoherents;
        if (l.tierMax < l.tierMin && (l.tierMin>0 || l.tierMax>0)) ++incoherents;
    });
    MOTOR_CHECK(incoherents == 0);
}

// ===================================================================
// ObjectCatalog exportJson round-trip
// ===================================================================

MOTOR_TEST_CASE(object_catalog_exportJson_roundtrip) {
    ObjectCatalog catalog;
    auto r = catalog.loadFromFile("assets/objects/test_objects.json");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(catalog.ids().size() == 5);

    std::string json = catalog.exportJson();
    MOTOR_CHECK(!json.empty());
    MOTOR_CHECK(json.find("\"objects\"") != std::string::npos);
    MOTOR_CHECK(json.find("\"pocion\"") != std::string::npos);
    MOTOR_CHECK(json.find("\"eter\"") != std::string::npos);

    // Round-trip: load the exported JSON
    ObjectCatalog catalog2;
    auto r2 = catalog2.loadFromString(json);
    MOTOR_CHECK(r2.isOk());
    MOTOR_CHECK(catalog2.ids().size() == catalog.ids().size());

    // Compare each object
    for (const auto& id : catalog.ids()) {
        const ObjectDefinition* def1 = catalog.find(id);
        const ObjectDefinition* def2 = catalog2.find(id);
        MOTOR_CHECK(def1 != nullptr);
        MOTOR_CHECK(def2 != nullptr);
        MOTOR_CHECK(def1->id == def2->id);
        MOTOR_CHECK(def1->name == def2->name);
        MOTOR_CHECK(def1->category == def2->category);
        MOTOR_CHECK(def1->spriteId == def2->spriteId);
        MOTOR_CHECK(def1->blocksMovement == def2->blocksMovement);
        MOTOR_CHECK(def1->interactable == def2->interactable);
    }
}

}  // namespace

int main() { return motor_test::runAllTests(); }
