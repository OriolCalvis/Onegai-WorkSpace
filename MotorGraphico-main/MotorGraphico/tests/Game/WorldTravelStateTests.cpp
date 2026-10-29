// WorldTravelState — H3-01, el atlas conectado a LevelTransition.
//
// Dos mitades, a propósito:
//
//   1. Casos con FIXTURES escritos por el propio test (mismo patrón que
//      tests/Level/WorldTopologyTests.cpp): las reglas de la pieza — orden,
//      entrada, retorno, navegación — sobre mundos de tres niveles donde el
//      resultado esperado se puede escribir a mano.
//
//   2. Casos sobre los ASSETS PUBLICADOS (assets/levels/*.json): el criterio
//      de cierre de H3-01 es una ruta del jugador — «desde Boundington se
//      llega al mapamundi y a otra nación, y se vuelve» —, y esa ruta solo
//      significa algo sobre el mundo real. Con fixtures seguiría en verde el
//      día que alguien borre el desembarco de Aeon.
//
// Los fixtures se borran al terminar; los assets se leen del directorio de
// trabajo (copiar_assets en el .cmake), así que corre en los tres sistemas.

#include "TestHarness.h"

#include "Game/WorldTravelState.h"
#include "Level/LevelLoader.h"
#include "Level/WorldTopology.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

using Game::TravelDestination;
using Game::WorldTravelState;

namespace {

// ---------------------------------------------------------------- fixtures --

struct FixtureDoor {
    std::string objectId;
    std::string target;
    bool hasEntry = false;
    int entryX = 0;
    int entryY = 0;
};

std::string fixturePath(const std::string& name) { return "wts_" + name + ".json"; }

void writeFixture(const std::string& name, const std::string& displayName,
                  const std::vector<FixtureDoor>& doors) {
    const std::string path = fixturePath(name);
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << "{\n  \"name\": \"" << displayName << "\",\n";
    file << "  \"map\": \"assets/maps/test_map.tmx\",\n";
    file << "  \"playerStart\": {\"x\": 0, \"y\": 0},\n";
    file << "  \"objects\": [";
    for (std::size_t i = 0; i < doors.size(); ++i) {
        const FixtureDoor& door = doors[i];
        file << (i == 0 ? "\n" : ",\n");
        file << "    {\"objectId\": \"" << door.objectId << "\", \"position\": {\"x\": 1, \"y\": 1}";
        if (!door.target.empty()) {
            file << ", \"targetLevel\": \"" << door.target << "\"";
            if (door.hasEntry) {
                file << ", \"targetPosition\": {\"x\": " << door.entryX << ", \"y\": " << door.entryY
                     << "}";
            }
        }
        file << "}";
    }
    file << (doors.empty() ? "]\n}\n" : "\n  ]\n}\n");
    file.close();
}

void removeFixture(const std::string& name) { std::remove(fixturePath(name).c_str()); }

LevelDefinition loadFixtureOrDie(const std::string& name) {
    auto loaded = LevelLoader::loadFromFile(fixturePath(name));
    if (!loaded.isOk()) {
        MOTOR_CHECK_FAIL("fixture " + fixturePath(name) + " no carga: " + loaded.errorMessage());
    }
    return loaded.value();
}

const ObjectSpawn* doorOf(const LevelDefinition& level, const std::string& objectId) {
    for (const ObjectSpawn& spawn : level.objects) {
        if (spawn.objectId == objectId) {
            return &spawn;
        }
    }
    return nullptr;
}

// Navega con las teclas de verdad (moveDown) hasta la fila cuyo destino es
// `targetLevelPath` y devuelve lo que el jugador obtendría al confirmar.
// Devuelve una transición vacía si el destino no está en el menú: así el caso
// falla en el MOTOR_CHECK de pending y no desreferencia una fila que no existe.
LevelTransition travelTo(const WorldTravelState& state, const std::string& targetLevelPath) {
    WorldTravelState copy = state;
    for (std::size_t i = 0; i < copy.destinations().size(); ++i) {
        if (copy.destinations()[copy.selected()].levelPath == targetLevelPath) {
            return copy.confirm();
        }
        copy.moveDown();
    }
    return LevelTransition{};
}

std::size_t rowOf(const WorldTravelState& state, const std::string& targetLevelPath) {
    for (std::size_t i = 0; i < state.destinations().size(); ++i) {
        if (state.destinations()[i].levelPath == targetLevelPath) {
            return i;
        }
    }
    return state.destinations().size();
}

// ------------------------------------------------------------ mundo real ----

constexpr const char* kBoundington = "assets/levels/ciudad_centro.json";
constexpr const char* kMapamundi = "assets/levels/mapamundi.json";
constexpr const char* kLandmass1 = "assets/levels/mundi_landmass_1.json";
constexpr const char* kLandmass2 = "assets/levels/mundi_landmass_2.json";
constexpr const char* kNacion = "assets/levels/es_dhin_thyraxion.json";

LevelDefinition loadReal(const std::string& path) {
    auto loaded = LevelLoader::loadFromFile(path);
    if (!loaded.isOk()) {
        MOTOR_CHECK_FAIL("no carga " + path + ": " + loaded.errorMessage());
    }
    return loaded.value();
}

WorldTopology atlasDelMundo() {
    auto loaded = WorldTopology::load({kBoundington});
    if (!loaded.isOk()) {
        MOTOR_CHECK_FAIL(std::string("atlas desde Boundington: ") + loaded.errorMessage());
    }
    return loaded.value();
}

}  // namespace

