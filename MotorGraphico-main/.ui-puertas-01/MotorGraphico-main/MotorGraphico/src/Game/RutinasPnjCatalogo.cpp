// PNJ-02 — El catalogo de rutinas (assets/catalogs/rutinas_pnj.json) y las
// cuentas sin estado de RutinasPnj: la pose, los carriles del nivel y la zona
// que ocupa cada PNJ. Ver include/Game/RutinasPnj.h.
#include "Game/RutinasPnj.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <set>
#include <sstream>

#include "Core/Json/JsonValue.h"
#include "Level/ObjectCatalog.h"
#include "World/World.h"

namespace Game {

namespace {

// Cuantos caracteres (no bytes) tiene un texto UTF-8.
std::size_t letras(const std::string& s) {
    std::size_t n = 0;
    for (const char c : s) n += (static_cast<unsigned char>(c) & 0xC0) != 0x80 ? 1 : 0;
    return n;
}

// --- El catalogo: lo que no se entiende se ignora (queda el valor de antes). ---
void leeDef(const JsonValue& j, DefRutinaPnj& d) {
    if (!j.isObject()) return;
    static const std::map<std::string, RutinaPnj> kRutinas{
        {"quieto", RutinaPnj::Quieto},     {"deambular", RutinaPnj::Deambular}, {"ir_y_volver", RutinaPnj::IrYVolver},
        {"charlar", RutinaPnj::Charlar},   {"vigilar", RutinaPnj::Vigilar},     {"actividad", RutinaPnj::Actividad}};
    static const std::map<std::string, MovimientoPnj> kMovimientos{
        {"respirar", MovimientoPnj::Respirar}, {"vaiven", MovimientoPnj::Vaiven},          {"golpe", MovimientoPnj::Golpe},
        {"giro", MovimientoPnj::Giro},         {"andar_sitio", MovimientoPnj::AndarSitio}};
    static const std::map<std::string, int> kRumbos{{"arriba", 0}, {"derecha", 1}, {"abajo", 2}, {"izquierda", 3}};
    if (j.has("rutina")) {
        const auto it = kRutinas.find(j["rutina"].asString());
        if (it != kRutinas.end()) d.rutina = it->second;
    }
    if (j["radio"].isNumber()) d.radio = std::clamp(j["radio"].asInt(), 0, 8);
    const JsonValue& pausa = j["pausa"];
    if (pausa.isArray() && pausa.size() == 2 && pausa[0].isNumber() && pausa[1].isNumber()) {
        d.pausaMin = std::max(0.5f, static_cast<float>(pausa[0].asNumber()));
        d.pausaMax = std::max(d.pausaMin, static_cast<float>(pausa[1].asNumber()));
    }
    if (j["mira"].isString()) {
        const std::string mira = j["mira"].asString();
        if (mira == "jugador") d.mira = MiraPnj::Jugador;
        else if (mira == "calle") d.mira = MiraPnj::Calle;
        else if (mira == "vecino") d.mira = MiraPnj::Vecino;
        else if (mira == "giro") d.mira = MiraPnj::Giro;
        else if (mira.rfind("fijo:", 0) == 0 && kRumbos.count(mira.substr(5)) != 0) {
            d.mira = MiraPnj::Fijo;
            d.rumboFijo = kRumbos.at(mira.substr(5));
        }
    }
    const JsonValue& destino = j["destino"];
    if (destino.isArray() && destino.size() == 2 && destino[0].isNumber() && destino[1].isNumber())
        d.destino = GridCoord{std::clamp(destino[0].asInt(), -12, 12), std::clamp(destino[1].asInt(), -12, 12)};
    const JsonValue& act = j["actividad"];
    if (act.isObject()) {
        d.actividad = act["nombre"].asString(act["movimiento"].asString("actividad"));
        const auto it = kMovimientos.find(act["movimiento"].asString());
        if (it != kMovimientos.end()) d.movimiento = it->second;
        if (act["icono"].isString()) d.icono = act["icono"].asString();
        const JsonValue& cada = act["cada"];
        if (cada.isArray() && cada.size() == 2 && cada[0].isNumber() && cada[1].isNumber()) {
            d.cadaMin = std::max(0.5f, static_cast<float>(cada[0].asNumber()));
            d.cadaMax = std::max(d.cadaMin, static_cast<float>(cada[1].asNumber()));
        }
    }
    const JsonValue& frases = j["frases"];
    if (frases.isArray()) {
        d.frases.clear();
        for (std::size_t i = 0; i < frases.size(); ++i)
            if (frases[i].isString() && !frases[i].asString().empty() && letras(frases[i].asString()) <= 40)
                d.frases.push_back(frases[i].asString());
    }
}

}  // namespace

// =====================================================================
// El catalogo
// =====================================================================
Result<CatalogoRutinasPnj> CatalogoRutinasPnj::desdeTexto(const std::string& json) {
    const auto doc = JsonValue::parse(json);
    if (!doc.isOk() || !doc.value().isObject()) return Result<CatalogoRutinasPnj>::Error("rutinas_pnj: JSON invalido");
    CatalogoRutinasPnj c;
    leeDef(doc.value()["porDefecto"], c.m_defecto);
    const JsonValue& rutinas = doc.value()["rutinas"];
    if (rutinas.isObject()) {
        for (const auto& [clave, valor] : rutinas.objectValues()) {
            if (clave.empty() || clave[0] == '_' || !valor.isObject()) continue;
            DefRutinaPnj d = c.m_defecto;  // lo que no dice, como porDefecto
            leeDef(valor, d);
            if (clave.back() == '*') c.m_prefijos.push_back({clave.substr(0, clave.size() - 1), d});
            else c.m_exactas[clave] = d;
        }
    }
    // Entre prefijos gana el mas largo: se miran de mas largo a mas corto.
    std::stable_sort(c.m_prefijos.begin(), c.m_prefijos.end(),
                     [](const auto& a, const auto& b) { return a.first.size() > b.first.size(); });
    return Result<CatalogoRutinasPnj>::Ok(std::move(c));
}

Result<CatalogoRutinasPnj> CatalogoRutinasPnj::carga(const std::string& ruta) {
    std::ifstream f(ruta, std::ios::binary);
    if (!f) return Result<CatalogoRutinasPnj>::Error("rutinas_pnj: no se puede abrir " + ruta);
    std::ostringstream texto;
    texto << f.rdbuf();
    return desdeTexto(texto.str());
}

const CatalogoRutinasPnj& CatalogoRutinasPnj::delJuego() {
    static const CatalogoRutinasPnj catalogo = [] {
        auto r = carga("assets/catalogs/rutinas_pnj.json");
        return r.isOk() ? r.value() : CatalogoRutinasPnj{};
    }();
    return catalogo;
}

const DefRutinaPnj& CatalogoRutinasPnj::para(const std::string& objectId) const {
    const auto exacta = m_exactas.find(objectId);
    if (exacta != m_exactas.end()) return exacta->second;
    for (const auto& [prefijo, def] : m_prefijos)
        if (objectId.rfind(prefijo, 0) == 0) return def;
    return m_defecto;
}

const std::string& EstadoPnj::iconoVisible() const {
    static const std::string kNada;
    return actividad > 0.0f && !def.icono.empty() ? def.icono : kNada;
}

std::vector<GridCoord> RutinasPnj::leeCarriles(const std::string& rutaNivel) {
    std::vector<GridCoord> salida;
    std::ifstream f(rutaNivel, std::ios::binary);
    if (!f) return salida;
    std::ostringstream texto;
    texto << f.rdbuf();
    const auto doc = JsonValue::parse(texto.str());
    if (!doc.isOk()) return salida;
    const JsonValue& lista = doc.value()["carriles"];
    for (std::size_t i = 0; lista.isArray() && i < lista.size(); ++i)
        if (lista[i].isArray() && lista[i].size() == 2) salida.push_back(GridCoord{lista[i][0].asInt(), lista[i][1].asInt()});
    return salida;
}

std::vector<GridCoord> RutinasPnj::zona(const ObjectDefinition* def, const GridCoord& origen, const Engine::World& mundo,
                                        const CatalogoRutinasPnj& catalogo) {
    if (def == nullptr || def->category != ObjectCategory::Npc) return {origen};
    const DefRutinaPnj& d = catalogo.para(def->id);
    int x0 = origen.x, x1 = origen.x, y0 = origen.y, y1 = origen.y;
    if (d.rutina == RutinaPnj::IrYVolver) {
        x0 = std::min(x0, origen.x + d.destino.x), x1 = std::max(x1, origen.x + d.destino.x);
        y0 = std::min(y0, origen.y + d.destino.y), y1 = std::max(y1, origen.y + d.destino.y);
    } else if (d.rutina != RutinaPnj::Quieto && d.rutina != RutinaPnj::Vigilar) {
        x0 -= d.radio, x1 += d.radio, y0 -= d.radio, y1 += d.radio;
    }
    std::set<std::pair<int, int>> casas;  // las mascotas necesitan su casa libre
    mundo.eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& ent) {
        if (ent.kind == Engine::EntityKind::Object && ent.object.objectId.rfind("mascota_", 0) == 0)
            casas.insert({ent.object.position.x, ent.object.position.y});
    });
    std::vector<GridCoord> salida;
    for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x)
            if (casas.count({x, y}) == 0) salida.push_back(GridCoord{x, y});
    return salida;
}

