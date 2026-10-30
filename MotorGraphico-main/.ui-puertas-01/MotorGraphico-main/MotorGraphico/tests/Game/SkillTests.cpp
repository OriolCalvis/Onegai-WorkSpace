// C-24 — Xarxa de seguretat al camí que es juga.
//
// Prova la resolució de combat Nd6 que viu a Skill.cpp i que mai no havia
// tingut un test propi: `ApplySkillEffect` i `ApplyBasicAttackNd6` baixaven
// només des de BattleState (que prova rol, no la tirada), així que el grau
// BOTCH/PARTIAL/SUCCESS/CRITICAL i les magnituds que sí entren en joc no
// estaven fixats per cap cas.
//
// Els valors esperats es deriven de les regles exactes de DicePoolEngine
// (6 -> 1 èxit, 5 -> mig, 1-4 -> res; pifia = cap 5/6 i mínim dos 1s;
// crític = dos 6 o més, D0013) i de opposed_degree (defensor guanya -> BOTCH, empat ->
// PARTIAL, atacant guanya -> SUCCESS, +dos 6 -> CRITICAL). El dany ho
// decideix la tirada, no la semilla: ScriptedRng escriu la cara a mà.

#include "TestHarness.h"

#include "Game/Skill.h"
#include "Render/ICombatant.h"
#include "ScriptedRng.h"

#include <algorithm>
#include <array>
#include <string>
#include <type_traits>

namespace {

// Combatient amb stats configurables, per controlar les piles de dados
// d'atacant i defensor per separat (pool = 1 + stat).
class StatCombatant : public ICombatant {
public:
    StatCombatant(int maxHealth, int health, std::array<int, 4> stats)
        : m_max(maxHealth), m_hp(health), m_stats(stats) {}

    void takeDamage(int amount) override { m_hp = std::max(0, m_hp - amount); }
    void heal(int amount) override { m_hp = std::min(m_max, m_hp + amount); }
    int health() const override { return m_hp; }
    int maxHealth() const override { return m_max; }
    bool isAlive() const override { return m_hp > 0; }

    int stat(RPG::Stat s) const override { return m_stats[static_cast<int>(s)]; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "combatent"; }

private:
    int m_max;
    int m_hp;
    std::array<int, 4> m_stats;
};

StatCombatant combatentZero(int hp = 50) {
    return StatCombatant(50, hp, {0, 0, 0, 0});  // CON DES INT CAR
}

}  // namespace

MOTOR_TEST_CASE(skilltarget_de_game_es_el_tipo_canonico_de_rpg) {
    static_assert(std::is_same<SkillTarget, RPG::SkillTarget>::value,
                  "Game no puede declarar otro SkillTarget");

    MOTOR_CHECK(SkillTarget::Self == RPG::SkillTarget::SELF);
    MOTOR_CHECK(SkillTarget::SingleEnemy == RPG::SkillTarget::SINGLE_ENEMY);
    MOTOR_CHECK(SkillTarget::AllEnemies == RPG::SkillTarget::ALL_ENEMIES);
    MOTOR_CHECK(SkillTarget::SingleAlly == RPG::SkillTarget::SINGLE_ALLY);
    MOTOR_CHECK(SkillTarget::AllAllies == RPG::SkillTarget::ALL_ALLIES);
    MOTOR_CHECK(SkillTarget::ZoneRadius == RPG::SkillTarget::ZONE_RADIUS);

    Skill legacy;
    legacy.target = SkillTarget::SingleAlly;
    MOTOR_CHECK(legacy.target == RPG::SkillTarget::SINGLE_ALLY);
}

