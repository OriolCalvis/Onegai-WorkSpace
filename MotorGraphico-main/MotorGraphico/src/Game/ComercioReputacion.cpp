// AV14-23 (D0015 §15, D-MOR-09) — ver include/Game/ComercioReputacion.h.
#include "Game/ComercioReputacion.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"

namespace Game {

namespace {

std::shared_ptr<const ReglasFacciones>& actuales() {
    static std::shared_ptr<const ReglasFacciones> r;
    return r;
}

std::string conSigno(int v) { return (v > 0 ? "+" : "") + std::to_string(v); }

}  // namespace

const Faccion* ReglasFacciones::faccion(const std::string& id) const {
    for (const Faccion& f : facciones)
        if (f.id == id) return &f;
    return nullptr;
}

const Faccion* ReglasFacciones::deTienda(const std::string& idTendero) const {
    if (idTendero.empty()) return nullptr;
    for (const Faccion& f : facciones)
        if (std::find(f.tiendas.begin(), f.tiendas.end(), idTendero) != f.tiendas.end()) return &f;
    return nullptr;
}

ReglasFacciones leeFacciones(const std::string& json) {
    ReglasFacciones r;
    auto raiz = JsonValue::parse(json);
    if (!raiz.isOk()) return r;
    const JsonValue& precios = raiz.value()["precios"];
    r.descuentoMaximo = std::max(0, precios["descuentoMaximo"].asInt(r.descuentoMaximo));
    r.recargoMaximo = std::max(0, precios["recargoMaximo"].asInt(r.recargoMaximo));
    const JsonValue& lista = raiz.value()["facciones"];
    for (std::size_t i = 0; lista.isArray() && i < lista.size(); ++i) {
        Faccion f;
        f.id = lista[i]["id"].asString("");
        if (f.id.empty()) continue;
        f.nombre = lista[i]["nombre"].asString(f.id);
        f.descripcion = lista[i]["descripcion"].asString("");
        const JsonValue& tiendas = lista[i]["tiendas"];
        for (std::size_t k = 0; tiendas.isArray() && k < tiendas.size(); ++k)
            f.tiendas.push_back(tiendas[k].asString(""));
        r.facciones.push_back(std::move(f));
    }
    return r;
}

const ReglasFacciones& facciones() {
    std::shared_ptr<const ReglasFacciones>& r = actuales();
    if (!r) {
        std::ifstream f("assets/rules/facciones.json", std::ios::binary);
        std::ostringstream texto;
        if (f) texto << f.rdbuf();
        r = std::make_shared<const ReglasFacciones>(leeFacciones(texto.str()));
    }
    return *r;
}

void ponFacciones(std::shared_ptr<const ReglasFacciones> reglas) { actuales() = std::move(reglas); }

int ajustePrecioPct(const ReglasFacciones& reglas, const RPG::CharacterSheet* ficha, const std::string& idTendero) {
    const Faccion* f = reglas.deTienda(idTendero);
    if (f == nullptr || ficha == nullptr) return 0;
    return std::clamp(ficha->repPriceModPct(f->id), -reglas.descuentoMaximo, reglas.recargoMaximo);
}

int precioConAjuste(int precio, int ajustePct) {
    if (precio <= 0) return precio;
    const double ajustado = static_cast<double>(precio) * (100 + ajustePct) / 100.0;
    return std::max(1, static_cast<int>(std::lround(ajustado)));
}

int recompraConAjuste(int recompraPct, int ajustePct) {
    if (ajustePct <= 0) return recompraPct;
    return std::max(0, static_cast<int>(std::lround(recompraPct * (100.0 - ajustePct) / 100.0)));
}

std::string comoTeVen(int r) {
    if (r <= -50) return "Te detestan";
    if (r <= -20) return "Desconfían";
    if (r < -5) return "Recelan";
    if (r <= 5) return "Neutral";
    if (r < 20) return "Te aprecian";
    if (r < 50) return "Te estiman";
    return "Eres de los suyos";
}

std::string tenderoPorNombre(const ReglasFacciones& reglas, const ObjectCatalog* catalogo, const std::string& nombre) {
    if (catalogo == nullptr || nombre.empty()) return {};
    for (const Faccion& f : reglas.facciones)
        for (const std::string& id : f.tiendas)
            if (const ObjectDefinition* def = catalogo->find(id); def != nullptr && def->name == nombre) return id;
    return {};
}

std::string lineaDeLaTienda(const ReglasFacciones& reglas, const RPG::CharacterSheet* ficha,
                            const std::string& idTendero) {
    const Faccion* f = reglas.deTienda(idTendero);
    const int pct = ajustePrecioPct(reglas, ficha, idTendero);
    if (f == nullptr || ficha == nullptr || pct == 0) return {};
    const int rep = ficha->getRep(f->id);
    std::string texto = comoTeVen(rep);
    texto[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(texto[0])));
    std::string linea = f->nombre + ": " + texto + " (" + conSigno(rep) + "). ";
    if (pct < 0) return linea + "Pagas un " + std::to_string(-pct) + " % menos.";
    return linea + "Pagas un " + std::to_string(pct) + " % más, y te pagan menos.";
}

std::vector<std::pair<std::string, std::string>> reputacionEnLaFicha(const ReglasFacciones& reglas,
                                                                     const RPG::CharacterSheet* ficha) {
    std::vector<std::pair<std::string, std::string>> filas;
    if (ficha == nullptr) return filas;
    auto fila = [&](const std::string& nombre, int rep) {
        filas.push_back({nombre, comoTeVen(rep) + " (" + conSigno(rep) + ")"});
    };
    for (const Faccion& f : reglas.facciones)
        if (const int rep = ficha->getRep(f.id); rep != 0) fila(f.nombre, rep);
    for (const auto& [id, rep] : ficha->factionRep)
        if (rep != 0 && reglas.faccion(id) == nullptr) fila(id, rep);
    return filas;
}

}  // namespace Game
