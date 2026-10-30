// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero pertenece a 1 paquete del corte B:
//   G-17  (fase G, 7 j) - Idioma seleccionable de la interfície
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

#include "Engine/Locale.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

#include "Core/Json/JsonValue.h"

// G-17 — Implementacion. El porque de cada decision esta en el header.

namespace Engine {
namespace {

std::string leerFichero(const std::string& ruta, bool& ok) {
    std::ifstream fh(ruta, std::ios::binary);
    if (!fh) {
        ok = false;
        return std::string();
    }
    std::ostringstream buf;
    buf << fh.rdbuf();
    ok = true;
    return buf.str();
}

}  // namespace

// ---------------------------------------------------------------------
// Claves y huecos
// ---------------------------------------------------------------------

bool Locale::isValidKey(const std::string& key) {
    if (key.empty() || key.front() == '.' || key.back() == '.') {
        return false;
    }
    bool puntoAnterior = false;
    for (char c : key) {
        const bool valido = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.';
        if (!valido) {
            return false;
        }
        if (c == '.' && puntoAnterior) {
            return false;
        }
        puntoAnterior = (c == '.');
    }
    return true;
}

std::vector<std::string> Locale::placeholdersOf(const std::string& tmpl) {
    std::vector<std::string> huecos;
    for (std::size_t i = 0; i < tmpl.size(); ++i) {
        if (tmpl[i] != '{') {
            continue;
        }
        // "{{" es una llave literal, no un hueco.
        if (i + 1 < tmpl.size() && tmpl[i + 1] == '{') {
            ++i;
            continue;
        }
        const std::size_t cierre = tmpl.find('}', i + 1);
        if (cierre == std::string::npos) {
            break;
        }
        const std::string nombre = tmpl.substr(i + 1, cierre - i - 1);
        if (!nombre.empty() &&
            std::find(huecos.begin(), huecos.end(), nombre) == huecos.end()) {
            huecos.push_back(nombre);
        }
        i = cierre;
    }
    return huecos;
}

// ---------------------------------------------------------------------
// Carga
// ---------------------------------------------------------------------

Result<bool> Locale::parseTable(const std::string& code, const std::string& json,
                                std::map<std::string, std::string>& out) const {
    auto parsed = JsonValue::parse(json);
    if (!parsed.isOk()) {
        return Result<bool>::Error("idioma '" + code + "': " + parsed.errorMessage());
    }
    const JsonValue& root = parsed.value();
    if (!root.isObject()) {
        return Result<bool>::Error("idioma '" + code + "': la raiz no es un objeto");
    }
    const JsonValue& textos = root.has("textos") ? root["textos"] : root;
    if (!textos.isObject()) {
        return Result<bool>::Error("idioma '" + code + "': 'textos' no es un objeto");
    }
    for (const auto& par : textos.objectValues()) {
        if (!isValidKey(par.first)) {
            // DEC-15 — el texto visible lleva acentos; la CLAVE no. Si
            // se dejara pasar, el dia que alguien la escriba de otra
            // manera el texto desaparece y no falla nada.
            return Result<bool>::Error(
                "idioma '" + code + "': la clave '" + par.first +
                "' no vale. Los identificadores internos son [a-z0-9_.], sin acentos "
                "ni mayusculas (DEC-15)");
        }
        if (!par.second.isString()) {
            return Result<bool>::Error("idioma '" + code + "': el texto de '" + par.first +
                                       "' no es una cadena");
        }
        out[par.first] = par.second.asString("");
    }
    return Result<bool>::Ok(true);
}

Result<bool> Locale::loadFromStrings(const std::string& manifestJson,
                                     const std::map<std::string, std::string>& tablesJson) {
    auto parsed = JsonValue::parse(manifestJson);
    if (!parsed.isOk()) {
        return Result<bool>::Error(std::string("manifiesto de idiomas: ") + parsed.errorMessage());
    }
    const JsonValue& root = parsed.value();
    const std::string referencia = root["idioma_de_referencia"].asString("");
    if (referencia.empty()) {
        return Result<bool>::Error(
            "el manifiesto de idiomas no dice 'idioma_de_referencia'. Es de donde sale "
            "el texto cuando una clave falta en el idioma activo: sin eso, faltar una "
            "clave seria no enseñar nada");
    }
    const JsonValue& lista = root["idiomas"];
    if (!lista.isArray() || lista.size() == 0) {
        return Result<bool>::Error("el manifiesto de idiomas no trae la lista 'idiomas'");
    }

    std::vector<std::string> idiomas;
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const std::string code = lista[i].asString("");
        if (code.empty()) {
            return Result<bool>::Error("el manifiesto trae un idioma con codigo vacio");
        }
        idiomas.push_back(code);
    }
    if (std::find(idiomas.begin(), idiomas.end(), referencia) == idiomas.end()) {
        return Result<bool>::Error("el idioma de referencia '" + referencia +
                                   "' no esta en la lista 'idiomas' del manifiesto");
    }

