#include "Game/SaveGame.h"

// C-04: restoreSaveGame consulta el catalogo para revalidar los enemigos
// invocados antes de volver a materializarlos.
#include "Level/ObjectCatalog.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <utility>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "Game/EnemyBrain.h"

namespace {

void writeGridCoord(JsonWriter& out, const std::string& name, const GridCoord& coord) {
    out.key(name);
    out.beginObject();
    out.member("x", coord.x);
    out.member("y", coord.y);
    out.endObject();
}

GridCoord readGridCoord(const JsonValue& node) {
    GridCoord coord;
    coord.x = node["x"].asInt(0);
    coord.y = node["y"].asInt(0);
    return coord;
}

void writeStringArray(JsonWriter& out, const std::string& name,
                      const std::vector<std::string>& values) {
    out.key(name);
    out.beginArray();
    for (const std::string& value : values) {
        out.value(value);
    }
    out.endArray();
}

std::vector<std::string> readStringArray(const JsonValue& node) {
    std::vector<std::string> values;
    values.reserve(node.size());
    for (std::size_t i = 0; i < node.size(); ++i) {
        values.push_back(node[i].asString());
    }
    return values;
}

bool fail(std::string* error, const std::string& message) {
    if (error != nullptr) {
        *error = message;
    }
    return false;
}

}  // namespace

// ---------------------------------------------------------------------
// RUN-10.1 — Formato
// ---------------------------------------------------------------------

std::string serializeSaveGame(const SaveGameData& data) {
    JsonWriter out;
    out.beginObject();

    out.member("schemaVersion", data.schemaVersion);
    out.member("levelPath", data.levelPath);

    writeGridCoord(out, "playerPosition", data.playerPosition);
    out.member("playerHealth", data.playerHealth);
    out.member("playerMaxHealth", data.playerMaxHealth);

    writeStringArray(out, "inventory", data.inventory);
    out.member("gold", data.gold);
    out.member("morality", data.moralityValue);
    out.member("stepsSinceRent", data.stepsSinceRent);

    out.key("businesses");
    out.beginArray();
    for (const OwnedBusiness& business : data.businesses) {
        out.beginObject();
        out.member("objectId", business.objectId);
        out.member("name", business.name);
        out.member("baseIncome", business.baseIncome);
        out.member("rentPercent", business.rentPercent);
        out.endObject();
    }
    out.endArray();

    out.key("worldObjects");
    out.beginArray();
    for (const SavedWorldObject& object : data.worldObjects) {
        out.beginObject();
        out.member("objectId", object.objectId);
        writeGridCoord(out, "position", object.position);
        out.endObject();
    }
    out.endArray();

    out.key("enemies");
    out.beginArray();
    for (const SavedEnemy& enemy : data.enemies) {
        out.beginObject();
        out.member("spawnIndex", enemy.spawnIndex);
        out.member("objectId", enemy.objectId);
        writeGridCoord(out, "position", enemy.position);
        out.member("health", enemy.health);
        out.member("summoned", enemy.summoned);  // C-04
        out.endObject();
    }
    out.endArray();

    out.key("narrative");
    out.beginObject();
    writeStringArray(out, "flags", data.narrativeFlags);
    writeStringArray(out, "beatsFired", data.narrativeBeatsFired);
    out.key("timers");
    out.beginArray();
    for (const RPG::TimerState& timer : data.narrativeTimers) {
        out.beginObject();
        out.member("id", timer.id);
        out.member("restante", timer.restante);
        out.member("activo", timer.activo);
        out.member("expirado", timer.expirado);
        out.key("avisosDados");
        out.beginArray();
        for (bool aviso : timer.avisosDados) {
            out.value(aviso);
        }
        out.endArray();
        out.endObject();
    }
    out.endArray();
    out.endObject();  // narrative

    out.endObject();  // raiz
    return out.str();
}