// ============================================================ reglas (fixtures)

MOTOR_TEST_CASE(open_da_error_si_el_nivel_no_esta_en_el_atlas) {
    writeFixture("aislado", "Aislado", {});
    writeFixture("raiz", "Raiz", {{"puerta", fixturePath("hoja"), false, 0, 0}});
    writeFixture("hoja", "Hoja", {{"vuelta", fixturePath("raiz"), false, 0, 0}});

    auto atlas = WorldTopology::load({fixturePath("raiz")});
    MOTOR_CHECK(atlas.isOk());

    // El atlas se construyó desde "raiz": "aislado" no existe para él.
    const LevelDefinition aislado = loadFixtureOrDie("aislado");
    auto opened = WorldTravelState::open(atlas.value(), aislado, fixturePath("aislado"));
    MOTOR_CHECK(!opened.isOk());
    MOTOR_CHECK(opened.errorMessage().find("no esta en el atlas") != std::string::npos);

    removeFixture("aislado");
    removeFixture("raiz");
    removeFixture("hoja");
}

MOTOR_TEST_CASE(open_da_error_si_el_nivel_no_tiene_salidas) {
    writeFixture("culdesac", "Cul de sac", {});
    writeFixture("raiz2", "Raiz", {{"puerta", fixturePath("culdesac"), false, 0, 0}});

    auto atlas = WorldTopology::load({fixturePath("raiz2")});
    MOTOR_CHECK(atlas.isOk());

    const LevelDefinition culdesac = loadFixtureOrDie("culdesac");
    auto opened = WorldTravelState::open(atlas.value(), culdesac, fixturePath("culdesac"));
    MOTOR_CHECK(!opened.isOk());
    MOTOR_CHECK(opened.errorMessage().find("ninguna salida") != std::string::npos);

    removeFixture("culdesac");
    removeFixture("raiz2");
}

MOTOR_TEST_CASE(los_destinos_salen_por_nombre_no_por_orden_de_fichero) {
    // Las puertas se declaran Z, M, A: el menú tiene que salir A, M, Z, porque
    // una lista que el jugador memoriza no puede bailar cuando el autor
    // reordena el JSON (o cuando gen_mapamundi.py se regenera).
    writeFixture("zzz", "Zubia", {{"vuelta", fixturePath("hub"), false, 0, 0}});
    writeFixture("mmm", "Mistarium", {{"vuelta", fixturePath("hub"), false, 0, 0}});
    writeFixture("aaa", "Alba", {{"vuelta", fixturePath("hub"), false, 0, 0}});
    writeFixture("hub", "Hub", {{"puerta_z", fixturePath("zzz"), false, 0, 0},
                                {"puerta_m", fixturePath("mmm"), false, 0, 0},
                                {"puerta_a", fixturePath("aaa"), false, 0, 0}});

    auto atlas = WorldTopology::load({fixturePath("hub")});
    MOTOR_CHECK(atlas.isOk());
    const LevelDefinition hub = loadFixtureOrDie("hub");
    auto opened = WorldTravelState::open(atlas.value(), hub, fixturePath("hub"));
    MOTOR_CHECK(opened.isOk());

    const WorldTravelState& state = opened.value();
    MOTOR_CHECK(state.destinations().size() == 3);
    MOTOR_CHECK(state.destinations()[0].label == "Alba");
    MOTOR_CHECK(state.destinations()[1].label == "Mistarium");
    MOTOR_CHECK(state.destinations()[2].label == "Zubia");
    MOTOR_CHECK(state.destinations()[0].objectId == "puerta_a");
    MOTOR_CHECK(state.destinations()[2].objectId == "puerta_z");
    MOTOR_CHECK(state.labels().size() == 3);
    MOTOR_CHECK(state.labels()[0] == "Alba");

    for (const std::string& n : {"zzz", "mmm", "aaa", "hub"}) removeFixture(n);
}

