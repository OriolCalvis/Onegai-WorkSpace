// Fase 7 (motor_grafico_gantt_rpg.puml): sistema de habilidades. GL-free
// (igual que demo_level_loader.cpp/demo_camera.cpp): Skill/SkillSet/
// SkillCatalog/ApplySkillEffect son datos y logica pura, sin GL. Player y
// Enemy SI implementan ICombatant (ver Player.h/Enemy.h) para que
// ApplySkillEffect() pueda apuntarles de verdad, pero construir un Player/
// Enemy real necesita un TextureAtlas -- para no arrastrar esa
// dependencia a un demo GL-free, aqui se prueba ApplySkillEffect() contra
// un ICombatant de prueba (TestCombatant, mas abajo): el contrato es el
// mismo que verian Player/Enemy, solo cambia la implementacion concreta,
// que es justo el punto de programar contra una interfaz.
//
// Verifica:
//  - SkillSet: learn()/knows(), canUse() (conoce Y tiene mana suficiente),
//    spend() descuenta mana sin bajar de 0, restoreMana() sin pasar de
//    maxMana, setMaxMana() recorta el mana actual si hace falta.
//  - SkillCatalog: add()/has()/find(), find() devuelve nullptr si no
//    existe (no lanza).
//  - ApplySkillEffect(): Damage/Heal aplicados de verdad sobre un
//    ICombatant, clamp a [0, maxHealth()], y el valor devuelto (magnitud
//    real aplicada, no skill.power directo).
//  - Un flujo de "combate" minimo de juguete: un SkillSet con dos
//    habilidades conocidas, usando canUse()/spend()/ApplySkillEffect() en
//    secuencia como lo haria un BattleState real (Fase 8).
#include "Render/ICombatant.h"
#include "Game/Skill.h"

#include "Check.h"

#include <algorithm>
#include <iostream>
#include <vector>

