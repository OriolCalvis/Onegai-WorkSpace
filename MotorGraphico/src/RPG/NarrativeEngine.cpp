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
    if (text == "startBattle") {
        out = EffectType::StartBattle;
        return true;
    }
    if (text == "skillCheck") {
        out = EffectType::SkillCheck;
        return true;
    }
    if (text == "startTimer") {
        out = EffectType::StartTimer;
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

std::vector<std::string> NarrativeState::beatsFired() const {
    std::vector<std::string> out(m_beatsFired.begin(), m_beatsFired.end());
    std::sort(out.begin(), out.end());
    return out;
}

TimerState& NarrativeState::timerRef(const std::string& id) {
    for (TimerState& t : m_timers) {
        if (t.id == id) return t;
    }
    m_timers.push_back(TimerState{});
    m_timers.back().id = id;
    return m_timers.back();
}

void NarrativeState::reset() {
    m_flags.clear();
    m_beatsFired.clear();
    m_timers.clear();
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
                    "\" (validos: setFlag/clearFlag/grantGold/log/skillCheck/startBattle)");
            }
            effect.arg = effectEntry["arg"].asString();
            effect.value = effectEntry["value"].asInt(0);
            if ((effect.type == EffectType::SetFlag || effect.type == EffectType::ClearFlag) &&
                effect.arg.empty()) {
                return Result<AdventureScript>::Error(
                    "beats[" + std::to_string(i) + "] (\"" + beat.id + "\").effects[" +
                    std::to_string(e) + "]: setFlag/clearFlag necesitan \"arg\" con el nombre de la flag");
            }
            if (effect.type == EffectType::SkillCheck) {
                // Sub-objeto dedicado: value (int) no cabe el pool rival ni
                // los 4 nombres de flag. Un bloque, no campos sueltos.
                const JsonValue& sc = effectEntry["skillCheck"];
                effect.skillCheck.skillId = sc["skillId"].asString();
                effect.skillCheck.cd = static_cast<float>(sc["cd"].asNumber(1.0));
                // Ed. 3 (bitácora 2.17): la prueba es enfrentada; el beat
                // declara cuántos dados tira el rival (el guardia «tira» 3,
                // la trampa «tira» 2). "cd" queda solo como compatibilidad
                // para beats pre-Ed. 3 (GameSession la convierte a dados).
                effect.skillCheck.rivalDice = static_cast<int>(sc["rivalDice"].asNumber(0));
                effect.skillCheck.flagBotch = sc["flagBotch"].asString();
                effect.skillCheck.flagPartial = sc["flagPartial"].asString();
                effect.skillCheck.flagSuccess = sc["flagSuccess"].asString();
                effect.skillCheck.flagCritical = sc["flagCritical"].asString();
                if (effect.skillCheck.skillId.empty() ||
                    effect.skillCheck.flagBotch.empty() ||
                    effect.skillCheck.flagPartial.empty() ||
                    effect.skillCheck.flagSuccess.empty() ||
                    effect.skillCheck.flagCritical.empty()) {
                    return Result<AdventureScript>::Error(
                        "beats[" + std::to_string(i) + "] (\"" + beat.id + "\").effects[" +
                        std::to_string(e) + "]: skillCheck necesita skillId, flagBotch, "
                        "flagPartial, flagSuccess y flagCritical (todos no vacios)");
                }
                if (effect.skillCheck.rivalDice < 0 || effect.skillCheck.rivalDice > 24) {
                    return Result<AdventureScript>::Error(
                        "beats[" + std::to_string(i) + "] (\"" + beat.id + "\").effects[" +
                        std::to_string(e) + "]: skillCheck.rivalDice fuera de rango (0..24)");
                }
                if (effect.skillCheck.cd < 0.0f || effect.skillCheck.cd > 3.0f) {
                    return Result<AdventureScript>::Error(
                        "beats[" + std::to_string(i) + "] (\"" + beat.id + "\").effects[" +
                        std::to_string(e) + "]: skillCheck.cd fuera de rango Nd6 "
                        "(0.0..3.0, GDD 7.1)");
                }
            }

            if (effect.type == EffectType::StartBattle) {
                // Sub-objeto por el mismo motivo que skillCheck: no cabe en
                // arg+value. Las dos flags son OBLIGATORIAS: un combate del
                // que no se puede saber como acabo es contenido que no
                // ramifica, y entonces no hacia falta pedirlo.
                const JsonValue& bt = effectEntry["startBattle"];
                effect.battle.monsterId = bt["monsterId"].asString();
                effect.battle.flagVictory = bt["flagVictory"].asString();
                effect.battle.flagDefeat = bt["flagDefeat"].asString();
                if (effect.battle.monsterId.empty() || effect.battle.flagVictory.empty() ||
                    effect.battle.flagDefeat.empty()) {
                    return Result<AdventureScript>::Error(
                        "beats[" + std::to_string(i) + "] (\"" + beat.id + "\").effects[" +
                        std::to_string(e) + "]: startBattle necesita monsterId, flagVictory "
                        "y flagDefeat (todos no vacios)");
                }
            }
            beat.effects.push_back(std::move(effect));
        }

        adv.beats.push_back(std::move(beat));
    }

    // F1 (bitacora 2.18): temporizadores de mision. Campo opcional — las
    // aventuras sin timers cargan igual que antes de existir esto.
    const JsonValue& timersValue = root["timers"];
    for (std::size_t i = 0; i < timersValue.size(); ++i) {
        const JsonValue& entry = timersValue[i];
        TimerDefinition t;
        t.id = entry["id"].asString();
        if (t.id.empty()) {
            return Result<AdventureScript>::Error("timers[" + std::to_string(i) +
                                                      "] no tiene campo \"id\"");
        }
        t.duracion = static_cast<float>(entry["duracion"].asNumber(0.0));
        t.arrancaPor = entry["arrancaPor"].asString();
        t.zona = entry["zona"].asString();
        t.flagExpira = entry["flagExpira"].asString();
        if (t.flagExpira.empty()) {
            return Result<AdventureScript>::Error("timers[" + std::to_string(i) + "] (\"" +
                                                      t.id + "\") no tiene \"flagExpira\" "
                                                      "(sin ella nadie se entera de que expiro)");
        }

        // Causas de expiracion: ["tiempo", "salir_de_zona"] o ambas. Si el
        // campo falta, el timer expira solo por tiempo (el caso comun).
        const JsonValue& causas = entry["expiraPor"];
        if (causas.isArray() && causas.size() > 0) {
            t.expiraPorTiempo = false;
            t.expiraPorSalirDeZona = false;
            for (std::size_t k = 0; k < causas.size(); ++k) {
                const std::string causa = causas[k].asString();
                if (causa == "tiempo") {
                    t.expiraPorTiempo = true;
                } else if (causa == "salir_de_zona") {
                    t.expiraPorSalirDeZona = true;
                } else {
                    return Result<AdventureScript>::Error("timers[" + std::to_string(i) +
                                                          "] (\"" + t.id + "\"): expiraPor desconocido \"" +
                                                          causa + "\" (validos: tiempo, salir_de_zona)");
                }
            }
        }
        if (t.expiraPorSalirDeZona && t.zona.empty()) {
            return Result<AdventureScript>::Error("timers[" + std::to_string(i) + "] (\"" +
                                                      t.id + "\"): expira por salir_de_zona pero "
                                                      "no declara \"zona\"");
        }
        if (t.expiraPorTiempo && t.duracion <= 0.0f) {
            return Result<AdventureScript>::Error("timers[" + std::to_string(i) + "] (\"" +
                                                      t.id + "\"): expira por tiempo pero "
                                                      "\"duracion\" no es positiva");
        }

        const std::string pres = entry["presentacion"].asString("contador");
        if (pres == "contador") {
            t.presentacion = TimerDefinition::Presentacion::Contador;
        } else if (pres == "barra") {
            t.presentacion = TimerDefinition::Presentacion::Barra;
        } else if (pres == "secreto") {
            t.presentacion = TimerDefinition::Presentacion::Secreto;
        } else {
            return Result<AdventureScript>::Error("timers[" + std::to_string(i) + "] (\"" +
                                                      t.id + "\"): presentacion desconocida \"" +
                                                      pres + "\" (validos: contador, barra, secreto)");
        }

        const JsonValue& avisos = entry["avisos"];
        for (std::size_t k = 0; k < avisos.size(); ++k) {
            TimerDefinition::Aviso a;
            a.fraccion = static_cast<float>(avisos[k]["fraccion"].asNumber(0.0));
            a.flag = avisos[k]["flag"].asString();
            if (a.flag.empty() || a.fraccion <= 0.0f || a.fraccion > 1.0f) {
                return Result<AdventureScript>::Error("timers[" + std::to_string(i) + "] (\"" +
                                                          t.id + "\").avisos[" + std::to_string(k) +
                                                          "]: necesita \"flag\" y \"fraccion\" en (0, 1]");
            }
            t.avisos.push_back(std::move(a));
        }
        adv.timers.push_back(std::move(t));
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
                // Diferido: el motor no tiene RNG ni catalogo de skills,
                // asi que solo empaqueta la peticion. GameSession la
                // resuelve contra DicePoolEngine y enciende la flag del
                // grado en el mismo NarrativeState (ver applyNarrative).
                case EffectType::SkillCheck:
                    result.skillChecks.push_back(effect.skillCheck);
                    break;
                // Diferido y ademas ASINCRONO: el combate dura turnos, asi
                // que la flag de victoria/derrota no se enciende aqui sino
                // cuando el BattleState se cierra (GameSession::
                // syncBattleOutcome).
                case EffectType::StartBattle:
                    result.battles.push_back(effect.battle);
                    break;
                // F1: arrancar un temporizador de mision no es diferido:
                // el reloj vive en el NarrativeState, que esta aqui mismo.
                case EffectType::StartTimer:
                    startTimer(effect.arg, state);
                    break;
            }
        }
        state.markBeatFired(beat.id);
        return result;
    }

    return result;
}

