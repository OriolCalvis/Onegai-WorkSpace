// Fase 10 (motor_grafico_gantt_rpg.puml): catalogo generico de objetos
// ("todo es un objeto"). GL-free igual que demo_level_loader/demo_skills/
// demo_battle: ObjectCatalog/ObjectSpawn/la accion Item de BattleState
// son datos y logica pura, sin Entity/Texture/GL.
//
// Verifica:
//  - ObjectCatalog::loadFromString(): las tres categorias (prop/enemy/
//    pickup) con sus datos especificos, defaults de campos opcionales,
//    merge de varios archivos (id repetido sobreescribe), y errores
//    estructurales (sin "objects", entrada sin "id", categoria/efecto
//    desconocidos) como Result::Error SIN mutar el catalogo (todo-o-nada).
//  - loadFromFile() sobre assets/objects/test_objects.json (el archivo
//    real del repo).
//  - LevelLoader con "objects": ObjectSpawn (objectId/position/patrulla
//    con default a position), compatibilidad con "enemies" en el mismo
//    nivel, y assets/levels/test_level.json real trayendo ambos.
//  - BattleState + accion Item (Fase 10 cerrando el hueco de la Fase 8):
//    pocion (heal, con clamp real), eter (restoreMana), item que no se
//    tiene, item sin efecto (llave, NO se consume), y el consumo real
//    (una instancia por uso) del inventario del actor.
#include "Game/BattleState.h"
#include "Render/ICombatant.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"
#include "Game/Skill.h"

#include "Check.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace {

// Mismo ICombatant de prueba que demo_skills.cpp/demo_battle.cpp.
class TestCombatant : public ICombatant {
public:
    explicit TestCombatant(int maxHealth) : m_health(maxHealth), m_maxHealth(maxHealth) {}

    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::clamp(m_health + amount, 0, m_maxHealth); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }

    // Stubs de los metodos RPG de ICombatant (stats/defensas/tier/id): un
    // TestCombatant de demo no necesita valores reales, solo dejar de ser
    // abstracto. Defaults neutros.
    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }

private:
    int m_health;
    int m_maxHealth;
};

void testCatalogParsing() {
    ObjectCatalog catalog;
    auto result = catalog.loadFromString(R"({
        "objects": [
            {"id": "roca", "category": "prop", "blocksMovement": true},
            {"id": "pocion", "name": "Pocion", "category": "pickup",
             "pickup": {"effect": "heal", "power": 15}},
            {"id": "slime", "name": "Slime", "category": "enemy", "spriteId": 2,
             "combat": {"maxHealth": 20, "maxMana": 4, "skills": ["golpe_gelatinoso"]}}
        ]
    })");
    require(result.isOk());
    require(result.value() == 3);
    require(catalog.size() == 3);

    // Prop: defaults de lo no especificado (name = id, spriteId = -1).
    const ObjectDefinition* roca = catalog.find("roca");
    require(roca != nullptr);
    require(roca->category == ObjectCategory::Prop);
    require(roca->name == "roca");
    require(roca->spriteId == -1);
    require(roca->blocksMovement);
    require(!roca->interactable);

    // Pickup: efecto + power.
    const ObjectDefinition* pocion = catalog.find("pocion");
    require(pocion != nullptr);
    require(pocion->category == ObjectCategory::Pickup);
    require(pocion->pickup.effect == PickupEffect::Heal);
    require(pocion->pickup.power == 15);

    // Enemy: CombatData completo.
    const ObjectDefinition* slime = catalog.find("slime");
    require(slime != nullptr);
    require(slime->category == ObjectCategory::Enemy);
    require(slime->combat.maxHealth == 20);
    require(slime->combat.maxMana == 4);
    require(slime->combat.skillIds.size() == 1);
    require(slime->combat.skillIds[0] == "golpe_gelatinoso");

    // No encontrado: nullptr, sin lanzar.
    require(catalog.find("no_existe") == nullptr);
    require(!catalog.has("no_existe"));

    // Merge de un segundo "archivo": anade uno nuevo y sobreescribe
    // "roca" (ultimo gana, ver loadFromString en el header).
    auto merged = catalog.loadFromString(R"({
        "objects": [
            {"id": "roca", "category": "prop", "blocksMovement": false},
            {"id": "llave", "category": "pickup", "pickup": {"effect": "none"}}
        ]
    })");
    require(merged.isOk());
    require(merged.value() == 2);
    require(catalog.size() == 4);                    // 3 + 2 - 1 sobreescrito
    require(!catalog.find("roca")->blocksMovement);  // la version nueva
    require(catalog.find("llave")->pickup.effect == PickupEffect::None);

    std::cout << "[OBJETOS] loadFromString(): 3 categorias, defaults y merge correctos.\n";
}

