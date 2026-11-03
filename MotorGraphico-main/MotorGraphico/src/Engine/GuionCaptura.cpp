// E-GUION-01 — Ver include/Engine/GuionCaptura.h.
#include "Engine/GuionCaptura.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <utility>

namespace Engine {

namespace {

using Engine::Input::InputKey;

std::string mayusculas(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

// Un fotograma es un entero sin signo escrito solo con digitos (sin "+", sin
// "-", sin espacios) y que cabe en un int con holgura.
bool leeFotograma(const std::string& texto, int& fotograma) {
    if (texto.empty() || texto.size() > 9) return false;
    int n = 0;
    for (const char c : texto) {
        if (c < '0' || c > '9') return false;
        n = n * 10 + (c - '0');
    }
    fotograma = n;
    return true;
}

bool nombreValido(const std::string& nombre) {
    if (nombre.empty()) return false;
    return std::all_of(nombre.begin(), nombre.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_' || c == '-';
    });
}

Result<GuionCaptura> falla(int linea, const std::string& motivo) {
    return Result<GuionCaptura>::Error("linea " + std::to_string(linea) + ": " + motivo);
}

}  // namespace

bool GuionCaptura::teclaDe(const std::string& nombre, InputKey& tecla) {
    static const std::pair<const char*, InputKey> tabla[] = {
        {"W", InputKey::W},           {"A", InputKey::A},
        {"S", InputKey::S},           {"D", InputKey::D},
        {"E", InputKey::E},           {"Q", InputKey::Q},
        {"F", InputKey::F},           {"L", InputKey::L},
        {"M", InputKey::M},           {"TAB", InputKey::Tab},
        {"ENTER", InputKey::Enter},   {"ESC", InputKey::Escape},
        {"ESPACIO", InputKey::Space}, {"BACKSPACE", InputKey::Backspace},
        {"ARRIBA", InputKey::UpArrow}, {"ABAJO", InputKey::DownArrow},
        {"IZQUIERDA", InputKey::LeftArrow}, {"DERECHA", InputKey::RightArrow},
        {"F5", InputKey::F5},         {"F9", InputKey::F9},
        {"0", InputKey::Digit0},      {"1", InputKey::Digit1},
        {"2", InputKey::Digit2},      {"3", InputKey::Digit3},
        {"4", InputKey::Digit4},      {"5", InputKey::Digit5},
        {"6", InputKey::Digit6},      {"7", InputKey::Digit7},
        {"8", InputKey::Digit8},      {"9", InputKey::Digit9},
    };
    const std::string buscada = mayusculas(nombre);
    for (const auto& [texto, valor] : tabla) {
        if (buscada == texto) {
            tecla = valor;
            return true;
        }
    }
    return false;
}

Result<GuionCaptura> GuionCaptura::parsea(const std::string& texto) {
    GuionCaptura guion;
    std::set<std::string> vistas;
    int numero = 0;
    int anterior = 0;
    int linea_fin = 0;
    std::istringstream entrada(texto);
    std::string linea;
    while (std::getline(entrada, linea)) {
        ++numero;
        const std::size_t almohadilla = linea.find('#');
        if (almohadilla != std::string::npos) linea.erase(almohadilla);
        std::istringstream palabras(linea);
        std::vector<std::string> p;
        for (std::string w; palabras >> w;) p.push_back(w);
        if (p.empty()) continue;

        if (linea_fin != 0) return falla(numero, "hay ordenes despues de 'fin' (linea " + std::to_string(linea_fin) + ")");
        if (p.size() < 2) return falla(numero, "falta la orden tras el fotograma");
        Orden o;
        o.linea = numero;
        if (!leeFotograma(p[0], o.fotograma))
            return falla(numero, "el fotograma '" + p[0] + "' no es un numero entero positivo");
        if (o.fotograma < anterior)
            return falla(numero, "el fotograma " + std::to_string(o.fotograma) + " va despues del " +
                                     std::to_string(anterior) + " (los fotogramas no pueden retroceder)");
        anterior = o.fotograma;

        const std::string orden = mayusculas(p[1]);
        if (orden == "PULSA" || orden == "SUELTA" || orden == "TOCA") {
            o.tipo = orden == "PULSA" ? Tipo::Pulsa : orden == "SUELTA" ? Tipo::Suelta : Tipo::Toca;
            if (p.size() < 3) return falla(numero, "'" + p[1] + "' necesita una tecla");
            if (p.size() > 3) return falla(numero, "sobra '" + p[3] + "' tras la tecla");
            if (!teclaDe(p[2], o.tecla)) return falla(numero, "tecla desconocida '" + p[2] + "'");
        } else if (orden == "CAPTURA") {
            o.tipo = Tipo::Captura;
            if (p.size() < 3) return falla(numero, "'captura' necesita un nombre");
            if (p.size() > 3) return falla(numero, "sobra '" + p[3] + "' tras el nombre");
            if (!nombreValido(p[2]))
                return falla(numero, "el nombre de captura '" + p[2] + "' solo puede llevar letras, cifras, '_' y '-'");
            if (!vistas.insert(p[2]).second)
                return falla(numero, "la captura '" + p[2] + "' ya esta pedida (se pisarian los ficheros)");
            o.nombre = p[2];
        } else if (orden == "FIN") {
            o.tipo = Tipo::Fin;
            if (p.size() > 2) return falla(numero, "sobra '" + p[2] + "' tras 'fin'");
            linea_fin = numero;
        } else {
            return falla(numero, "orden desconocida '" + p[1] + "' (pulsa, suelta, toca, captura o fin)");
        }
        guion.m_ordenes.push_back(o);
        guion.m_ultimo = o.fotograma;

        Cambio c;
        c.fotograma = o.fotograma;
        c.linea = o.linea;
        c.tecla = o.tecla;
        if (o.tipo == Tipo::Pulsa || o.tipo == Tipo::Suelta) {
            c.pulsada = o.tipo == Tipo::Pulsa;
            guion.m_cambios.push_back(c);
        } else if (o.tipo == Tipo::Toca) {
            c.pulsada = true;
            guion.m_cambios.push_back(c);
            c.fotograma = o.fotograma + 1;  // la suelta, antes que las ordenes de ese fotograma
            c.prioridad = 0;
            c.pulsada = false;
            guion.m_cambios.push_back(c);
        }
    }
    if (guion.m_ordenes.empty()) return Result<GuionCaptura>::Error("el guion no tiene ninguna orden");
    std::stable_sort(guion.m_cambios.begin(), guion.m_cambios.end(), [](const Cambio& a, const Cambio& b) {
        if (a.fotograma != b.fotograma) return a.fotograma < b.fotograma;
        if (a.prioridad != b.prioridad) return a.prioridad < b.prioridad;
        return a.linea < b.linea;
    });
    return Result<GuionCaptura>::Ok(std::move(guion));
}

Result<GuionCaptura> GuionCaptura::carga(const std::string& ruta) {
    std::ifstream f(ruta, std::ios::binary);
    if (!f) return Result<GuionCaptura>::Error(ruta + ": no se puede abrir el guion");
    std::ostringstream todo;
    todo << f.rdbuf();
    auto r = parsea(todo.str());
    if (!r.isOk()) return Result<GuionCaptura>::Error(ruta + ": " + r.errorMessage());
    return r;
}

Input::RawInputState GuionCaptura::teclas(int fotograma) const {
    bool abajo[Input::kInputKeyCount] = {};
    for (const Cambio& c : m_cambios) {
        if (c.fotograma > fotograma) break;
        abajo[static_cast<std::size_t>(c.tecla)] = c.pulsada;
    }
    Input::RawInputState estado;
    for (std::size_t i = 0; i < Input::kInputKeyCount; ++i)
        if (abajo[i]) estado.setDown(static_cast<InputKey>(i));
    return estado;
}

std::vector<std::string> GuionCaptura::capturasEn(int fotograma) const {
    std::vector<std::string> nombres;
    for (const Orden& o : m_ordenes)
        if (o.tipo == Tipo::Captura && o.fotograma == fotograma) nombres.push_back(o.nombre);
    return nombres;
}

std::vector<std::string> GuionCaptura::capturas() const {
    std::vector<std::string> nombres;
    for (const Orden& o : m_ordenes)
        if (o.tipo == Tipo::Captura) nombres.push_back(o.nombre);
    return nombres;
}

void CapturaEnCurso::situa(int fotograma) {
    m_fotograma = fotograma;
    m_pendientes = m_activo ? m_guion.capturasEn(fotograma) : std::vector<std::string>{};
}

std::vector<std::string> CapturaEnCurso::tomaCapturas() { return std::exchange(m_pendientes, {}); }

void CapturaEnCurso::avisaPerdidas() {
    for (const std::string& nombre : tomaCapturas())
        std::cerr << "GUION: la captura '" << nombre << "' del fotograma " << m_fotograma << " no se volco\n";
}

}  // namespace Engine
