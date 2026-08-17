#include "Game/GameSession.h"

#include "Level/ObjectCatalog.h"
#include "Render/TileMap.h"
#include "RPG/DicePoolEngine.h"
#include "RPG/Definitions/SkillDefinition.h"

#include <algorithm>

GameSession::GameSession(TileMap* map, ObjectCatalog* catalog, SkillCatalog* skillCatalog,
                         LevelDefinition level, ICombatant* playerBody, SkillSet* playerSkills,
                         std::vector<std::string>* inventory)
    : m_map(map)
    , m_catalog(catalog)
    , m_skillCatalog(skillCatalog)
    , m_level(std::move(level))
    , m_playerBody(playerBody)
    , m_playerSkills(playerSkills)
    , m_inventory(inventory)
    , m_playerPosition(m_level.playerStart) {
    // Reparte los ObjectSpawn del nivel (Fase 10) en dos listas segun su
    // categoria en el catalogo: los enemigos necesitan cuerpo/habilidades
    // propios (se crean aqui desde CombatData), el resto son datos
    // estaticos del mundo. Un objectId que no este en el catalogo se
    // trata como objeto normal sin efecto (no se descarta y no se
    // rechaza el nivel: mismo criterio permisivo que LevelLoader, que a
    // proposito no valida ids -- ver su comentario; un id roto se ve en
    // pantalla como un objeto inerte, no revienta la partida).
    for (const ObjectSpawn& spawn : m_level.objects) {
        const ObjectDefinition* def =
            m_catalog != nullptr ? m_catalog->find(spawn.objectId) : nullptr;
        if (def != nullptr && def->category == ObjectCategory::Enemy) {
            WorldEnemy enemy;
            enemy.objectId = spawn.objectId;
            enemy.position = spawn.position;
            // La patrulla viene del SPAWN (donde patrulla ESTE slime), no
            // del catalogo (que describe que es un slime): antes se
            // parseaba del JSON y se tiraba -- ver EnemyBrain.h.
            enemy.brain = std::make_unique<EnemyBrain>(spawn.position, spawn.patrolMin,
                                                       spawn.patrolMax, def->combat.maxHealth);
            enemy.skills = std::make_unique<SkillSet>(def->combat.maxMana);
            for (const std::string& skillId : def->combat.skillIds) {
                enemy.skills->learn(skillId);
            }
            m_enemies.push_back(std::move(enemy));
        } else {
            m_worldObjects.push_back(spawn);
        }
    }

    // Caso limite defensivo: una partida que arranca con el jugador ya
    // muerto (nivel cargado sobre un ICombatant sin vida) es GameOver de
    // salida, no una exploracion imposible.
    if (m_playerBody == nullptr || !m_playerBody->isAlive()) {
        m_mode = GameMode::GameOver;
    }
}

void GameSession::logLine(const std::string& line) { m_log.push_back(line); }

