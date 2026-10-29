// G-12 (rodaja de calcul, opencode 14/09) — el bonus de vida de l'equip
// (equipMaxHealthBonus, acumulat per InventoryEngine des de D-05) arriba al
// healthCap(). Abans nomes hi entrava l'aproximacio equipArmorBonus/2.
//
// Alcance: nomes la formula de CharacterSheet::healthCap() (que delega en
// TierRules::compute_cap_health). Fora: Application/GameSession (cablejat
// G-12 en vol d'una altra sessio), InventoryEngine.cpp i la logica de la
// capa. El tope del tier es manté dins del min(), com ja faisia el terme
// d'armadura: aquesta rodaja NO canvia la semantica de caps.
//
// Unit i no demo: sense assets, sense cwd i sense GL — les definicions es
// construeixen a ma i el dau no hi interve.

#include "TestHarness.h"

#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/TierRules.h"

namespace {

RPG::ClassDefinition makeClass() {
    RPG::ClassDefinition clazz;
    clazz.id = "guerrera";
    clazz.baseHealth = 2;
    return clazz;
}

// Fulla basica: con=1 (CON*3=3), car=0. Sense bonus: 2+3+0 = 5. Cap de
// tier (default 27) lluny del tot perque les sumes siguin exactes.
RPG::CharacterSheet makeSheet() {
    RPG::CharacterSheet sheet;
    sheet.tier = 1;
    sheet.baseStats = {1, 0, 0, 0};
    return sheet;
}

}  // namespace

// Ho prova de D-05/G-12: equipar un objecte amb maxHealthBonus puja el cap
// en eixe mateix valor, i el terme d'armadura (meitat) continua comptant.
MOTOR_TEST_CASE(bonus_de_vida_de_equip_suma_al_cap) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::CharacterSheet sheet = makeSheet();

    sheet.recompute_derived(rules, &clazz);
    const int base = sheet.healthCap();  // 5
    MOTOR_CHECK(base == 5);

    sheet.equipMaxHealthBonus = 10;
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == base + 10);

    // L'aproximacio d'armadura (meitat de l'armadura) cohabita amb el terme
    // explicit: 4 d'armadura -> +2.
    sheet.equipArmorBonus = 4;
    sheet.equipMaxHealthBonus = 0;
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == base + 2);

    sheet.equipArmorBonus = 4;
    sheet.equipMaxHealthBonus = 10;
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == base + 2 + 10);
}

// Desequipar (el camp torna a 0, com fa InventoryEngine::unequip) retira el
// terme: el cap torna al valor d'abans.
MOTOR_TEST_CASE(desequipar_retira_el_bonus_del_cap) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::CharacterSheet sheet = makeSheet();

    sheet.equipMaxHealthBonus = 10;
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 15);

    sheet.equipMaxHealthBonus = 0;
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 5);
}

// El recompute es idempotent i l'equip incremental (10 -> 13) suma nomes la
// diferència: cap de 15 -> 18, no 15 + 10 + 13.
MOTOR_TEST_CASE(recompute_repetit_no_duplica_el_bonus) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::CharacterSheet sheet = makeSheet();

    sheet.equipMaxHealthBonus = 10;
    sheet.recompute_derived(rules, &clazz);
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 15);

    sheet.equipMaxHealthBonus = 13;
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 15 + 3);

    // I una correccio a la baixa es reflecteix tambe.
    sheet.equipMaxHealthBonus = 5;
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 10);
}

// El tope del tier mana amb el bonus a dins del min(): la vida no pot
// superar healthCap(tier) per molt equip que porti.
MOTOR_TEST_CASE(el_tope_del_tier_clampa_junt_amb_el_bonus) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    clazz.baseHealth = 34;  // 34 + CON*3 = 37: per si sol ja supera el cap del tier 1 (35)
    RPG::CharacterSheet sheet = makeSheet();

    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 35);

    sheet.equipMaxHealthBonus = 40;
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 35);
}

// El camí de lectura canonical (recompute_derived -> healthCap()) i el de
// formula directa coincideixen amb el bonus posat.
MOTOR_TEST_CASE(cache_i_formula_directa_coincideixen_amb_bonus) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::CharacterSheet sheet = makeSheet();

    sheet.equipMaxHealthBonus = 7;
    MOTOR_CHECK(sheet.healthCap(rules, &clazz) == 12);
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 12);
}

int main() { return motor_test::runAllTests(); }