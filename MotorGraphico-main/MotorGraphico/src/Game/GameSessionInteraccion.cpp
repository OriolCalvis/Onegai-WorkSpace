// Lo que pasa cuando el jugador pulsa E, y lo que pasa despues.
//
// Hablar, cruzar una puerta, abrir un cofre, entrar en combate y cerrar
// lo que quedara abierto. Es el flujo que el jugador percibe como «una
// accion», aunque por dentro toque media docena de sistemas.
//
// Salio de src/Game/GameSession.cpp el 23/09/2026 (EST-23): aquel fichero
// tenia 1167 lineas y lo cruzan seis paquetes del plan (C-04, C-17, D-08,
// E-04, G-12, G-14) — de los sitios donde dos agentes se pisan sin
// enterarse. Se movio, no se reescribio: ni un cuerpo tocado.
//
// El estado sigue viviendo en GameSession (40 miembros). Sacarlo a una
// clase de datos es el otro medio relevo y va aparte, porque tocaria cada
// m_ de las mil lineas y el diff dejaria de poder leerse.


#include "Game/GameSession.h"
#include "Game/Acompanante.h"  // ACOMPANANTES-01
#include "Game/Contenedores.h"
#include "Game/ContenedoresEquipo.h"
#include "Game/Actuacion.h"  // ACTUAR-01
#include "Game/CartasEnCombate.h"  // H2-06
#include "Game/Refuerzos.h"  // ROSITA-01
#include "Game/RutinasPnj.h"  // PNJ-02

#include "RPG/InventoryEngine.h"  // G-12

#include "Level/ObjectCatalog.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "Runtime/RuntimeTileMap.h"

#include <algorithm>
#include <optional>

namespace {

// CAER-01 — Quien cae en un combate de la historia que no es ineludible se
// levanta con un cuarto de su vida (1 como poco). Devuelve con cuanta.
int levantaHeroe(ICombatant& heroe) {
    const int objetivo = std::max(1, heroe.maxHealth() / 4);
    if (heroe.health() < objetivo) heroe.heal(objetivo - heroe.health());
    return heroe.health();
}

}  // namespace

void GameSession::startInteraction(const ObjectDefinition& def) {
    if (def.category != ObjectCategory::Npc) {
        return;
    }
    sessionData()->m_dialogueSpeaker = def.name;
    sessionData()->m_dialogueLines = def.dialogue.lines;

    // La narrativa MANDA sobre el dialogo fijo del catalogo: si la
    // aventura activa tiene algo que decir por boca de este NPC ahora
    // mismo, eso es lo que se dice; si no, las lineas de siempre.
    //
    // Este es el orden importante y no el contrario: hace que un NPC de
    // relleno siga funcionando sin tocarlo, y que ese MISMO NPC pase a
    // formar parte de una aventura con solo escribir un beat en un JSON
    // -- sin editar el catalogo de objetos ni el nivel.
    bool narrado = false;
    if (sessionData()->m_narrative != nullptr && sessionData()->m_narrativeState != nullptr) {
        const RPG::NarrativeResult narrated = sessionData()->m_narrative->talkTo(def.id, *sessionData()->m_narrativeState);
        if (narrated.fired) {
            // Un solo sitio para aplicar NarrativeResult (oro, log, y ahora
            // skillChecks Nd6): antes esto estaba duplicado aqui y en
            // applyNarrative, y la deuda tecnica saltaba justo ahora, al
            // anadir un tercer efecto diferido. applyNarrative deja
            // sessionData()->m_dialogueLines/m_dialogueSpeaker listos; mas abajo se decide
            // modo Dialogue/Shop.
            applyNarrative(narrated);
            narrado = !sessionData()->m_dialogueLines.empty();
        }
    }
    // TUTORIAL-01 — Un beat que monta un combate: se lee y, al cerrarlo, se pelea.
    if (sessionData()->m_battle != nullptr) return;

    resolveShopOffers(def);

    // TUTORIAL-01 — Lo que dice la aventura se lee antes de comprar: con un beat, la
    // hoja del dialogo y, al cerrarla, la tienda (closeInteraction). Sus
    // lineas ya estan en el log (applyNarrative); las del catalogo, aqui.
    // TIENDA-02 — Y sin beat tambien (Oriol, 08/10: «un minimo de dialogo todos»):
    // el tendero dice lo suyo y, al cerrar, la tienda. Solo uno sin lineas (que
    // validar_todo no deja pasar) abriria la tienda en el acto.
    const bool habla = narrado || !sessionData()->m_dialogueLines.empty();
    sessionData()->m_mode = sessionData()->m_shopOffers.empty() || habla ? GameMode::Dialogue : GameMode::Shop;
    if (narrado) return;
    for (const std::string& line : sessionData()->m_dialogueLines) {
        logLine(sessionData()->m_dialogueSpeaker + ": " + line);
    }
}


