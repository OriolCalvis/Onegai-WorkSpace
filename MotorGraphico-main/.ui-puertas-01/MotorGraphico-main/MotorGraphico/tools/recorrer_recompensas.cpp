// AV18-04: recorre decisiones con NarrativeEngine y devuelve sus premios/costes.
// No ejecuta combate, movimiento ni comercio: los resultados externos son escenarios explícitos.
#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "RPG/NarrativeEngine.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>

namespace {
void exige(bool si, const std::string& mensaje) {
    if (!si) throw std::runtime_error(mensaje);
}
std::string lee(const std::string& ruta) {
    std::ifstream f(ruta);
    exige(bool(f), "No se puede leer " + ruta);
    return std::string(std::istreambuf_iterator<char>(f), {});
}
std::vector<std::string> textos(const JsonValue& v) {
    std::vector<std::string> r;
    for (std::size_t i = 0; i < v.size(); ++i) r.push_back(v[i].asString());
    return r;
}
struct Paso {
    int dia = 0, orden = 0;
    std::string aventura, opcion;
};
// Estado de una reproducción de consola. No añade reglas al motor.
struct Partida {
    RPG::NarrativeState estado;
    RPG::NarrativeEngine motor;
    std::set<std::string> usadas;
};
void resultado(const RPG::NarrativeResult& r, const JsonValue& ruta, Partida& p,
               const Paso& paso, JsonWriter& w) {
    if (!r.fired) return;
    w.beginObject();
    w.member("dia", paso.dia); w.member("orden", paso.orden);
    w.member("aventura", paso.aventura); w.member("beat", r.beatId);
    w.member("opcion", paso.opcion); w.member("oro", r.goldDelta);
    w.key("objetos"); w.beginArray();
    for (const auto& id : r.objetos) w.value(id);
    w.endArray(); w.endObject();
    const std::string grado = ruta["grado"].asString();
    for (const auto& c : r.skillChecks) {
        const std::string f = grado == "critico" ? c.flagCritical : grado == "exito" ? c.flagSuccess :
                              grado == "parcial" ? c.flagPartial : c.flagBotch;
        exige(!f.empty(), "El escenario no tiene salida para la tirada de " + r.beatId);
        p.estado.setFlag(f);
    }
    for (const auto& b : r.battles) {
        const bool gana = ruta["victoria"].asBool();
        if (gana) for (const auto& rf : b.refuerzos)
            if (rf.requiere.isSatisfiedBy(p.estado)) for (const auto& f : rf.flags) p.estado.setFlag(f);
        const std::string f = gana ? b.flagVictory : b.flagDefeat;
        exige(!f.empty(), "Combate sin salida del escenario");
        p.estado.setFlag(f);
    }
    for (const auto& t : r.tablas) {
        const int cara = ruta["cara_tabla"].asInt(1);
        exige(cara > 0 && static_cast<std::size_t>(cara) <= t.flags.size(), "Cara de tabla inválida");
        p.estado.setFlag(t.flags[static_cast<std::size_t>(cara - 1)]);
    }
}
// Política de salida declarada por el escenario, como en las demos de campaña.
bool salida(const RPG::OpcionDeDialogo& o) {
    if (o.despedida) return true;
    if (o.sigue) return false;
    for (const auto& e : o.efectos) {
        using T = RPG::EffectType;
        if (e.type == T::Log) continue;
        if (e.type != T::SetFlag && e.type != T::ClearFlag && e.type != T::SetVariable && e.type != T::AddVariable) return false;
        bool libro = e.arg.find("_n_") != std::string::npos;
        for (const std::string fin : {"_en_charla", "_pag", "_abierta", "_visto"})
            libro |= e.arg.size() >= fin.size() && e.arg.compare(e.arg.size() - fin.size(), fin.size(), fin) == 0;
        if (!libro) return false;
    }
    return true;
}
void drena(const JsonValue& ruta, const std::vector<std::string>& preferencias, Partida& p,
           Paso paso, JsonWriter& w) {
    for (int n = 0; n < 200; ++n) {
        if (!p.estado.eleccion().activa()) {
            auto r = p.motor.tick(p.estado);
            if (!r.fired) r = p.motor.sigue(p.estado);
            if (!r.fired) return;
            paso.opcion.clear(); resultado(r, ruta, p, paso, w); continue;
        }
        const auto eleccion = p.estado.eleccion();
        const auto* guion = p.motor.adventure();
        const auto b = std::find_if(guion->beats.begin(), guion->beats.end(),
                                   [&](const auto& b) { return b.id == eleccion.beatId; });
        exige(b != guion->beats.end(), "Elección sin beat");
        int k = -1;
        auto libre = [&](std::size_t i) { return !p.usadas.count(eleccion.beatId + "|" + eleccion.textos[i]); };
        for (const auto& texto : preferencias)
            for (std::size_t i = 0; k < 0 && i < eleccion.textos.size(); ++i)
                if (eleccion.textos[i] == texto && libre(i)) k = static_cast<int>(i);
        for (std::size_t i = 0; k < 0 && i < eleccion.indices.size(); ++i)
            if (b->opciones[eleccion.indices[i]].pasaPagina && libre(i)) k = static_cast<int>(i);
        for (std::size_t i = 0; k < 0 && i < eleccion.indices.size(); ++i)
            if (b->opciones[eleccion.indices[i]].despedida) k = static_cast<int>(i);
        if (k < 0 && ruta["politica"].asString() == "ultima_salida") {
            k = static_cast<int>(eleccion.textos.size()) - 1;
            for (std::size_t i = 0; i < eleccion.indices.size(); ++i)
                if (salida(b->opciones[eleccion.indices[i]])) k = static_cast<int>(i);
        }
        // Sin política ni preferencia no se inventa una decisión.
        exige(k >= 0, "Falta preferencia en " + eleccion.beatId);
        paso.opcion = eleccion.textos[k];
        p.usadas.insert(eleccion.beatId + "|" + paso.opcion);
        resultado(p.motor.elige(k, p.estado), ruta, p, paso, w);
    }
    throw std::runtime_error("La charla no termina (200 pasos)");
}
}

