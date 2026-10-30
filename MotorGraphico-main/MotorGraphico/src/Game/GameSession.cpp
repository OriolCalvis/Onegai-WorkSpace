// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 6 paquetes del corte B:
//   C-04  (fase C, 2 j) - Un enemic invocat per narrativa no trenca el guardat
//   C-17  (fase C, 6 j) - World és el propietari de les entitats
//   D-08  (fase D, 3 j) - Els enemics del joc són els del catàleg
//   E-04  (fase E, 7 j) - El joc de comandes
//   G-12  (fase G, 5 j) - Inventari, equip i fitxa utilitzables
//   G-14  (fase G, 3 j) - HUD, diàlegs i comerç amb accions visibles
//
// AVISO - FICHERO DE CRUCE. Con 6 paquetes encima, es de los
// sitios donde dos agentes se pisan sin enterarse. Antes de una
// refactorizacion grande aqui, mira si hay trabajo sin commitear
// (git status) y di en que paquete estas trabajando.
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// EL NUCLEO DE LA SESION: nacer, moverse y latir.
//
// 23/09/2026 (EST-23): de 1167 lineas a ~290. Comercio, narrativa,
// interaccion e inventario salieron a ficheros propios; aqui queda lo que
// es de la sesion misma —construirse con su mundo, decidir si una casilla
// deja pasar, mover al jugador y avanzar el reloj—.


#include "RPG/Narrative/FranjasDelDia.h"
#include "RPG/Narrative/NarrativeState.h"
#include "Game/GameSession.h"
#include "Game/Contenedores.h"
#include "Game/Actuacion.h"  // ACTUAR-01
#include "Game/RutinasPnj.h"  // PNJ-02
#include "Game/SessionData.h"
#include "Game/EnemyCatalogBridge.h"

#include "RPG/InventoryEngine.h"  // G-12

#include "Level/ObjectCatalog.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/Definitions/SkillDefinition.h"
#include "Runtime/RuntimeTileMap.h"

#include <algorithm>


GameSession::GameSession(RuntimeTileMap* map, ObjectCatalog* catalog, SkillCatalog* skillCatalog,
                         LevelDefinition level, ICombatant* playerBody, SkillSet* playerSkills,
                         std::vector<std::string>* inventory, Engine::World* world,
                         const RPG::Catalogs::MonsterCatalog* monsterCatalog)

    : m_data(std::make_unique<SessionData>()), m_rutinas(std::make_shared<Game::RutinasPnj>()) {
    // EST-23b — Los 40 miembros se movieron a SessionData, pero la lista
    // de inicializacion quedo solo con m_data: estas ocho asignaciones se
    // perdieron con ella. Sin ellas la sesion arranca con el catalogo, el
    // nivel, el cuerpo del jugador, el inventario y los dos catalogos a
    // nullptr (y m_level vacio), asi que no aparece ningun enemigo, no hay
    // narrativa ni puertas, y update() revienta con el cuerpo nulo.
    // Se restauran aqui y ANTES de leerlas: la posicion inicial y el
    // reparto de spawns de mas abajo las necesitan ya puestas.
    sessionData()->m_map = map;
    sessionData()->m_catalog = catalog;
    sessionData()->m_skillCatalog = skillCatalog;
    sessionData()->m_monsterCatalog = monsterCatalog;
    sessionData()->m_level = std::move(level);
    sessionData()->m_playerBody = playerBody;
    sessionData()->m_playerSkills = playerSkills;
    sessionData()->m_inventory = inventory;
    // C-17 — El World es el propietario de las entidades. Si quien llama
    // no lo inyecta (demos, tests), la sesion se crea uno propio que le
    // vive mientras dure.
    if (world != nullptr) {
        sessionData()->m_world = world;
    } else {
        sessionData()->m_ownedWorld = std::make_unique<Engine::World>();
        sessionData()->m_world = sessionData()->m_ownedWorld.get();
    }
    sessionData()->m_playerPosition = sessionData()->m_level.playerStart;

    // Reparte los ObjectSpawn del nivel (Fase 10) en dos familias segun
    // su categoria en el catalogo: los enemigos necesitan cuerpo/
    // habilidades propios (se crean aqui desde CombatData), el resto
    // son datos estaticos del mundo. Un objectId que no este en el
    // catalogo se trata como objeto normal sin efecto (no se descarta y
    // no se rechaza el nivel: mismo criterio permisivo que LevelLoader,
    // que a proposito no valida ids -- ver su comentario; un id roto se
    // ve en pantalla como un objeto inerte, no revienta la partida).
    //
    // Antes esto llenaba dos listas de la sesion; desde C-17 puebla el
    // World. flushPendingChanges() los hace visibles (y solo visibles
    // para las queries): Application construye los marcadores de render
    // justo despues con los handles estables.
    for (std::size_t spawnIndex = 0; spawnIndex < sessionData()->m_level.objects.size(); ++spawnIndex) {
        const ObjectSpawn& spawn = sessionData()->m_level.objects[spawnIndex];
        const ObjectDefinition* def =
            sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(spawn.objectId) : nullptr;
        if (def != nullptr && def->category == ObjectCategory::Enemy) {
            // D-08 — Usar EnemyCatalog::createEnemy() en vez de construir
            // EnemyBrain a mano: si hay MonsterDefinition en el catalogo
            // RPG, el enemigo obtiene stats reales (vida, defensas, tier).
            Engine::WorldEnemy enemy = EnemyCatalog::createEnemy(
                spawn.objectId, def, sessionData()->m_monsterCatalog,
                spawn.position, spawn.patrolMin, spawn.patrolMax,
                false, static_cast<int>(spawnIndex));
            sessionData()->m_world->createEnemy(std::move(enemy));
        } else {
            sessionData()->m_world->createObject(spawn);
        }
    }
    sessionData()->m_world->flushPendingChanges();

    // Caso limite defensivo: una partida que arranca con el jugador ya
    // muerto (nivel cargado sobre un ICombatant sin vida) es GameOver de
    // salida, no una exploracion imposible.
    if (sessionData()->m_playerBody == nullptr || !sessionData()->m_playerBody->isAlive()) {
        sessionData()->m_mode = GameMode::GameOver;
    }
}

