// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   D-05  (fase D, 3 j) - Objectes, equipament i consumibles
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// D-05 (meitat motor) — InventoryEngine aplica cada bonificador una sola
// vegada i distingeix l'escut de qualsevol altre objecte a la ma esquerra.
//
// Alcance: nomes el motor (src/RPG/InventoryEngine.cpp). Fora d'aquesta
// entrega, a proposit: els loaders d'RpgCatalogs.h (zona viva de zcode,
// D-04), el trasllat DEC-05 de test_objects.json/masmorra (zona codex) i
// l'acumulador de maxHealthBonus (necessita camp a CharacterSheet +
// decisio de balanc: territori D-05-complet/G-12).
//
// Unit y no demo: sense assets, sense cwd y sense GL — les definicions es
// construeixen a ma y el dau no hi intervé.

#include "TestHarness.h"

#include "RPG/InventoryEngine.h"

#include <array>
#include <string>
#include <utility>

namespace {

RPG::ClassDefinition makeClass() {
    RPG::ClassDefinition clazz;
    clazz.id = "guerrera";
    // Compatibilitats y pesos buits: els checks passen sempre (rank 0 =
    // indeterminat; simbols buits a l'equip = compatible). Aixo no es el
    // que es prova aqui.
    return clazz;
}

RPG::EquipmentDefinition makeSword(const std::string& id, bool twoHanded) {
    RPG::EquipmentDefinition def;
    def.id = id;
    def.slot = twoHanded ? "TwoHanded" : "MainHand";
    def.twoHanded = twoHanded;
    def.rarity = RPG::ItemRarity::Common;
    def.statBonuses = {0, 2, 0, 0};
    def.caBonus = 3;
    return def;
}

}  // namespace

// Criteri d'acceptacio del paquet: equipar una arma a dues mans dona el
// mateix bonus que una d'una ma del mateix valor, no el doble — ni en
// l'equip incremental ni despres d'un REBUILD.
MOTOR_TEST_CASE(una_arma_a_dues_mans_no_duplica_el_bonus) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    cat.insert(makeSword("espasa_1m", false));
    cat.insert(makeSword("espasa_2m", true));

    RPG::CharacterSheet oneHand;
    RPG::InventoryEngine::add_to_inventory(oneHand, "espasa_1m", 1, false);
    MOTOR_CHECK(RPG::InventoryEngine::equip(oneHand, *cat.find("espasa_1m"), clazz, rules, cat).isOk());

    RPG::CharacterSheet twoHands;
    RPG::InventoryEngine::add_to_inventory(twoHands, "espasa_2m", 1, false);
    MOTOR_CHECK(RPG::InventoryEngine::equip(twoHands, *cat.find("espasa_2m"), clazz, rules, cat).isOk());

    // La de dues mans ocupa les dues mans amb el mateix id...
    const size_t mh = static_cast<size_t>(RPG::EquipSlot::MainHand);
    const size_t oh = static_cast<size_t>(RPG::EquipSlot::OffHand);
    MOTOR_CHECK(twoHands.equippedIds[mh] == "espasa_2m");
    MOTOR_CHECK(twoHands.equippedIds[oh] == "espasa_2m");
    // ...pero el bonus es el mateix que el d'una ma.
    MOTOR_CHECK(twoHands.equipBonuses == oneHand.equipBonuses);
    MOTOR_CHECK(twoHands.equipArmorBonus == oneHand.equipArmorBonus);
    MOTOR_CHECK(twoHands.equipShieldBonus == oneHand.equipShieldBonus);
    MOTOR_CHECK((twoHands.equipBonuses == std::array<int, 4>{0, 2, 0, 0}));
    MOTOR_CHECK(twoHands.equipArmorBonus == 3);
    MOTOR_CHECK(twoHands.equipShieldBonus == 0);

    // ...i un REBUILD no ho canvia (abans ho duplicava).
    RPG::InventoryEngine::recalc_equipment_bonuses(twoHands, cat, RPG::InventoryEngine::RecalcMode::REBUILD);
    MOTOR_CHECK((twoHands.equipBonuses == std::array<int, 4>{0, 2, 0, 0}));
    MOTOR_CHECK(twoHands.equipArmorBonus == 3);
    MOTOR_CHECK(twoHands.equipShieldBonus == 0);
}