MOTOR_TEST_CASE(confirm_lleva_la_entrada_que_declara_la_puerta) {
    writeFixture("destino", "Destino", {{"vuelta", fixturePath("origen"), false, 0, 0}});
    writeFixture("origen", "Origen", {{"puerta", fixturePath("destino"), true, 7, 3}});

    auto atlas = WorldTopology::load({fixturePath("origen")});
    MOTOR_CHECK(atlas.isOk());
    const LevelDefinition origen = loadFixtureOrDie("origen");
    auto opened = WorldTravelState::open(atlas.value(), origen, fixturePath("origen"));
    MOTOR_CHECK(opened.isOk());

    const LevelTransition transition = opened.value().confirm();
    MOTOR_CHECK(transition.pending);
    MOTOR_CHECK(transition.levelPath == fixturePath("destino"));
    MOTOR_CHECK(transition.hasEntryPosition);
    MOTOR_CHECK(transition.entryPosition.x == 7);
    MOTOR_CHECK(transition.entryPosition.y == 3);

    removeFixture("destino");
    removeFixture("origen");
}

MOTOR_TEST_CASE(sin_entrada_declarada_el_destino_decide_donde_se_aterriza) {
    // Sin targetPosition, hasEntryPosition tiene que quedar false: es lo que
    // hace que Application::loadLevel use el playerStart del destino en vez de
    // un (0,0) inventado.
    writeFixture("destino2", "Destino", {{"vuelta", fixturePath("origen2"), false, 0, 0}});
    writeFixture("origen2", "Origen", {{"puerta", fixturePath("destino2"), false, 0, 0}});

    auto atlas = WorldTopology::load({fixturePath("origen2")});
    MOTOR_CHECK(atlas.isOk());
    const LevelDefinition origen = loadFixtureOrDie("origen2");
    auto opened = WorldTravelState::open(atlas.value(), origen, fixturePath("origen2"));
    MOTOR_CHECK(opened.isOk());

    const LevelTransition transition = opened.value().confirm();
    MOTOR_CHECK(transition.pending);
    MOTOR_CHECK(!transition.hasEntryPosition);

    removeFixture("destino2");
    removeFixture("origen2");
}

