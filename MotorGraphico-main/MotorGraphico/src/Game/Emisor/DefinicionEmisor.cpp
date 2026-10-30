// U4-07.E02 — ver include/Game/Emisor/DefinicionEmisor.h.
#include "Game/Emisor/DefinicionEmisor.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "Game/Personajes/FamiliasDeParticulas.h"

namespace Game::Emisor {

namespace {

using R = Result<DefinicionEmisor>;

R fallo(const std::string& ruta, const std::string& motivo) { return R::Error(ruta + ": " + motivo); }

// Numero finito dentro de [min,max] (o (min,max] si minExcluido). Devuelve "" si vale.
std::string numeroEn(const JsonValue& v, double min, double max, bool minExcluido, double& out) {
    if (!v.isNumber()) return "debe ser un numero";
    const double x = v.asNumber();
    if (!std::isfinite(x)) return "no es finito";
    if (x < min || x > max || (minExcluido && x == min)) return "fuera de rango";
    out = x;
    return "";
}

std::string parEn(const JsonValue& v, double min, double max, bool minExcluido, double& a, double& b) {
    if (!v.isArray() || v.size() != 2) return "debe ser un array [min,max]";
    const std::string e1 = numeroEn(v[std::size_t{0}], min, max, minExcluido, a);
    if (!e1.empty()) return "[0] " + e1;
    const std::string e2 = numeroEn(v[std::size_t{1}], min, max, minExcluido, b);
    if (!e2.empty()) return "[1] " + e2;
    if (b < a) return "max menor que min";
    return "";
}

bool esEntero(double x) { return std::floor(x) == x; }

// Copia deliberada del leerColor de ParticulasDeCartas.cpp (esta en su namespace anonimo y no se puede
// reutilizar sin tocar un fichero fuera del ambito): aqui un formato malo es Error, no el color por defecto.
bool leerColor(const std::string& hex, std::array<std::uint8_t, 3>& c) {
    if (hex.size() != 6 || hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) return false;
    for (std::size_t i = 0; i < 3; ++i)
        c[i] = static_cast<std::uint8_t>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
    return true;
}

}  // namespace

Result<DefinicionEmisor> leerEmisor(const JsonValue& json) {
    const std::string raiz = "emisor";
    if (!json.isObject()) return fallo(raiz, "debe ser un objeto");

    static const std::set<std::string> permitidas = {
        "familia", "tasa", "duracion", "cortar", "vida", "velocidad", "angulo",
        "abanico", "maximo", "semilla", "opacidad", "contraste", "color"};
    for (const auto& [clave, valor] : json.objectValues()) {
        (void)valor;
        if (!permitidas.count(clave)) return fallo(raiz, "clave desconocida: " + clave);
    }

    DefinicionEmisor d;
    std::string e;
    double x = 0.0;

    if (!json.has("familia") || !json["familia"].isString()) return fallo(raiz + ".familia", "obligatoria (texto)");
    d.familia = json["familia"].asString();
    {
        const auto fams = Game::familiasDeParticulas();
        if (std::find(fams.begin(), fams.end(), d.familia) == fams.end())
            return fallo(raiz + ".familia", "familia desconocida: " + d.familia);
    }

    if (json.has("tasa")) {
        e = numeroEn(json["tasa"], 0.0, kMaxTasa, false, d.tasa);
        if (!e.empty()) return fallo(raiz + ".tasa", e + " (0.." + std::to_string(static_cast<int>(kMaxTasa)) + ")");
    }
    if (json.has("duracion")) {
        e = numeroEn(json["duracion"], 0.0, kMaxDuracion, false, d.duracion);
        if (!e.empty()) return fallo(raiz + ".duracion", e + " (0.." + std::to_string(static_cast<int>(kMaxDuracion)) + ")");
    }
    if (json.has("cortar")) {
        if (!json["cortar"].isBool()) return fallo(raiz + ".cortar", "debe ser true o false");
        d.cortar = json["cortar"].asBool();
    }

    if (!json.has("vida")) return fallo(raiz + ".vida", "obligatoria ([min,max])");
    e = parEn(json["vida"], 0.0, kMaxVida, true, d.vidaMin, d.vidaMax);
    if (!e.empty()) return fallo(raiz + ".vida", e + " ((0.." + std::to_string(static_cast<int>(kMaxVida)) + "])");

    if (json.has("velocidad")) {
        e = parEn(json["velocidad"], 0.0, kMaxVelocidad, false, d.velocidadMin, d.velocidadMax);
        if (!e.empty()) return fallo(raiz + ".velocidad", e + " (0.." + std::to_string(static_cast<int>(kMaxVelocidad)) + ")");
    }
    if (json.has("angulo")) {
        e = numeroEn(json["angulo"], -1.0e6, 1.0e6, false, d.anguloCentro);
        if (!e.empty()) return fallo(raiz + ".angulo", e);
    }
    if (json.has("abanico")) {
        e = numeroEn(json["abanico"], 0.0, 360.0, true, d.abanico);
        if (!e.empty()) return fallo(raiz + ".abanico", e + " ((0..360])");
    }

    if (!json.has("maximo")) return fallo(raiz + ".maximo", "obligatorio (entero)");
    e = numeroEn(json["maximo"], 1.0, static_cast<double>(kMaxPorEmisor), false, x);
    if (e.empty() && !esEntero(x)) e = "debe ser entero";
    if (!e.empty()) return fallo(raiz + ".maximo", e + " (1.." + std::to_string(kMaxPorEmisor) + ")");
    d.maximo = static_cast<int>(x);

    if (json.has("semilla")) {
        e = numeroEn(json["semilla"], 0.0, kMaxSemilla, false, x);
        if (e.empty() && !esEntero(x)) e = "debe ser entero";
        if (!e.empty()) return fallo(raiz + ".semilla", e + " (entero 0..2^53-1)");
        d.semilla = static_cast<std::uint64_t>(x);
    }
    if (json.has("opacidad")) {
        e = numeroEn(json["opacidad"], 0.0, 2.0, true, x);
        // Se vuelve a comprobar tras el cast: 1e-46 es > 0 como double y 0.0f como float.
        if (e.empty() && !(static_cast<float>(x) > 0.0f && static_cast<float>(x) <= 2.0f)) e = "fuera de rango";
        if (!e.empty()) return fallo(raiz + ".opacidad", e + " ((0..2])");
        d.opacidad = static_cast<float>(x);
    }
    if (json.has("contraste")) {
        e = numeroEn(json["contraste"], 0.0, 1.0, false, x);
        if (!e.empty()) return fallo(raiz + ".contraste", e + " ([0,1])");
        d.contraste = static_cast<float>(x);
    }
    if (json.has("color")) {
        if (!json["color"].isString() || !leerColor(json["color"].asString(), d.colorBase))
            return fallo(raiz + ".color", "debe ser \"rrggbb\" (6 hex)");
    }
    return R::Ok(std::move(d));
}

}  // namespace Game::Emisor
