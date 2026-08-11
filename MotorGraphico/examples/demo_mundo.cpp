// demo_mundo — el mundo de Egaroth dentro del motor.
//
// Cierra la brecha B24 de GAMEMACHINE_NECESIDADES: hasta ahora el motor no
// tenia concepto de mundo. Habia tres niveles de ciudad sueltos y un
// LevelTransition para saltar entre ellos, pero nada que dijera que Bomengrid
// es la capital de Udrax, que Udrax linda con Ecla, o donde cae eso en un mapa.
//
// Ese dato SI existia: vivia en dndWeebCC/data/mapa/geografia.json (19
// poligonos de nacion, 95 ciudades con coordenadas, 26 zonas) y no se
// exportaba a ningun sitio. Ahora llega como assets/catalogs/locations.json.
//
// El mapa esta FECHADO en 2000 b.f., el primer ano de las Grandes Cruzadas.
// No describe Egaroth en general: describe Egaroth ese ano.
//
// GL-free: no abre ventana, solo lee JSON. Se puede ejecutar en CI.

#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "Check.h"
#include "RPG/Catalogs/RpgCatalogs.h"

using RPG::LocationDefinition;
using RPG::Catalogs::LocationCatalog;

namespace {

const char* nombreDe(LocationDefinition::Kind k) {
    switch (k) {
        case LocationDefinition::Kind::Nation: return "nacion";
        case LocationDefinition::Kind::City:   return "ciudad";
        case LocationDefinition::Kind::Zone:   return "zona";
    }
    return "?";
}

}  // namespace

int main() {
    LocationCatalog mundo;
    // OJO: dentro de require(), no de assert(). En Release assert() no evalua
    // la expresion y el catalogo se quedaria vacio sin avisar (ver Check.h).
    require(mundo.loadFromFile("assets/catalogs/locations.json"));

    std::map<std::string, int> porTipo;
    std::map<std::string, int> ciudadesPorNacion;
    std::vector<const LocationDefinition*> santuarios;
    int sinNombreCanonico = 0;
    int nacionesConPoligono = 0;

    mundo.forEach([&](const LocationDefinition& l) {
        porTipo[nombreDe(l.kind)]++;
        if (l.kind == LocationDefinition::Kind::Nation && !l.polygon.empty()) {
            ++nacionesConPoligono;
        }
        if (l.kind == LocationDefinition::Kind::City) {
            ciudadesPorNacion[l.controlledBy]++;
            if (l.pendingCanonName) ++sinNombreCanonico;
        }
        if (l.terrain == "santuario") santuarios.push_back(&l);
    });

    std::printf("Egaroth cargado: %d localizaciones\n", static_cast<int>(mundo.size()));
    for (const auto& [tipo, n] : porTipo) {
        std::printf("  %-8s %3d\n", tipo.c_str(), n);
    }

    require(porTipo["nacion"] == 19);
    require(porTipo["ciudad"] == 95);
    require(porTipo["zona"] == 26);
    require(nacionesConPoligono == 19);   // toda nacion tiene forma

    // Una capital concreta, para ver que los campos narrativos llegan enteros.
    const LocationDefinition* bomengrid = nullptr;
    mundo.forEach([&](const LocationDefinition& l) {
        if (l.name == "Bomengrid") bomengrid = &l;
    });
    require(bomengrid != nullptr);
    require(bomengrid->settlementSize == "capital");
    require(bomengrid->controlledBy == "Udrax");
    require(bomengrid->hasPosition);
    std::printf("\n%s: %s de %s, gobierna %s, %s habitantes, en (%d,%d)\n",
                bomengrid->name.c_str(), bomengrid->settlementSize.c_str(),
                bomengrid->controlledBy.c_str(), bomengrid->ruler.c_str(),
                bomengrid->population.c_str(), bomengrid->x, bomengrid->y);

    // Los santuarios son los sitios sagrados del panteon: Himetsumota, el
    // Templo de Sofia, el de Chronos, el de Envidia y la Torre del Viento.
    std::printf("\nsantuarios (%d):\n", static_cast<int>(santuarios.size()));
    for (const auto* s : santuarios) {
        std::printf("  %-22s en %s\n", s->name.c_str(), s->controlledBy.c_str());
    }
    require(santuarios.size() == 5);

    std::printf("\nciudades por nacion:\n");
    for (const auto& [nacion, n] : ciudadesPorNacion) {
        std::printf("  %-20s %2d\n", nacion.c_str(), n);
    }

    // Deuda conocida, y se comprueba que no crece: Ostad es la unica nacion
    // cuya capital sigue sin nombre canonico (el indice de lugares de Onegai
    // dice literalmente "mas Ostad sin capital").
    std::printf("\nciudades sin nombre canonico todavia: %d\n", sinNombreCanonico);
    require(sinNombreCanonico == 1);

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