bool parseSaveGame(const std::string& text, SaveGameData* out, std::string* error) {
    if (out == nullptr) {
        return fail(error, "parseSaveGame: destino nulo");
    }

    Result<JsonValue> parsed = JsonValue::parse(text);
    if (!parsed.isOk()) {
        return fail(error, "guardado ilegible: " + parsed.errorMessage());
    }

    const JsonValue& root = parsed.value();
    if (!root.isObject()) {
        return fail(error, "guardado ilegible: la raiz no es un objeto JSON");
    }

    // RUN-10.4 — La version se comprueba ANTES de tocar nada. Se rechaza
    // tanto la ausente (un JSON cualquiera que no es un guardado) como la
    // distinta: leer un esquema futuro con el codigo viejo produciria una
    // partida silenciosamente incompleta.
    if (!root.has("schemaVersion") || !root["schemaVersion"].isNumber()) {
        return fail(error, "no parece un guardado: falta 'schemaVersion'");
    }
    const int version = root["schemaVersion"].asInt(0);
    if (version != SaveGameData::kSchemaVersion) {
        return fail(error, "version de guardado incompatible: el fichero es v" +
                               std::to_string(version) + " y este motor lee v" +
                               std::to_string(SaveGameData::kSchemaVersion));
    }

    if (!root["levelPath"].isString() || root["levelPath"].asString().empty()) {
        return fail(error, "guardado sin 'levelPath': no se sabe que nivel cargar");
    }

    // A partir de aqui ya no se puede fallar: se rellena una copia local y
    // solo al final se entrega, para no dejar "out" a medias.
    SaveGameData data;
    data.schemaVersion = version;
    data.levelPath = root["levelPath"].asString();

    data.playerPosition = readGridCoord(root["playerPosition"]);
    data.playerHealth = root["playerHealth"].asInt(0);
    data.playerMaxHealth = root["playerMaxHealth"].asInt(0);

    data.inventory = readStringArray(root["inventory"]);
    data.gold = root["gold"].asInt(0);
    data.moralityValue = root["morality"].asInt(0);
    data.stepsSinceRent = root["stepsSinceRent"].asInt(0);

    const JsonValue& businesses = root["businesses"];
    for (std::size_t i = 0; i < businesses.size(); ++i) {
        const JsonValue& node = businesses[i];
        OwnedBusiness business;
        business.objectId = node["objectId"].asString();
        business.name = node["name"].asString();
        business.baseIncome = node["baseIncome"].asInt(0);
        business.rentPercent = node["rentPercent"].asInt(100);
        data.businesses.push_back(business);
    }

    const JsonValue& objects = root["worldObjects"];
    for (std::size_t i = 0; i < objects.size(); ++i) {
        const JsonValue& node = objects[i];
        SavedWorldObject object;
        object.objectId = node["objectId"].asString();
        object.position = readGridCoord(node["position"]);
        data.worldObjects.push_back(object);
    }

    const JsonValue& enemies = root["enemies"];
    for (std::size_t i = 0; i < enemies.size(); ++i) {
        const JsonValue& node = enemies[i];
        SavedEnemy enemy;
        enemy.spawnIndex = node["spawnIndex"].asInt(-1);
        enemy.objectId = node["objectId"].asString();
        enemy.position = readGridCoord(node["position"]);
        enemy.health = node["health"].asInt(0);
        // C-04 — Ausente en guardados anteriores: false, es decir "del
        // nivel", que es lo unico que habia entonces.
        enemy.summoned = node["summoned"].asBool(false);
        data.enemies.push_back(enemy);
    }

    const JsonValue& narrative = root["narrative"];
    data.narrativeFlags = readStringArray(narrative["flags"]);
    data.narrativeBeatsFired = readStringArray(narrative["beatsFired"]);
    const JsonValue& timers = narrative["timers"];
    for (std::size_t i = 0; i < timers.size(); ++i) {
        const JsonValue& node = timers[i];
        RPG::TimerState timer;
        timer.id = node["id"].asString();
        timer.restante = static_cast<float>(node["restante"].asNumber(0.0));
        timer.activo = node["activo"].asBool(false);
        timer.expirado = node["expirado"].asBool(false);
        const JsonValue& avisos = node["avisosDados"];
        for (std::size_t j = 0; j < avisos.size(); ++j) {
            timer.avisosDados.push_back(avisos[j].asBool(false));
        }
        data.narrativeTimers.push_back(timer);
    }

    *out = data;
    return true;
}