bool GameSession::interact() {
    if (sessionData()->m_mode != GameMode::Exploration) {
        return false;
    }

    // 1) La celda del jugador: una puerta se PISA (transicion).
    const Engine::EntityHandle here = objectAt(sessionData()->m_playerPosition.x, sessionData()->m_playerPosition.y);
    if (here.valid()) {
        const ObjectSpawn& spawn = sessionData()->m_world->get(here)->object;
        if (!spawn.targetLevel.empty()) {
            if (cierraPuerta(spawn)) return true;  // G4-01
            LevelTransition& t = sessionData()->m_transition;
            t.pending = true;
            t.levelPath = spawn.targetLevel;
            t.entryPosition = spawn.targetPosition;
            t.hasEntryPosition = spawn.hasTargetPosition;
            // TUTORIAL-01 — Se vuelve por donde se entro: la puerta en la que se
            // aparecio lleva a la del nivel de antes (ver LevelTransition).
            const GridCoord& llegada = sessionData()->m_level.playerStart;
            if (!t.vueltaNivel.empty() && t.vueltaNivel == spawn.targetLevel && spawn.position.x == llegada.x &&
                spawn.position.y == llegada.y) {
                t.entryPosition = t.vueltaPuerta;
                t.hasEntryPosition = true;
            }
            const ObjectDefinition* def =
                sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(spawn.objectId) : nullptr;
            logLine("Entras en " + (def != nullptr ? def->name : spawn.objectId) + ".");
            return true;
        }
    }

    if (Game::interactuaContenedor(*sessionData())) return true;
    // 2) La propia celda y las cuatro contiguas: NPC con el que hablar
    // o cartel de negocio que leer. Se incluye la celda del jugador
    // (offset 0,0) porque un cartel no bloquea el paso y es normal
    // acabar encima de el.
    const int dx[] = {0, 0, 0, -1, 1};
    const int dy[] = {0, -1, 1, 0, 0};
    for (int i = 0; i < 5; ++i) {
        const Engine::EntityHandle probe = objectAt(sessionData()->m_playerPosition.x + dx[i],
                                                    sessionData()->m_playerPosition.y + dy[i]);
        if (!probe.valid()) {
            continue;
        }
        const ObjectSpawn& spawn = sessionData()->m_world->get(probe)->object;
        const ObjectDefinition* def =
            sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(spawn.objectId) : nullptr;
        if (def == nullptr) {
            continue;
        }
        if (def->category == ObjectCategory::Npc) {
            m_rutinas->habla(probe, sessionData()->m_playerPosition);  // PNJ-02: se para y te mira
            startInteraction(*def);
            return true;
        }
        // ACTUAR-01 — Un sitio para actuar: la tirada, el publico y el oro (Game/Actuacion.h).
        if (def->actuacion.rivalDice > 0) {
            addGold(Game::actuaEn(*sessionData(), *def).oro);
            return true;
        }
        if (def->business.price > 0) {
            sessionData()->m_currentBusinessId = def->id;
            sessionData()->m_currentBusinessName =
                def->business.businessName.empty() ? def->name : def->business.businessName;
            sessionData()->m_currentBusinessPrice = def->business.price;
            sessionData()->m_currentBusinessIncome = def->business.baseIncome;
            sessionData()->m_mode = GameMode::Business;
            return true;
        }
    }
    return false;
}