int GameSession::enemyIndexAt(int x, int y) const {
    for (std::size_t i = 0; i < m_enemies.size(); ++i) {
        if (m_enemies[i].position.x == x && m_enemies[i].position.y == y) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int GameSession::worldObjectIndexAt(int x, int y) const {
    for (std::size_t i = 0; i < m_worldObjects.size(); ++i) {
        if (m_worldObjects[i].position.x == x && m_worldObjects[i].position.y == y) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool GameSession::tileBlocks(int x, int y) const {
    if (m_map == nullptr) {
        return false;  // sin mapa, nada bloquea (tests de solo objetos)
    }
    if (x < 0 || y < 0 || x >= m_map->getWidth() || y >= m_map->getHeight()) {
        return true;  // fuera del mapa: como un muro
    }
    // Cualquier capa con colision en esa celda bloquea (TileMap::getTile
    // lanza fuera de rango, pero el rango ya esta comprobado arriba).
    for (int layer = 0; layer < m_map->getLayerCount(); ++layer) {
        if (m_map->getTile(layer, x, y).hasCollision()) {
            return true;
        }
    }
    return false;
}

bool GameSession::tryMovePlayer(int dx, int dy) {
    m_lastPickupId.clear();
    if (m_mode != GameMode::Exploration) {
        return false;
    }
    const int nx = m_playerPosition.x + dx;
    const int ny = m_playerPosition.y + dy;

    if (tileBlocks(nx, ny)) {
        return false;
    }

    // Enemigo en el destino: no se entra, empieza el combate.
    int enemyIdx = enemyIndexAt(nx, ny);
    if (enemyIdx >= 0) {
        startBattle(enemyIdx);
        return false;
    }

    // Objeto en el destino: bloquea (prop) o se recoge (pickup).
    int objIdx = worldObjectIndexAt(nx, ny);
    if (objIdx >= 0) {
        const ObjectSpawn& spawn = m_worldObjects[static_cast<std::size_t>(objIdx)];
        const ObjectDefinition* def =
            m_catalog != nullptr ? m_catalog->find(spawn.objectId) : nullptr;
        if (def != nullptr && def->blocksMovement) {
            return false;
        }
        if (def != nullptr && def->category == ObjectCategory::Pickup) {
            if (m_inventory != nullptr) {
                m_inventory->push_back(spawn.objectId);
            }
            m_lastPickupId = spawn.objectId;
            logLine("Recoges " + def->name + ".");
            m_worldObjects.erase(m_worldObjects.begin() + objIdx);
        }
    }

    m_playerPosition = GridCoord{nx, ny};

    // El tiempo del juego son los pasos del jugador (ver el comentario
    // de m_stepsSinceRent): cada kStepsPerRentCycle celdas andadas se
    // cobra un ciclo de rentas de todo lo que poseas.
    if (!m_businesses.empty() && ++m_stepsSinceRent >= kStepsPerRentCycle) {
        m_stepsSinceRent = 0;
        collectRent();
    }
    return true;
}

void GameSession::update(float deltaTime) {
    // El mundo solo avanza en exploracion: durante un combate, un
    // dialogo o una compra, los enemigos no siguen patrullando por
    // detras (mismo criterio modal que tryMovePlayer, y evita que al
    // salir de una tienda te encuentres a un slime encima).
    if (m_mode != GameMode::Exploration) {
        return;
    }

    for (std::size_t i = 0; i < m_enemies.size(); ++i) {
        WorldEnemy& enemy = m_enemies[i];
        if (enemy.brain == nullptr || !enemy.brain->isAlive()) {
            continue;
        }

        // Que puede pisar el enemigo. Se responde con el MISMO criterio
        // de colision que el jugador (tileBlocks + objetos que bloquean)
        // en vez de reimplementarlo: si manana cambia que bloquea el
        // paso, cambia en un solo sitio.
        const std::size_t self = i;
        enemy.brain->update(deltaTime, [&](const GridCoord& cell) {
            if (tileBlocks(cell.x, cell.y)) {
                return false;
            }
            if (cell.x == m_playerPosition.x && cell.y == m_playerPosition.y) {
                return false;  // no se pisa al jugador: el combate lo inicia EL
            }
            const int objIdx = worldObjectIndexAt(cell.x, cell.y);
            if (objIdx >= 0) {
                const ObjectDefinition* def =
                    m_catalog != nullptr
                        ? m_catalog->find(m_worldObjects[static_cast<std::size_t>(objIdx)].objectId)
                        : nullptr;
                if (def != nullptr && def->blocksMovement) {
                    return false;
                }
            }
            // Ni encima de otro enemigo (dos slimes en la misma celda
            // son un solo slime a la vista, y el combate no sabria con
            // cual empezar).
            for (std::size_t j = 0; j < m_enemies.size(); ++j) {
                if (j != self && m_enemies[j].brain != nullptr && m_enemies[j].brain->isAlive() &&
                    m_enemies[j].position.x == cell.x && m_enemies[j].position.y == cell.y) {
                    return false;
                }
            }
            return true;
        });

        // La posicion autoritativa es la del brain; WorldEnemy::position
        // es su espejo para quien ya lo lee (minimapa, marcadores).
        enemy.position = enemy.brain->position();
    }
}

int GameSession::basePriceOf(const std::string& objectId) const {
    const ObjectDefinition* def = m_catalog != nullptr ? m_catalog->find(objectId) : nullptr;
    return def != nullptr ? def->pickup.price : 0;
}

void GameSession::addGold(int amount) {
    // Nunca por debajo de cero: un cobro mayor que el saldo es un bug de
    // quien llama, y dejar oro negativo lo propagaria en silencio por
    // toda la partida (mismo criterio que HudBar clampando su valor).
    m_gold = std::max(0, m_gold + amount);
}

void GameSession::startInteraction(const ObjectDefinition& def) {
    if (def.category != ObjectCategory::Npc) {
        return;
    }
    m_dialogueSpeaker = def.name;
    m_dialogueLines = def.dialogue.lines;

    // La narrativa MANDA sobre el dialogo fijo del catalogo: si la
    // aventura activa tiene algo que decir por boca de este NPC ahora
    // mismo, eso es lo que se dice; si no, las lineas de siempre.
    //
    // Este es el orden importante y no el contrario: hace que un NPC de
    // relleno siga funcionando sin tocarlo, y que ese MISMO NPC pase a
    // formar parte de una aventura con solo escribir un beat en un JSON
    // -- sin editar el catalogo de objetos ni el nivel.
    if (m_narrative != nullptr && m_narrativeState != nullptr) {
        const RPG::NarrativeResult narrated = m_narrative->talkTo(def.id, *m_narrativeState);
        if (narrated.fired) {
            // Un solo sitio para aplicar NarrativeResult (oro, log, y ahora
            // skillChecks Nd6): antes esto estaba duplicado aqui y en
            // applyNarrative, y la deuda tecnica saltaba justo ahora, al
            // anadir un tercer efecto diferido. applyNarrative deja
            // m_dialogueLines/m_dialogueSpeaker listos; mas abajo se decide
            // modo Dialogue/Shop.
            applyNarrative(narrated);
        }
    }

    // Oferta resuelta AHORA y no en cada consulta del HUD: el precio de
    // cada articulo puede venir de la tienda o del catalogo, y hacer esa
    // resolucion una vez al abrir evita que el HUD tenga que conocer esa
    // regla. Los articulos sin precio en ninguno de los dos sitios se
    // descartan: un articulo a coste 0 seria oro infinito al revenderlo.
    m_shopOffers.clear();
    for (const ShopItem& item : def.shop.items) {
        const ObjectDefinition* itemDef =
            m_catalog != nullptr ? m_catalog->find(item.objectId) : nullptr;
        if (itemDef == nullptr) {
            logLine("(articulo desconocido en la tienda: " + item.objectId + ")");
            continue;
        }
        const int price = item.price > 0 ? item.price : itemDef->pickup.price;
        if (price <= 0) {
            continue;
        }
        m_shopOffers.push_back(ShopOffer{item.objectId, itemDef->name, price});
    }

    m_mode = m_shopOffers.empty() ? GameMode::Dialogue : GameMode::Shop;
    for (const std::string& line : m_dialogueLines) {
        logLine(m_dialogueSpeaker + ": " + line);
    }
}

void GameSession::resolveSkillChecks(const std::vector<RPG::SkillCheckRequest>& checks) {
    // Una tirada Nd6 pedida desde un beat (GDD 7.1): N dados = stat del
    // jugador segun la skill implicada, CD del beat, grado -> flag. Es el
    // MISMO DicePoolEngine que usa el combate, asi que narrativa y combate
    // comparten reglas (coherencia pedida para todo el sistema).
    //
    // IMPORTANTE: esto convierte a GameSession en un segundo escritor de
    // flags narrativas (el primero es el propio NarrativeEngine con
    // setFlag/clearFlag). Es consistente con como ya pasaba con el oro
    // (GameSession es quien lo aplica, no el motor narrativo), y la regla
    // de oro sigue en pie: LAS FLAGS SON LA UNICA FUENTE DE VERDAD. Lo
    // unico que cambia es quien empuja el lapiz.
    if (m_narrativeState == nullptr) {
        return;  // Sin estado no hay donde escribir el resultado.
    }
    for (const RPG::SkillCheckRequest& c : checks) {
        const RPG::SkillDefinition* def =
            m_nd6Skills != nullptr ? m_nd6Skills->find(c.skillId) : nullptr;
        if (def == nullptr) {
            logLine("(skillCheck: skill Nd6 desconocida \"" + c.skillId +
                    "\", se ignora. Falta setNd6SkillCatalog?)");
            continue;
        }
        if (m_playerBody == nullptr) {
            logLine("(skillCheck: sin jugador para tirar \"" + c.skillId + "\", se ignora)");
            continue;
        }
        const int dice = m_playerBody->stat(def->casting_stat);
        const RPG::PoolResult pool =
            RPG::DicePoolEngine::roll_pool(dice, RPG::DiceMod::NORMAL, *m_rngInUse);
        const RPG::CheckOutcome out = RPG::DicePoolEngine::resolve_against_cd(pool, c.cd);
        switch (out.degree) {
            case RPG::Degree::BOTCH:    m_narrativeState->setFlag(c.flagBotch);    break;
            case RPG::Degree::PARTIAL:  m_narrativeState->setFlag(c.flagPartial);  break;
            case RPG::Degree::SUCCESS:  m_narrativeState->setFlag(c.flagSuccess);  break;
            case RPG::Degree::CRITICAL: m_narrativeState->setFlag(c.flagCritical); break;
        }
        logLine("(tirada " + c.skillId + " CD " + std::to_string(c.cd) + ": " +
                std::to_string(pool.successes) + " exitos -> " +
                std::to_string(static_cast<int>(out.degree)) + ")");
    }
}

void GameSession::resolveBattles(const std::vector<RPG::BattleRequest>& battles) {
    // Materializa al enemigo del catalogo y arranca el combate. Ver el
    // header para por que las flags no se encienden aqui.
    for (const RPG::BattleRequest& req : battles) {
        if (m_battle != nullptr || !m_pendingBattleVictoryFlag.empty()) {
            m_battleQueue.push_back(req);  // uno de cada vez; el resto espera
            continue;
        }
        const ObjectDefinition* def =
            m_catalog != nullptr ? m_catalog->find(req.monsterId) : nullptr;
        if (def == nullptr || def->category != ObjectCategory::Enemy) {
            // Un id roto no revienta la partida (mismo criterio permisivo
            // que LevelLoader), pero SI enciende la flag de derrota: si no,
            // la aventura se queda esperando para siempre una flag que
            // nadie va a poner, y el jugador no puede avanzar.
            logLine("(startBattle: enemigo \"" + req.monsterId +
                    "\" no esta en el catalogo, se da por perdido)");
            if (m_narrativeState != nullptr) {
                m_narrativeState->setFlag(req.flagDefeat);
            }
            continue;
        }

        // El enemigo aparece donde esta el jugador: lo ha emboscado la
        // narrativa, no estaba patrullando el nivel. patrolMin==patrolMax
        // ==posicion => quieto.
        WorldEnemy enemy;
        enemy.objectId = req.monsterId;
        enemy.position = m_playerPosition;
        enemy.brain = std::make_unique<EnemyBrain>(m_playerPosition, m_playerPosition,
                                                    m_playerPosition, def->combat.maxHealth);
        enemy.skills = std::make_unique<SkillSet>(def->combat.maxMana);
        for (const std::string& skillId : def->combat.skillIds) {
            enemy.skills->learn(skillId);
        }
        m_enemies.push_back(std::move(enemy));

        m_pendingBattleVictoryFlag = req.flagVictory;
        m_pendingBattleDefeatFlag = req.flagDefeat;
        startBattle(static_cast<int>(m_enemies.size()) - 1);
    }
}

bool GameSession::applyNarrative(const RPG::NarrativeResult& result) {
    if (!result.fired) {
        return false;
    }
    m_dialogueSpeaker = result.speaker;
    m_dialogueLines = result.lines;
    m_shopOffers.clear();
    m_mode = GameMode::Dialogue;

    addGold(result.goldDelta);
    resolveSkillChecks(result.skillChecks);
    resolveBattles(result.battles);
    for (const std::string& entry : result.log) {
        logLine(entry);
    }
    for (const std::string& line : m_dialogueLines) {
        // speaker vacio = voz del narrador: la linea va sola, sin "X: ".
        logLine(m_dialogueSpeaker.empty() ? line : m_dialogueSpeaker + ": " + line);
    }
    return true;
}

bool GameSession::enterLevelNarrative(const std::string& levelId) {
    if (m_narrative == nullptr || m_narrativeState == nullptr) {
        return false;
    }
    return applyNarrative(m_narrative->enterLevel(levelId, *m_narrativeState));
}

bool GameSession::talkNarrative(const std::string& npcId) {
    if (m_narrative == nullptr || m_narrativeState == nullptr) {
        return false;
    }
    return applyNarrative(m_narrative->talkTo(npcId, *m_narrativeState));
}

int GameSession::occupancyPercent(int rentPercent) {
    // Por debajo del alquiler justo se llena del todo (nadie se va
    // porque le cobres poco). Por encima, se pierden 0.4 puntos de
    // ocupacion por cada punto de subida: a 200% queda el 60%.
    //
    // El 0.4 no es arbitrario: con 0.5 exacto, ingreso = renta x
    // ocupacion daba EXACTAMENTE lo mismo a 200% que a 100%, asi que
    // exprimir a los inquilinos no aportaba ni un oro y solo costaba
    // moral -- una decision sin dilema, que es una decision muerta. Con
    // 0.4 la curva tiene un optimo economico alrededor del 175%: subir
    // hasta ahi RENTA de verdad (y por eso tienta), pasarse ya no.
    if (rentPercent <= 100) {
        return 100;
    }
    return std::max(0, 100 - (rentPercent - 100) * 2 / 5);
}

int GameSession::incomeFor(int baseIncome, int rentPercent) {
    // El producto se hace en un solo paso y con division final para no
    // perder precision por redondeos intermedios (a estos numeros
    // importa: 12 * 150 / 100 * 75 / 100 no es lo mismo segun el orden).
    const long long value =
        static_cast<long long>(baseIncome) * rentPercent * occupancyPercent(rentPercent);
    return static_cast<int>(value / 10000);
}

bool GameSession::ownsBusiness(const std::string& objectId) const {
    for (const OwnedBusiness& b : m_businesses) {
        if (b.objectId == objectId) {
            return true;
        }
    }
    return false;
}

bool GameSession::buyCurrentBusiness() {
    if (m_mode != GameMode::Business || m_currentBusinessId.empty()) {
        return false;
    }
    if (ownsBusiness(m_currentBusinessId)) {
        logLine("Ya es tuyo.");
        return false;
    }
    if (m_gold < m_currentBusinessPrice) {
        logLine("Te faltan " + std::to_string(m_currentBusinessPrice - m_gold) +
                " oro para comprar " + m_currentBusinessName + ".");
        return false;
    }
    m_gold -= m_currentBusinessPrice;
    m_businesses.push_back(
        OwnedBusiness{m_currentBusinessId, m_currentBusinessName, m_currentBusinessIncome, 100});
    logLine("Compras " + m_currentBusinessName + " por " + std::to_string(m_currentBusinessPrice) +
            " oro.");
    return true;
}

bool GameSession::adjustCurrentRent(int deltaPercent) {
    if (m_mode != GameMode::Business || m_currentBusinessId.empty()) {
        return false;
    }
    for (OwnedBusiness& b : m_businesses) {
        if (b.objectId != m_currentBusinessId) {
            continue;
        }
        const int before = b.rentPercent;
        b.rentPercent = std::clamp(b.rentPercent + deltaPercent, kMinRent, kMaxRent);
        const int applied = b.rentPercent - before;
        if (applied == 0) {
            return false;  // ya estaba en el tope
        }

        // La moral se mueve con el CAMBIO, no con el nivel absoluto: si
        // no, bastaria con subir y bajar en bucle para farmear
        // reputacion. Bajar el alquiler te acerca a heroe, subirlo a
        // villano, uno a uno por cada 10 puntos.
        // Un punto de moral por cada 5 de alquiler: con /10 el rango
        // entero (50%..200%) apenas movia 15 puntos de una escala de
        // 200, y el sistema no se notaba.
        const int moral = -applied / 5;
        const std::string que = applied > 0 ? "Subes" : "Bajas";
        m_morality.record(moral, que + " el alquiler de " + b.name + " al " +
                                     std::to_string(b.rentPercent) + "%");
        logLine(que + " el alquiler de " + b.name + " al " + std::to_string(b.rentPercent) +
                "% (ocupacion " + std::to_string(occupancyPercent(b.rentPercent)) + "%).");
        return true;
    }
    logLine("Ese negocio no es tuyo.");
    return false;
}

int GameSession::collectRent() {
    int total = 0;
    for (const OwnedBusiness& b : m_businesses) {
        total += incomeFor(b.baseIncome, b.rentPercent);

        // La moral tambien depende de lo que SOSTIENES, no solo de lo
        // que cambias: cada cobro con alquiler abusivo resta, y cada
        // cobro con alquiler generoso suma. Sin esto, el jugador podia
        // subir la renta, comerse el golpe de moral una vez y cobrar
        // caro para siempre a coste cero; ahora mantener un alquiler
        // injusto tiene un precio recurrente.
        const int sostenido = (100 - b.rentPercent) / 25;
        if (sostenido != 0) {
            m_morality.record(
                sostenido,
                (sostenido > 0 ? "Alquiler generoso en " : "Alquiler abusivo en ") + b.name);
        }
    }
    if (total > 0) {
        m_gold += total;
        logLine("Rentas cobradas: +" + std::to_string(total) + " oro.");
    }
    return total;
}

bool GameSession::interact() {
    if (m_mode != GameMode::Exploration) {
        return false;
    }

    // 1) La celda del jugador: una puerta se PISA (transicion).
    const int hereIdx = worldObjectIndexAt(m_playerPosition.x, m_playerPosition.y);
    if (hereIdx >= 0) {
        const ObjectSpawn& spawn = m_worldObjects[static_cast<std::size_t>(hereIdx)];
        if (!spawn.targetLevel.empty()) {
            m_transition.pending = true;
            m_transition.levelPath = spawn.targetLevel;
            m_transition.entryPosition = spawn.targetPosition;
            m_transition.hasEntryPosition = spawn.hasTargetPosition;
            const ObjectDefinition* def =
                m_catalog != nullptr ? m_catalog->find(spawn.objectId) : nullptr;
            logLine("Entras en " + (def != nullptr ? def->name : spawn.objectId) + ".");
            return true;
        }
    }

    // 2) La propia celda y las cuatro contiguas: NPC con el que hablar
    // o cartel de negocio que leer. Se incluye la celda del jugador
    // (offset 0,0) porque un cartel no bloquea el paso y es normal
    // acabar encima de el.
    const int dx[] = {0, 0, 0, -1, 1};
    const int dy[] = {0, -1, 1, 0, 0};
    for (int i = 0; i < 5; ++i) {
        const int idx = worldObjectIndexAt(m_playerPosition.x + dx[i], m_playerPosition.y + dy[i]);
        if (idx < 0) {
            continue;
        }
        const ObjectSpawn& spawn = m_worldObjects[static_cast<std::size_t>(idx)];
        const ObjectDefinition* def =
            m_catalog != nullptr ? m_catalog->find(spawn.objectId) : nullptr;
        if (def == nullptr) {
            continue;
        }
        if (def->category == ObjectCategory::Npc) {
            startInteraction(*def);
            return true;
        }
        if (def->business.price > 0) {
            m_currentBusinessId = def->id;
            m_currentBusinessName =
                def->business.businessName.empty() ? def->name : def->business.businessName;
            m_currentBusinessPrice = def->business.price;
            m_currentBusinessIncome = def->business.baseIncome;
            m_mode = GameMode::Business;
            return true;
        }
    }
    return false;
}

void GameSession::closeInteraction() {
    if (m_mode != GameMode::Dialogue && m_mode != GameMode::Shop && m_mode != GameMode::Business) {
        return;
    }
    m_mode = GameMode::Exploration;
    m_dialogueLines.clear();
    m_shopOffers.clear();
    m_dialogueSpeaker.clear();
    m_currentBusinessId.clear();
    m_currentBusinessName.clear();
    m_currentBusinessPrice = 0;
    m_currentBusinessIncome = 0;

    // Ya en Exploration: ¿tiene la aventura algo que decir por su
    // cuenta? Un solo tick por cierre (no un bucle while): un disparo,
    // un beat. Si un cierre encadena con otro, hace falta cerrar otra
    // vez -- que es como se lee en pantalla de todas formas, y evita
    // que un contenido mal escrito cuelgue el juego en un bucle
    // infinito de beats que se retroalimentan.
    if (m_narrative != nullptr && m_narrativeState != nullptr) {
        applyNarrative(m_narrative->tick(*m_narrativeState));
    }
}

bool GameSession::buy(std::size_t index) {
    if (m_mode != GameMode::Shop || index >= m_shopOffers.size()) {
        return false;
    }
    const ShopOffer& offer = m_shopOffers[index];
    if (m_gold < offer.price) {
        logLine("No te llega para " + offer.name + " (" + std::to_string(offer.price) + " oro).");
        return false;
    }
    // Cobrar y entregar en el mismo paso: sin estados intermedios donde
    // el oro ya se fue pero el objeto no ha llegado.
    m_gold -= offer.price;
    if (m_inventory != nullptr) {
        m_inventory->push_back(offer.objectId);
    }
    logLine("Compras " + offer.name + " por " + std::to_string(offer.price) + " oro.");
    return true;
}

bool GameSession::sell(std::size_t index) {
    if (m_mode != GameMode::Shop || m_inventory == nullptr || index >= m_inventory->size()) {
        return false;
    }
    const std::string objectId = (*m_inventory)[index];
    const int base = basePriceOf(objectId);
    if (base <= 0) {
        logLine("Eso no tiene precio aqui.");
        return false;
    }
    // El porcentaje de recompra es del NPC con el que se comercia; se
    // busca su definicion por NOMBRE porque es lo unico que se guardo al
    // abrir la tienda. Si no aparece, 50% (el valor por defecto de
    // ShopData): peor es no poder vender.
    int percent = 50;
    if (m_catalog != nullptr) {
        for (const ObjectSpawn& spawn : m_worldObjects) {
            const ObjectDefinition* def = m_catalog->find(spawn.objectId);
            if (def != nullptr && def->name == m_dialogueSpeaker &&
                def->category == ObjectCategory::Npc) {
                percent = def->shop.buybackPercent;
                break;
            }
        }
    }
    const int paid = std::max(1, base * percent / 100);
    m_gold += paid;
    const ObjectDefinition* def = m_catalog != nullptr ? m_catalog->find(objectId) : nullptr;
    logLine("Vendes " + (def != nullptr ? def->name : objectId) + " por " + std::to_string(paid) +
            " oro.");
    m_inventory->erase(m_inventory->begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

void GameSession::startBattle(int enemyIndex) {
    WorldEnemy& enemy = m_enemies[static_cast<std::size_t>(enemyIndex)];
    const ObjectDefinition* def = m_catalog != nullptr ? m_catalog->find(enemy.objectId) : nullptr;
    std::string enemyName = def != nullptr ? def->name : enemy.objectId;

    // El jugador entra en combate con SU cuerpo/habilidades/inventario
    // reales (no copias): lo que gasta o cura en combate se conserva al
    // volver a explorar, y la accion Item (Fase 10) consume del
    // inventario de verdad.
    std::vector<BattleParticipant> allies{{"Heroe", m_playerBody, m_playerSkills, m_inventory}};
    std::vector<BattleParticipant> enemies{
        {enemyName, enemy.brain.get(), enemy.skills.get(), nullptr}};

    m_battle = std::make_unique<BattleState>(std::move(allies), std::move(enemies), m_skillCatalog,
                                             m_catalog);
    m_battleEnemyIndex = enemyIndex;
    m_mode = GameMode::Battle;
    logLine("Un " + enemyName + " te corta el paso!");
}

bool GameSession::syncBattleOutcome() {
    if (m_battle == nullptr || m_battle->outcome() == BattleOutcome::InProgress) {
        return false;
    }
    const BattleOutcome outcome = m_battle->outcome();

    // El log del combate se copia ANTES de destruirlo (si no, se pierde
    // con el BattleState y el cuadro de dialogo se queda sin las lineas
    // del turno que acaba de terminar).
    for (const std::string& line : m_battle->log()) {
        m_log.push_back(line);
    }

    std::string enemyName;
    if (m_battleEnemyIndex >= 0 &&
        static_cast<std::size_t>(m_battleEnemyIndex) < m_enemies.size()) {
        const ObjectDefinition* def =
            m_catalog != nullptr
                ? m_catalog->find(m_enemies[static_cast<std::size_t>(m_battleEnemyIndex)].objectId)
                : nullptr;
        enemyName = def != nullptr
                        ? def->name
                        : m_enemies[static_cast<std::size_t>(m_battleEnemyIndex)].objectId;
    }

    switch (outcome) {
        case BattleOutcome::Victory:
            if (m_battleEnemyIndex >= 0) {
                m_enemies.erase(m_enemies.begin() + m_battleEnemyIndex);
            }
            logLine("Has derrotado al " + enemyName + ".");
            m_mode = GameMode::Exploration;
            break;
        case BattleOutcome::Fled:
            // El enemigo sigue en su celda: huir no lo mata (ver header).
            logLine("Escapas del " + enemyName + ".");
            m_mode = GameMode::Exploration;
            break;
        case BattleOutcome::Defeat:
            logLine("Has caido derrotado.");
            m_mode = GameMode::GameOver;
            break;
        case BattleOutcome::InProgress:
            break;  // inalcanzable (comprobado arriba)
    }

    // Flags del combate pedido por la narrativa (effect "startBattle").
    // Huir cuenta como derrota a efectos de trama: el enemigo sigue vivo y
    // lo que fuera a pasar, pasa. Si se quisiera distinguir haria falta una
    // tercera flag, y de momento ninguna aventura lo necesita.
    if (m_narrativeState != nullptr && !m_pendingBattleVictoryFlag.empty()) {
        const bool gano = (outcome == BattleOutcome::Victory);
        m_narrativeState->setFlag(gano ? m_pendingBattleVictoryFlag
                                       : m_pendingBattleDefeatFlag);
    }
    m_pendingBattleVictoryFlag.clear();
    m_pendingBattleDefeatFlag.clear();

    m_battle.reset();
    m_battleEnemyIndex = -1;

    // Siguiente combate encolado, si lo hay (oleadas).
    if (!m_battleQueue.empty() && m_mode != GameMode::GameOver) {
        std::vector<RPG::BattleRequest> siguiente{m_battleQueue.front()};
        m_battleQueue.erase(m_battleQueue.begin());
        resolveBattles(siguiente);
    }
    return true;
}
