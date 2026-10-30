// C-24 — Xarxa de seguretat al camí que es juga.
//
// Proba l'EnemyBrain pura (patrulla i llibre de combat ICombatant), que és
// la peça GL-free que GameSession posseeix directament: mai no hi ha hagut
// un test unitari d'aquesta classe -- només s'exercitava des dels demos i,
// ara, des del bridge de catàleg (EnemyCatalogBridgeTests). Aquí es fixen
// els tres comportaments documentats al propi header:
//   - la patrulla per rang amb rebot al mateix tic als extrems,
//   - l'acumulació de temps per passos (2 fps o 120 fps, mateix ritme),
//   - el descompte de vida/defenses en llesca legacy sense CharacterSheet.
//
// Clamps defensius del constructor (vida mínima 1, interval mínim 0.01) i
// clamps de populate/tier/stat/health, perquè ningú més els ha fixat.

#include "TestHarness.h"

#include "Game/EnemyBrain.h"
#include "Game/EnemyBrainData.h"
#include "RPG/TierRules.h"

namespace {

RPG::MonsterDefinition makeMonster() {
    RPG::MonsterDefinition mon;
    mon.id = "slime";
    mon.name = "Slime";
    mon.tier = 1;
    mon.stats = {3, 1, 0, 0};  // CON DES INT CAR
    mon.maxHealth = 15;
    mon.role = "striker";
    mon.skillIds = {"golpe"};
    mon.passiveId = "gel";
    return mon;
}

bool allStepsFree(const GridCoord&) { return true; }

}  // namespace

MOTOR_TEST_CASE(datos_del_brain_conservan_los_clamps_del_constructor) {
    // Esta es la misma ruta que usa EnemyBrain al nacer; fija que al mover
    // el estado no se haya perdido la protección contra un enemigo muerto
    // ni contra un bucle de patrulla infinito.
    EnemyBrainData data(GridCoord{2, 3}, GridCoord{1, 3}, GridCoord{4, 3}, 0, 0.0f);
    MOTOR_CHECK(data.health == 1);
    MOTOR_CHECK(data.maxHealth == 1);
    MOTOR_CHECK(data.stepInterval == 0.01f);
}

MOTOR_TEST_CASE(el_constructor_clampa_vida_minima_y_intervalo) {
    // maxHealth 0 n'hi hauria un enemic nascut mort (fractura #1); l'interval
    // 0 donaria un bucle infinit dins d'update().
    EnemyBrain b(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{5, 0}, 0, 0.0f);
    MOTOR_CHECK(b.maxHealth() == 1);
    MOTOR_CHECK(b.health() == 1);
    MOTOR_CHECK(b.isAlive());
}

MOTOR_TEST_CASE(rango_degenerado_devuelve_idle_y_no_mueve) {
    // patrolMin.x == patrolMax.x: enemic quiet (el que LevelLoader produeix
    // quan el JSON no porta patrulla). És el cas normal, no un error.
    EnemyBrain quiet(GridCoord{4, 3}, GridCoord{7, 3}, GridCoord{7, 3});
    quiet.update(5.0f, allStepsFree);
    MOTOR_CHECK(quiet.aiState() == EnemyBrain::kIdle);
    MOTOR_CHECK(quiet.position().x == 4);

    EnemyBrain invertido(GridCoord{4, 3}, GridCoord{8, 3}, GridCoord{7, 3});
    invertido.update(5.0f, allStepsFree);
    MOTOR_CHECK(invertido.aiState() == EnemyBrain::kIdle);
    MOTOR_CHECK(invertido.position().x == 4);
}

MOTOR_TEST_CASE(la_patrulla_avanza_un_paso_por_intervalo) {
    EnemyBrain b(GridCoord{0, 2}, GridCoord{0, 2}, GridCoord{5, 2}, 50, 0.5f);
    b.update(0.5f, allStepsFree);
    MOTOR_CHECK(b.aiState() == EnemyBrain::kPatrol);
    MOTOR_CHECK(b.position().x == 1);
    b.update(0.5f, allStepsFree);
    b.update(0.5f, allStepsFree);
    MOTOR_CHECK(b.position().x == 3);
}

MOTOR_TEST_CASE(rebota_en_los_extremos_sin_tic_de_pausa) {
    // El comentari del header és explícit: "rebotar al intentar salir deja
    // al enemigo parado un tic en cada punta" seria un bug; al mateix tic en
    // què s'arriba a l'extrem s'inverteix el sentit.
    EnemyBrain b(GridCoord{4, 0}, GridCoord{0, 0}, GridCoord{5, 0}, 50, 0.5f);
    b.update(0.5f, allStepsFree);  // 4 -> 5
    MOTOR_CHECK(b.position().x == 5);
    MOTOR_CHECK(b.direction() == -1);   // retorna en el mateix tic
    b.update(0.5f, allStepsFree);       // 5 -> 4 sense pausar-se al 5
    MOTOR_CHECK(b.position().x == 4);
}

