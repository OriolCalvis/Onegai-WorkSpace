#include "Game/Contenedores.h"

#include <algorithm>
#include <cmath>
#include <set>
#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

namespace Game {
namespace {
bool entero(const JsonValue& n, int minimo, int maximo) {
    const double v = n.asNumber(-1);
    return n.isNumber() && std::isfinite(v) && v == std::floor(v) && v >= minimo && v <= maximo;
}
}  // namespace

Result<ContenedorDef> leeContenedor(const JsonValue& n) {
    using R = Result<ContenedorDef>;
    ContenedorDef d;
    if (n.isNull()) return R::Ok(d);
    if (!n.isObject()) return R::Error("contenedor debe ser un objeto");
    d.tipo = n["tipo"].asString();
    const std::set<std::string> tipos{"cofre", "barril", "saco", "armario", "cadaver", "escondite"};
    if (!tipos.count(d.tipo)) return R::Error("Tipo de contenedor desconocido: " + d.tipo);
    d.tabla = n["tabla"].asString();
    if (d.tabla.empty() || !entero(n["presupuesto"], 0, 1000000))
        return R::Error("Contenedor sin tabla o con presupuesto inválido");
    d.presupuesto = n["presupuesto"].asInt();
    const JsonValue& g = n["garantizados"];
    if (!g.isNull() && (!g.isArray() || g.size() > 64)) return R::Error("garantizados debe ser una lista de hasta 64 ids");
    for (std::size_t i = 0; i < g.size(); ++i) {
        if (!g[i].isString() || g[i].asString().empty()) return R::Error("Garantizado sin id");
        d.garantizados.push_back(g[i].asString());
    }
    const JsonValue& r = n["requisitos"];
    if (!r.isNull() && !r.isObject()) return R::Error("requisitos debe ser un objeto");
    for (const char* campo : {"percepcion", "cd"})
        if (!r[campo].isNull() && !entero(r[campo], 0, 8))
            return R::Error(std::string("requisitos.") + campo + " debe ser un número de dados entre 0 y 8");
    if (!r["abrir"].isNull() && !r["abrir"].isString()) return R::Error("requisitos.abrir debe ser texto");
    d.requisitos.percepcion = r["percepcion"].asInt(0);
    d.requisitos.cd = r["cd"].asInt(0);
    d.requisitos.abrir = r["abrir"].asString();
    const std::string& abrir = d.requisitos.abrir;
    if (!abrir.empty() && abrir != "ganzua" && abrir != "fuerza" &&
        !(abrir.compare(0, 6, "llave:") == 0 && abrir.size() > 6)) return R::Error("Requisito de apertura desconocido");
    if ((abrir == "ganzua" || abrir == "fuerza") && d.requisitos.cd == 0)
        return R::Error("La apertura enfrentada necesita al menos un dado rival");
    return R::Ok(std::move(d));
}

std::string escribeContenedor(const ContenedorDef& d) {
    JsonWriter w;
    w.beginObject();
    w.member("tipo", d.tipo); w.member("tabla", d.tabla); w.member("presupuesto", d.presupuesto);
    w.key("garantizados"); w.beginArray(); for (const auto& id : d.garantizados) w.value(id); w.endArray();
    w.key("requisitos"); w.beginObject(); w.member("percepcion", d.requisitos.percepcion);
    w.member("abrir", d.requisitos.abrir); w.member("cd", d.requisitos.cd); w.endObject(); w.endObject();
    return w.str();
}

Result<std::vector<TablaBotin>> leeTablasBotin(const std::string& texto) {
    using R = Result<std::vector<TablaBotin>>;
    const auto raiz = JsonValue::parse(texto);
    if (!raiz.isOk()) return R::Error(raiz.errorMessage());
    const JsonValue& lista = raiz.value()["tablas"];
    if (!lista.isArray()) return R::Error("Se esperaba una lista de tablas de botín");
    std::vector<TablaBotin> tablas;
    std::set<std::string> ids;
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const JsonValue& n = lista[i];
        TablaBotin t;
        t.id = n["id"].asString();
        if (t.id.empty() || !ids.insert(t.id).second) return R::Error("Tabla de botín sin id único");
        if (!entero(n["tiradas"], 0, 64) || !n["oro"].isArray() || n["oro"].size() != 2 ||
            !entero(n["oro"][std::size_t{0}], 0, 1000000) || !entero(n["oro"][std::size_t{1}], 0, 1000000))
            return R::Error("Límites inválidos en la tabla " + t.id);
        t.tiradas = n["tiradas"].asInt();
        t.oroMin = n["oro"][std::size_t{0}].asInt(); t.oroMax = n["oro"][std::size_t{1}].asInt();
        if (t.oroMin > t.oroMax || !n["entradas"].isArray()) return R::Error("Tabla sin entradas o con oro invertido");
        std::set<std::string> objetos;
        const JsonValue& entradas = n["entradas"];
        for (std::size_t j = 0; j < entradas.size(); ++j) {
            const JsonValue& e = entradas[j];
            if (e["id"].asString().empty() || !objetos.insert(e["id"].asString()).second ||
                !entero(e["valor"], 1, 1000000) || !entero(e["peso"], 1, 1000000) ||
                (!e["unico"].isNull() && !e["unico"].isBool())) return R::Error("Entrada inválida en " + t.id);
            t.entradas.push_back({e["id"].asString(), e["valor"].asInt(), e["peso"].asInt(), e["unico"].asBool(false)});
        }
        tablas.push_back(std::move(t));
    }
    return R::Ok(std::move(tablas));
}
}  // namespace Game
