// D-08 — Els enemics del joc són els del catàleg.
//
// Prova que EnemyCatalogBridge cria enemics amb stats reals del
// MonsterDefinition (no zeros) i que el fallback a ObjectCatalog
// funciona quan no hi ha catàleg RPG.

#include "TestHarness.h"

#include "Game/EnemyCatalogBridge.h"
#include "Game/EnemyBrain.h"
#include "World/World.h"
#include "Level/ObjectCatalog.h"
#include "RPG/Catalogs/RpgCatalogs.h"

#include <array>
#include <string>
#include <vector>

namespace EnemyCatalogBridge = EnemyCatalog;

namespace {

RPG::Catalogs::Catalog<RPG::MonsterDefinition> makeMonsterCatalog() {
    RPG::Catalogs::Catalog<RPG::MonsterDefinition> cat;
    const std::string json = R"({
        "entries": [
            {
                "id": "slime",
                "name": "Slime",
                "tier": 1,
                "stats": [3, 1, 0, 0],
                "maxHealth": 15,
                "ca": 10,
                "defMental": 8,
                "resFis": 12,
                "precMag": 10,
                "skillIds": ["golpe"],
                "rank": "criatura"
            },
            {
                "id": "dragon_ancient",
                "name": "Dragon Antic",
                "tier": 5,
                "stats": [6, 3, 4, 2],
                "maxHealth": 200,
                "ca": 20,
                "defMental": 18,
                "resFis": 22,
                "precMag": 16,
                "skillIds": ["aliento_dragón", "golpe", "aura_miedo"],
                "rank": "jefe"
            }
        ]
    })";
    auto result = cat.loadFromString(json);
    MOTOR_CHECK(result.isOk());
    return cat;
}

ObjectDefinition makeEnemyDef(const std::string& id) {
    ObjectDefinition def;
    def.id = id;
    def.category = ObjectCategory::Enemy;
    def.combat.maxHealth = 10;  // ObjectCatalog diu 10, MonsterDefinition diu 15
    def.combat.maxMana = 2;
    def.combat.skillIds = {"golpe_basico"};
    return def;
}

}  // namespace

MOTOR_TEST_CASE(enemic_amb_monster_catalog_té_stats_reals) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("slime");

    auto enemy = EnemyCatalogBridge::createEnemy(
        "slime", &def, &catalog,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7});

    // Ha d'utilitzar el maxHealth del MonsterDefinition (15), no del ObjectCatalog (10)
    MOTOR_CHECK(enemy.brain->maxHealth() == 15);

    // Les stats han d'estar poblades (no zeros)
    MOTOR_CHECK(enemy.brain->stat(RPG::Stat::CON) == 3);
    MOTOR_CHECK(enemy.brain->stat(RPG::Stat::DES) == 1);
}

MOTOR_TEST_CASE(enemic_sense_monster_catalog_fallback_a_object_catalog) {
    ObjectDefinition def = makeEnemyDef("slime");

    auto enemy = EnemyCatalogBridge::createEnemy(
        "slime", &def, nullptr,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7});

    // Sense MonsterCatalog, utilitza el maxHealth de ObjectCatalog (10)
    MOTOR_CHECK(enemy.brain->maxHealth() == 10);

    // Les stats són zero (sense populate_from_monster_catalog)
    MOTOR_CHECK(enemy.brain->stat(RPG::Stat::CON) == 0);
}

MOTOR_TEST_CASE(enemic_no_trobat_al_monster_catalog_fallback) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("slime_inexistent");

    auto enemy = EnemyCatalogBridge::createEnemy(
        "slime_inexistent", &def, &catalog,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7});

    // No trobat al catàleg RPG → fallback a ObjectCatalog
    MOTOR_CHECK(enemy.brain->maxHealth() == 10);
    MOTOR_CHECK(enemy.brain->stat(RPG::Stat::CON) == 0);
}

MOTOR_TEST_CASE(vida_explicita_sobreescribe_la_del_catàleg) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("slime");

    auto enemy = EnemyCatalogBridge::createEnemy(
        "slime", &def, &catalog,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7},
        false, 0, 5);  // health = 5

    MOTOR_CHECK(enemy.brain->health() == 5);
    MOTOR_CHECK(enemy.brain->maxHealth() == 15);  // maxHealth continua sent el del catàleg
}