void GameSession::logLine(const std::string& line) { sessionData()->m_log.push_back(line); }

Engine::EntityHandle GameSession::enemyAt(int x, int y) const {
    return sessionData()->m_world->findEnemyAt(x, y);
}

Engine::EntityHandle GameSession::objectAt(int x, int y) const {
    return sessionData()->m_world->findObjectAt(x, y);
}

bool GameSession::tileBlocks(int x, int y) const {
    if (sessionData()->m_map == nullptr) {
        return false;  // sin mapa, nada bloquea (tests de solo objetos)
    }
    if (x < 0 || y < 0 || x >= sessionData()->m_map->getWidth() || y >= sessionData()->m_map->getHeight()) {
        return true;  // fuera del mapa: como un muro
    }
    // Cualquier capa con colision en esa celda bloquea (TileMap::getTile
    // lanza fuera de rango, pero el rango ya esta comprobado arriba).
    for (int layer = 0; layer < sessionData()->m_map->getLayerCount(); ++layer) {
        if (sessionData()->m_map->getTile(layer, x, y).hasCollision()) {
            return true;
        }
    }
    return false;
}

bool GameSession::tryMovePlayer(int dx, int dy) {
    sessionData()->m_lastPickupId.clear();
    if (sessionData()->m_mode != GameMode::Exploration) {
        return false;
    }
    const int nx = sessionData()->m_playerPosition.x + dx;
    const int ny = sessionData()->m_playerPosition.y + dy;

    if (tileBlocks(nx, ny)) {
        return false;
    }
    // AV19-02: un acantilado (dos niveles de diferencia o mas) no se sube ni se baja andando.
    if (sessionData()->m_map != nullptr &&
        !sessionData()->m_map->alturas().sePuedePasar(sessionData()->m_playerPosition, GridCoord{nx, ny})) {
        return false;
    }

    // Enemigo en el destino: no se entra, empieza el combate.
    const Engine::EntityHandle enemyHandle = enemyAt(nx, ny);
    if (enemyHandle.valid()) {
        startBattle({enemyHandle});
        return false;
    }

    // Objeto en el destino: bloquea (prop) o se recoge (pickup).
    const Engine::EntityHandle objHandle = objectAt(nx, ny);
    if (objHandle.valid()) {
        Engine::WorldEntity* entity = sessionData()->m_world->get(objHandle);
        const ObjectSpawn& spawn = entity->object;
        const ObjectDefinition* def =
            sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(spawn.objectId) : nullptr;
        if (def != nullptr && def->blocksMovement) {
            return false;
        }
        if (def != nullptr && def->category == ObjectCategory::Pickup) {
            if (sessionData()->m_inventory != nullptr) {
                sessionData()->m_inventory->push_back(spawn.objectId);
            }
            sessionData()->m_lastPickupId = spawn.objectId;
            logLine("Recoges " + def->name + ".");
            sessionData()->m_world->destroy(objHandle);
        }
        // AV17-11: el fuego (o una casa en llamas) no cierra el paso, pero quema al pisarlo.
        if (def != nullptr && def->fuego.dano > 0 && sessionData()->m_playerBody != nullptr) {
            sessionData()->m_playerBody->takeDamage(def->fuego.dano);
            logLine("Te quemas con " + def->name + ": pierdes " + std::to_string(def->fuego.dano) + " PV.");
        }
    }

    sessionData()->m_playerPosition = GridCoord{nx, ny};
    // ACTUAR-01 — Con los pasos se renueva la gente de los sitios para actuar (Game/Actuacion.h).
    if (sessionData()->m_narrativeState != nullptr) Game::cuentaUnPaso(*sessionData()->m_narrativeState);

    // El tiempo del juego son los pasos del jugador (ver el comentario
    // de sessionData()->m_stepsSinceRent): cada SessionData::kStepsPerRentCycle celdas andadas se
    // cobra un ciclo de rentas de todo lo que poseas.
    // PO-49: con la cuenta de dias en marcha las rentas se cobran UNA vez por dia de
    // historia (no cada 40 pasos: caminar en circulo no da dinero); sin dias, el ciclo de pasos.
    const int diaHistoria = sessionData()->m_narrativeState != nullptr
                                ? RPG::franjas::momentoDe(*sessionData()->m_narrativeState).dia
                                : 0;
    if (diaHistoria > 0) {
        cobraRentasDelDia(diaHistoria);
    } else if (!sessionData()->m_businesses.empty() &&
               ++sessionData()->m_stepsSinceRent >= SessionData::kStepsPerRentCycle) {
        sessionData()->m_stepsSinceRent = 0;
        collectRent();
    }
    return true;
}

