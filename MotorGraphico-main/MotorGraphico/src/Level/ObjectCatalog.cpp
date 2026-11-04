#include "Level/ObjectCatalog.h"

#include <algorithm>
#include "Core/Json/JsonValue.h"

#include <fstream>
#include <sstream>

namespace {

// "prop"/"enemy"/"pickup" -> ObjectCategory. Devuelve false si el texto
// no es ninguna de las tres (el llamador convierte eso en Result::Error
// con el indice de la entrada: una categoria desconocida es un error
// estructural, no un default silencioso -- si el JSON dice "enemigo" por
// error tipografico, callar y tratarlo como Prop escondería el bug del
// contenido).
bool parseCategory(const std::string& text, ObjectCategory& out) {
    if (text == "prop") {
        out = ObjectCategory::Prop;
        return true;
    }
    if (text == "enemy") {
        out = ObjectCategory::Enemy;
        return true;
    }
    if (text == "pickup") {
        out = ObjectCategory::Pickup;
        return true;
    }
    if (text == "npc") {
        out = ObjectCategory::Npc;
        return true;
    }
    return false;
}

// "none"/"heal"/"restoreMana" -> PickupEffect. Mismo criterio estricto
// que parseCategory.
bool parsePickupEffect(const std::string& text, PickupEffect& out) {
    if (text == "none") {
        out = PickupEffect::None;
        return true;
    }
    if (text == "heal") {
        out = PickupEffect::Heal;
        return true;
    }
    if (text == "restoreMana") {
        out = PickupEffect::RestoreMana;
        return true;
    }
    return false;
}

}  // namespace

void ObjectCatalog::add(ObjectDefinition definition) {
    // insert_or_assign, no insert: un id repetido sobreescribe (ver el
    // comentario de loadFromString en el header, "ultimo archivo gana").
    m_objects.insert_or_assign(definition.id, std::move(definition));
}

bool ObjectCatalog::has(const std::string& id) const { return m_objects.count(id) > 0; }

const ObjectDefinition* ObjectCatalog::find(const std::string& id) const {
    auto it = m_objects.find(id);
    return it != m_objects.end() ? &it->second : nullptr;
}

