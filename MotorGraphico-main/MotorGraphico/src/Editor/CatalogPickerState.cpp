// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   E-14  (fase E, 3 j) - Selectors de dades i referències
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado >>>

#include "Editor/CatalogPickerState.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace RPG {

namespace {

// Conversió simple a minúscules. DEC-15: el text visible admet accents,
// però la cerca es fa plegant-los (mateix criteri que NormalizeForSearch
// de ProblemsState, reutilitzat aquí de forma independent per no
// dependre d'un fitxer que encara està en curs).
std::string normalizeSearch(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        // Latin-1 en UTF-8: dos bytes que comencen per 0xC3. Es pleguen a
        // la seva lletra sense accent. DEC-15 permet accents al text
        // visible; qui cerca escriu sense ells.
        if (c == 0xC3 && i + 1 < text.size()) {
            const unsigned char d = static_cast<unsigned char>(text[i + 1]);
            const unsigned char b = static_cast<unsigned char>(d | 0x20);
            char plegado = 0;
            if (b >= 0xA0 && b <= 0xA5) plegado = 'a';
            else if (b >= 0xA8 && b <= 0xAB) plegado = 'e';
            else if (b >= 0xAC && b <= 0xAF) plegado = 'i';
            else if (b >= 0xB2 && b <= 0xB6) plegado = 'o';
            else if (b >= 0xB9 && b <= 0xBC) plegado = 'u';
            else if (b == 0xB1) plegado = 'n';
            else if (b == 0xA7) plegado = 'c';
            if (plegado != 0) {
                out += plegado;
                ++i;
                continue;
            }
        }
        if (c >= 'A' && c <= 'Z') {
            out += static_cast<char>(c + ('a' - 'A'));
        } else {
            out += static_cast<char>(c);
        }
    }
    return out;
}

// Mapa de noms de fitxer a tipus de catàleg.
CatalogType typeFromFilename(const std::string& filename) {
    if (filename == "skills") return CatalogType::Skill;
    if (filename == "monsters") return CatalogType::Monster;
    if (filename == "conditions") return CatalogType::Condition;
    if (filename == "npcs") return CatalogType::Npc;
    if (filename == "locations") return CatalogType::Location;
    if (filename == "adventures") return CatalogType::Adventure;
    if (filename == "events") return CatalogType::Event;
    if (filename == "equipment") return CatalogType::Equipment;
    if (filename == "consumables") return CatalogType::Consumable;
    if (filename == "spells") return CatalogType::Spell;
    if (filename == "traps") return CatalogType::Trap;
    if (filename == "deities") return CatalogType::Deity;
    if (filename == "mounts") return CatalogType::Mount;
    if (filename == "summons") return CatalogType::Summon;
    if (filename == "loot_tables") return CatalogType::LootTable;
    if (filename == "races") return CatalogType::Race;
    if (filename == "classes") return CatalogType::Class;
    if (filename == "backgrounds") return CatalogType::Background;
    if (filename == "feats") return CatalogType::Feat;
    if (filename == "traits") return CatalogType::Trait;
    if (filename == "passives") return CatalogType::Passive;
    if (filename == "pregen_characters") return CatalogType::PregenCharacter;
    return CatalogType::Object;
}

// Extreu el nom de fitxer sense extensió ni directori.
std::string stemFromPath(const std::string& path) {
    auto lastSlash = path.find_last_of("/\\");
    std::string base = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);
    auto dot = base.rfind('.');
    return (dot == std::string::npos) ? base : base.substr(0, dot);
}

// Llegeix tot el fitxer a un string.
Result<std::string> readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return Result<std::string>::Error("No es pot obrir: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return Result<std::string>::Ok(ss.str());
}

// Carrega entries d'un JSON de catàleg RPG ("entries" array).
void loadRpgCatalog(const JsonValue& root, CatalogType type,
                    std::vector<CatalogEntry>& out) {
    const JsonValue& entries = root["entries"];
    if (!entries.isArray()) return;
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const JsonValue& e = entries[i];
        const std::string id = e["id"].asString("");
        const std::string name = e["name"].asString("");
        if (id.empty()) continue;
        out.push_back({id, name.empty() ? id : name, type});
    }
}