// Desequipar la de dues mans per qualsevol de les dues mans ho neteja tot
// restant el bonus UNA sola vegada.
MOTOR_TEST_CASE(desequipar_dues_mans_resta_el_bonus_una_vegada) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    cat.insert(makeSword("espasa_2m", true));

    RPG::CharacterSheet sheet;
    RPG::InventoryEngine::add_to_inventory(sheet, "espasa_2m", 1, false);
    MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find("espasa_2m"), clazz, rules, cat).isOk());

    std::string out;
    MOTOR_CHECK(RPG::InventoryEngine::unequip(sheet, RPG::EquipSlot::OffHand, cat, &out).isOk());
    MOTOR_CHECK(out == "espasa_2m");
    const size_t mh = static_cast<size_t>(RPG::EquipSlot::MainHand);
    const size_t oh = static_cast<size_t>(RPG::EquipSlot::OffHand);
    MOTOR_CHECK(sheet.equippedIds[mh].empty());
    MOTOR_CHECK(sheet.equippedIds[oh].empty());
    MOTOR_CHECK((sheet.equipBonuses == std::array<int, 4>{0, 0, 0, 0}));
    MOTOR_CHECK(sheet.equipArmorBonus == 0);
    MOTOR_CHECK(sheet.equipShieldBonus == 0);
}

// Una daga a la ma esquerra NO es un escut: el seu caBonus va a armor.
// (Abans tot slot OffHand anava a equipShieldBonus.)
MOTOR_TEST_CASE(un_objecte_a_la_ma_esquerra_no_compta_com_a_escut) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    RPG::EquipmentDefinition daga;
    daga.id = "daga_merce";
    daga.slot = "OffHand";
    daga.rarity = RPG::ItemRarity::Common;
    daga.caBonus = 2;
    cat.insert(daga);

    RPG::CharacterSheet sheet;
    RPG::InventoryEngine::add_to_inventory(sheet, "daga_merce", 1, false);
    MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find("daga_merce"), clazz, rules, cat).isOk());

    MOTOR_CHECK(sheet.equipShieldBonus == 0);
    MOTOR_CHECK(sheet.equipArmorBonus == 2);
}

// ...pero un escut de veritat continua anant a shield.
MOTOR_TEST_CASE(un_escut_de_veritat_continua_anant_a_shield) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    RPG::EquipmentDefinition escut;
    escut.id = "escudo_de_madera";
    escut.slot = "Shield";
    escut.rarity = RPG::ItemRarity::Common;
    escut.caBonus = 2;
    cat.insert(escut);

    RPG::CharacterSheet sheet;
    RPG::InventoryEngine::add_to_inventory(sheet, "escudo_de_madera", 1, false);
    MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find("escudo_de_madera"), clazz, rules, cat).isOk());

    MOTOR_CHECK(sheet.equipShieldBonus == 2);
    MOTOR_CHECK(sheet.equipArmorBonus == 0);
}

// D-05 — maxHealthBonus ja no es descarta: s'acumula a l'equipar, el
// REBUILD el conserva y el unequip el retira. (Consumir-lo al healthCap
// es G-12: avui cap asset el porta, asi que aixo no canvia cap numero.)
MOTOR_TEST_CASE(el_bonus_de_vida_de_l_equip_s_acumula_i_es_retira) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    RPG::EquipmentDefinition anell;
    anell.id = "anillo_vitalidad";
    anell.slot = "Accessory";
    anell.rarity = RPG::ItemRarity::Common;
    anell.maxHealthBonus = 5;
    cat.insert(anell);

    RPG::CharacterSheet sheet;
    sheet.tier = 2;  // PART-02: los anillos se abren en el Tier 2
    MOTOR_CHECK(sheet.equipMaxHealthBonus == 0);
    RPG::InventoryEngine::add_to_inventory(sheet, "anillo_vitalidad", 1, false);
    MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find("anillo_vitalidad"), clazz, rules, cat).isOk());
    MOTOR_CHECK(sheet.equipMaxHealthBonus == 5);

    RPG::InventoryEngine::recalc_equipment_bonuses(sheet, cat, RPG::InventoryEngine::RecalcMode::REBUILD);
    MOTOR_CHECK(sheet.equipMaxHealthBonus == 5);

    MOTOR_CHECK(RPG::InventoryEngine::unequip(sheet, RPG::EquipSlot::Accessory, cat).isOk());
    MOTOR_CHECK(sheet.equipMaxHealthBonus == 0);
}

