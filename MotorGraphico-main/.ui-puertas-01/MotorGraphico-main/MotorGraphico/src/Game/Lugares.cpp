// AV15-02 — Los sitios de la ciudad por su nombre (ver include/Game/Lugares.h).
#include "Game/Lugares.h"

#include <fstream>
#include <sstream>

#include "Core/Json/JsonValue.h"

namespace Game {

namespace {

// "assets/levels/x.json" y "/ruta/al/proyecto/assets/levels/x.json" son el mismo nivel.
bool mismoNivel(const std::string& a, const std::string& b) {
    if (a.empty() || b.empty()) return false;
    const std::string& corta = a.size() <= b.size() ? a : b;
    const std::string& larga = a.size() <= b.size() ? b : a;
    if (larga.compare(larga.size() - corta.size(), corta.size(), corta) != 0) return false;
    return larga.size() == corta.size() || larga[larga.size() - corta.size() - 1] == '/';
}

}  // namespace

Result<Lugares> Lugares::desdeTexto(const std::string& json) {
    const auto doc = JsonValue::parse(json);
    if (!doc.isOk()) return Result<Lugares>::Error("lugares: JSON invalido");
    const JsonValue& lista = doc.value()["lugares"];
    if (!lista.isArray()) return Result<Lugares>::Error("lugares: falta la lista \"lugares\"");
    Lugares l;
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const JsonValue& e = lista[i];
        Lugar lugar{e["nivel"].asString(), e["nombre"].asString(), e["funcion"].asString(), e["emblema"].asString(),
                    e["quien"].asString()};
        if (!lugar.nivel.empty() && !lugar.nombre.empty()) l.m_lugares.push_back(std::move(lugar));
    }
    const JsonValue& calles = doc.value()["calles"];  // AV16-06 (opcional)
    for (std::size_t i = 0; calles.isArray() && i < calles.size(); ++i) {
        Calle c{calles[i]["nivel"].asString(), calles[i]["nombre"].asString(), calles[i]["salir"].asString()};
        if (!c.nivel.empty() && !c.nombre.empty()) l.m_calles.push_back(std::move(c));
    }
    return Result<Lugares>::Ok(std::move(l));
}

Result<Lugares> Lugares::carga(const std::string& ruta) {
    std::ifstream f(ruta, std::ios::binary);
    if (!f) return Result<Lugares>::Error("lugares: no se puede abrir " + ruta);
    std::ostringstream texto;
    texto << f.rdbuf();
    return desdeTexto(texto.str());
}

const Lugares& Lugares::delJuego() {
    static const Lugares lugares = [] {
        auto r = carga("assets/catalogs/lugares.json");
        return r.isOk() ? r.value() : Lugares{};
    }();
    return lugares;
}

const Lugar* Lugares::de(const std::string& nivel) const {
    for (const Lugar& l : m_lugares)
        if (mismoNivel(l.nivel, nivel)) return &l;
    return nullptr;
}

bool Lugares::esSalida(const std::string& objectId) { return objectId.rfind("salida_", 0) == 0; }

const Calle* Lugares::calle(const std::string& nivel) const {
    for (const Calle& c : m_calles)
        if (mismoNivel(c.nivel, nivel)) return &c;
    return nullptr;
}

std::string Lugares::nombreDeDestino(const std::string& nivel) const {
    if (const Lugar* l = de(nivel)) return l->nombre;
    if (const Calle* c = calle(nivel)) return c->nombre;
    return {};
}

TextoPuerta Lugares::textoDePuerta(const std::string& objectId, const std::string& nivelDestino,
                                   const std::string& nombreObjeto) const {
    if (esSalida(objectId)) {
        const Calle* c = calle(nivelDestino);
        return {"Salir a", c != nullptr && !c->salir.empty() ? c->salir : "la calle"};
    }
    const Lugar* l = de(nivelDestino);
    if (l != nullptr) return {"Entrar en", l->nombre};
    // Un paso que ya se llama como una orden («Volver al barco») o una dirección («Hacia Este»)
    // no es un edificio: «Volver al barco», «Ir hacia Este», no «Entrar en Volver al barco».
    for (const char* verbo : {"Volver", "Subir", "Bajar", "Ir", "Salir"}) {
        const std::string v = std::string(verbo) + " ";
        if (nombreObjeto.rfind(v, 0) == 0) return {verbo, nombreObjeto.substr(v.size())};
    }
    if (nombreObjeto.rfind("Hacia ", 0) == 0) return {"Ir", "h" + nombreObjeto.substr(1)};
    return {"Entrar en", nombreObjeto};
}

}  // namespace Game
