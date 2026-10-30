// Implementacion del vocabulario del formato. Los cuerpos vienen tal cual
// del namespace anonimo de ProjectSerializer.cpp (21/09/2026): se movieron,
// no se reescribieron. Ver include/Project/SerializerHelpers.h.

#include "Project/SerializerHelpers.h"
#include "Core/Errors/ScopeGuard.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cmath>
#include <filesystem>
#include <limits>
#include <fstream>
#include <sstream>
#include <set>
#include <utility>
#include <variant>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace ProjectIO {

// Las dos versiones de esquema viven en la cabecera.

Result<std::string> requiredString(const JsonValue& object, const char* key,
                                   const char* context) {
    const JsonValue& value = object[key];
    if (!value.isString() || value.asString().empty()) {
        return Result<std::string>::Error(std::string(context) + ": falta string no vacío '" + key +
                                          "'");
    }
    return Result<std::string>::Ok(value.asString());
}

Result<int> requiredInt(const JsonValue& object, const char* key, const char* context) {
    const JsonValue& value = object[key];
    if (!value.isNumber() || !std::isfinite(value.asNumber()) ||
        std::trunc(value.asNumber()) != value.asNumber() ||
        value.asNumber() < std::numeric_limits<int>::min() ||
        value.asNumber() > std::numeric_limits<int>::max()) {
        return Result<int>::Error(std::string(context) + ": se esperaba entero válido '" + key + "'");
    }
    return Result<int>::Ok(value.asInt());
}

Result<bool> requiredBool(const JsonValue& object, const char* key, const char* context) {
    const JsonValue& value = object[key];
    if (!value.isBool()) {
        return Result<bool>::Error(std::string(context) + ": falta booleano '" + key + "'");
    }
    return Result<bool>::Ok(value.asBool());
}

Result<bool> checkSchema(const JsonValue& root, int expected, const char* context) {
    if (!root.isObject()) {
        return Result<bool>::Error(std::string(context) + ": la raíz debe ser un objeto JSON");
    }
    if (!root["schemaVersion"].isNumber()) {
        return Result<bool>::Error(std::string(context) + ": falta 'schemaVersion'");
    }
    const double actual = root["schemaVersion"].asNumber();
    if (actual != expected) {
        return Result<bool>::Error(std::string(context) + ": schemaVersion incompatible v" +
                                   std::to_string(actual) + "; se esperaba v" +
                                   std::to_string(expected));
    }
    return Result<bool>::Ok(true);
}

void writeStrings(JsonWriter& writer, const char* key, const std::vector<std::string>& values) {
    writer.key(key);
    writer.beginArray();
    for (const std::string& value : values) {
        writer.value(value);
    }
    writer.endArray();
}

Result<std::vector<std::string>> readStrings(const JsonValue& object, const char* key,
                                             const char* context) {
    const JsonValue& values = object[key];
    if (!values.isArray()) {
        return Result<std::vector<std::string>>::Error(std::string(context) + ": falta array '" +
                                                       key + "'");
    }
    std::vector<std::string> result;
    result.reserve(values.size());
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (!values[index].isString()) {
            return Result<std::vector<std::string>>::Error(
                std::string(context) + ": '" + key + "' contiene un valor no string");
        }
        result.push_back(values[index].asString());
    }
    return Result<std::vector<std::string>>::Ok(std::move(result));
}


// =====================================================================
// E-01 — Capas de eventos
//
// El disparador y la clase de condicion viajan como TEXTO y no como
// numero. Un entero seria mas corto y convertiria cada reordenacion del
// enum en una corrupcion silenciosa de todos los mapas guardados: un
// "interact" que pasa a valer 2 se lee como "autorun" y el cofre se abre
// solo. El texto tambien hace el fichero legible cuando hay que mirarlo a
// mano, que con contenido generado por scripts pasa a menudo.
//
// SOBRE schemaVersion. Esta capa NO sube la version del mapa. Anadir un
// tipo de capa es compatible al LEER —un mapa sin eventos se lee igual
// que siempre— y subirla dejaria ilegibles de golpe los 97 niveles que ya
// existen. Al reves no lo es: un binario anterior que abra un mapa con
// eventos falla con "capa: tipo desconocido 'event'", que es un error
// claro y no una perdida de datos. Congelar los esquemas es G-07 y ahi es
// donde esta decision debe quedar registrada.
// =====================================================================