void GameSession::ponVuelta(const std::string& nivel, GridCoord puerta) {
    sessionData()->m_transition.vueltaNivel = nivel;
    sessionData()->m_transition.vueltaPuerta = puerta;
}

bool GameSession::pause() {
    if (sessionData()->m_mode != GameMode::Exploration) {
        return false;
    }
    sessionData()->m_mode = GameMode::Pause;
    return true;
}

bool GameSession::resume() {
    if (sessionData()->m_mode != GameMode::Pause) {
        return false;
    }
    sessionData()->m_mode = GameMode::Exploration;
    return true;
}

void GameSession::closeInteraction() {
    if (sessionData()->m_mode != GameMode::Dialogue && sessionData()->m_mode != GameMode::Shop && sessionData()->m_mode != GameMode::Business) {
        return;
    }
    // OPCIONES-01 — Con una eleccion pendiente no se cierra: se contesta (chooseOption).
    if (sessionData()->m_mode == GameMode::Dialogue && sessionData()->m_narrativeState != nullptr &&
        sessionData()->m_narrativeState->eleccion().activa()) {
        return;
    }
    // DIALOGOS-01 — Al cerrar un dialogo, antes de abrir la tienda: lo que salte
    // solo (lo que acaba de pasar se cuenta antes) y, si no salta nada y la
    // opcion elegida lo pedia, la charla sigue (se vuelve a hablar con el
    // mismo). La tienda espera a que acabe la conversacion.
    if (sessionData()->m_mode == GameMode::Dialogue && sessionData()->m_battle == nullptr &&
        sessionData()->m_narrative != nullptr && sessionData()->m_narrativeState != nullptr) {
        const auto ofertas = sessionData()->m_shopOffers;
        RPG::NarrativeResult r = sessionData()->m_narrative->tick(*sessionData()->m_narrativeState);
        if (!r.fired) r = sessionData()->m_narrative->sigue(*sessionData()->m_narrativeState);
        if (r.fired) {
            applyNarrative(r);
            if (sessionData()->m_mode == GameMode::Dialogue && sessionData()->m_battle == nullptr)
                sessionData()->m_shopOffers = ofertas;
            if (sessionData()->m_mode != GameMode::Dialogue || !sessionData()->m_dialogueLines.empty() ||
                sessionData()->m_narrativeState->eleccion().activa())
                return;
        }
    }
    // TUTORIAL-01 — Cerrar lo que dijo un beat abre lo que el beat dejo montado: su
    // combate (applyNarrative) o la tienda de quien hablaba (startInteraction).
    if (sessionData()->m_mode == GameMode::Dialogue &&
        (sessionData()->m_battle != nullptr || !sessionData()->m_shopOffers.empty())) {
        sessionData()->m_mode = sessionData()->m_battle != nullptr ? GameMode::Battle : GameMode::Shop;
        if (sessionData()->m_battle != nullptr) sessionData()->m_shopOffers.clear();
        sessionData()->m_dialogueLines.clear();
        return;
    }
    sessionData()->m_mode = GameMode::Exploration;
    sessionData()->m_dialogueLines.clear();
    sessionData()->m_shopOffers.clear();
    sessionData()->m_dialogueSpeaker.clear();
    sessionData()->m_currentBusinessId.clear();
    sessionData()->m_currentBusinessName.clear();
    sessionData()->m_currentBusinessPrice = 0;
    sessionData()->m_currentBusinessIncome = 0;

    // Ya en Exploration: ¿tiene la aventura algo que decir por su
    // cuenta? Un solo tick por cierre (no un bucle while): un disparo,
    // un beat. Si un cierre encadena con otro, hace falta cerrar otra
    // vez -- que es como se lee en pantalla de todas formas, y evita
    // que un contenido mal escrito cuelgue el juego en un bucle
    // infinito de beats que se retroalimentan.
    if (sessionData()->m_narrative != nullptr && sessionData()->m_narrativeState != nullptr) {
        applyNarrative(sessionData()->m_narrative->tick(*sessionData()->m_narrativeState));
    }
}


