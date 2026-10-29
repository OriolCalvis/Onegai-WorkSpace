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

#include <cstddef>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "Core/Check.h"
#include "Game/WorldTravelState.h"
#include "Level/LevelLoader.h"
#include "Level/WorldTopology.h"
#include "RPG/Catalogs/RpgCatalogs.h"

using RPG::LocationDefinition;
using RPG::Catalogs::LocationCatalog;

namespace {

const char* nombreDe(LocationDefinition::Kind k) {
    switch (k) {
        case LocationDefinition::Kind::Nation:
            return "nacion";
        case LocationDefinition::Kind::City:
            return "ciudad";
        case LocationDefinition::Kind::Zone:
            return "zona";
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
            if (l.pendingCanonName)
                ++sinNombreCanonico;
        }
        if (l.terrain == "santuario")
            santuarios.push_back(&l);
    });

    std::printf("Egaroth cargado: %d localizaciones\n", static_cast<int>(mundo.size()));
    for (const auto& [tipo, n] : porTipo) {
        std::printf("  %-8s %3d\n", tipo.c_str(), n);
    }

    require(porTipo["nacion"] == 19);
    require(porTipo["ciudad"] == 95);
    require(porTipo["zona"] == 26);
    require(nacionesConPoligono == 19);  // toda nacion tiene forma

    // Una capital concreta, para ver que los campos narrativos llegan enteros.
    const LocationDefinition* bomengrid = nullptr;
    mundo.forEach([&](const LocationDefinition& l) {
        if (l.name == "Bomengrid")
            bomengrid = &l;
    });
    require(bomengrid != nullptr);
    require(bomengrid->settlementSize == "capital");
    require(bomengrid->controlledBy == "Udrax");
    require(bomengrid->hasPosition);
    std::printf("\n%s: %s de %s, gobierna %s, %s habitantes, en (%d,%d)\n", bomengrid->name.c_str(),
                bomengrid->settlementSize.c_str(), bomengrid->controlledBy.c_str(),
                bomengrid->ruler.c_str(), bomengrid->population.c_str(), bomengrid->x,
                bomengrid->y);

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

    // El catalogo cuenta el mundo; WorldTopology comprueba que se puede
    // RECORRER. Boundington tiene una ciudad central conectada a todos sus
    // interiores y cada destino se carga de verdad, no como una ruta de
    // texto sin comprobar.
    auto boundington = WorldTopology::load({"assets/levels/ciudad_centro.json"});
    require(boundington.isOk());
    require(boundington.value().size() >= 15);
    require(boundington.value().links().size() >= 20);
    require(boundington.value().canReach("assets/levels/ciudad_centro.json",
                                         "assets/levels/interior_castillo.json"));
    require(boundington.value().canReach("assets/levels/ciudad_centro.json",
                                         "assets/levels/interior_posada.json"));
    require(boundington.value().canReach("assets/levels/interior_castillo.json",
                                         "assets/levels/ciudad_centro.json"));
    std::printf("\nBoundington conectado: %d niveles y %d puertas transitables\n",
                static_cast<int>(boundington.value().size()),
                static_cast<int>(boundington.value().links().size()));


    // H3-01 — el atlas conectado a LevelTransition: CRUZAR EL MAR.
    //
    // Lo de arriba comprueba que el mundo es un grafo; esto comprueba que se
    // puede JUGAR. El mapamundi es un archipielago: 13 de sus 14 desembarcos
    // no son alcanzables andando desde playerStart (tools/conectividad.py los
    // reporta como aplazados a esta tarea desde el 19/08), asi que sin viaje el
    // jugador llega al mar y solo puede desembarcar en la masa de la que vino.
    auto mar = LevelLoader::loadFromFile("assets/levels/mapamundi.json");
    require(mar.isOk());
    auto viaje = Game::WorldTravelState::open(boundington.value(), mar.value(),
                                              "assets/levels/mapamundi.json");
    require(viaje.isOk());

    Game::WorldTravelState menu = viaje.value();
    std::printf("\nMenu de viaje desde el mapamundi (%d destinos):\n",
                static_cast<int>(menu.destinations().size()));
    const std::vector<std::string> filas = menu.displayLines();
    for (std::size_t i = 0; i < filas.size(); ++i) {
        std::printf("  %s %s\n", i == menu.selected() ? ">" : " ", filas[i].c_str());
    }
    // Las ocho masas que tienen mapa. El canon tiene catorce: las otras seis
    // (desembarco_landmass_9..14) no declaran destino y no se inventan aqui.
    require(menu.destinations().size() == 8);
    // De todas se vuelve: una puerta sin retorno encierra al jugador y carga
    // sin dar ningun error (regla 2 de tools/validar_mundo_unido.py).
    require(menu.diagnostic().empty());

    // La ruta del criterio de cierre, salto a salto.
    std::printf("\nRuta Boundington -> mapamundi -> otra nacion -> vuelta:\n");
    std::printf("  1. ciudad_centro --camino_a_egaroth--> mundi_landmass_1 (andando)\n");
    std::printf("  2. mundi_landmass_1 --regreso_al_mundo--> mapamundi (andando)\n");

    // 3. En el mar se navega: se baja hasta la masa de Ashye/Bosmurg/... y se
    //    confirma. Sale un LevelTransition, que es lo que
    //    Application::applyPendingTransition() ya sabe ejecutar.
    const std::string otraMasa = "assets/levels/mundi_landmass_2.json";
    // Acotado a proposito: menu.moveDown() envuelve, y un destino que no esta
    // convertiria este bucle en un cuelgue dentro de CTest.
    for (std::size_t paso = 0; paso < menu.destinations().size() &&
                               menu.selectedDestination().levelPath != otraMasa; ++paso) {
        menu.moveDown();
    }
    require(menu.selectedDestination().levelPath == otraMasa);
    const LevelTransition aOtraMasa = menu.confirm();
    require(aOtraMasa.pending);
    require(aOtraMasa.levelPath == otraMasa);
    std::printf("  3. mapamundi --VIAJE--> %s\n", aOtraMasa.levelPath.c_str());

    auto otra = LevelLoader::loadFromFile(otraMasa);
    require(otra.isOk());
    bool nacion = false;
    for (const ObjectSpawn& spawn : otra.value().objects) {
        if (spawn.targetLevel == "assets/levels/es_dhin_thyraxion.json") nacion = true;
    }
    require(nacion);
    std::printf("  4. mundi_landmass_2 --acceso_es_dhin_thyraxion--> otra nacion (andando)\n");
    std::printf("  5. ...y de vuelta: regreso_al_mundo -> mapamundi -> VIAJE -> landmass_1\n");

    // 6. Vuelta a casa: desde el mar se elige la masa de Boundington y desde
    //    alli su puerta a la ciudad. Se abre un menu NUEVO (el jugador volvio
    //    a estar en el mar) y se confirma.
    Game::WorldTravelState vuelta = viaje.value();
    const std::string casa = "assets/levels/mundi_landmass_1.json";
    for (std::size_t paso = 0; paso < vuelta.destinations().size() &&
                               vuelta.selectedDestination().levelPath != casa; ++paso) {
        vuelta.moveDown();
    }
    require(vuelta.selectedDestination().levelPath == casa);
    const LevelTransition aCasa = vuelta.confirm();
    require(aCasa.pending);
    require(boundington.value().canReach(aCasa.levelPath, "assets/levels/ciudad_centro.json"));
    std::printf("  6. mapamundi --VIAJE--> mundi_landmass_1 --ciudad_mrnu1agq8t6u--> ciudad_centro\n");

    std::printf("\ntodas las comprobaciones han pasado\n");
    return 0;
}