void applyNarrativeState(const SaveGameData& data, RPG::NarrativeState& state) {
    for (const std::string& flag : data.narrativeFlags) {
        state.setFlag(flag);
    }
    for (const std::string& beat : data.narrativeBeatsFired) {
        state.markBeatFired(beat);
    }
    for (const RPG::TimerState& timer : data.narrativeTimers) {
        RPG::TimerState& live = state.timerRef(timer.id);
        live.restante = timer.restante;
        live.activo = timer.activo;
        live.expirado = timer.expirado;
        // C-08 — Se copia tal cual, con el tamano que tuviera el
        // guardado: aqui no hay AdventureScript con el que compararlo.
        // Quien conoce la definicion viva es NarrativeEngine::tickTimers,
        // y es alli donde el vector se ajusta antes de indexarlo.
        live.avisosDados = timer.avisosDados;
    }
}

// ---------------------------------------------------------------------
// RUN-10.3 — Fichero
// ---------------------------------------------------------------------

bool writeSaveGameFile(const std::string& path, const SaveGameData& data, std::string* error) {
    if (path.empty()) {
        return fail(error, "ruta de guardado vacia");
    }

    const std::string temporary = path + ".tmp";
    {
        // Binario a proposito: en Windows el modo texto convertiria cada
        // '\n' en "\r\n" y el fichero dejaria de ser byte a byte el mismo
        // en las tres plataformas (y el round-trip de los tests, tambien).
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file) {
            return fail(error, "no se puede escribir en " + temporary);
        }
        const std::string text = serializeSaveGame(data);
        file.write(text.data(), static_cast<std::streamsize>(text.size()));
        file.flush();
        if (!file) {
            return fail(error, "fallo al escribir " + temporary);
        }
    }  // el ofstream se cierra aqui: el rename no puede adelantarse al cierre

    // std::rename no sobreescribe en Windows si el destino existe, asi que
    // se retira antes. En POSIX rename() ya es atomico y remove() sobre un
    // fichero inexistente es inocuo, de modo que el mismo codigo vale para
    // los tres sistemas.
    std::remove(path.c_str());
    if (std::rename(temporary.c_str(), path.c_str()) != 0) {
        std::remove(temporary.c_str());
        return fail(error, "no se puede sustituir " + path);
    }
    return true;
}

bool readSaveGameFile(const std::string& path, SaveGameData* out, std::string* error) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return fail(error, "no se puede abrir " + path);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return parseSaveGame(buffer.str(), out, error);
}

// ---------------------------------------------------------------------
// RUN-10.2 — Captura y restauracion sobre una GameSession
//
// Viven aqui y no en GameSession.cpp para que el guardado sea UN solo
// sitio: formato, fichero y traduccion desde/hacia el estado vivo. Quien
// busque "que se guarda" no tiene que leer las 760 lineas de la sesion.
// ---------------------------------------------------------------------

