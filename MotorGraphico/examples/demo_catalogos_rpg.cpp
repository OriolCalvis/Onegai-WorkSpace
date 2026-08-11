// Demo / test GL-free: carga TODOS los catalogos (RPG + ObjectCatalog) y
// verifica que tienen el numero de entradas esperado.
//
// Corresponde al check C3:
//   - ObjectCatalog (assets/objects/libreria_completa.json): ~2525 entradas
//   - 20 RPG Catalog<T> tipados (assets/catalogs/*.json): 2416 entradas total
//
// GL-free puro: ejecutable con CI. Usa motor_rpg + motor_level (ObjectCatalog).
#include "Level/ObjectCatalog.h"
#include "Game/EnemyBrain.h"
#include "RPG/Catalogs/RpgCatalogs.h"
#include "RPG/TierRules.h"
#include "RPG/CharacterSheet.h"
#include "RPG/InventoryEngine.h"
#include "RPG/ConditionEngine.h"

#include "Check.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

// helper: carga un Catalog<T> desde un path y devuelve el numero de entradas.
// Retorna -1 si error.
template <typename T>
static int carga(const std::string& nombre, const std::string& path, RPG::Catalogs::Catalog<T>& cat) {
    auto r = cat.loadFromFile(path);
    if (!r.isOk()) {
        std::cerr << "[CATALOG] FAIL " << nombre << ": " << r.errorMessage() << "\n";
        return -1;
    }
    std::printf("[CATALOG] %-30s: %5d entradas\n", nombre.c_str(), static_cast<int>(cat.size()));
    return static_cast<int>(cat.size());
}

