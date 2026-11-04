// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   C-01  (fase C, 2 j) - La curació cura qui toca
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Resources/ICatalog.h"

#include "RPG/RandomEngine.h"
#include "RPG/SkillTarget.h"

#include "Render/ICombatant.h"

// Habilidad de combate (motor_grafico_gantt_rpg.puml, Fase 7 "Sistema de
// habilidades"): datos puros, sin logica de motor (sin GL, sin Entity).
// "Que hace" una habilidad al usarse de verdad (aplicar dano/curacion a
// un objetivo concreto en un combate) es responsabilidad de quien la use
// (el futuro BattleState de la Fase 8), no de Skill: mismo principio de
// separacion que LevelDefinition/EnemySpawn (datos) frente a Enemy
// (entidad que se dibuja y actualiza).
// Compatibilidad de fuente para los consumidores Game antiguos. El tipo y
// sus valores viven en RPG::SkillTarget: catálogo y combate hablan el mismo
// contrato, incluida ZONE_RADIUS.
using SkillTarget = RPG::SkillTarget;
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
    RPG::SkillTarget target = RPG::SkillTarget::SINGLE_ENEMY;
    // D-09: pont cap a SkillDefinition (que ja porta aquests tres camps al
    // cataleg): condicio a aplicar quan el grau hi arriba. Buit = el
    // comportament antic (cap condicio). minDegree en 0..3 (BOTCH/PARTIAL/
    // SUCCESS/CRITICAL, mateix ordre que RPG::Degree).
    std::string applyConditionId;
    int applyConditionMinDegree = 2;
    int conditionDurationRounds = 3;
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
    // D0027 — Devuelve el mana a un valor exacto (la foto de un combate), dentro de 0..max.
    void ponMana(int mana) { m_mana = mana < 0 ? 0 : (mana > m_maxMana ? m_maxMana : mana); }

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
    // D-09: condicio pendent d'aplicar (id + rondes de la carta). Buida si
    // la carta no en porta o el grau no hi arriba. L'aplica BattleState
    // via ConditionEngine; ApplySkillEffect no toca fitxes.
    std::string pendingConditionId;
    int pendingConditionRounds = 0;
    // D-11 O2: invocació pendent (id del summon). Buida si la carta no en
    // porta o el grau no hi arriba (>= SUCCESS). BattleState la
    // materialitza via GameSession::spawnEnemyFromCatalog.
    std::string pendingSummonId;
    // D-11: vocabulari d'efectes. Resultat de l'execució
    // de effect_scaling de la carta.
    std::string effectFamily;
    bool effectSupported = true;
    std::string effectDiagnostic;
    std::vector<std::string> appliedEffects;
};

// Ataque básico mediante Nd6 ONEgAI (Gracia Fundamental):
//   - pool = 1 dado base + stat(DES) del caster (§1.1 GDD)
//   - rival = pool de defensa del objetivo (DES o CON, Ed. 3)
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
//   • oposicion:
//       - Damage usa la defensa fisica (DES o CON) como basic attack.
//       - Heal se resuelve sin rival: la defensa del aliado no lo bloquea.
//   • pool de dados: N = 1 + caster.stat(casting_stat)
//   • magnitude_by_degree:
//       BOTCH    = 0
//       PARTIAL  = max(1, skill.power / 2)  (mitad, nunca 0)
//       SUCCESS  = skill.power              (valor histórico)
//       CRITICAL = skill.power * 1.5        (dos 6 o más, D0013 → 1.5×)
//
// NO descuenta el mana del que la usa (eso es SkillSet::spend(), aparte).
// Devuelve SkillApplyResult {hp_change (>=0), grado (0..3)}.
SkillApplyResult ApplySkillEffect(const Skill& skill, ICombatant& caster, ICombatant& target,
                                  RPG::RandomEngine& rng);

// H2-06 — Las mismas cartas que montan por dentro ApplyBasicAttackNd6 y
// ApplySkillEffect, para quien tenga que tirar con ventajas, desventajas o
// dados de mas (Game::CartasEnCombate): asi el golpe de arma y la habilidad
// legada de un enemigo se resuelven con los mismos numeros en los dos caminos.
namespace RPG { struct SkillDefinition; }
RPG::SkillDefinition definicionAtaqueBasico();
RPG::SkillDefinition definicionDeHabilidad(const Skill& skill);