const char* eventTriggerToText(EventTrigger trigger) {
    switch (trigger) {
        case EventTrigger::Interact: return "interact";
        case EventTrigger::Touch:    return "touch";
        case EventTrigger::Autorun:  return "autorun";
        case EventTrigger::Parallel: return "parallel";
    }
    return "interact";
}

Result<EventTrigger> eventTriggerFromText(const std::string& text, const char* context) {
    if (text == "interact") return Result<EventTrigger>::Ok(EventTrigger::Interact);
    if (text == "touch")    return Result<EventTrigger>::Ok(EventTrigger::Touch);
    if (text == "autorun")  return Result<EventTrigger>::Ok(EventTrigger::Autorun);
    if (text == "parallel") return Result<EventTrigger>::Ok(EventTrigger::Parallel);
    // No se cae a un valor por defecto: un disparador que no se entiende
    // convertiria una pagina que espera al jugador en una que arranca
    // sola, y el autor no veria ningun error.
    return Result<EventTrigger>::Error(std::string(context) + ": disparador desconocido '" +
                                       text + "'");
}

const char* eventConditionKindToText(EventConditionKind kind) {
    switch (kind) {
        case EventConditionKind::Flag:     return "flag";
        case EventConditionKind::Variable: return "variable";
        case EventConditionKind::Item:     return "item";
        case EventConditionKind::LocalSwitch: return "local_switch";
    }
    return "flag";
}

Result<EventConditionKind> eventConditionKindFromText(const std::string& text,
                                                      const char* context) {
    if (text == "flag")     return Result<EventConditionKind>::Ok(EventConditionKind::Flag);
    if (text == "variable") return Result<EventConditionKind>::Ok(EventConditionKind::Variable);
    if (text == "item")     return Result<EventConditionKind>::Ok(EventConditionKind::Item);
    if (text == "local_switch") return Result<EventConditionKind>::Ok(EventConditionKind::LocalSwitch);
    return Result<EventConditionKind>::Error(std::string(context) + ": clase de condicion "
                                             "desconocida '" + text + "'");
}

void writeEventCommands(JsonWriter& writer, const std::vector<MapEventCommand>& commands) {
    writer.beginArray();
    for (const MapEventCommand& command : commands) {
        writer.beginObject();
        writer.member("type", command.type);
        writer.member("version", command.version);
        // params se escribe SIEMPRE, aunque este vacio: un comando de un
        // tipo que este motor no conoce tiene que volver a salir igual
        // que entro, y "sin params" y "params vacios" no son lo mismo
        // para quien lo escribio.
        writer.key("params");
        writer.beginObject();
        for (const auto& parametro : command.params) {
            writer.member(parametro.first, parametro.second);
        }
        writer.endObject();
        writer.key("children");
        writeEventCommands(writer, command.children);
        writer.endObject();
    }
    writer.endArray();
}

// La profundidad se vigila AL LEER y no solo al validar: validar ocurre
// despues de haber construido el arbol, y un fichero con diez mil niveles
// se lleva la pila por delante mientras se construye. Es la diferencia
// entre rechazar un fichero corrupto y morir abriendolo.
Result<std::vector<MapEventCommand>> readEventCommands(const JsonValue& array,
                                                       const char* context,
                                                       std::size_t depth) {
    using Salida = Result<std::vector<MapEventCommand>>;
    if (depth > EventLayerDocument::kMaxCommandDepth) {
        return Salida::Error(std::string(context) + ": los comandos anidan mas de " +
                             std::to_string(EventLayerDocument::kMaxCommandDepth) + " niveles");
    }
    if (!array.isArray()) {
        return Salida::Error(std::string(context) + ": 'commands'/'children' debe ser un array");
    }
    std::vector<MapEventCommand> commands;
    commands.reserve(array.size());
    for (std::size_t index = 0; index < array.size(); ++index) {
        const JsonValue& node = array[index];
        const Result<std::string> type = requiredString(node, "type", "comando");
        if (!type.isOk()) {
            return Salida::Error(std::string(context) + ": " + type.errorMessage());
        }
        MapEventCommand command;
        command.type = type.value();
        command.version = node.has("version") ? node["version"].asInt(1) : 1;
        const JsonValue& params = node["params"];
        if (params.isObject()) {
            for (const auto& parametro : params.objectValues()) {
                if (!parametro.second.isString()) {
                    return Salida::Error(std::string(context) + ": el parametro '" +
                                         parametro.first + "' del comando '" + command.type +
                                         "' no es texto");
                }
                command.params[parametro.first] = parametro.second.asString();
            }
        }
        if (node.has("children")) {
            const Salida hijos = readEventCommands(node["children"], context, depth + 1);
            if (!hijos.isOk()) {
                return hijos;
            }
            command.children = hijos.value();
        }
        commands.push_back(std::move(command));
    }
    return Salida::Ok(std::move(commands));
}

