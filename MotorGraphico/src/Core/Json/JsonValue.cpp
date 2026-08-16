#include "Core/Json/JsonValue.h"

#include <cctype>
#include <cstdlib>

namespace {

// Excepcion interna de parseo: nunca escapa de JsonValue::parse() (se
// captura ahi y se convierte a Result<JsonValue>::Error). No hereda de
// EngineException a proposito: JsonValue no forma parte de Core::Errors,
// y esta excepcion es un detalle de implementacion de un solo uso dentro
// de este .cpp (evita acoplar el parser a la jerarquia de excepciones del
// motor por algo que jamas sale de esta unidad de compilacion).
struct JsonParseError {
    std::string message;
};

}  // namespace

// Recursive-descent parser clasico. Guarda una referencia al texto
// completo + una posicion (m_pos): no copia substrings salvo al extraer
// tokens concretos (numeros, strings).
//
// A proposito FUERA del namespace anonimo de arriba (a diferencia de
// JsonParseError): JsonValue.h declara "friend class JsonParser;", que
// se resuelve contra ::JsonParser (namespace global) -- si esta clase
// estuviera en un namespace anonimo, seria una clase DISTINTA
// ((anonimo)::JsonParser) sin relacion con la friend declaration, y el
// compilador rechazaria cada acceso a los campos privados de JsonValue
// (m_type, m_array, m_object...) que hace el parser. No pasa nada por
// quedar en scope global: JsonParser no se declara en ningun header, asi
// que sigue sin ser visible fuera de este .cpp.
class JsonParser {
public:
    explicit JsonParser(const std::string& text) : m_text(text) {}

    JsonValue parseDocument() {
        skipWhitespace();
        JsonValue value = parseValue();
        skipWhitespace();
        if (m_pos != m_text.size()) {
            throw JsonParseError{"contenido extra tras el valor JSON en la posicion " +
                                 std::to_string(m_pos)};
        }
        return value;
    }

private:
    const std::string& m_text;
    std::size_t m_pos = 0;

    char peek() const {
        if (m_pos >= m_text.size()) {
            throw JsonParseError{"fin de texto inesperado"};
        }
        return m_text[m_pos];
    }

    void expect(char c) {
        if (m_pos >= m_text.size() || m_text[m_pos] != c) {
            throw JsonParseError{std::string("se esperaba '") + c + "' en la posicion " +
                                 std::to_string(m_pos)};
        }
        ++m_pos;
    }

    void skipWhitespace() {
        while (m_pos < m_text.size() &&
              (m_text[m_pos] == ' ' || m_text[m_pos] == '\t' || m_text[m_pos] == '\n' ||
               m_text[m_pos] == '\r')) {
            ++m_pos;
        }
    }

    bool consumeLiteral(const char* literal) {
        std::size_t len = 0;
        while (literal[len] != '\0') {
            ++len;
        }
        if (m_text.compare(m_pos, len, literal) == 0) {
            m_pos += len;
            return true;
        }
        return false;
    }

