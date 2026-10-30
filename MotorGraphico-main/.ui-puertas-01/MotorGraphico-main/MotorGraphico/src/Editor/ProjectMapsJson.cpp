// H4-02 — Cambiar una cadena de primer nivel de un JSON sin reescribir el
// resto. Ver include/Editor/ProjectMaps.h.
//
// POR QUE NO PARSEAR Y VOLVER A ESCRIBIR. JsonValue guarda los objetos en un
// std::map: reescribir un nivel para cambiarle el nombre lo devolveria con
// las claves en orden alfabetico, otro sangrado y los numeros como doubles.
// Los niveles estan versionados y los tocan varios autores: un cambio de
// nombre tiene que ser un diff de una linea, no del fichero entero.
#include <cstdio>
#include <string>

#include "Editor/ProjectMaps.h"

namespace Editor {
namespace {

std::size_t skipSpaces(const std::string& t, std::size_t i) {
    while (i < t.size() && (t[i] == ' ' || t[i] == '\t' || t[i] == '\r' || t[i] == '\n'))
        ++i;
    return i;
}

// i apunta a la comilla de apertura; devuelve la posicion tras la de cierre
// (o npos si la cadena no se cierra).
std::size_t skipString(const std::string& t, std::size_t i) {
    for (++i; i < t.size(); ++i) {
        if (t[i] == '\\')
            ++i;
        else if (t[i] == '"')
            return i + 1;
    }
    return std::string::npos;
}

// Salta un valor cualquiera (cadena, objeto, lista o literal). npos si esta roto.
std::size_t skipValue(const std::string& t, std::size_t i) {
    i = skipSpaces(t, i);
    if (i >= t.size())
        return std::string::npos;
    if (t[i] == '"')
        return skipString(t, i);
    if (t[i] == '{' || t[i] == '[') {
        int depth = 0;
        while (i < t.size()) {
            if (t[i] == '"') {
                i = skipString(t, i);
                if (i == std::string::npos)
                    return i;
                continue;
            }
            if (t[i] == '{' || t[i] == '[')
                ++depth;
            if (t[i] == '}' || t[i] == ']') {
                if (--depth == 0)
                    return i + 1;
            }
            ++i;
        }
        return std::string::npos;
    }
    while (i < t.size() && t[i] != ',' && t[i] != '}' && t[i] != ']' && t[i] != ' ' &&
           t[i] != '\n' && t[i] != '\r' && t[i] != '\t')
        ++i;
    return i;
}

std::string quoted(const std::string& value) {
    std::string out = "\"";
    for (const char c : value) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buffer[8];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned char>(c));
                    out += buffer;
                } else {
                    out += c;
                }
        }
    }
    return out + "\"";
}

}  // namespace

Result<std::string> replaceTopLevelString(const std::string& text, const std::string& key,
                                          const std::string& value, bool insertIfMissing) {
    using R = Result<std::string>;
    std::size_t i = skipSpaces(text, 0);
    if (i >= text.size() || text[i] != '{')
        return R::Error("el fichero no es un objeto JSON");
    const std::size_t open = i;
    i = skipSpaces(text, i + 1);
    bool empty = i < text.size() && text[i] == '}';
    while (!empty) {
        if (i >= text.size() || text[i] != '"')
            return R::Error("JSON mal formado: se esperaba una clave");
        const std::size_t keyEnd = skipString(text, i);
        if (keyEnd == std::string::npos)
            return R::Error("JSON mal formado: clave sin cerrar");
        const std::string found = text.substr(i + 1, keyEnd - i - 2);
        i = skipSpaces(text, keyEnd);
        if (i >= text.size() || text[i] != ':')
            return R::Error("JSON mal formado: falta ':'");
        const std::size_t valueStart = skipSpaces(text, i + 1);
        const std::size_t valueEnd = skipValue(text, valueStart);
        if (valueEnd == std::string::npos)
            return R::Error("JSON mal formado en '" + found + "'");
        if (found == key) {
            if (text[valueStart] != '"')
                return R::Error("'" + key + "' no es una cadena");
            return R::Ok(text.substr(0, valueStart) + quoted(value) + text.substr(valueEnd));
        }
        i = skipSpaces(text, valueEnd);
        if (i < text.size() && text[i] == ',') {
            i = skipSpaces(text, i + 1);
            continue;
        }
        if (i < text.size() && text[i] == '}')
            break;
        return R::Error("JSON mal formado tras '" + found + "'");
    }
    if (!insertIfMissing)
        return R::Error("falta la clave '" + key + "'");
    const std::string member = "\n  " + quoted(key) + ": " + quoted(value) + (empty ? "\n" : ",");
    return R::Ok(text.substr(0, open + 1) + member + text.substr(open + 1));
}

}  // namespace Editor