// ...y en una arma a dues mans es compta un sol cop, com la resta.
MOTOR_TEST_CASE(el_bonus_de_vida_a_dues_mans_no_es_duplica) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    RPG::EquipmentDefinition def = makeSword("espasa_vital_2m", true);
    def.maxHealthBonus = 4;
    cat.insert(def);

    RPG::CharacterSheet sheet;
    RPG::InventoryEngine::add_to_inventory(sheet, "espasa_vital_2m", 1, false);
    MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find("espasa_vital_2m"), clazz, rules, cat).isOk());
    MOTOR_CHECK(sheet.equipMaxHealthBonus == 4);

    RPG::InventoryEngine::recalc_equipment_bonuses(sheet, cat, RPG::InventoryEngine::RecalcMode::REBUILD);
    MOTOR_CHECK(sheet.equipMaxHealthBonus == 4);
}

// D-05 — can_equip exigeix els `requiredStats` de la carta: sense DES 2
// no hi ha arc; amb DES 2, sí. El missatge diu stat, valor y mínimo.
MOTOR_TEST_CASE(els_requisits_de_la_carta_es_compleixen_o_no_s_equipa) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    RPG::EquipmentDefinition arco = makeSword("arco_corto", false);
    arco.slot = "MainHand";
    arco.requiredStats = {0, 2, 0, 0};
    cat.insert(arco);

    RPG::CharacterSheet fluix;
    RPG::InventoryEngine::add_to_inventory(fluix, "arco_corto", 1, false);
    MOTOR_CHECK(!RPG::InventoryEngine::equip(fluix, *cat.find("arco_corto"), clazz, rules, cat).isOk());
    MOTOR_CHECK(fluix.equippedIds[static_cast<size_t>(RPG::EquipSlot::MainHand)].empty());

    RPG::CharacterSheet apte;
    apte.baseStats = {0, 2, 0, 0};
    RPG::InventoryEngine::add_to_inventory(apte, "arco_corto", 1, false);
    MOTOR_CHECK(RPG::InventoryEngine::equip(apte, *cat.find("arco_corto"), clazz, rules, cat).isOk());
    MOTOR_CHECK(apte.equippedIds[static_cast<size_t>(RPG::EquipSlot::MainHand)] == "arco_corto");
}

// NIVELES-01 — Una pieza con requiredLevel no se lleva puesta antes de ese
// nivel (en la mochila si); al llegar, si. El motivo dice el nivel.
MOTOR_TEST_CASE(una_pieza_con_nivel_espera_a_que_el_heroe_llegue) {
    RPG::TierRules rules;
    RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    RPG::EquipmentDefinition espada = makeSword("espada_de_alaro", false);
    espada.requiredLevel = 8;
    cat.insert(espada);

    RPG::CharacterSheet novata;
    novata.nivel = 6;
    const auto pronto = RPG::InventoryEngine::can_equip(novata, *cat.find("espada_de_alaro"), clazz, rules);
    MOTOR_CHECK(!pronto.isOk() && pronto.errorMessage() == "necesitas nivel 8 (tienes el 6)");
    RPG::InventoryEngine::add_to_inventory(novata, "espada_de_alaro", 1, false);
    MOTOR_CHECK(!RPG::InventoryEngine::equip(novata, *cat.find("espada_de_alaro"), clazz, rules, cat).isOk());
    MOTOR_CHECK(novata.equippedIds[static_cast<size_t>(RPG::EquipSlot::MainHand)].empty());

    novata.nivel = 8;
    MOTOR_CHECK(RPG::InventoryEngine::equip(novata, *cat.find("espada_de_alaro"), clazz, rules, cat).isOk());
    MOTOR_CHECK(novata.equippedIds[static_cast<size_t>(RPG::EquipSlot::MainHand)] == "espada_de_alaro");
}