std::vector<std::string> ObjectCatalog::ids() const {
    std::vector<std::string> result;
    result.reserve(m_objects.size());
    for (const auto& entry : m_objects) {
        result.push_back(entry.first);
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<std::string> ObjectCatalog::ids(ObjectCategory category) const {
    std::vector<std::string> result;
    for (const auto& entry : m_objects) {
        if (entry.second.category == category) {
            result.push_back(entry.first);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<std::string> ObjectCatalog::missingPreviewIds() const {
    std::vector<std::string> result;
    for (const auto& entry : m_objects) {
        if (entry.second.previewPng.empty()) {
            result.push_back(entry.first);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

Result<int> ObjectCatalog::loadFromString(const std::string& jsonText) {
    Result<JsonValue> parsed = JsonValue::parse(jsonText);
    if (!parsed.isOk()) {
        return Result<int>::Error(parsed.errorMessage());
    }
    const JsonValue& root = parsed.value();
    if (!root.isObject()) {
        return Result<int>::Error("El JSON de catalogo debe ser un objeto en la raiz");
    }
    const JsonValue& objectsValue = root["objects"];
    if (!objectsValue.isArray()) {
        return Result<int>::Error("El catalogo no tiene campo \"objects\" o no es un array");
    }

    // Se parsea TODO a un buffer local antes de tocar m_objects: si la
    // entrada 7 de 10 es invalida, el catalogo queda exactamente como
    // estaba (ni las 6 primeras se aplican) -- mismo criterio
    // todo-o-nada que Texture ("no hay estados a medio construir").
    std::vector<ObjectDefinition> pending;
    for (std::size_t i = 0; i < objectsValue.size(); ++i) {
        const JsonValue& entry = objectsValue[i];
        ObjectDefinition def;

        def.id = entry["id"].asString();
        if (def.id.empty()) {
            return Result<int>::Error("objects[" + std::to_string(i) +
                                      "] no tiene campo \"id\" (o esta vacio)");
        }

        std::string categoryText = entry["category"].asString();
        if (!parseCategory(categoryText, def.category)) {
            return Result<int>::Error("objects[" + std::to_string(i) + "] (\"" + def.id +
                                      "\"): categoria desconocida \"" + categoryText +
                                      "\" (validas: prop/enemy/pickup/npc)");
        }

        def.name = entry["name"].asString(def.id);  // sin nombre: el id sirve de nombre
        def.spriteId = entry["spriteId"].asInt(-1);
        def.previewPng = entry["previewPng"].asString();
        def.raceId = entry["raceId"].asString();
        def.blocksMovement = entry["blocksMovement"].asBool(false);
        def.tapaVista = entry["tapaVista"].asBool(false);
        def.interactable = entry["interactable"].asBool(false);

        if (def.category == ObjectCategory::Enemy) {
            const JsonValue& combatValue = entry["combat"];
            def.combat.maxHealth = combatValue["maxHealth"].asInt(def.combat.maxHealth);
            def.combat.maxMana = combatValue["maxMana"].asInt(def.combat.maxMana);
            const JsonValue& skillsValue = combatValue["skills"];
            for (std::size_t s = 0; s < skillsValue.size(); ++s) {
                std::string skillId = skillsValue[s].asString();
                if (!skillId.empty()) {
                    def.combat.skillIds.push_back(std::move(skillId));
                }
            }
            const JsonValue& statsValue = combatValue["stats"];
            const char* const kStats[4] = {"CON", "DES", "INT", "CAR"};
            for (std::size_t s = 0; s < 4 && statsValue.isObject(); ++s) {
                const int v = statsValue[kStats[s]].asInt(0);
                if (v < 0 || v > 8) {
                    return Result<int>::Error("objects[" + std::to_string(i) + "] (\"" + def.id + "\"): combat.stats." +
                                              kStats[s] + " tiene que ir de 0 a 8");
                }
                def.combat.stats[s] = v;
            }
        } else if (def.category == ObjectCategory::Pickup) {
            const JsonValue& pickupValue = entry["pickup"];
            std::string effectText = pickupValue["effect"].asString("none");
            if (!parsePickupEffect(effectText, def.pickup.effect)) {
                return Result<int>::Error("objects[" + std::to_string(i) + "] (\"" + def.id +
                                          "\"): efecto de pickup desconocido \"" + effectText +
                                          "\" (validos: none/heal/restoreMana)");
            }
            def.pickup.power = pickupValue["power"].asInt(0);
        }
        // "price" va FUERA del bloque "pickup" a proposito: cualquier
        // objeto puede tener precio (un arbusto decorativo comprado para
        // tu jardin, manana), aunque hoy solo se compren pickups.
        def.pickup.price = entry["price"].asInt(0);

        // --- NPC (dialogo + tienda opcional) ---
        const JsonValue& dialogueValue = entry["dialogue"];
        for (std::size_t d = 0; d < dialogueValue.size(); ++d) {
            std::string line = dialogueValue[d].asString();
            if (!line.empty()) {
                def.dialogue.lines.push_back(std::move(line));
            }
        }
        const JsonValue& shopValue = entry["shop"];
        if (shopValue.isObject()) {
            def.shop.buybackPercent = shopValue["buybackPercent"].asInt(25);  // AV18-07: 25 %
            const JsonValue& itemsValue = shopValue["items"];
            for (std::size_t it = 0; it < itemsValue.size(); ++it) {
                const JsonValue& itemValue = itemsValue[it];
                ShopItem item;
                // Admite dos formas: "pocion" (precio del catalogo) o
                // {"objectId": "pocion", "price": 30}. La corta es la
                // que se usara el 90% de las veces y obligar a la larga
                // solo anadiria ruido al contenido.
                if (itemValue.isObject()) {
                    item.objectId = itemValue["objectId"].asString();
                    item.price = itemValue["price"].asInt(0);
                } else {
                    item.objectId = itemValue.asString();
                }
                if (!item.objectId.empty()) {
                    def.shop.items.push_back(std::move(item));
                }
            }
        }

        // --- Negocio en venta (cartel) ---
        const JsonValue& businessValue = entry["business"];
        if (businessValue.isObject()) {
            def.business.price = businessValue["price"].asInt(0);
            def.business.baseIncome = businessValue["baseIncome"].asInt(0);
            def.business.businessName = businessValue["name"].asString(def.name);
        }

        // --- ACTUAR-01: sitio para actuar en la calle ---
        const JsonValue& actuacionValue = entry["actuacion"];
        if (actuacionValue.isObject()) {
            def.actuacion.rivalDice = actuacionValue["rivalDice"].asInt(0);
            def.actuacion.afluencia = actuacionValue["afluencia"].asInt(0);
            def.actuacion.lugar = actuacionValue["lugar"].asString("");
        }

        def.fuego.dano = entry["fuego"]["dano"].asInt(0);  // AV17-11: {"fuego": {"dano": 2}}

        const auto contenedor = Game::leeContenedor(entry["contenedor"]);
        if (!contenedor.isOk()) return Result<int>::Error(def.id + ": " + contenedor.errorMessage());
        def.contenedor = contenedor.value();
        pending.push_back(std::move(def));
    }

    for (ObjectDefinition& def : pending) {
        add(std::move(def));
    }
    return Result<int>::Ok(static_cast<int>(pending.size()));
}

Result<int> ObjectCatalog::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return Result<int>::Error("No se pudo abrir el archivo: " + path);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return loadFromString(buffer.str());
}

std::string ObjectCatalog::exportJson() const {
    std::ostringstream out;
    out << "{\n  \"objects\": [\n";

    const auto allIds = ids();
    for (std::size_t idx = 0; idx < allIds.size(); ++idx) {
        const ObjectDefinition* def = find(allIds[idx]);
        if (!def) continue;

        out << "    {\n";
        out << "      \"id\": \"" << def->id << "\",\n";
        out << "      \"category\": \"" <<
            (def->category == ObjectCategory::Prop ? "prop" :
             def->category == ObjectCategory::Enemy ? "enemy" :
             def->category == ObjectCategory::Pickup ? "pickup" : "npc") << "\",\n";
        out << "      \"name\": \"" << def->name << "\",\n";
        out << "      \"spriteId\": " << def->spriteId << ",\n";
        if (!def->previewPng.empty()) {
            out << "      \"previewPng\": \"" << def->previewPng << "\",\n";
        }
        if (!def->raceId.empty()) {
            out << "      \"raceId\": \"" << def->raceId << "\",\n";
        }
        if (def->blocksMovement) {
            out << "      \"blocksMovement\": true,\n";
        }
        if (def->tapaVista) {
            out << "      \"tapaVista\": true,\n";
        }
        if (def->interactable) {
            out << "      \"interactable\": true,\n";
        }

        if (def->category == ObjectCategory::Enemy) {
            out << "      \"combat\": {\n";
            out << "        \"maxHealth\": " << def->combat.maxHealth << ",\n";
            out << "        \"maxMana\": " << def->combat.maxMana;
            if (def->combat.stats != std::array<int, 4>{}) {
                out << ",\n        \"stats\": {\"CON\": " << def->combat.stats[0] << ", \"DES\": " << def->combat.stats[1]
                    << ", \"INT\": " << def->combat.stats[2] << ", \"CAR\": " << def->combat.stats[3] << "}";
            }
            if (!def->combat.skillIds.empty()) {
                out << ",\n        \"skills\": [";
                for (std::size_t s = 0; s < def->combat.skillIds.size(); ++s) {
                    if (s > 0) out << ", ";
                    out << "\"" << def->combat.skillIds[s] << "\"";
                }
                out << "]";
            }
            out << "\n      },\n";
        } else if (def->category == ObjectCategory::Pickup) {
            out << "      \"pickup\": {\n";
            out << "        \"effect\": \"" <<
                (def->pickup.effect == PickupEffect::None ? "none" :
                 def->pickup.effect == PickupEffect::Heal ? "heal" : "restoreMana") << "\",\n";
            out << "        \"power\": " << def->pickup.power << "\n";
            out << "      },\n";
        }

        if (def->pickup.price != 0) {
            out << "      \"price\": " << def->pickup.price << ",\n";
        }

        if (!def->dialogue.lines.empty()) {
            out << "      \"dialogue\": [";
            for (std::size_t d = 0; d < def->dialogue.lines.size(); ++d) {
                if (d > 0) out << ", ";
                out << "\"" << def->dialogue.lines[d] << "\"";
            }
            out << "],\n";
        }

        if (!def->shop.items.empty()) {
            out << "      \"shop\": {\n";
            out << "        \"buybackPercent\": " << def->shop.buybackPercent << ",\n";
            out << "        \"items\": [";
            for (std::size_t it = 0; it < def->shop.items.size(); ++it) {
                if (it > 0) out << ", ";
                const auto& item = def->shop.items[it];
                if (item.price != 0) {
                    out << "{\"objectId\": \"" << item.objectId << "\", \"price\": " << item.price << "}";
                } else {
                    out << "\"" << item.objectId << "\"";
                }
            }
            out << "]\n      },\n";
        }

        if (def->business.price != 0 || def->business.baseIncome != 0 || !def->business.businessName.empty()) {
            out << "      \"business\": {\n";
            out << "        \"price\": " << def->business.price << ",\n";
            out << "        \"baseIncome\": " << def->business.baseIncome << ",\n";
            out << "        \"name\": \"" << def->business.businessName << "\"\n";
            out << "      },\n";
        }
        if (!def->contenedor.tipo.empty()) out << "      \"contenedor\": " << Game::escribeContenedor(def->contenedor) << ",\n";
        if (def->fuego.dano > 0) out << "      \"fuego\": {\"dano\": " << def->fuego.dano << "},\n";
        if (def->actuacion.rivalDice != 0 || def->actuacion.afluencia != 0 || !def->actuacion.lugar.empty()) {
            out << "      \"actuacion\": {\"rivalDice\": " << def->actuacion.rivalDice << ", \"afluencia\": "
                << def->actuacion.afluencia << ", \"lugar\": \"" << def->actuacion.lugar << "\"},\n";
        }

        // Remove trailing comma if present and close object
        out.seekp(-2, std::ios::cur); // Remove last ",\n"
        out << "\n    }";
        if (idx + 1 < allIds.size()) out << ",";
        out << "\n";
    }

    out << "  ]\n}\n";
    return out.str();
}