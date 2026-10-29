// BLOQUES-03 — ver include/Game/Monumentos.h.
#include "Game/Monumentos.h"

#include <algorithm>
#include <cmath>
#include <fstream>
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

Result<bool> Monumentos::carga(const std::string& carpetaAssets) {
    m_defs.clear();
    m_gidPorNombre.clear();
    m_tileset.clear();
    m_cargado = false;
    const std::string ruta = carpetaAssets + "/textures/monumentos.json";
    std::ifstream existe(ruta, std::ios::binary);
    if (!existe) return Result<bool>::Ok(false);
    existe.close();
    auto leido = leeJson(ruta);
    if (!leido.isOk()) return Result<bool>::Error("Monumentos: " + leido.errorMessage());
    const JsonValue& raiz = leido.value();
    m_tileset = raiz["tileset"].asString();
    const JsonValue& lista = raiz["monumentos"];
    if (m_tileset.empty() || !lista.isArray())
        return Result<bool>::Error("Monumentos: " + ruta + " sin «tileset» o «monumentos»");
    auto tiles = leeJson(carpetaAssets + "/" + raiz["indiceTileset"].asString());
    if (!tiles.isOk()) return Result<bool>::Error("Monumentos: " + tiles.errorMessage());
    const JsonValue& indice = tiles.value()["tiles"];
    for (std::size_t i = 0; indice.isArray() && i < indice.size(); ++i)
        m_gidPorNombre[indice[i]["nombre"].asString()] = indice[i]["gid"].asInt(0);
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const JsonValue& m = lista[i];
        Def d;
        d.id = m["id"].asString();
        d.hoja = m["hoja"].asString();
        d.ancho = m["ancho"].asInt(0);
        d.alto = m["alto"].asInt(0);
        d.tiraAncho = m["tiraAncho"].asInt(32);
        d.altoHoja = m["altoHoja"].asInt(0);
        d.columnas = m["columnas"].asInt(0);
        d.anclajeX = m["anclaje"]["x"].asInt(0);
        d.anclajeY = m["anclaje"]["y"].asInt(0);
        const JsonValue& ps = m["posaderos"];  // MASC-01: [[x, y, altura], ...]
        for (std::size_t k = 0; ps.isArray() && k < ps.size(); ++k)
            if (ps[k].isArray() && ps[k].size() >= 3)
                d.posaderos.push_back({static_cast<float>(ps[k][0].asNumber(0.0)), static_cast<float>(ps[k][1].asNumber(0.0)),
                                       static_cast<float>(ps[k][2].asNumber(0.0))});
        const JsonValue& ls = m["libres"];  // H-CIU-2: [[i, j], ...] casillas de la huella sin base
        for (std::size_t k = 0; ls.isArray() && k < ls.size(); ++k)
            if (ls[k].isArray() && ls[k].size() >= 2) d.libres.push_back({ls[k][0].asInt(0), ls[k][1].asInt(0)});
        const JsonValue& ts = m["tiles"];
        for (std::size_t k = 0; ts.isArray() && k < ts.size(); ++k) {
            const std::string nombre = ts[k].asString();
            if (m_gidPorNombre.find(nombre) == m_gidPorNombre.end())
                return Result<bool>::Error("Monumentos: el tile «" + nombre + "» de " + d.id + " no esta en el indice");
            d.tiles.push_back(nombre);
        }
        if (d.id.empty() || d.hoja.empty() || d.tiles.empty() || d.ancho <= 0 || d.alto <= 0 || d.columnas <= 0 ||
            d.altoHoja <= 0)
            return Result<bool>::Error("Monumentos: «" + d.id + "» incompleto (id, hoja, tiles, ancho, alto, columnas, altoHoja)");
        m_defs.push_back(d);
    }
    m_cargado = true;
    return Result<bool>::Ok(true);
}

std::pair<int, int> Monumentos::celdaDeTira(int ancho, int alto, int k, int columnas, int anclajeX, int tiraAncho) {
    (void)columnas;
    // Centro de la tira respecto al origen de la celda (0,0) de la huella:
    // la esquina delantera esta en x = (ancho - alto) * 32 + 32 y el anclaje
    // de la hoja cae ahi. Las celdas con diagonal d = i - j tienen el centro
    // en x = d * 32 + 32; la que esta mas delante es la de mayor i + j.
    const float xr = static_cast<float>((ancho - alto) * 32 + 32 - anclajeX + k * tiraAncho) + static_cast<float>(tiraAncho) * 0.5f;
    int d = static_cast<int>(std::lround((xr - 32.0f) / 32.0f));
    d = std::max(-(alto - 1), std::min(ancho - 1, d));
    const int i = std::min(ancho - 1, alto - 1 + d);
    return {i, i - d};
}