MOTOR_TEST_CASE(dragon_tier5_té_stats_altes) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("dragon_ancient");
    def.combat.maxHealth = 50;  // ObjectCatalog diu 50, MonsterDefinition diu 200

    auto enemy = EnemyCatalogBridge::createEnemy(
        "dragon_ancient", &def, &catalog,
        GridCoord{0, 0}, GridCoord{0, 0}, GridCoord{0, 0});

    MOTOR_CHECK(enemy.brain->maxHealth() == 200);
    MOTOR_CHECK(enemy.brain->tier() == 5);
    MOTOR_CHECK(enemy.brain->stat(RPG::Stat::CON) == 6);
}

// ===== H2-07: el prefijo `enemy_` de assets/objects contra el id desnudo de
// monsters.json. Sin esto 430 de 460 enemigos no resolvian ficha y jugaban con
// la vida del placeholder (10). Las dos grafias del catálogo de prueba:
//
//   objects/enemy_slime   <->  monsters/slime          (lo que hay)
//   objects/slime          <->  monsters/slime          (prueba de que el
//                                                     exacto sigue ganando)

MOTOR_TEST_CASE(enemic_amb_prefixe_enemy_troba_la_seva_ficha) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("enemy_slime");
    def.combat.maxHealth = 10;  // el placeholder que se colaba sin la ficha

    auto enemy = EnemyCatalogBridge::createEnemy(
        "enemy_slime", &def, &catalog,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7});

    // 15 viene de monsters.json (slime), no de los 10 del ObjectCatalog.
    MOTOR_CHECK(enemy.brain->maxHealth() == 15);
    MOTOR_CHECK(enemy.brain->stat(RPG::Stat::CON) == 3);
    MOTOR_CHECK(enemy.brain->stat(RPG::Stat::DES) == 1);
}

MOTOR_TEST_CASE(enemic_amb_prefixe_enemy_també_correge_l_habilitat) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("enemy_slime");

    auto enemy = EnemyCatalogBridge::createEnemy(
        "enemy_slime", &def, &catalog,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7});

    // skillIds tambien vienen del catálogo ("golpe"), no del "golpe_basico"
    // del ObjectCatalog: el prefijo no arregla solo la vida.
    MOTOR_CHECK(enemy.skills != nullptr);
    MOTOR_CHECK(enemy.skills->knows("golpe"));
}

MOTOR_TEST_CASE(id_exacte_guanya_al_prefijo) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("slime");
    def.combat.maxHealth = 10;

    auto enemy = EnemyCatalogBridge::createEnemy(
        "slime", &def, &catalog,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7});

    MOTOR_CHECK(enemy.brain->maxHealth() == 15);
}

// El caso que de verdad fija el ORDEN: cuando el catálogo tiene LAS DOS
// grafias, gana la literal. Sin esto, invertir el orden del find pasava los
// 12 tests: nadie miraba qué ficha se elegía cuando había dos candidatas.
MOTOR_TEST_CASE(amb_grafies_al_cataleg_gana_la_literal) {
    RPG::Catalogs::Catalog<RPG::MonsterDefinition> cat;
    // Delimitador propio: un nombre con parentesis ("Slime (con prefijo)")
    // produciria el )" que cierra un R"(...)" antes de tiempo.
    const std::string json = R"J({
        "entries": [
            {"id": "slime", "name": "Slime", "tier": 1, "stats": [3, 1, 0, 0],
             "maxHealth": 15, "rank": "criatura"},
            {"id": "enemy_slime", "name": "Slime (con prefijo)", "tier": 4,
             "stats": [8, 8, 8, 8], "maxHealth": 99, "rank": "elite"}
        ]
    })J";
    auto result = cat.loadFromString(json);
    MOTOR_CHECK(result.isOk());
    ObjectDefinition def = makeEnemyDef("enemy_slime");
    def.combat.maxHealth = 10;

    auto enemy = EnemyCatalogBridge::createEnemy(
        "enemy_slime", &def, &cat,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7});

    // 99 y tier 4 = la entrada LITERAL "enemy_slime"; no los 15/tier 1 de
    // "slime" a la que se caería buscando sin el prefijo primero.
    MOTOR_CHECK(enemy.brain->maxHealth() == 99);
    MOTOR_CHECK(enemy.brain->tier() == 4);
}

MOTOR_TEST_CASE(prefixe_sin_ficha_torna_a_caure_al_placeholder) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("enemy_larva_inexistente");

    auto enemy = EnemyCatalogBridge::createEnemy(
        "enemy_larva_inexistente", &def, &catalog,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7});

    // No hay ficha ni con prefijo: fallback real, 10 y stats a cero.
    MOTOR_CHECK(enemy.brain->maxHealth() == 10);
    MOTOR_CHECK(enemy.brain->stat(RPG::Stat::CON) == 0);
}

