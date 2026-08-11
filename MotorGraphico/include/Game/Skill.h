#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Resources/ICatalog.h"

#include "RPG/RandomEngine.h"

class ICombatant;

// Habilidad de combate (motor_grafico_gantt_rpg.puml, Fase 7 "Sistema de
// habilidades"): datos puros, sin logica de motor (sin GL, sin Entity).
// "Que hace" una habilidad al usarse de verdad (aplicar dano/curacion a
// un objetivo concreto en un combate) es responsabilidad de quien la use
// (el futuro BattleState de la Fase 8), no de Skill: mismo principio de
// separacion que LevelDefinition/EnemySpawn (datos) frente a Enemy
// (entidad que se dibuja y actualiza).
enum class SkillTarget { Self, SingleEnemy, SingleAlly, AllEnemies, AllAllies };
enum class SkillEffect { Damage, Heal };

struct Skill {
    std::string id;    // clave estable (ver EnemySpawn::skillIds / SkillSet::learn)
    std::string name;  // nombre para mostrar en el HUD (menu de comandos, Fase 9)
    int mpCost = 0;
    // Cantidad base de dano/curacion; el sistema de combate (Fase 8)
    // decide como escalarla (stats del que la usa, resistencias del
    // objetivo...) -- Skill no sabe nada de eso, solo guarda el dato base.
    int power = 0;
    SkillEffect effect = SkillEffect::Damage;
    SkillTarget target = SkillTarget::SingleEnemy;
};

// Coleccion de habilidades conocidas por una entidad (Player/Enemy) mas su
// reserva de mana. No posee las Skill (solo referencias por id a un
// catalogo externo, ver SkillCatalog mas abajo): igual que EnemySpawn no
// duplica los stats del tipo de enemigo, SkillSet no duplica los datos de
// cada Skill, solo sabe cuales conoce esta entidad concreta y cuanto mana
// le queda.
class SkillSet {
public:
    explicit SkillSet(int maxMana = 0);

    void learn(const std::string& skillId);
    bool knows(const std::string& skillId) const;
    // Copia ordenada (alfabeticamente) de los ids conocidos -- NO una
    // referencia al mapa interno (m_known es un detalle de
    // implementacion, esto es la interfaz estable). Ordenada a proposito:
    // unordered_map no garantiza ningun orden de iteracion, y la IA de
    // combate (BattleState::resolveEnemyTurn(), Fase 8) recorre esto para
    // elegir que habilidad usar -- sin un orden fijo, la eleccion seria
    // no-deterministica entre ejecuciones/compiladores distintos, y no se
    // podria testear.
    std::vector<std::string> knownSkillIds() const;

    int mana() const { return m_mana; }
    int maxMana() const { return m_maxMana; }
    // Si el mana actual queda por encima del nuevo maximo, se recorta
    // (mismo criterio que Camera::setZoom clampando a un minimo: nunca
    // deja el objeto en un estado inconsistente).
    void setMaxMana(int maxMana);

    // true si la entidad conoce la habilidad Y tiene mana suficiente. No
    // la consume (ver spend()): permite a la UI/IA comprobar que opciones
    // mostrar/elegir sin efectos secundarios.
    bool canUse(const Skill& skill) const;

    // Descuenta skill.mpCost de m_mana (clampado a 0, nunca negativo).
    // Precondicion logica: canUse(skill) ya devolvio true -- spend() no
    // lo revalida internamente, igual que Enemy::takeDamage no valida
    // amount>=0 (ver su comentario): mantiene la clase simple y deja la
    // validacion a quien orquesta el combate, que de todas formas ya
    // tiene que llamar a canUse() para decidir si ofrecer la opcion.
    void spend(const Skill& skill);
    void restoreMana(int amount);

private:
    int m_mana;
    int m_maxMana;
    // Set simulado con un mapa (valor siempre true): unordered_set
    // serviria igual, pero unordered_map<string,bool> deja la puerta
    // abierta a guardar algo mas por habilidad (ej. nivel de maestria)
    // sin cambiar la interfaz publica.
    std::unordered_map<std::string, bool> m_known;
};