void GameSession::startBattle(const std::vector<Engine::EntityHandle>& handles,
                              const std::vector<RPG::AcompananteDef>& aLado) {
    // Uno o un grupo (Oriol, 05/10): cada enemigo vivo del World entra con
    // su nombre; si se repite, numerado («Perdido saqueador 2») para poder
    // elegir a quien se apunta. ACOMPANANTES-01: aLado, quien pelea con el
    // heroe (Venides, Xila), detras de el en su bando.
    std::vector<BattleParticipant> enemies;
    std::vector<Engine::EntityHandle> enCombate;
    std::vector<std::string> nombres;
    for (const Engine::EntityHandle handle : handles) {
        Engine::WorldEntity* entity = sessionData()->m_world->get(handle);
        if (entity == nullptr || entity->kind != Engine::EntityKind::Enemy) continue;
        Engine::WorldEnemy& enemy = entity->enemy;
        const ObjectDefinition* def =
            sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(enemy.objectId) : nullptr;
        const std::string base = def != nullptr ? def->name : enemy.objectId;
        const int repetido = static_cast<int>(std::count(nombres.begin(), nombres.end(), base));
        nombres.push_back(base);
        enemies.push_back({repetido == 0 ? base : base + " " + std::to_string(repetido + 1), enemy.brain.get(),
                           enemy.skills.get(), nullptr});
        enCombate.push_back(handle);
    }
    if (enemies.empty()) return;
    const std::string enemyName = enemies.front().name;

    // El jugador entra en combate con SU cuerpo/habilidades/inventario
    // reales (no copias): lo que gasta o cura en combate se conserva al
    // volver a explorar, y la accion Item (Fase 10) consume del
    // inventario de verdad.
    // H2-06 — En el registro del combate el heroe se llama como su ficha.
    const RPG::CharacterSheet* ficha = characterSheet();
    const std::string heroe = ficha != nullptr && !ficha->displayName.empty() ? ficha->displayName : "Heroe";
    std::vector<BattleParticipant> allies{{heroe, sessionData()->m_playerBody, sessionData()->m_playerSkills, sessionData()->m_inventory}};

    // H2-06 / AV13-35 — El heroe juega siempre desde su mesa (Game/CartasEnCombate.h): sin ficha, con
    // las basicas; sin catalogo de cartas (las pruebas), con uno vacio. Ya no hay combate sin mesa.
    static const Game::CatalogoCartas kSinCartas{{}, {}, {}, {}};
    std::unique_ptr<Game::CartasEnCombate> mesa;
    if (sessionData()->m_playerBody != nullptr)
        mesa = Game::mesaConBotin(m_cartas != nullptr ? *m_cartas : kSinCartas, *sessionData()->m_playerBody, sessionData()->m_inventoryRules.equipment);
    // ACOMPANANTES-01 — La mesa es la duena de los acompanantes y la que los
    // hace actuar. Sus curas son de toda la aventura.
    std::string aLadoTexto;
    const RPG::AdventureScript* aventura =
        sessionData()->m_narrative != nullptr ? sessionData()->m_narrative->adventure() : nullptr;
    for (const RPG::AcompananteDef& def : aLado) {
        if (mesa == nullptr) break;
        const std::string clave = Game::Acompanante::claveCuras(aventura != nullptr ? aventura->id : "", def.id);
        const int usadas = sessionData()->m_narrativeState != nullptr ? sessionData()->m_narrativeState->variable(clave) : 0;
        allies.push_back({def.nombre, mesa->sumaAcompanante(def, def.cura.veces - usadas), nullptr, nullptr});
        aLadoTexto += (aLadoTexto.empty() ? "" : ", ") + def.nombre;
    }

    sessionData()->m_battle = std::make_unique<BattleState>(std::move(allies), std::move(enemies), sessionData()->m_skillCatalog,
                                             sessionData()->m_catalog);
    sessionData()->m_battle->setConditionCatalog(sessionData()->m_conditions);
    if (mesa != nullptr) sessionData()->m_battle->setMesa(std::move(mesa));
    sessionData()->m_battleEnemyHandles = enCombate;
    sessionData()->m_mode = GameMode::Battle;
    if (enCombate.size() == 1) logLine("Un " + enemyName + " te corta el paso!");
    else logLine("Te cortan el paso " + std::to_string(enCombate.size()) + " enemigos!");
    if (!aLadoTexto.empty()) logLine("A tu lado: " + aLadoTexto + ".");
    montaTablero();  // AV13-10: el combate en las casillas del nivel, cada uno en la suya
}

