// FICHA-01 — ver include/HUD/Iconos.h.
#include "HUD/Iconos.h"

#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Hud {

namespace {
void leerTabla(const JsonValue& raiz, const char* clave, std::map<std::string, std::string>& tabla) {
    tabla.clear();
    if (!raiz.has(clave) || !raiz[clave].isObject()) return;
    for (const auto& [k, v] : raiz[clave].objectValues()) tabla[k] = v.asString();
}
}  // namespace

Result<bool> Iconos::cargar(const std::string& rutaJson) {
    std::ifstream in(rutaJson, std::ios::binary);
    if (!in) return Result<bool>::Error("Iconos: no se puede abrir " + rutaJson);
    std::stringstream ss;
    ss << in.rdbuf();
    auto parsed = JsonValue::parse(ss.str());
    if (!parsed.isOk()) return Result<bool>::Error("Iconos: " + parsed.errorMessage());
    const JsonValue& raiz = parsed.value();
    if (!raiz.has("iconos") || !raiz["iconos"].isObject()) {
        return Result<bool>::Error("Iconos: " + rutaJson + " no tiene la tabla 'iconos'");
    }
    m_frames.clear();
    for (const auto& [id, def] : raiz["iconos"].objectValues()) {
        const int f = def["frame"].asInt(0);
        if (f > 0) m_frames[id] = f;
    }
    m_columnas = raiz["columns"].asInt(0);
    m_filas = raiz["rows"].asInt(0);
    m_lado = raiz["frameSize"].isArray() ? raiz["frameSize"][0].asInt(16) : 16;
    m_imagen = raiz["image"].asString("iconos.png");
    leerTabla(raiz, "stats", m_stats);
    leerTabla(raiz, "ranuras", m_ranuras);
    leerTabla(raiz, "condiciones", m_condiciones);
    leerTabla(raiz, "misiones", m_misiones);
    leerTabla(raiz, "codice", m_codice);
    if (raiz.has("condicionPorDefecto")) {
        m_perjuicio = raiz["condicionPorDefecto"]["negativa"].asString(m_perjuicio);
        m_mejora = raiz["condicionPorDefecto"]["positiva"].asString(m_mejora);
    }
    if (m_frames.empty()) return Result<bool>::Error("Iconos: " + rutaJson + " sin iconos");
    return Result<bool>::Ok(true);
}

int Iconos::frame(const std::string& id) const {
    const auto it = m_frames.find(id);
    return it == m_frames.end() ? 0 : it->second;
}

int Iconos::viaTabla(const std::map<std::string, std::string>& tabla, const std::string& clave) const {
    const auto it = tabla.find(clave);
    return it == tabla.end() ? 0 : frame(it->second);
}

int Iconos::frameStat(const std::string& clave) const { return viaTabla(m_stats, clave); }
int Iconos::frameRanura(const std::string& ranura) const { return viaTabla(m_ranuras, ranura); }
int Iconos::frameMision(const std::string& estado) const { return viaTabla(m_misiones, estado); }
int Iconos::frameCodice(const std::string& tipo) const { return viaTabla(m_codice, tipo); }

int Iconos::frameCondicion(const std::string& id, bool negativa) const {
    if (const int propio = viaTabla(m_condiciones, id); propio > 0) return propio;
    return frame(negativa ? m_perjuicio : m_mejora);
}

}  // namespace Hud