// PO-49 — Una renta por cada dia de historia que ha pasado desde el ultimo cobro. El dia en que
// compras (o la primera vez que se mira) solo se apunta: la primera renta llega al dia siguiente.
void GameSession::cobraRentasDelDia(int dia) {
    RPG::NarrativeState* estado = sessionData()->m_narrativeState;
    if (estado == nullptr) return;
    constexpr const char* kUltimoCobro = "renta.dia";
    if (sessionData()->m_businesses.empty()) {  // sin negocios no se debe nada: el contador sigue al dia
        if (estado->hasVariable(kUltimoCobro)) estado->setVariable(kUltimoCobro, dia);
        return;
    }
    if (!estado->hasVariable(kUltimoCobro)) {
        estado->setVariable(kUltimoCobro, dia);
        return;
    }
    int ultimo = estado->variable(kUltimoCobro);
    if (dia <= ultimo) return;
    estado->setVariable(kUltimoCobro, dia);
    for (int d = ultimo; d < dia && d < ultimo + 3; ++d) collectRent();  // tope de 3: sin cobros infinitos
}

void GameSession::update(float deltaTime) {
    Game::revisaEscondites(*sessionData());
    // PNJ-02 — Los PNJ con rutina (si la Application la ha montado) andan con
    // el mismo criterio de "que bloquea" que el jugador. Fuera de exploracion
    // acaban el paso que llevan y no echan a andar.
    if (m_rutinas->activa()) {
        Game::MundoPnj pnj;
        pnj.jugador = sessionData()->m_playerPosition;
        pnj.parado = sessionData()->m_mode != GameMode::Exploration;
        pnj.libre = [this](const GridCoord& c) {
            return !tileBlocks(c.x, c.y) && !objectAt(c.x, c.y).valid() && !enemyAt(c.x, c.y).valid();
        };
        pnj.pasa = [this](const GridCoord& de, const GridCoord& a) {  // AV19-02: sin cruzar acantilados
            return sessionData()->m_map == nullptr || sessionData()->m_map->alturas().sePuedePasar(de, a);
        };
        m_rutinas->avanza(deltaTime, *sessionData()->m_world, pnj);
    }

    // El mundo solo avanza en exploracion: durante un combate, un
    // dialogo o una compra, los enemigos no siguen patrullando por
    // detras (mismo criterio modal que tryMovePlayer, y evita que al
    // salir de una tienda te encuentres a un slime encima).
    if (sessionData()->m_mode != GameMode::Exploration) {
        return;
    }

    // C-17 — La patrulla recorre el World (no una lista interna) y
    // escribe el espejo enemy.position del brain. Los looks up espaciales
    // dentro del callback de colision usan las mismas queries que el
    // jugador: una sola fuente de verdad para "que bloquea".
    sessionData()->m_world->eachEntity([&](Engine::EntityHandle self, Engine::WorldEntity& entity) {
        if (entity.kind != Engine::EntityKind::Enemy) {
            return;
        }
        Engine::WorldEnemy& enemy = entity.enemy;
        if (enemy.brain == nullptr || !enemy.brain->isAlive()) {
            return;
        }

        // Que puede pisar el enemigo. Se responde con el MISMO criterio
        // de colision que el jugador (tileBlocks + objetos que bloquean)
        // en vez de reimplementarlo: si manana cambia que bloquea el
        // paso, cambia en un solo sitio.
        enemy.brain->update(deltaTime, [&](const GridCoord& cell) {
            if (tileBlocks(cell.x, cell.y)) {
                return false;
            }
            // AV19-02: tampoco sube ni baja un acantilado (2 niveles o mas), como el jugador.
            if (sessionData()->m_map != nullptr &&
                !sessionData()->m_map->alturas().sePuedePasar(enemy.brain->position(), cell)) {
                return false;
            }
            if (cell.x == sessionData()->m_playerPosition.x && cell.y == sessionData()->m_playerPosition.y) {
                return false;  // no se pisa al jugador: el combate lo inicia EL
            }
            const Engine::EntityHandle obj = objectAt(cell.x, cell.y);
            if (obj.valid()) {
                const ObjectDefinition* def =
                    sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(sessionData()->m_world->get(obj)->objectId()) : nullptr;
                if (def != nullptr && def->blocksMovement) {
                    return false;
                }
            }
            // Ni encima de otro enemigo (dos slimes en la misma celda
            // son un solo slime a la vista, y el combate no sabria con
            // cual empezar). Se excluye al propio: a un enemigo no le
            // bloquea su propia celda.
            const Engine::EntityHandle other = enemyAt(cell.x, cell.y);
            if (other.valid() && other != self) {
                return false;
            }
            return true;
        });

        // La posicion autoritativa es la del brain; enemy.position es su
        // espejo para quien ya lo lee (minimapa, marcadores).
        enemy.position = enemy.brain->position();
    });
}