// Carrega entries d'un JSON d'objectes ("objects" array).
void loadObjectCatalog(const JsonValue& root, std::vector<CatalogEntry>& out) {
    const JsonValue& objects = root["objects"];
    if (!objects.isArray()) return;
    for (std::size_t i = 0; i < objects.size(); ++i) {
        const JsonValue& o = objects[i];
        const std::string id = o["id"].asString("");
        const std::string name = o["name"].asString("");
        if (id.empty()) continue;
        out.push_back({id, name.empty() ? id : name, CatalogType::Object});
    }
}

}  // namespace

// --- Alimentació ---

void CatalogPickerState::addEntry(CatalogEntry entry) {
    // Sobreescriu si l'id ja existeix (mateix criteri que ObjectCatalog::add)
    for (auto& existing : m_entries) {
        if (existing.id == entry.id) {
            existing = std::move(entry);
            return;
        }
    }
    m_entries.push_back(std::move(entry));
}

void CatalogPickerState::addEntries(const std::vector<CatalogEntry>& entries) {
    for (const auto& e : entries) addEntry(e);
}

// --- Càrrega des de fitxer ---

bool CatalogPickerState::loadCatalogs(const std::string& assetsDir) {
    // Llista de fitxers de catàleg RPG coneguts.
    static const char* kCatalogFiles[] = {
        "skills", "monsters", "conditions", "npcs", "locations",
        "adventures", "events", "equipment", "consumables", "spells",
        "traps", "deities", "mounts", "summons", "loot_tables",
        "races", "classes", "backgrounds", "feats", "traits",
        "passives", "pregen_characters"
    };

    bool allOk = true;

    // Carrega catàlegs RPG des de assets/catalogs/
    for (const char* name : kCatalogFiles) {
        const std::string path = assetsDir + "/catalogs/" + name + ".json";
        auto result = readFile(path);
        if (!result.isOk()) { allOk = false; continue; }
        auto json = JsonValue::parse(result.value());
        if (!json.isOk()) { allOk = false; continue; }
        CatalogType type = typeFromFilename(name);
        loadRpgCatalog(json.value(), type, m_entries);
    }

    // Carrega objectes des de assets/objects/ (fitxers coneguts)
    static const char* kObjectFiles[] = {
        "test_objects", "empty_objects"
    };
    for (const char* name : kObjectFiles) {
        const std::string path = assetsDir + "/objects/" + name + ".json";
        auto result = readFile(path);
        if (!result.isOk()) continue;  // objectes opcionals
        auto json = JsonValue::parse(result.value());
        if (!json.isOk()) continue;
        loadObjectCatalog(json.value(), m_entries);
    }

    return allOk;
}

// --- Cerca i filtre ---

std::vector<CatalogEntry> CatalogPickerState::search(
        CatalogType type, const std::string& query) const {
    std::vector<CatalogEntry> results;
    const std::string normalizedQuery = normalizeSearch(query);

    for (const auto& entry : m_entries) {
        if (entry.type != type) continue;
        if (!normalizedQuery.empty()) {
            const std::string normalized_name = normalizeSearch(entry.name);
            const std::string normalized_id = normalizeSearch(entry.id);
            if (normalized_name.find(normalizedQuery) == std::string::npos &&
                normalized_id.find(normalizedQuery) == std::string::npos) {
                continue;
            }
        }
        results.push_back(entry);
    }
    return results;
}

// --- Selecció ---

Result<CatalogEntry> CatalogPickerState::select(
        CatalogType type, const std::string& id) const {
    for (const auto& entry : m_entries) {
        if (entry.id == id) {
            if (entry.type != type) {
                return Result<CatalogEntry>::Error(
                    "Tipus incompatible: " + id + " és " +
                    std::to_string(static_cast<int>(entry.type)) +
                    ", es demana " + std::to_string(static_cast<int>(type)));
            }
            return Result<CatalogEntry>::Ok(entry);
        }
    }
    return Result<CatalogEntry>::Error("Id no trobat: " + id);
}

bool CatalogPickerState::isValidReference(
        CatalogType type, const std::string& id) const {
    for (const auto& entry : m_entries) {
        if (entry.id == id && entry.type == type) return true;
    }
    return false;
}

const CatalogEntry* CatalogPickerState::findById(const std::string& id) const {
    for (const auto& entry : m_entries) {
        if (entry.id == id) return &entry;
    }
    return nullptr;
}

}  // namespace RPG
