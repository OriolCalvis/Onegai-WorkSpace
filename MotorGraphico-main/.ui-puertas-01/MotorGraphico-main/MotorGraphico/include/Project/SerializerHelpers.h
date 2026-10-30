#pragma once

// Piezas compartidas por los tres serializadores del proyecto.
//
// POR QUE EXISTE. Salieron del namespace anonimo de ProjectSerializer.cpp
// el 21/09/2026, cuando ese fichero tenia 1081 lineas y lo cruzaban seis
// paquetes del plan (C-11, C-13, E-01, E-11, E-13, E-16). Mientras todo
// vivia en el mismo sitio, quien tocaba el formato de MAPA y quien tocaba
// el formato de PROYECTO se pisaban aunque no compartieran ni una funcion.
//
// No son "utilidades": son el vocabulario del formato. Leer un campo que
// tiene que estar, comprobar la version del esquema, escribir una coordenada
// y guardar sin dejar el fichero a medias. Si cambian, cambia el contrato,
// y por eso estan aqui y no repetidas en cada fichero.
//
// Van en su propio namespace y cada .cpp hace `using namespace ProjectIO;`:
// asi los cuerpos que se movieron no tuvieron que tocarse ni una linea.
// Ver planificacion/METODOLOGIA_RELEVOS.md (la ley de los 600).

#include "Project/ProjectSerializer.h"

#include <string>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

namespace ProjectIO {

constexpr int kProjectSchemaVersion = 1;
constexpr int kMapSchemaVersion = 1;

Result<std::string> requiredString(const JsonValue& object, const char* key,
                                   const char* context);

Result<int> requiredInt(const JsonValue& object, const char* key, const char* context);

Result<bool> requiredBool(const JsonValue& object, const char* key, const char* context);

Result<bool> checkSchema(const JsonValue& root, int expected, const char* context);

void writeStrings(JsonWriter& writer, const char* key, const std::vector<std::string>& values);

Result<std::vector<std::string>> readStrings(const JsonValue& object, const char* key,
                                             const char* context);


const char* eventTriggerToText(EventTrigger trigger);

Result<EventTrigger> eventTriggerFromText(const std::string& text, const char* context);

const char* eventConditionKindToText(EventConditionKind kind);

Result<EventConditionKind> eventConditionKindFromText(const std::string& text,
                                                      const char* context);

void writeEventCommands(JsonWriter& writer, const std::vector<MapEventCommand>& commands);

Result<std::vector<MapEventCommand>> readEventCommands(const JsonValue& array,
                                                       const char* context,
                                                       std::size_t depth);

Result<GridCoord> readGridCoord(const JsonValue& value, const char* context);

void writeGridCoord(JsonWriter& writer, const char* key, GridCoord position);

// Temporal exclusivo junto al destino; lo reemplaza tras cerrar, sin borrarlo
// antes ni tocar temporales ajenos. Rutas UTF-8, compartido con SaveGame.
Result<bool> atomicWrite(const std::string& path, const std::string& text);

Result<std::string> readFile(const std::string& path);

// readId es plantilla: se define aqui porque cada ids::XxxId la instancia
// en su propio fichero (mapa, capa, entidad, recurso, region). Vino tal
// cual del namespace anonimo de ProjectSerializer.cpp.
template <class Id>
Result<Id> readId(const JsonValue& object, const char* key, const char* context) {
    const Result<std::string> value = requiredString(object, key, context);
    if (!value.isOk()) {
        return Result<Id>::Error(value.errorMessage());
    }
    const Result<Id> parsed = Id::parse(value.value());
    if (!parsed.isOk()) {
        return Result<Id>::Error(std::string(context) + ": '" + key + "' " +
                                 parsed.errorMessage());
    }
    return parsed;
}

}  // namespace ProjectIO
