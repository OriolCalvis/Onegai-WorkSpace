// BND-04 — Ver ReputacionAventura.h.
#include "ReputacionAventura.h"

#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <sstream>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

namespace RPG {
namespace reputacionAventura {

namespace {

std::optional<std::set<std::string>>& conocidas() {
    static std::optional<std::set<std::string>> c;
    return c;
}

constexpr const char* kFichero = "assets/rules/facciones.json";

// Las facciones del fichero. Si no se encuentra o no se lee, NO se guarda nada
// (la proxima carga lo vuelve a intentar, quiza desde otro directorio) y el
// error dice el camino que se miro.
Result<std::set<std::string>> facciones() {
    auto& c = conocidas();
    if (c) return Result<std::set<std::string>>::Ok(*c);
    std::error_code ec;
    const std::string camino = std::filesystem::absolute(kFichero, ec).string();
    std::ifstream f(kFichero, std::ios::binary);
    if (!f)
        return Result<std::set<std::string>>::Error("no se encuentra " + camino +
                                                    " (se busca desde el directorio de trabajo)");
    std::ostringstream texto;
    texto << f.rdbuf();
    auto raiz = JsonValue::parse(texto.str());
    if (!raiz.isOk())
        return Result<std::set<std::string>>::Error(camino + " no es JSON valido: " + raiz.errorMessage());
    std::set<std::string> ids;
    const JsonValue& lista = raiz.value()["facciones"];
    for (std::size_t i = 0; lista.isArray() && i < lista.size(); ++i) {
        const std::string id = lista[i]["id"].asString("");
        if (!id.empty()) ids.insert(id);
    }
    if (ids.empty()) return Result<std::set<std::string>>::Error(camino + " no trae ninguna faccion");
    c = ids;
    return Result<std::set<std::string>>::Ok(std::move(ids));
}

// Entero exacto, como el validador de Python: ni decimales ni texto ni bool.
bool esEntero(const JsonValue& v) {
    if (!v.isNumber()) return false;
    const double d = v.asNumber();
    return d == static_cast<double>(static_cast<long long>(d)) && d >= -1e9 && d <= 1e9;
}

}  // namespace

void ponFaccionesConocidas(std::set<std::string> ids) {
    if (ids.empty()) conocidas().reset();
    else conocidas() = std::move(ids);
}

Result<bool> leeCondicion(const JsonValue& requiresValue, NarrativeCondition& c, const std::string& donde) {
    const JsonValue& rep = requiresValue["reputacion"];
    if (rep.isNull()) return Result<bool>::Ok(true);
    if (!rep.isObject() || rep.objectValues().empty())
        return Result<bool>::Error(donde + ": requires.reputacion es {faccion: {min, max}}");
    auto conocidasOError = facciones();
    if (!conocidasOError.isOk()) return Result<bool>::Error(donde + ": requires.reputacion: " + conocidasOError.errorMessage());
    const std::set<std::string>& validas = conocidasOError.value();
    for (const auto& [faccion, rango] : rep.objectValues()) {
        const std::string en = donde + ": requires.reputacion[\"" + faccion + "\"]";
        if (validas.count(faccion) == 0)
            return Result<bool>::Error(en + ": no es una faccion de assets/rules/facciones.json");
        if (!rango.isObject() || (!rango.has("min") && !rango.has("max")))
            return Result<bool>::Error(en + ": es {min, max} (de -100 a 100), alguno de los dos");
        for (const auto& [clave, valor] : rango.objectValues()) {
            if (clave != "min" && clave != "max")
                return Result<bool>::Error(en + ": la clave \"" + clave + "\" no existe (solo min y max)");
            if (!esEntero(valor)) return Result<bool>::Error(en + ": " + clave + " es un numero entero");
        }
        NarrativeCondition::RangoDeReputacion r;
        r.faccion = faccion;
        if (rango.has("min")) r.min = rango["min"].asInt(-101);
        if (rango.has("max")) r.max = rango["max"].asInt(101);
        const bool minBien = !rango.has("min") || (r.min >= -100 && r.min <= 100);
        const bool maxBien = !rango.has("max") || (r.max >= -100 && r.max <= 100);
        if (!minBien || !maxBien || r.min > r.max)
            return Result<bool>::Error(en + ": min y max de -100 a 100, con min <= max");
        c.reputaciones.push_back(std::move(r));
    }
    return Result<bool>::Ok(true);
}

void escribeCondicion(JsonWriter& w, const NarrativeCondition& c) {
    if (c.reputaciones.empty()) return;
    w.key("reputacion");
    w.beginObject();
    for (const NarrativeCondition::RangoDeReputacion& r : c.reputaciones) {
        w.key(r.faccion);
        w.beginObject();
        if (r.min != std::numeric_limits<int>::min()) w.member("min", r.min);
        if (r.max != std::numeric_limits<int>::max()) w.member("max", r.max);
        w.endObject();
    }
    w.endObject();
}

}  // namespace reputacionAventura
}  // namespace RPG
