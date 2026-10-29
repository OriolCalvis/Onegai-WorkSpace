// ACOMPANANTES-01 — ver AcompanantesAventura.h.
#include "AcompanantesAventura.h"

#include <algorithm>

#include "BeatsAventura.h"  // ROSITA-01: el "requires" de un refuerzo, como el de un beat

#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"

namespace RPG {
namespace acompanantes {

namespace {

constexpr const char* kStats[4] = {"CON", "DES", "INT", "CAR"};

Result<std::vector<AcompananteDef>> error(std::size_t i, const std::string& id, const std::string& que) {
    return Result<std::vector<AcompananteDef>>::Error("acompanantes[" + std::to_string(i) + "]" +
                                                      (id.empty() ? std::string() : " (\"" + id + "\")") + ": " + que);
}

}  // namespace

Result<std::vector<AcompananteDef>> lee(const JsonValue& lista) {
    std::vector<AcompananteDef> out;
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const JsonValue& e = lista[i];
        AcompananteDef a;
        a.id = e["id"].asString();
        if (a.id.empty()) return error(i, "", "no tiene \"id\"");
        for (const AcompananteDef& otro : out)
            if (otro.id == a.id) return error(i, a.id, "el id esta repetido");
        a.nombre = e["nombre"].asString(a.id);
        a.vida = e["vida"].asInt(0);
        if (a.vida < 1) return error(i, a.id, "\"vida\" tiene que ser 1 o mas");
        const JsonValue& stats = e["stats"];
        if (stats.isObject()) {
            for (const auto& [clave, valor] : stats.objectValues()) {
                const auto* fin = std::end(kStats);
                const auto* s = std::find_if(std::begin(kStats), fin, [&](const char* k) { return clave == k; });
                if (s == fin) return error(i, a.id, "stat desconocido \"" + clave + "\" (validos: CON, DES, INT, CAR)");
                const int n = valor.asInt(-1);
                if (n < 0 || n > 8) return error(i, a.id, "el stat " + clave + " tiene que ir de 0 a 8");
                a.stats[static_cast<std::size_t>(s - std::begin(kStats))] = n;
            }
        }
        a.ataques = e["ataques"].asInt(1);
        if (a.ataques < 0 || a.ataques > 4) return error(i, a.id, "\"ataques\" tiene que ir de 0 a 4");
        a.atacable = e["atacable"].asBool(true);
        const std::string alCaer = e["alCaer"].asString("cae");
        if (alCaer != "cae" && alCaer != "se_retira")
            return error(i, a.id, "\"alCaer\" desconocido \"" + alCaer + "\" (validos: cae, se_retira)");
        a.seRetira = alCaer == "se_retira";
        a.soloIA = e["ia"].asBool(false);  // AV13-29
        if (e.has("cura")) {
            const JsonValue& c = e["cura"];
            a.cura.dado = c["dado"].asInt(0);
            a.cura.mas = c["mas"].asInt(0);
            a.cura.veces = c["veces"].asInt(0);
            a.cura.umbral = static_cast<float>(c["umbral"].asNumber(0.34));
            if (a.cura.dado < 1 || a.cura.veces < 1 || a.cura.mas < 0 || a.cura.umbral <= 0.0f || a.cura.umbral > 1.0f)
                return error(i, a.id, "\"cura\" necesita dado y veces de 1 o mas, mas de 0 o mas y umbral en (0, 1]");
        }
        out.push_back(std::move(a));
    }
    return Result<std::vector<AcompananteDef>>::Ok(std::move(out));
}

Result<std::vector<BattleRequest::ConAcompanante>> leeDeCombate(const JsonValue& lista,
                                                                const std::vector<AcompananteDef>& definidos,
                                                                const std::string& donde) {
    using R = Result<std::vector<BattleRequest::ConAcompanante>>;
    std::vector<BattleRequest::ConAcompanante> out;
    for (std::size_t k = 0; k < lista.size(); ++k) {
        BattleRequest::ConAcompanante con;
        con.id = lista[k]["id"].asString();
        con.siFlag = lista[k]["siFlag"].asString();
        if (con.id.empty()) return R::Error(donde + ": cada acompañante necesita id");
        const bool definido = std::any_of(definidos.begin(), definidos.end(),
                                          [&](const AcompananteDef& a) { return a.id == con.id; });
        if (!definido) return R::Error(donde + ": el acompañante \"" + con.id + "\" no esta en \"acompanantes\"");
        for (const auto& otro : out)
            if (otro.id == con.id) return R::Error(donde + ": el acompañante \"" + con.id + "\" esta repetido");
        out.push_back(std::move(con));
    }
    if (out.size() > BattleRequest::kMaxAcompanantes)
        return R::Error(donde + ": como mucho " + std::to_string(BattleRequest::kMaxAcompanantes) +
                        " acompañantes por combate");
    return R::Ok(std::move(out));
}

void escribe(JsonWriter& w, const std::vector<AcompananteDef>& lista) {
    if (lista.empty()) return;
    w.key("acompanantes");
    w.beginArray();
    for (const AcompananteDef& a : lista) {
        w.beginObject();
        w.member("id", a.id);
        w.member("nombre", a.nombre);
        w.member("vida", a.vida);
        w.key("stats");
        w.beginObject();
        for (std::size_t s = 0; s < 4; ++s) w.member(kStats[s], a.stats[s]);
        w.endObject();
        w.member("ataques", a.ataques);
        w.member("atacable", a.atacable);
        w.member("alCaer", a.seRetira ? "se_retira" : "cae");
        if (a.soloIA) w.member("ia", true);
        if (a.cura.veces > 0) {
            w.key("cura");
            w.beginObject();
            w.member("dado", a.cura.dado);
            w.member("mas", a.cura.mas);
            w.member("veces", a.cura.veces);
            w.member("umbral", a.cura.umbral);
            w.endObject();
        }
        w.endObject();
    }
    w.endArray();
}

namespace {

void escribeConAcompanantes(JsonWriter& w, const std::vector<BattleRequest::ConAcompanante>& lista) {
    if (lista.empty()) return;
    w.key("acompanantes");
    w.beginArray();
    for (const BattleRequest::ConAcompanante& con : lista) {
        w.beginObject();
        w.member("id", con.id);
        if (!con.siFlag.empty()) w.member("siFlag", con.siFlag);
        w.endObject();
    }
    w.endArray();
}

void escribeTextos(JsonWriter& w, const char* clave, const std::vector<std::string>& lista) {
    if (lista.empty()) return;
    w.key(clave);
    w.beginArray();
    for (const std::string& s : lista) w.value(s);
    w.endArray();
}

}  // namespace

Result<std::vector<BattleRequest::Refuerzo>> leeRefuerzos(const JsonValue& lista,
                                                          const std::vector<AcompananteDef>& definidos,
                                                          const std::string& donde) {
    using R = Result<std::vector<BattleRequest::Refuerzo>>;
    std::vector<BattleRequest::Refuerzo> out;
    for (std::size_t k = 0; k < lista.size(); ++k) {
        const JsonValue& e = lista[k];
        const std::string aqui = donde + ", refuerzos[" + std::to_string(k) + "]";
        BattleRequest::Refuerzo r;
        auto requiere = beats::leeCondicion(e["requires"], aqui);
        if (!requiere.isOk()) return R::Error(requiere.errorMessage());
        r.requiere = std::move(requiere.value());
        r.vidaEnemigo = static_cast<float>(e["cuando"]["vidaEnemigo"].asNumber(0.0));
        r.vidaHeroe = static_cast<float>(e["cuando"]["vidaHeroe"].asNumber(0.0));
        r.golpeMortal = e["cuando"]["golpeMortal"].asBool(false);  // AV13-34
        if (r.golpeMortal && r.vidaEnemigo <= 0.0f) return R::Error(aqui + ": \"golpeMortal\" necesita vidaEnemigo");
        const auto fuera = [](float v) { return v < 0.0f || v > 1.0f; };
        if (fuera(r.vidaEnemigo) || fuera(r.vidaHeroe) || (r.vidaEnemigo <= 0.0f && r.vidaHeroe <= 0.0f))
            return R::Error(aqui + ": \"cuando\" necesita vidaEnemigo o vidaHeroe en (0, 1]");
        r.flags = beats::cadenas(e["flags"]);
        r.lineas = beats::cadenas(e["lineas"]);
        auto aLado = leeDeCombate(e["acompanantes"], definidos, aqui);
        if (!aLado.isOk()) return R::Error(aLado.errorMessage());
        r.acompanantes = std::move(aLado.value());
        if (r.acompanantes.empty() && r.flags.empty())
            return R::Error(aqui + ": un refuerzo trae acompanantes o enciende flags");
        out.push_back(std::move(r));
    }
    return R::Ok(std::move(out));
}

void escribeDeCombate(JsonWriter& w, const BattleRequest& combate) {
    escribeConAcompanantes(w, combate.acompanantes);
    if (combate.refuerzos.empty()) return;
    w.key("refuerzos");
    w.beginArray();
    for (const BattleRequest::Refuerzo& r : combate.refuerzos) {
        w.beginObject();
        beats::escribeCondicion(w, r.requiere);  // solo si tiene algo
        w.key("cuando");
        w.beginObject();
        if (r.vidaEnemigo > 0.0f) w.member("vidaEnemigo", r.vidaEnemigo);
        if (r.vidaHeroe > 0.0f) w.member("vidaHeroe", r.vidaHeroe);
        if (r.golpeMortal) w.member("golpeMortal", true);
        w.endObject();
        escribeTextos(w, "flags", r.flags);
        escribeTextos(w, "lineas", r.lineas);
        escribeConAcompanantes(w, r.acompanantes);
        w.endObject();
    }
    w.endArray();
}

}  // namespace acompanantes
}  // namespace RPG
