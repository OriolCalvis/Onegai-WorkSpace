// MAPA-01 — Ver include/Game/RumboNiveles.h.
#include "Game/RumboNiveles.h"

#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Game {

namespace {

std::vector<std::string> leeDeDisco(const std::string& nivel) {
    std::vector<std::string> out;
    std::ifstream f(nivel);
    if (!f) return out;
    std::stringstream texto;
    texto << f.rdbuf();
    const auto json = JsonValue::parse(texto.str());
    if (!json.isOk()) return out;
    const JsonValue& objetos = json.value()["objects"];
    for (std::size_t i = 0; i < objetos.size(); ++i) {
        const std::string destino = objetos[i]["targetLevel"].asString();
        if (!destino.empty()) out.push_back(destino);
    }
    return out;
}

}  // namespace

RumboNiveles::RumboNiveles() : m_lector(leeDeDisco) {}
RumboNiveles::RumboNiveles(Lector lector) : m_lector(std::move(lector)) {}

std::string RumboNiveles::nombre(const std::string& ruta) {
    const std::size_t barra = ruta.find_last_of("/\\");
    return barra == std::string::npos ? ruta : ruta.substr(barra + 1);
}

const std::vector<std::string>& RumboNiveles::salidas(const std::string& nivel) {
    const std::string clave = nombre(nivel);
    auto it = m_salidas.find(clave);
    if (it == m_salidas.end()) it = m_salidas.emplace(clave, m_lector(nivel)).first;
    return it->second;
}

std::set<std::string> RumboNiveles::primerosPasos(const std::string& desde, const std::vector<std::string>& hacia) {
    std::set<std::string> metas;
    for (const std::string& h : hacia) metas.insert(nombre(h));
    const std::string origen = nombre(desde);
    if (metas.empty() || metas.count(origen) > 0) return {};
    // Por anchura, capa a capa: cada nivel recuerda por que salidas de `desde`
    // se llega a el en el menor numero de puertas (puede haber varias: la
    // plaza tiene dos puertas al mercado).
    std::map<std::string, std::set<std::string>> via;  // nombre de nivel -> salidas de `desde`
    std::map<std::string, std::string> ruta;           // nombre de nivel -> ruta para leerlo
    std::vector<std::string> capa;
    via[origen];
    for (const std::string& s : salidas(desde)) {
        const std::string n = nombre(s);
        if (n == origen) continue;
        if (via.count(n) == 0) {
            capa.push_back(n);
            ruta[n] = s;
        }
        via[n].insert(s);
    }
    for (int vuelta = 0; !capa.empty() && vuelta < 64; ++vuelta) {
        std::set<std::string> pasos;
        for (const std::string& n : capa)
            if (metas.count(n) > 0) pasos.insert(via[n].begin(), via[n].end());
        if (!pasos.empty()) return pasos;
        std::vector<std::string> siguiente;
        std::set<std::string> enSiguiente;
        for (const std::string& n : capa) {
            for (const std::string& s : salidas(ruta[n])) {
                const std::string m = nombre(s);
                if (via.count(m) > 0 && enSiguiente.count(m) == 0) continue;  // ya visto, mas cerca
                if (via.count(m) == 0) {
                    siguiente.push_back(m);
                    enSiguiente.insert(m);
                    ruta[m] = s;
                }
                via[m].insert(via[n].begin(), via[n].end());
            }
        }
        capa = std::move(siguiente);
    }
    return {};
}

}  // namespace Game
