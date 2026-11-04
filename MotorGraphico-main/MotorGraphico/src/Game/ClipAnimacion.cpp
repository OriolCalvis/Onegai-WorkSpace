#include "Game/ClipAnimacion.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <utility>

namespace Game {

bool tipoEventoValido(const std::string& tipo) {
    return tipo == "logico" || tipo == "visual" || tipo == "sonido";
}

double ClipAnimacion::duracionTotal() const {
    double total = 0.0;
    for (const FotogramaClip& f : fotogramas) {
        total += f.duracionMs;
    }
    return total;
}

namespace {

using ClipResult = Result<ClipAnimacion>;

std::string idx(const std::string& base, std::size_t i) {
    return base + "[" + std::to_string(i) + "]";
}

// Entero estricto: debe ser numero finito, sin parte decimal, dentro de int.
bool esEntero(const JsonValue& v) {
    if (!v.isNumber()) {
        return false;
    }
    const double d = v.asNumber();
    return std::isfinite(d) && d == std::floor(d) &&
           d >= static_cast<double>(std::numeric_limits<int>::min()) &&
           d <= static_cast<double>(std::numeric_limits<int>::max());
}

// "" si padre[clave] es un entero estricto; si no, el motivo (ausente / tipo / decimal / rango).
std::string motivoNoEntero(const JsonValue& padre, const char* clave) {
    if (!padre.has(clave)) {
        return "ausente";
    }
    const JsonValue& v = padre[clave];
    if (!v.isNumber()) {
        return "tipo incorrecto, no entero";
    }
    if (!esEntero(v)) {
        const double d = v.asNumber();
        return std::isfinite(d) && d != std::floor(d) ? "no entero (tiene parte decimal)"
                                                      : "no entero (fuera del rango de int)";
    }
    return "";
}

ClipResult fallo(const std::string& ruta, const std::string& motivo) {
    return ClipResult::Error(ruta + ": " + motivo);
}

ClipResult leerFotogramas(const JsonValue& clipJson, const std::string& ruta, ClipAnimacion& clip) {
    const std::string rutaF = ruta + ".fotogramas";
    const JsonValue& arr = clipJson["fotogramas"];
    if (!arr.isArray() || arr.size() == 0) {
        return fallo(rutaF, "ausente o vacio (se espera un array con al menos un fotograma)");
    }
    for (std::size_t i = 0; i < arr.size(); ++i) {
        const JsonValue& f = arr[i];
        const std::string rutaI = idx(rutaF, i);
        if (!f.isObject()) {
            return fallo(rutaI, "se espera un objeto");
        }
        if (const std::string m = motivoNoEntero(f, "sprite"); !m.empty()) {
            return fallo(rutaI + ".sprite", m);
        }
        if (f["sprite"].asInt() < 1) {
            return fallo(rutaI + ".sprite", "debe ser >= 1 (" + std::to_string(f["sprite"].asInt()) + ")");
        }
        if (const std::string m = motivoNoEntero(f, "duracionMs"); !m.empty()) {
            return fallo(rutaI + ".duracionMs", m);
        }
        if (f["duracionMs"].asInt() <= 0) {
            return fallo(rutaI + ".duracionMs",
                         "debe ser > 0 (" + std::to_string(f["duracionMs"].asInt()) + " <= 0)");
        }
        clip.fotogramas.push_back({f["sprite"].asInt(), f["duracionMs"].asInt()});
    }
    return ClipResult::Ok(clip);
}

ClipResult leerEventos(const JsonValue& clipJson, const std::string& ruta, ClipAnimacion& clip) {
    if (!clipJson.has("eventos")) {
        return ClipResult::Ok(clip);
    }
    const std::string rutaE = ruta + ".eventos";
    const JsonValue& arr = clipJson["eventos"];
    if (!arr.isArray()) {
        return fallo(rutaE, "se espera un array");
    }
    const int n = static_cast<int>(clip.fotogramas.size());
    std::set<std::pair<std::string, int>> vistos;
    for (std::size_t i = 0; i < arr.size(); ++i) {
        const JsonValue& e = arr[i];
        const std::string rutaI = idx(rutaE, i);
        if (!e.isObject()) {
            return fallo(rutaI, "se espera un objeto");
        }
        if (!e["id"].isString() || !ids::isValidValue(e["id"].asString())) {
            return fallo(rutaI + ".id", "ausente o no valido (se espera [a-z0-9_]+)");
        }
        if (const std::string m = motivoNoEntero(e, "fotograma"); !m.empty()) {
            return fallo(rutaI + ".fotograma", m);
        }
        const int f = e["fotograma"].asInt();
        if (f < 0) {
            return fallo(rutaI + ".fotograma", "negativo (" + std::to_string(f) + ")");
        }
        if (f >= n) {
            return fallo(rutaI + ".fotograma",
                         "fuera del clip (" + std::to_string(f) + " >= " + std::to_string(n) + ")");
        }
        if (e.has("tipo") && !e["tipo"].isString()) {
            return fallo(rutaI + ".tipo", "se espera una cadena");
        }
        if (e.has("tipo") && !tipoEventoValido(e["tipo"].asString())) {
            return fallo(rutaI + ".tipo", "tipo de evento no reconocido: " + e["tipo"].asString() +
                                              " (se espera logico, visual o sonido)");
        }
        if (e.has("dato") && !e["dato"].isString()) {
            return fallo(rutaI + ".dato", "se espera una cadena");
        }
        EventoFotograma ev;
        ev.id = e["id"].asString();
        ev.fotograma = f;
        ev.tipo = e.has("tipo") ? e["tipo"].asString() : "logico";
        ev.dato = e.has("dato") ? e["dato"].asString() : "";
        ev.orden = static_cast<int>(i);
        if (!vistos.insert({ev.id, f}).second) {
            return fallo(rutaI, "evento duplicado (id '" + ev.id + "', fotograma " + std::to_string(f) + ")");
        }
        clip.eventos.push_back(std::move(ev));
    }
    return ClipResult::Ok(clip);
}

ClipResult leerUno(const JsonValue& clipJson, const std::string& ruta) {
    if (!clipJson.isObject()) {
        return fallo(ruta, "se espera un objeto");
    }
    ClipAnimacion clip;
    if (!clipJson["id"].isString()) {
        return fallo(ruta + ".id", "ausente o no es cadena");
    }
    auto id = IdClip::parse(clipJson["id"].asString());
    if (!id.isOk()) {
        return fallo(ruta + ".id", "no valido (se espera [a-z0-9_]+)");
    }
    clip.id = id.value();
    if (clipJson.has("bucle")) {
        if (!clipJson["bucle"].isBool()) {
            return fallo(ruta + ".bucle", "se espera un booleano");
        }
        clip.bucle = clipJson["bucle"].asBool();
    }
    ClipResult r = leerFotogramas(clipJson, ruta, clip);
    if (!r.isOk()) {
        return r;
    }
    r = leerEventos(clipJson, ruta, clip);
    if (!r.isOk()) {
        return r;
    }
    return ClipResult::Ok(std::move(clip));
}

}  // namespace

std::vector<EventoFotograma> eventosOrdenados(const ClipAnimacion& clip) {
    std::vector<EventoFotograma> salida = clip.eventos;
    std::stable_sort(salida.begin(), salida.end(), [](const EventoFotograma& a, const EventoFotograma& b) {
        return a.fotograma != b.fotograma ? a.fotograma < b.fotograma : a.orden < b.orden;
    });
    return salida;
}

std::vector<const EventoFotograma*> eventosDeFotograma(const ClipAnimacion& clip, int fotograma) {
    std::vector<const EventoFotograma*> salida;
    for (const EventoFotograma& e : clip.eventos) {
        if (e.fotograma == fotograma) {
            salida.push_back(&e);
        }
    }
    std::stable_sort(salida.begin(), salida.end(), [](const EventoFotograma* a, const EventoFotograma* b) {
        return a->orden < b->orden;
    });
    return salida;
}

Result<std::vector<ClipAnimacion>> leerCatalogoClips(const JsonValue& raiz) {
    using R = Result<std::vector<ClipAnimacion>>;
    if (!raiz.isObject()) {
        return R::Error("raiz: se espera un objeto");
    }
    if (!esEntero(raiz["schemaVersion"]) || raiz["schemaVersion"].asInt() != 1) {
        return R::Error("schemaVersion: ausente o distinto de 1");
    }
    const JsonValue& clips = raiz["clips"];
    if (!clips.isArray() || clips.size() == 0) {
        return R::Error("clips: ausente o vacio (se espera un array con al menos un clip)");
    }
    std::vector<ClipAnimacion> salida;
    std::set<std::string> ids;
    for (std::size_t i = 0; i < clips.size(); ++i) {
        const std::string ruta = idx("clips", i);
        ClipResult c = leerUno(clips[i], ruta);
        if (!c.isOk()) {
            return R::Error(c.errorMessage());
        }
        if (!ids.insert(c.value().id.value()).second) {
            return R::Error(ruta + ".id: clip repetido ('" + c.value().id.value() + "')");
        }
        salida.push_back(std::move(c.value()));
    }
    return R::Ok(std::move(salida));
}

Result<ClipAnimacion> leerClip(const JsonValue& raiz) {
    auto todos = leerCatalogoClips(raiz);
    if (!todos.isOk()) {
        return ClipResult::Error(todos.errorMessage());
    }
    if (todos.value().size() != 1) {
        return ClipResult::Error("clips: se espera exactamente un clip (hay " +
                                 std::to_string(todos.value().size()) + ")");
    }
    return ClipResult::Ok(std::move(todos.value().front()));
}

}  // namespace Game
