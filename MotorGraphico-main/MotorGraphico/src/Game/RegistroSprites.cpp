// SPRITES-01 — ver include/Game/RegistroSprites.h.
#include "Game/RegistroSprites.h"

#include <cstdio>
#include <fstream>
#include <set>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Game {

namespace {

Result<JsonValue> leeJson(const std::string& ruta) {
    std::ifstream entrada(ruta, std::ios::binary);
    if (!entrada) return Result<JsonValue>::Error("no puedo abrir " + ruta);
    std::stringstream texto;
    texto << entrada.rdbuf();
    auto leido = JsonValue::parse(texto.str());
    if (!leido.isOk()) return Result<JsonValue>::Error(ruta + ": " + leido.errorMessage());
    return leido;
}

// La carpeta del manifiesto, para resolver su «image» relativa.
std::string carpetaDe(const std::string& ruta) {
    const auto barra = ruta.find_last_of("/\\");
    return barra == std::string::npos ? std::string() : ruta.substr(0, barra + 1);
}

}  // namespace

Result<bool> RegistroSprites::carga(const std::string& carpetaAssets) {
    m_hojas.clear();
    m_actores.clear();
    m_fijos.clear();
    m_alias.clear();
    m_patrones.clear();
    m_avisos.clear();
    m_cargado = false;
    const std::string ruta = carpetaAssets + "/textures/sprites.json";
    auto indice = leeJson(ruta);
    if (!indice.isOk()) return Result<bool>::Error("RegistroSprites: " + indice.errorMessage());
    const JsonValue& raiz = indice.value();
    if (!raiz["hojas"].isArray()) return Result<bool>::Error("RegistroSprites: " + ruta + " sin «hojas»");
    for (std::size_t i = 0; i < raiz["hojas"].size(); ++i) {
        const JsonValue& h = raiz["hojas"][i];
        const std::string id = h["id"].asString();
        const std::string manifiesto = h["manifiesto"].asString();
        if (id.empty() || manifiesto.empty()) {
            m_avisos.push_back("hoja sin id o sin manifiesto en " + ruta);
            continue;
        }
        auto r = cargaHoja(carpetaAssets, id, manifiesto, h["prefijo"].asString());
        if (!r.isOk()) m_avisos.push_back(r.errorMessage());
    }
    for (const auto& [de, a] : raiz["alias"].objectValues()) m_alias[de] = a.asString();
    if (raiz["patrones"].isArray()) {
        for (std::size_t i = 0; i < raiz["patrones"].size(); ++i) {
            const JsonValue& p = raiz["patrones"][i];
            Patron patron;
            patron.sufijo = p.has("sufijo");
            patron.texto = patron.sufijo ? p["sufijo"].asString() : p["prefijo"].asString();
            patron.id = p["id"].asString();
            if (patron.texto.empty() || patron.id.empty()) {
                m_avisos.push_back("patron sin texto o sin id en " + ruta);
                continue;
            }
            m_patrones.push_back(patron);
        }
    }
    m_cargado = true;
    return Result<bool>::Ok(true);
}