void testCatalogErrors() {
    ObjectCatalog catalog;
    require(catalog.loadFromString(R"({"objects": [{"id": "ok", "category": "prop"}]})").isOk());
    std::size_t sizeBefore = catalog.size();

    // Raiz sin "objects".
    require(!catalog.loadFromString(R"({"cosas": []})").isOk());
    // Entrada sin "id".
    require(!catalog.loadFromString(R"({"objects": [{"category": "prop"}]})").isOk());
    // Categoria desconocida.
    require(!catalog.loadFromString(R"({"objects": [{"id": "x", "category": "enemigo"}]})").isOk());
    // Efecto de pickup desconocido.
    require(!catalog
                 .loadFromString(
                     R"({"objects": [{"id": "x", "category": "pickup",
                        "pickup": {"effect": "revivir"}}]})")
                 .isOk());
    // JSON roto.
    require(!catalog.loadFromString("{ no es json").isOk());

    // Todo-o-nada: una entrada valida seguida de una invalida no aplica
    // NINGUNA (el catalogo queda como estaba).
    require(!catalog
                 .loadFromString(
                     R"({"objects": [{"id": "nuevo", "category": "prop"},
                        {"id": "malo", "category": "???"}]})")
                 .isOk());
    require(catalog.size() == sizeBefore);
    require(!catalog.has("nuevo"));

    std::cout << "[OBJETOS] errores estructurales -> Result::Error sin mutar el catalogo.\n";
}

void testCatalogFromFile() {
    ObjectCatalog catalog;
    auto result = catalog.loadFromFile("assets/objects/test_objects.json");
    require(result.isOk());
    require(result.value() == 5);
    require(catalog.has("arbusto") && catalog.has("llave_cueva") && catalog.has("pocion") &&
            catalog.has("eter") && catalog.has("slime"));
    require(catalog.find("eter")->pickup.effect == PickupEffect::RestoreMana);

    std::cout << "[OBJETOS] loadFromFile(\"assets/objects/test_objects.json\") correcto.\n";
}

void testLevelObjects() {
    // "objects" + "enemies" conviven; patrulla con default a position.
    auto parsed = LevelLoader::loadFromString(R"({
        "name": "Nivel mixto", "map": "assets/maps/test_map.tmx",
        "enemies": [{"type": "goblin", "position": {"x": 1, "y": 1}}],
        "objects": [
            {"objectId": "arbusto", "position": {"x": 0, "y": 2}},
            {"objectId": "slime", "position": {"x": 2, "y": 0},
             "patrolMin": {"x": 2, "y": 0}, "patrolMax": {"x": 3, "y": 0}}
        ]
    })");
    require(parsed.isOk());
    const LevelDefinition& level = parsed.value();
    require(level.enemies.size() == 1);
    require(level.objects.size() == 2);
    require(level.objects[0].objectId == "arbusto");
    // Sin patrulla explicita: patrolMin == patrolMax == position.
    require(level.objects[0].patrolMin.x == 0 && level.objects[0].patrolMin.y == 2);
    require(level.objects[0].patrolMax.x == 0 && level.objects[0].patrolMax.y == 2);
    require(level.objects[1].patrolMax.x == 3);

    // ObjectSpawn sin "objectId": error estructural.
    require(!LevelLoader::loadFromString(R"({
        "map": "m.tmx", "objects": [{"position": {"x": 0, "y": 0}}]
    })")
                 .isOk());

    // El nivel real del repo trae ambos arrays.
    auto fromFile = LevelLoader::loadFromFile("assets/levels/test_level.json");
    require(fromFile.isOk());
    require(fromFile.value().enemies.size() == 2);
    require(fromFile.value().objects.size() == 4);

    std::cout << "[OBJETOS] LevelLoader con \"objects\" (+compat \"enemies\") correcto.\n";
}

