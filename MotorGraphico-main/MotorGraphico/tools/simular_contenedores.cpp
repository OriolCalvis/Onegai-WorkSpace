// AV18-04: adaptador de consola. La regla y el azar viven en motor_level.
#include "Game/Contenedores.h"
#include "Core/Json/JsonValue.h"
#include "Core/Json/JsonWriter.h"
#include "Level/ObjectCatalog.h"
#include "RPG/Narrative/NarrativeState.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>

namespace {
std::string lee(const std::string& ruta) {
    std::ifstream f(ruta);
    if (!f) throw std::runtime_error("No se puede leer " + ruta);
    return std::string(std::istreambuf_iterator<char>(f), {});
}
}
int main() {
    try {
        auto proyecto = JsonValue::parse(lee("assets/proyectos/boundington.json"));
        auto solicitud = JsonValue::parse(std::string(std::istreambuf_iterator<char>(std::cin), {}));
        if (!proyecto.isOk() || !solicitud.isOk()) throw std::runtime_error("JSON inválido");
        const int semilla = solicitud.value()["semilla"].asInt();
        const auto& aperturas = solicitud.value()["aperturas"];
        if (semilla < 1 || semilla > 1000000000 || !aperturas.isArray() || aperturas.size() > 10000)
            throw std::runtime_error("Semilla o aperturas inválidas");
        ObjectCatalog objetos;
        const auto& archivos = proyecto.value()["catalogs"];
        for (std::size_t i=0; i<archivos.size(); ++i) {
            auto r=objetos.loadFromFile("assets/objects/"+archivos[i].asString());
            if (!r.isOk()) throw std::runtime_error(r.errorMessage());
        }
        auto tablas=Game::leeTablasBotin(lee("assets/catalogs/contenedores_tablas.json"));
        if (!tablas.isOk()) throw std::runtime_error(tablas.errorMessage());
        std::map<std::string,RPG::NarrativeState> partidas;
        JsonWriter w; w.beginArray();
        for (std::size_t i=0; i<aperturas.size(); ++i) {
            const auto& a=aperturas[i]; const auto* objeto=objetos.find(a["objectId"].asString());
            const auto grupo=a["grupo"].asString(), nivel=a["nivel"].asString(), instancia=a["instancia"].asString();
            if (!objeto || grupo.empty() || nivel.empty() || instancia.empty()) throw std::runtime_error("Apertura sin identidad u objeto");
            auto t=std::find_if(tablas.value().begin(),tablas.value().end(),[&](const auto& t){return t.id==objeto->contenedor.tabla;});
            if (t==tablas.value().end()) throw std::runtime_error("Tabla no encontrada");
            auto r=Game::generaContenedor(objeto->contenedor,*t,objetos,partidas[grupo],Game::claveContenedor(nivel,instancia),semilla);
            if (!r.isOk()) throw std::runtime_error(r.errorMessage());
            w.beginObject();w.member("grupo",grupo);w.member("clave",r.value().clave);w.member("oro",r.value().oro);
            w.key("objetos");w.beginArray();for(const auto& id:r.value().objetos)w.value(id);w.endArray();w.endObject();
        }
        w.endArray(); std::cout<<w.str(); return 0;
    } catch (const std::exception& e) {std::cerr<<e.what()<<'\n';return 2;}
}
