// H6-03 — los ajustes del jugador sobreviven a salir y entrar. Ver la cabecera
// para el porqué; aquí están las reglas de lectura, que son las que deciden si
// un fichero tocado a mano o a medias deja al jugador sin juego.

#include "Game/OptionsFile.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

namespace Game {
namespace {

constexpr int kSchemaVersion = 1;

int clampVolume(int value) {
    if (value < OptionsState::kVolumeMin) return OptionsState::kVolumeMin;
    if (value > OptionsState::kVolumeMax) return OptionsState::kVolumeMax;
    return value;
}

// Un volumen se lee con las tres respuestas posibles distinguidas: ausente
// (conserva el default, y no es un problema), presente y sano, presente y
// fuera de rango o de tipo (se cura y SE DICE). La política es la de
// SaveGameValidation: la ausencia conserva el default; un campo presente debe
// tener su tipo.
int readVolume(const JsonValue& node, const char* key, int current,
               std::vector<std::string>& problems) {
    const JsonValue& field = node[key];
    if (field.isNull()) {
        return current;
    }
    if (!field.isNumber()) {
        problems.push_back(std::string("audio.") + key + " no es un numero");
        return current;
    }
    const int value = field.asInt(current);
    const int clamped = clampVolume(value);
    if (clamped != value) {
        problems.push_back(std::string("audio.") + key + " valia " + std::to_string(value) +
                           ", fuera de " + std::to_string(OptionsState::kVolumeMin) + ".." +
                           std::to_string(OptionsState::kVolumeMax) + ": se recorto");
    }
    return clamped;
}

std::string readFile(const std::string& path, bool& opened) {
    std::ifstream file(path, std::ios::binary);
    opened = file.is_open();
    if (!opened) {
        return {};
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace

Result<void> OptionsFile::write(const std::string& path, const OptionsValues& values) {
    // Se recorta al ESCRIBIR y no solo al leer: persistir un 99 obligaría a la
    // próxima lectura a curarlo, y un fichero que el propio motor ha escrito no
    // debería necesitar cura.
    JsonWriter out;
    out.beginObject();
    out.member("schemaVersion", kSchemaVersion);
    out.member("language", values.language);
    out.key("audio");
    out.beginObject();
    out.member("music", clampVolume(values.music));
    out.member("sfx", clampVolume(values.sfx));
    out.member("ambience", clampVolume(values.ambience));
    out.endObject();
    out.endObject();

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return Result<void>::Error("no se pudo escribir '" + path + "'");
    }
    file << out.str();
    file.flush();
    if (!file) {
        return Result<void>::Error("se pudo abrir pero no escribir '" + path + "'");
    }
    return Result<void>::Ok();
}

OptionsRead OptionsFile::read(const std::string& path) {
    OptionsRead result;

    bool opened = false;
    const std::string text = readFile(path, opened);
    if (!opened) {
        // Sin fichero legible no hay nada que curar: son los defaults de
        // siempre y `problems` queda VACIO, porque la primera vez que se arranca
        // el juego no es un problema. `found` es lo que distingue «no había» de
        // «había y estaba roto» — y de paso evita prometer lo que ifstream no
        // puede separar: ausente y sin permisos se tratan igual, y en los dos
        // casos lo correcto es arrancar con los defaults.
        return result;
    }
    result.found = true;

    const Result<JsonValue> parsed = JsonValue::parse(text);
    if (!parsed.isOk()) {
        result.problems.push_back("no es JSON valido: " + parsed.errorMessage());
        return result;
    }
    if (!parsed.value().isObject()) {
        result.problems.push_back("la raiz no es un objeto");
        return result;
    }
    const JsonValue& root = parsed.value();

    const JsonValue& version = root["schemaVersion"];
    if (!version.isNull()) {
        if (!version.isNumber()) {
            result.problems.push_back("schemaVersion no es un numero");
            return result;
        }
        if (version.asInt(kSchemaVersion) != kSchemaVersion) {
            // No se interpreta a ciegas: una version que este motor no conoce
            // puede significar cosas distintas, y adivinar es como se acaba
            // poniendo la musica a 0 sin que nadie sepa por que.
            result.problems.push_back("schemaVersion " +
                                      std::to_string(version.asInt(kSchemaVersion)) +
                                      " no entendida (este motor escribe " +
                                      std::to_string(kSchemaVersion) + ")");
            return result;
        }
    }

    const JsonValue& audio = root["audio"];
    if (audio.isNull()) {
        result.problems.push_back("falta 'audio': se usan los valores por defecto");
    } else if (!audio.isObject()) {
        result.problems.push_back("'audio' no es un objeto");
    } else {
        result.values.music = readVolume(audio, "music", result.values.music, result.problems);
        result.values.sfx = readVolume(audio, "sfx", result.values.sfx, result.problems);
        result.values.ambience =
            readVolume(audio, "ambience", result.values.ambience, result.problems);
    }

    const JsonValue& language = root["language"];
    if (!language.isNull()) {
        if (!language.isString()) {
            result.problems.push_back("'language' no es un texto");
        } else {
            const std::string value = language.asString();
            bool known = false;
            for (const std::string& candidate : OptionsState::languages()) {
                if (candidate == value) {
                    known = true;
                    break;
                }
            }
            if (known) {
                result.values.language = value;
            } else {
                // Se rechaza en vez de conservarse: un idioma que no está en la
                // lista dejaría la interfaz sin textos y el jugador no sabría
                // por qué. Es el mismo criterio que OptionsState::setLanguage.
                result.problems.push_back("idioma '" + value + "' desconocido: se usa " +
                                          result.values.language);
            }
        }
    }

    return result;
}

OptionsValues OptionsFile::fromState(const OptionsState& state) {
    OptionsValues values;
    values.music = state.volume("music");
    values.sfx = state.volume("sfx");
    values.ambience = state.volume("ambience");
    values.language = state.language();
    return values;
}

bool OptionsFile::applyTo(OptionsState& state, const OptionsValues& values) {
    state.setVolume("music", values.music);
    state.setVolume("sfx", values.sfx);
    state.setVolume("ambience", values.ambience);
    return state.setLanguage(values.language);
}

}  // namespace Game
