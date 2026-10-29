// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo toca 1 paquete del corte B:
//   E-16  (fase E, 8 j) - Edició RPG mínima del projecte
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "Editor/ProjectDatabaseState.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

#include "Core/Json/JsonWriter.h"

namespace RPG {

namespace {

// E-16 — Versió del format. Quan desar i carregar deixin de ser la mateixa
// forma, això és el que permetrà migrar en comptes de fallar.
constexpr int kSchemaVersion = 1;

// Escriu un JsonValue qualsevol amb l'escriptor de flux. Fa falta recursió
// perquè `fields` és un objecte lliure: l'autor hi posa els camps que vol i
// save() no en sap els noms d'avançada.
void escriuValor(JsonWriter& writer, const JsonValue& valor) {
    if (valor.isNull()) {
        writer.valueNull();
    } else if (valor.isBool()) {
        writer.value(valor.asBool());
    } else if (valor.isNumber()) {
        // JsonWriter no té value(double). Els enters s'escriuen com a int, que
        // és exacte, i la resta com a float, que és el que l'escriptor ofereix.
        const double numero = valor.asNumber();
        if (numero == std::floor(numero) && std::fabs(numero) < 2147483647.0) {
            writer.value(static_cast<int>(numero));
        } else {
            writer.value(static_cast<float>(numero));
        }
    } else if (valor.isString()) {
        writer.value(valor.asString());
    } else if (valor.isArray()) {
        writer.beginArray();
        for (std::size_t i = 0; i < valor.size(); ++i) {
            escriuValor(writer, valor[i]);
        }
        writer.endArray();
    } else {
        writer.beginObject();
        for (const auto& camp : valor.objectValues()) {
            writer.key(camp.first);
            escriuValor(writer, camp.second);
        }
        writer.endObject();
    }
}

}  // namespace

ProjectDatabaseState::ProjectDatabaseState(MapDocument* doc) : m_doc(doc) {}

Result<std::string> ProjectDatabaseState::createEntry(const std::string& type) {
    std::string id = type + "_0";
    for (const auto& e : m_entries) {
        if (e.id == id) id = type + "_" + std::to_string(m_entries.size());
    }
    ProjectDatabaseEntry entry;
    entry.id = id;
    entry.type = type;
    entry.valid = false;
    m_entries.push_back(entry);
    return Result<std::string>::Ok(id);
}

Result<std::string> ProjectDatabaseState::duplicateEntry(const std::string& entryId) {
    for (const auto& e : m_entries) {
        if (e.id == entryId) {
            std::string newId = e.type + "_" + std::to_string(m_entries.size());
            ProjectDatabaseEntry copy = e;
            copy.id = newId;
            m_entries.push_back(copy);
            return Result<std::string>::Ok(newId);
        }
    }
    return Result<std::string>::Error("Entrada no encontrada");
}

bool ProjectDatabaseState::updateField(const std::string& entryId, const std::string& field, const JsonValue& value) {
    for (auto& e : m_entries) {
        if (e.id == entryId) {
            e.fields[field] = value;
            return true;
        }
    }
    return false;
}

bool ProjectDatabaseState::validateEntry(const std::string& entryId) {
    for (auto& e : m_entries) {
        if (e.id == entryId) {
            e.valid = !e.id.empty();
            return e.valid;
        }
    }
    return false;
}

// Reparacion de HEAD (17/09, Claude): estas dos definiciones devolvian
// Result<void> mientras la cabecera declara bool, y motor_project entero
// dejaba de compilar. Se alinean con la cabecera. Siguen SIN implementar
// (E-16, segunda parte: ProjectSerializer), asi que devuelven false: un
// guardado que no guarda no puede decir que ha ido bien.
// E-16 — Desar i carregar. Eren dos stubs que retornaven false: el formulari
// de l'editor es podia omplir i validar, però no anava enlloc.
//
// El format és {"schemaVersion": N, "entries": [...]}. "entries" no és
// opcional: un JSON vàlid sense aquesta clau no és una base de dades i
// carregar-lo ha de fallar, no donar una base buida per bona.
bool ProjectDatabaseState::save(const std::string& projectPath) {
    JsonWriter writer;
    writer.beginObject();
    writer.member("schemaVersion", kSchemaVersion);
    writer.key("entries");
    writer.beginArray();
    for (const ProjectDatabaseEntry& entry : m_entries) {
        writer.beginObject();
        writer.member("id", entry.id);
        writer.member("name", entry.name);
        writer.member("type", entry.type);
        writer.member("valid", entry.valid);
        writer.key("fields");
        escriuValor(writer, entry.fields);
        writer.endObject();
    }
    writer.endArray();
    writer.endObject();

    std::ofstream sortida(projectPath);
    if (!sortida.is_open()) {
        return false;
    }
    sortida << writer.str();
    sortida.flush();
    // Comprovar l'estat abans de tancar: un disc ple falla al bolcar, i sense
    // això es donaria per desat el que no s'ha escrit.
    const bool escrit = static_cast<bool>(sortida);
    sortida.close();
    return escrit;
}

bool ProjectDatabaseState::load(const std::string& projectPath) {
    std::ifstream entrada(projectPath);
    if (!entrada.is_open()) {
        return false;
    }
    std::stringstream buffer;
    buffer << entrada.rdbuf();

    const Result<JsonValue> arrel = JsonValue::parse(buffer.str());
    if (!arrel.isOk() || !arrel.value().isObject()) {
        return false;
    }
    if (!arrel.value().has("entries")) {
        return false;
    }
    const JsonValue& entries = arrel.value()["entries"];
    if (!entries.isArray()) {
        return false;
    }

    // Es llegeix tot a una llista local i només s'instal·la si el fitxer sencer
    // era llegible: mig carregat, l'editor ensenyaria una base de dades que no
    // és la del fitxer.
    std::vector<ProjectDatabaseEntry> llegides;
    llegides.reserve(entries.size());
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const JsonValue& e = entries[i];
        if (!e.isObject()) {
            return false;
        }
        ProjectDatabaseEntry entry;
        entry.id = e["id"].asString();
        entry.name = e["name"].asString();
        entry.type = e["type"].asString();
        entry.valid = e["valid"].asBool(false);
        entry.fields = e["fields"];
        if (entry.id.empty()) {
            // Sense id no es pot ni buscar ni duplicar: millor fallar ara.
            return false;
        }
        llegides.push_back(std::move(entry));
    }

    m_entries = std::move(llegides);
    return true;
}

size_t ProjectDatabaseState::count(const std::string& type) const {
    size_t c = 0;
    for (const auto& e : m_entries) {
        if (e.type == type) ++c;
    }
    return c;
}

const ProjectDatabaseEntry* ProjectDatabaseState::getEntry(const std::string& id) const {
    for (const auto& e : m_entries) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

bool ProjectDatabaseState::hasId(const std::string& id) const {
    for (const auto& e : m_entries) {
        if (e.id == id) return true;
    }
    return false;
}

} // namespace RPG