bool GameSession::syncBattleOutcome() {
    if (sessionData()->m_battle == nullptr) {
        return false;
    }

    // D-11 O2 — materialitza invocacions pendents ABANS de comprovar el
    // desenllaç. Un summon pot aparèixer en qualsevol moment de la batalla
    // (turn del jugador o del enemic), inclòs quan el combat continua
    // (InProgress). Si el combat ja va acabar, les invocacions no s'apliquen
    // (l'enemic ja no té sentit dins la batalla tancada).
    if (sessionData()->m_battle->outcome() == BattleOutcome::InProgress) {
        if (sessionData()->m_pendingBattle.has_value() && sessionData()->m_narrativeState != nullptr)  // ROSITA-01
            Game::llegaRefuerzos(*sessionData()->m_battle, *sessionData()->m_pendingBattle, *sessionData()->m_narrativeState,
                                 sessionData()->m_narrative != nullptr ? sessionData()->m_narrative->adventure() : nullptr);
        return false;  // combat encara en marxa
    }

    const BattleOutcome outcome = sessionData()->m_battle->outcome();

    // El log del combate se copia ANTES de destruirlo (si no, se pierde
    // con el BattleState y el cuadro de dialogo se queda sin las lineas
    // del turno que acaba de terminar).
    for (const std::string& line : sessionData()->m_battle->log()) {
        sessionData()->m_log.push_back(line);
    }

    // H2-05 — Los ids se copian aqui, antes de que las entidades mueran con
    // el World: son la clave de su ficha de monstruo, y sin ellos no habria
    // de quien leer el botin. En grupo, uno por enemigo.
    std::string enemyName;
    std::vector<std::string> enemyObjectIds;
    std::vector<bool> caidos;
    for (const Engine::EntityHandle handle : sessionData()->m_battleEnemyHandles) {
        Engine::WorldEntity* fight = sessionData()->m_world->get(handle);
        enemyObjectIds.push_back(fight != nullptr ? fight->objectId() : std::string());
        caidos.push_back(fight == nullptr || fight->kind != Engine::EntityKind::Enemy ||
                         fight->enemy.brain == nullptr || !fight->enemy.brain->isAlive());
        if (fight != nullptr && enemyName.empty()) {
            const ObjectDefinition* def =
                sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(fight->objectId()) : nullptr;
            enemyName = def != nullptr ? def->name : fight->objectId();
        }
    }
    const bool grupo = sessionData()->m_battleEnemyHandles.size() > 1;
    // CAER-01 — Caer en un combate de la historia no acaba la partida, salvo
    // en un momento ineludible (ver RPG::BattleRequest). Los del mapa, si.
    const std::optional<RPG::BattleRequest> pedido = std::move(sessionData()->m_pendingBattle);
    sessionData()->m_pendingBattle.reset();
    const bool seLevanta = pedido.has_value() && !pedido->ineludible;

    switch (outcome) {
        case BattleOutcome::Victory:
            // C-17 — Derrotados: los enemigos se dan de baja del World (el
            // render lo detecta porque su handle deja de estar vivo).
            // H2-05 — El botin se reparte ANTES de la baja, que es cuando
            // el id del enemigo sigue vivo; la linea de victoria se
            // escribe despues para que el botin se lea debajo.
            for (const std::string& id : enemyObjectIds) repartirBotin(id);
            for (const Engine::EntityHandle handle : sessionData()->m_battleEnemyHandles)
                sessionData()->m_world->destroy(handle);
            // FASE1-BND-01 — Quien huye a 0 PV se va (Sandes «escapará al quedarse a 0
            // de vida»); en una pelea sin matar, los que caen siguen vivos.
            for (std::size_t i = 0; pedido.has_value() && i < enemyObjectIds.size(); ++i) {
                const std::string& id = enemyObjectIds[i];
                if (std::find(pedido->huyen.begin(), pedido->huyen.end(), id) == pedido->huyen.end() ||
                    std::find(enemyObjectIds.begin(), enemyObjectIds.begin() + static_cast<long>(i), id) !=
                        enemyObjectIds.begin() + static_cast<long>(i))
                    continue;  // no huye, o ya se ha contado
                const ObjectDefinition* def = sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(id) : nullptr;
                logLine((def != nullptr ? def->name : id) + " huye malherido.");
            }
            if (pedido.has_value() && pedido->sinMatar) logLine("Quedan fuera de combate, pero nadie ha muerto.");
            else logLine(grupo ? std::string("Has derrotado a todos.") : "Has derrotado al " + enemyName + ".");
            sessionData()->m_mode = GameMode::Exploration;
            break;
        case BattleOutcome::Fled:
            // Los vivos siguen en su celda: huir no los mata (ver header);
            // los que cayeron en el combate, si.
            for (std::size_t i = 0; i < sessionData()->m_battleEnemyHandles.size(); ++i)
                if (grupo && caidos[i]) sessionData()->m_world->destroy(sessionData()->m_battleEnemyHandles[i]);
            logLine(grupo ? std::string("Escapas.") : "Escapas del " + enemyName + ".");
            sessionData()->m_mode = GameMode::Exploration;
            break;
        case BattleOutcome::Defeat:
            logLine("Has caído derrotado.");
            if (!seLevanta || sessionData()->m_playerBody == nullptr) {
                sessionData()->m_mode = GameMode::GameOver;
                break;
            }
            // Los que ganaron se van (la aventura cuenta adonde) y el heroe
            // se levanta con un cuarto de la vida.
            for (const Engine::EntityHandle handle : sessionData()->m_battleEnemyHandles)
                sessionData()->m_world->destroy(handle);
            logLine("Te levantas con " + std::to_string(levantaHeroe(*sessionData()->m_playerBody)) + " PV.");
            sessionData()->m_mode = GameMode::Exploration;
            break;
        case BattleOutcome::InProgress:
            break;  // inalcanzable (comprobado arriba)
    }

    // Flags del combate pedido por la narrativa (effect "startBattle"). Huir
    // enciende flagHuida o, si la aventura no la distingue, flagDefeat: el
    // enemigo sigue vivo y lo que fuera a pasar, pasa (CAER-01).
    RPG::NarrativeState* trama = sessionData()->m_narrativeState;
    if (trama != nullptr && pedido.has_value()) {
        const std::string& flag = outcome == BattleOutcome::Victory ? pedido->flagVictory
                                  : outcome == BattleOutcome::Fled && !pedido->flagHuida.empty() ? pedido->flagHuida
                                                                                                 : pedido->flagDefeat;
        if (!flag.empty()) trama->setFlag(flag);
    }
    // Quien cae no sigue peleando: las oleadas que faltaban se pierden.
    if (outcome == BattleOutcome::Defeat) {
        for (const RPG::BattleRequest& resto : sessionData()->m_battleQueue)
            if (trama != nullptr && !resto.flagDefeat.empty()) trama->setFlag(resto.flagDefeat);
        sessionData()->m_battleQueue.clear();
    }

    // ACOMPANANTES-01 — Las curas gastadas (Xila: dos al dia) cuentan para
    // toda la aventura, no para este combate.
    const RPG::AdventureScript* aventura =
        sessionData()->m_narrative != nullptr ? sessionData()->m_narrative->adventure() : nullptr;
    for (const BattleParticipant& p : sessionData()->m_battle->allies()) {
        const auto* a = dynamic_cast<const Game::Acompanante*>(p.combatant);
        if (a == nullptr || !a->cura() || sessionData()->m_narrativeState == nullptr) continue;
        sessionData()->m_narrativeState->setVariable(Game::Acompanante::claveCuras(aventura != nullptr ? aventura->id : "", a->def().id),
                                                     a->def().cura.veces - a->curasQueQuedan());
    }

    sessionData()->m_battle.reset();
    sessionData()->m_battleEnemyHandles.clear();  // AV13-29/33: los tuyos se quedan mientras ves que te rescatan
    if (sessionData()->m_mode != GameMode::GameOver) quitaAcompanantes();

    // Siguiente combate encolado, si lo hay (oleadas).
    if (!sessionData()->m_battleQueue.empty() && sessionData()->m_mode != GameMode::GameOver) {
        std::vector<RPG::BattleRequest> siguiente{sessionData()->m_battleQueue.front()};
        sessionData()->m_battleQueue.erase(sessionData()->m_battleQueue.begin());
        resolveBattles(siguiente);
    }
    // TUTORIAL-01 — Acabado el combate, la aventura dice lo suyo, como al cerrar un
    // dialogo: sin este tick, «tut_victoria» esperaba a la siguiente charla.
    if (sessionData()->m_mode == GameMode::Exploration && sessionData()->m_narrative != nullptr &&
        sessionData()->m_narrativeState != nullptr) {
        applyNarrative(sessionData()->m_narrative->tick(*sessionData()->m_narrativeState));
    }
    return true;
}