MOTOR_TEST_CASE(enemic_amb_prefixe_i_sense_cataleg_no_revienta) {
    ObjectDefinition def = makeEnemyDef("enemy_slime");

    // monsterCatalog == nullptr con id prefijada: nullptr, no crash.
    auto enemy = EnemyCatalogBridge::createEnemy(
        "enemy_slime", &def, nullptr,
        GridCoord{5, 5}, GridCoord{3, 3}, GridCoord{7, 7});

    MOTOR_CHECK(enemy.brain->maxHealth() == 10);
}

MOTOR_TEST_CASE(cataleg_anul_o_id_anula_no_revientan) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("enemy_slime");

    MOTOR_CHECK(EnemyCatalogBridge::findMonster(nullptr, "enemy_slime") == nullptr);
    MOTOR_CHECK(EnemyCatalogBridge::findMonster(&catalog, "") == nullptr);
    // "enemy_" a pelo son 6 caracteres: no es un id, no hay ficha que buscar.
    MOTOR_CHECK(EnemyCatalogBridge::findMonster(&catalog, "enemy_") == nullptr);
}

MOTOR_TEST_CASE(invocat_no_patrulla) {
    auto catalog = makeMonsterCatalog();
    ObjectDefinition def = makeEnemyDef("slime");

    auto enemy = EnemyCatalogBridge::createEnemy(
        "slime", &def, &catalog,
        GridCoord{5, 5}, GridCoord{5, 5}, GridCoord{5, 5},
        true, 42);  // summoned=true, identity=42

    MOTOR_CHECK(enemy.summoned == true);
    MOTOR_CHECK(enemy.spawnIndex == 42);
    MOTOR_CHECK(enemy.brain->maxHealth() == 15);
}

// ACOMPANANTES-01 (estudio de dificultad, 06/10) — Un enemigo sin ficha de
// monstruo lleva sus dados en el objeto (combat.stats): el devoto del Ocaso,
// 12 PV y CON 1. Fuera de 0-8, error con nombre; sin stats, todo a 0; y el
// catalogo los vuelve a escribir.
MOTOR_TEST_CASE(los_dados_de_un_enemigo_sin_ficha_salen_del_objeto) {
    ObjectCatalog objetos;
    MOTOR_CHECK(objetos.loadFromString(R"({"objects": [
        {"id": "devoto", "name": "Devoto", "category": "enemy",
         "combat": {"maxHealth": 12, "maxMana": 0, "skills": [], "stats": {"CON": 1, "DES": 2}}},
        {"id": "raso", "name": "Raso", "category": "enemy", "combat": {"maxHealth": 9}}
    ]})").isOk());
    const ObjectDefinition* devoto = objetos.find("devoto");
    MOTOR_CHECK(devoto != nullptr && devoto->combat.stats[0] == 1 && devoto->combat.stats[1] == 2 &&
                devoto->combat.stats[2] == 0 && devoto->combat.stats[3] == 0);
    if (devoto == nullptr) return;
    auto enemigo = EnemyCatalogBridge::createEnemy("devoto", devoto, nullptr, GridCoord{1, 1}, GridCoord{1, 1},
                                                    GridCoord{1, 1});
    MOTOR_CHECK(enemigo.brain->maxHealth() == 12);
    MOTOR_CHECK(enemigo.brain->stat(RPG::Stat::CON) == 1 && enemigo.brain->stat(RPG::Stat::DES) == 2);
    auto raso = EnemyCatalogBridge::createEnemy("raso", objetos.find("raso"), nullptr, GridCoord{1, 1},
                                                 GridCoord{1, 1}, GridCoord{1, 1});
    MOTOR_CHECK(raso.brain->stat(RPG::Stat::CON) == 0 && raso.brain->stat(RPG::Stat::DES) == 0);
    ObjectCatalog malo;
    MOTOR_CHECK(!malo.loadFromString(R"({"objects": [{"id": "x", "name": "X", "category": "enemy",
        "combat": {"maxHealth": 5, "stats": {"DES": 9}}}]})").isOk());
    // Al exportar el catalogo, los dados van con el (y el raso, sin ellos).
    const std::string exportado = objetos.exportJson();
    MOTOR_CHECK(exportado.find("\"stats\": {\"CON\": 1, \"DES\": 2, \"INT\": 0, \"CAR\": 0}") != std::string::npos);
    ObjectCatalog otra;
    MOTOR_CHECK(otra.loadFromString(exportado).isOk());
    MOTOR_CHECK(otra.find("devoto") != nullptr && otra.find("devoto")->combat.stats == devoto->combat.stats);
    MOTOR_CHECK(otra.find("raso") != nullptr && otra.find("raso")->combat.stats == (std::array<int, 4>{}));
}

int main() { return motor_test::runAllTests(); }
