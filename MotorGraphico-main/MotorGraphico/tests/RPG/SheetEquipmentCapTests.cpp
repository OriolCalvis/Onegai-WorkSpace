// G-12b (opencode 17/09) — el CABLEADO final no llegaba a la ficha: la
// cabecera decia "GameSession llama a recompute_derived() cada vez que
// cambia [equipo]", pero GameSession no llama a nada y el equip vive en
// InventoryEngine, asi que el bonus de vida/armadura equipado se quedaba
// en los campos directos sin poblar cachedHealthCap/cachedDefenses -- lo
// que leen de verdad Player/Enemy via healthCap()/defenseValues().
//
// Esto es el bucle G-12 completo probado en la capa RPG (GL-free, sin
// assets): InventoryEngine::equip -> apply_bonus -> recompute_derived ->
// healthCap() cacheado con el bonus dentro del min() del tier
// (equipArmorBonus/2 + equipMaxHealthBonus, rodaja G-12 del 14/09).
// unequip() conserva el contrato documentado (el llamador recomputa); su
// prueba recorre ese camino.
//
// Fora de alcance (zona de otras sesiones en vuelo): GameSession,
// Application y el desenlace del equip desde la UI.

#include "TestHarness.h"

#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"
#include "RPG/InventoryEngine.h"
#include "RPG/TierRules.h"

namespace {

RPG::ClassDefinition makeClass() {
    RPG::ClassDefinition clazz;
    clazz.id = "guerrera";
    clazz.baseHealth = 2;
    return clazz;
}

RPG::EquipmentDefinition makeEquip(const std::string& id, const std::string& slot,
                                   int caBonus, int maxHealthBonus) {
    RPG::EquipmentDefinition def;
    def.id = id;
    def.slot = slot;
    def.rarity = RPG::ItemRarity::Common;
    def.caBonus = caBonus;
    def.maxHealthBonus = maxHealthBonus;
    return def;
}

// Fulla basica: con=1 (CON*3=3), car=0. Sense bonus: 2+3 = 5 de cap, CA
// 10 (10 + DES 0 + sin armadura).
RPG::CharacterSheet makeSheet() {
    RPG::CharacterSheet sheet;
    sheet.tier = 1;
    sheet.baseStats = {1, 0, 0, 0};
    return sheet;
}

bool equip(RPG::CharacterSheet& sheet, const RPG::EquipmentDefinition& def,
           const RPG::ClassDefinition& clazz, const RPG::TierRules& rules,
           const RPG::Catalogs::EquipmentCatalog& cat) {
    RPG::InventoryEngine::add_to_inventory(sheet, def.id, 1, false);
    return RPG::InventoryEngine::equip(sheet, *cat.find(def.id), clazz, rules, cat).isOk();
}

}  // namespace

// El hueco real: equipar POBLA la cache al instante. Antes de la rodaja,
// healthCap() (sin argumentos) seguia en el valor sin equip.
MOTOR_TEST_CASE(equipar_armadura_refresca_ficha_y_cap_al_instante) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::CharacterSheet sheet = makeSheet();
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 5);                      // 2 + CON*3
    MOTOR_CHECK(sheet.defenseValues().ca_fisica() == 10);

    RPG::Catalogs::EquipmentCatalog cat;
    cat.insert(makeEquip("peto", "Torso", /*ca*/ 4, /*vida*/ 10));

    MOTOR_CHECK(equip(sheet, *cat.find("peto"), clazz, rules, cat));
    MOTOR_CHECK(sheet.equipArmorBonus == 4);
    MOTOR_CHECK(sheet.equipMaxHealthBonus == 10);
    // Cache fresca: cap = 5 + (4/2) + 10; CA = 10 + 4.
    MOTOR_CHECK(sheet.healthCap() == 17);
    MOTOR_CHECK(sheet.defenseValues().ca_fisica() == 14);
}