MOTOR_TEST_CASE(el_ataque_basico_traduce_la_cara_del_dado_al_grado) {
    // Casting DES=0 -> 1 die; objectiu CON=DES=0 -> 0 daus de defensa.
    StatCombatant caster = combatentZero();
    StatCombatant target = combatentZero();
    // 6 amb un sol dau -> 1 èxit, no crític (el crític demana dos 6:
    // D0013) -> SUCCESS -> mag 10.
    ScriptedRng dado({6});
    auto sis = ApplyBasicAttackNd6(caster, target, dado);
    MOTOR_CHECK(sis.degree == 2);
    MOTOR_CHECK(sis.hpChange == 10);
    MOTOR_CHECK(target.health() == 50 - 10);

    // DES=1 -> 2 daus: 6, 6 -> dos 6 -> CRITICAL; mag 15 * 1.5 (extra) = 22.
    StatCombatant destre(50, 50, {0, 1, 0, 0});  // CON DES INT CAR
    StatCombatant target2 = combatentZero();
    ScriptedRng dosSisos({6, 6});
    auto crit = ApplyBasicAttackNd6(destre, target2, dosSisos);
    MOTOR_CHECK(crit.degree == 3);
    MOTOR_CHECK(crit.hpChange == 22);
    MOTOR_CHECK(target2.health() == 50 - 22);

    ScriptedRng dado5({5});  // 0.5 èxit -> SUCCESS -> mag 10
    auto ok = ApplyBasicAttackNd6(caster, target, dado5);
    MOTOR_CHECK(ok.degree == 2);
    MOTOR_CHECK(ok.hpChange == 10);

    ScriptedRng dado4({4});  // 0 èxits -> empat -> PARTIAL -> mag 5
    auto parcial = ApplyBasicAttackNd6(caster, target, dado4);
    MOTOR_CHECK(parcial.degree == 1);
    MOTOR_CHECK(parcial.hpChange == 5);
}

MOTOR_TEST_CASE(pifia_con_dos_unos_no_deja_penetrar) {
    // DES=1 -> 2 daus. Dos 1s, cap 5/6 -> pifia -> BOTCH -> mag 0.
    StatCombatant caster(50, 50, {0, 1, 0, 0});  // CON DES INT CAR
    StatCombatant target = combatentZero();
    ScriptedRng dado({1, 1});
    auto res = ApplyBasicAttackNd6(caster, target, dado);
    MOTOR_CHECK(res.degree == 0);
    MOTOR_CHECK(res.hpChange == 0);
    MOTOR_CHECK(target.health() == 50);
}

MOTOR_TEST_CASE(si_la_defensa_gana_la_enfrentada_es_botch) {
    // CON=1 al defensor -> 1 dau de defensa. Atac {1} vs defensa {6}:
    // el defensor supera -> BOTCH -> 0 de dany.
    StatCombatant caster = combatentZero();
    StatCombatant target(50, 50, {1, 0, 0, 0});  // CON DES INT CAR
    ScriptedRng dado({1, 6});
    auto res = ApplyBasicAttackNd6(caster, target, dado);
    MOTOR_CHECK(res.degree == 0);
    MOTOR_CHECK(res.hpChange == 0);
    MOTOR_CHECK(target.health() == 50);
}

MOTOR_TEST_CASE(curar_no_tiene_rival_y_las_magnitudes_son_las_de_la_carta) {
    Skill cura;
    cura.id = "cura";
    cura.power = 10;
    cura.effect = SkillEffect::Heal;
    cura.target = SkillTarget::SingleAlly;

    StatCombatant caste(50, 50, {0, 0, 0, 0});  // INT=0 -> 1 dau
    StatCombatant aliada(50, 28, {0, 0, 0, 0});

    // 5 -> mig èxit, sense defensor -> SUCCESS -> potencia 10.
    ScriptedRng d5({5});
    auto ok = ApplySkillEffect(cura, caste, aliada, d5);
    MOTOR_CHECK(ok.degree == 2);
    MOTOR_CHECK(ok.hpChange == 10);
    MOTOR_CHECK(aliada.health() == 38);

    // 4 -> 0 èxits -> PARTIAL -> max(1, 10/2) = 5.
    ScriptedRng d4({4});
    auto parcial = ApplySkillEffect(cura, caste, aliada, d4);
    MOTOR_CHECK(parcial.degree == 1);
    MOTOR_CHECK(parcial.hpChange == 5);
    MOTOR_CHECK(aliada.health() == 43);

    // 6, 6 (INT=1 -> 2 daus) -> dos 6 -> CRITICAL: mag int(10*1.5)=15 *
    // 1.5 (extra) = 22. Combatent fresc a 28: 28 + 22 = 50 (clamp al màxim).
    StatCombatant savia(50, 50, {1, 1, 1, 1});
    StatCombatant aliada2(50, 28, {0, 0, 0, 0});
    ScriptedRng d6({6, 6});
    auto critic = ApplySkillEffect(cura, savia, aliada2, d6);
    MOTOR_CHECK(critic.degree == 3);
    MOTOR_CHECK(critic.hpChange == 22);
    MOTOR_CHECK(aliada2.health() == 50);
}