Result<bool> RegistroSprites::cargaHoja(const std::string& carpetaAssets, const std::string& id,
                                        const std::string& manifiesto, const std::string& prefijo) {
    const std::string ruta = carpetaAssets + "/" + manifiesto;
    auto leido = leeJson(ruta);
    if (!leido.isOk()) return Result<bool>::Error("hoja " + id + ": " + leido.errorMessage());
    const JsonValue& m = leido.value();
    HojaSprites hoja;
    hoja.id = id;
    const std::string imagen = m["image"].asString();
    if (imagen.empty()) return Result<bool>::Error("hoja " + id + ": " + ruta + " sin «image»");
    hoja.imagen = carpetaDe(manifiesto) + imagen;
    hoja.celdaAncho = m["frameSize"][0].asInt(64);
    hoja.celdaAlto = m["frameSize"][1].asInt(64);
    hoja.columnas = m["columns"].asInt(1);
    hoja.filas = m["rows"].asInt(1);
    if (hoja.columnas <= 0 || hoja.filas <= 0 || hoja.celdaAncho <= 0 || hoja.celdaAlto <= 0)
        return Result<bool>::Error("hoja " + id + ": medidas sin sentido en " + ruta);
    hoja.sombraBlob = m["sombraBlob"].asBool(true);  // INT-03
    if (m["actores"].isArray()) {
        hoja.actores = true;
        for (std::size_t i = 0; i < m["actores"].size(); ++i) {
            const JsonValue& a = m["actores"][i];
            const std::string clave = a["id"].asString();
            const int fila = a["fila"].asInt(0);
            if (clave.empty() || fila < 1 || fila > hoja.filas) {
                m_avisos.push_back("hoja " + id + ": actor «" + clave + "» con fila fuera de la hoja");
                continue;
            }
            SpriteActor actor{id, fila, {}};
            const JsonValue& variantes = a["variantes"];
            for (std::size_t k = 0; k < variantes.size(); ++k) {
                const int v = variantes[k].asInt(0);
                if (v < 1 || v > hoja.filas) {
                    m_avisos.push_back("hoja " + id + ": variante de «" + clave + "» fuera de la hoja");
                    continue;
                }
                actor.variantes.push_back(v);
            }
            m_actores[prefijo + clave] = std::move(actor);
        }
    } else if (m["frames"].isObject()) {
        const int total = hoja.columnas * hoja.filas;
        for (const auto& [clave, v] : m["frames"].objectValues()) {
            const int frame = v.asInt(0);
            if (frame < 1 || frame > total) {
                m_avisos.push_back("hoja " + id + ": frame de «" + clave + "» fuera de la hoja");
                continue;
            }
            m_fijos[prefijo + clave] = SpriteFijo{id, frame};
        }
    } else {
        return Result<bool>::Error("hoja " + id + ": " + ruta + " no tiene ni «actores» ni «frames»");
    }
    m_hojas[id] = hoja;
    return Result<bool>::Ok(true);
}

std::string RegistroSprites::resuelveAlias(const std::string& id) const {
    const auto it = m_alias.find(id);
    return it == m_alias.end() ? id : it->second;
}

const HojaSprites* RegistroSprites::hoja(const std::string& id) const {
    const auto it = m_hojas.find(id);
    return it == m_hojas.end() ? nullptr : &it->second;
}

std::vector<std::string> RegistroSprites::candidatos(const std::string& objectId) const {
    std::vector<std::string> ids{resuelveAlias(objectId)};
    for (const Patron& p : m_patrones) {
        const bool encaja = p.sufijo ? (objectId.size() >= p.texto.size() &&
                                        objectId.compare(objectId.size() - p.texto.size(), p.texto.size(), p.texto) == 0)
                                     : objectId.compare(0, p.texto.size(), p.texto) == 0;
        if (encaja) ids.push_back(p.id);
    }
    return ids;
}

const SpriteActor* RegistroSprites::actor(const std::string& objectId, const std::string& raceId) const {
    for (const std::string& id : candidatos(objectId)) {
        const auto it = m_actores.find(id);
        if (it != m_actores.end()) return &it->second;
    }
    if (!raceId.empty()) {
        const auto it = m_actores.find("race:" + raceId);
        if (it != m_actores.end()) return &it->second;
    }
    return nullptr;
}

const SpriteFijo* RegistroSprites::fijo(const std::string& objectId) const {
    for (const std::string& id : candidatos(objectId)) {
        const auto it = m_fijos.find(id);
        if (it != m_fijos.end()) return &it->second;
    }
    return nullptr;
}

const SpriteActor* actorConRespaldo(const RegistroSprites& registro, const std::string& objectId,
                                    const std::string& raceId, bool humanoide) {
    if (const SpriteActor* propio = registro.actor(objectId, raceId)) return propio;
    if (!humanoide) return nullptr;
    const SpriteActor* neutra = registro.actor("", kRazaRespaldo);
    static std::set<std::string> avisados;
    if (neutra != nullptr && avisados.insert(objectId).second)
        std::fprintf(stderr, "RegistroSprites: «%s» no tiene fila propia: lleva race:%s (respaldo)\n",
                     objectId.c_str(), kRazaRespaldo);
    return neutra;
}

}  // namespace Game