// NIVELES-01 — El catalogo lleva el nivel de cada pieza (tools/niveles_equipo.py):
// el equipo de Tier 1 se reparte entre los niveles 1, 5 y 10, el de Tier 2
// empieza en el 20, el de Tier 3 en el 40 y el de Tier 4 en el 60.
MOTOR_TEST_CASE(el_catalogo_reparte_el_equipo_por_niveles) {
    RPG::Catalogs::EquipmentCatalog cat;
    MOTOR_CHECK(cat.loadFromFile("assets/catalogs/equipment.json").isOk());
    int tier1alto = 0;
    bool todos = true;
    cat.forEach([&](const RPG::EquipmentDefinition& e) {
        todos &= e.requiredLevel >= 1;
        if (e.tier == 1 && e.requiredLevel > 1) ++tier1alto;
        if (e.tier == 3) todos &= e.requiredLevel >= 40;
        if (e.tier == 4) todos &= e.requiredLevel >= 60;
    });
    MOTOR_CHECK(todos);
    MOTOR_CHECK(tier1alto > 30);
}

// PART-02 (Oriol, 07/10): «tier 1 que solo es casco, pechera, brazos, piernas, zapatos y armas, no tiene nada
// de anillos ni todas esas cosas que se desbloquean en tier 2 [...] donde van collares, cinturon y anillos».
namespace {
RPG::EquipmentDefinition makeAbalorio(const std::string& id, const std::string& slot) {
    RPG::EquipmentDefinition def;
    def.id = id;
    def.slot = slot;
    def.rarity = RPG::ItemRarity::Common;
    def.statBonuses = {1, 0, 0, 0};
    return def;
}
}  // namespace

MOTOR_TEST_CASE(las_ranuras_nuevas_y_su_nombre) {
    bool dos = false;
    using RPG::EquipSlot;
    using RPG::InventoryEngine;
    MOTOR_CHECK(InventoryEngine::resolve_slot("Arms", dos) == EquipSlot::Arms && !dos);
    MOTOR_CHECK(InventoryEngine::resolve_slot("brazos", dos) == EquipSlot::Arms);
    MOTOR_CHECK(InventoryEngine::resolve_slot("Neck", dos) == EquipSlot::Neck);
    MOTOR_CHECK(InventoryEngine::resolve_slot("collar", dos) == EquipSlot::Neck);
    MOTOR_CHECK(InventoryEngine::resolve_slot("amuleto", dos) == EquipSlot::Neck);
    MOTOR_CHECK(InventoryEngine::resolve_slot("Belt", dos) == EquipSlot::Belt);
    MOTOR_CHECK(InventoryEngine::resolve_slot("cinturon", dos) == EquipSlot::Belt);
    MOTOR_CHECK(InventoryEngine::resolve_slot("Ring", dos) == EquipSlot::Accessory);
    MOTOR_CHECK(InventoryEngine::resolve_slot("Accessory", dos) == EquipSlot::Accessory);
    MOTOR_CHECK(InventoryEngine::resolve_slot("Ring2", dos) == EquipSlot::Ring2);
    MOTOR_CHECK(InventoryEngine::resolve_slot("Head", dos) == EquipSlot::Head);  // las de antes, igual
    MOTOR_CHECK(InventoryEngine::resolve_slot("Feet", dos) == EquipSlot::Feet);
    for (std::size_t i = 0; i < RPG::kRanurasDeEquipo; ++i) {
        const auto ranura = static_cast<EquipSlot>(i);
        MOTOR_CHECK(InventoryEngine::slot_name(ranura) != "Unknown");
        MOTOR_CHECK(InventoryEngine::resolve_slot(InventoryEngine::slot_name(ranura), dos) == ranura);
    }
    // el Tier 1: casco, pechera, brazos, piernas, zapatos y armas; el 2 abre el resto
    for (const EquipSlot r : {EquipSlot::Head, EquipSlot::Torso, EquipSlot::Arms, EquipSlot::Legs, EquipSlot::Feet,
                              EquipSlot::MainHand, EquipSlot::OffHand})
        MOTOR_CHECK(RPG::tierDeRanura(r) == 1);
    for (const EquipSlot r : {EquipSlot::Neck, EquipSlot::Belt, EquipSlot::Accessory, EquipSlot::Ring2})
        MOTOR_CHECK(RPG::tierDeRanura(r) == 2);
    MOTOR_CHECK(RPG::CharacterSheet{}.equippedIds.size() == 11);
}