MOTOR_TEST_CASE(un_destino_sin_vuelta_se_ve_pero_no_se_puede_tomar) {
    // "trampa" no tiene puerta de vuelta: el atlas lo sabe y el menú tiene que
    // decirlo. Es la regla 2 de tools/validar_mundo_unido.py aplicada en la
    // partida — una puerta sin retorno encierra al jugador y no da error.
    writeFixture("trampa", "Sin vuelta", {});
    writeFixture("bueno", "Bueno", {{"vuelta", fixturePath("base"), false, 0, 0}});
    writeFixture("base", "Base", {{"a_trampa", fixturePath("trampa"), false, 0, 0},
                                  {"a_bueno", fixturePath("bueno"), false, 0, 0}});

    auto atlas = WorldTopology::load({fixturePath("base")});
    MOTOR_CHECK(atlas.isOk());
    const LevelDefinition base = loadFixtureOrDie("base");
    auto opened = WorldTravelState::open(atlas.value(), base, fixturePath("base"));
    MOTOR_CHECK(opened.isOk());

    WorldTravelState state = opened.value();
    MOTOR_CHECK(state.destinations().size() == 2);
    // Orden alfabético: "Bueno" primero, "Sin vuelta" después.
    MOTOR_CHECK(state.destinations()[0].label == "Bueno");
    MOTOR_CHECK(state.destinations()[0].hasReturn);
    MOTOR_CHECK(state.destinations()[1].label == "Sin vuelta");
    MOTOR_CHECK(!state.destinations()[1].hasReturn);

    // Se ve, con el reparo escrito en la fila.
    MOTOR_CHECK(state.displayLines()[0] == "Bueno");
    MOTOR_CHECK(state.displayLines()[1] == "Sin vuelta (sin retorno)");
    // Y se explica en el diagnóstico.
    bool explicado = false;
    for (const std::string& line : state.diagnostic()) {
        if (line.find("a_trampa") != std::string::npos && line.find("no se vuelve") != std::string::npos) {
            explicado = true;
        }
    }
    MOTOR_CHECK(explicado);

    // Viajar al bueno sí; a la trampa no.
    state.moveDown();
    MOTOR_CHECK(state.selected() == 1);
    MOTOR_CHECK(!state.confirm().pending);
    state.moveUp();
    MOTOR_CHECK(state.confirm().pending);
    MOTOR_CHECK(state.confirm().levelPath == fixturePath("bueno"));

    for (const std::string& n : {"trampa", "bueno", "base"}) removeFixture(n);
}

MOTOR_TEST_CASE(la_navegacion_envuelve_como_en_el_resto_de_menus) {
    writeFixture("n1", "Uno", {{"vuelta", fixturePath("menu"), false, 0, 0}});
    writeFixture("n2", "Dos", {{"vuelta", fixturePath("menu"), false, 0, 0}});
    writeFixture("n3", "Tres", {{"vuelta", fixturePath("menu"), false, 0, 0}});
    writeFixture("menu", "Menu", {{"a", fixturePath("n1"), false, 0, 0},
                                  {"b", fixturePath("n2"), false, 0, 0},
                                  {"c", fixturePath("n3"), false, 0, 0}});

    auto atlas = WorldTopology::load({fixturePath("menu")});
    MOTOR_CHECK(atlas.isOk());
    const LevelDefinition menu = loadFixtureOrDie("menu");
    auto opened = WorldTravelState::open(atlas.value(), menu, fixturePath("menu"));
    MOTOR_CHECK(opened.isOk());
    WorldTravelState state = opened.value();

    // MenuLogic de producción: wraparound en las dos puntas, igual que en
    // OptionsState. No se duplica la navegación aquí. Las filas salen en orden
    // alfabético (Dos, Tres, Uno), que es la regla 2 y no el orden del JSON.
    MOTOR_CHECK(state.destinations()[0].label == "Dos");
    MOTOR_CHECK(state.destinations()[1].label == "Tres");
    MOTOR_CHECK(state.destinations()[2].label == "Uno");
    MOTOR_CHECK(state.selected() == 0);
    state.moveUp();
    MOTOR_CHECK(state.selected() == 2);
    MOTOR_CHECK(state.selectedDestination().label == "Uno");
    state.moveDown();
    MOTOR_CHECK(state.selected() == 0);
    state.moveDown();
    state.moveDown();
    MOTOR_CHECK(state.selected() == 2);
    MOTOR_CHECK(state.destinations()[state.selected()].label == "Uno");

    for (const std::string& n : {"n1", "n2", "n3", "menu"}) removeFixture(n);
}

MOTOR_TEST_CASE(dos_puertas_al_mismo_sitio_son_dos_destinos) {
    // Boundington tiene puerta_banco y puerta_banco_sur al mismo interior: son
    // dos sitios donde ponerse, no un destino duplicado. No se deduplica.
    writeFixture("comun", "Comun", {{"vuelta", fixturePath("doble"), false, 0, 0}});
    writeFixture("doble", "Doble", {{"norte", fixturePath("comun"), true, 1, 1},
                                    {"sur", fixturePath("comun"), true, 9, 9}});

    auto atlas = WorldTopology::load({fixturePath("doble")});
    MOTOR_CHECK(atlas.isOk());
    const LevelDefinition doble = loadFixtureOrDie("doble");
    auto opened = WorldTravelState::open(atlas.value(), doble, fixturePath("doble"));
    MOTOR_CHECK(opened.isOk());

    const WorldTravelState& state = opened.value();
    MOTOR_CHECK(state.destinations().size() == 2);
    // Mismo label: el desempate por objectId hace el orden determinista.
    MOTOR_CHECK(state.destinations()[0].objectId == "norte");
    MOTOR_CHECK(state.destinations()[1].objectId == "sur");
    MOTOR_CHECK(state.destinations()[0].entryPosition.x == 1);
    MOTOR_CHECK(state.destinations()[1].entryPosition.x == 9);

    removeFixture("comun");
    removeFixture("doble");
}