namespace {

// Filas o columnas de manzana de mas que admite un monumento: tres para los grandes (iglesia, coliseo,
// castillo...: 30 casillas o mas), una para las casas y los locales (si no, una manzana de cuatro casas
// se volveria una casa con patio).
int sobraMaxima(int ancho, int alto) { return ancho * alto >= 30 ? 3 : 1; }

struct Huella {  // una componente conexa (4 vecinos) de tiles de edificio
    int x0 = 0, y0 = 0, ancho = 0, alto = 0, celdas = 0;
    int porOri[4] = {0, 0, 0, 0};  // H-CIU-2: casillas de cada orientacion (solo familias edifici_o0..o3)
};

// «edifici_o2» -> base «edifici», orientacion 2. Cualquier otro nombre: base vacia.
bool partesOrientacion(const std::string& nombre, std::string& base, int& ori) {
    if (nombre.size() < 4 || nombre[nombre.size() - 3] != '_' || nombre[nombre.size() - 2] != 'o') return false;
    const char c = nombre.back();
    if (c < '0' || c > '3') return false;
    base = nombre.substr(0, nombre.size() - 3);
    ori = c - '0';
    return true;
}

// oriDeGid: gid -> orientacion (0-3) cuando la huella es la de una familia entera; si no, null.
std::vector<Huella> huellasDe(const RuntimeTileMap& mapa, const std::set<int>& gids, const std::map<int, int>* oriDeGid = nullptr) {
    std::vector<Huella> salida;
    const int ancho = mapa.getWidth(), alto = mapa.getHeight();
    std::vector<char> visto(static_cast<std::size_t>(ancho * alto), 0);
    for (int y0 = 0; y0 < alto; ++y0) {
        for (int x0 = 0; x0 < ancho; ++x0) {
            if (visto[static_cast<std::size_t>(y0 * ancho + x0)] || gids.count(mapa.getTile(0, x0, y0).getTilesetID()) == 0) continue;
            std::vector<std::pair<int, int>> pila{{x0, y0}};
            visto[static_cast<std::size_t>(y0 * ancho + x0)] = 1;
            Huella h;
            int minX = x0, maxX = x0, minY = y0, maxY = y0;
            while (!pila.empty()) {
                const auto [x, y] = pila.back();
                pila.pop_back();
                ++h.celdas;
                if (oriDeGid != nullptr) ++h.porOri[oriDeGid->at(mapa.getTile(0, x, y).getTilesetID())];
                minX = std::min(minX, x), maxX = std::max(maxX, x), minY = std::min(minY, y), maxY = std::max(maxY, y);
                for (const auto [dx, dy] : {std::pair<int, int>{1, 0}, {-1, 0}, {0, 1}, {0, -1}}) {
                    const int nx = x + dx, ny = y + dy;
                    if (nx < 0 || ny < 0 || nx >= ancho || ny >= alto) continue;
                    const std::size_t k = static_cast<std::size_t>(ny * ancho + nx);
                    if (visto[k] || gids.count(mapa.getTile(0, nx, ny).getTilesetID()) == 0) continue;
                    visto[k] = 1;
                    pila.push_back({nx, ny});
                }
            }
            h.x0 = minX, h.y0 = minY, h.ancho = maxX - minX + 1, h.alto = maxY - minY + 1;
            salida.push_back(h);
        }
    }
    return salida;
}

// Definiciones que dibujan la huella h con los tiles «tiles» (ver planifica).
std::vector<const Monumentos::Def*> candidatasDe(const std::vector<Monumentos::Def>& defs, const Huella& h,
                                                 const std::vector<std::string>& tiles) {
    using Def = Monumentos::Def;
    std::vector<const Def*> candidatas;
    for (const Def& d : defs)
        // La huella tiene que medir lo que dice el monumento (el umbral de
        // la puerta, que no es tile de edificio, puede faltar; y mas: la
        // esquina derruida de la iglesia son ruinas).
        if (d.tiles == tiles && d.ancho == h.ancho && d.alto == h.alto && h.celdas * 10 >= d.ancho * d.alto * 7)
            candidatas.push_back(&d);
    // Oriol (22/10): «me has quitado los monumentos». Si no hay uno que mida justo, vale el que
    // cabe con hasta tres filas o columnas de sobra (la manzana del Excel es mas grande: el coliseo
    // que pinto mide 8x9 y el dibujo 8x6); la sobra queda detras, como patio del monumento.
    if (candidatas.empty())
        for (const Def& d : defs)
            if (d.tiles == tiles && h.ancho - d.ancho >= 0 && h.ancho - d.ancho <= sobraMaxima(d.ancho, d.alto) &&
                h.alto - d.alto >= 0 && h.alto - d.alto <= sobraMaxima(d.ancho, d.alto) && h.celdas * 10 >= h.ancho * h.alto * 7)
                candidatas.push_back(&d);
    // Si caben varios, el que deja menos sobra.
    if (candidatas.size() > 1) {
        auto sobra = [&](const Def* d) { return (h.ancho - d->ancho) + (h.alto - d->alto); };
        int mejor = sobra(candidatas.front());
        for (const Def* d : candidatas) mejor = std::min(mejor, sobra(d));
        candidatas.erase(std::remove_if(candidatas.begin(), candidatas.end(), [&](const Def* d) { return sobra(d) != mejor; }),
                         candidatas.end());
    }
    return candidatas;
}

MonumentoColocado colocaEn(const Huella& h, const Monumentos::Def& d) {
    using Def = Monumentos::Def;
    MonumentoColocado m;
    m.id = d.id;
    m.hoja = d.hoja;
    // Alineado por delante: la sobra queda detras (arriba y a la izquierda en pantalla).
    m.x0 = h.x0 + (h.ancho - d.ancho);
    m.y0 = h.y0 + (h.alto - d.alto);
    m.ancho = d.ancho;
    m.alto = d.alto;
    m.cubreX0 = h.x0;
    m.cubreY0 = h.y0;
    m.cubreAncho = h.ancho;
    m.cubreAlto = h.alto;
    for (int k = 0; k < d.columnas; ++k) {
        const auto [i, j] = Monumentos::celdaDeTira(d.ancho, d.alto, k, d.columnas, d.anclajeX, d.tiraAncho);
        TiraMonumento t;
        t.x = m.x0 + i;
        t.y = m.y0 + j;
        t.frame = k + 1;
        t.dx = static_cast<float>((d.ancho - d.alto) * 32 + 32 - d.anclajeX + k * d.tiraAncho - (i - j) * 32);
        t.dy = static_cast<float>((d.ancho + d.alto) * 16 - d.anclajeY - (i + j) * 16);
        m.tiras.push_back(t);
    }
    for (const PosaderoMonumento& p : d.posaderos)
        m.posaderos.push_back({static_cast<float>(m.x0) + p.x, static_cast<float>(m.y0) + p.y, p.altura});
    // H-CIU-2: solo si la definicion sabe donde esta su base (monumentos.json «libres»).
    if (!d.libres.empty()) {
        for (const auto& [i, j] : d.libres) m.libres.push_back({m.x0 + i, m.y0 + j});
        for (int y = h.y0; y < h.y0 + h.alto; ++y)
            for (int x = h.x0; x < h.x0 + h.ancho; ++x)
                if (x < m.x0 || y < m.y0 || x >= m.x0 + d.ancho || y >= m.y0 + d.alto) m.libres.push_back({x, y});
    }
    return m;
}

}  // namespace

