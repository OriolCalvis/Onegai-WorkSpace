// Fase 8 (motor_grafico_gantt_rpg.puml): combate por turnos. GL-free
// (igual que demo_skills.cpp): BattleState opera sobre ICombatant/
// SkillSet/SkillCatalog (Fase 7), nunca sobre Player/Enemy concretos, asi
// que se prueba contra un ICombatant de prueba (TestCombatant, mismo que
// demo_skills.cpp) sin necesitar TextureAtlas ni GL.
//
// Verifica:
//  - Ataque basico: dano fijo, log de texto.
//  - UseSkill: exito (maná se descuenta, efecto real via
//    ApplySkillEffect), y fallo controlado si no se puede pagar (no-op +
//    log, no crashea).
//  - resolveEnemyTurn(): IA minima -- usa la primera habilidad que puede
//    pagar (orden alfabetico, determinista), si no ataque basico; se
//    detiene si el combate termina a mitad de ronda.
//  - Desenlaces: Victory (todos los enemigos caen), Defeat (todos los
//    aliados caen), Fled (accion Flee) -- y que tras terminar, cualquier
//    accion posterior es no-op (no seguir mutando estado).
//  - Casos limite: indices de aliado/objetivo fuera de rango o ya
//    muertos -- no-op, no crashea.
#include "Game/BattleState.h"
#include "Render/ICombatant.h"
#include "Game/Skill.h"

#include "Check.h"
#include "ScriptedRng.h"

#include <algorithm>
#include <iostream>

// NOTA SOBRE EL DADO. El daño ya no es fijo: pasa por
// ApplyBasicAttackNd6/ApplySkillEffect -> SkillExecutor -> DicePoolEngine,
// asi que depende del GRADO de la tirada. Con un TestCombatant (stat 0,
// defensas por defecto) el pool es de 1d6 contra CD 0.0, y ahi:
//
//     dado 1-4 -> 0 exitos  -> BOTCH    -> magnitud 0     (no hace nada)
//     dado 5   -> medio     -> SUCCESS  -> magnitud normal
//     dado 6   -> 1 exito   -> CRITICAL -> magnitud x1.5
//
// Por eso cada batalla inyecta su ScriptedRng: sin el, estas
// comprobaciones estarian midiendo qué salio de la semilla por defecto y
// no la regla. El dado {5} es el caso "acierto normal", que es el que
// reproduce el daño plano historico (10 el ataque basico, skill.power la
// habilidad) y deja los numeros de siempre.

namespace {

// Mismo ICombatant de prueba que demo_skills.cpp (ver su comentario):
// contrato identico al de Player/Enemy, sin arrastrar TextureAtlas/GL a
// un demo que no los necesita.
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

void testBasicAttackAndSkill() {
    SkillCatalog catalog;
    catalog.add(Skill{"tajo", "Tajo", 3, 12, SkillEffect::Damage, SkillTarget::SingleEnemy});

    TestCombatant playerBody(30);
    TestCombatant slimeBody(20);
    SkillSet playerSkills(/*maxMana=*/3);
    playerSkills.learn("tajo");

    std::vector<BattleParticipant> allies{{"Heroe", &playerBody, &playerSkills}};
    std::vector<BattleParticipant> enemies{{"Slime", &slimeBody, nullptr}};
    BattleState battle(std::move(allies), std::move(enemies), &catalog);
    ScriptedRng dado({5});  // acierto normal en todas las tiradas
    battle.setRandomEngine(dado);
    require(battle.outcome() == BattleOutcome::InProgress);

    // Ataque basico: magnitud de SUCCESS = 10.
    battle.resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});
    require(slimeBody.health() == 10);
    require(battle.log().size() == 1);
    require(battle.log().back().find("ataca") != std::string::npos);

    // UseSkill sin mana suficiente (cuesta 3, tiene 3... primero
    // agotamos el mana a mano para probar el camino de fallo).
    playerSkills.spend(Skill{"", "", 3, 0, SkillEffect::Damage, SkillTarget::Self});  // deja mana=0
    require(playerSkills.mana() == 0);
    battle.resolveAllyAction(0, BattleAction{BattleActionType::UseSkill, "tajo", 0, ""});
    require(slimeBody.health() == 10);  // sin cambio: la accion fallo
    require(battle.log().back().find("no puede usar") != std::string::npos);

    // UseSkill con mana de sobra: dano real de la Skill (12), no el
    // ataque basico (10).
    playerSkills.restoreMana(3);
    battle.resolveAllyAction(0, BattleAction{BattleActionType::UseSkill, "tajo", 0, ""});
    require(slimeBody.health() == 0);  // 10 - 12 clampado a 0
    require(playerSkills.mana() == 0);
    require(battle.outcome() == BattleOutcome::Victory);  // el unico enemigo murio

    std::cout << "[BATTLE] ataque basico + UseSkill (exito/fallo) + Victory correctos.\n";
}