MOTOR_TEST_CASE(un_estado_vacio_no_viaja) {
    // Result<T> guarda T m_value{} (Result.h:17), así que un WorldTravelState
    // vacío es representable: confirm() y selectedDestination() tienen que
    // aguantarlo sin desreferenciar un vector vacío.
    WorldTravelState state;
    MOTOR_CHECK(state.destinations().empty());
    MOTOR_CHECK(!state.confirm().pending);
    MOTOR_CHECK(state.selectedDestination().levelPath.empty());
    state.moveUp();
    state.moveDown();
    MOTOR_CHECK(state.selected() == 0);
    MOTOR_CHECK(state.displayLines().empty());
}

// ================================================== criterio de cierre (assets)

MOTOR_TEST_CASE(el_mapamundi_ofrece_las_ocho_masas_que_tienen_mapa) {
    const WorldTopology atlas = atlasDelMundo();
    const LevelDefinition mapamundi = loadReal(kMapamundi);

    // El canon tiene CATORCE masas (world_grid.json) y solo ocho tienen mapa
    // de detalle: las otras seis se dibujan con su desembarco pendiente
    // (gen_mapamundi.py). La pieza ofrece las ocho y no inventa las seis.
    int desembarcos = 0;
    int desembarcosSinDestino = 0;
    for (const ObjectSpawn& spawn : mapamundi.objects) {
        if (spawn.objectId.rfind("desembarco_landmass_", 0) != 0) continue;
        ++desembarcos;
        if (spawn.targetLevel.empty()) ++desembarcosSinDestino;
    }
    MOTOR_CHECK(desembarcos == 14);
    MOTOR_CHECK(desembarcosSinDestino == 6);

    auto opened = WorldTravelState::open(atlas, mapamundi, kMapamundi);
    MOTOR_CHECK(opened.isOk());
    const WorldTravelState& state = opened.value();
    MOTOR_CHECK(state.destinations().size() == 8);
    MOTOR_CHECK(state.levelPath() == kMapamundi);

    // Las ocho masas, y cada una con el nombre que el autor le dio.
    for (int n = 1; n <= 8; ++n) {
        const std::string path = "assets/levels/mundi_landmass_" + std::to_string(n) + ".json";
        const std::size_t row = rowOf(state, path);
        if (row == state.destinations().size()) {
            MOTOR_CHECK_FAIL(path + " no se ofrece como destino desde el mapamundi");
        }
        MOTOR_CHECK(!state.destinations()[row].label.empty());
        MOTOR_CHECK(state.destinations()[row].hasReturn);
    }

    // Orden alfabético por nombre: el menú no sigue el orden del JSON.
    for (std::size_t i = 1; i < state.destinations().size(); ++i) {
        MOTOR_CHECK(state.destinations()[i - 1].label <= state.destinations()[i].label);
    }
    // El nombre canónico de la masa 3 es Aeon; si el atlas deja de dar
    // nombres, esto deja de cumplirse y el menú pasaría a enseñar rutas.
    bool aeon = false;
    for (const TravelDestination& destination : state.destinations()) {
        if (destination.label.find("Aeon") != std::string::npos) aeon = true;
    }
    MOTOR_CHECK(aeon);
}

