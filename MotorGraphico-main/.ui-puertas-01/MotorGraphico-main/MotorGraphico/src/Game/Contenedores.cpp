#include "Game/Contenedores.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <set>
#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "Level/ObjectCatalog.h"
#include "RPG/Narrative/NarrativeState.h"
#include "RPG/RandomEngine.h"

namespace Game {
namespace {
std::string hex(const std::string& s) {
    const char* digitos = "0123456789abcdef";
    std::string r;
    for (unsigned char c : s) { r += digitos[c >> 4]; r += digitos[c & 15]; }
    return r;
}
std::string deshex(const std::string& s) {
    if (s.size() % 2 != 0) return {};
    std::string r;
    for (std::size_t i = 0; i < s.size(); i += 2) {
        auto valor = [](char c) { return c >= '0' && c <= '9' ? c - '0' :
            c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1; };
        const int a = valor(s[i]), b = valor(s[i + 1]);
        if (a < 0 || b < 0) return {};
        r += static_cast<char>(a * 16 + b);
    }
    return r;
}
std::uint64_t huella(const std::string& s) {
    std::uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : s) { h ^= c; h *= 1099511628211ULL; }
    return h;
}
std::string item(const std::string& clave, const std::string& id) { return clave + ".item." + hex(id); }
std::string unico(const std::string& id) { return "botin.unico." + hex(id); }
}  // namespace

std::string claveContenedor(const std::string& nivel, const std::string& instancia) {
    return "botin." + hex(nivel) + "." + hex(instancia);
}

bool contenedorAbierto(const RPG::NarrativeState& estado, const std::string& clave) {
    return estado.hasFlag(clave + ".abierto");
}

ContenidoContenedor contenidoContenedor(const RPG::NarrativeState& estado, const std::string& clave) {
    ContenidoContenedor r;
    r.clave = clave;
    r.oro = std::max(0, estado.variable(clave + ".oro"));
    const std::string prefijo = clave + ".item.";
    for (const auto& [nombre, cantidad] : estado.variables()) {
        if (nombre.compare(0, prefijo.size(), prefijo) != 0) continue;
        const std::string id = deshex(nombre.substr(prefijo.size()));
        if (id.empty() || cantidad < 0 || cantidad > 128) continue;
        r.objetos.insert(r.objetos.end(), static_cast<std::size_t>(cantidad), id);
    }
    return r;
}

Result<ContenidoContenedor> generaContenedor(const ContenedorDef& def, const TablaBotin& tabla,
    const ObjectCatalog& catalogo, RPG::NarrativeState& estado, const std::string& clave, int semilla) {
    using R = Result<ContenidoContenedor>;
    if (clave.empty() || def.tipo.empty() || def.tabla != tabla.id || def.presupuesto < 0 || def.presupuesto > 1000000)
        return R::Error("Contenedor sin identidad, tabla o presupuesto válido");
    if (contenedorAbierto(estado, clave)) return R::Ok(contenidoContenedor(estado, clave));
    if (tabla.tiradas < 0 || tabla.tiradas > 64 || def.garantizados.size() > 64 ||
        tabla.oroMin < 0 || tabla.oroMax < tabla.oroMin || tabla.oroMax > 1000000)
        return R::Error("Límites de tabla de botín inválidos");
    std::set<std::string> ids;
    long long pesos = 0;
    for (const auto& e : tabla.entradas) {
        if (e.id.empty() || !ids.insert(e.id).second || e.valor <= 0 || e.peso <= 0 ||
            catalogo.find(e.id) == nullptr) return R::Error("Entrada de botín inválida: " + e.id);
        const int precio = catalogo.find(e.id)->pickup.price;
        if (precio > 0 && precio != e.valor) return R::Error("Valor distinto al catálogo: " + e.id);
        pesos += e.peso;
    }
    if (pesos > std::numeric_limits<int>::max()) return R::Error("La suma de pesos no cabe");
    // Se genera sobre una copia: un garantizado imposible no deja medio premio.
    RPG::NarrativeState siguiente = estado;
    int restante = def.presupuesto;
    auto concede = [&](const std::string& id, int valor, bool esUnico) {
        siguiente.addToVariable(item(clave, id), 1);
        restante -= valor;
        if (esUnico) siguiente.setFlag(unico(id));
    };
    for (const std::string& id : def.garantizados) {
        const ObjectDefinition* objeto = catalogo.find(id);
        if (objeto == nullptr) return R::Error("Garantizado desconocido: " + id);
        const auto e = std::find_if(tabla.entradas.begin(), tabla.entradas.end(),
            [&](const EntradaBotin& x) { return x.id == id; });
        const int valor = e != tabla.entradas.end() ? e->valor : objeto->pickup.price;
        const bool esUnico = e != tabla.entradas.end() && e->unico;
        if (valor < 0 || valor > restante) return R::Error("El garantizado supera el presupuesto: " + id);
        if (esUnico && siguiente.hasFlag(unico(id))) return R::Error("Único ya reservado: " + id);
        concede(id, valor, esUnico);
    }
    if (tabla.oroMin > restante) return R::Error("El oro mínimo supera el presupuesto restante");
    RPG::Xoroshiro128p rng(huella(clave) ^ static_cast<std::uint64_t>(semilla));
    const int oro = rng.roll_in_range(tabla.oroMin, std::min(tabla.oroMax, restante));
    siguiente.setVariable(clave + ".oro", oro);
    restante -= oro;
    for (int tirada = 0; tirada < tabla.tiradas; ++tirada) {
        int suma = 0;
        for (const auto& e : tabla.entradas)
            if (e.valor <= restante && !(e.unico && siguiente.hasFlag(unico(e.id)))) suma += e.peso;
        if (suma == 0) break;
        int dado = rng.roll_in_range(1, suma);
        for (const auto& e : tabla.entradas) {
            if (e.valor > restante || (e.unico && siguiente.hasFlag(unico(e.id)))) continue;
            dado -= e.peso;
            if (dado <= 0) { concede(e.id, e.valor, e.unico); break; }
        }
    }
    siguiente.setFlag(clave + ".abierto");
    estado = std::move(siguiente);
    return R::Ok(contenidoContenedor(estado, clave));
}

bool cogeDelContenedor(RPG::NarrativeState& estado, const std::string& clave,
                      std::size_t indice, std::vector<std::string>& mochila) {
    const auto r = contenidoContenedor(estado, clave);
    if (!contenedorAbierto(estado, clave) || indice >= r.objetos.size()) return false;
    const std::string& id = r.objetos[indice];
    mochila.push_back(id);
    estado.addToVariable(item(clave, id), -1);
    return true;
}

int cogeOroContenedor(RPG::NarrativeState& estado, const std::string& clave) {
    if (!contenedorAbierto(estado, clave)) return 0;
    const int oro = std::max(0, estado.variable(clave + ".oro"));
    estado.setVariable(clave + ".oro", 0);
    return oro;
}
}  // namespace Game
