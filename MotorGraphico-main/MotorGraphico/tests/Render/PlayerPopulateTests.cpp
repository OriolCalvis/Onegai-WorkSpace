// Fractura #1 — Player::populate_from_class_race_background()
//
// Mirror of EnemyBrain::populate_from_monster_catalog() for player characters.
// Populates BOTH the external CharacterSheet AND the legacy Player fields
// from ClassDefinition, RaceDefinition, and BackgroundDefinition.

#include "TestHarness.h"

#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/TierRules.h"
#include "Render/Player.h"

namespace {

RPG::ClassDefinition makeWarrior() {
    RPG::ClassDefinition cls;
    cls.id = "guerrero";
    cls.name = "Guerrero";
    cls.tier = 1;
    cls.role = "martial";
    cls.baseHealth = 10;
    cls.primaryStat = "CON";
    cls.secondaryStat = "DES";
    cls.maxSkillCards = 10;
    cls.startingSkillIds = {"skill_corte_basico", "skill_tajo_poderoso"};
    cls.startingEquipmentIds = {"equip_espada_corta", "equip_escudo_madera"};
    cls.passiveId = "passive_resistencia_guerrero";
    return cls;
}

RPG::RaceDefinition makeHuman() {
    RPG::RaceDefinition race;
    race.id = "humano";
    race.name = "Humano";
    race.tier = 1;
    race.statBonuses = {0, 0, 0, 0};
    return race;
}

RPG::RaceDefinition makeDwarf() {
    RPG::RaceDefinition race;
    race.id = "enano";
    race.name = "Enano";
    race.tier = 1;
    race.statBonuses = {2, 0, 0, 0};
    race.passiveTraitId = "trait_resistencia_enana";
    race.racialTraitId = "trait_vision_oscura";
    return race;
}

RPG::BackgroundDefinition makeSoldier() {
    RPG::BackgroundDefinition bg;
    bg.id = "soldado";
    bg.name = "Soldado";
    bg.tier = 0;
    bg.startingWealthGold = 15;
    bg.skillProficiencyIds = {"skill_marciales_menores"};
    bg.featureIds = {"feat_rango_militar"};
    bg.factionId = "faccion_legion";
    bg.startingEquipmentIds = {"equip_espada_corta"};
    return bg;
}

RPG::BackgroundDefinition makeNoble() {
    RPG::BackgroundDefinition bg;
    bg.id = "noble";
    bg.name = "Noble";
    bg.tier = 0;
    bg.startingWealthGold = 50;
    bg.skillProficiencyIds = {"skill_etiqueta"};
    bg.featureIds = {"feat_contactos_nobles"};
    bg.startingEquipmentIds = {"equip_amuleto_nobleza"};
    return bg;
}

RPG::TierRules makeTierRules() {
    return RPG::TierRules();
}

bool contains(const std::vector<std::string>& v, const std::string& val) {
    for (const auto& s : v) { if (s == val) return true; }
    return false;
}

}  // namespace

MOTOR_TEST_CASE(populate_basic_warrior_human) {
    auto cls = makeWarrior();
    auto race = makeHuman();
    auto bg = makeSoldier();
    auto rules = makeTierRules();

    RPG::CharacterSheet sheet;
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&sheet);

    std::array<int, 4> baseStats = {4, 3, 2, 3};
    player.populate_from_class_race_background(cls, race, &bg, "Artorias", baseStats, rules);

    MOTOR_CHECK(sheet.id == "pj_guerrero");
    MOTOR_CHECK(sheet.displayName == "Artorias");
    MOTOR_CHECK(sheet.raceId == "humano");
    MOTOR_CHECK(sheet.classId == "guerrero");
    MOTOR_CHECK(sheet.backgroundId == "soldado");
    MOTOR_CHECK(sheet.tier == 1);

    MOTOR_CHECK(sheet.baseStats[0] == 4);
    MOTOR_CHECK(sheet.baseStats[1] == 3);
    MOTOR_CHECK(sheet.baseStats[2] == 2);
    MOTOR_CHECK(sheet.baseStats[3] == 3);
    MOTOR_CHECK(sheet.racialBonuses[0] == 0);

    MOTOR_CHECK(sheet.knownSkillIds.size() >= 2);
    MOTOR_CHECK(sheet.knownSkillIds[0] == "skill_corte_basico");
    MOTOR_CHECK(sheet.knownSkillIds[1] == "skill_tajo_poderoso");
    MOTOR_CHECK(contains(sheet.knownSkillIds, "skill_marciales_menores"));

    int equipCount = 0;
    for (const auto& p : sheet.inventoryEquipment) {
        if (p.first == "equip_espada_corta") ++equipCount;
    }
    MOTOR_CHECK(equipCount == 1);

    MOTOR_CHECK(sheet.passiveIds.size() >= 1);
    MOTOR_CHECK(sheet.passiveIds[0] == "passive_resistencia_guerrero");
    MOTOR_CHECK(sheet.gold == 15);
    MOTOR_CHECK(sheet.primaryFactionId == "faccion_legion");
    MOTOR_CHECK(sheet.featIds.size() >= 1);

    MOTOR_CHECK(player.tier() == 1);
    MOTOR_CHECK(player.combatant_id() == "pj_guerrero");
    MOTOR_CHECK(player.health() > 0);
    MOTOR_CHECK(player.health() == player.maxHealth());
}

