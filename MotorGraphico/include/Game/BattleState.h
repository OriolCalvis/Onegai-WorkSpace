#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "RPG/RandomEngine.h"

class ICombatant;
class SkillSet;
class SkillCatalog;
class ObjectCatalog;

// Combate por turnos estilo Dragon Quest clasico (motor_grafico_gantt_rpg.puml,
// Fase 8). GL-free a proposito: opera sobre ICombatant/SkillSet
// (interfaces y datos de la Fase 7), NUNCA sobre Player/Enemy concretos,
// asi que se puede testear sin ventana ni TextureAtlas -- mismo criterio
// que demo_skills.cpp (ver examples/demo_battle.cpp). En el motor real,
// quien arme la batalla (la futura Application, o un demo) le pasa
// Player*/Enemy* como ICombatant*: ambos ya implementan la interfaz (ver
// ICombatant.h).
//
// Cola de turnos deliberadamente simple (sin velocidad/iniciativa -- ver
// motor_grafico_dafo.md sobre sobre-ingenieria): una ronda es
// resolveAllyAction() para cada aliado vivo (llamado desde fuera, uno por
// uno -- normalmente el jugador elige via HUD, Fase 9) seguido de UNA
// llamada a resolveEnemyTurn() que resuelve TODOS los enemigos vivos de
// golpe. Quien orquesta el bucle de combate (Application/demo) decide
// cuando ha terminado el turno de los aliados y toca llamar a
// resolveEnemyTurn().
//
// Item (Fase 10): consume un objeto del inventario del que actua, con el
// efecto que diga el ObjectCatalog (PickupData, ver ObjectCatalog.h) --
// exactamente la pieza que faltaba cuando se escribio la Fase 8 ("anadir
// BattleActionType::Item es trivial una vez exista ese catalogo": ya
// existe). El item se aplica SIEMPRE sobre el propio actor (beberse una
// pocion), no admite objetivo en este MVP; lanzar items a otros es una
// extension futura si algun contenido la pide.
enum class BattleActionType { Attack, UseSkill, Item, Flee };

struct BattleAction {
    BattleActionType type = BattleActionType::Attack;
    // Ignorado si type == Flee/Item. Para UseSkill, el id resuelve contra
    // el SkillCatalog que se le paso a BattleState en el constructor.
    std::string skillId;
    // Indice dentro del bando CONTRARIO al que actua (allies() si actua
    // un enemigo, enemies() si actua un aliado). Ignorado si type ==
    // Flee/Item (Item siempre actua sobre uno mismo, ver arriba).
    int targetIndex = 0;
    // Solo para Item (Fase 10): id de un ObjectDefinition (categoria
    // Pickup) que ademas debe estar en el inventario del actor (ver
    // BattleParticipant::inventory). Va DESPUES de targetIndex a
    // proposito, aunque tematicamente pegue junto a skillId: los
    // llamadores anteriores a la Fase 10 inicializan BattleAction con
    // llaves posicionales de 3 elementos ({type, skillId, targetIndex},
    // ver demo_battle.cpp) -- insertar un miembro en medio los rompia en
    // SILENCIO (el 0 de targetIndex pasaria a inicializar un
    // std::string desde un char* nulo: compila y crashea en runtime).
    std::string itemId;
};

enum class BattleOutcome { InProgress, Victory, Defeat, Fled };

// Un participante del combate: el ICombatant real (Player/Enemy/un
// ICombatant de prueba) mas su SkillSet (puede ser nullptr si el
// participante no tiene ninguna habilidad, ej. un enemigo "solo ataca
// basico" -- ver resolveEnemyTurn()) y un nombre para el log de texto
// (Fase 9: el futuro cuadro de dialogo del HUD lee de aqui). Ninguno de
// los dos punteros es propietario (mismo criterio que
// IsometricRenderer::addToQueue con IRenderable*): BattleState no crea ni
// destruye nada, solo referencia lo que ya existe durante la batalla.
struct BattleParticipant {
    std::string name;
    ICombatant* combatant = nullptr;
    SkillSet* skills = nullptr;
    // Inventario del participante (ids de item, con repeticion: dos
    // pociones = dos entradas), para la accion Item (Fase 10). No
    // propietario, igual que combatant/skills: apunta al vector real del
    // dueno (ej. Player::inventory() -- las pociones gastadas en combate
    // deben desaparecer del inventario REAL del jugador, no de una
    // copia). nullptr = sin inventario (enemigos, en este MVP).
    std::vector<std::string>* inventory = nullptr;
};

