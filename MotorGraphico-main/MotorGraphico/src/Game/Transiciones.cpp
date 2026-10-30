// AV16-10 — ver include/Game/Transiciones.h.
#include "Game/Transiciones.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Game {

Result<bool> Transiciones::carga(const std::string& carpetaAssets) {
    m_rango.clear();
    const std::string ruta = carpetaAssets + "/textures/transiciones.json";
    std::ifstream entrada(ruta, std::ios::binary);
    if (!entrada) return Result<bool>::Ok(false);
    std::stringstream texto;
    texto << entrada.rdbuf();
    auto leido = JsonValue::parse(texto.str());
    if (!leido.isOk()) return Result<bool>::Error("Transiciones: " + ruta + ": " + leido.errorMessage());
    const JsonValue& raiz = leido.value();
    m_imagen = raiz["imagen"].asString("transiciones.png");
    m_columnas = raiz["columnas"].asInt(15);
    m_ancho = raiz["celda"]["ancho"].asInt(128);
    m_alto = raiz["celda"]["alto"].asInt(64);
    const JsonValue& mats = raiz["materiales"];
    for (std::size_t i = 0; mats.isArray() && i < mats.size(); ++i) {
        const int gid = mats[i]["gid"].asInt(0);
        if (gid > 0) m_rango[gid] = mats[i]["rango"].asInt(static_cast<int>(i));
    }
    return Result<bool>::Ok(!m_rango.empty());
}

int Transiciones::rango(int gid) const {
    const auto it = m_rango.find(gid);
    return it == m_rango.end() ? -1 : it->second;
}

std::vector<Transiciones::Capa> Transiciones::capas(const std::array<int, 4>& gids) const {
    std::array<int, 4> r{};
    for (std::size_t i = 0; i < 4; ++i) {
        r[i] = rango(gids[i]);
        if (r[i] < 0) return {};
    }
    const std::set<int> distintos(r.begin(), r.end());
    std::vector<Capa> out;
    bool primero = true;
    for (const int rango : distintos) {  // de menor a mayor: lo de arriba se dibuja despues
        if (primero) {  // el de abajo ya esta dibujado por la cuadricula de siempre
            primero = false;
            continue;
        }
        int mascara = 0;
        for (int i = 0; i < 4; ++i)
            if (r[static_cast<std::size_t>(i)] >= rango) mascara |= 1 << i;
        if (mascara != 15) out.push_back({rango, mascara});
    }
    return out;
}

}  // namespace Game
