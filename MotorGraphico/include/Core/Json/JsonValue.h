#pragma once

#include <map>
#include <string>
#include <vector>

#include "Core/Errors/Result.h"

// Valor JSON minimo, de un parser propio sin dependencias (motor_json,
// GL-free): el sistema de niveles (Fase 6 de motor_grafico_gantt_rpg.puml,
// "Sistema de niveles y contenido") necesita leer JSON y no hay ninguna
// libreria JSON vendorizada todavia (a diferencia de TMX, que usa
// tinyxml2 -- ver README, seccion third_party/). Vendorizar nlohmann/json
// (single header) es la opcion natural si el esquema crece mucho, pero
// para lo que necesita LevelLoader (objetos, arrays, strings, numeros,
// bool, null; sin comentarios ni trailing commas) un parser propio de
// unas pocas decenas de lineas es mas simple que anadir una dependencia
// nueva.
//
// Tipo "variant" simplificado (sin std::variant, para poder usar
// switch/if en vez de std::visit): m_type decide cual de los campos es
// valido; el resto quedan a su valor por defecto y se ignoran.
class JsonValue {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    JsonValue() = default;

    // Parsea "text" como un unico documento JSON. Devuelve Result<T>, no
    // lanza: es una ruta de carga de CONTENIDO (nivel, catalogo de
    // enemigos...), no de inicializacion del motor -- mismo criterio que
    // ResourceManager<T>::load() (ver Core::Errors::ResultT en
    // motor_grafico_clases.puml: "Result<T> para rutas calientes/de
    // contenido, excepciones para arranque").
    static Result<JsonValue> parse(const std::string& text);

    Type type() const { return m_type; }
    bool isNull() const { return m_type == Type::Null; }
    bool isBool() const { return m_type == Type::Bool; }
    bool isNumber() const { return m_type == Type::Number; }
    bool isString() const { return m_type == Type::String; }
    bool isArray() const { return m_type == Type::Array; }
    bool isObject() const { return m_type == Type::Object; }

    // Accesores "seguros con default": devuelven el valor por defecto (no
    // lanzan, no hacen assert) si el tipo no coincide. Mismo criterio
    // permisivo que FogOfWar::stateAt() fuera de rango: un JSON de
    // contenido mal escrito no deberia poder tirar el motor abajo por un
    // campo con el tipo equivocado; LevelLoader (el llamador) es quien
    // decide si un campo ausente/invalido es un Result::Error fatal o un
    // valor por defecto razonable.
    bool asBool(bool defaultValue = false) const;
    double asNumber(double defaultValue = 0.0) const;
    int asInt(int defaultValue = 0) const;
    // Por VALOR (no const&): un default como asString("") crea un
    // std::string temporal: devolver una referencia a el seria una
    // referencia colgante en cuanto termina la expresion que llama a
    // asString(). Copiar es mas simple y correcto; el tamano tipico de
    // estos strings (nombres, ids, rutas) hace la copia irrelevante.
    std::string asString(const std::string& defaultValue = "") const;

    // Object: acceso por clave. Devuelve una JsonValue Null "estatica" (no
    // una referencia colgante) si la clave no existe o this no es un
    // Object: encadenar accesos (ej. root["enemies"][0]["position"]["x"])
    // nunca hace falta -- si algun eslabon falta, todo lo posterior sigue
    // resolviendo a Null en vez de un puntero/referencia invalida.
    const JsonValue& operator[](const std::string& key) const;
    bool has(const std::string& key) const;

    // Array: acceso por indice / tamano. Fuera de rango -> Null (mismo
    // criterio). size() tambien sirve para Object (numero de claves); para
    // cualquier otro tipo devuelve 0.
    const JsonValue& operator[](std::size_t index) const;
    std::size_t size() const;

    // Vista inmutable de las claves de un objeto. El cargador de niveles
    // la necesita para conservar propiedades por instancia de contenido;
    // para valores que no son Object devuelve un mapa vacio.
    const std::map<std::string, JsonValue>& objectValues() const;

private:
    Type m_type = Type::Null;
    bool m_bool = false;
    double m_number = 0.0;
    std::string m_string;
    std::vector<JsonValue> m_array;
    std::map<std::string, JsonValue> m_object;

    // El parser (clase interna de JsonValue.cpp) construye instancias
    // rellenando estos campos directamente; no hay setters publicos
    // porque JsonValue es un valor inmutable una vez parseado (nadie
    // deberia mutar un documento JSON ya cargado desde fuera del parser).
    friend class JsonParser;
};