Result<GridCoord> readGridCoord(const JsonValue& value, const char* context) {
    if (!value.isObject()) {
        return Result<GridCoord>::Error(std::string(context) + ": posición debe ser objeto");
    }
    const Result<int> x = requiredInt(value, "x", context);
    const Result<int> y = requiredInt(value, "y", context);
    if (!x.isOk()) {
        return Result<GridCoord>::Error(x.errorMessage());
    }
    if (!y.isOk()) {
        return Result<GridCoord>::Error(y.errorMessage());
    }
    return Result<GridCoord>::Ok(GridCoord{x.value(), y.value()});
}

void writeGridCoord(JsonWriter& writer, const char* key, GridCoord position) {
    writer.key(key);
    writer.beginObject();
    writer.member("x", position.x);
    writer.member("y", position.y);
    writer.endObject();
}

Result<bool> atomicWrite(const std::string& path, const std::string& text) try {
    if (path.empty()) {
        return Result<bool>::Error("ruta de documento vacía");
    }
    namespace fs = std::filesystem;
    const fs::path destination = fs::u8path(path);
    // Reservar una carpeta mediante create_directory es exclusivo entre
    // procesos. No truncamos un .tmp ajeno ni seguimos sus enlaces. Al vivir
    // junto al destino, el reemplazo sigue dentro del mismo sistema de archivos.
    static std::atomic<unsigned long> sequence{0};
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    fs::path temporaryDirectory;
    fs::path temporary;
    bool reserved = false;
    for (int attempt = 0; attempt < 64; ++attempt) {
        temporaryDirectory = destination;
        temporaryDirectory += ".tmp-" + std::to_string(stamp) + "-" +
            std::to_string(sequence.fetch_add(1, std::memory_order_relaxed));
        temporary = temporaryDirectory / "contenido";
        std::error_code ec;
        if (fs::create_directory(temporaryDirectory, ec)) {
            reserved = true;
            break;
        }
        if (ec && ec != std::errc::file_exists) {
            return Result<bool>::Error("no se puede preparar el guardado '" + path + "': " + ec.message());
        }
    }
    if (!reserved) {
        return Result<bool>::Error("no se puede reservar un temporal para '" + path + "'");
    }
    // Se destruye DESPUES del stream: Windows no borra un fichero abierto.
    // Solo retiramos nuestros dos recursos, nunca temporales preexistentes.
    auto cleanup = makeScopeGuard([&]() noexcept {
        std::error_code ec;
        fs::remove(temporary, ec);
        fs::remove(temporaryDirectory, ec);
    });
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) {
        return Result<bool>::Error("no se puede abrir el temporal de '" + path + "'");
    }
    output.write(text.data(), static_cast<std::streamsize>(text.size()));
    output.flush();
    output.close();
    if (!output) {
        return Result<bool>::Error("no se puede completar el temporal de '" + path + "'");
    }

#if defined(_WIN32)
    if (!MoveFileExW(temporary.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        return Result<bool>::Error("no se puede sustituir atómicamente '" + path + "'");
    }
#else
    if (std::rename(temporary.c_str(), destination.c_str()) != 0) {
        return Result<bool>::Error("no se puede sustituir atómicamente '" + path + "'");
    }
#endif
    return Result<bool>::Ok(true);
} catch (const std::filesystem::filesystem_error& exception) {
    return Result<bool>::Error("no se puede guardar '" + path + "': " + exception.what());
}

Result<std::string> readFile(const std::string& path) try {
    std::ifstream input(std::filesystem::u8path(path), std::ios::binary);
    if (!input) {
        return Result<std::string>::Error("no se puede abrir '" + path + "'");
    }
    std::ostringstream content;
    content << input.rdbuf();
    if (!input.good() && !input.eof()) {
        return Result<std::string>::Error("no se puede leer '" + path + "'");
    }
    return Result<std::string>::Ok(content.str());
} catch (const std::filesystem::filesystem_error& exception) {
    return Result<std::string>::Error("no se puede leer '" + path + "': " + exception.what());
}

}  // namespace ProjectIO
