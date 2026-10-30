// MAPA-02 — ver include/Game/MarcasMapa.h.
#include "Game/MarcasMapa.h"

#include <algorithm>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

namespace Game {

namespace {
const std::vector<Chincheta> kNinguna;
}  // namespace

MarcasMapa::Cambio MarcasMapa::alterna(const std::string& nivel, int x, int y) {
    std::vector<Chincheta>& lista = chinchetas[nivel];
    const auto aqui = std::find_if(lista.begin(), lista.end(), [&](const Chincheta& c) { return c.x == x && c.y == y; });
    if (aqui != lista.end()) {
        lista.erase(aqui);
        if (lista.empty()) chinchetas.erase(nivel);
        return Cambio::Quitada;
    }
    if (static_cast<int>(lista.size()) >= kMaximo) return Cambio::Llena;
    Chincheta c;
    c.x = x, c.y = y;
    // El color sigue al de la ultima; el numero, el primero libre (1..5).
    c.color = lista.empty() ? 0 : (lista.back().color + 1) % kColores;
    for (c.numero = 1; std::any_of(lista.begin(), lista.end(), [&](const Chincheta& o) { return o.numero == c.numero; });)
        ++c.numero;
    lista.push_back(c);
    return Cambio::Puesta;
}

const std::vector<Chincheta>& MarcasMapa::de(const std::string& nivel) const {
    const auto it = chinchetas.find(nivel);
    return it == chinchetas.end() ? kNinguna : it->second;
}

bool MarcasMapa::operator==(const MarcasMapa& o) const {
    if (misionSeguida != o.misionSeguida || chinchetas.size() != o.chinchetas.size()) return false;
    for (const auto& [nivel, lista] : chinchetas) {
        const std::vector<Chincheta>& otra = o.de(nivel);
        if (otra.size() != lista.size()) return false;
        for (std::size_t i = 0; i < lista.size(); ++i)
            if (lista[i].x != otra[i].x || lista[i].y != otra[i].y || lista[i].color != otra[i].color ||
                lista[i].numero != otra[i].numero)
                return false;
    }
    return true;
}

// "mapa": {"seguida": "dia1", "chinchetas": {"ciudad_centro.json": [[x, y, color, numero], ...]}}
void escribeMarcasMapa(JsonWriter& out, const MarcasMapa& marcas) {
    if (marcas.vacias()) return;
    out.key("mapa");
    out.beginObject();
    if (!marcas.misionSeguida.empty()) out.member("seguida", marcas.misionSeguida);
    if (!marcas.chinchetas.empty()) {
        out.key("chinchetas");
        out.beginObject();
        for (const auto& [nivel, lista] : marcas.chinchetas) {
            out.key(nivel);
            out.beginArray();
            for (const Chincheta& c : lista) {
                out.beginArray();
                out.value(c.x), out.value(c.y), out.value(c.color), out.value(c.numero);
                out.endArray();
            }
            out.endArray();
        }
        out.endObject();
    }
    out.endObject();
}

MarcasMapa leeMarcasMapa(const JsonValue& nodo) {
    MarcasMapa m;
    if (!nodo.isObject()) return m;
    m.misionSeguida = nodo["seguida"].asString("");
    const JsonValue& todas = nodo["chinchetas"];
    if (!todas.isObject()) return m;
    for (const auto& [nivel, lista] : todas.objectValues()) {
        if (!lista.isArray()) continue;
        for (std::size_t i = 0; i < lista.size(); ++i) {
            const JsonValue& v = lista[i];
            if (!v.isArray() || v.size() < 2 || !v[std::size_t{0}].isNumber() || !v[std::size_t{1}].isNumber()) continue;
            std::vector<Chincheta>& destino = m.chinchetas[nivel];
            if (static_cast<int>(destino.size()) >= MarcasMapa::kMaximo) break;
            Chincheta c;
            c.x = v[std::size_t{0}].asInt(0), c.y = v[std::size_t{1}].asInt(0);
            c.color = std::clamp(v[std::size_t{2}].asInt(0), 0, MarcasMapa::kColores - 1);
            c.numero = std::clamp(v[std::size_t{3}].asInt(static_cast<int>(destino.size()) + 1), 1, MarcasMapa::kMaximo);
            destino.push_back(c);
        }
        if (m.chinchetas.count(nivel) > 0 && m.chinchetas[nivel].empty()) m.chinchetas.erase(nivel);
    }
    return m;
}

}  // namespace Game