void testBattleItemAction() {
    ObjectCatalog objects;
    require(objects.loadFromFile("assets/objects/test_objects.json").isOk());
    SkillCatalog skills;

    TestCombatant heroBody(30);
    heroBody.takeDamage(20);  // 10/30: hueco de sobra para la pocion
    SkillSet heroSkills(/*maxMana=*/8);
    heroSkills.spend(Skill{"", "", 8, 0, SkillEffect::Damage, SkillTarget::Self});  // mana=0
    std::vector<std::string> inventory{"pocion", "pocion", "eter", "llave_cueva"};

    TestCombatant slimeBody(20);
    std::vector<BattleParticipant> allies{{"Heroe", &heroBody, &heroSkills, &inventory}};
    std::vector<BattleParticipant> enemies{{"Slime", &slimeBody, nullptr, nullptr}};
    BattleState battle(std::move(allies), std::move(enemies), &skills, &objects);

    // Pocion: +15 PV reales (10 -> 25), consume UNA de las dos.
    battle.resolveAllyAction(0, BattleAction{BattleActionType::Item, "", 0, "pocion"});
    require(heroBody.health() == 25);
    require(std::count(inventory.begin(), inventory.end(), "pocion") == 1);
    require(battle.log().back().find("+15 PV") != std::string::npos);

    // Segunda pocion: clamp real (25 -> 30, solo +5).
    battle.resolveAllyAction(0, BattleAction{BattleActionType::Item, "", 0, "pocion"});
    require(heroBody.health() == 30);
    require(std::count(inventory.begin(), inventory.end(), "pocion") == 0);
    require(battle.log().back().find("+5 PV") != std::string::npos);

    // Eter: +5 PM (0 -> 5), consumido.
    battle.resolveAllyAction(0, BattleAction{BattleActionType::Item, "", 0, "eter"});
    require(heroSkills.mana() == 5);
    require(std::count(inventory.begin(), inventory.end(), "eter") == 0);

    // Item que ya no se tiene: log de fallo, sin cambios.
    std::size_t logSize = battle.log().size();
    battle.resolveAllyAction(0, BattleAction{BattleActionType::Item, "", 0, "pocion"});
    require(battle.log().size() == logSize + 1);
    require(battle.log().back().find("no tiene") != std::string::npos);
    require(heroBody.health() == 30);

    // Llave (efecto none): no hace nada Y NO se consume.
    battle.resolveAllyAction(0, BattleAction{BattleActionType::Item, "", 0, "llave_cueva"});
    require(battle.log().back().find("no tiene efecto en combate") != std::string::npos);
    require(std::count(inventory.begin(), inventory.end(), "llave_cueva") == 1);

    // Sin ObjectCatalog (llamador pre-Fase 10): fallo controlado.
    TestCombatant soloBody(10);
    std::vector<std::string> soloInventory{"pocion"};
    std::vector<BattleParticipant> soloAllies{{"Solo", &soloBody, nullptr, &soloInventory}};
    std::vector<BattleParticipant> soloEnemies{{"Rata", &slimeBody, nullptr, nullptr}};
    BattleState noCatalog(std::move(soloAllies), std::move(soloEnemies), &skills);
    noCatalog.resolveAllyAction(0, BattleAction{BattleActionType::Item, "", 0, "pocion"});
    require(noCatalog.log().back().find("no puede usar") != std::string::npos);
    require(soloInventory.size() == 1);  // no consumido

    std::cout << "[OBJETOS] accion Item en BattleState (heal/mana/fallos/consumo real) "
                 "correcta.\n";
}

}  // namespace

int main() {
    testCatalogParsing();
    testCatalogErrors();
    testCatalogFromFile();
    testLevelObjects();
    testBattleItemAction();
    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