std::vector<MonumentoColocado> Monumentos::planifica(const RuntimeTileMap& mapa, const std::string& tileset) const {
    std::vector<MonumentoColocado> salida;
    if (!m_cargado || tileset != m_tileset || mapa.getLayerCount() == 0) return salida;
    // ARTE-03: varias definiciones pueden compartir tiles (las casas de
    // piedra de 2x2, 3x3... y sus variantes). Las huellas de cada juego de
    // tiles se calculan una vez; cada huella se dibuja con la definicion de
    // su tamano y, si hay varias, con la variante que decide un hash de su
    // posicion (siempre la misma para la misma casa).
    //
    // H-CIU-2 (E-CIU-3): un edificio es su familia entera. Las casas del Excel salen con casillas de
    // varias orientaciones (edifici_o0 y edifici_o3 en la misma casa); si se miran por separado cada
    // trozo es «otro edificio» y la casa cae a bloques. Para los tiles «<familia>_oN» la huella es la
    // union de las cuatro orientaciones y el sprite es el de la orientacion con mas casillas (empate:
    // la de menor numero, o0 si cabe). Lo que no se coloca asi se prueba tambien por orientaciones
    // sueltas, como antes, para no perder las casas pegadas que ya salian con sprite.
    struct Grupo {
        std::vector<std::vector<std::string>> orientaciones;  // tiles de cada orientacion (indice = orientacion), vacio si no hay
        std::set<int> gids;
        std::map<int, int> oriDeGid;
    };
    std::map<std::string, Grupo> grupos;  // clave: base de la familia o juego de tiles
    std::map<std::vector<std::string>, std::vector<Huella>> sueltas;  // huellas por juego de tiles exacto (la via de siempre)
    for (const Def& d : m_defs) {
        std::string base;
        int ori = 0;
        const bool fam = d.tiles.size() == 1 && partesOrientacion(d.tiles[0], base, ori);
        if (!sueltas.count(d.tiles)) {
            std::set<int> gids;
            for (const std::string& n : d.tiles) gids.insert(m_gidPorNombre.at(n));
            sueltas[d.tiles] = huellasDe(mapa, gids);
        }
        if (!fam) continue;
        Grupo& g = grupos[base];
        g.orientaciones.resize(4);
        g.orientaciones[static_cast<std::size_t>(ori)] = d.tiles;
        g.gids.insert(m_gidPorNombre.at(d.tiles[0]));
        g.oriDeGid[m_gidPorNombre.at(d.tiles[0])] = ori;
    }
    std::vector<std::pair<Huella, const Def*>> colocadas;
    auto hash = [](const Huella& h) { return (static_cast<unsigned>(h.x0) * 73856093u) ^ (static_cast<unsigned>(h.y0) * 19349663u); };
    auto solapa = [&](const Huella& h) {
        for (const auto& [o, d] : colocadas) {
            (void)d;
            if (h.x0 < o.x0 + o.ancho && o.x0 < h.x0 + h.ancho && h.y0 < o.y0 + o.alto && o.y0 < h.y0 + h.alto) return true;
        }
        return false;
    };
    auto elige = [&](const Huella& h, const std::vector<std::string>& tiles) -> const Def* {
        const auto candidatas = candidatasDe(m_defs, h, tiles);
        return candidatas.empty() ? nullptr : candidatas[hash(h) % candidatas.size()];
    };
    // 1) familias enteras
    for (const auto& [base, g] : grupos) {
        for (const Huella& h : huellasDe(mapa, g.gids, &g.oriDeGid)) {
            int orden[4] = {0, 1, 2, 3};
            std::stable_sort(orden, orden + 4, [&](int a, int b) { return h.porOri[a] > h.porOri[b]; });
            for (int k = 0; k < 4; ++k) {
                const auto& tiles = g.orientaciones[static_cast<std::size_t>(orden[k])];
                if (tiles.empty() || h.porOri[orden[k]] == 0) continue;
                if (const Def* d = elige(h, tiles)) {
                    colocadas.push_back({h, d});
                    break;
                }
            }
        }
    }
    // 2) la via de siempre, por juego de tiles exacto, para lo que la familia no ha colocado
    for (const auto& [tiles, lista] : sueltas) {
        for (const Huella& h : lista) {
            if (solapa(h)) continue;
            if (const Def* d = elige(h, tiles)) colocadas.push_back({h, d});
        }
    }
    for (const auto& [h, d] : colocadas) salida.push_back(colocaEn(h, *d));
    return salida;
}

int Monumentos::liberaCeldas(RuntimeTileMap& mapa, const std::vector<MonumentoColocado>& plan) {
    int n = 0;
    for (const MonumentoColocado& m : plan)
        for (const auto& [x, y] : m.libres)
            if (mapa.quitaColision(x, y)) ++n;
    return n;
}

std::set<std::pair<int, int>> Monumentos::celdasCubiertas(const std::vector<MonumentoColocado>& plan) {
    std::set<std::pair<int, int>> celdas;
    for (const MonumentoColocado& m : plan) {
        for (int j = 0; j < m.alto; ++j)
            for (int i = 0; i < m.ancho; ++i) celdas.insert({m.x0 + i, m.y0 + j});
        for (int j = 0; j < m.cubreAlto; ++j)
            for (int i = 0; i < m.cubreAncho; ++i) celdas.insert({m.cubreX0 + i, m.cubreY0 + j});
    }
    return celdas;
}

}  // namespace Game