// Catalogo de Skill por id (Fase 7): quien construya el contenido del
// juego rellena aqui las habilidades disponibles (desde JSON, a mano, o
// mas adelante desde su propio archivo -- fuera del alcance de esta
// iteracion) y luego SkillSet/EnemySpawn las referencian por id. Vive
// junto a Skill (no en Core::Resources) porque no encaja en
// ResourceManager<T>: ahi T es un recurso GL propietario de un nombre GL
// (ver su comentario en motor_grafico_clases.puml), y Skill es dato plano
// sin ningun recurso que liberar.
//
// : public ICatalog<Skill> (fractura #2, ver ARCHITECTURE.md): mismo
// contrato has/find/size que ResourceManager y ObjectCatalog. has/find ya
// existian con la firma canonica; solo se anade size() (que faltaba) y la
// declaracion de herencia. Marcar find/has como override formaliza lo que
// ya cumplia.
class SkillCatalog : public ICatalog<Skill> {
public:
    void add(Skill skill);
    bool has(const std::string& id) const override;
    // nullptr si no existe: mismo criterio que TextureManager::get() /
    // ResourceManager<T>::find() (puntero no-propietario, nullptr = "no
    // encontrado", sin lanzar).
    const Skill* find(const std::string& id) const override;
    std::size_t size() const override { return m_skills.size(); }

private:
    std::unordered_map<std::string, Skill> m_skills;
};

// Resultado enriquecido de ApplySkillEffect / ApplyBasicAttackNd6: el
// motor RPG devuelve el grado de exito Nd6 (BOTCH/PARTIAL/SUCCESS/
// CRITICAL) y la magnitud final realmente aplicada, para que BattleState
// pueda loggear "(Parcial: 5 de dano)" en vez de simplemente "10".
struct SkillApplyResult {
    int hpChange = 0;
    int degree = 0;  // 0=BOTCH 1=PARTIAL 2=SUCCESS 3=CRITICAL
};

// Ataque básico mediante Nd6 ONEgAI (Gracia Fundamental):
//   - pool = 1 dado base + stat(DES) del caster (§1.1 GDD)
//   - CD = DicePoolEngine::defense_to_cd(target.defense_value(ARMOR_CLASS))
//   - Magnitudes: BOTCH=0, PARTIAL=5, SUCCESS=10, CRITICAL=15
//     (SUCCESS coincide con el kBasicAttackPower histórico del MVP).
SkillApplyResult ApplyBasicAttackNd6(ICombatant& caster, ICombatant& target,
                                     RPG::RandomEngine& rng);

// Aplica el efecto de "skill" sobre "target" de verdad ORQUESTADO POR EL
// MOTOR Nd6 ONEgAI (antes: daño directo sin dados).
//
// Adaptación Skill legacy → SkillDefinition temporal (no rompemos el
// contrato Skill.h actual; el motor real lo usaría más adelante con
// SkillDefinition directamente):
//
//   • casting_stat:
//       - Damage → DES (atacar, mismo criterio basic attack)
//       - Heal   → INT  (curar = conocimiento del sanador; es GDD §1.1)
//   • save_attribute (CD):
//       - Damage tipo "golpe cuerpo a cuerpo" → ARMOR_CLASS (mismo
//         criterio que basic attack). Si quisieramos distinguir hechizos
//         de dano, la Skill legacy no lo expresa; usamos el caso mas
//         comun.
//       - Heal → no hay tirada de salvacion (la curacion NO falla por
//         "defensa" del aliado). Guardamos el cd = 0.0 y N = stat INT.
//   • pool de dados: N = 1 + caster.stat(casting_stat)
//   • magnitude_by_degree:
//       BOTCH    = 0
//       PARTIAL  = max(1, skill.power / 2)  (mitad, nunca 0)
//       SUCCESS  = skill.power              (valor histórico)
//       CRITICAL = skill.power * 1.5        (todo 6s → 1.5×)
//
// NO descuenta el mana del que la usa (eso es SkillSet::spend(), aparte).
// Devuelve SkillApplyResult {hp_change (>=0), grado (0..3)}.
SkillApplyResult ApplySkillEffect(const Skill& skill, ICombatant& caster, ICombatant& target,
                                  RPG::RandomEngine& rng);