MOTOR_TEST_CASE(en_el_tier_1_no_hay_anillos_collar_ni_cinturon) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    for (const auto& [id, slot] : {std::pair{"anillo", "Ring"}, std::pair{"collar", "Neck"},
                                   std::pair{"cinturon", "Belt"}, std::pair{"brazales", "Arms"}})
        cat.insert(makeAbalorio(id, slot));
    RPG::CharacterSheet sheet;
    sheet.tier = 1;
    for (const char* id : {"anillo", "collar", "cinturon"}) {
        RPG::InventoryEngine::add_to_inventory(sheet, id, 1, false);
        const auto r = RPG::InventoryEngine::equip(sheet, *cat.find(id), clazz, rules, cat);
        MOTOR_CHECK(!r.isOk() && r.errorMessage().find("Tier 2") != std::string::npos);
    }
    for (const std::string& puesto : sheet.equippedIds)
        MOTOR_CHECK(puesto.empty());  // nada puesto, todo sigue en la mochila
    MOTOR_CHECK(sheet.inventoryEquipment.size() == 3);
    // los brazos, si
    RPG::InventoryEngine::add_to_inventory(sheet, "brazales", 1, false);
    MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find("brazales"), clazz, rules, cat).isOk());
    MOTOR_CHECK(sheet.equippedIds[static_cast<size_t>(RPG::EquipSlot::Arms)] == "brazales");

    // En el Tier 2, todo: cada cosa en su ranura.
    sheet.tier = 2;
    for (const char* id : {"anillo", "collar", "cinturon"})
        MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find(id), clazz, rules, cat).isOk());
    MOTOR_CHECK(sheet.equippedIds[static_cast<size_t>(RPG::EquipSlot::Accessory)] == "anillo");
    MOTOR_CHECK(sheet.equippedIds[static_cast<size_t>(RPG::EquipSlot::Neck)] == "collar");
    MOTOR_CHECK(sheet.equippedIds[static_cast<size_t>(RPG::EquipSlot::Belt)] == "cinturon");
    MOTOR_CHECK(sheet.equipBonuses[0] == 4);  // los cuatro suman su bonus una vez
}

MOTOR_TEST_CASE(dos_anillos_uno_en_cada_mano) {
    RPG::TierRules rules;
    const RPG::ClassDefinition clazz = makeClass();
    RPG::Catalogs::EquipmentCatalog cat;
    for (const char* id : {"anillo_de_fuego", "anillo_de_hielo", "anillo_de_rayo"})
        cat.insert(makeAbalorio(id, "Ring"));
    RPG::CharacterSheet sheet;
    sheet.tier = 2;
    for (const char* id : {"anillo_de_fuego", "anillo_de_hielo", "anillo_de_rayo"})
        RPG::InventoryEngine::add_to_inventory(sheet, id, 1, false);
    const auto anillo = [&](RPG::EquipSlot s) { return sheet.equippedIds[static_cast<size_t>(s)]; };
    MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find("anillo_de_fuego"), clazz, rules, cat).isOk());
    MOTOR_CHECK(anillo(RPG::EquipSlot::Accessory) == "anillo_de_fuego");
    // el segundo, a la otra mano
    MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find("anillo_de_hielo"), clazz, rules, cat).isOk());
    MOTOR_CHECK(anillo(RPG::EquipSlot::Accessory) == "anillo_de_fuego" && anillo(RPG::EquipSlot::Ring2) == "anillo_de_hielo");
    MOTOR_CHECK(sheet.equipBonuses[0] == 2);
    // con las dos llenas, el tercero cambia el de la mano principal (que vuelve a la mochila)
    MOTOR_CHECK(RPG::InventoryEngine::equip(sheet, *cat.find("anillo_de_rayo"), clazz, rules, cat).isOk());
    MOTOR_CHECK(anillo(RPG::EquipSlot::Accessory) == "anillo_de_rayo" && anillo(RPG::EquipSlot::Ring2) == "anillo_de_hielo");
    MOTOR_CHECK(sheet.equipBonuses[0] == 2);
    bool vuelto = false;
    for (const auto& [id, n] : sheet.inventoryEquipment)
        vuelto = vuelto || (id == "anillo_de_fuego" && n == 1);
    MOTOR_CHECK(vuelto);
    // y se quita cada uno de su mano
    MOTOR_CHECK(RPG::InventoryEngine::unequip(sheet, RPG::EquipSlot::Ring2, cat).isOk());
    MOTOR_CHECK(anillo(RPG::EquipSlot::Ring2).empty() && sheet.equipBonuses[0] == 1);
}

int main() { return motor_test::runAllTests(); }