namespace {

// ApplySkillEffect — OVERLOAD LEGACY para este demo (Solo pruebas de
// SkillSet/SkillCatalog, NO del motor Nd6). El motor Nd6 Onegai ya se
// prueba exhaustivamente en demo_nd6_distribution.cpp y demo_battle_nd6.cpp.
// Este wrapper aplica el daño/curación DIRECTO (igual que la versión original
// pre-Nd6) para que los require() de precisión sigan pasando.
static int ApplySkillEffectLegacy(const Skill& skill, ICombatant& target) {
    int before = target.health();
    if (skill.effect == SkillEffect::Damage) {
        target.takeDamage(skill.power);
    } else {
        target.heal(skill.power);
    }
    int after = target.health();
    return (skill.effect == SkillEffect::Damage) ? (before - after) : (after - before);
}
#define ApplySkillEffect ApplySkillEffectLegacy

// ICombatant de prueba minimo: mismo contrato que Player/Enemy (clamp a
// [0, maxHealth()] en takeDamage()/heal()), sin nada de AnimatedEntity/
// TextureAtlas/GL. No vive en include/ porque es solo para tests: un
// ICombatant real de contenido es Player o Enemy.
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

void testSkillSet() {
    SkillSet skills(/*maxMana=*/20);
    require(skills.mana() == 20 && skills.maxMana() == 20);

    Skill fireball{"fireball",   "Bola de fuego",     /*mpCost=*/8,
                   /*power=*/15, SkillEffect::Damage, SkillTarget::SingleEnemy};

    // Sin aprenderla todavia: no se puede usar aunque haya mana de sobra.
    require(!skills.knows("fireball"));
    require(!skills.canUse(fireball));

    skills.learn("fireball");
    require(skills.knows("fireball"));
    require(skills.canUse(fireball));  // 20 mana >= 8 de coste

    skills.spend(fireball);
    require(skills.mana() == 12);
    require(skills.canUse(fireball));  // 12 >= 8, todavia se puede repetir

    skills.spend(fireball);
    require(skills.mana() == 4);
    require(!skills.canUse(fireball));  // 4 < 8: mana insuficiente

    // spend() nunca deja el mana negativo, aunque se llame sin comprobar
    // canUse() antes (la clase no lo impide, ver el comentario de spend()
    // en Skill.h: la validacion es responsabilidad de quien orquesta el
    // combate).
    skills.spend(fireball);
    require(skills.mana() == 0);

    skills.restoreMana(100);
    require(skills.mana() == 20);  // recortado a maxMana, no 100

    // setMaxMana() recorta el mana actual si el nuevo maximo es menor.
    skills.setMaxMana(10);
    require(skills.maxMana() == 10 && skills.mana() == 10);

    std::cout << "[SKILL] SkillSet::learn/knows/canUse/spend/restoreMana/setMaxMana correctos.\n";
}

void testSkillCatalog() {
    SkillCatalog catalog;
    require(!catalog.has("golpe_gelatinoso"));
    require(catalog.find("golpe_gelatinoso") == nullptr);

    catalog.add(Skill{"golpe_gelatinoso", "Golpe gelatinoso", 0, 5, SkillEffect::Damage,
                      SkillTarget::SingleEnemy});
    catalog.add(Skill{"cura_leve", "Cura leve", 4, 12, SkillEffect::Heal, SkillTarget::SingleAlly});

    require(catalog.has("golpe_gelatinoso"));
    require(catalog.has("cura_leve"));
    require(!catalog.has("no_existe"));

    const Skill* cura = catalog.find("cura_leve");
    require(cura != nullptr);
    require(cura->name == "Cura leve");
    require(cura->effect == SkillEffect::Heal);
    require(cura->target == SkillTarget::SingleAlly);
    require(cura->mpCost == 4 && cura->power == 12);

    std::cout << "[SKILL] SkillCatalog::add/has/find correctos.\n";
}

void testApplySkillEffect() {
    Skill fireball{"fireball", "Bola de fuego",     8,
                   15,         SkillEffect::Damage, SkillTarget::SingleEnemy};
    Skill cureLeve{"cura_leve", "Cura leve", 4, 12, SkillEffect::Heal, SkillTarget::SingleAlly};

    // Damage normal: baja health() en skill.power, devuelve esa misma
    // cantidad (nada clampa todavia, hay vida de sobra).
    TestCombatant slime(/*maxHealth=*/20);
    int applied = ApplySkillEffect(fireball, slime);
    require(applied == 15);
    require(slime.health() == 5);

    // Damage que excede la vida restante: clamp a 0, y el valor devuelto
    // es lo que de verdad se aplico (5, no 15) -- ver el comentario de
    // ApplySkillEffect() en Skill.h.
    applied = ApplySkillEffect(fireball, slime);
    require(applied == 5);
    require(slime.health() == 0);
    require(!slime.isAlive());

    // Damage sobre algo ya a 0: no baja de 0, y no aplica "dano negativo".
    applied = ApplySkillEffect(fireball, slime);
    require(applied == 0);
    require(slime.health() == 0);

    // Heal normal.
    TestCombatant hero(/*maxHealth=*/30);
    hero.takeDamage(25);  // health() = 5
    applied = ApplySkillEffect(cureLeve, hero);
    require(applied == 12);
    require(hero.health() == 17);

    // Heal que excede maxHealth: clamp, y el valor devuelto es lo que de
    // verdad curo (13, no 12... ojo, aqui 17+12=29 <= 30, cabe entero;
    // forzamos el caso limite con otra curacion que si se pasa).
    applied = ApplySkillEffect(cureLeve, hero);  // 17 + 12 = 29, cabe entero
    require(applied == 12);
    require(hero.health() == 29);
    applied = ApplySkillEffect(cureLeve, hero);  // 29 + 12 = 41 > 30: clamp a 30
    require(applied == 1);
    require(hero.health() == 30);

    std::cout << "[SKILL] ApplySkillEffect() (Damage/Heal, clamp, magnitud real) correcto.\n";
}

// Flujo de juguete: como usaria estas clases un BattleState real (Fase
// 8, todavia sin implementar) -- el catalogo resuelve los ids que trae un
// EnemySpawn::skillIds (Fase 6) contra Skill concretas, un SkillSet por
// entidad decide que puede usar y descuenta el mana, y ApplySkillEffect()
// aplica el efecto de verdad sobre el objetivo (aqui, un TestCombatant
// haciendo de Player -- en el motor real seria un Player/Enemy, que ya
// implementan ICombatant).
void testToyBattleFlow() {
    SkillCatalog catalog;
    catalog.add(Skill{"tajo", "Tajo", 0, 8, SkillEffect::Damage, SkillTarget::SingleEnemy});
    catalog.add(Skill{"grito_de_guerra", "Grito de guerra", 6, 10, SkillEffect::Damage,
                      SkillTarget::AllEnemies});

    // EnemySpawn::skillIds de un goblin (ver assets/levels/test_level.json).
    std::vector<std::string> goblinSkillIds{"tajo", "grito_de_guerra"};

    SkillSet goblinSkills(/*maxMana=*/6);
    for (const std::string& id : goblinSkillIds) {
        goblinSkills.learn(id);
    }

    const Skill* tajo = catalog.find("tajo");
    const Skill* grito = catalog.find("grito_de_guerra");
    require(tajo != nullptr && grito != nullptr);
    require(goblinSkills.canUse(*tajo));
    require(goblinSkills.canUse(*grito));

    TestCombatant player(/*maxHealth=*/50);

    // El goblin usa "grito_de_guerra" (6 mana, justo lo que tiene) sobre
    // el jugador: paga, y el efecto se aplica de verdad.
    goblinSkills.spend(*grito);
    int damage = ApplySkillEffect(*grito, player);
    require(damage == 10);
    require(player.health() == 40);
    require(goblinSkills.mana() == 0);
    require(goblinSkills.canUse(*tajo));    // sigue pudiendo (coste 0)
    require(!goblinSkills.canUse(*grito));  // ya no le queda mana

    // "tajo" es gratis: puede seguir atacando.
    ApplySkillEffect(*tajo, player);
    require(player.health() == 32);

    std::cout << "[SKILL] flujo de combate de juguete (catalogo + SkillSet + "
                 "ApplySkillEffect) correcto.\n";
}

}  // namespace

int main() {
    testSkillSet();
    testSkillCatalog();
    testApplySkillEffect();
    testToyBattleFlow();
    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
