#pragma once

#include <cmath>
#include <initializer_list>
#include <limits>
#include <string>

#include "Core/Json/JsonValue.h"

// G-11: la ausencia conserva los defaults del esquema v1; un campo presente
// debe tener su tipo. Validar antes de convertir evita truncados y conversiones
// fuera del rango de int/float, sin introducir reglas de balance del juego.
namespace Game::SaveGameValidation {

enum class Kind { Text, Integer, Real, Boolean, Object, Array };

struct Field {
    const char* name;
    Kind kind;
};

inline bool integer(const JsonValue& value) {
    if (!value.isNumber()) return false;
    const double number = value.asNumber();
    return std::isfinite(number) && std::trunc(number) == number &&
           number >= std::numeric_limits<int>::min() &&
           number <= std::numeric_limits<int>::max();
}

inline bool real(const JsonValue& value) {
    if (!value.isNumber()) return false;
    const double number = value.asNumber();
    return std::isfinite(number) &&
           std::abs(number) <= std::numeric_limits<float>::max();
}

inline bool expect(const JsonValue& value, Kind kind, const std::string& path,
                   std::string* error) {
    bool valid = false;
    const char* description = "";
    switch (kind) {
        case Kind::Text:
            valid = value.isString();
            description = "texto";
            break;
        case Kind::Integer:
            valid = integer(value);
            description = "un entero dentro del rango de int";
            break;
        case Kind::Real:
            valid = real(value);
            description = "un numero finito dentro del rango de float";
            break;
        case Kind::Boolean:
            valid = value.isBool();
            description = "un booleano";
            break;
        case Kind::Object:
            valid = value.isObject();
            description = "un objeto";
            break;
        case Kind::Array:
            valid = value.isArray();
            description = "una lista";
            break;
    }
    if (!valid && error != nullptr) {
        *error = "guardado invalido en '" + path + "': se esperaba " + description;
    }
    return valid;
}

inline std::string child(const std::string& path, const std::string& name) {
    return path.empty() ? name : path + "." + name;
}

inline bool fields(const JsonValue& node, const std::string& path,
                   std::initializer_list<Field> members, std::string* error) {
    for (const Field& member : members) {
        if (node.has(member.name) &&
            !expect(node[member.name], member.kind, child(path, member.name), error)) {
            return false;
        }
    }
    return true;
}

inline bool position(const JsonValue& node, const std::string& path, std::string* error) {
    return expect(node, Kind::Object, path, error) &&
           fields(node, path, {{"x", Kind::Integer}, {"y", Kind::Integer}}, error);
}

template <typename Validate>
bool array(const JsonValue& parent, const char* name, const std::string& path,
           Validate validate, std::string* error) {
    if (!parent.has(name)) return true;
    const JsonValue& values = parent[name];
    const std::string arrayPath = child(path, name);
    if (!expect(values, Kind::Array, arrayPath, error)) return false;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!validate(values[i], arrayPath + "[" + std::to_string(i) + "]", error)) {
            return false;
        }
    }
    return true;
}

inline bool text(const JsonValue& value, const std::string& path, std::string* error) {
    return expect(value, Kind::Text, path, error);
}

inline bool boolean(const JsonValue& value, const std::string& path, std::string* error) {
    return expect(value, Kind::Boolean, path, error);
}

inline bool business(const JsonValue& node, const std::string& path, std::string* error) {
    return expect(node, Kind::Object, path, error) &&
           fields(node, path, {{"objectId", Kind::Text}, {"name", Kind::Text},
                               {"baseIncome", Kind::Integer}, {"rentPercent", Kind::Integer}},
                  error);
}

inline bool worldObject(const JsonValue& node, const std::string& path, std::string* error) {
    if (!expect(node, Kind::Object, path, error) ||
        !fields(node, path, {{"objectId", Kind::Text}}, error)) return false;
    return !node.has("position") || position(node["position"], child(path, "position"), error);
}

inline bool enemy(const JsonValue& node, const std::string& path, std::string* error) {
    return worldObject(node, path, error) &&
           fields(node, path, {{"spawnIndex", Kind::Integer}, {"health", Kind::Integer},
                               {"summoned", Kind::Boolean}}, error);
}