MOTOR_TEST_CASE(populate_dwarf_racial_bonuses_and_traits) {
    auto cls = makeWarrior();
    auto race = makeDwarf();
    auto rules = makeTierRules();

    RPG::CharacterSheet sheet;
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&sheet);

    std::array<int, 4> baseStats = {3, 3, 3, 3};
    player.populate_from_class_race_background(cls, race, nullptr, "Thorin", baseStats, rules);

    MOTOR_CHECK(sheet.racialBonuses[0] == 2);
    MOTOR_CHECK(sheet.racialBonuses[1] == 0);
    MOTOR_CHECK(player.stat(RPG::Stat::CON) == 5);

    MOTOR_CHECK(sheet.traitIds.size() >= 2);
    MOTOR_CHECK(contains(sheet.traitIds, "trait_resistencia_enana"));
    MOTOR_CHECK(contains(sheet.traitIds, "trait_vision_oscura"));

    MOTOR_CHECK(sheet.backgroundId == "");
    MOTOR_CHECK(sheet.gold == 0);
}

MOTOR_TEST_CASE(populate_noble_background_no_duplicates) {
    auto cls = makeWarrior();
    auto race = makeHuman();
    auto bg = makeNoble();
    auto rules = makeTierRules();

    RPG::CharacterSheet sheet;
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&sheet);

    std::array<int, 4> baseStats = {4, 3, 2, 3};
    player.populate_from_class_race_background(cls, race, &bg, "Prestige", baseStats, rules);

    bool hasAmulet = false;
    for (const auto& p : sheet.inventoryEquipment) {
        if (p.first == "equip_amuleto_nobleza") hasAmulet = true;
    }
    MOTOR_CHECK(hasAmulet);
    MOTOR_CHECK(sheet.gold == 50);
    MOTOR_CHECK(contains(sheet.featIds, "feat_contactos_nobles"));
    MOTOR_CHECK(contains(sheet.knownSkillIds, "skill_etiqueta"));
}

MOTOR_TEST_CASE(populate_no_sheet_is_noop) {
    auto cls = makeWarrior();
    auto race = makeHuman();
    auto rules = makeTierRules();

    Player player({0, 0}, nullptr);
    std::array<int, 4> baseStats = {4, 3, 2, 3};
    player.populate_from_class_race_background(cls, race, nullptr, "Ghost", baseStats, rules);

    MOTOR_CHECK(player.health() == 100);
    MOTOR_CHECK(player.tier() == 1);
}

MOTOR_TEST_CASE(populate_syncs_legacy_stats_from_sheet) {
    auto cls = makeWarrior();
    auto race = makeDwarf();
    auto rules = makeTierRules();

    RPG::CharacterSheet sheet;
    Player player({0, 0}, nullptr);
    player.set_character_sheet(&sheet);

    std::array<int, 4> baseStats = {5, 2, 1, 4};
    player.populate_from_class_race_background(cls, race, nullptr, "Brond", baseStats, rules);

    MOTOR_CHECK(player.stat(RPG::Stat::CON) == 7);
    MOTOR_CHECK(player.stat(RPG::Stat::DES) == 2);
    MOTOR_CHECK(player.stat(RPG::Stat::INT) == 1);
    MOTOR_CHECK(player.stat(RPG::Stat::CAR) == 4);
}

int main() { return motor_test::runAllTests(); }