MOTOR_TEST_CASE(idayuelta_completa_vuelve_al_origen) {
    EnemyBrain b(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{5, 0}, 50, 0.5f);
    for (int i = 0; i < 5; ++i)
        b.update(0.5f, allStepsFree);  // 0->5
    MOTOR_CHECK(b.position().x == 5);
    for (int i = 0; i < 5; ++i)
        b.update(0.5f, allStepsFree);  // 5->0
    MOTOR_CHECK(b.position().x == 0);
    MOTOR_CHECK(b.direction() == 1);   // torna a mirar cap a la dreta
}

MOTOR_TEST_CASE(acumula_el_sobrante_de_tiempo) {
    // Valors binari-exactes (0.25, 0.5, 1.25): la suma de sobres floats com
    // 0.4+0.1 no és 0.5 exacte i la deriva faria el test aleatori.
    EnemyBrain b(GridCoord{1, 0}, GridCoord{1, 0}, GridCoord{5, 0}, 50, 0.5f);
    b.update(0.25f, allStepsFree);
    MOTOR_CHECK(b.position().x == 1);  // mig interval: encara no
    b.update(0.25f, allStepsFree);
    MOTOR_CHECK(b.position().x == 2);  // 0.5 acumulats: un pas

    // Un delta gran dona varis passos en un sol frame i guarda el sobrant.
    EnemyBrain c(GridCoord{1, 0}, GridCoord{1, 0}, GridCoord{9, 0}, 50, 0.5f);
    c.update(1.25f, allStepsFree);  // 2.5 intervals -> dos passos, sobrant 0.25
    MOTOR_CHECK(c.position().x == 3);
    c.update(0.25f, allStepsFree);  // sobrant 0.25 + 0.25 = un pas mes
    MOTOR_CHECK(c.position().x == 4);
}

MOTOR_TEST_CASE(obstaculo_invierte_el_sentido_y_gasta_el_tic) {
    // Contra un mur el pas es gasta donant mitja volta, no es queda empenyent.
    EnemyBrain b(GridCoord{3, 0}, GridCoord{0, 0}, GridCoord{9, 0}, 50, 0.5f);
    bool blocked = false;
    b.update(0.5f, [&blocked](const GridCoord& next) {
        return !(blocked && next.x == 4);
    });
    MOTOR_CHECK(b.position().x == 4);  // pas normal cap a la dreta
    blocked = true;                    // el mur és A LA CELL·LA 5 (la que ve)
    b.update(0.5f, [&blocked](const GridCoord& next) { return !(blocked && next.x == 5); });
    MOTOR_CHECK(b.position().x == 4);  // xocat contra el mur
    MOTOR_CHECK(b.direction() == -1);  // i ha girat
    blocked = false;
    b.update(0.5f, [](const GridCoord&) { return true; });  // ara torna
    MOTOR_CHECK(b.position().x == 3);
}

MOTOR_TEST_CASE(combate_vida_dano_curacion_y_clamps) {
    EnemyBrain b(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 50, 0.5f);
    b.takeDamage(30);
    MOTOR_CHECK(b.health() == 20);
    b.takeDamage(1000);          // mai negativa
    MOTOR_CHECK(b.health() == 0);
    MOTOR_CHECK(!b.isAlive());
    b.heal(10);
    MOTOR_CHECK(b.health() == 10);
    b.heal(1000);                // mai per sobre del màxim
    MOTOR_CHECK(b.health() == 50);
}

MOTOR_TEST_CASE(sin_sheet_las_defensas_son_la_lesca_legacy) {
    EnemyBrain b(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 50, 0.5f);
    b.populate_from_monster_catalog(makeMonster());
    MOTOR_CHECK(b.maxHealth() == 15);
    MOTOR_CHECK(b.health() == 15);
    MOTOR_CHECK(b.stat(RPG::Stat::CON) == 3);
    MOTOR_CHECK(b.stat(RPG::Stat::CAR) == 0);
    MOTOR_CHECK(b.tier() == 1);
    MOTOR_CHECK(b.combatant_id() == "slime");
    // ca = 10 + DES + equip_armor = 11; phy/vul = 10 + CON/INT/CAR.
    const RPG::DefenseBlock def = b.defenses();
    MOTOR_CHECK(def.values[static_cast<int>(RPG::Defense::ARMOR_CLASS)] == 11);
    MOTOR_CHECK(def.values[static_cast<int>(RPG::Defense::PHYSICAL_SAVE)] == 13);
    MOTOR_CHECK(def.values[static_cast<int>(RPG::Defense::WILL_SAVE)] == 10);
    MOTOR_CHECK(def.values[static_cast<int>(RPG::Defense::SPELL_SAVE_DC)] == 10);
    MOTOR_CHECK(b.initiative_bonus() == 1);  // DES
}