inline bool timer(const JsonValue& node, const std::string& path, std::string* error) {
    return expect(node, Kind::Object, path, error) &&
           fields(node, path, {{"id", Kind::Text}, {"restante", Kind::Real},
                               {"activo", Kind::Boolean}, {"expirado", Kind::Boolean}}, error) &&
           array(node, "avisosDados", path, boolean, error);
}

inline bool localSwitches(const JsonValue& node, const std::string& path, std::string* error) {
    // El contrato histórico ignora letras desconocidas y eventos sin id.
    // Aquí solo comprobamos los tipos; no cambiamos esa política de recuperación.
    return expect(node, Kind::Object, path, error) &&
           fields(node, path, {{"event", Kind::Text}}, error) &&
           array(node, "on", path, text, error);
}

inline bool narrative(const JsonValue& node, const std::string& path, std::string* error) {
    if (!expect(node, Kind::Object, path, error) ||
        !fields(node, path, {{"variables", Kind::Object}}, error)) return false;
    for (const auto& variable : node["variables"].objectValues()) {
        if (!expect(variable.second, Kind::Integer,
                    child(child(path, "variables"), variable.first), error)) return false;
    }
    return array(node, "flags", path, text, error) &&
           array(node, "beatsFired", path, text, error) &&
           array(node, "timers", path, timer, error) &&
           array(node, "localSwitches", path, localSwitches, error);
}

// GUARDAR-FICHA — El heroe: sus ids en texto, la lista de lo puesto y su
// aspecto (textos, la ropa por ranura y las capas que no se pintan).
inline bool hero(const JsonValue& node, const std::string& path, std::string* error) {
    if (!expect(node, Kind::Object, path, error) ||
        !fields(node, path, {{"raceId", Kind::Text}, {"classId", Kind::Text}, {"backgroundId", Kind::Text},
                             {"deityId", Kind::Text}, {"bondId", Kind::Text}, {"fearId", Kind::Text},
                             {"flawId", Kind::Text}, {"goalId", Kind::Text}, {"idealId", Kind::Text},
                             {"personalityId", Kind::Text}, {"virtueId", Kind::Text},
                             {"appearance", Kind::Object}}, error) ||
        !array(node, "equipped", path, text, error)) return false;
    if (!node.has("appearance")) return true;
    const JsonValue& aspecto = node["appearance"];
    const std::string ruta = child(path, "appearance");
    if (!fields(aspecto, ruta, {{"raza", Kind::Text}, {"piel", Kind::Text}, {"talla", Kind::Text},
                                {"ojos", Kind::Text}, {"peloEstilo", Kind::Text}, {"peloColor", Kind::Text},
                                {"barbaEstilo", Kind::Text}, {"barbaColor", Kind::Text},
                                {"ropa", Kind::Object}}, error)) return false;
    if (aspecto.has("ropa")) {
        for (const auto& pieza : aspecto["ropa"].objectValues()) {
            if (!expect(pieza.second, Kind::Text, child(child(ruta, "ropa"), pieza.first), error)) return false;
        }
    }
    return array(aspecto, "quitarCapas", ruta, text, error);
}

inline bool optionalFields(const JsonValue& root, std::string* error) {
    if (!fields(root, "", {{"characterName", Kind::Text}, {"savedAt", Kind::Text}, {"returnLevel", Kind::Text},
                           {"playerHealth", Kind::Integer}, {"playerMaxHealth", Kind::Integer},
                           {"playerLevel", Kind::Integer}, {"playerLevelProgress", Kind::Integer},
                           {"playerTier", Kind::Integer}, {"playerTierMilestones", Kind::Integer},
                           {"playerMilestones", Kind::Integer},
                           {"gold", Kind::Integer}, {"morality", Kind::Integer},
                           {"stepsSinceRent", Kind::Integer}}, error)) return false;
    if (root.has("playerPosition") && !position(root["playerPosition"], "playerPosition", error)) {
        return false;
    }
    if (root.has("returnDoor") && !position(root["returnDoor"], "returnDoor", error)) return false;  // PUERTAS-01
    if (root.has("narrative") && !narrative(root["narrative"], "narrative", error)) return false;
    if (root.has("hero") && !hero(root["hero"], "hero", error)) return false;  // GUARDAR-FICHA
    return array(root, "inventory", "", text, error) &&
           array(root, "businesses", "", business, error) &&
           array(root, "worldObjects", "", worldObject, error) &&
           array(root, "enemies", "", enemy, error);
}

}  // namespace Game::SaveGameValidation
