#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Game/MesaCombate.h"
#include "Game/GolpesCombate.h"
#include "RPG/RandomEngine.h"
#include "RPG/Catalogs/RpgCatalogs.h"

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
    // D0036 — Quien es en este combate («a0», «e3»: bando e indice). El nombre solo se enseña y se repite.
    // Lo pone la batalla al recibirlo.
    std::string id;
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

    // Resuelve la accion de allies()[allyIndex]: la juega la mesa de cartas (setMesa; en el juego
    // siempre hay una). Huir y usar un objeto los resuelve la batalla. No hace nada si el combate ya
    // termino o el aliado no esta en pie. AV13-35: sin mesa, el ataque y las habilidades no hacen
    // nada (el combate antiguo, el Nd6 y las habilidades con mana, se borro).
    void resolveAllyAction(std::size_t allyIndex, const BattleAction& action);

    // Resuelve el turno de TODOS los enemigos vivos, uno tras otro, por la mesa de cartas (sin mesa,
    // no hacen nada: AV13-35). Se detiene en cuanto el combate termina a mitad de ronda: los
    // enemigos que quedaban no actuan.
    void resolveEnemyTurn();

    BattleOutcome outcome() const { return m_outcome; }
    // CAER-01 — Un combate ineludible de la historia (BattleRequest): Huir
    // no hace nada, ni gasta el turno. La barra la ensena bloqueada.
    void prohibeHuida() { m_sinHuida = true; }
    bool sePuedeHuir() const { return !m_sinHuida; }
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
    // la antigua resolucion por dificultad (la pifia iba la ultima y con
    // CD 0 nunca se alcanzaba), demo_battle y demo_game_session
    // empezaron a fallar sin que la regla nueva fuese peor -- solo habia
    // cambiado la cara del dado que tocaba. Ver examples/ScriptedRng.h.
    void setRandomEngine(RPG::RandomEngine& rng) { m_rngInUse = &rng; }

    // Referencia no propietaria; el catalogo debe sobrevivir a la batalla.
    void setConditionCatalog(const RPG::Catalogs::ConditionCatalog* catalog) { m_conditions = catalog; }
    // H2-03 — Para ensenar las condiciones con su nombre (Game/EstadosCombate).
    const RPG::Catalogs::ConditionCatalog* conditionCatalog() const { return m_conditions; }

    // H2-06 — La mesa de cartas del heroe (Game/MesaCombate.h). Sin mesa, el
    // combate es el de siempre; con ella, el heroe juega sus cartas y los
    // enemigos atacan contra sus reacciones y estados. La batalla es su duena.
    void setMesa(std::unique_ptr<Game::IMesaCombate> mesa) { m_mesa = std::move(mesa); }
    Game::IMesaCombate* mesa() const { return m_mesa.get(); }
    // AV13-06 — Las casillas del nivel donde se pelea (las pone GameSession al
    // empezar; sin ellas, el combate de siempre, sin sitio).
    void ponTablero(Game::TableroCombate t) { m_campo.tablero = std::move(t); }
    const Game::TableroCombate* tablero() const { return m_campo.tablero ? &*m_campo.tablero : nullptr; }
    Game::TableroCombate* tablero() { return m_campo.tablero ? &*m_campo.tablero : nullptr; }
    // AV14-01 — Los golpes que ha resuelto la batalla, el mas nuevo al final
    // (BattleStateGolpes.cpp): la pantalla los ensena una vez cada uno.
    const std::vector<Game::GolpeCombate>& golpes() const { return m_campo.golpes; }
    // AV13-12 — La primera mitad del turno del rival: cada enemigo vivo (o solo el
    // enemigo `solo`) se acerca con sus pasos; pega al llegar, en resolveEnemyTurn o
    // resuelveEnemigo. true si alguien anda.
    bool acercaEnemigos(int solo = -1);
    // AV13-11 — La iniciativa (BattleStateTurno.cpp): 1d6 + DES para el heroe y cada
    // enemigo, con los dados de la batalla (tirarla despues de setRandomEngine). Sin
    // tirarla, el turno de siempre: tu, y luego todos los enemigos de golpe.
    void tiraIniciativa();
    const Game::OrdenDeTurno& orden() const { return m_campo.orden; }
    int enemigoEnTurno() const;  // a que enemigo le toca; -1: a un aliado, o sin iniciativa
    // AV13-29 — A que aliado le toca (0 el heroe; -1: a un enemigo). Sin iniciativa, al heroe.
    int aliadoEnTurno() const;
    // Si a ese aliado lo juega el jugador (el heroe siempre; los acompañantes salvo los de IA).
    bool loJuegaElJugador(std::size_t aliado);
    // El turno de un aliado que va solo (un acompañante de IA): su accion de siempre.
    void resuelveAliadoSolo(std::size_t aliado);
    // Cierra el turno de quien lo tiene y pasa al siguiente en pie; al dar la vuelta,
    // la ronda se cierra (tickRound). Sin iniciativa no hace nada.
    void pasaAlSiguiente();
    // El turno de un solo enemigo (el de resolveEnemyTurn, para uno): si sigue en pie
    // y tiene a alguien a tiro, actua.
    void resuelveEnemigo(std::size_t i);
    // AV13-34 (BattleStateTurno.cpp) — El golpe mortal guionizado: el enemigo `enemigo`, a `umbral` de
    // su vida o menos, te deja a las puertas con su siguiente accion (si su golpe ya te mataba, te
    // quedas a 1 PV igual). golpeMortalDado(): ya ha pasado (los refuerzos que lo esperan, llegan).
    void ponGolpeMortal(std::size_t enemigo, float umbral);
    bool golpeMortalDado() const { return m_campo.golpeMortal.dado; }
    // ROSITA-01 — Alguien que llega a mitad del combate se pone al final de tu
    // bando (como una invocacion); `linea`, al registro (Game/Refuerzos.h).
    // AV13-29: con iniciativa, el que llega juega el ultimo de la ronda (BattleStateTurno.cpp).
    void sumaAliado(BattleParticipant aliado);
    void anota(const std::string& linea) { m_log.push_back(linea); }

    // H0-06 — Un tick de ronda: una llamada por ronda de combate. BattleState
    // no lleva cuenta de turnos (eso es de quien orqueste el bucle), pero la
    // ronda SIEMPRE se cierra con alguna accion (normalmente un
    // resolveAllyAction + resolveEnemyTurn), y tras cada una de esas rondas el
    // orquestador (Application/el demo) llama aqui una vez. Avanza las
    // condiciones de TODOS los vivos: aplica el dano por ronda declarado por
    // la carta (ConditionEngine::tick_conditions NO cambia la vida; el
    // propietario de la vida es BattleState, que aplica el damageInflicted y
    // notifica la perdida real con on_damage_received, igual que hace con los
    // golpes), caduca las que han agotado las rondas, y re-evalua el desenlace
    // (un DOT puede acabar con un bando a mitad de ronda).
    void tickRound();

