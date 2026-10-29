// Leer una aventura de disco: el JSON de FORMATO_AVENTURAS.md a structs.
//
// Es la mitad que VALIDA, y por eso esta sola: aqui es donde un campo que
// falta tiene que dar un error con nombre y no una aventura a medias. La
// escritura vive en AdventureSerializer.cpp y el flujo en NarrativeEngine.
//
// Salio de src/RPG/NarrativeEngine.cpp el 22/09/2026 (EST-25), donde eran
// 265 de sus 744 lineas.

#include "RPG/Narrative/AdventureScript.h"

#include <cmath>
#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>

#include "AcompanantesAventura.h"  // ACOMPANANTES-01
#include "BeatsAventura.h"         // VARIABLES-01 / OPCIONES-01
#include "Core/Json/JsonValue.h"

namespace RPG {

namespace {

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

}  // namespace

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
        const JsonValue& donde = entry["donde"];  // MAPA-01: un nivel o una lista
        if (donde.isString()) obj.donde.push_back(donde.asString());
        for (std::size_t k = 0; donde.isArray() && k < donde.size(); ++k) obj.donde.push_back(donde[k].asString());
        if (obj.doneFlag.empty()) {
            // Sin flag, el objetivo no podria completarse nunca: es un
            // error de contenido, no un objetivo "decorativo".
            return Result<AdventureScript>::Error("objectives[" + std::to_string(i) + "] (\"" +
                                                      obj.id + "\") no tiene \"doneFlag\"");
        }
        adv.objectives.push_back(std::move(obj));
    }

    // ACOMPANANTES-01 — Antes que los beats: un combate solo puede pedir
    // acompañantes de esta lista.
    auto acompanantes = acompanantes::lee(root["acompanantes"]);
    if (!acompanantes.isOk()) return Result<AdventureScript>::Error(acompanantes.errorMessage());
    adv.acompanantes = std::move(acompanantes.value());

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

        const std::string donde = "beats[" + std::to_string(i) + "] (\"" + beat.id + "\")";
        auto condicion = beats::leeCondicion(entry["requires"], donde);
        if (!condicion.isOk()) return Result<AdventureScript>::Error(condicion.errorMessage());
        beat.requirements = std::move(condicion.value());

        beat.speaker = entry["speaker"].asString();
        beat.lines = beats::cadenas(entry["lines"]);
        beat.once = entry["once"].asBool(false);
        beat.pagina = entry["pagina"].asBool(false);  // DIALOGOS-01

        auto efectos = beats::leeEfectos(entry["effects"], donde, adv.acompanantes);
        if (!efectos.isOk()) return Result<AdventureScript>::Error(efectos.errorMessage());
        beat.effects = std::move(efectos.value());

        // OPCIONES-01 — Lo que se puede contestar (al final: la pregunta son sus lines).
        auto opciones = beats::leeOpciones(entry["opciones"], donde, beat.speaker, adv.acompanantes);
        if (!opciones.isOk()) return Result<AdventureScript>::Error(opciones.errorMessage());
        beat.opciones = std::move(opciones.value());
        if (!beat.opciones.empty() && beat.lines.empty())
            return Result<AdventureScript>::Error(donde + ": un beat con opciones necesita lines (la pregunta)");

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
        t.nombre = entry["nombre"].asString();
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

}  // namespace RPG