MOTOR_TEST_CASE(la_habilidad_de_dano_parte_su_magnitud_por_grado) {
    Skill flama;
    flama.id = "flama";
    flama.power = 10;
    flama.effect = SkillEffect::Damage;
    flama.target = SkillTarget::SingleEnemy;

    StatCombatant caster = combatentZero();
    StatCombatant target = combatentZero();

    ScriptedRng d5({5});  // SUCCESS -> potència 10
    auto ok = ApplySkillEffect(flama, caster, target, d5);
    MOTOR_CHECK(ok.degree == 2);
    MOTOR_CHECK(ok.hpChange == 10);

    ScriptedRng d4({4});  // PARTIAL -> max(1, 5) = 5
    auto parcial = ApplySkillEffect(flama, caster, target, d4);
    MOTOR_CHECK(parcial.degree == 1);
    MOTOR_CHECK(parcial.hpChange == 5);
    MOTOR_CHECK(target.health() == 50 - 15);

    ScriptedRng d6({6});  // un sol dau: 6 -> SUCCESS, no crític -> 10
    auto sis = ApplySkillEffect(flama, caster, target, d6);
    MOTOR_CHECK(sis.degree == 2);
    MOTOR_CHECK(sis.hpChange == 10);

    StatCombatant destre(50, 50, {1, 1, 1, 1});  // 2 daus
    ScriptedRng d66({6, 6});  // CRITICAL + dos 6 -> int(15*1.5)=22
    auto critic = ApplySkillEffect(flama, destre, target, d66);
    MOTOR_CHECK(critic.degree == 3);
    MOTOR_CHECK(critic.hpChange == 22);
}

MOTOR_TEST_CASE(la_pifia_de_una_habilidad_es_botch) {
    Skill flama;
    flama.id = "flama";
    flama.power = 10;
    flama.effect = SkillEffect::Damage;

    StatCombatant caster(50, 50, {0, 1, 0, 0});  // DES=1 -> 2 daus
    StatCombatant target = combatentZero();
    ScriptedRng dado({1, 1});
    auto res = ApplySkillEffect(flama, caster, target, dado);
    MOTOR_CHECK(res.degree == 0);
    MOTOR_CHECK(res.hpChange == 0);
    MOTOR_CHECK(target.health() == 50);
}

MOTOR_TEST_CASE(power_negativo_se_clampa_a_cero) {
    Skill roto;
    roto.id = "roto";
    roto.power = -5;
    roto.effect = SkillEffect::Damage;

    StatCombatant caster = combatentZero();
    StatCombatant target = combatentZero();
    ScriptedRng d5({5});  // SUCCESS -> mag_success 0
    auto res = ApplySkillEffect(roto, caster, target, d5);
    MOTOR_CHECK(res.degree == 2);
    MOTOR_CHECK(res.hpChange == 0);
    MOTOR_CHECK(target.health() == 50);
}

MOTOR_TEST_CASE(la_condicion_pendiente_viaja_solo_si_el_grado_llega) {
    Skill golpe;
    golpe.id = "golpe";
    golpe.power = 8;
    golpe.effect = SkillEffect::Damage;
    golpe.applyConditionId = "sangrado";
    golpe.applyConditionMinDegree = 2;  // SUCCESS
    golpe.conditionDurationRounds = 3;

    StatCombatant caster = combatentZero();
    StatCombatant target = combatentZero();

    ScriptedRng d5({5});  // SUCCESS >= 2 -> condició i 3 rondes
    auto ok = ApplySkillEffect(golpe, caster, target, d5);
    MOTOR_CHECK(ok.pendingConditionId == "sangrado");
    MOTOR_CHECK(ok.pendingConditionRounds == 3);

    ScriptedRng d4({4});  // PARTIAL (1) < 2 -> cap condició
    auto parcial = ApplySkillEffect(golpe, caster, target, d4);
    MOTOR_CHECK(parcial.pendingConditionId.empty());
}

