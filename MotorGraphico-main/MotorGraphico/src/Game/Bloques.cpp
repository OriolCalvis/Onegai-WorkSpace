// BLOQUES-01 — ver include/Game/Bloques.h.
#include "Game/Bloques.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

#include "Core/Json/JsonValue.h"
#include "Runtime/RuntimeTileMap.h"

namespace Game {

namespace {

Result<JsonValue> leeJson(const std::string& ruta) {
    std::ifstream entrada(ruta, std::ios::binary);
    if (!entrada) return Result<JsonValue>::Error("no puedo abrir " + ruta);
    std::stringstream texto;
    texto << entrada.rdbuf();
    auto leido = JsonValue::parse(texto.str());
    if (!leido.isOk()) return Result<JsonValue>::Error(ruta + ": " + leido.errorMessage());
    return leido;
}

}  // namespace

Result<bool> Bloques::carga(const std::string& carpetaAssets) {
    m_tipos.clear();
    m_tileset.clear();
    m_hoja.clear();
    m_umbral.clear();
    m_cargado = false;
    const std::string ruta = carpetaAssets + "/textures/bloques.json";
    std::ifstream existe(ruta, std::ios::binary);
    if (!existe) return Result<bool>::Ok(false);
    existe.close();
    auto leido = leeJson(ruta);
    if (!leido.isOk()) return Result<bool>::Error("Bloques: " + leido.errorMessage());
    const JsonValue& raiz = leido.value();
    m_hoja = raiz["hoja"].asString();
    m_tileset = raiz["tileset"].asString();
    m_umbral = raiz["umbral"].asString();
    const std::string indice = carpetaAssets + "/" + raiz["indiceTileset"].asString();
    if (m_hoja.empty() || m_tileset.empty() || !raiz["tipos"].isObject())
        return Result<bool>::Error("Bloques: " + ruta + " sin «hoja», «tileset» o «tipos»");
    auto tiles = leeJson(indice);
    if (!tiles.isOk()) return Result<bool>::Error("Bloques: " + tiles.errorMessage());
    std::map<std::string, int> gidPorNombre;
    const JsonValue& lista = tiles.value()["tiles"];
    for (std::size_t i = 0; lista.isArray() && i < lista.size(); ++i)
        gidPorNombre[lista[i]["nombre"].asString()] = lista[i]["gid"].asInt(0);
    for (const auto& [nombre, def] : raiz["tipos"].objectValues()) {
        const auto it = gidPorNombre.find(nombre);
        if (it == gidPorNombre.end() || it->second <= 0)
            return Result<bool>::Error("Bloques: el tile «" + nombre + "» no esta en " + indice);
        Tipo tipo;
        const JsonValue& mats = def["material"];
        if (mats.isArray()) {
            const JsonValue& pesos = def["pesosMaterial"];
            for (std::size_t i = 0; i < mats.size(); ++i) {
                const int peso = pesos.isArray() && i < pesos.size() ? pesos[i].asInt(1) : 1;
                if (peso > 0 && !mats[i].asString().empty()) tipo.materiales.push_back({mats[i].asString(), peso});
            }
        } else if (!mats.asString().empty()) {
            tipo.materiales.push_back({mats.asString(), 1});
        }
        tipo.bandas.clear();
        const JsonValue& bandas = def["bandas"];
        if (bandas.isArray()) {
            const JsonValue& pesos = def["pesos"];
            for (std::size_t i = 0; i < bandas.size(); ++i) {
                const int peso = pesos.isArray() && i < pesos.size() ? pesos[i].asInt(1) : 1;
                if (peso > 0) tipo.bandas.push_back({std::max(1, std::min(3, bandas[i].asInt(2))), peso});
            }
        }
        if (tipo.bandas.empty()) tipo.bandas.push_back({std::max(1, std::min(3, bandas.asInt(2))), 1});
        tipo.ventanas = def["ventanas"].asBool(true);
        tipo.puertas = def["puertas"].asBool(true);
        tipo.porCelda = def["variantePorCelda"].asBool(false);
        if (tipo.materiales.empty()) return Result<bool>::Error("Bloques: «" + nombre + "» sin material");
        m_tipos[it->second] = tipo;
    }
    m_cargado = true;
    return Result<bool>::Ok(true);
}

std::string Bloques::nombreFrame(const std::string& material, int bandas, char izq, char der) {
    return material + std::to_string(bandas) + "_" + izq + der;
}

unsigned Bloques::mezcla(int a, int b, int c) {
    unsigned h = 2166136261u;
    for (int v : {a, b, c}) {
        h ^= static_cast<unsigned>(v) + 0x9e3779b9u;
        h *= 16777619u;
        h ^= h >> 13;
    }
    return h;
}

const std::string& Bloques::Tipo::materialDe(unsigned hash) const {
    int total = 0;
    for (const auto& [material, peso] : materiales) total += peso;
    int tirada = static_cast<int>((hash >> 8) % static_cast<unsigned>(std::max(1, total)));
    for (const auto& [material, peso] : materiales) {
        tirada -= peso;
        if (tirada < 0) return material;
    }
    return materiales.front().first;
}

int Bloques::Tipo::bandasDe(unsigned hash) const {
    int total = 0;
    for (const auto& [plantas, peso] : bandas) total += peso;
    int tirada = static_cast<int>(hash % static_cast<unsigned>(std::max(1, total)));
    for (const auto& [plantas, peso] : bandas) {
        tirada -= peso;
        if (tirada < 0) return plantas;
    }
    return bandas.front().first;
}

std::vector<BloqueColocado> Bloques::planifica(const RuntimeTileMap& mapa, const std::string& tileset,
                                               const std::vector<GridCoord>& puertas,
                                               const std::set<std::pair<int, int>>* excluir) const {
    std::vector<BloqueColocado> salida;
    if (!m_cargado || tileset != m_tileset || mapa.getLayerCount() == 0) return salida;
    const int ancho = mapa.getWidth(), alto = mapa.getHeight();
    auto gidDe = [&](int x, int y) { return mapa.getTile(0, x, y).getTilesetID(); };
    auto tipoDe = [&](int x, int y) -> const Tipo* {
        if (x < 0 || y < 0 || x >= ancho || y >= alto) return nullptr;
        if (excluir != nullptr && excluir->count({x, y}) != 0) return nullptr;  // lo pinta un monumento
        const auto it = m_tipos.find(gidDe(x, y));
        return it == m_tipos.end() ? nullptr : &it->second;
    };
    auto transitable = [&](int x, int y) {
        return x >= 0 && y >= 0 && x < ancho && y < alto && !mapa.getTile(0, x, y).hasCollision();
    };
    std::set<std::pair<int, int>> conPuerta;
    for (const GridCoord& p : puertas) conPuerta.insert({p.x, p.y});

    // Edificios: componentes conexas (4 vecinos) del mismo gid. Cada uno
    // decide sus plantas y, si ninguna cara tiene puerta de verdad, una
    // puerta decorativa en una cara que de a una celda transitable.
    std::vector<int> edificio(static_cast<std::size_t>(ancho * alto), -1);
    struct Datos { int bandas = 2; std::string material; std::pair<int, int> puertaDeco{-1, -1}; char lado = 'i'; };
    std::vector<Datos> edificios;
    for (int y0 = 0; y0 < alto; ++y0) {
        for (int x0 = 0; x0 < ancho; ++x0) {
            const Tipo* tipo = tipoDe(x0, y0);
            if (tipo == nullptr || edificio[static_cast<std::size_t>(y0 * ancho + x0)] >= 0) continue;
            const int id = static_cast<int>(edificios.size());
            const int gid = gidDe(x0, y0);
            std::vector<std::pair<int, int>> pila{{x0, y0}}, celdas;
            edificio[static_cast<std::size_t>(y0 * ancho + x0)] = id;
            while (!pila.empty()) {
                const auto [x, y] = pila.back();
                pila.pop_back();
                celdas.push_back({x, y});
                for (const auto [dx, dy] : {std::pair<int, int>{1, 0}, {-1, 0}, {0, 1}, {0, -1}}) {
                    const int nx = x + dx, ny = y + dy;
                    if (nx < 0 || ny < 0 || nx >= ancho || ny >= alto) continue;
                    std::size_t k = static_cast<std::size_t>(ny * ancho + nx);
                    if (edificio[k] >= 0 || gidDe(nx, ny) != gid) continue;
                    edificio[k] = id;
                    pila.push_back({nx, ny});
                }
            }
            Datos d;
            d.bandas = tipo->bandasDe(mezcla(gid, x0, y0));
            d.material = tipo->materialDe(mezcla(gid, x0, y0));
            bool puertaReal = false;
            std::vector<std::pair<std::pair<int, int>, char>> candidatas;
            for (const auto& [x, y] : celdas) {
                if (tipoDe(x, y + 1) == nullptr && conPuerta.count({x, y + 1}) != 0) puertaReal = true;
                if (tipoDe(x + 1, y) == nullptr && conPuerta.count({x + 1, y}) != 0) puertaReal = true;
                if (tipoDe(x, y + 1) == nullptr && transitable(x, y + 1)) candidatas.push_back({{x, y}, 'i'});
                if (tipoDe(x + 1, y) == nullptr && transitable(x + 1, y)) candidatas.push_back({{x, y}, 'd'});
            }
            if (!puertaReal && tipo->puertas && !candidatas.empty()) {
                std::sort(candidatas.begin(), candidatas.end());
                const auto& c = candidatas[mezcla(x0, y0, static_cast<int>(celdas.size())) % candidatas.size()];
                d.puertaDeco = c.first;
                d.lado = c.second;
            }
            edificios.push_back(d);
        }
    }

    for (int y = 0; y < alto; ++y) {
        for (int x = 0; x < ancho; ++x) {
            const Tipo* tipo = tipoDe(x, y);
            if (tipo == nullptr) continue;
            const Datos& d = edificios[static_cast<std::size_t>(edificio[static_cast<std::size_t>(y * ancho + x)])];
            // La cara izquierda mira a (x, y+1); la derecha, a (x+1, y).
            const bool expIzq = tipoDe(x, y + 1) == nullptr;
            const bool expDer = tipoDe(x + 1, y) == nullptr;
            char izq = 'p', der = 'p';
            if (expIzq) {
                if (tipo->puertas && conPuerta.count({x, y + 1}) != 0) izq = 'd';
                else if (d.puertaDeco == std::make_pair(x, y) && d.lado == 'i') izq = 'd';
                else if (tipo->ventanas && ventanaIzquierda(x, y)) izq = 'v';
            }
            if (expDer) {
                if (tipo->puertas && conPuerta.count({x + 1, y}) != 0) der = 'd';
                else if (d.puertaDeco == std::make_pair(x, y) && d.lado == 'd') der = 'd';
                else if (tipo->ventanas && ventanaDerecha(x, y)) der = 'v';
            }
            const std::string& material = tipo->porCelda ? tipo->materialDe(mezcla(gidDe(x, y), x / 3, y / 3)) : d.material;
            salida.push_back({x, y, nombreFrame(material, d.bandas, izq, der), izq, der, d.bandas, tipo->puertas});
        }
    }
    return salida;
}

std::set<std::pair<int, int>> celdasConPuertaEnPared(const std::vector<BloqueColocado>& plan) {
    std::set<std::pair<int, int>> celdas;
    for (const BloqueColocado& b : plan) {
        if (b.izq == 'd') celdas.insert({b.x, b.y + 1});
        if (b.der == 'd') celdas.insert({b.x + 1, b.y});
    }
    return celdas;
}

}  // namespace Game
