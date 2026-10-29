// PERGAMINO-01 — ver include/HUD/Pergamino/FuenteAtlas.h.
#include "HUD/Pergamino/FuenteAtlas.h"

#include <cmath>
#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Pergamino {

namespace {

const char* const kIds[] = {"pixel",   "lora16",  "lora18",  "lora20",  "lora22",
                            "lora24",  "lora18b", "lora20b", "lora22b", "lora24b",
                            "lora28b", "lora16i", "lora18i", "lora13"};
static_assert(sizeof(kIds) / sizeof(kIds[0]) == static_cast<std::size_t>(Letra::Total),
              "una entrada de kIds por Letra");

std::uint64_t par(std::uint32_t a, std::uint32_t b) {
    return (static_cast<std::uint64_t>(a) << 32) | b;
}

void anadeUtf8(std::string& s, std::uint32_t c) {
    if (c < 0x80) {
        s += static_cast<char>(c);
    } else if (c < 0x800) {
        s += static_cast<char>(0xC0 | (c >> 6));
        s += static_cast<char>(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        s += static_cast<char>(0xE0 | (c >> 12));
        s += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        s += static_cast<char>(0x80 | (c & 0x3F));
    } else {
        s += static_cast<char>(0xF0 | (c >> 18));
        s += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
        s += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        s += static_cast<char>(0x80 | (c & 0x3F));
    }
}

}  // namespace

std::vector<std::uint32_t> decodificaUtf8(const std::string& texto) {
    std::vector<std::uint32_t> salida;
    salida.reserve(texto.size());
    std::size_t i = 0;
    while (i < texto.size()) {
        const unsigned char c = static_cast<unsigned char>(texto[i]);
        int extra = 0;
        std::uint32_t cp = c;
        if (c >= 0xF0) { extra = 3; cp = c & 0x07; }
        else if (c >= 0xE0) { extra = 2; cp = c & 0x0F; }
        else if (c >= 0xC0) { extra = 1; cp = c & 0x1F; }
        else if (c >= 0x80) { salida.push_back('?'); ++i; continue; }
        if (i + static_cast<std::size_t>(extra) >= texto.size() && extra > 0) {
            salida.push_back('?');  // secuencia cortada al final del texto
            break;
        }
        bool bien = true;
        for (int k = 1; k <= extra; ++k) {
            const unsigned char s = static_cast<unsigned char>(texto[i + static_cast<std::size_t>(k)]);
            if ((s & 0xC0) != 0x80) { bien = false; break; }
            cp = (cp << 6) | (s & 0x3F);
        }
        salida.push_back(bien ? cp : '?');
        i += static_cast<std::size_t>(bien ? extra + 1 : 1);
    }
    return salida;
}

std::uint32_t mayuscula(std::uint32_t c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    if (c >= 0xE0 && c <= 0xFE && c != 0xF7) return c - 0x20;  // à..þ -> À..Þ
    return c;
}

Result<bool> FuenteAtlas::carga(const std::string& rutaJson) {
    std::ifstream entrada(rutaJson, std::ios::binary);
    if (!entrada) return Result<bool>::Error("FuenteAtlas: no puedo abrir " + rutaJson);
    std::stringstream texto;
    texto << entrada.rdbuf();
    return cargaTexto(texto.str());
}

Result<bool> FuenteAtlas::cargaTexto(const std::string& json) {
    auto leido = JsonValue::parse(json);
    if (!leido.isOk()) return Result<bool>::Error("FuenteAtlas: " + leido.errorMessage());
    const JsonValue& raiz = leido.value();
    m_ancho = raiz["ancho"].asInt(0);
    m_alto = raiz["alto"].asInt(0);
    if (m_ancho <= 0 || m_alto <= 0) return Result<bool>::Error("FuenteAtlas: atlas sin medidas");
    for (std::size_t i = 0; i < m_fuentes.size(); ++i) {
        const JsonValue& f = raiz["fuentes"][kIds[i]];
        if (!f.isObject()) return Result<bool>::Error(std::string("FuenteAtlas: falta la fuente ") + kIds[i]);
        Fuente& destino = m_fuentes[i];
        destino = Fuente{};
        destino.pixel = f["tipo"].asString() == "pixel";
        destino.ascenso = f["ascenso"].asInt(0);
        destino.descenso = f["descenso"].asInt(0);
        for (const auto& [clave, v] : f["glifos"].objectValues()) {
            if (!v.isArray() || v.size() < 7) continue;
            Glifo g{v[0].asInt(), v[1].asInt(), v[2].asInt(), v[3].asInt(),
                    v[4].asInt(), v[5].asInt(), v[6].asInt()};
            destino.glifos[static_cast<std::uint32_t>(std::stoul(clave))] = g;
        }
        for (const auto& [clave, v] : f["kerning"].objectValues()) {
            const auto coma = clave.find(',');
            if (coma == std::string::npos) continue;
            destino.kerning[par(static_cast<std::uint32_t>(std::stoul(clave.substr(0, coma))),
                                static_cast<std::uint32_t>(std::stoul(clave.substr(coma + 1))))] =
                v.asInt();
        }
    }
    m_cargada = true;
    return Result<bool>::Ok(true);
}

int FuenteAtlas::ascenso(Letra letra, int escala) const { return fuente(letra).ascenso * escala; }
int FuenteAtlas::descenso(Letra letra, int escala) const { return fuente(letra).descenso * escala; }

float FuenteAtlas::coloca(Letra letra, const std::string& texto, float x, float lineaBase,
                          int escala, std::vector<GlifoColocado>& salida) const {
    const Fuente& f = fuente(letra);
    const std::vector<std::uint32_t> cps = decodificaUtf8(texto);
    if (f.pixel) {
        const float paso = 6.0f * static_cast<float>(escala);
        float pluma = 0.0f;
        for (std::uint32_t c : cps) {
            if (c == '\n') break;
            const auto it = f.glifos.find(mayuscula(c));
            if (it != f.glifos.end() && it->second.w > 0) {
                const Glifo& g = it->second;
                salida.push_back({Rect{x + pluma, lineaBase + static_cast<float>(g.apoyoY * escala),
                                       static_cast<float>(g.w * escala), static_cast<float>(g.h * escala)},
                                  Rect{static_cast<float>(g.x), static_cast<float>(g.y),
                                       static_cast<float>(g.w), static_cast<float>(g.h)}});
            }
            pluma += paso;
        }
        // El ultimo glifo no lleva la columna de aire detras.
        return cps.empty() ? 0.0f : pluma - static_cast<float>(escala);
    }
    long pluma64 = 0;
    std::uint32_t anterior = 0;
    const float x0 = std::round(x);
    for (std::uint32_t c : cps) {
        if (c == '\n') break;
        auto it = f.glifos.find(c);
        if (it == f.glifos.end()) it = f.glifos.find('?');
        if (it == f.glifos.end()) continue;
        if (anterior != 0) {
            const auto k = f.kerning.find(par(anterior, c));
            if (k != f.kerning.end()) pluma64 += k->second;
        }
        const Glifo& g = it->second;
        if (g.w > 0 && g.h > 0) {
            const float gx = x0 + std::round(static_cast<float>(pluma64) / 64.0f) + static_cast<float>(g.apoyoX);
            salida.push_back({Rect{gx, lineaBase + static_cast<float>(g.apoyoY),
                                   static_cast<float>(g.w), static_cast<float>(g.h)},
                              Rect{static_cast<float>(g.x), static_cast<float>(g.y),
                                   static_cast<float>(g.w), static_cast<float>(g.h)}});
        }
        pluma64 += g.avance64;
        anterior = c;
    }
    return std::round(static_cast<float>(pluma64) / 64.0f);
}

int FuenteAtlas::ancho(Letra letra, const std::string& texto, int escala) const {
    std::vector<GlifoColocado> descarte;
    return static_cast<int>(coloca(letra, texto, 0.0f, 0.0f, escala, descarte));
}

std::vector<std::string> FuenteAtlas::envuelve(Letra letra, const std::string& texto,
                                               float anchoMax, int escala) const {
    std::vector<std::string> lineas;
    std::size_t inicio = 0;
    while (inicio <= texto.size()) {
        const std::size_t fin = texto.find('\n', inicio);
        const std::string parrafo =
            texto.substr(inicio, fin == std::string::npos ? std::string::npos : fin - inicio);
        std::string actual;
        std::size_t p = 0;
        bool alguna = false;
        while (p <= parrafo.size()) {
            const std::size_t espacio = parrafo.find(' ', p);
            std::string palabra =
                parrafo.substr(p, espacio == std::string::npos ? std::string::npos : espacio - p);
            p = espacio == std::string::npos ? parrafo.size() + 1 : espacio + 1;
            if (palabra.empty() && espacio != std::string::npos) continue;
            const std::string prueba = actual.empty() ? palabra : actual + " " + palabra;
            if (anchoMax <= 0.0f || static_cast<float>(ancho(letra, prueba, escala)) <= anchoMax) {
                actual = prueba;
                continue;
            }
            if (!actual.empty()) {
                lineas.push_back(actual);
                alguna = true;
            }
            // Una palabra que no cabe sola se corta por caracteres.
            actual.clear();
            std::string trozo;
            for (std::uint32_t c : decodificaUtf8(palabra)) {
                std::string candidato = trozo;
                anadeUtf8(candidato, c);
                if (!trozo.empty() && static_cast<float>(ancho(letra, candidato, escala)) > anchoMax) {
                    lineas.push_back(trozo);
                    alguna = true;
                    trozo.clear();
                    anadeUtf8(trozo, c);
                } else {
                    trozo = candidato;
                }
            }
            actual = trozo;
        }
        if (!actual.empty() || !alguna) lineas.push_back(actual);
        if (fin == std::string::npos) break;
        inicio = fin + 1;
    }
    return lineas;
}

std::string FuenteAtlas::recorta(Letra letra, const std::string& texto, float anchoMax,
                                 int escala) const {
    const std::string linea = texto.substr(0, texto.find('\n'));
    if (static_cast<float>(ancho(letra, linea, escala)) <= anchoMax) return linea;
    const std::string puntos = fuente(letra).pixel ? "..." : "\xE2\x80\xA6";
    const std::vector<std::uint32_t> cps = decodificaUtf8(linea);
    std::string mejor = puntos;
    std::string base;
    for (std::uint32_t c : cps) {
        std::string candidato = base;
        anadeUtf8(candidato, c);
        std::string conPuntos = candidato;
        while (!conPuntos.empty() && conPuntos.back() == ' ') conPuntos.pop_back();
        conPuntos += puntos;
        if (static_cast<float>(ancho(letra, conPuntos, escala)) > anchoMax) break;
        base = candidato;
        mejor = conPuntos;
    }
    return mejor;
}

}  // namespace Pergamino