    JsonValue parseValue() {
        skipWhitespace();
        char c = peek();
        switch (c) {
            case '{':
                return parseObject();
            case '[':
                return parseArray();
            case '"':
                return parseString();
            case 't':
            case 'f':
                return parseBool();
            case 'n':
                return parseNull();
            default:
                if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
                    return parseNumber();
                }
                throw JsonParseError{"caracter inesperado '" + std::string(1, c) +
                                     "' en la posicion " + std::to_string(m_pos)};
        }
    }

    JsonValue parseObject() {
        expect('{');
        JsonValue result;
        result.m_type = JsonValue::Type::Object;
        skipWhitespace();
        if (m_pos < m_text.size() && m_text[m_pos] == '}') {
            ++m_pos;
            return result;
        }
        while (true) {
            skipWhitespace();
            if (peek() != '"') {
                throw JsonParseError{"se esperaba una clave string en la posicion " +
                                     std::to_string(m_pos)};
            }
            std::string key = parseRawString();
            skipWhitespace();
            expect(':');
            JsonValue value = parseValue();
            result.m_object[key] = std::move(value);
            skipWhitespace();
            if (m_pos < m_text.size() && m_text[m_pos] == ',') {
                ++m_pos;
                continue;
            }
            expect('}');
            break;
        }
        return result;
    }

    JsonValue parseArray() {
        expect('[');
        JsonValue result;
        result.m_type = JsonValue::Type::Array;
        skipWhitespace();
        if (m_pos < m_text.size() && m_text[m_pos] == ']') {
            ++m_pos;
            return result;
        }
        while (true) {
            JsonValue value = parseValue();
            result.m_array.push_back(std::move(value));
            skipWhitespace();
            if (m_pos < m_text.size() && m_text[m_pos] == ',') {
                ++m_pos;
                continue;
            }
            expect(']');
            break;
        }
        return result;
    }

    // Sin soporte de \uXXXX a proposito: el contenido que este parser
    // necesita leer (nombres/ids/rutas de nivel) no lo necesita, y anadir
    // decodificacion UTF-16 de escapes solo para no usarla nunca es
    // complejidad de mas (ver dafo.md, "riesgo de sobre-ingenieria").
    std::string parseRawString() {
        expect('"');
        std::string result;
        while (true) {
            if (m_pos >= m_text.size()) {
                throw JsonParseError{"string sin cerrar"};
            }
            char c = m_text[m_pos++];
            if (c == '"') {
                break;
            }
            if (c == '\\') {
                if (m_pos >= m_text.size()) {
                    throw JsonParseError{"escape sin terminar al final del string"};
                }
                char esc = m_text[m_pos++];
                switch (esc) {
                    case '"':
                        result += '"';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case '/':
                        result += '/';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    default:
                        throw JsonParseError{std::string("escape no soportado '\\") + esc +
                                             "' (solo \\\" \\\\ \\/ \\n \\t \\r \\b \\f)"};
                }
                continue;
            }
            result += c;
        }
        return result;
    }

    JsonValue parseString() {
        JsonValue result;
        result.m_type = JsonValue::Type::String;
        result.m_string = parseRawString();
        return result;
    }

    JsonValue parseNumber() {
        std::size_t start = m_pos;
        if (m_pos < m_text.size() && m_text[m_pos] == '-') {
            ++m_pos;
        }
        while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
            ++m_pos;
        }
        if (m_pos < m_text.size() && m_text[m_pos] == '.') {
            ++m_pos;
            while (m_pos < m_text.size() &&
                  std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
                ++m_pos;
            }
        }
        if (m_pos < m_text.size() && (m_text[m_pos] == 'e' || m_text[m_pos] == 'E')) {
            ++m_pos;
            if (m_pos < m_text.size() && (m_text[m_pos] == '+' || m_text[m_pos] == '-')) {
                ++m_pos;
            }
            while (m_pos < m_text.size() &&
                  std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
                ++m_pos;
            }
        }
        if (m_pos == start) {
            throw JsonParseError{"numero invalido en la posicion " + std::to_string(start)};
        }
        std::string token = m_text.substr(start, m_pos - start);
        JsonValue result;
        result.m_type = JsonValue::Type::Number;
        result.m_number = std::strtod(token.c_str(), nullptr);
        return result;
    }

    JsonValue parseBool() {
        JsonValue result;
        result.m_type = JsonValue::Type::Bool;
        if (consumeLiteral("true")) {
            result.m_bool = true;
            return result;
        }
        if (consumeLiteral("false")) {
            result.m_bool = false;
            return result;
        }
        throw JsonParseError{"se esperaba 'true' o 'false' en la posicion " + std::to_string(m_pos)};
    }

    JsonValue parseNull() {
        if (consumeLiteral("null")) {
            return JsonValue{};  // Type::Null por defecto
        }
        throw JsonParseError{"se esperaba 'null' en la posicion " + std::to_string(m_pos)};
    }
};

Result<JsonValue> JsonValue::parse(const std::string& text) {
    try {
        JsonParser parser(text);
        return Result<JsonValue>::Ok(parser.parseDocument());
    } catch (const JsonParseError& e) {
        return Result<JsonValue>::Error(std::string("Error de parseo JSON: ") + e.message);
    }
}

bool JsonValue::asBool(bool defaultValue) const {
    return m_type == Type::Bool ? m_bool : defaultValue;
}

double JsonValue::asNumber(double defaultValue) const {
    return m_type == Type::Number ? m_number : defaultValue;
}

int JsonValue::asInt(int defaultValue) const {
    return m_type == Type::Number ? static_cast<int>(m_number) : defaultValue;
}

std::string JsonValue::asString(const std::string& defaultValue) const {
    return m_type == Type::String ? m_string : defaultValue;
}

const JsonValue& JsonValue::operator[](const std::string& key) const {
    static const JsonValue kNull;
    if (m_type != Type::Object) {
        return kNull;
    }
    auto it = m_object.find(key);
    return it != m_object.end() ? it->second : kNull;
}

bool JsonValue::has(const std::string& key) const {
    return m_type == Type::Object && m_object.find(key) != m_object.end();
}

const JsonValue& JsonValue::operator[](std::size_t index) const {
    static const JsonValue kNull;
    if (m_type != Type::Array || index >= m_array.size()) {
        return kNull;
    }
    return m_array[index];
}

std::size_t JsonValue::size() const {
    if (m_type == Type::Array) {
        return m_array.size();
    }
    if (m_type == Type::Object) {
        return m_object.size();
    }
    return 0;
}

const std::map<std::string, JsonValue>& JsonValue::objectValues() const {
    static const std::map<std::string, JsonValue> empty;
    return isObject() ? m_object : empty;
}