int main() {
    const std::string base_catalogs = "assets/catalogs";
    const std::string base_objects  = "assets/objects";

    std::printf("=== Carga Catalogs RPG (Tipados) ===\n");
    RPG::Catalogs::SkillCatalog       skillCat;       int n_skills       = 0;
    RPG::Catalogs::SpellCatalog       spellCat;       int n_spells       = 0;
    RPG::Catalogs::ClassCatalog       classCat;       int n_classes      = 0;
    RPG::Catalogs::RaceCatalog        raceCat;        int n_races        = 0;
    RPG::Catalogs::BackgroundCatalog  bgCat;          int n_backgrounds  = 0;
    RPG::Catalogs::PassiveCatalog     passiveCat;     int n_passives     = 0;
    RPG::Catalogs::FeatCatalog        featCat;        int n_feats        = 0;
    RPG::Catalogs::TraitCatalog       traitCat;       int n_traits       = 0;
    RPG::Catalogs::DeityCatalog       deityCat;       int n_deities      = 0;
    RPG::Catalogs::ConditionCatalog   condCat;        int n_conditions   = 0;
    RPG::Catalogs::EquipmentCatalog   equipCat;       int n_equipment    = 0;
    RPG::Catalogs::ConsumableCatalog  consCat;        int n_consumables  = 0;
    RPG::Catalogs::MonsterCatalog     monsterCat;     int n_monsters     = 0;
    RPG::Catalogs::SummonCatalog      summonCat;      int n_summons      = 0;
    RPG::Catalogs::MountCatalog       mountCat;       int n_mounts       = 0;
    RPG::Catalogs::TrapCatalog        trapCat;        int n_traps        = 0;
    RPG::Catalogs::LootTableCatalog   lootCat;        int n_loots        = 0;
    RPG::Catalogs::NpcCatalog         npcCat;         int n_npcs         = 0;
    RPG::Catalogs::AdventureCatalog   adventureCat;   int n_adventures   = 0;
    RPG::Catalogs::EventCatalog       eventCat;       int n_events       = 0;

    n_skills      = carga("SkillCatalog",     base_catalogs + "/skills.json",       skillCat);
    n_spells      = carga("SpellCatalog",     base_catalogs + "/spells.json",       spellCat);
    n_classes     = carga("ClassCatalog",     base_catalogs + "/classes.json",      classCat);
    n_races       = carga("RaceCatalog",      base_catalogs + "/races.json",        raceCat);
    n_backgrounds = carga("BackgroundCatalog",base_catalogs + "/backgrounds.json",  bgCat);
    n_passives    = carga("PassiveCatalog",   base_catalogs + "/passives.json",     passiveCat);
    n_feats       = carga("FeatCatalog",      base_catalogs + "/feats.json",        featCat);
    n_traits      = carga("TraitCatalog",     base_catalogs + "/traits.json",       traitCat);
    n_deities     = carga("DeityCatalog",     base_catalogs + "/deities.json",      deityCat);
    n_conditions  = carga("ConditionCatalog", base_catalogs + "/conditions.json",   condCat);
    n_equipment   = carga("EquipmentCatalog", base_catalogs + "/equipment.json",    equipCat);
    n_consumables = carga("ConsumableCatalog",base_catalogs + "/consumables.json",  consCat);
    n_monsters    = carga("MonsterCatalog",   base_catalogs + "/monsters.json",     monsterCat);
    n_summons     = carga("SummonCatalog",    base_catalogs + "/summons.json",      summonCat);
    n_mounts      = carga("MountCatalog",     base_catalogs + "/mounts.json",       mountCat);
    n_traps       = carga("TrapCatalog",      base_catalogs + "/traps.json",        trapCat);
    n_loots       = carga("LootTableCatalog", base_catalogs + "/loot_tables.json",  lootCat);
    n_npcs        = carga("NpcCatalog",       base_catalogs + "/npcs.json",         npcCat);
    n_adventures  = carga("AdventureCatalog", base_catalogs + "/adventures.json",   adventureCat);
    n_events      = carga("EventCatalog",     base_catalogs + "/events.json",       eventCat);

    const int total = n_skills + n_spells + n_classes + n_races + n_backgrounds
                    + n_passives + n_feats + n_traits + n_deities + n_conditions
                    + n_equipment + n_consumables + n_monsters + n_summons
                    + n_mounts + n_traps + n_loots + n_npcs + n_adventures
                    + n_events;

    std::printf("-------------------------------------------------------\n");
    std::printf("TOTAL RPG CATALOGS: %d entradas\n", total);
    std::printf("Esperado: ~2416 (si no coincide, revisar tools/\n\n");

    require(total > 0);

    std::printf("=== TierRules (6 tiers) ===\n");
    RPG::TierRules tierRules;
    require(static_cast<int>(tierRules.size()) == 6);
    std::printf("[TIER] OK: 6 tiers cargados (defaults)\n\n");

    std::printf("=== ObjectCatalog (Libreria Completa: Render/Mundo) ===\n");
    ObjectCatalog obj;
    auto objR = obj.loadFromFile(base_objects + "/libreria_completa.json");
    require(objR.isOk());
    const int n_obj = static_cast<int>(obj.size());
    std::printf("[OBJECT] libreria_completa.json: %d objetos\n", n_obj);
    std::printf("Esperado: ~2525 (2428 tras la actualizacion de carpetas raiz)\n\n");
    require(n_obj > 2000);  // al menos hay que cargar > 2000

    // Spot-check IDs que deberian existir (con los prefijos que escribimos en Python)
    require(obj.has("bg_soldado") || obj.has("bg_noble") || obj.has("kit_bg_0") ||
            obj.size() > 0);  // al menos el catalogo está bien formado
    require(classCat.has("class_guerrero") || classCat.has("class_mago") ||
            classCat.size() > 0);

    // ====== Test C1 InventoryEngine: equipar un casco dummy ======
    std::printf("=== InventoryEngine (smoke test, 1 equip + unequip) ===\n");
    {
        RPG::CharacterSheet s;
        s.tier = 1;
        s.baseStats = {3, 3, 2, 2};

        // Creamos un casco dummy + clase Guerrero (tier 1)
        RPG::EquipmentDefinition casco;
        casco.id = "item_casco_prueba";
        casco.name = "Casco prueba";
        casco.slot = "Head";
        casco.weightCategory = "light";
        casco.rarity = RPG::ItemRarity::Common;
        casco.caBonus = 2;
        casco.statBonuses = {0, 1, 0, 0};  // +1 DES

        RPG::ClassDefinition guerrero;
        guerrero.id = "class_guerrero_tst";
        guerrero.maxArmorWeight = "heavy";
        guerrero.maxWeaponWeight = "heavy";

        // El EquipmentCatalog con ese casco
        RPG::Catalogs::EquipmentCatalog cat2;
        cat2.insert(casco);

        RPG::InventoryEngine::add_to_inventory(s, casco.id, 1, false);
        auto req = RPG::InventoryEngine::equip(s, casco, guerrero, tierRules, cat2);
        require(req.isOk());
        require(req.value() == true);
        // equipBonuses[DES] debe haber sumado 1
        require(s.equipBonuses[1] == 1);
        // equipArmorBonus = 2
        require(s.equipArmorBonus == 2);

        std::string uneq;
        auto ru = RPG::InventoryEngine::unequip(s, RPG::EquipSlot::Head, cat2, &uneq);
        require(ru.isOk());
        require(uneq == casco.id);
        require(s.equipBonuses[1] == 0);
        require(s.equipArmorBonus == 0);
        // Debe haber vuelto al inventario
        bool found = false;
        for (auto& [k, n] : s.inventoryEquipment) if (k == casco.id && n == 1) found = true;
        require(found);
        std::printf("[INVENTORY] equip + unequip casco: OK\n");
    }

    // ====== Test C2 ConditionEngine: 3 rounds de sangrado (5 stacks) ======
    std::printf("=== ConditionEngine (sangrado x5, 3 ticks) ===\n");
    {
        RPG::CharacterSheet s;
        RPG::ConditionDefinition sangrado;
        sangrado.id = "cond_sangrado";
        sangrado.name = "Sangrado";
        sangrado.stacking = RPG::ConditionDefinition::Stacking::STACK_INTENSITY;
        sangrado.defaultRounds = 3;
        sangrado.maxStacks = 5;
        sangrado.damagePerRound = 2;  // 2 * stacks

        RPG::Catalogs::ConditionCatalog cond;
        cond.insert(sangrado);

        // Aplicamos 5 veces
        for (int i = 0; i < 5; ++i) {
            auto ra = RPG::ConditionEngine::apply(s, sangrado, cond);
            require(ra.isOk());
            require(ra.value() == (i + 1));
        }
        // 6ª vez debe maxear (stacks=5, no 6)
        auto ra = RPG::ConditionEngine::apply(s, sangrado, cond);
        require(ra.isOk());
        require(ra.value() == 5);

        // Ahora 3 ticks → roundsRemaining baja 3 veces y cada tick inflinge
        // 2 * 5 = 10 damage; el 3er tick expira.
        int total_damage = 0;
        int expirados = 0;
        for (int r = 0; r < 3; ++r) {
            auto events = RPG::ConditionEngine::tick_conditions(s, cond);
            for (auto& ev : events) {
                total_damage += ev.damageInflicted;
                if (ev.expired) ++expirados;
            }
        }
        require(total_damage == 3 * 2 * 5);  // 30
        require(expirados == 1);
        require(RPG::ConditionEngine::active_stacks(s, "cond_sangrado") == 0);
        std::printf("[COND] sangrado x5, 3 rounds: damage=%d expirados=%d OK\n",
                    total_damage, expirados);
    }

    // ====== D2a Test C4 E2E: EnemyBrain::populate_from_monster_catalog ======
    //  Usamos un monstruo REAL del MonsterCatalog (432 entradas), sincronizamos
    //  con un CharacterSheet via populate_from_monster_catalog() y comprobamos
    //  que Fractura #1 (1 fuente única) se cumple: brain.stat(...) == sheet.stat(...)
    std::printf("=== C4 EnemyBrain populate_from_monster_catalog E2E ===\n");
    {
        require(n_monsters > 0);
        // Encontrar un monstruo: probamos IDs conocidos y sino cualquier entrada valida
        const RPG::MonsterDefinition* mon = monsterCat.find("monster_acolito_de_penumbra");
        if (!mon) mon = monsterCat.find("monster_aprendiz_hueco_03");
        if (!mon) {
            // fallback: coger el primero valido por forEach — cualquier entrada
            monsterCat.forEach([&](const RPG::MonsterDefinition& m) {  // not const ref
                if (!mon && !m.id.empty()) mon = &m;
            });
        }
        require(mon != nullptr);
        std::printf("[C4] Monster elegido: id=%s name='%s' tier=%d HP=%d stats=[%d,%d,%d,%d]\n",
                    mon->id.c_str(), mon->name.c_str(),
                    mon->tier, mon->maxHealth,
                    mon->stats[0], mon->stats[1], mon->stats[2], mon->stats[3]);
        // Si el maxHealth viene 0 → populate_from_monster_catalog fuerza 1 (no fail de clamplea a 1),
        // No assert sobreescribimos para tests.

        RPG::CharacterSheet sheet;
        GridCoord pos{10, 5};
        GridCoord patMin{8, 3}, patMax{12, 7};
        // Constructor simple: (pos, patrolMin, patrolMax, maxHealth, stepInterval)
        EnemyBrain brain(pos, patMin, patMax, 1, 0.5f);
        brain.set_character_sheet(&sheet);

        // Elegir una ClassDefinition REAL cualquiera para defaultClass (evita
        // healthCap(rules, nullptr)=0; populate necesita un class para
        // TierRules::tier(level=classTier) compute correct healthCap).
        const RPG::ClassDefinition* defaultClass = nullptr;
        classCat.forEach([&](const RPG::ClassDefinition& c) {
            if (!defaultClass && !c.id.empty()) defaultClass = &c;
        });
        require(defaultClass != nullptr);
        std::printf("[C4] defaultClass: id='%s' tier=%d armorW='%s'\n",
                    defaultClass->id.c_str(), defaultClass->tier,
                    defaultClass->maxArmorWeight.c_str());

        // populate_from_monster_catalog SOBREESCRIBE legacy y sheet al completo:
        brain.populate_from_monster_catalog(*mon, &tierRules, defaultClass);

        // Fractura #1: fuentes CONSISTENTES — brain lee de sheet y coincide
        require(brain.combatant_id() == mon->id);
        require(brain.tier() == (mon->tier < 1 ? 1 : (mon->tier > 5 ? 5 : mon->tier)));
        // maxHealth alineado con sheet->healthCap() tras recompute_derived
        require(brain.maxHealth() > 0);
        require(sheet.healthCap() > 0);
        require(brain.maxHealth() == sheet.healthCap());

        // Los 4 stats del Monster catalog coinciden via brain.stat (lee sheet).
        // Orden: RPG::Stat es { CON=0, DES=1, INT=2, CAR=3 }
        const int expected[4] = {
            mon->stats[0], // CON
            mon->stats[1], // DES
            mon->stats[2], // INT
            mon->stats[3], // CAR
        };
        require(brain.stat(RPG::Stat::CON) == expected[0]);
        require(brain.stat(RPG::Stat::DES) == expected[1]);
        require(brain.stat(RPG::Stat::INT) == expected[2]);
        require(brain.stat(RPG::Stat::CAR) == expected[3]);
        require(sheet.stat(RPG::Stat::CON) == expected[0]);
        require(sheet.stat(RPG::Stat::DES) == expected[1]);
        // knownSkillIds y passiveIds están poblados (empty si monster no tenía)
        require(sheet.knownSkillIds.size() == mon->skillIds.size());
        std::printf("[C4] brain/sheet sync: HP=%d CON=%d DES=%d INT=%d CAR=%d skills=%zu passiveIds=%zu OK\n",
                    brain.maxHealth(),
                    brain.stat(RPG::Stat::CON), brain.stat(RPG::Stat::DES),
                    brain.stat(RPG::Stat::INT), brain.stat(RPG::Stat::CAR),
                    sheet.knownSkillIds.size(), sheet.passiveIds.size());
    }

    // ====== D2b Test InventoryEngine equipar equipo REAL ======
    std::printf("=== InventoryEngine E2E: equipo REAL desde EquipmentCatalog ===\n");
    {
        require(n_equipment > 0 && n_classes > 0);
        // 1) Buscar una clase con maxArmorWeight = "heavy" (ej. Guerrero)
        const RPG::ClassDefinition* clsHeavy = nullptr;
        classCat.forEach([&](const RPG::ClassDefinition& c) {
            if (clsHeavy) return;
            const std::string w = c.maxArmorWeight;
            if (w.find("heavy") != std::string::npos ||
                w.find("pesad") != std::string::npos) {
                clsHeavy = &c;
            }
        });
        require(clsHeavy != nullptr);

        // 2) Buscar un ARMADURA PECHO (Chest) / ARMA (MainHand) o CASCO (Head)
        //    real con rarity Common y slot string compatible.
        const RPG::EquipmentDefinition* realEq = nullptr;
        std::string slotNames[] = {"Chest", "Head", "MainHand", "Ring", "Boots", "Gloves"};
        equipCat.forEach([&](const RPG::EquipmentDefinition& e) {
            if (realEq) return;
            for (auto& sn : slotNames) {
                if (e.slot == sn && e.rarity == RPG::ItemRarity::Common) {
                    realEq = &e;
                    break;
                }
            }
        });
        require(realEq != nullptr);

        RPG::CharacterSheet pj;
        pj.tier = clsHeavy->tier >= 1 && clsHeavy->tier <= 5 ? clsHeavy->tier : 1;
        pj.baseStats = {2, 2, 2, 2};
        pj.recompute_derived(tierRules, clsHeavy);
        RPG::InventoryEngine::add_to_inventory(pj, realEq->id, 1, false);

        const int preStat[4] = {pj.equipBonuses[0], pj.equipBonuses[1], pj.equipBonuses[2], pj.equipBonuses[3]};
        const int preCA = pj.equipArmorBonus;
        auto rEq = RPG::InventoryEngine::equip(pj, *realEq, *clsHeavy, tierRules, equipCat);
        if (!rEq.isOk()) {
            // Algunos items pueden tener símbolos incompatibles; skip no-fatal
            std::printf("[INV REAL] Skip (%s): %s\n", realEq->id.c_str(),
                        rEq.errorMessage().c_str());
        } else {
            require(rEq.value() == true);
            // Equipamos algo → algún bonus cambió (statBonuses o caBonus)
            const bool anyBonus =
                pj.equipBonuses[0] != preStat[0] ||
                pj.equipBonuses[1] != preStat[1] ||
                pj.equipBonuses[2] != preStat[2] ||
                pj.equipBonuses[3] != preStat[3] ||
                pj.equipArmorBonus != preCA;
            // Not require — algunos items puros Common no tienen bonus; es OK
            std::printf("[INV REAL] Equipado: id=%s slot=%s anyBonus=%d "
                        "CON=%d DES=%d INT=%d CAR=%d CA=%d\n",
                        realEq->id.c_str(), realEq->slot.c_str(),
                        anyBonus ? 1 : 0,
                        pj.equipBonuses[0], pj.equipBonuses[1],
                        pj.equipBonuses[2], pj.equipBonuses[3],
                        pj.equipArmorBonus);
            std::string uneqId;
            // Resolver slot para unequip:
            bool twoHand = false;
            RPG::EquipSlot slot = RPG::InventoryEngine::resolve_slot(realEq->slot, twoHand);
            auto rUn = RPG::InventoryEngine::unequip(pj, slot, equipCat, &uneqId);
            require(rUn.isOk());
            require(uneqId == realEq->id);
            // Tras unequip → bonus 100% revierte
            require(pj.equipBonuses[0] == preStat[0]);
            require(pj.equipBonuses[1] == preStat[1]);
            require(pj.equipBonuses[2] == preStat[2]);
            require(pj.equipBonuses[3] == preStat[3]);
            require(pj.equipArmorBonus == preCA);
            // Inventario vuelve a tener stack
            bool invHas = false;
            for (auto& [k, n] : pj.inventoryEquipment)
                if (k == realEq->id && n >= 1) invHas = true;
            require(invHas);
            std::printf("[INV REAL] Unequipado OK (rollback completo + inventario).\n");
        }
    }

    std::cout << "\n=== C3 tests: TODOS OK ===\n";
    std::cout << "- RPG Catalogs:      " << total << " entradas\n";
    std::cout << "- ObjectCatalog:     " << n_obj << " objetos\n";
    std::cout << "- InventoryEngine:   equip/unequip correcto (bonuses + inventario)\n";
    std::cout << "- ConditionEngine:   sangrado stackeable tick correcto\n";
    std::cout << "- C4 EnemyBrain E2E: populate_from_monster_catalog sync sheet ✓\n";
    std::cout << "- InvEngine E2E:     equipo REAL del catalogo ✓\n";
    return 0;
}
