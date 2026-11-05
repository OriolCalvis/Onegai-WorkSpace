// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 5 paquetes del corte B:
//   C-03  (fase C, 2 j) - L'estat del jugador sobreviu el canvi de nivell
//   C-04  (fase C, 2 j) - Un enemic invocat per narrativa no trenca el guardat
//   C-08  (fase C, 1 j) - Un guardat antic no escriu fora de rang
//   E-02  (fase E, 3 j) - Interruptors locals i variables
//   G-11  (fase G, 4 j) - Selector de guardats i recuperació de derrota
//
// Format i serialització de guardats. Extret de SaveGame.cpp (EST-29)
// per reduir-lo per sota de 500 línies. Consulta la nota llarga del
// header per al context complet.
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Game/SaveGame.h"
#include "Game/SaveGameValidation.h"

#include <algorithm>
#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "Game/ReglasMoral.h"  // MORAL-01

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

// GUARDAR-FICHA — Los textos del heroe (SavedHero), con su clave: los mismos
// para escribir y para leer.
struct CampoHeroe {
    const char* clave;
    std::string SavedHero::*campo;
};
const CampoHeroe kIdsDelHeroe[] = {
    {"raceId", &SavedHero::raceId},       {"classId", &SavedHero::classId},
    {"backgroundId", &SavedHero::backgroundId}, {"deityId", &SavedHero::deityId},
    {"bondId", &SavedHero::bondId},       {"fearId", &SavedHero::fearId},
    {"flawId", &SavedHero::flawId},       {"goalId", &SavedHero::goalId},
    {"idealId", &SavedHero::idealId},     {"personalityId", &SavedHero::personalityId},
    {"virtueId", &SavedHero::virtueId}};
const CampoHeroe kAspectoDelHeroe[] = {
    {"raza", &SavedHero::raza},           {"piel", &SavedHero::piel},
    {"talla", &SavedHero::talla},         {"ojos", &SavedHero::ojos},
    {"peloEstilo", &SavedHero::peloEstilo}, {"peloColor", &SavedHero::peloColor},
    {"barbaEstilo", &SavedHero::barbaEstilo}, {"barbaColor", &SavedHero::barbaColor}};

void writeHero(JsonWriter& out, const SavedHero& heroe) {
    out.key("hero");
    out.beginObject();
    for (const CampoHeroe& c : kIdsDelHeroe) out.member(c.clave, heroe.*c.campo);
    writeStringArray(out, "equipped", heroe.equipped);
    out.key("appearance");
    out.beginObject();
    for (const CampoHeroe& c : kAspectoDelHeroe) out.member(c.clave, heroe.*c.campo);
    out.key("ropa");
    out.beginObject();
    for (const auto& [ranura, pieza] : heroe.ropa) out.member(ranura, pieza);
    out.endObject();
    writeStringArray(out, "quitarCapas", heroe.quitarCapas);
    if (!heroe.marcas.empty())  // EDITOR-PERSONALIZACION-01: aditivo, un guardado de antes no lo trae
        writeStringArray(out, "marcas", heroe.marcas);
    out.endObject();
    if (!heroe.reputacion.empty()) {  // AV14-23: solo si alguna faccion te ha visto
        out.key("reputacion");
        out.beginObject();
        for (const auto& [faccion, valor] : heroe.reputacion) out.member(faccion, valor);
        out.endObject();
    }
    out.endObject();
}