// C-17 — Vistas derivadas (ver declaracion en GameSession.h): se leen del
// World, no de listas internas. Los enemigos comparten el brain vivo via
// shared_ptr; es una vista, quien quiera escribir usa world().

// H2-05 — Ganar da el botin del monstruo. El dato ya existe y no hay que
// inventarlo: es el campo `loot` de monsters.json (432 fichas, 28 ids
// distintos), que el cargador deja en MonsterDefinition::lootIds.
//
// Esos ids son de EQUIPO, no de objeto, asi que cada pieza entra con
// objectIdForEquipment() -- el mismo camino que usan equipar y vender. Por
// eso lo que cae se puede equipar y vender sin trabajo nuevo: no es un
// objeto suelto colgando del catalogo. El log lo nombra con el nombre de la
// ficha ("Botin: Daga de milicia"), no con el id crudo.
//
// Huir no pasa por aqui (ver el caso Fled): el enemigo sigue vivo y su
// botin sigue con el.
void GameSession::repartirBotin(const std::string& enemyObjectId) {
    if (enemyObjectId.empty() || sessionData()->m_monsterCatalog == nullptr ||
        sessionData()->m_inventory == nullptr) {
        return;
    }
    const RPG::MonsterDefinition* monster = sessionData()->m_monsterCatalog->find(enemyObjectId);
    if (monster == nullptr || monster->lootIds.empty()) {
        return;
    }

    std::vector<std::string> nombres;
    for (const std::string& equipmentId : monster->lootIds) {
        const std::string objectId = objectIdForEquipment(equipmentId);
        sessionData()->m_inventory->push_back(objectId);
        const RPG::EquipmentDefinition* ficha = equipmentFor(objectId);
        nombres.push_back(ficha != nullptr ? ficha->name : objectId);
    }

    std::string linea = "Botin: ";
    for (std::size_t i = 0; i < nombres.size(); ++i) {
        if (i != 0) linea += ", ";
        linea += nombres[i];
    }
    logLine(linea);
}
