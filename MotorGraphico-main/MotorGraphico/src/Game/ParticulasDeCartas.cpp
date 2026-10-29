// PART-02d — ver include/Game/ParticulasDeCartas.h.
#include "Game/ParticulasDeCartas.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Game {

namespace {

ParticulasDeCartas::Color leerColor(const std::string& hex, ParticulasDeCartas::Color porDefecto) {
    if (hex.size() != 6 || hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
        return porDefecto;
    ParticulasDeCartas::Color c{};
    for (std::size_t i = 0; i < 3; ++i)
        c[i] = static_cast<std::uint8_t>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
    return c;
}

}  // namespace

Result<ParticulasDeCartas> ParticulasDeCartas::cargar(const std::string& ruta) {
    std::ifstream in(ruta, std::ios::binary);
    if (!in)
        return Result<ParticulasDeCartas>::Error("no se puede abrir " + ruta);
    std::stringstream ss;
    ss << in.rdbuf();
    auto json = JsonValue::parse(ss.str());
    if (!json.isOk())
        return Result<ParticulasDeCartas>::Error(ruta + ": " + json.errorMessage());
    ParticulasDeCartas p;
    for (const auto& [carta, familia] : json.value()["cartas"].objectValues())
        if (!familia.asString().empty()) p.m_porCarta[carta] = familia.asString();
    for (const auto& [familia, color] : json.value()["colores"].objectValues())
        p.m_colores[familia] = leerColor(color.asString(), {255, 240, 210});
    return Result<ParticulasDeCartas>::Ok(std::move(p));
}

std::string ParticulasDeCartas::familiaDe(const std::string& carta, const std::vector<std::string>& etiquetas) const {
    const auto it = m_porCarta.find(carta);
    if (it != m_porCarta.end())
        return it->second;
    for (const std::string& e : etiquetas) {
        const std::string f = familiaDeElemento(e);
        if (!f.empty())
            return f;
    }
    return "";
}

ParticulasDeCartas::Color ParticulasDeCartas::colorDe(const std::string& familia) const {
    const auto it = m_colores.find(familia);
    return it != m_colores.end() ? it->second : Color{255, 240, 210};
}

const std::map<std::string, std::string>& elementosYSusFamilias() {
    // Los de tools/elementos.py (ELEMENTOS y ALIAS): test_validar_sprites comprueba que son los mismos.
    static const std::map<std::string, std::string> kElementos = {
        {"Fuego", "llamas"},   {"Agua", "agua"},     {"Hielo", "escarcha"},    {"Rayo", "rayos"},
        {"Tierra", "piedras"}, {"Viento", "viento"}, {"Luz", "luz"},           {"Tinieblas", "sombra"},
        {"Naturaleza", "hojas"}, {"Veneno", "veneno"}, {"Sangre", "sangre"},   {"Arcano", "arcano"},
        {"Vacio", "vacio"},    {"Vacío", "vacio"},   {"Sagrado", "luz"},       {"Oscuro", "sombra"},
    };
    return kElementos;
}

std::string familiaDeElemento(const std::string& etiqueta) {
    const auto& e = elementosYSusFamilias();
    const auto it = e.find(etiqueta);
    return it != e.end() ? it->second : "";
}

void destellosDeCarta(const std::string& familia, const ParticulasDeCartas::Color& color, std::uint32_t semilla,
                      double edad, std::vector<Destello>& out) {
    if (familia.empty() || edad < 0.0 || edad >= kDuracionDeCarta)
        return;
    // Un abalorio de +8 sobre el cuerpo (el torso y la cadera de la talla media, en el frame de 64x64), con
    // el triple de particulas: un estallido corto. Sin destellos: lo que se ve es la carta.
    HojaDeBrillo hoja;
    FuenteDeBrillo f;
    f.ranura = "carta";
    f.esAbalorio = true;
    f.brillo.nivel = 8;
    f.brillo.abalorio = true;
    f.brillo.particulas = familia;
    f.brillo.color = color;
    f.parte = 3.0f;
    f.conDestellos = false;
    for (int y = 34; y <= 50; ++y)
        for (int x = 26; x <= 38; ++x)
            f.pixeles[0].push_back({static_cast<std::uint8_t>(x), static_cast<std::uint8_t>(y)});
    hoja.fuentes.push_back(std::move(f));
    const std::size_t antes = out.size();
    destellosDeBrillo(hoja, 1, edad + 0.61 * static_cast<double>(semilla % 97u), out);
    // Entran deprisa (0,08 s) y se apagan en los ultimos 0,4 s.
    const float entrada = static_cast<float>(std::min(1.0, edad / 0.08));
    const float salida = static_cast<float>(std::min(1.0, (kDuracionDeCarta - edad) / 0.4));
    for (std::size_t i = antes; i < out.size(); ++i)
        out[i].alfa *= entrada * salida;
}

}  // namespace Game
