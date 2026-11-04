#include "Game/MaquinaAnimacion.h"

#include "MaquinaAnimacionValidacion.h"

#include <set>
#include <utility>

namespace Game {

namespace {

using namespace lectura;
using MaqResult = Result<MaquinaAnimacion>;

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
    if (Fallo f = claveDesconocida(e, ruta, {"id", "clip", "clipsPorRumbo", "politicaRumbo"}); !f.empty()) {
        return f;
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
    if (Fallo f = claveDesconocida(t, ruta, {"desde", "hacia", "cuando", "esperaFinClip", "politica", "prioridad"});
        !f.empty()) {
        return f;
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
    if (Fallo f = claveDesconocida(m, ruta, {"id", "inicial", "parametros", "estados", "transiciones"});
        !f.empty()) {
        return f;
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

// Una sola maquina (el objeto de "maquinas[i]"): no hay raiz, asi que no comprueba schemaVersion
// (es del documento, lo valida leerCatalogoMaquinas) y sus rutas de error empiezan por "maquina".
Result<MaquinaAnimacion> leerMaquinaAnimacion(const JsonValue& maquina, const ExisteClip& existeClip) {
    return leerMaquinaRuta(maquina, "maquina", existeClip);
}

Result<std::vector<MaquinaAnimacion>> leerCatalogoMaquinas(const JsonValue& raiz,
                                                           const ExisteClip& existeClip) {
    using R = Result<std::vector<MaquinaAnimacion>>;
    if (!raiz.isObject()) {
        return R::Error("raiz: se espera un objeto");
    }
    if (Fallo f = claveDesconocida(raiz, "raiz", {"schemaVersion", "maquinas"}); !f.empty()) {
        return R::Error(f);
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
