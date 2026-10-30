// MASC-01 — ver include/Game/FullaMascotes.h.
#include "Game/FullaMascotes.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Game {

namespace {

AnimMascota llegeixAnim(const JsonValue& a) {
    AnimMascota r;
    r.fps = static_cast<float>(a["fps"].asNumber(8.0));
    r.aguanta = a["aguanta"].asBool(false);
    const JsonValue& dirs = a["dirs"];
    for (std::size_t d = 0; dirs.isArray() && d < dirs.size() && d < 4; ++d)
        if (dirs[d].isArray() && dirs[d].size() >= 2) r.dirs[d] = {dirs[d][0].asInt(0), dirs[d][1].asInt(0)};
    return r;
}

}  // namespace

Result<bool> FullaMascotes::carga(const std::string& rutaManifest) {
    m_especies.clear();
    m_efectes.clear();
    std::ifstream entrada(rutaManifest, std::ios::binary);
    if (!entrada) return Result<bool>::Ok(false);
    std::stringstream text;
    text << entrada.rdbuf();
    auto llegit = JsonValue::parse(text.str());
    if (!llegit.isOk()) return Result<bool>::Error("FullaMascotes: " + rutaManifest + ": " + llegit.errorMessage());
    const JsonValue& arrel = llegit.value();
    m_peuX = arrel["peu"]["x"].asInt(32);
    m_peuY = arrel["peu"]["y"].asInt(60);
    const JsonValue& llista = arrel["especies"];
    for (std::size_t i = 0; llista.isArray() && i < llista.size(); ++i) {
        const JsonValue& e = llista[i];
        FullaEspecie f;
        f.especie.id = e["id"].asString();
        f.especie.velocitat = static_cast<float>(e["velocitat"].asNumber(1.4));
        f.especie.radi = e["radi"].asInt(4);
        f.especie.afectuosa = e["afectuosa"].asBool(true);
        f.ombra = static_cast<float>(e["ombra"].asNumber(24.0));
        for (const auto& [nom, a] : e["anims"].objectValues()) f.anims[nom] = llegeixAnim(a);
        f.especie.dorm = f.anims.count("dorm") != 0;
        const JsonValue& mg = e["magia"];
        f.magia.color = mg["color"].asString();
        f.magia.particula = mg["particula"].asString();
        f.magia.quantes = mg["quantes"].asInt(0);
        f.magia.alcada = static_cast<float>(mg["alcada"].asNumber(10.0));
        f.magia.anell = mg["anell"].asBool(false);
        f.magia.rastre = mg["rastre"].asBool(false);
        if (f.especie.id.empty() || f.anims.count("camina") == 0 || f.anims.count("repos") == 0)
            return Result<bool>::Error("FullaMascotes: l'especie «" + f.especie.id + "» no te «camina» i «repos»");
        m_especies.push_back(f);
    }
    for (const auto& [nom, v] : arrel["efectes"].objectValues()) m_efectes[nom] = v.asInt(0);
    return Result<bool>::Ok(!m_especies.empty());
}

std::vector<EspecieMascota> FullaMascotes::nomesEspecies() const {
    std::vector<EspecieMascota> r;
    for (const FullaEspecie& f : m_especies) r.push_back(f.especie);
    return r;
}

int FullaMascotes::indexDe(const std::string& id) const {
    for (std::size_t i = 0; i < m_especies.size(); ++i)
        if (m_especies[i].especie.id == id) return static_cast<int>(i);
    return -1;
}

int FullaMascotes::efecte(const std::string& nom) const {
    const auto it = m_efectes.find(nom);
    return it == m_efectes.end() ? 0 : it->second;
}

std::string FullaMascotes::animDe(const Mascota& m, const FullaEspecie& e) {
    const auto te = [&](const char* nom) { return e.anims.count(nom) != 0; };
    switch (m.estat) {
        case EstatMascota::Camina: return "camina";
        case EstatMascota::Fuig: return te("fuig") ? "fuig" : "camina";
        case EstatMascota::Saluda: return te("saluda") ? "saluda" : "repos";
        case EstatMascota::Dorm: return te("dorm") ? "dorm" : "repos";
        case EstatMascota::Repos: break;
    }
    return m.variant == 1 && te("repos2") ? "repos2" : "repos";
}

int FullaMascotes::frameDe(const Mascota& m) const {
    if (m.especie < 0 || m.especie >= static_cast<int>(m_especies.size())) return 0;
    const FullaEspecie& e = m_especies[static_cast<std::size_t>(m.especie)];
    std::string nom = animDe(m, e);
    if (!e.anims.at(nom).te(m.direccio)) nom = "repos";
    const AnimMascota& a = e.anims.at(nom);
    if (!a.te(m.direccio)) return 0;
    const auto [primer, quants] = a.dirs[static_cast<std::size_t>(m.direccio)];
    int k = 0;
    if (m.estat == EstatMascota::Camina || m.estat == EstatMascota::Fuig) {
        // Pel cami fet, no pel rellotge: els peus no rellisquen.
        const float velocitat = e.especie.velocitat * (m.estat == EstatMascota::Fuig ? 1.7f : 1.0f);
        k = static_cast<int>(std::floor(m.recorregut * a.fps / std::max(0.1f, velocitat))) % quants;
    } else {
        const int t = static_cast<int>(std::floor(m.rellotge * a.fps));
        k = a.aguanta ? std::min(quants - 1, t) : t % quants;
    }
    return primer + k;
}

}  // namespace Game