private:
    void notifyDamageReceived(ICombatant& target, int previousHealth);
    const RPG::Catalogs::ConditionCatalog* m_conditions = nullptr;
    void checkOutcome();
    void logLine(const std::string& line);
    Game::ContextoBatalla contexto();
    // AV13-12 — A cuantas casillas pega un enemigo (1; 5 si tiene habilidades) y si llega a un aliado.
    int alcanceDe(const BattleParticipant& enemigo) const;
    bool llegaA(const BattleParticipant& enemigo, const BattleParticipant& aliado) const;
    // AV14-01 — La vida de cada uno antes de una accion, y la ultima tirada de la mesa;
    // tras ella, anotaGolpes apunta lo que ha cambiado, con el grado de su tirada.
    struct FotoVidas {
        std::vector<int> aliados, enemigos;
        int tirada = 0;
    };
    FotoVidas fotoVidas() const;
    void anotaGolpes(const FotoVidas& antes, const std::string& de, Game::GolpeCombate::Grado porDefecto);
    void guionGolpeMortal(std::size_t enemigo);  // AV13-34: tras la accion del enemigo

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

    std::unique_ptr<Game::IMesaCombate> m_mesa;  // H2-06
    bool m_sinHuida = false;                       // CAER-01
    Game::CampoDeBatalla m_campo;                  // AV13-06/AV13-11/AV14-01: tablero, orden y golpes
};