MOTOR_TEST_CASE(grau_minim_fora_de_rang_es_clampa_i_rotacio_de_rondes) {
    Skill golpe;
    golpe.id = "golpe";
    golpe.power = 8;
    golpe.effect = SkillEffect::Damage;
    golpe.applyConditionId = "sangrado";
    golpe.applyConditionMinDegree = 5;  // > 3 -> CRITICAL
    golpe.conditionDurationRounds = 0;  // l'executor el torna 1

    StatCombatant caster(50, 50, {1, 1, 1, 1});  // 2 daus
    StatCombatant target = combatentZero();
    ScriptedRng d6({6, 6});  // CRITICAL (3) >= CRITICAL(3) -> condició, 1 roda
    auto critic = ApplySkillEffect(golpe, caster, target, d6);
    MOTOR_CHECK(critic.pendingConditionId == "sangrado");
    MOTOR_CHECK(critic.pendingConditionRounds == 1);
}

MOTOR_TEST_CASE(skillset_aprende_no_duplica_y_lista_ordenada) {
    SkillSet set(10);
    set.learn("b");
    set.learn("a");
    set.learn("c");
    set.learn("b");  // repetida: no duplica

    MOTOR_CHECK(set.knows("a"));
    MOTOR_CHECK(set.knows("b"));
    MOTOR_CHECK(!set.knows("z"));
    const auto ids = set.knownSkillIds();
    MOTOR_CHECK(ids.size() == 3);
    MOTOR_CHECK(ids[0] == "a");
    MOTOR_CHECK(ids[1] == "b");
    MOTOR_CHECK(ids[2] == "c");
}

MOTOR_TEST_CASE(can_use_exige_conocer_y_tener_mana) {
    SkillSet set(10);
    set.learn("golpe");
    Skill golpe;
    golpe.id = "golpe";
    golpe.mpCost = 4;

    MOTOR_CHECK(set.canUse(golpe));          // coneix i mana suficient
    SkillSet set2(3);
    set2.learn("golpe");
    MOTOR_CHECK(!set2.canUse(golpe));        // mana insuficient
    SkillSet set3(10);
    MOTOR_CHECK(!set3.canUse(golpe));        // no la coneix
}

MOTOR_TEST_CASE(mana_se_gasta_se_restaura_y_se_recorta) {
    SkillSet set(10);
    Skill golpe;
    golpe.id = "golpe";
    golpe.mpCost = 4;

    set.spend(golpe);
    MOTOR_CHECK(set.mana() == 6);
    set.spend(golpe);
    set.spend(golpe);
    MOTOR_CHECK(set.mana() == 0);  // mai negatiu

    set.restoreMana(5);
    MOTOR_CHECK(set.mana() == 5);
    set.restoreMana(100);
    MOTOR_CHECK(set.mana() == 10);  // clampa al màxim

    set.setMaxMana(3);  // per sota de l'actual: es recorta
    MOTOR_CHECK(set.mana() == 3);
    MOTOR_CHECK(set.maxMana() == 3);
}

MOTOR_TEST_CASE(skillcatalog_guarda_busca_y_elimina_referencia_a_nadie) {
    SkillCatalog catalog;
    Skill a;
    a.id = "a";
    Skill b;
    b.id = "b";
    catalog.add(a);
    catalog.add(b);

    MOTOR_CHECK(catalog.size() == 2);
    MOTOR_CHECK(catalog.has("a"));
    MOTOR_CHECK(catalog.has("b"));
    MOTOR_CHECK(!catalog.has("c"));

    const Skill* pa = catalog.find("a");
    MOTOR_CHECK(pa != nullptr);
    MOTOR_CHECK(pa->id == "a");
    MOTOR_CHECK(catalog.find("c") == nullptr);
}

int main() { return motor_test::runAllTests(); }