    // Todo o nada: se construye aparte y solo se instala si TODAS las
    // tablas estan bien. Media carga deja la interfaz en dos idiomas.
    std::map<std::string, std::map<std::string, std::string>> tablas;
    for (const std::string& code : idiomas) {
        const auto it = tablesJson.find(code);
        if (it == tablesJson.end()) {
            return Result<bool>::Error("falta la tabla del idioma '" + code +
                                       "', que el manifiesto declara");
        }
        std::map<std::string, std::string> tabla;
        const Result<bool> r = parseTable(code, it->second, tabla);
        if (!r.isOk()) {
            return r;
        }
        tablas[code] = std::move(tabla);
    }
    if (tablas[referencia].empty()) {
        return Result<bool>::Error("la tabla del idioma de referencia '" + referencia +
                                   "' esta vacia");
    }

    m_reference = referencia;
    m_languages = std::move(idiomas);
    m_tables = std::move(tablas);
    if (std::find(m_languages.begin(), m_languages.end(), m_active) == m_languages.end()) {
        m_active = m_reference;
    }
    return Result<bool>::Ok(true);
}

Result<bool> Locale::loadFromDirectory(const std::string& directory) {
    const std::string base = directory.empty() || directory.back() == '/'
                                 ? directory
                                 : directory + "/";
    bool ok = false;
    const std::string manifiesto = leerFichero(base + kManifestFile, ok);
    if (!ok) {
        return Result<bool>::Error("no se puede leer " + base + kManifestFile);
    }
    // Se lee el manifiesto dos veces —aqui para saber que ficheros hay, y
    // dentro de loadFromStrings para validarlo— a cambio de que la
    // validacion viva en UN solo sitio y las dos rutas de carga no puedan
    // discrepar.
    auto parsed = JsonValue::parse(manifiesto);
    if (!parsed.isOk()) {
        return Result<bool>::Error(std::string("manifiesto de idiomas: ") + parsed.errorMessage());
    }
    const JsonValue& lista = parsed.value()["idiomas"];
    std::map<std::string, std::string> tablas;
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const std::string code = lista[i].asString("");
        if (code.empty()) {
            continue;
        }
        const std::string contenido = leerFichero(base + code + ".json", ok);
        if (!ok) {
            return Result<bool>::Error("no se puede leer " + base + code + ".json, que el "
                                       "manifiesto declara");
        }
        tablas[code] = contenido;
    }
    return loadFromStrings(manifiesto, tablas);
}

Result<bool> Locale::setLanguage(const std::string& code) {
    if (std::find(m_languages.begin(), m_languages.end(), code) == m_languages.end()) {
        return Result<bool>::Error("el idioma '" + code + "' no esta en el manifiesto");
    }
    m_active = code;
    return Result<bool>::Ok(true);
}

// ---------------------------------------------------------------------
// Pedir texto
// ---------------------------------------------------------------------

const std::map<std::string, std::string>* Locale::table(const std::string& code) const {
    const auto it = m_tables.find(code);
    return it == m_tables.end() ? nullptr : &it->second;
}

LocalizedText Locale::text(const std::string& key) const {
    LocalizedText salida;

    const std::map<std::string, std::string>* activa = table(m_active);
    if (activa != nullptr) {
        const auto it = activa->find(key);
        if (it != activa->end()) {
            salida.text = it->second;
            return salida;
        }
    }
    const std::map<std::string, std::string>* referencia = table(m_reference);
    if (referencia != nullptr) {
        const auto it = referencia->find(key);
        if (it != referencia->end()) {
            salida.text = it->second;
            salida.fromFallback = true;
            return salida;
        }
    }
    // Ni en el idioma ni en la referencia. Se devuelve la clave a la
    // vista: un hueco en blanco en la pantalla no se lo cuenta nadie.
    salida.text = "[" + key + "]";
    salida.missing = true;
    return salida;
}

