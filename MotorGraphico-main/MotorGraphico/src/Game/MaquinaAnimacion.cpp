#include "Game/MaquinaAnimacion.h"

#include <cmath>
#include <limits>
#include <set>
#include <utility>

namespace Game {

namespace {

using MaqResult = Result<MaquinaAnimacion>;

// Cada ayudante devuelve "" si todo va bien o el mensaje de error con su ruta.
using Fallo = std::string;

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

// Campo de texto obligatorio con forma de id; deja el valor en "salida".
template <class Id>
Fallo leerId(const JsonValue& padre, const char* clave, const std::string& ruta, Id& salida) {
    const std::string rutaC = ruta + "." + clave;
    if (!padre.has(clave) || !padre[clave].isString()) {
        return fallo(rutaC, "ausente o no es cadena");
    }
    auto id = Id::parse(padre[clave].asString());
    if (!id.isOk()) {
        return fallo(rutaC, "no valido (se espera [a-z0-9_]+): " + padre[clave].asString());
    }
    salida = id.value();
    return "";
}

// Array opcional: si la clave existe debe ser un array. "presente" indica si existe.
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

// ---- parametros ----

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

// ---- estados ----

constexpr const char* kRumbos[4] = {"arriba", "derecha", "abajo", "izquierda"};

Fallo leerClipRef(const JsonValue& v, const std::string& ruta, const ExisteClip& existe, IdClip& salida) {
    if (!v.isString()) {
        return fallo(ruta, "se espera una cadena (id de clip)");
    }
    auto id = IdClip::parse(v.asString());
    if (!id.isOk()) {
        return fallo(ruta, "no valido (se espera [a-z0-9_]+): " + v.asString());
    }
    if (!existe || !existe(id.value())) {
        return fallo(ruta, "clip no existe: " + v.asString());
    }
    salida = id.value();
    return "";
}

Fallo leerClipsPorRumbo(const JsonValue& o, const std::string& ruta, const ExisteClip& existe,
                        EstadoAnimacion& estado) {
    if (!o.isObject()) {
        return fallo(ruta, "se espera un objeto con las 4 claves arriba, derecha, abajo, izquierda");
    }
    if (o.size() != 4) {
        return fallo(ruta, "se esperan exactamente 4 entradas (hay " + std::to_string(o.size()) + ")");
    }
    for (const char* rumbo : kRumbos) {
        if (!o.has(rumbo)) {
            return fallo(ruta + "." + rumbo, "ausente");
        }
        IdClip clip;
        if (Fallo f = leerClipRef(o[rumbo], ruta + "." + rumbo, existe, clip); !f.empty()) {
            return f;
        }
        estado.clipsPorRumbo.push_back(clip);
    }
    return "";
}

Fallo leerEstado(const JsonValue& e, const std::string& ruta, const ExisteClip& existe,
                 EstadoAnimacion& estado) {
    if (!e.isObject()) {
        return fallo(ruta, "se espera un objeto");
    }
    if (Fallo f = leerId(e, "id", ruta, estado.id); !f.empty()) {
        return f;
    }
    const bool conClip = e.has("clip");
    const bool conRumbo = e.has("clipsPorRumbo");
    if (conClip == conRumbo) {
        return fallo(ruta, "se espera exactamente uno de clip o clipsPorRumbo");
    }
    if (conClip) {
        if (e.has("politicaRumbo")) {
            return fallo(ruta + ".politicaRumbo", "solo aplica con clipsPorRumbo");
        }
        return leerClipRef(e["clip"], ruta + ".clip", existe, estado.clip);
    }
    if (Fallo f = leerClipsPorRumbo(e["clipsPorRumbo"], ruta + ".clipsPorRumbo", existe, estado);
        !f.empty()) {
        return f;
    }
    return leerPolitica(e, "politicaRumbo", ruta, estado.politicaRumbo);
}

Fallo leerEstados(const JsonValue& m, const std::string& ruta, const ExisteClip& existe,
                  MaquinaAnimacion& maq) {
    const JsonValue& arr = m["estados"];
    if (!arr.isArray() || arr.size() == 0) {
        return fallo(ruta + ".estados", "ausente o vacio (se espera un array con al menos un estado)");
    }
    std::set<std::string> vistos;
    for (std::size_t i = 0; i < arr.size(); ++i) {
        const std::string rutaI = idx(ruta + ".estados", i);
        EstadoAnimacion estado;
        if (Fallo f = leerEstado(arr[i], rutaI, existe, estado); !f.empty()) {
            return f;
        }
        if (!vistos.insert(estado.id.value()).second) {
            return fallo(rutaI + ".id", "estado repetido: " + estado.id.value());
        }
        maq.estados.push_back(std::move(estado));
    }
    return "";
}

bool existeEstado(const MaquinaAnimacion& maq, const IdEstado& id) {
    for (const EstadoAnimacion& e : maq.estados) {
        if (e.id == id) {
            return true;
        }
    }
    return false;
}

// ---- condiciones ----

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
    if (id.value() == kParametroFinClip) {
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

Fallo leerCondicion(const JsonValue& c, const std::string& ruta, const MaquinaAnimacion& maq,
                    CondicionAnimacion& cond) {
    if (!c.isObject()) {
        return fallo(ruta, "se espera un objeto");
    }
    if (c["param"].isString() && c["param"].asString() == "finClip") {
        cond.parametro = IdParametro::parse(kParametroFinClip).value();
    } else if (Fallo f = leerId(c, "param", ruta, cond.parametro); !f.empty()) {
        return f;
    }
    TipoParametro tipo = TipoParametro::Booleano;
    if (!tipoDe(maq, cond.parametro, tipo)) {
        return fallo(ruta + ".param", "no declarado: " + cond.parametro.value());
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
    if (esNumerico(cond.op) && !c.has("valor")) {
        return fallo(ruta + ".valor", "ausente (el operador " + texto + " lo exige)");
    }
    if (c.has("valor")) {
        if (!c["valor"].isNumber() || !std::isfinite(c["valor"].asNumber())) {
            return fallo(ruta + ".valor", "se espera un numero finito");
        }
        cond.valor = c["valor"].asNumber();
    }
    return "";
}

// ---- transiciones ----

Fallo leerCuando(const JsonValue& t, const std::string& ruta, const MaquinaAnimacion& maq,
                 TransicionAnimacion& tr) {
    if (Fallo f = arrayOpcional(t, "cuando", ruta); !f.empty()) {
        return f;
    }
    const JsonValue& arr = t["cuando"];
    for (std::size_t i = 0; i < arr.size(); ++i) {
        CondicionAnimacion cond;
        if (Fallo f = leerCondicion(arr[i], idx(ruta + ".cuando", i), maq, cond); !f.empty()) {
            return f;
        }
        tr.cuando.push_back(std::move(cond));
    }
    if (t.has("esperaFinClip")) {
        if (!t["esperaFinClip"].isBool()) {
            return fallo(ruta + ".esperaFinClip", "se espera un booleano");
        }
        tr.esperaFinClip = t["esperaFinClip"].asBool();
    }
    if (tr.cuando.empty() && !tr.esperaFinClip) {
        return fallo(ruta + ".cuando", "transicion sin condiciones ni esperaFinClip (siempre cierta)");
    }
    return "";
}

Fallo leerTransicion(const JsonValue& t, const std::string& ruta, const MaquinaAnimacion& maq,
                     TransicionAnimacion& tr) {
    if (!t.isObject()) {
        return fallo(ruta, "se espera un objeto");
    }
    if (t.has("desde")) {
        IdEstado desde;
        if (Fallo f = leerId(t, "desde", ruta, desde); !f.empty()) {
            return f;
        }
        if (!existeEstado(maq, desde)) {
            return fallo(ruta + ".desde", "estado desconocido: " + desde.value());
        }
        tr.desde = desde;
    }
    if (Fallo f = leerId(t, "hacia", ruta, tr.hacia); !f.empty()) {
        return f;
    }
    if (!existeEstado(maq, tr.hacia)) {
        return fallo(ruta + ".hacia", "estado desconocido: " + tr.hacia.value());
    }
    if (t.has("prioridad")) {
        if (!esEntero(t["prioridad"])) {
            return fallo(ruta + ".prioridad", "se espera un entero");
        }
        tr.prioridad = t["prioridad"].asInt();
    }
    if (Fallo f = leerPolitica(t, "politica", ruta, tr.politica); !f.empty()) {
        return f;
    }
    return leerCuando(t, ruta, maq, tr);
}

Fallo leerTransiciones(const JsonValue& m, const std::string& ruta, MaquinaAnimacion& maq) {
    if (Fallo f = arrayOpcional(m, "transiciones", ruta); !f.empty()) {
        return f;
    }
    const JsonValue& arr = m["transiciones"];
    for (std::size_t i = 0; i < arr.size(); ++i) {
        TransicionAnimacion tr;
        tr.orden = static_cast<int>(i);
        if (Fallo f = leerTransicion(arr[i], idx(ruta + ".transiciones", i), maq, tr); !f.empty()) {
            return f;
        }
        maq.transiciones.push_back(std::move(tr));
    }
    return "";
}

// ---- maquina ----

Fallo leerMaquinaEn(const JsonValue& m, const std::string& ruta, const ExisteClip& existe,
                    MaquinaAnimacion& maq) {
    if (!m.isObject()) {
        return fallo(ruta, "se espera un objeto");
    }
    if (Fallo f = leerId(m, "id", ruta, maq.id); !f.empty()) {
        return f;
    }
    if (Fallo f = leerParametros(m, ruta, maq); !f.empty()) {
        return f;
    }
    if (Fallo f = leerEstados(m, ruta, existe, maq); !f.empty()) {
        return f;
    }
    if (Fallo f = leerId(m, "inicial", ruta, maq.inicial); !f.empty()) {
        return f;
    }
    if (!existeEstado(maq, maq.inicial)) {
        return fallo(ruta + ".inicial", "estado desconocido: " + maq.inicial.value());
    }
    return leerTransiciones(m, ruta, maq);
}

MaqResult leerMaquinaRuta(const JsonValue& m, const std::string& ruta, const ExisteClip& existe) {
    MaquinaAnimacion maq;
    if (Fallo f = leerMaquinaEn(m, ruta, existe, maq); !f.empty()) {
        return MaqResult::Error(f);
    }
    return MaqResult::Ok(std::move(maq));
}

}  // namespace

Result<MaquinaAnimacion> leerMaquinaAnimacion(const JsonValue& maquina, const ExisteClip& existeClip) {
    return leerMaquinaRuta(maquina, "maquina", existeClip);
}

Result<std::vector<MaquinaAnimacion>> leerCatalogoMaquinas(const JsonValue& raiz,
                                                           const ExisteClip& existeClip) {
    using R = Result<std::vector<MaquinaAnimacion>>;
    if (!raiz.isObject()) {
        return R::Error("raiz: se espera un objeto");
    }
    if (!esEntero(raiz["schemaVersion"]) || raiz["schemaVersion"].asInt() != 1) {
        return R::Error("schemaVersion: ausente o distinto de 1");
    }
    const JsonValue& maquinas = raiz["maquinas"];
    if (!maquinas.isArray() || maquinas.size() == 0) {
        return R::Error("maquinas: ausente o vacio (se espera un array con al menos una maquina)");
    }
    std::vector<MaquinaAnimacion> salida;
    std::set<std::string> ids;
    for (std::size_t i = 0; i < maquinas.size(); ++i) {
        const std::string ruta = idx("maquinas", i);
        MaqResult r = leerMaquinaRuta(maquinas[i], ruta, existeClip);
        if (!r.isOk()) {
            return R::Error(r.errorMessage());
        }
        if (!ids.insert(r.value().id.value()).second) {
            return R::Error(ruta + ".id: maquina repetida ('" + r.value().id.value() + "')");
        }
        salida.push_back(std::move(r.value()));
    }
    return R::Ok(std::move(salida));
}

}  // namespace Game