SaveGameData GameSession::captureSaveGame(const std::string& levelPath) const {
    SaveGameData data;
    data.levelPath = levelPath;

    data.playerPosition = m_playerPosition;
    if (m_playerBody != nullptr) {
        data.playerHealth = m_playerBody->health();
        data.playerMaxHealth = m_playerBody->maxHealth();
    }
    if (m_inventory != nullptr) {
        data.inventory = *m_inventory;
    }

    // C-03 — El guardado lee el estado del jugador por el mismo camino
    // que el cambio de nivel: si manana se anade un campo a
    // PlayerCarryOver, aparece en los dos sitios o en ninguno.
    const PlayerCarryOver carried = captureCarryOver();
    data.gold = carried.gold;
    data.moralityValue = carried.moralityValue;
    data.stepsSinceRent = carried.stepsSinceRent;
    data.businesses = carried.businesses;

    // Solo lo que SIGUE en el mundo: lo recogido y lo derrotado se
    // reconoce al cargar porque no aparece en la lista.
    data.worldObjects.reserve(m_worldObjects.size());
    for (const ObjectSpawn& spawn : m_worldObjects) {
        SavedWorldObject object;
        object.objectId = spawn.objectId;
        object.position = spawn.position;
        data.worldObjects.push_back(object);
    }

    data.enemies.reserve(m_enemies.size());
    for (const WorldEnemy& source : m_enemies) {
        SavedEnemy enemy;
        enemy.spawnIndex = source.spawnIndex;
        enemy.summoned = source.summoned;  // C-04
        enemy.objectId = source.objectId;
        enemy.position = source.brain != nullptr ? source.brain->position() : source.position;
        enemy.health = source.brain != nullptr ? source.brain->health() : 0;
        data.enemies.push_back(enemy);
    }

    if (m_narrativeState != nullptr) {
        data.narrativeFlags = m_narrativeState->flags();
        data.narrativeBeatsFired = m_narrativeState->beatsFired();
        data.narrativeTimers = m_narrativeState->timers();
    }

    return data;
}