LocalizedText Locale::format(const std::string& key,
                             const std::map<std::string, std::string>& args) const {
    LocalizedText salida = text(key);
    const std::string tmpl = salida.text;

    std::string resultado;
    resultado.reserve(tmpl.size() + 32);
    std::set<std::string> usados;

    for (std::size_t i = 0; i < tmpl.size(); ++i) {
        if (tmpl[i] == '{' && i + 1 < tmpl.size() && tmpl[i + 1] == '{') {
            resultado += '{';
            ++i;
            continue;
        }
        if (tmpl[i] == '}' && i + 1 < tmpl.size() && tmpl[i + 1] == '}') {
            resultado += '}';
            ++i;
            continue;
        }
        if (tmpl[i] != '{') {
            resultado += tmpl[i];
            continue;
        }
        const std::size_t cierre = tmpl.find('}', i + 1);
        if (cierre == std::string::npos) {
            // Plantilla mal cerrada: se copia tal cual y se vera.
            resultado += tmpl.substr(i);
            break;
        }
        const std::string nombre = tmpl.substr(i + 1, cierre - i - 1);
        const auto it = args.find(nombre);
        if (it == args.end()) {
            // El hueco se queda A LA VISTA. Poner una cadena vacia
            // convierte «(Èxit: 7 de daño)» en «(: de daño)», que parece
            // un texto raro y no un fallo.
            resultado += '{';
            resultado += nombre;
            resultado += '}';
            if (std::find(salida.missingArgs.begin(), salida.missingArgs.end(), nombre) ==
                salida.missingArgs.end()) {
                salida.missingArgs.push_back(nombre);
            }
        } else {
            resultado += it->second;
            usados.insert(nombre);
        }
        i = cierre;
    }

    for (const auto& par : args) {
        if (usados.count(par.first) == 0) {
            salida.unusedArgs.push_back(par.first);
        }
    }
    salida.text = std::move(resultado);
    return salida;
}

// ---------------------------------------------------------------------
// Comprobaciones
// ---------------------------------------------------------------------

std::vector<std::string> Locale::missingKeys(const std::string& code) const {
    std::vector<std::string> faltan;
    const std::map<std::string, std::string>* referencia = table(m_reference);
    const std::map<std::string, std::string>* otra = table(code);
    if (referencia == nullptr || otra == nullptr) {
        return faltan;
    }
    for (const auto& par : *referencia) {
        if (otra->count(par.first) == 0) {
            faltan.push_back(par.first);
        }
    }
    return faltan;
}

std::vector<std::string> Locale::orphanKeys(const std::string& code) const {
    std::vector<std::string> sobran;
    const std::map<std::string, std::string>* referencia = table(m_reference);
    const std::map<std::string, std::string>* otra = table(code);
    if (referencia == nullptr || otra == nullptr) {
        return sobran;
    }
    for (const auto& par : *otra) {
        if (referencia->count(par.first) == 0) {
            sobran.push_back(par.first);
        }
    }
    return sobran;
}

std::vector<PlaceholderMismatch> Locale::checkPlaceholders() const {
    std::vector<PlaceholderMismatch> fallos;
    const std::map<std::string, std::string>* referencia = table(m_reference);
    if (referencia == nullptr) {
        return fallos;
    }
    for (const std::string& code : m_languages) {
        if (code == m_reference) {
            continue;
        }
        const std::map<std::string, std::string>* otra = table(code);
        if (otra == nullptr) {
            continue;
        }
        for (const auto& par : *referencia) {
            const auto it = otra->find(par.first);
            if (it == otra->end()) {
                // Que falte la clave lo dice missingKeys(); aqui solo
                // interesan las que estan en los dos y no encajan.
                continue;
            }
            const std::vector<std::string> esperados = placeholdersOf(par.second);
            const std::vector<std::string> tiene = placeholdersOf(it->second);
            PlaceholderMismatch fallo;
            for (const std::string& h : esperados) {
                if (std::find(tiene.begin(), tiene.end(), h) == tiene.end()) {
                    fallo.missing.push_back(h);
                }
            }
            for (const std::string& h : tiene) {
                if (std::find(esperados.begin(), esperados.end(), h) == esperados.end()) {
                    fallo.extra.push_back(h);
                }
            }
            if (!fallo.missing.empty() || !fallo.extra.empty()) {
                fallo.key = par.first;
                fallo.language = code;
                fallos.push_back(std::move(fallo));
            }
        }
    }
    return fallos;
}

}  // namespace Engine
