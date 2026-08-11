#include "RPG/NarrativeEngine.h"

#include "Core/Json/JsonValue.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace RPG {

namespace {

// Lista de strings de un array JSON, saltando los vacios. Se usa para
// las tres listas de NarrativeCondition y para las lineas de dialogo.
std::vector<std::string> readStringArray(const JsonValue& value) {
    std::vector<std::string> out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        std::string s = value[i].asString();
        if (!s.empty()) {
            out.push_back(std::move(s));
        }
    }
    return out;
}

// "talk"/"enter"/"auto" -> TriggerType. Criterio ESTRICTO (devuelve
// false en vez de un default) igual que ObjectCatalog::parseCategory: un
// "tak" por error tipografico tiene que ser un error de carga ruidoso, no
// un beat que silenciosamente nunca se dispara -- ese es el bug de
// contenido mas caro de encontrar que existe en un sistema narrativo.
bool parseTriggerType(const std::string& text, TriggerType& out) {
    if (text == "talk") {
        out = TriggerType::Talk;
        return true;
    }
    if (text == "enter") {
        out = TriggerType::Enter;
        return true;
    }
    if (text == "auto") {
        out = TriggerType::Auto;
        return true;
    }
    return false;
}

bool parseEffectType(const std::string& text, EffectType& out) {
    if (text == "setFlag") {
        out = EffectType::SetFlag;
        return true;
    }
    if (text == "clearFlag") {
        out = EffectType::ClearFlag;
        return true;
    }
    if (text == "grantGold") {
        out = EffectType::GrantGold;
        return true;
    }
    if (text == "log") {
        out = EffectType::Log;
        return true;
    }
    return false;
}

}  // namespace

// ---------------------------------------------------------------------
// NarrativeState
// ---------------------------------------------------------------------

std::vector<std::string> NarrativeState::flags() const {
    std::vector<std::string> out(m_flags.begin(), m_flags.end());
    // Ordenado y no en el orden arbitrario del unordered_set: asi un
    // volcado de flags es comparable entre ejecuciones (depuracion,
    // guardado, tests).
    std::sort(out.begin(), out.end());
    return out;
}

void NarrativeState::reset() {
    m_flags.clear();
    m_beatsFired.clear();
}

// ---------------------------------------------------------------------
// NarrativeCondition
// ---------------------------------------------------------------------

