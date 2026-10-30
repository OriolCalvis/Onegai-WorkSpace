// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 6 paquetes del corte B:
//   C-11  (fase C, 4 j) - Un sol format de projecte
//   C-13  (fase C, 3 j) - Les entitats del document no perden res
//   E-01  (fase E, 3 j) - L'esdeveniment és part del document de mapa
//   E-11  (fase E, 5 j) - Projectes i arbre de mapes
//   E-13  (fase E, 4 j) - Navegador visual i importació de recursos
//   E-16  (fase E, 8 j) - Edició RPG mínima del projecte
//
// AVISO - FICHERO DE CRUCE. Con 6 paquetes encima, es de los
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

#include "Project/ProjectSerializer.h"

#include <cstdio>
#include <cmath>
#include <limits>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

#include "Project/SerializerHelpers.h"

// El vocabulario del formato vive en SerializerHelpers; el `using` deja
// los cuerpos de abajo exactamente como estaban antes del corte.
using namespace ProjectIO;  // NOLINT(build/namespaces)


Result<std::string> ProjectSerializer::serializeProject(const Editor::Project& project) {
    const Result<bool> validation = project.validate();
    if (!validation.isOk()) {
        return Result<std::string>::Error("proyecto inválido: " + validation.errorMessage());
    }

    JsonWriter writer;
    writer.beginObject();
    writer.member("schemaVersion", kProjectSchemaVersion);
    writer.member("id", project.id);
    writer.member("name", project.name);
    writer.member("description", project.description);
    writer.member("epoch", project.epoch);
    writer.member("author", project.author);
    writer.member("prefix", project.prefix);
    writer.key("settings");
    writer.beginObject();
    writer.member("schemaVersion", project.settings.schemaVersion);
    writer.member("mapsDirectory", project.settings.mapsDirectory);
    writer.member("levelsDirectory", project.settings.levelsDirectory);
    writer.member("adventuresDirectory", project.settings.adventuresDirectory);
    writer.member("catalogsDirectory", project.settings.catalogsDirectory);
    writer.member("initialMap", project.settings.initialMap);
    // E-11 — Claves aditivas: solo se escriben cuando estan puestas de
    // verdad. Escribirlas siempre cambiaria el texto de todos los
    // manifiestos que ya existen, y la regla 1 de FORMATO_PROYECTOS.md es
    // que un cambio aditivo no toca lo ya escrito.
    if (!project.settings.title.empty()) {
        writer.member("title", project.settings.title);
    }
    if (project.settings.hasInitialCell) {
        writer.key("initialCell");
        writer.beginObject();
        writer.member("x", project.settings.initialCell.x);
        writer.member("y", project.settings.initialCell.y);
        writer.endObject();
    }
    writer.endObject();
    writer.member("entry", project.entry);
    writeStrings(writer, "levels", project.levels);
    writeStrings(writer, "maps", project.maps);
    writeStrings(writer, "adventures", project.adventures);
    writeStrings(writer, "catalogs", project.catalogs);
    writer.endObject();
    return Result<std::string>::Ok(writer.str());
}

