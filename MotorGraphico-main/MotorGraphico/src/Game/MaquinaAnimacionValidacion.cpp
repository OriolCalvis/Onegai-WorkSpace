#include "MaquinaAnimacionValidacion.h"

#include <cmath>
#include <limits>
#include <set>
#include <utility>

namespace Game::lectura {

std::string idx(const std::string& base, std::size_t i) {
    return base + "[" + std::to_string(i) + "]";
}

Fallo fallo(const std::string& ruta, const std::string& motivo) {
    return ruta + ": " + motivo;
}

bool esEntero(const JsonValue& v) {
    if (!v.isNumber()) {
        return false;
    }
    const double d = v.asNumber();
    return std::isfinite(d) && d == std::floor(d) &&
           d >= static_cast<double>(std::numeric_limits<int>::min()) &&
           d <= static_cast<double>(std::numeric_limits<int>::max());
}

Fallo claveDesconocida(const JsonValue& objeto, const std::string& ruta,
                       std::initializer_list<const char*> permitidas) {
    for (const auto& par : objeto.objectValues()) {
        bool conocida = false;
        for (const char* p : permitidas) {
            conocida = conocida || par.first == p;
        }
        if (!conocida) {
            return fallo(ruta, "clave desconocida: " + par.first);
        }
    }
    return "";
}

Fallo arrayOpcional(const JsonValue& padre, const char* clave, const std::string& ruta) {
    if (padre.has(clave) && !padre[clave].isArray()) {
        return fallo(ruta + "." + clave, "se espera un array");
    }
    return "";
}

Fallo leerPolitica(const JsonValue& padre, const char* clave, const std::string& ruta,
                   PoliticaCambio& salida) {
    if (!padre.has(clave)) {
        return "";
    }
    const std::string rutaC = ruta + "." + clave;
    if (!padre[clave].isString()) {
        return fallo(rutaC, "se espera una cadena");
    }
    const std::string s = padre[clave].asString();
    if (s == "reiniciarSiempre") {
        salida = PoliticaCambio::ReiniciarSiempre;
    } else if (s == "reiniciarSiDistinto") {
        salida = PoliticaCambio::ReiniciarSiDistinto;
    } else if (s == "conservarFase") {
        salida = PoliticaCambio::ConservarFase;
    } else {
        return fallo(rutaC, "politica desconocida: " + s);
    }
    return "";
}

const IdParametro& idParametroFinClip() {
    static const IdParametro id = [] {
        auto r = IdParametro::parse(kParametroFinClip);
        return r.isOk() ? r.value() : IdParametro{};
    }();
    return id;
}

namespace {

// "finClip" (nombre en el JSON) no cabe en un IdParametro ([a-z0-9_]+): internamente es kParametroFinClip.
bool esReservado(const std::string& nombre) {
    return nombre == "rumbo" || nombre == "finClip" || nombre == kParametroFinClip;
}

Fallo leerTipoParametro(const JsonValue& p, const std::string& ruta, TipoParametro& tipo) {
    const std::string rutaT = ruta + ".tipo";
    if (!p.has("tipo") || !p["tipo"].isString()) {
        return fallo(rutaT, "ausente o no es cadena");
    }
    const std::string s = p["tipo"].asString();
    if (s == "booleano") {
        tipo = TipoParametro::Booleano;
    } else if (s == "entero") {
        tipo = TipoParametro::Entero;
    } else if (s == "real") {
        tipo = TipoParametro::Real;
    } else if (s == "disparo") {
        tipo = TipoParametro::Disparo;
    } else {
        return fallo(rutaT, "tipo desconocido: " + s);
    }
    return "";
}

const ParametroAnimacion* buscaDeclarado(const MaquinaAnimacion& maq, const IdParametro& id) {
    for (const ParametroAnimacion& p : maq.parametros) {
        if (p.id == id) {
            return &p;
        }
    }
    return nullptr;
}

// Tipo de un parametro declarado o reservado; false si no existe.
bool tipoDe(const MaquinaAnimacion& maq, const IdParametro& id, TipoParametro& tipo) {
    if (id.value() == "rumbo") {
        tipo = TipoParametro::Entero;
        return true;
    }
    if (id == idParametroFinClip()) {
        tipo = TipoParametro::Booleano;
        return true;
    }
    if (const ParametroAnimacion* p = buscaDeclarado(maq, id)) {
        tipo = p->tipo;
        return true;
    }
    return false;
}

bool operadorDeTexto(const std::string& s, OperadorCondicion& op) {
    static const std::pair<const char*, OperadorCondicion> tabla[] = {
        {"cierto", OperadorCondicion::Cierto},         {"falso", OperadorCondicion::Falso},
        {"igual", OperadorCondicion::Igual},           {"distinto", OperadorCondicion::Distinto},
        {"mayor", OperadorCondicion::Mayor},           {"mayorIgual", OperadorCondicion::MayorIgual},
        {"menor", OperadorCondicion::Menor},           {"menorIgual", OperadorCondicion::MenorIgual},
        {"disparado", OperadorCondicion::Disparado}};
    for (const auto& par : tabla) {
        if (s == par.first) {
            op = par.second;
            return true;
        }
    }
    return false;
}

bool esNumerico(OperadorCondicion op) {
    return op != OperadorCondicion::Cierto && op != OperadorCondicion::Falso &&
           op != OperadorCondicion::Disparado;
}

bool compatible(OperadorCondicion op, TipoParametro tipo) {
    switch (tipo) {
        case TipoParametro::Booleano:
            return op == OperadorCondicion::Cierto || op == OperadorCondicion::Falso;
        case TipoParametro::Disparo:
            return op == OperadorCondicion::Disparado;
        case TipoParametro::Entero:
        case TipoParametro::Real:
            return esNumerico(op);
    }
    return false;
}

const char* nombreTipo(TipoParametro tipo) {
    switch (tipo) {
        case TipoParametro::Booleano:
            return "booleano";
        case TipoParametro::Entero:
            return "entero";
        case TipoParametro::Real:
            return "real";
        case TipoParametro::Disparo:
            return "disparo";
    }
    return "?";
}

// Resuelve "param": solo "finClip" exacto (nunca el alias interno "finclip") o un id declarado.
Fallo leerParam(const JsonValue& c, const std::string& ruta, const MaquinaAnimacion& maq,
                CondicionAnimacion& cond, TipoParametro& tipo) {
    if (c["param"].isString() && c["param"].asString() == "finClip") {
        cond.parametro = idParametroFinClip();
    } else if (Fallo f = leerId(c, "param", ruta, cond.parametro); !f.empty()) {
        return f;
    }
    if (cond.parametro.value() == kParametroFinClip && c["param"].asString() != "finClip") {
        return fallo(ruta + ".param", "no declarado: " + cond.parametro.value());
    }
    if (!tipoDe(maq, cond.parametro, tipo)) {
        return fallo(ruta + ".param", "no declarado: " + cond.parametro.value());
    }
    return "";
}

Fallo leerValor(const JsonValue& c, const std::string& ruta, const std::string& op, TipoParametro tipo,
                CondicionAnimacion& cond) {
    const std::string rutaV = ruta + ".valor";
    if (!esNumerico(cond.op)) {
        return c.has("valor") ? fallo(rutaV, "no aplica al operador " + op) : "";
    }
    if (!c.has("valor")) {
        return fallo(rutaV, "ausente (el operador " + op + " lo exige)");
    }
    if (!c["valor"].isNumber() || !std::isfinite(c["valor"].asNumber())) {
        return fallo(rutaV, "se espera un numero finito");
    }
    cond.valor = c["valor"].asNumber();
    if (tipo == TipoParametro::Entero && !esEntero(c["valor"])) {
        return fallo(rutaV, "se espera un entero para un parametro entero");
    }
    if (cond.parametro.value() == "rumbo" && (cond.valor < 0.0 || cond.valor > 3.0)) {
        return fallo(rutaV, "rumbo fuera de 0..3");
    }
    return "";
}

}  // namespace

Fallo leerParametros(const JsonValue& m, const std::string& ruta, MaquinaAnimacion& maq) {
    if (Fallo f = arrayOpcional(m, "parametros", ruta); !f.empty()) {
        return f;
    }
    const JsonValue& arr = m["parametros"];
    std::set<std::string> vistos;
    for (std::size_t i = 0; i < arr.size(); ++i) {
        const std::string rutaI = idx(ruta + ".parametros", i);
        if (!arr[i].isObject()) {
            return fallo(rutaI, "se espera un objeto");
        }
        if (Fallo f = claveDesconocida(arr[i], rutaI, {"id", "tipo"}); !f.empty()) {
            return f;
        }
        ParametroAnimacion p;
        if (arr[i]["id"].isString() && esReservado(arr[i]["id"].asString())) {
            return fallo(rutaI + ".id", "nombre reservado: " + arr[i]["id"].asString());
        }
        if (Fallo f = leerId(arr[i], "id", rutaI, p.id); !f.empty()) {
            return f;
        }
        if (!vistos.insert(p.id.value()).second) {
            return fallo(rutaI + ".id", "parametro repetido: " + p.id.value());
        }
        if (Fallo f = leerTipoParametro(arr[i], rutaI, p.tipo); !f.empty()) {
            return f;
        }
        maq.parametros.push_back(std::move(p));
    }
    return "";
}

Fallo leerCondicion(const JsonValue& c, const std::string& ruta, const MaquinaAnimacion& maq,
                    CondicionAnimacion& cond) {
    if (!c.isObject()) {
        return fallo(ruta, "se espera un objeto");
    }
    if (Fallo f = claveDesconocida(c, ruta, {"param", "op", "valor"}); !f.empty()) {
        return f;
    }
    TipoParametro tipo = TipoParametro::Booleano;
    if (Fallo f = leerParam(c, ruta, maq, cond, tipo); !f.empty()) {
        return f;
    }
    if (!c.has("op") || !c["op"].isString()) {
        return fallo(ruta + ".op", "ausente o no es cadena");
    }
    const std::string texto = c["op"].asString();
    if (!operadorDeTexto(texto, cond.op)) {
        return fallo(ruta + ".op", "desconocido: " + texto);
    }
    if (!compatible(cond.op, tipo)) {
        return fallo(ruta + ".op", texto + " no aplica a parametro " + nombreTipo(tipo));
    }
    return leerValor(c, ruta, texto, tipo, cond);
}

}  // namespace Game::lectura
