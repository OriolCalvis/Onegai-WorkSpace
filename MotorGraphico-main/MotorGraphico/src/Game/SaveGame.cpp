// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 5 paquetes del corte B:
//   C-03  (fase C, 2 j) - L'estat del jugador sobreviu el canvi de nivell
//   C-04  (fase C, 2 j) - Un enemic invocat per narrativa no trenca el guardat
//   C-08  (fase C, 1 j) - Un guardat antic no escriu fora de rang
//   E-02  (fase E, 3 j) - Interruptors locals i variables
//   G-11  (fase G, 4 j) - Selector de guardats i recuperació de derrota
//
// AVISO - FICHERO DE CRUCE. Con 5 paquetes encima, es de los
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

#include "Game/SaveGame.h"

// EST-23b — SaveGame.h ya solo trae los tipos de valor; quien USA la
// clase la incluye, que es lo que deberia haber hecho siempre.
#include "Game/GameSession.h"
#include "Game/RutinasPnj.h"  // PNJ-02: un PNJ que pasea se guarda donde nacio
// C-04: restoreSaveGame consulta el catalogo para revalidar los enemigos
// invocados antes de volver a materializarlos.
#include "Level/ObjectCatalog.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

#include "Game/EnemyBrain.h"
#include "Project/SerializerHelpers.h"

namespace {

bool fail(std::string* error, const std::string& message) {
    if (error != nullptr) {
        *error = message;
    }
    return false;
}

// Ids de objeto que han cambiado de nombre: un guardado de antes trae el viejo
// en la misma casilla, y eso no es que el nivel haya cambiado.
bool mismoObjeto(const std::string& guardado, const std::string& ahora) {
    if (guardado == ahora) return true;
    static const std::pair<const char*, const char*> kRenombrados[] = {
        {"cofre", "cofre_del_vacio"},  // OBJETOS-01 (08/10/2026): el cofre de El Vacío
    };
    for (const auto& [antes, despues] : kRenombrados)
        if (guardado == antes && ahora == despues) return true;
    return false;
}

}  // namespace

// El formato y la restauracion narrativa viven en SaveGameFormat.cpp.
// Esta unidad conserva el I/O y el puente con la sesion; no los duplica.

// ---------------------------------------------------------------------
// RUN-10.3 — Fichero
// ---------------------------------------------------------------------

bool writeSaveGameFile(const std::string& path, const SaveGameData& data, std::string* error) {
    if (path.empty()) {
        return fail(error, "ruta de guardado vacia");
    }

    // Misma escritura atomica que proyectos y mapas: el destino no se
    // elimina antes del reemplazo y los temporales tienen dueño exclusivo.
    const auto saved = ProjectIO::atomicWrite(path, serializeSaveGame(data));
    return saved.isOk() ? true : fail(error, saved.errorMessage());
}