bool GameSession::restoreSaveGame(const SaveGameData& data, std::string* error) {
    // --- Fase 1: VALIDAR sin escribir nada ---
    //
    // Las dos listas se resuelven enteras antes de tocar la sesion. Si el
    // nivel ha cambiado (se anadio o quito un objeto en el JSON), aplicar
    // el guardado pondria la vida de un lobo en un barril: preferimos
    // negarnos y decir por que.
    std::vector<std::size_t> objectSlots;
    objectSlots.reserve(data.worldObjects.size());
    for (const SavedWorldObject& object : data.worldObjects) {
        const int slot = worldObjectIndexAt(object.position.x, object.position.y);
        if (slot < 0) {
            return fail(error, "el guardado no encaja con el nivel: no hay ningun objeto en (" +
                                   std::to_string(object.position.x) + "," +
                                   std::to_string(object.position.y) + ")");
        }
        if (m_worldObjects[static_cast<std::size_t>(slot)].objectId != object.objectId) {
            return fail(error, "el nivel ha cambiado desde que se guardo: se esperaba '" +
                                   object.objectId + "' en (" + std::to_string(object.position.x) +
                                   "," + std::to_string(object.position.y) + ")");
        }
        objectSlots.push_back(static_cast<std::size_t>(slot));
    }

    // C-04 — Marca para "este no sale de m_enemies porque no estaba ahi":
    // un invocado por la narrativa no existe en una sesion recien
    // construida, hay que volver a materializarlo del catalogo.
    const std::size_t kSummoned = static_cast<std::size_t>(-1);

    std::vector<std::size_t> enemySlots;
    enemySlots.reserve(data.enemies.size());
    for (const SavedEnemy& enemy : data.enemies) {
        if (enemy.summoned) {
            // Lo unico que se puede validar de un invocado es que su
            // definicion siga existiendo: si alguien borro el monstruo
            // del catalogo entre partida y partida, restaurar lo
            // colocaria como un enemigo sin cuerpo.
            const ObjectDefinition* def =
                m_catalog != nullptr ? m_catalog->find(enemy.objectId) : nullptr;
            if (def == nullptr || def->category != ObjectCategory::Enemy) {
                return fail(error, "el guardado trae un enemigo invocado que ya no esta en el "
                                   "catalogo: '" + enemy.objectId + "'");
            }
            enemySlots.push_back(kSummoned);
            continue;
        }

        std::size_t slot = m_enemies.size();
        for (std::size_t i = 0; i < m_enemies.size(); ++i) {
            if (!m_enemies[i].summoned && m_enemies[i].spawnIndex == enemy.spawnIndex) {
                slot = i;
                break;
            }
        }
        if (slot == m_enemies.size()) {
            return fail(error, "el guardado no encaja con el nivel: enemigo fuera de rango (" +
                                   std::to_string(enemy.spawnIndex) + ")");
        }
        if (m_enemies[slot].objectId != enemy.objectId) {
            return fail(error, "el nivel ha cambiado desde que se guardo: se esperaba '" +
                                   enemy.objectId + "' en la posicion " +
                                   std::to_string(enemy.spawnIndex) + " del nivel");
        }
        enemySlots.push_back(slot);
    }

    // --- Fase 2: aplicar ---

    std::vector<ObjectSpawn> remainingObjects;
    remainingObjects.reserve(objectSlots.size());
    for (std::size_t slot : objectSlots) {
        remainingObjects.push_back(m_worldObjects[slot]);
    }
    m_worldObjects = std::move(remainingObjects);

    std::vector<WorldEnemy> remainingEnemies;
    remainingEnemies.reserve(enemySlots.size());
    for (std::size_t i = 0; i < enemySlots.size(); ++i) {
        if (enemySlots[i] == kSummoned) {
            continue;  // C-04: se materializan abajo, sobre m_enemies ya asignado
        }
        WorldEnemy& live = m_enemies[enemySlots[i]];
        const SavedEnemy& saved = data.enemies[i];
        if (live.brain != nullptr) {
            live.brain->setPosition(saved.position);
            live.brain->set_health(saved.health);
        }
        live.position = saved.position;
        remainingEnemies.push_back(std::move(live));
    }
    m_enemies = std::move(remainingEnemies);

    // C-04 — Los invocados que seguian vivos al guardar vuelven a
    // levantarse del catalogo, con su posicion y la vida que les
    // quedaba. Van al final de la lista y no en su sitio original: el
    // orden de m_enemies no es identidad de nadie (para eso estan
    // spawnIndex/summoned), solo orden de dibujado y de turno.
    //
    // m_nextSummonIndex se lleva por encima del mayor ordinal
    // restaurado, para que la siguiente emboscada de la partida no
    // reutilice la identidad de uno que ya esta en el mundo.
    for (const SavedEnemy& saved : data.enemies) {
        if (!saved.summoned) {
            continue;
        }
        spawnEnemyFromCatalog(saved.objectId, saved.position, /*summoned=*/true,
                              saved.spawnIndex, saved.health);
        if (saved.spawnIndex >= m_nextSummonIndex) {
            m_nextSummonIndex = saved.spawnIndex + 1;
        }
    }

    // Jugador. ICombatant no tiene "poner vida a X" a proposito (la vida
    // se pierde o se cura, no se asigna): se llega al valor guardado
    // curando al maximo y aplicando el dano que falte. Es la unica forma
    // de restaurar sin abrir un setter que el combate podria usar mal.
    m_playerPosition = data.playerPosition;
    if (m_playerBody != nullptr && data.playerMaxHealth > 0) {
        m_playerBody->heal(m_playerBody->maxHealth());
        const int missing = m_playerBody->maxHealth() - data.playerHealth;
        if (missing > 0) {
            m_playerBody->takeDamage(missing);
        }
    }
    if (m_inventory != nullptr) {
        *m_inventory = data.inventory;
    }

    PlayerCarryOver carried;
    carried.gold = data.gold;
    carried.moralityValue = data.moralityValue;
    carried.stepsSinceRent = data.stepsSinceRent;
    carried.businesses = data.businesses;
    applyCarryOver(carried);

    // Narrativa: solo se ENCIENDE lo guardado. No se llama a reset()
    // porque restaurar se hace sobre una sesion recien construida y
    // borraria tambien lo que la Application hubiera preparado antes.
    if (m_narrativeState != nullptr) {
        applyNarrativeState(data, *m_narrativeState);
    }

    // Un combate en curso no sobrevive al guardado (ver captureSaveGame):
    // se vuelve a exploracion, salvo que el jugador este muerto.
    m_battle.reset();
    m_battleEnemyIndex = -1;
    m_dialogueLines.clear();
    m_dialogueSpeaker.clear();
    m_shopOffers.clear();
    m_transition = LevelTransition{};
    m_mode = (m_playerBody != nullptr && m_playerBody->isAlive()) ? GameMode::Exploration
                                                                 : GameMode::GameOver;
    return true;
}