void testEnemyAiAndDefeat() {
    SkillCatalog catalog;
    catalog.add(Skill{"mordisco", "Mordisco venenoso", 2, 8, SkillEffect::Damage,
                      SkillTarget::SingleEnemy});

    TestCombatant playerBody(15);
    TestCombatant goblinBody(25);
    SkillSet goblinSkills(/*maxMana=*/2);
    goblinSkills.learn("mordisco");

    std::vector<BattleParticipant> allies{{"Heroe", &playerBody, nullptr}};
    std::vector<BattleParticipant> enemies{{"Goblin", &goblinBody, &goblinSkills}};
    BattleState battle(std::move(allies), std::move(enemies), &catalog);
    ScriptedRng dado({5});  // acierto normal en todas las tiradas
    battle.setRandomEngine(dado);

    // Turno 1: el goblin puede pagar "mordisco" (2 mana) -> lo usa (8 de
    // dano), no ataque basico (10).
    battle.resolveEnemyTurn();
    require(playerBody.health() == 7);  // 15 - 8
    require(goblinSkills.mana() == 0);
    require(battle.log().back().find("Mordisco venenoso") != std::string::npos);

    // Turno 2: sin mana, ataque basico (10 de dano) -> el jugador (7 HP)
    // muere.
    battle.resolveEnemyTurn();
    require(playerBody.health() == 0);
    require(!playerBody.isAlive());
    require(battle.outcome() == BattleOutcome::Defeat);

    // Tras Defeat, cualquier accion es no-op: ni el log crece ni el
    // estado cambia.
    std::size_t logSizeBefore = battle.log().size();
    battle.resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});
    battle.resolveEnemyTurn();
    require(battle.log().size() == logSizeBefore);
    require(battle.outcome() == BattleOutcome::Defeat);  // no lo pisa nada despues

    std::cout << "[BATTLE] IA enemiga (habilidad si puede pagarla, si no ataque basico) y "
                 "Defeat correctos.\n";
}

void testFleeAndInvalidIndices() {
    SkillCatalog catalog;
    TestCombatant playerBody(20);
    TestCombatant enemyBody(20);

    std::vector<BattleParticipant> allies{{"Heroe", &playerBody, nullptr}};
    std::vector<BattleParticipant> enemies{{"Rata", &enemyBody, nullptr}};
    BattleState battle(std::move(allies), std::move(enemies), &catalog);

    // Indices fuera de rango / objetivo invalido: no-op, no crashea.
    battle.resolveAllyAction(
        5, BattleAction{BattleActionType::Attack, "", 0, ""});  // allyIndex invalido
    battle.resolveAllyAction(
        0, BattleAction{BattleActionType::Attack, "", 9, ""});  // targetIndex invalido
    require(battle.log().empty());
    require(enemyBody.health() == 20);

    // Flee: termina el combate sin matar a nadie.
    battle.resolveAllyAction(0, BattleAction{BattleActionType::Flee, "", 0, ""});
    require(battle.outcome() == BattleOutcome::Fled);
    require(battle.log().size() == 1);
    require(battle.log().back().find("huye") != std::string::npos);

    // Tras Fled, no-op tambien (mismo criterio que Defeat/Victory).
    battle.resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});
    require(battle.log().size() == 1);
    require(enemyBody.health() == 20);

    std::cout << "[BATTLE] Flee e indices invalidos (no-op, sin crash) correctos.\n";
}

// El grado de la tirada decide el dano: la misma accion, con el mismo
// atacante y el mismo objetivo, quita 0, lo normal o x1.5 segun lo que
// salga. Es la diferencia entre el combate de antes (dano plano) y el
// Nd6, y no habia ni una comprobacion que lo fijara -- por eso el fallo
// de resolve_against_cd, que anulaba justo la rama de la pifia, no lo
// detecto nadie.
void testElGradoDecideElDano() {
    struct Caso { int cara; int esperado; const char* etiqueta; };
    // Ataque basico: magnitude_by_degree = {0, 5, 10, 15}. Con 1d6 contra
    // CD 0.0, PARTIAL no es alcanzable (harian falta exitos > 0 y < 0).
    //
    // El 6 da 22 y no 15 porque se acumulan DOS cosas: la magnitud de
    // CRITICAL (15) y ademas el extra de "todos los dados a 6" (x1.5) del
    // PASO 8 de SkillExecutor -> 15 * 1.5 = 22 (truncado). Con un solo
    // dado ambas condiciones son la MISMA tirada, asi que en Tier I el
    // critico del ataque basico siempre lleva el x1.5; con pools grandes
    // el all-sixes es raro y el critico normal se queda en 15. Queda
    // fijado aqui para que se vea: si algun dia se decide que no deben
    // acumularse, este test lo dira en vez de cambiar el dano en silencio.
    const Caso casos[] = {
        {1,  0, "pifia (1)"},
        {4,  0, "pifia (4)"},
        {5, 10, "acierto (5)"},
        {6, 22, "critico + all-sixes (6)"},
    };

    for (const Caso& c : casos) {
        SkillCatalog catalog;
        TestCombatant heroe(30);
        TestCombatant diana(100);   // sobra vida: nunca se recorta a 0
        std::vector<BattleParticipant> allies{{"Heroe", &heroe, nullptr}};
        std::vector<BattleParticipant> enemies{{"Diana", &diana, nullptr}};
        BattleState battle(std::move(allies), std::move(enemies), &catalog);

        ScriptedRng dado({c.cara});
        battle.setRandomEngine(dado);
        battle.resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});

        require(dado.consumed() == 1);              // 1d6: stat 0 -> 1 dado
        require(100 - diana.health() == c.esperado);
        require(diana.isAlive());
    }

    std::cout << "[BATTLE] el grado de la tirada decide el dano (0 / 10 / 15) correcto.\n";
}

}  // namespace

int main() {
    testBasicAttackAndSkill();
    testEnemyAiAndDefeat();
    testFleeAndInvalidIndices();
    testElGradoDecideElDano();
    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
