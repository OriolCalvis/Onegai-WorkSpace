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

    // C-10: el parser decodifica \uXXXX a UTF-8. Ya no es un capricho:
    // JsonWriter ESCAPA asi los caracteres de control, y el contrato del
    // guardado es el round-trip — lo que sale del escritor tiene que
    // volver a entrar por aqui sin perdidas. Los pares de sustitucion
    // UTF-16 (\uD83D\uDE00) se combinan en su codepoint.
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
                    case 'u': {
                        unsigned int codepoint = parseHex4();
                        if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                            // Sustituto alto: exige el bajo a continuacion.
                            if (m_pos + 1 < m_text.size() && m_text[m_pos] == '\\' &&
                                m_text[m_pos + 1] == 'u') {
                                m_pos += 2;
                                const unsigned int low = parseHex4();
                                if (low < 0xDC00 || low > 0xDFFF) {
                                    throw JsonParseError{"sustituto bajo invalido tras un sustituto alto"};
                                }
                                codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                            } else {
                                throw JsonParseError{"sustituto alto sin su sustituto bajo"};
                            }
                        } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
                            throw JsonParseError{"sustituto bajo sin sustituto alto"};
                        }
                        appendUtf8(result, codepoint);
                        break;
                    }
                    default:
                        throw JsonParseError{std::string("escape no soportado '\\") + esc +
                                             "' (solo \\\" \\\\ \\/ \\n \\t \\r \\b \\f \\uXXXX)"};
                }
                continue;
            }
            result += c;
        }
        return result;
    }

    // C-10: los cuatro digitos hexadecimales de un escape \uXXXX.
    unsigned int parseHex4() {
        if (m_pos + 4 > m_text.size()) {
            throw JsonParseError{"escape \\u incompleto al final del string"};
        }
        unsigned int value = 0;
        for (int i = 0; i < 4; ++i) {
            const char digit = m_text[m_pos++];
            value <<= 4;
            if (digit >= '0' && digit <= '9') {
                value |= static_cast<unsigned int>(digit - '0');
            } else if (digit >= 'a' && digit <= 'f') {
                value |= static_cast<unsigned int>(digit - 'a' + 10);
            } else if (digit >= 'A' && digit <= 'F') {
                value |= static_cast<unsigned int>(digit - 'A' + 10);
            } else {
                throw JsonParseError{std::string("digito hexadecimal invalido '") + digit +
                                     "' en un escape \\u"};
            }
        }
        return value;
    }

    // C-10: codepoint -> UTF-8 (de 1 a 4 bytes).
    static void appendUtf8(std::string& out, unsigned int codepoint) {
        if (codepoint < 0x80) {
            out += static_cast<char>(codepoint);
        } else if (codepoint < 0x800) {
            out += static_cast<char>(0xC0 | (codepoint >> 6));
            out += static_cast<char>(0x80 | (codepoint & 0x3F));
        } else if (codepoint < 0x10000) {
            out += static_cast<char>(0xE0 | (codepoint >> 12));
            out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (codepoint & 0x3F));
        } else {
            out += static_cast<char>(0xF0 | (codepoint >> 18));
            out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
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
