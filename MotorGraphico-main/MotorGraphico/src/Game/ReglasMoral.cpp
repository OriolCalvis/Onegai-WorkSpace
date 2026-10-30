// MORAL-01 — Ver include/Game/ReglasMoral.h.
#include "Game/ReglasMoral.h"

#include <cstdio>
#include <set>
#include <utility>

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

namespace moral {

namespace {

bool gravedadDe(const std::string& texto, Gravedad& out) {
    if (texto == "menor") out = Gravedad::Menor;
    else if (texto == "medio") out = Gravedad::Medio;
    else if (texto == "mayor") out = Gravedad::Mayor;
    else return false;
    return true;
}

Result<bool> leeParametros(const JsonValue& raiz, Parametros& p) {
    const JsonValue& peso = raiz["peso"];
    const char* nombres[3] = {"menor", "medio", "mayor"};
    for (int g = 0; g < 3; ++g) {
        if (!peso.has(nombres[g])) continue;
        p.peso[g] = peso[nombres[g]].asInt(-1);
        if (p.peso[g] < 0 || p.peso[g] > 100)
            return Result<bool>::Error(std::string("moral.json: peso.") + nombres[g] + " va de 0 a 100");
    }
    if (raiz.has("repeticion")) {
        const JsonValue& rep = raiz["repeticion"];
        p.repeticion.clear();
        for (std::size_t i = 0; i < rep.size(); ++i) {
            const double v = rep[i].asNumber(-1.0);
            if (v < 0.0 || v > 1.0) return Result<bool>::Error("moral.json: cada repeticion va de 0 a 1");
            p.repeticion.push_back(v);
        }
        if (p.repeticion.empty()) return Result<bool>::Error("moral.json: repeticion necesita al menos un valor");
    }
    if (raiz.has("topePorNoche")) p.tope = raiz["topePorNoche"].asInt(-1);
    if (raiz.has("resistencia")) p.resistencia = raiz["resistencia"].asInt(0);
    if (p.tope < 0 || p.tope > 100) return Result<bool>::Error("moral.json: topePorNoche va de 0 a 100");
    if (p.resistencia < 1 || p.resistencia > 1000) return Result<bool>::Error("moral.json: resistencia va de 1 a 1000");
    return Result<bool>::Ok(true);
}

Result<DefinicionActo> leeActo(const JsonValue& a, const std::string& donde) {
    DefinicionActo d;
    d.id = a["id"].asString();
    if (d.id.empty()) return Result<DefinicionActo>::Error(donde + ": sin id");
    const std::string en = "actos.json, " + d.id;
    d.tipo = a["tipo"].asString("");
    if (!gravedadDe(a["gravedad"].asString("menor"), d.gravedad))
        return Result<DefinicionActo>::Error(en + ": gravedad es menor, medio o mayor");
    d.signo = a["signo"].asInt(0);
    if (d.signo < -1 || d.signo > 1) return Result<DefinicionActo>::Error(en + ": signo es 1, -1 o 0");
    static const std::set<std::string> kContra{"", "inocente", "culpable", "combatiente", "rendido"};
    const std::string contra = a["contra"].asString("");
    if (kContra.count(contra) == 0)
        return Result<DefinicionActo>::Error(en + ": contra es inocente, culpable, combatiente o rendido");
    d.contraInocente = contra == "inocente";
    d.pureza = a["pureza"].asInt(0);
    if (d.pureza < -100 || d.pureza > 100) return Result<DefinicionActo>::Error(en + ": pureza va de -100 a 100");
    for (const auto& [faccion, valor] : a["reputacion"].objectValues()) {
        const int v = valor.asInt(0);
        if (faccion.empty() || v < -100 || v > 100)
            return Result<DefinicionActo>::Error(en + ": cada reputacion es {faccion: -100..100}");
        d.reputacion[faccion] = v;
    }
    d.motivo = a["motivo"].asString("");
    if (d.motivo.empty()) return Result<DefinicionActo>::Error(en + ": sin motivo (es lo que lee el jugador)");
    return Result<DefinicionActo>::Ok(std::move(d));
}

std::string leeFichero(const std::string& ruta) {
    std::FILE* f = std::fopen(ruta.c_str(), "rb");
    if (f == nullptr) return {};
    std::string texto;
    char trozo[4096];
    std::size_t n = 0;
    while ((n = std::fread(trozo, 1, sizeof(trozo), f)) > 0) texto.append(trozo, n);
    std::fclose(f);
    return texto;
}

std::shared_ptr<const Reglas>& actuales() {
    static std::shared_ptr<const Reglas> r;
    return r;
}

}  // namespace

ActoApuntado DefinicionActo::apuntado() const {
    ActoApuntado a;
    a.id = id;
    a.tipo = tipo;
    a.gravedad = gravedad;
    a.signo = signo;
    a.contraInocente = contraInocente;
    a.motivo = motivo;
    return a;
}

const DefinicionActo* Reglas::acto(const std::string& id) const {
    const auto it = actos.find(id);
    return it != actos.end() ? &it->second : nullptr;
}

Result<Reglas> Reglas::lee(const std::string& textoMoral, const std::string& textoActos) {
    Reglas r;
    if (!textoMoral.empty()) {
        auto raiz = JsonValue::parse(textoMoral);
        if (!raiz.isOk()) return Result<Reglas>::Error("moral.json: " + raiz.errorMessage());
        auto p = leeParametros(raiz.value(), r.parametros);
        if (!p.isOk()) return Result<Reglas>::Error(p.errorMessage());
    }
    if (!textoActos.empty()) {
        auto raiz = JsonValue::parse(textoActos);
        if (!raiz.isOk()) return Result<Reglas>::Error("actos.json: " + raiz.errorMessage());
        const JsonValue& lista = raiz.value()["actos"];
        for (std::size_t i = 0; i < lista.size(); ++i) {
            auto acto = leeActo(lista[i], "actos.json, actos[" + std::to_string(i) + "]");
            if (!acto.isOk()) return Result<Reglas>::Error(acto.errorMessage());
            const std::string id = acto.value().id;
            if (!r.actos.emplace(id, std::move(acto.value())).second)
                return Result<Reglas>::Error("actos.json: el id «" + id + "» está repetido");
        }
    }
    return Result<Reglas>::Ok(std::move(r));
}

const Reglas& reglas() {
    std::shared_ptr<const Reglas>& r = actuales();
    if (!r) {
        auto leidas = Reglas::lee(leeFichero("assets/rules/moral.json"), leeFichero("assets/rules/actos.json"));
        r = std::make_shared<const Reglas>(leidas.isOk() ? std::move(leidas.value()) : Reglas{});
    }
    return *r;
}

void ponReglas(std::shared_ptr<const Reglas> r) { actuales() = std::move(r); }

// ---------------------------------------------------------------------
// El guardado
// ---------------------------------------------------------------------

void escribeEstado(JsonWriter& w, const Estado& e) {
    w.beginObject();
    w.member("pureza", e.pureza);
    w.key("pendientes");
    w.beginArray();
    for (const ActoApuntado& a : e.pendientes) {
        w.beginObject();
        w.member("id", a.id);
        if (!a.tipo.empty()) w.member("tipo", a.tipo);
        w.member("gravedad", static_cast<int>(a.gravedad));
        w.member("signo", a.signo);
        if (a.contraInocente) w.member("inocente", true);
        w.member("motivo", a.motivo);
        w.endObject();
    }
    w.endArray();
    w.key("marcas");
    w.beginArray();
    for (const Marca& m : e.marcas) {
        w.beginObject();
        w.member("acto", m.acto);
        w.member("motivo", m.motivo);
        w.member("dia", m.dia);
        w.endObject();
    }
    w.endArray();
    w.endObject();
}

Estado leeEstado(const JsonValue& v) {
    Estado e;
    if (!v.isObject()) return e;
    e.pureza = v["pureza"].asInt(100);
    const JsonValue& pendientes = v["pendientes"];
    for (std::size_t i = 0; i < pendientes.size(); ++i) {
        const JsonValue& p = pendientes[i];
        ActoApuntado a;
        a.id = p["id"].asString();
        a.tipo = p["tipo"].asString("");
        const int g = p["gravedad"].asInt(0);
        a.gravedad = g >= 2 ? Gravedad::Mayor : g == 1 ? Gravedad::Medio : Gravedad::Menor;
        a.signo = p["signo"].asInt(0) < 0 ? -1 : 1;
        a.contraInocente = p["inocente"].asBool(false);
        a.motivo = p["motivo"].asString("");
        if (!a.id.empty()) e.pendientes.push_back(std::move(a));
    }
    const JsonValue& marcas = v["marcas"];
    for (std::size_t i = 0; i < marcas.size(); ++i) {
        Marca m;
        m.acto = marcas[i]["acto"].asString();
        m.motivo = marcas[i]["motivo"].asString("");
        m.dia = marcas[i]["dia"].asInt(0);
        if (!m.acto.empty()) e.marcas.push_back(std::move(m));
    }
    return e;
}

}  // namespace moral