SavedHero readHero(const JsonValue& node) {
    SavedHero heroe;
    if (!node.isObject()) return heroe;
    for (const CampoHeroe& c : kIdsDelHeroe) heroe.*c.campo = node[c.clave].asString("");
    heroe.equipped = readStringArray(node["equipped"]);
    const JsonValue& aspecto = node["appearance"];
    for (const CampoHeroe& c : kAspectoDelHeroe) heroe.*c.campo = aspecto[c.clave].asString("");
    if (aspecto["ropa"].isObject()) {
        for (const auto& [ranura, pieza] : aspecto["ropa"].objectValues()) heroe.ropa[ranura] = pieza.asString("");
    }
    heroe.quitarCapas = readStringArray(aspecto["quitarCapas"]);
    heroe.marcas = readStringArray(aspecto["marcas"]);
    if (node["reputacion"].isObject()) {  // AV14-23
        for (const auto& [faccion, valor] : node["reputacion"].objectValues()) heroe.reputacion[faccion] = valor.asInt(0);
    }
    return heroe;
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

    // G-11 — aditivas: solo se escriben cuando estan puestas.
    if (data.playerLevel > 1) {
        out.member("playerLevel", data.playerLevel);
    }
    if (data.playerLevelProgress > 0) {
        out.member("playerLevelProgress", data.playerLevelProgress);
    }
    if (data.playerTier > 1) {
        out.member("playerTier", data.playerTier);
    }
    if (data.playerTierMilestones > 0) {
        out.member("playerTierMilestones", data.playerTierMilestones);
    }
    if (data.playerMilestones > 0) {
        out.member("playerMilestones", data.playerMilestones);
    }
    if (!data.characterName.empty()) {
        out.member("characterName", data.characterName);
    }
    if (!data.savedAt.empty()) {
        out.member("savedAt", data.savedAt);
    }
    if (!data.hero.empty()) {
        writeHero(out, data.hero);  // GUARDAR-FICHA
    }
    if (!data.returnLevel.empty()) {  // PUERTAS-01
        out.member("returnLevel", data.returnLevel);
        writeGridCoord(out, "returnDoor", data.returnDoor);
    }
    Game::escribeMarcasMapa(out, data.marcasMapa);  // MAPA-02: solo si hay algo marcado
    if (data.combate.hay()) {  // D0033
        out.key("combate");
        out.beginObject();
        out.key("enemigos");
        out.beginArray();
        for (const CombateGuardado::Enemigo& e : data.combate.enemigos) {
            out.beginObject();
            out.member("summoned", e.summoned);
            out.member("spawnIndex", e.spawnIndex);
            out.member("objectId", e.objectId);
            out.endObject();
        }
        out.endArray();
        out.key("aLado");
        out.beginArray();
        for (const std::string& id : data.combate.aLado) out.value(id);
        out.endArray();
        out.member("pedidos", data.combate.pedidos);
        out.endObject();
    }

    writeStringArray(out, "inventory", data.inventory);
    out.member("gold", data.gold);
    out.member("morality", data.moralityValue);
    if (!data.moral.porDefecto()) {  // MORAL-01: solo si hay algo, para que los de antes salgan igual
        out.key("moral");
        moral::escribeEstado(out, data.moral);
    }
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
    // E-02 — Variables. Se escriben siempre, aunque esten vacias: "sin
    // variables" y "clave ausente" tienen que leerse igual, y escribirla
    // siempre hace el fichero autoexplicativo para quien lo abra a mano.
    out.key("variables");
    out.beginObject();
    for (const auto& variable : data.narrativeVariables) {
        out.member(variable.first, variable.second);
    }
    out.endObject();
    // E-02 — Interruptores locales. Solo los ENCENDIDOS: un evento que no
    // tiene ninguno no ocupa sitio. Las letras van como array de strings
    // de un caracter en vez de una mascara de bits, para que el guardado
    // se pueda leer y corregir a mano cuando algo va mal.
    out.key("localSwitches");
    out.beginArray();
    for (const RPG::NarrativeState::LocalSwitches& evento : data.narrativeLocalSwitches) {
        out.beginObject();
        out.member("event", evento.eventId);
        out.key("on");
        out.beginArray();
        for (char letra : evento.on) {
            out.value(std::string(1, letra));
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
    if (!root.has("schemaVersion")) {
        return fail(error, "no parece un guardado: falta 'schemaVersion'");
    }
    if (!Game::SaveGameValidation::expect(root["schemaVersion"],
                                         Game::SaveGameValidation::Kind::Integer,
                                         "schemaVersion", error)) return false;
    const int version = root["schemaVersion"].asInt(0);
    if (version != SaveGameData::kSchemaVersion) {
        return fail(error, "version de guardado incompatible: el fichero es v" +
                               std::to_string(version) + " y este motor lee v" +
                               std::to_string(SaveGameData::kSchemaVersion));
    }

    if (!root["levelPath"].isString() || root["levelPath"].asString().empty()) {
        return fail(error, "guardado sin 'levelPath': no se sabe que nivel cargar");
    }
    if (!Game::SaveGameValidation::optionalFields(root, error)) return false;

    // Todos los campos presentes ya tienen tipos y rangos representables.
    // Los ausentes mantienen los defaults de v1; la copia se entrega al final.
    SaveGameData data;
    data.schemaVersion = version;
    data.levelPath = root["levelPath"].asString();

    data.playerPosition = readGridCoord(root["playerPosition"]);
    data.playerHealth = root["playerHealth"].asInt(0);
    data.playerMaxHealth = root["playerMaxHealth"].asInt(0);
    data.playerLevel = std::max(1, root["playerLevel"].asInt(1));
    data.playerLevelProgress = std::max(0, root["playerLevelProgress"].asInt(0));
    data.playerTier = std::max(1, root["playerTier"].asInt(1));
    data.playerTierMilestones = std::max(0, root["playerTierMilestones"].asInt(0));
    data.playerMilestones = std::max(0, root["playerMilestones"].asInt(0));

    data.characterName = root["characterName"].asString("");
    data.savedAt = root["savedAt"].asString("");
    data.hero = readHero(root["hero"]);  // GUARDAR-FICHA: sin clave, vacio
    data.returnLevel = root["returnLevel"].asString("");  // PUERTAS-01: sin clave, vacio
    data.returnDoor = readGridCoord(root["returnDoor"]);
    data.marcasMapa = Game::leeMarcasMapa(root["mapa"]);  // MAPA-02: sin clave, sin marcas
    const JsonValue& combate = root["combate"];  // D0033: sin clave, sin combate
    for (std::size_t i = 0; i < combate["enemigos"].size(); ++i) {
        const JsonValue& e = combate["enemigos"][i];
        data.combate.enemigos.push_back({e["summoned"].asBool(false), e["spawnIndex"].asInt(-1), e["objectId"].asString("")});
    }
    for (std::size_t i = 0; i < combate["aLado"].size(); ++i) data.combate.aLado.push_back(combate["aLado"][i].asString(""));
    data.combate.pedidos = combate["pedidos"].asString("");

    data.inventory = readStringArray(root["inventory"]);
    data.gold = root["gold"].asInt(0);
    data.moralityValue = root["morality"].asInt(0);
    data.moral = moral::leeEstado(root["moral"]);  // MORAL-01
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

    // E-02 — Opcionales a proposito: un guardado anterior a E-02 no las
    // trae y tiene que entrar igual, con las dos vacias.
    const JsonValue& variables = narrative["variables"];
    if (variables.isObject()) {
        for (const auto& variable : variables.objectValues()) {
            if (variable.second.isNumber()) {
                data.narrativeVariables[variable.first] = variable.second.asInt(0);
            }
        }
    }
    const JsonValue& localSwitches = narrative["localSwitches"];
    if (localSwitches.isArray()) {
        for (std::size_t i = 0; i < localSwitches.size(); ++i) {
            const JsonValue& node = localSwitches[i];
            RPG::NarrativeState::LocalSwitches evento;
            evento.eventId = node["event"].asString();
            if (evento.eventId.empty()) {
                continue;   // sin id no hay a quien devolverselos
            }
            const JsonValue& on = node["on"];
            for (std::size_t j = 0; j < on.size(); ++j) {
                const std::string letra = on[j].asString();
                if (letra.size() == 1) {
                    evento.on.push_back(letra[0]);
                }
            }
            data.narrativeLocalSwitches.push_back(std::move(evento));
        }
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
    // E-02 — Variables e interruptores locales. Sin esto el guardado los
    // lleva y cargar la partida los tira: el contador de hierbas volveria
    // a cero y el cofre se podria abrir otra vez.
    for (const auto& variable : data.narrativeVariables) {
        state.setVariable(variable.first, variable.second);
    }
    for (const RPG::NarrativeState::LocalSwitches& evento : data.narrativeLocalSwitches) {
        for (char letra : evento.on) {
            // Una letra corrupta en el fichero se ignora en vez de tumbar
            // la carga: perder un interruptor es malo, no poder cargar la
            // partida es peor.
            state.setLocalSwitch(evento.eventId, letra, true);
        }
    }
}