class BattleState {
public:
    // catalog: no propietario, debe seguir vivo mientras dure el combate
    // (se usa en cada resolveAllyAction()/resolveEnemyTurn() para
    // resolver ids de Skill). objectCatalog (Fase 10): idem pero para la
    // accion Item; su default nullptr mantiene compilando a todos los
    // llamadores anteriores a la Fase 10 (demo_battle, demo_battle_hud)
    // sin cambiarlos -- con nullptr, cualquier accion Item falla
    // controladamente ("no puede usar"), igual que UseSkill con catalog
    // nullptr. Comprueba el desenlace nada mas construir (por si alguien
    // arma un combate con un bando ya sin ICombatant vivos, caso limite
    // defensivo).
    BattleState(std::vector<BattleParticipant> allies, std::vector<BattleParticipant> enemies,
                SkillCatalog* catalog, ObjectCatalog* objectCatalog = nullptr);

    // Resuelve la accion de allies()[allyIndex]. No hace nada (no-op) si
    // el combate ya termino, si allyIndex esta fuera de rango o muerto,
    // si el objetivo (para Attack/UseSkill) esta fuera de rango o muerto,
    // o si UseSkill referencia un id que no existe en el catalogo o que
    // el aliado no puede pagar -- en los tres ultimos casos queda log()
    // con una linea explicando el fallo, para que el HUD (Fase 9) pueda
    // mostrarlo igual que un turno valido.
    void resolveAllyAction(std::size_t allyIndex, const BattleAction& action);

    // Resuelve el turno de TODOS los enemigos vivos, uno tras otro, con
    // una IA minima (Fase 8, "sin variacion de IA" en el MVP del Gantt):
    // cada enemigo usa la PRIMERA habilidad conocida (SkillSet::
    // knownSkillIds(), orden alfabetico -- determinista) que pueda pagar;
    // si no conoce ninguna o no puede pagar ninguna, ataque basico. El
    // objetivo es siempre el primer aliado vivo (sin aleatoriedad, mismo
    // criterio de determinismo que Enemy::update() -- ver su comentario
    // en Enemy.h). Se detiene en cuanto el combate termina a mitad de
    // ronda (ej. el ultimo aliado muere tras el segundo de tres
    // enemigos): los enemigos que quedaban no actuan.
    void resolveEnemyTurn();

    BattleOutcome outcome() const { return m_outcome; }
    const std::vector<BattleParticipant>& allies() const { return m_allies; }
    const std::vector<BattleParticipant>& enemies() const { return m_enemies; }

    // Log de texto de TODA la batalla (no se vacia entre turnos): una
    // linea por accion resuelta ("Player ataca a Slime (10 de dano)."),
    // pensado para que un cuadro de dialogo del HUD (Fase 9) muestre las
    // ultimas N lineas.
    const std::vector<std::string>& log() const { return m_log; }

    // Inyecta el generador de las tiradas de esta batalla. La referencia
    // debe seguir viva mientras viva la BattleState (no se copia); si no
    // se llama, se usa el Xoroshiro128p interno y nada cambia.
    //
    // Existe porque sin esto NO SE PUEDE PROBAR EL COMBATE. Todo el daño
    // pasa por ApplyBasicAttackNd6/ApplySkillEffect -> SkillExecutor ->
    // DicePoolEngine, asi que un ataque ya no quita un numero fijo: quita
    // 0, la mitad, lo normal o x1.5 segun el grado de la tirada. Un test
    // que afirme "al slime le quedan 10 PV" sin controlar el dado no esta
    // comprobando la regla, esta comprobando qué salio de la semilla por
    // defecto -- y se rompe en cuanto alguien toca cuántos dados se
    // consumen, aunque las reglas sigan siendo correctas.
    //
    // Lo descubrio justo eso: al corregir el orden de las ramas de
    // DicePoolEngine::resolve_against_cd (la pifia iba la ultima y con
    // CD 0 nunca se alcanzaba), demo_battle y demo_game_session
    // empezaron a fallar sin que la regla nueva fuese peor -- solo habia
    // cambiado la cara del dado que tocaba. Ver examples/ScriptedRng.h.
    void setRandomEngine(RPG::RandomEngine& rng) { m_rngInUse = &rng; }

private:
    void checkOutcome();
    void logLine(const std::string& line);

    // resolveItemAction: la rama Item de resolveAllyAction(), separada
    // porque no comparte la validacion de objetivo enemigo (un item
    // siempre actua sobre el propio actor).
    void resolveItemAction(BattleParticipant& actor, const std::string& itemId);

    std::vector<BattleParticipant> m_allies;
    std::vector<BattleParticipant> m_enemies;
    SkillCatalog* m_catalog;
    ObjectCatalog* m_objectCatalog;
    BattleOutcome m_outcome = BattleOutcome::InProgress;
    std::vector<std::string> m_log;

    // Motor de números aleatorios para TIRADAS Nd6 ONEgAI del combate.
    // Cada batalla tiene su propia instancia (no global) para que los
    // seeds sean independientes y se pueda reproducir una batalla
    // concreta (debug / test).
    RPG::Xoroshiro128p m_rng;

    // A dónde van de verdad las tiradas: al m_rng de arriba salvo que
    // alguien inyecte otro con setRandomEngine(). Ver el comentario de
    // ese método.
    RPG::RandomEngine* m_rngInUse = &m_rng;
};