MOTOR_TEST_CASE(ningun_destino_del_mapamundi_deja_al_jugador_encerrado) {
    const WorldTopology atlas = atlasDelMundo();
    const LevelDefinition mapamundi = loadReal(kMapamundi);
    auto opened = WorldTravelState::open(atlas, mapamundi, kMapamundi);
    MOTOR_CHECK(opened.isOk());

    const WorldTravelState& state = opened.value();
    for (const TravelDestination& destination : state.destinations()) {
        if (!destination.hasReturn) {
            MOTOR_CHECK_FAIL("desde " + destination.levelPath + " no se vuelve al mapamundi");
        }
        // Y viajar de verdad a cada una produce una transición ejecutable.
        const LevelTransition transition = travelTo(state, destination.levelPath);
        if (!transition.pending) {
            MOTOR_CHECK_FAIL("no se puede confirmar el viaje a " + destination.levelPath);
        }
        if (transition.levelPath != destination.levelPath) {
            MOTOR_CHECK_FAIL("el viaje a " + destination.levelPath + " lleva a " + transition.levelPath);
        }
    }
    MOTOR_CHECK(state.diagnostic().empty());
}

MOTOR_TEST_CASE(de_boundington_al_mapamundi_a_otra_nacion_y_de_vuelta) {
    // Criterio de cierre de H3-01, jugado entero sobre los assets publicados.
    const WorldTopology atlas = atlasDelMundo();

    // 1. Boundington -> la masa de Egaroth: es una puerta andable, no un viaje.
    const LevelDefinition boundington = loadReal(kBoundington);
    const ObjectSpawn* camino = doorOf(boundington, "camino_a_egaroth");
    MOTOR_CHECK(camino != nullptr);
    MOTOR_CHECK(camino->targetLevel == kLandmass1);

    // 2. Dentro de la masa, el regreso al mar.
    const LevelDefinition landmass1 = loadReal(kLandmass1);
    const ObjectSpawn* regreso = doorOf(landmass1, "regreso_al_mundo");
    MOTOR_CHECK(regreso != nullptr);
    MOTOR_CHECK(regreso->targetLevel == kMapamundi);
    // Y el atlas confirma que la vuelta a Boundington existe.
    MOTOR_CHECK(atlas.canReach(kLandmass1, kBoundington));

    // 3. En el mar: AQUÍ es donde hace falta viajar. Se elige la masa de
    //    Ashye/Bosmurg/... (landmass_2), a la que no se puede andar.
    const LevelDefinition mapamundi = loadReal(kMapamundi);
    auto enElMar = WorldTravelState::open(atlas, mapamundi, kMapamundi);
    MOTOR_CHECK(enElMar.isOk());
    const LevelTransition aOtraMasa = travelTo(enElMar.value(), kLandmass2);
    MOTOR_CHECK(aOtraMasa.pending);
    MOTOR_CHECK(aOtraMasa.levelPath == kLandmass2);

    // 4. En esa masa, una nación distinta de la de salida.
    const LevelDefinition landmass2 = loadReal(kLandmass2);
    const ObjectSpawn* nacion = doorOf(landmass2, "acceso_es_dhin_thyraxion");
    MOTOR_CHECK(nacion != nullptr);
    MOTOR_CHECK(nacion->targetLevel == kNacion);
    const LevelDefinition nacionNivel = loadReal(kNacion);
    MOTOR_CHECK(nacionNivel.name.find("Dhin Thyraxion") != std::string::npos);

    // 5. Y se vuelve: nación -> masa -> mar -> (viaje) -> masa de Boundington
    //    -> Boundington.
    const ObjectSpawn* vueltaNacion = doorOf(nacionNivel, "regreso_al_mundo");
    MOTOR_CHECK(vueltaNacion != nullptr);
    MOTOR_CHECK(vueltaNacion->targetLevel == kLandmass2);

    auto otraVezEnElMar = WorldTravelState::open(atlas, mapamundi, kMapamundi);
    MOTOR_CHECK(otraVezEnElMar.isOk());
    const LevelTransition aCasa = travelTo(otraVezEnElMar.value(), kLandmass1);
    MOTOR_CHECK(aCasa.pending);

    auto enLaMasaDeCasa = WorldTravelState::open(atlas, landmass1, kLandmass1);
    MOTOR_CHECK(enLaMasaDeCasa.isOk());
    const LevelTransition aBoundington = travelTo(enLaMasaDeCasa.value(), kBoundington);
    MOTOR_CHECK(aBoundington.pending);
    MOTOR_CHECK(aBoundington.levelPath == kBoundington);
}

int main() { return motor_test::runAllTests(); }