MOTOR_TEST_CASE(populate_clampa_el_tier_del_catalogo) {
    RPG::MonsterDefinition mon = makeMonster();
    mon.tier = 0;  // fora del rang 1..5: acaba en 1
    EnemyBrain b(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 50, 0.5f);
    b.populate_from_monster_catalog(mon);
    MOTOR_CHECK(b.tier() == 1);

    mon.tier = 7;  // fora del rang: acaba en 5
    EnemyBrain b2(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 50, 0.5f);
    b2.populate_from_monster_catalog(mon);
    MOTOR_CHECK(b2.tier() == 5);
}

MOTOR_TEST_CASE(sheet_hace_de_fuente_unica_de_verdad) {
    RPG::CharacterSheet sheet;
    RPG::TierRules rules;  // 6 tiers per defecte; tier 1 healthCap 35
    EnemyBrain b(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 50, 0.5f);
    b.set_character_sheet(&sheet);
    b.populate_from_monster_catalog(makeMonster(), &rules);
    // recompute_derived: baseHealth 8 + CON*3 + CAR*1 + healthBonus(0) = 17 (< 35).
    MOTOR_CHECK(b.maxHealth() == 17);
    MOTOR_CHECK(b.health() == 17);             // ratio 15/15 -> 17/17
    MOTOR_CHECK(b.tier() == 1);
    MOTOR_CHECK(b.combatant_id() == "slime");
    MOTOR_CHECK(sheet.id == "slime");
    MOTOR_CHECK(sheet.displayName == "Slime");
    MOTOR_CHECK(sheet.classId == "striker");
    MOTOR_CHECK(sheet.tier == 1);
    MOTOR_CHECK(sheet.baseStats == makeMonster().stats);
    MOTOR_CHECK(sheet.knownSkillIds.size() == 1 && sheet.knownSkillIds[0] == "golpe");
    MOTOR_CHECK(sheet.passiveIds.size() == 1 && sheet.passiveIds[0] == "gel");
}

MOTOR_TEST_CASE(sin_tier_rules_no_se_recalcula_el_tope_del_sheet) {
    // Amb sheet adjunt però sense recompute_derived, l'accessor del tope
    // llegeix el CACHE del sheet (0) i no el m_maxHealth: és el mateix
    // contracte que declara el CharacterSheet ("si no se ha llamado:
    // defaults 0s defensas"). El bus real (GameSession/Catalogs) SEMPRE
    // passa TierRules; aquest test adverteix si algú 'arregla' el cas
    // canviant-lo silenciosament.
    RPG::CharacterSheet sheet;
    EnemyBrain b(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 50, 0.5f);
    b.set_character_sheet(&sheet);
    b.populate_from_monster_catalog(makeMonster());
    MOTOR_CHECK(b.maxHealth() == 0);
}

MOTOR_TEST_CASE(set_character_sheet_adopta_el_tope_cacheados) {
    RPG::CharacterSheet sheet;
    sheet.cachedHealthCap = 17;  // com si recompute_derived ja s'hagués cridat
    EnemyBrain b(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 50, 0.5f);
    b.set_character_sheet(&sheet);
    MOTOR_CHECK(b.maxHealth() == 17);
    MOTOR_CHECK(b.health() == 17);
}

MOTOR_TEST_CASE(setters_de_equip_armor_y_health_respetan_los_clamps) {
    EnemyBrain b(GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0}, 50, 0.5f);
    b.set_equip_armor(3);  // ca = 10 + DES(0) + equip_armor(3) = 13
    MOTOR_CHECK(b.defenses().values[static_cast<int>(RPG::Defense::ARMOR_CLASS)] == 13);

    b.set_max_health(0);
    MOTOR_CHECK(b.maxHealth() == 1);
    b.set_health(999);
    MOTOR_CHECK(b.health() == b.maxHealth());
    b.set_health(-5);
    MOTOR_CHECK(b.health() == 0);
}

int main() { return motor_test::runAllTests(); }