const char* RutinasPnj::nombreRumbo(int rumbo) {
    static const char* const kNombres[4] = {"up", "right", "down", "left"};
    return kNombres[((rumbo % 4) + 4) % 4];
}

int RutinasPnj::rumboHacia(const GridCoord& desde, const GridCoord& hacia) {
    const int dx = hacia.x - desde.x, dy = hacia.y - desde.y;
    if (dx == 0 && dy == 0) return 2;
    if (std::abs(dy) >= std::abs(dx)) return dy < 0 ? 0 : 2;  // como el jugador: manda el eje y
    return dx < 0 ? 3 : 1;
}

std::string RutinasPnj::animacion(const EstadoPnj& e, bool reducidas) {
    if (e.andando()) return std::string("walk_") + nombreRumbo(e.rumbo);
    if (e.actividad > 0.0f) {
        switch (e.def.movimiento) {
            case MovimientoPnj::Vaiven:  // mira a un lado y a otro (pregonar, barrer, buscar)
                if (!reducidas) return std::string("idle_") + (std::fmod(e.reloj, 1.2f) < 0.6f ? "left" : "right");
                break;
            case MovimientoPnj::AndarSitio:  // el ciclo de andar sin moverse (bailar, impaciente)
                if (!reducidas) return std::string("walk_") + nombreRumbo(e.rumbo);
                break;
            case MovimientoPnj::Giro:  // los cuatro rumbos, despacio
                return std::string("idle_") + nombreRumbo(e.rumbo + static_cast<int>(e.reloj / 0.8f));
            case MovimientoPnj::Golpe:
            case MovimientoPnj::Respirar:
                break;
        }
    }
    return std::string("idle_") + nombreRumbo(e.rumbo);
}

float RutinasPnj::saltoPx(const EstadoPnj& e, bool reducidas) {
    if (e.andando()) return 0.0f;
    if (e.actividad > 0.0f && e.def.movimiento == MovimientoPnj::Golpe)
        return std::fmod(e.reloj, 0.5f) < 0.2f ? 2.0f : 0.0f;  // baja y sube 2 px, ritmico
    if (e.actividad > 0.0f && e.def.movimiento == MovimientoPnj::AndarSitio && !reducidas) return 0.0f;
    return std::fmod(e.reloj, 2.4f) < 0.8f ? -1.0f : 0.0f;  // respirar: sube 1 px un rato
}

}  // namespace Game