// El inventario-flujo G-12 con acceso a la misma ranura: el segundo equipo
// reemplaza al primero y la cache refleja SOLO el nuevo (sin dobles).
MOTOR_TEST_CASE(sustituir_equipo_recalcula_una_sola_vez) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::CharacterSheet sheet = makeSheet();
    sheet.recompute_derived(rules, &clazz);

    RPG::Catalogs::EquipmentCatalog cat;
    cat.insert(makeEquip("peto_viejo", "Torso", /*ca*/ 2, /*vida*/ 0));
    cat.insert(makeEquip("peto_nuevo", "Torso", /*ca*/ 5, /*vida*/ 7));

    MOTOR_CHECK(equip(sheet, *cat.find("peto_viejo"), clazz, rules, cat));
    MOTOR_CHECK(sheet.healthCap() == 5 + 1);                   // 5 + 2/2
    MOTOR_CHECK(sheet.defenseValues().ca_fisica() == 10 + 2);

    MOTOR_CHECK(equip(sheet, *cat.find("peto_nuevo"), clazz, rules, cat));
    MOTOR_CHECK(sheet.equipArmorBonus == 5);
    MOTOR_CHECK(5 + 5 / 2 + 7 + 0 == 14);  // formula esperada documentada
    MOTOR_CHECK(sheet.healthCap() == 14);  // 5 + (5/2) + 7, sin sumar el viejo
    MOTOR_CHECK(sheet.defenseValues().ca_fisica() == 10 + 5);
}

// El tope del tier manda dentro del min() aunque el equipo pida mas: se
// puede sobrecapar con tope, nunca por encima del tier.
MOTOR_TEST_CASE(el_bonus_de_vida_equipado_no_pasa_del_tope_del_tier) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    clazz.baseHealth = 34;  // 34 + CON*3 = 37: ya por encima del cap del tier 1 (35)
    RPG::CharacterSheet sheet = makeSheet();

    RPG::Catalogs::EquipmentCatalog cat;
    // PART-02 (Oriol): en el Tier 1 no hay anillos ni colgantes; el bonus de vida, en unos brazales.
    cat.insert(makeEquip("brazales_vitales", "Arms", /*ca*/ 0, /*vida*/ 40));

    MOTOR_CHECK(equip(sheet, *cat.find("brazales_vitales"), clazz, rules, cat));
    MOTOR_CHECK(sheet.equipMaxHealthBonus == 40);
    MOTOR_CHECK(sheet.healthCap() == 35);  // cache ya con el clamp
}

// Contrato de unequip (documentado en CharacterSheet.h: el llamador
// recomputa): el campo se vacia y el recompute manual devuelve la ficha
// al cap sin equip.
MOTOR_TEST_CASE(desequipar_y_recomputar_devuelve_el_cap_base) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::CharacterSheet sheet = makeSheet();
    sheet.recompute_derived(rules, &clazz);

    RPG::Catalogs::EquipmentCatalog cat;
    cat.insert(makeEquip("brazales", "Arms", /*ca*/ 0, /*vida*/ 10));  // PART-02: en el Tier 1, brazales

    MOTOR_CHECK(equip(sheet, *cat.find("brazales"), clazz, rules, cat));
    MOTOR_CHECK(sheet.healthCap() == 15);

    RPG::InventoryEngine::unequip(sheet, RPG::EquipSlot::Arms, cat);
    MOTOR_CHECK(sheet.equipMaxHealthBonus == 0);
    sheet.recompute_derived(rules, &clazz);
    MOTOR_CHECK(sheet.healthCap() == 5);
}

// A dos manos el id compartido entre MainHand+OffHand se cuenta UNA vez en
// la cache (misma regla que el REBUILD de D-05).
MOTOR_TEST_CASE(un_arma_a_dos_manos_no_dobla_su_bonus_en_la_cache) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::CharacterSheet sheet = makeSheet();
    sheet.recompute_derived(rules, &clazz);

    RPG::EquipmentDefinition espada = makeEquip("mandoble", "TwoHanded", /*ca*/ 3, /*vida*/ 6);
    espada.twoHanded = true;
    RPG::Catalogs::EquipmentCatalog cat;
    cat.insert(espada);

    MOTOR_CHECK(equip(sheet, *cat.find("mandoble"), clazz, rules, cat));
    MOTOR_CHECK(5 + 3 / 2 + 6 == 12);  // formula esperada documentada
    MOTOR_CHECK(sheet.healthCap() == 12);  // 5 + (3/2) + 6, no (3/2)*2 + 12
}

int main() { return motor_test::runAllTests(); }