// basePriceOf vive en GameSessionComercio.cpp (H2-08): es la mitad del
// comercio —comprar y vender lo usan los dos— y aqui solo lo colgaba una
// llamada desde el otro fichero.

void GameSession::addGold(int amount) {
    // Nunca por debajo de cero: un cobro mayor que el saldo es un bug de
    // quien llama, y dejar oro negativo lo propagaria en silencio por
    // toda la partida (mismo criterio que HudBar clampando su valor).
    sessionData()->m_gold = std::max(0, sessionData()->m_gold + amount);
}

// C-03 — El traspaso del estado del jugador entre sesiones, en un solo
// sitio. Ver PlayerCarryOver en GameSession.h.
PlayerCarryOver GameSession::captureCarryOver() const {
    PlayerCarryOver state;
    state.gold = sessionData()->m_gold;
    state.moralityValue = sessionData()->m_morality.value();
    state.moral = sessionData()->m_morality.estado();  // MORAL-01
    state.stepsSinceRent = sessionData()->m_stepsSinceRent;
    state.businesses = sessionData()->m_businesses;
    return state;
}

void GameSession::applyCarryOver(const PlayerCarryOver& state) {
    // Asignacion, no acumulacion: ver el comentario de la declaracion.
    // setValue() no registra motivo a proposito -- cruzar una puerta no
    // es una accion moral, es el mismo estado en otro mapa.
    sessionData()->m_gold = std::max(0, state.gold);
    sessionData()->m_morality.setValue(state.moralityValue);
    sessionData()->m_morality.restauraEstado(state.moral);  // MORAL-01
    if (sessionData()->m_narrativeState != nullptr) sessionData()->m_narrativeState->ponMoral(state.moralityValue);
    sessionData()->m_stepsSinceRent = std::max(0, state.stepsSinceRent);
    sessionData()->m_businesses = state.businesses;
}

std::vector<ObjectSpawn> GameSession::worldObjects() const {
    std::vector<ObjectSpawn> out;
    sessionData()->m_world->eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& entity) {
        if (entity.kind == Engine::EntityKind::Object) {
            out.push_back(entity.object);
        }
    });
    return out;
}

std::vector<WorldEnemy> GameSession::enemies() const {
    std::vector<WorldEnemy> out;
    sessionData()->m_world->eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& entity) {
        if (entity.kind == Engine::EntityKind::Enemy) {
            out.push_back(entity.enemy);
        }
    });
    return out;
}

// ---------------------------------------------------------------------
// G-12 — Inventario y equipo (los stubs de GDB-12, hechos de verdad)
// ---------------------------------------------------------------------
