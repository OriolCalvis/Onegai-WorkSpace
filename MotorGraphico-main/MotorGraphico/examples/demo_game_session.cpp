// Cierre del ciclo de juego: GameSession (exploracion <-> combate), la
// pieza que las Fases 8-12 iban dejando pendiente ("transicion
// automatica exploracion<->combate: necesita una Application real").
// GL-free como el resto del nucleo: se compila y EJECUTA de verdad,
// usando el TileMap y el ObjectCatalog REALES del motor (no mocks).
//
// Verifica el ciclo completo:
//  - Movimiento con colision: tiles con collision del TMX real, bordes
//    del mapa, y objetos con blocksMovement (Fase 10).
//  - Pickups: se recogen al entrar (van al inventario REAL del jugador,
//    el mismo que consume la accion Item del combate), desaparecen del
//    mundo y dejan linea en el log.
//  - Encuentro: chocar con un objeto de categoria Enemy NO mueve al
//    jugador y arranca un BattleState con los CombatData del catalogo
//    (vida/mana/habilidades del TIPO de enemigo).
//  - Los tres desenlaces: Victory (el enemigo desaparece del mundo),
//    Fled (sigue ahi), Defeat (GameOver terminal), con el log del
//    combate conservado tras destruir el BattleState.
//  - Continuidad: el mana gastado y los items consumidos en combate se
//    conservan al volver a explorar (es el MISMO jugador, no una copia).
#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "Game/Skill.h"
#include "Render/TileMap.h"

#include "Check.h"
#include "ScriptedRng.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace {

// Mismo ICombatant de prueba que demo_skills/demo_battle/demo_object_catalog.
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

// Catalogo minimo compartido por los tests (mismos ids que
// assets/objects/test_objects.json, con un enemigo debil para poder
// matarlo en pocos turnos).
ObjectCatalog makeCatalog() {
    ObjectCatalog catalog;
    auto result = catalog.loadFromString(R"({
        "objects": [
            {"id": "arbusto", "name": "Arbusto", "category": "prop", "blocksMovement": true},
            {"id": "pocion", "name": "Pocion", "category": "pickup",
             "pickup": {"effect": "heal", "power": 15}},
            {"id": "slime", "name": "Slime", "category": "enemy",
             "combat": {"maxHealth": 10, "maxMana": 0, "skills": []}},
            {"id": "goblin", "name": "Goblin", "category": "enemy",
             "combat": {"maxHealth": 200, "maxMana": 0, "skills": []}}
        ]
    })");
    require(result.isOk());
    return catalog;
}

LevelDefinition makeLevel() {
    LevelDefinition level;
    level.name = "Prueba";
    level.mapPath = "assets/maps/test_map.tmx";
    level.playerStart = GridCoord{0, 0};
    // test_map.tmx es 4x3 con colision en (1,1) y (2,1) (gid 2).
    level.objects.push_back(
        ObjectSpawn{"arbusto", GridCoord{1, 0}, GridCoord{1, 0}, GridCoord{1, 0}});
    level.objects.push_back(
        ObjectSpawn{"pocion", GridCoord{0, 1}, GridCoord{0, 1}, GridCoord{0, 1}});
    level.objects.push_back(
        ObjectSpawn{"slime", GridCoord{0, 2}, GridCoord{0, 2}, GridCoord{0, 2}});
    return level;
}