bool NarrativeCondition::isSatisfiedBy(const NarrativeState& state) const {
    for (const std::string& flag : allFlags) {
        if (!state.hasFlag(flag)) {
            return false;
        }
    }
    for (const std::string& flag : notFlags) {
        if (state.hasFlag(flag)) {
            return false;
        }
    }
    if (!anyFlags.empty()) {
        bool some = false;
        for (const std::string& flag : anyFlags) {
            if (state.hasFlag(flag)) {
                some = true;
                break;
            }
        }
        if (!some) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------
// AdventureScript: carga
// ---------------------------------------------------------------------

Result<AdventureScript> AdventureScript::loadFromString(const std::string& jsonText) {
    Result<JsonValue> parsed = JsonValue::parse(jsonText);
    if (!parsed.isOk()) {
        return Result<AdventureScript>::Error(parsed.errorMessage());
    }
    const JsonValue& root = parsed.value();
    if (!root.isObject()) {
        return Result<AdventureScript>::Error(
            "El JSON de aventura debe ser un objeto en la raiz");
    }

    // Se construye en una copia local y solo se devuelve si TODO ha
    // parseado: mismo todo-o-nada que ObjectCatalog::loadFromString (un
    // beat invalido no puede dejar media aventura cargada, porque media
    // aventura es una partida que se queda encallada sin decir por que).
    AdventureScript adv;

    adv.id = root["id"].asString();
    if (adv.id.empty()) {
        return Result<AdventureScript>::Error("La aventura no tiene campo \"id\"");
    }
    adv.name = root["name"].asString(adv.id);
    adv.description = root["description"].asString();

    const JsonValue& objectivesValue = root["objectives"];
    for (std::size_t i = 0; i < objectivesValue.size(); ++i) {
        const JsonValue& entry = objectivesValue[i];
        ObjectiveDefinition obj;
        obj.id = entry["id"].asString();
        if (obj.id.empty()) {
            return Result<AdventureScript>::Error("objectives[" + std::to_string(i) +
                                                      "] no tiene campo \"id\"");
        }
        obj.text = entry["text"].asString(obj.id);
        obj.doneFlag = entry["doneFlag"].asString();
        if (obj.doneFlag.empty()) {
            // Sin flag, el objetivo no podria completarse nunca: es un
            // error de contenido, no un objetivo "decorativo".
            return Result<AdventureScript>::Error("objectives[" + std::to_string(i) + "] (\"" +
                                                      obj.id + "\") no tiene \"doneFlag\"");
        }
        adv.objectives.push_back(std::move(obj));
    }

    const JsonValue& beatsValue = root["beats"];
    if (!beatsValue.isArray()) {
        return Result<AdventureScript>::Error(
            "La aventura no tiene campo \"beats\" o no es un array");
    }
    for (std::size_t i = 0; i < beatsValue.size(); ++i) {
        const JsonValue& entry = beatsValue[i];
        NarrativeBeat beat;

        beat.id = entry["id"].asString();
        if (beat.id.empty()) {
            return Result<AdventureScript>::Error("beats[" + std::to_string(i) +
                                                      "] no tiene campo \"id\"");
        }

        const JsonValue& triggerValue = entry["trigger"];
        const std::string triggerText = triggerValue["type"].asString();
        if (!parseTriggerType(triggerText, beat.trigger.type)) {
            return Result<AdventureScript>::Error(
                "beats[" + std::to_string(i) + "] (\"" + beat.id + "\"): trigger.type desconocido \"" +
                triggerText + "\" (validos: talk/enter/auto)");
        }
        beat.trigger.target = triggerValue["target"].asString();
        if (beat.trigger.type != TriggerType::Auto && beat.trigger.target.empty()) {
            return Result<AdventureScript>::Error("beats[" + std::to_string(i) + "] (\"" +
                                                      beat.id +
                                                      "\"): trigger.target vacio (talk/enter lo "
                                                      "necesitan para saber con quien/donde)");
        }

        const JsonValue& requiresValue = entry["requires"];
        beat.requirements.allFlags = readStringArray(requiresValue["allFlags"]);
        beat.requirements.anyFlags = readStringArray(requiresValue["anyFlags"]);
        beat.requirements.notFlags = readStringArray(requiresValue["notFlags"]);

        beat.speaker = entry["speaker"].asString();
        beat.lines = readStringArray(entry["lines"]);
        beat.once = entry["once"].asBool(false);

        const JsonValue& effectsValue = entry["effects"];
        for (std::size_t e = 0; e < effectsValue.size(); ++e) {
            const JsonValue& effectEntry = effectsValue[e];
            NarrativeEffect effect;
            const std::string effectText = effectEntry["type"].asString();
            if (!parseEffectType(effectText, effect.type)) {
                return Result<AdventureScript>::Error(
                    "beats[" + std::to_string(i) + "] (\"" + beat.id + "\").effects[" +
                    std::to_string(e) + "]: tipo desconocido \"" + effectText +
                    "\" (validos: setFlag/clearFlag/grantGold/log)");
            }
            effect.arg = effectEntry["arg"].asString();
            effect.value = effectEntry["value"].asInt(0);
            if ((effect.type == EffectType::SetFlag || effect.type == EffectType::ClearFlag) &&
                effect.arg.empty()) {
                return Result<AdventureScript>::Error(
                    "beats[" + std::to_string(i) + "] (\"" + beat.id + "\").effects[" +
                    std::to_string(e) + "]: setFlag/clearFlag necesitan \"arg\" con el nombre de la flag");
            }
            beat.effects.push_back(std::move(effect));
        }

        adv.beats.push_back(std::move(beat));
    }

    return Result<AdventureScript>::Ok(std::move(adv));
}

Result<AdventureScript> AdventureScript::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return Result<AdventureScript>::Error("No se pudo abrir la aventura: " + path);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    Result<AdventureScript> result = loadFromString(buffer.str());
    if (!result.isOk()) {
        // Se anade la ruta al mensaje: el error del parser habla de
        // beats[3], y sin el archivo eso no localiza nada.
        return Result<AdventureScript>::Error(path + ": " + result.errorMessage());
    }
    return result;
}

// ---------------------------------------------------------------------
// NarrativeEngine
// ---------------------------------------------------------------------

NarrativeResult NarrativeEngine::fire(TriggerType type, const std::string& target,
                                      NarrativeState& state) const {
    NarrativeResult result;
    if (m_adventure == nullptr) {
        return result;
    }

    for (const NarrativeBeat& beat : m_adventure->beats) {
        if (beat.trigger.type != type) {
            continue;
        }
        // Auto ignora el target (no tiene): comparar strings vacios
        // funcionaria igual, pero dejarlo explicito evita que alguien
        // "arregle" el JSON poniendole un target a un beat auto.
        if (type != TriggerType::Auto && beat.trigger.target != target) {
            continue;
        }
        if (beat.once && state.beatFired(beat.id)) {
            continue;
        }
        if (!beat.requirements.isSatisfiedBy(state)) {
            continue;
        }

        // --- Encontrado: el PRIMERO que encaja gana (ver el header) ---
        result.fired = true;
        result.beatId = beat.id;
        result.speaker = beat.speaker;
        result.lines = beat.lines;

        // Los efectos se aplican EN ORDEN y DESPUES de haber decidido el
        // beat: si un efecto enciende una flag que otro beat de mas
        // abajo esperaba, ese otro beat se disparara en el SIGUIENTE
        // disparo, no en este. Un disparo, un beat: es lo que evita las
        // cascadas accidentales en las que hablar con un NPC recorre
        // media aventura de golpe.
        for (const NarrativeEffect& effect : beat.effects) {
            switch (effect.type) {
                case EffectType::SetFlag:
                    state.setFlag(effect.arg);
                    break;
                case EffectType::ClearFlag:
                    state.clearFlag(effect.arg);
                    break;
                case EffectType::GrantGold:
                    result.goldDelta += effect.value;
                    break;
                case EffectType::Log:
                    result.log.push_back(effect.arg);
                    break;
            }
        }
        state.markBeatFired(beat.id);
        return result;
    }

    return result;
}

std::vector<ObjectiveStatus> NarrativeEngine::objectives(const NarrativeState& state) const {
    std::vector<ObjectiveStatus> out;
    if (m_adventure == nullptr) {
        return out;
    }
    out.reserve(m_adventure->objectives.size());
    for (const ObjectiveDefinition& def : m_adventure->objectives) {
        out.push_back(ObjectiveStatus{def.id, def.text, state.hasFlag(def.doneFlag)});
    }
    return out;
}

bool NarrativeEngine::allObjectivesDone(const NarrativeState& state) const {
    // Sin aventura no hay objetivos que cumplir; devolver true aqui
    // haria que "aventura terminada" y "no hay aventura" fueran
    // indistinguibles para quien pregunte.
    if (m_adventure == nullptr || m_adventure->objectives.empty()) {
        return false;
    }
    for (const ObjectiveDefinition& def : m_adventure->objectives) {
        if (!state.hasFlag(def.doneFlag)) {
            return false;
        }
    }
    return true;
}

}  // namespace RPG