void NarrativeEngine::startTimer(const std::string& timerId, NarrativeState& state) const {
    if (m_adventure == nullptr) {
        return;
    }
    for (const TimerDefinition& t : m_adventure->timers) {
        if (t.id != timerId) {
            continue;
        }
        TimerState& ts = state.timerRef(t.id);
        if (ts.activo || ts.expirado) {
            return;  // ya esta en marcha o ya se consumo: no se reinicia
        }
        ts.activo = true;
        ts.restante = t.duracion;
        ts.avisosDados.assign(t.avisos.size(), false);
        return;
    }
    // Id desconocido = no-op (mismo criterio permisivo que el resto del
    // motor; validar_todo.py es quien lo denuncia en el contenido).
}

void NarrativeEngine::tickTimers(NarrativeState& state, float dtActivo,
                                 const std::string& nivelActual) const {
    if (m_adventure == nullptr) {
        return;
    }

    // 1) Timers que arrancan por flag (ademas del efecto startTimer).
    for (const TimerDefinition& t : m_adventure->timers) {
        if (!t.arrancaPor.empty() && state.hasFlag(t.arrancaPor)) {
            startTimer(t.id, state);
        }
    }

    for (const TimerDefinition& t : m_adventure->timers) {
        TimerState& ts = state.timerRef(t.id);

        // C-08 — El vector de avisos se ajusta a la definicion ACTUAL
        // antes de que nadie lo indexe. startTimer() lo dimensiona bien,
        // pero un guardado lo restaura con el tamano que tenia el dia en
        // que se guardo: anadir un aviso a una aventura convertia todas
        // las partidas ya empezadas en una escritura fuera de rango
        // (ts.avisosDados[i] con i < t.avisos.size()).
        //
        // resize() y no assign(): conserva el prefijo, es decir los
        // avisos que el jugador YA recibio, y rellena con false solo los
        // nuevos. Quitar un aviso tambien queda cubierto (el vector
        // sobra y se recorta) aunque ese caso no petaba.
        if (ts.avisosDados.size() != t.avisos.size()) {
            ts.avisosDados.resize(t.avisos.size(), false);
        }

        if (!ts.activo) {
            continue;
        }

        // 2) Expirar por SALIR DE LA ZONA: la casa arde aunque quedara
        //    tiempo en el reloj (bitacora 2.18).
        if (t.expiraPorSalirDeZona && !nivelActual.empty() && nivelActual != t.zona) {
            ts.activo = false;
            ts.expirado = true;
            state.setFlag(t.flagExpira);
            continue;
        }

        // 3) El tiempo solo avanza con dt ACTIVO: el orquestador pasa 0
        //    en combate, dialogo, pausa o AFK.
        if (!t.expiraPorTiempo || dtActivo <= 0.0f) {
            continue;
        }
        ts.restante -= dtActivo;

        // Avisos por umbral: fraccion sobre el tiempo RESTANTE.
        for (std::size_t i = 0; i < t.avisos.size(); ++i) {
            if (!ts.avisosDados[i] &&
                ts.restante <= t.duracion * t.avisos[i].fraccion) {
                ts.avisosDados[i] = true;
                state.setFlag(t.avisos[i].flag);
            }
        }

        if (ts.restante <= 0.0f) {
            ts.restante = 0.0f;
            ts.activo = false;
            ts.expirado = true;
            state.setFlag(t.flagExpira);
        }
    }
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