void testExplorationCollisionAndPickup() {
    TileMap map;
    auto mapResult = map.loadFromFile("assets/maps/test_map.tmx");
    require(mapResult.isOk());
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;

    TestCombatant player(40);
    SkillSet playerSkills(5);
    std::vector<std::string> inventory;
    GameSession session(&map, &catalog, &skills, makeLevel(), &player, &playerSkills, &inventory);

    require(session.mode() == GameMode::Exploration);
    require(session.playerPosition().x == 0 && session.playerPosition().y == 0);
    // El nivel trae 3 objetos: 2 van al mundo (arbusto, pocion) y el
    // slime a la lista de enemigos (con su CombatData).
    require(session.worldObjects().size() == 2);
    require(session.enemies().size() == 1);
    require(session.enemies()[0].brain->maxHealth() == 10);  // del catalogo, no un default

    // Borde del mapa: bloquea.
    require(!session.tryMovePlayer(-1, 0));
    require(session.playerPosition().x == 0);

    // (1,0) tiene el arbusto (blocksMovement): bloquea.
    require(!session.tryMovePlayer(1, 0));
    require(session.playerPosition().x == 0 && session.playerPosition().y == 0);

    // (0,1) tiene la pocion: se entra Y se recoge.
    require(session.tryMovePlayer(0, 1));
    require(session.playerPosition().y == 1);
    require(inventory.size() == 1 && inventory[0] == "pocion");
    require(session.lastPickupId() == "pocion");
    require(session.worldObjects().size() == 1);  // la pocion ya no esta en el mundo
    require(!session.log().empty() && session.log().back().find("Recoges") != std::string::npos);

    // Tile con colision del TMX real: (1,1) es gid 2 (collision=true).
    require(map.getTile(0, 1, 1).hasCollision());
    require(!session.tryMovePlayer(1, 0));
    require(session.playerPosition().x == 0 && session.playerPosition().y == 1);

    std::cout << "[SESION] exploracion: colision de tiles/objetos/bordes y recogida correcta.\n";
}

void testEncounterAndVictory() {
    TileMap map;
    require(map.loadFromFile("assets/maps/test_map.tmx").isOk());
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;

    TestCombatant player(40);
    SkillSet playerSkills(5);
    std::vector<std::string> inventory;
    GameSession session(&map, &catalog, &skills, makeLevel(), &player, &playerSkills, &inventory);

    // Bajar dos veces: (0,1) recoge la pocion, (0,2) tiene el slime ->
    // encuentro (no se entra en la celda, pero cambia el modo).
    require(session.tryMovePlayer(0, 1));
    require(!session.tryMovePlayer(0, 1));        // false: no hubo movimiento...
    require(session.mode() == GameMode::Battle);  // ...pero SI encuentro
    require(session.playerPosition().y == 1);     // el jugador no entro en la celda
    require(session.battle() != nullptr);
    require(session.battle()->enemies().size() == 1);
    require(session.battle()->enemies()[0].name == "Slime");  // nombre del catalogo

    // En combate no se explora.
    require(!session.tryMovePlayer(0, -1));
    require(session.playerPosition().y == 1);

    // Slime de 10 PV: un ataque basico lo mata SI la tirada acierta. El
    // dano ya no es plano (pasa por DicePoolEngine), asi que se inyecta
    // el dado: un 5 = medio exito = SUCCESS = magnitud normal (10). Ver
    // la nota del dado en demo_battle.cpp.
    ScriptedRng dadoSlime({5});
    session.battle()->setRandomEngine(dadoSlime);
    session.battle()->resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});
    require(session.battle()->outcome() == BattleOutcome::Victory);

    // syncBattleOutcome(): aplica el desenlace al mundo.
    require(session.syncBattleOutcome());
    require(session.mode() == GameMode::Exploration);
    require(session.battle() == nullptr);
    require(session.enemies().empty());  // el slime desaparecio del mundo
    // El log del combate se conservo tras destruir el BattleState.
    bool foundAttack = false;
    bool foundVictory = false;
    for (const std::string& line : session.log()) {
        if (line.find("ataca") != std::string::npos)
            foundAttack = true;
        if (line.find("derrotado al Slime") != std::string::npos)
            foundVictory = true;
    }
    require(foundAttack && foundVictory);

    // Con el enemigo muerto, la celda (0,2) ya es transitable.
    require(session.tryMovePlayer(0, 1));
    require(session.playerPosition().y == 2);

    // Segundo sync sin combate: no-op.
    require(!session.syncBattleOutcome());

    std::cout << "[SESION] encuentro + victoria (enemigo eliminado, log conservado) correcto.\n";
}