bool readSaveGameFile(const std::string& path, SaveGameData* out, std::string* error) try {
    std::ifstream file(std::filesystem::u8path(path), std::ios::binary);
    if (!file) {
        return fail(error, "no se puede abrir " + path);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return parseSaveGame(buffer.str(), out, error);
} catch (const std::filesystem::filesystem_error& exception) {
    return fail(error, "no se puede leer " + path + ": " + exception.what());
}

// ---------------------------------------------------------------------
// RUN-10.2 — Captura y restauracion sobre una GameSession
//
// Viven aqui y no en GameSession.cpp para mantener unido el puente entre
// la partida y sus datos. El formato se delega en SaveGameFormat.cpp.
// ---------------------------------------------------------------------

SaveGameData GameSession::captureSaveGame(const std::string& levelPath) const {
    SaveGameData data;
    data.levelPath = levelPath;

    data.playerPosition = sessionData()->m_playerPosition;
    data.returnLevel = sessionData()->m_transition.vueltaNivel;  // PUERTAS-01
    data.returnDoor = sessionData()->m_transition.vueltaPuerta;
    if (sessionData()->m_playerBody != nullptr) {
        data.playerHealth = sessionData()->m_playerBody->health();
        data.playerMaxHealth = sessionData()->m_playerBody->maxHealth();
        if (const RPG::CharacterSheet* ficha = sessionData()->m_playerBody->character_sheet()) {
            data.playerLevel = ficha->nivel;  // NIVELES-01
            data.playerLevelProgress = ficha->progresoNivel;
            data.playerTier = ficha->tier;
            data.playerTierMilestones = ficha->hitosTier;
            data.playerMilestones = ficha->milestones;  // H2-04
            // GUARDAR-FICHA — Quien es: su nombre, lo elegido al crearlo y lo
            // puesto. El aspecto lo pone la creacion (PantallaNuevaPartida).
            data.characterName = ficha->displayName;
            SavedHero& h = data.hero;
            h.raceId = ficha->raceId;
            h.classId = ficha->classId;
            h.backgroundId = ficha->backgroundId;
            h.deityId = ficha->deityId;
            h.bondId = ficha->bondId;
            h.fearId = ficha->fearId;
            h.flawId = ficha->flawId;
            h.goalId = ficha->goalId;
            h.idealId = ficha->idealId;
            h.personalityId = ficha->personalityId;
            h.virtueId = ficha->virtueId;
            h.equipped.assign(ficha->equippedIds.begin(), ficha->equippedIds.end());
            h.reputacion = ficha->factionRep;  // AV14-23 (D-MOR-09)
        }
    }
    if (sessionData()->m_inventory != nullptr) {
        data.inventory = *sessionData()->m_inventory;
    }

    // C-03 — El guardado lee el estado del jugador por el mismo camino
    // que el cambio de nivel: si manana se anade un campo a
    // PlayerCarryOver, aparece en los dos sitios o en ninguno.
    const PlayerCarryOver carried = captureCarryOver();
    data.gold = carried.gold;
    data.moralityValue = carried.moralityValue;
    data.moral = carried.moral;  // MORAL-01
    data.stepsSinceRent = carried.stepsSinceRent;
    data.businesses = carried.businesses;

    // Solo lo que SIGUE en el mundo: lo recogido y lo derrotado se
    // reconoce al cargar porque no aparece en la lista.
    // C-17 — Las entidades viven en el World: se leen de ahi, no de
    // listas internas.
    sessionData()->m_world->eachEntity([&](Engine::EntityHandle h, const Engine::WorldEntity& source) {
        if (source.kind == Engine::EntityKind::Object) {
            if (source.object.objectId.rfind("acompanante_", 0) == 0) return;  // AV13-29: solo viven en el combate
            SavedWorldObject object;
            object.objectId = source.object.objectId;
            object.position = m_rutinas->origenDe(h, source.object.position);  // PNJ-02: donde nacio
            data.worldObjects.push_back(object);
        } else if (source.kind == Engine::EntityKind::Enemy && source.enemy.brain != nullptr) {
            SavedEnemy enemy;
            enemy.spawnIndex = source.enemy.spawnIndex;
            enemy.summoned = source.enemy.summoned;  // C-04
            enemy.objectId = source.enemy.objectId;
            enemy.position = source.enemy.brain->position();
            enemy.health = source.enemy.brain->health();
            data.enemies.push_back(enemy);
        }
    });

    if (sessionData()->m_narrativeState != nullptr) {
        data.narrativeFlags = sessionData()->m_narrativeState->flags();
        data.narrativeBeatsFired = sessionData()->m_narrativeState->beatsFired();
        data.narrativeTimers = sessionData()->m_narrativeState->timers();
        data.narrativeVariables = sessionData()->m_narrativeState->variables();
        data.narrativeLocalSwitches = sessionData()->m_narrativeState->localSwitches();
    }

    return data;
}

bool GameSession::restoreSaveGame(const SaveGameData& data, std::string* error) {
    // --- Fase 1: VALIDAR sin escribir nada ---
    //
    // Todos los handles se resuelven antes de tocar el World. Si el nivel
    // ha cambiado (se anadio o quito un objeto en el JSON), aplicar el
    // guardado pondria la vida de un lobo en un barril: preferimos
    // negarnos y decir por que. Los handles estables de World son la
    // identidad aqui: los objetos se buscan por posicion (no se mueven),
    // los enemigos por spawnIndex (patrullan, y su posicion al cargar no
    // tiene por que coincidir con la guardada).
    //
    // C-17 — Antes se reconocian los objetos/enemigos por INDICE en las
    // listas de la sesion; ahora las entidades viven en World y se
    // identifican por handle.
    std::vector<Engine::EntityHandle> objectHandles;
    objectHandles.reserve(data.worldObjects.size());
    for (const SavedWorldObject& object : data.worldObjects) {
        const Engine::EntityHandle handle = objectAt(object.position.x, object.position.y);
        if (!handle.valid()) {
            return fail(error, "el guardado no encaja con el nivel: no hay ningun objeto en (" +
                                   std::to_string(object.position.x) + "," +
                                   std::to_string(object.position.y) + ")");
        }
        if (!mismoObjeto(object.objectId, sessionData()->m_world->get(handle)->objectId())) {
            return fail(error, "el nivel ha cambiado desde que se guardo: se esperaba '" +
                                   object.objectId + "' en (" + std::to_string(object.position.x) +
                                   "," + std::to_string(object.position.y) + ")");
        }
        objectHandles.push_back(handle);
    }

    // C-04 — Marca para "este no sale del nivel porque no estaba ahi":
    // un invocado por la narrativa no existe en una sesion recien
    // construida, hay que volver a materializarlo del catalogo.
    struct PlannedEnemy {
        Engine::EntityHandle handle;  // invalido para summoned: se re-crea en Fase 2
    };
    std::vector<PlannedEnemy> enemyPlans;
    enemyPlans.reserve(data.enemies.size());
    for (const SavedEnemy& enemy : data.enemies) {
        if (enemy.summoned) {
            // Lo unico que se puede validar de un invocado es que su
            // definicion siga existiendo: si alguien borro el monstruo
            // del catalogo entre partida y partida, restaurar lo
            // colocaria como un enemigo sin cuerpo.
            const ObjectDefinition* def =
                sessionData()->m_catalog != nullptr ? sessionData()->m_catalog->find(enemy.objectId) : nullptr;
            if (def == nullptr || def->category != ObjectCategory::Enemy) {
                return fail(error, "el guardado trae un enemigo invocado que ya no esta en el "
                                   "catalogo: '" + enemy.objectId + "'");
            }
            enemyPlans.push_back(PlannedEnemy{});
            continue;
        }

        // Un enemigo del nivel se reconoce por su spawnIndex, no por su
        // posicion ni su id (varios lobos comparten definicion y los que
        // patrullan no estan donde nacieron).
        Engine::EntityHandle handle;
        sessionData()->m_world->eachEntity([&](Engine::EntityHandle h, const Engine::WorldEntity& entity) {
            if (handle.valid() || entity.kind != Engine::EntityKind::Enemy ||
                entity.enemy.summoned) {
                return;
            }
            if (entity.enemy.spawnIndex == enemy.spawnIndex) {
                handle = h;
            }
        });
        if (!handle.valid()) {
            return fail(error, "el guardado no encaja con el nivel: enemigo fuera de rango (" +
                                   std::to_string(enemy.spawnIndex) + ")");
        }
        if (sessionData()->m_world->get(handle)->objectId() != enemy.objectId) {
            return fail(error, "el nivel ha cambiado desde que se guardo: se esperaba '" +
                                   enemy.objectId + "' en la posicion " +
                                   std::to_string(enemy.spawnIndex) + " del nivel");
        }
        enemyPlans.push_back(PlannedEnemy{handle});
    }

    // --- Fase 2: aplicar ---

    // Barrido: todo lo vivo en el World que el guardado NO reconoce se
    // destruye. Lo recogido (objetos) y lo derrotado (enemigos) desde
    // que se guardo queda fuera; lo que se conserva es exactamente lo
    // que habia en la partida.
    std::vector<Engine::EntityHandle> keep;
    keep.reserve(objectHandles.size() + enemyPlans.size());
    keep.insert(keep.end(), objectHandles.begin(), objectHandles.end());
    for (const PlannedEnemy& plan : enemyPlans) {
        if (plan.handle.valid()) {
            keep.push_back(plan.handle);
        }
    }
    sessionData()->m_world->eachEntity([&](Engine::EntityHandle handle, Engine::WorldEntity&) {
        for (const Engine::EntityHandle& kept : keep) {
            if (kept == handle) {
                return;
            }
        }
        sessionData()->m_world->destroy(handle);
    });

    // Enemigos conservados: se les restaura la posicion y la vida que
    // tenian al guardar (el nivel acaba de darles vida maxima y su sitio
    // de nacimiento).
    for (std::size_t i = 0; i < enemyPlans.size(); ++i) {
        if (!enemyPlans[i].handle.valid()) {
            continue;
        }
        Engine::WorldEnemy& live = sessionData()->m_world->get(enemyPlans[i].handle)->enemy;
        const SavedEnemy& saved = data.enemies[i];
        if (live.brain != nullptr) {
            live.brain->setPosition(saved.position);
            live.brain->set_health(saved.health);
        }
        live.position = saved.position;
    }

    // C-04 — Los invocados que seguian vivos al guardar vuelven a
    // levantarse del catalogo, con su posicion y la vida que les
    // quedaba. Sirve tambien el barrido de arriba: los que HABIA
    // destruido ya no estan en el World.
    // sessionData()->m_nextSummonIndex se lleva por encima del mayor ordinal
    // restaurado, para que la siguiente emboscada de la partida no
    // reutilice la identidad de uno que ya esta en el mundo.
    for (const SavedEnemy& saved : data.enemies) {
        if (!saved.summoned) {
            continue;
        }
        spawnEnemyFromCatalog(saved.objectId, saved.position, /*summoned=*/true,
                              saved.spawnIndex, saved.health);
        if (saved.spawnIndex >= sessionData()->m_nextSummonIndex) {
            sessionData()->m_nextSummonIndex = saved.spawnIndex + 1;
        }
    }

    // Todo lo encolado arriba (destruir el barrido, crear los invocados,
    // recolocar) entra en vigor de una vez.
    sessionData()->m_world->flushPendingChanges();

    // Jugador. ICombatant no tiene "poner vida a X" a proposito (la vida
    // se pierde o se cura, no se asigna): se llega al valor guardado
    // curando al maximo y aplicando el dano que falte. Es la unica forma
    // de restaurar sin abrir un setter que el combate podria usar mal.
    sessionData()->m_playerPosition = data.playerPosition;
    // NIVELES-01 — Tier, nivel e hitos antes que la vida: el tier fija el tope.
    if (sessionData()->m_playerBody != nullptr &&
        (data.playerLevel > 1 || data.playerLevelProgress > 0 || data.playerMilestones > 0 || data.playerTier > 1)) {
        if (RPG::CharacterSheet* ficha = sessionData()->m_playerBody->character_sheet()) {
            static const RPG::TierRules kSinReglas;
            const InventoryRules& r = sessionData()->m_inventoryRules;
            ficha->restauraProgresion(data.playerTier, data.playerLevel, data.playerLevelProgress,
                                      data.playerTierMilestones, data.playerMilestones,
                                      r.tiers != nullptr ? *r.tiers : kSinReglas);
            if (sessionData()->m_narrativeState != nullptr) {
                sessionData()->m_narrativeState->ponHeroe(ficha->nivel, ficha->tier);
            }
        }
    }
    if (sessionData()->m_playerBody != nullptr && data.playerMaxHealth > 0) {
        sessionData()->m_playerBody->heal(sessionData()->m_playerBody->maxHealth());
        const int missing = sessionData()->m_playerBody->maxHealth() - data.playerHealth;
        if (missing > 0) {
            sessionData()->m_playerBody->takeDamage(missing);
        }
    }
    if (sessionData()->m_inventory != nullptr) {
        *sessionData()->m_inventory = data.inventory;
    }

    PlayerCarryOver carried;
    carried.gold = data.gold;
    carried.moralityValue = data.moralityValue;
    carried.moral = data.moral;  // MORAL-01
    carried.stepsSinceRent = data.stepsSinceRent;
    carried.businesses = data.businesses;
    applyCarryOver(carried);

    // Narrativa: solo se ENCIENDE lo guardado. No se llama a reset()
    // porque restaurar se hace sobre una sesion recien construida y
    // borraria tambien lo que la Application hubiera preparado antes.
    if (sessionData()->m_narrativeState != nullptr) {
        applyNarrativeState(data, *sessionData()->m_narrativeState);
    }

    // Un combate en curso no sobrevive al guardado (ver captureSaveGame):
    // se vuelve a exploracion, salvo que el jugador este muerto.
    sessionData()->m_battle.reset();
    sessionData()->m_battleEnemyHandles.clear();
    quitaAcompanantes();  // AV13-29
    sessionData()->m_dialogueLines.clear();
    sessionData()->m_dialogueSpeaker.clear();
    sessionData()->m_shopOffers.clear();
    sessionData()->m_transition = LevelTransition{};
    // PUERTAS-01 — Y por donde se habia entrado: la puerta en la que se aparecio sigue
    // llevando a la de antes (Oriol, 08/10: «se sale por donde se entra»).
    sessionData()->m_transition.vueltaNivel = data.returnLevel;
    sessionData()->m_transition.vueltaPuerta = data.returnDoor;
    sessionData()->m_mode = (sessionData()->m_playerBody != nullptr && sessionData()->m_playerBody->isAlive()) ? GameMode::Exploration
                                                                 : GameMode::GameOver;
    return true;
}