int main() {
    try {
        auto entrada = JsonValue::parse(std::string(std::istreambuf_iterator<char>(std::cin), {}));
        exige(entrada.isOk(), "Solicitud JSON inválida");
        const auto& rutas = entrada.value()["rutas"];
        exige(rutas.isArray() && rutas.size() > 0 && rutas.size() <= 10, "Rutas inválidas");
        auto manifest = JsonValue::parse(lee("assets/proyectos/boundington.json"));
        exige(manifest.isOk(), "Manifiesto inválido");
        std::map<std::string, RPG::AdventureScript> guiones;
        for (const auto& nombre : textos(manifest.value()["adventures"])) {
            exige(nombre.find('/') == std::string::npos && nombre.find("..") == std::string::npos, "Ruta de aventura inválida");
            auto g = RPG::AdventureScript::loadFromFile("assets/adventures/" + nombre);
            exige(g.isOk(), g.isOk() ? "" : g.errorMessage());
            guiones.emplace(nombre, std::move(g.value()));
        }
        JsonWriter w; w.beginArray();
        std::set<std::string> perfiles;
        for (std::size_t i = 0; i < rutas.size(); ++i) {
            const auto& ruta = rutas[i]; Partida p;
            const std::string perfil = ruta["perfil"].asString(), grado = ruta["grado"].asString();
            exige(!perfil.empty() && perfiles.insert(perfil).second, "Perfil vacío o repetido");
            exige(grado == "exito" || grado == "parcial" || grado == "pifia" || grado == "critico", "Grado inválido");
            exige(ruta["victoria"].isBool(), "Falta resultado de combate");
            for (const auto& f : textos(ruta["flags_iniciales"])) p.estado.setFlag(f);
            w.beginObject(); w.member("perfil", perfil); w.key("traza"); w.beginArray();
            const auto& acciones = ruta["acciones"];
            exige(acciones.isArray() && acciones.size() <= 200, "Acciones inválidas");
            int anterior = 0;
            for (std::size_t j = 0; j < acciones.size(); ++j) {
                const auto& a = acciones[j];
                Paso paso{a["dia"].asInt(), a["orden"].asInt(), a["aventura"].asString(), {}};
                const int tiempo = paso.dia * 100 + paso.orden;
                exige(paso.dia >= 1 && paso.dia <= 3 && paso.orden > 0 && paso.orden < 90 && tiempo >= anterior, "Día/orden inválido");
                anterior = tiempo;
                const auto g = guiones.find(paso.aventura);
                exige(g != guiones.end(), "Aventura fuera del proyecto: " + paso.aventura);
                p.motor.setAdventure(&g->second);
                auto preferencias = textos(a["preferencias"]);
                const auto generales = textos(ruta["preferencias"]);
                preferencias.insert(preferencias.end(), generales.begin(), generales.end());
                const auto accion = a["accion"].asString(), destino = a["destino"].asString();
                if (accion == "entrar") {
                    const std::string nivel = "assets/levels/" + destino + ".json";
                    p.estado.ponLugar(nivel);
                    resultado(p.motor.enterLevel(nivel, p.estado), ruta, p, paso, w);
                } else if (accion == "hablar") resultado(p.motor.talkTo(destino, p.estado), ruta, p, paso, w);
                else exige(accion == "auto", "Acción desconocida: " + accion);
                drena(ruta, preferencias, p, paso, w);
            }
            w.endArray(); w.key("flags"); w.beginArray();
            for (const auto& f : p.estado.flags()) w.value(f);
            w.endArray();
            for (const auto& f : textos(ruta["flags_finales"])) exige(p.estado.hasFlag(f), "Ruta incompleta: falta " + f);
            w.endObject();
        }
        w.endArray(); std::cout << w.str() << '\n'; return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