void testFleeKeepsEnemyAndDefeatIsTerminal() {
    TileMap map;
    require(map.loadFromFile("assets/maps/test_map.tmx").isOk());
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;

    // --- Huida: el enemigo sigue en su celda ---
    {
        TestCombatant player(40);
        SkillSet playerSkills(5);
        std::vector<std::string> inventory;
        GameSession session(&map, &catalog, &skills, makeLevel(), &player, &playerSkills,
                            &inventory);
        session.tryMovePlayer(0, 1);
        session.tryMovePlayer(0, 1);  // encuentro
        require(session.mode() == GameMode::Battle);
        session.battle()->resolveAllyAction(0, BattleAction{BattleActionType::Flee, "", 0, ""});
        require(session.syncBattleOutcome());
        require(session.mode() == GameMode::Exploration);
        require(session.enemies().size() == 1);  // sigue vivo y en su sitio
        // Volver a chocar con el: nuevo combate.
        require(!session.tryMovePlayer(0, 1));
        require(session.mode() == GameMode::Battle);
    }

    // --- Derrota: GameOver terminal ---
    {
        LevelDefinition level = makeLevel();
        // Cambiar el slime por el goblin de 200 PV (imposible de matar a
        // 10 de dano por turno antes de que mate a un jugador de 15).
        level.objects.back().objectId = "goblin";
        TestCombatant player(15);
        SkillSet playerSkills(0);
        std::vector<std::string> inventory;
        GameSession session(&map, &catalog, &skills, level, &player, &playerSkills, &inventory);
        session.tryMovePlayer(0, 1);
        session.tryMovePlayer(0, 1);  // encuentro
        require(session.mode() == GameMode::Battle);
        // Dos turnos enemigos (10 de dano basico cada uno) matan al
        // jugador de 15 PV -- con el dado a 5 (acierto normal); si se
        // dejara al azar, dos pifias seguidas dejarian al jugador vivo y
        // este bloque fallaria sin que nada estuviese roto.
        ScriptedRng dadoGoblin({5});
        session.battle()->setRandomEngine(dadoGoblin);
        session.battle()->resolveEnemyTurn();
        session.battle()->resolveEnemyTurn();
        require(session.battle()->outcome() == BattleOutcome::Defeat);
        require(session.syncBattleOutcome());
        require(session.mode() == GameMode::GameOver);
        // Terminal: ni se explora ni se sale de ahi.
        require(!session.tryMovePlayer(0, -1));
        require(session.mode() == GameMode::GameOver);
    }

    std::cout << "[SESION] huida (enemigo intacto) y derrota (GameOver terminal) correctas.\n";
}

void testPlayerStateCarriesOver() {
    TileMap map;
    require(map.loadFromFile("assets/maps/test_map.tmx").isOk());
    ObjectCatalog catalog = makeCatalog();
    SkillCatalog skills;
    skills.add(Skill{"tajo", "Tajo", 3, 12, SkillEffect::Damage, SkillTarget::SingleEnemy});

    TestCombatant player(40);
    player.takeDamage(30);  // 10/40: hueco para la pocion
    SkillSet playerSkills(5);
    playerSkills.learn("tajo");
    std::vector<std::string> inventory;
    GameSession session(&map, &catalog, &skills, makeLevel(), &player, &playerSkills, &inventory);

    session.tryMovePlayer(0, 1);  // recoge la pocion
    require(inventory.size() == 1);
    session.tryMovePlayer(0, 1);  // encuentro con el slime
    require(session.mode() == GameMode::Battle);
    ScriptedRng dadoObjeto({5});  // acierto normal: el "tajo" hace sus 12
    session.battle()->setRandomEngine(dadoObjeto);

    // Usa la pocion EN COMBATE: consume del inventario real recogido en
    // exploracion (la cadena completa Fase 10 + Fase 12 + sesion).
    session.battle()->resolveAllyAction(0, BattleAction{BattleActionType::Item, "", 0, "pocion"});
    require(player.health() == 25);  // 10 + 15
    require(inventory.empty());      // consumida de verdad

    // Y una habilidad: gasta mana real del jugador.
    session.battle()->resolveAllyAction(0, BattleAction{BattleActionType::UseSkill, "tajo", 0, ""});
    require(playerSkills.mana() == 2);                               // 5 - 3
    require(session.battle()->outcome() == BattleOutcome::Victory);  // 12 de dano > 10 PV
    require(session.syncBattleOutcome());

    // De vuelta en exploracion, el estado del jugador es el que dejo el
    // combate (no se revierte nada).
    require(session.mode() == GameMode::Exploration);
    require(player.health() == 25);
    require(playerSkills.mana() == 2);

    std::cout << "[SESION] continuidad del jugador (vida/mana/inventario) entre modos correcta.\n";
}

}  // namespace

int main() {
    testExplorationCollisionAndPickup();
    testEncounterAndVictory();
    testFleeKeepsEnemyAndDefeatIsTerminal();
    testPlayerStateCarriesOver();
    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