Result<Editor::Project> ProjectSerializer::deserializeProject(const std::string& text) {
    const Result<JsonValue> parsed = JsonValue::parse(text);
    if (!parsed.isOk()) {
        return Result<Editor::Project>::Error("proyecto JSON inválido: " + parsed.errorMessage());
    }
    const JsonValue& root = parsed.value();
    // C-11: rechazar tambien formatos mezclados, sin perder campos en silencio.
    if (root.isObject()) {
        bool legacy = !root.has("schemaVersion");
        for (const char* key : {"nombre", "descripcion", "epoca", "autor", "prefijo", "entrada",
                                "niveles", "mapas", "aventuras", "catalogos"})
            legacy = legacy || root.has(key);
        if (legacy) return Result<Editor::Project>::Error(
            "formato legacy: ejecuta python3 tools/migrar_proyectos.py --write");
    }
    const Result<bool> schema = checkSchema(root, kProjectSchemaVersion, "proyecto");
    if (!schema.isOk()) {
        return Result<Editor::Project>::Error(schema.errorMessage());
    }

    const std::set<std::string> allowed = {"schemaVersion", "id", "name", "description", "epoch",
        "author", "prefix", "settings", "entry", "levels", "maps", "adventures", "catalogs"};
    for (const auto& field : root.objectValues()) {
        if (!allowed.count(field.first)) return Result<Editor::Project>::Error(
            "proyecto: campo desconocido '" + field.first + "'");
    }
    const Result<std::string> id = requiredString(root, "id", "proyecto");
    const Result<std::string> name = requiredString(root, "name", "proyecto");
    if (!id.isOk()) {
        return Result<Editor::Project>::Error(id.errorMessage());
    }
    if (!name.isOk()) {
        return Result<Editor::Project>::Error(name.errorMessage());
    }
    if (!root["settings"].isObject()) {
        return Result<Editor::Project>::Error("proyecto: falta objeto 'settings'");
    }

    for (const char* key : {"description", "epoch", "author", "prefix", "entry"}) {
        if (!root[key].isString()) return Result<Editor::Project>::Error(
            std::string("proyecto: falta string '") + key + "'");
    }
    if (!root["settings"]["initialMap"].isString())
        return Result<Editor::Project>::Error("project.settings: falta string 'initialMap'");

    Editor::Project project;
    project.id = id.value();
    project.name = name.value();
    project.description = root["description"].asString("");
    project.epoch = root["epoch"].asString("");
    project.author = root["author"].asString("");
    project.prefix = root["prefix"].asString("");
    project.entry = root["entry"].asString("");

    const JsonValue& settings = root["settings"];
    const std::set<std::string> allowedSettings = {"schemaVersion", "mapsDirectory", "levelsDirectory",
        "adventuresDirectory", "catalogsDirectory", "initialMap",
        // E-11. La lista es estricta a proposito —un campo desconocido es
        // error, y asi se cazan las erratas— pero eso obliga a apuntar
        // aqui cada clave nueva o el manifiesto que la lleve no abrira.
        "title", "initialCell"};
    for (const auto& field : settings.objectValues()) {
        if (!allowedSettings.count(field.first)) return Result<Editor::Project>::Error(
            "project.settings: campo desconocido '" + field.first + "'");
    }

    const Result<bool> settingsSchema = checkSchema(settings, Editor::ProjectSettings::kCurrentSchemaVersion,
                                                    "project.settings");
    if (!settingsSchema.isOk()) {
        return Result<Editor::Project>::Error(settingsSchema.errorMessage());
    }
    const Result<std::string> mapsDirectory =
        requiredString(settings, "mapsDirectory", "project.settings");
    const Result<std::string> levelsDirectory =
        requiredString(settings, "levelsDirectory", "project.settings");
    const Result<std::string> adventuresDirectory =
        requiredString(settings, "adventuresDirectory", "project.settings");
    const Result<std::string> catalogsDirectory =
        requiredString(settings, "catalogsDirectory", "project.settings");
    if (!mapsDirectory.isOk() || !levelsDirectory.isOk() || !adventuresDirectory.isOk() ||
        !catalogsDirectory.isOk()) {
        const std::string error = !mapsDirectory.isOk()      ? mapsDirectory.errorMessage()
                                  : !levelsDirectory.isOk()  ? levelsDirectory.errorMessage()
                                  : !adventuresDirectory.isOk()
                                      ? adventuresDirectory.errorMessage()
                                      : catalogsDirectory.errorMessage();
        return Result<Editor::Project>::Error(error);
    }
    project.settings.schemaVersion = settings["schemaVersion"].asInt();
    project.settings.mapsDirectory = mapsDirectory.value();
    project.settings.levelsDirectory = levelsDirectory.value();
    project.settings.adventuresDirectory = adventuresDirectory.value();
    project.settings.catalogsDirectory = catalogsDirectory.value();
    project.settings.initialMap = settings["initialMap"].asString("");
    project.settings.title = settings["title"].asString("");
    const JsonValue& celda = settings["initialCell"];
    if (celda.isObject()) {
        if (!celda["x"].isNumber() || !celda["y"].isNumber()) {
            return Result<Editor::Project>::Error(
                "project.settings.initialCell: faltan 'x' e 'y' numericos");
        }
        project.settings.hasInitialCell = true;
        project.settings.initialCell = GridCoord{celda["x"].asInt(0), celda["y"].asInt(0)};
    }

    const Result<std::vector<std::string>> levels = readStrings(root, "levels", "proyecto");
    const Result<std::vector<std::string>> maps = readStrings(root, "maps", "proyecto");
    const Result<std::vector<std::string>> adventures = readStrings(root, "adventures", "proyecto");
    const Result<std::vector<std::string>> catalogs = readStrings(root, "catalogs", "proyecto");
    if (!levels.isOk() || !maps.isOk() || !adventures.isOk() || !catalogs.isOk()) {
        const std::string error = !levels.isOk()      ? levels.errorMessage()
                                  : !maps.isOk()       ? maps.errorMessage()
                                  : !adventures.isOk() ? adventures.errorMessage()
                                                       : catalogs.errorMessage();
        return Result<Editor::Project>::Error(error);
    }
    project.levels = levels.value();
    project.maps = maps.value();
    project.adventures = adventures.value();
    project.catalogs = catalogs.value();

    const Result<bool> validation = project.validate();
    if (!validation.isOk()) {
        return Result<Editor::Project>::Error("proyecto inválido: " + validation.errorMessage());
    }
    return Result<Editor::Project>::Ok(std::move(project));
}
