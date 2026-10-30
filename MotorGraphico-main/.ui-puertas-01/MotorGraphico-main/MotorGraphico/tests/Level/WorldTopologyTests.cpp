// WorldTopology — recorrido del mundo por las puertas de los niveles.
//
// Existe por un fallo concreto: load() y canReach() recorrian un
// std::vector mientras le hacian push_back, guardando una REFERENCIA al
// elemento en curso. Al crecer, el vector se realocaba y esa referencia
// quedaba colgando en mitad de su propia iteracion. Con pocos niveles no
// se notaba (el vector no llegaba a realocar); con los 93 de Boundington,
// demo_mundo abortaba con std::length_error dentro de basic_string.
//
// La cadena de este test es larga a proposito: tiene que forzar varias
// realocaciones de "pending". Un test de dos niveles habria pasado
// igualmente con el codigo roto.
//
// Los niveles fixture los escribe el propio test en el directorio de
// trabajo y los borra al terminar: sin assets, sin cwd concreto, corre en
// los tres sistemas.

#include "TestHarness.h"

#include "Level/WorldTopology.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace {

constexpr int kChainLength = 64;

// Un HUB con muchas puertas es lo que de verdad destapa el fallo, y la
// razon es sutil: dentro de load(), la ruta del nivel en curso se vuelve a
// leer al construir CADA enlace, y es el push_back de la puerta ANTERIOR
// el que puede realocar. Con una puerta por nivel no hay segunda lectura y
// el codigo roto pasaba el test igual. Boundington tiene una ciudad
// central con decenas de puertas: por eso alli si petaba.
constexpr int kHubDoors = 48;

std::string chainPath(int index) {
    return "wt_nivel_" + std::to_string(index) + ".json";
}

// Nivel n con una puerta al nivel n+1 (el ultimo no tiene salida).
void writeChainLevel(int index, int last) {
    std::ofstream file(chainPath(index), std::ios::binary | std::ios::trunc);
    file << "{\n  \"name\": \"Nivel " << index << "\",\n";
    file << "  \"map\": \"assets/maps/test_map.tmx\",\n";
    file << "  \"playerStart\": {\"x\": 0, \"y\": 0},\n";
    file << "  \"objects\": [";
    if (index < last) {
        file << "\n    {\n      \"objectId\": \"puerta\",\n";
        file << "      \"position\": {\"x\": 1, \"y\": 1},\n";
        file << "      \"targetLevel\": \"" << chainPath(index + 1) << "\",\n";
        file << "      \"targetPosition\": {\"x\": 2, \"y\": 2}\n    }\n  ";
    }
    file << "]\n}\n";
}

std::string hubPath() { return "wt_hub.json"; }
std::string leafPath(int index) { return "wt_hoja_" + std::to_string(index) + ".json"; }

// Un nivel sin salidas.
void writeLeaf(int index) {
    std::ofstream file(leafPath(index), std::ios::binary | std::ios::trunc);
    file << "{\n  \"name\": \"Hoja " << index << "\",\n";
    file << "  \"map\": \"assets/maps/test_map.tmx\",\n";
    file << "  \"playerStart\": {\"x\": 0, \"y\": 0},\n";
    file << "  \"objects\": []\n}\n";
}

// El hub: kHubDoors puertas, cada una a una hoja distinta.
void writeHub() {
    std::ofstream file(hubPath(), std::ios::binary | std::ios::trunc);
    file << "{\n  \"name\": \"Hub\",\n";
    file << "  \"map\": \"assets/maps/test_map.tmx\",\n";
    file << "  \"playerStart\": {\"x\": 0, \"y\": 0},\n";
    file << "  \"objects\": [";
    for (int i = 0; i < kHubDoors; ++i) {
        if (i > 0) file << ",";
        file << "\n    {\n      \"objectId\": \"puerta_" << i << "\",\n";
        file << "      \"position\": {\"x\": " << i << ", \"y\": 0},\n";
        file << "      \"targetLevel\": \"" << leafPath(i) << "\"\n    }";
    }
    file << "\n  ]\n}\n";
}

struct HubFixture {
    HubFixture() {
        writeHub();
        for (int i = 0; i < kHubDoors; ++i) {
            writeLeaf(i);
        }
    }
    ~HubFixture() {
        std::remove(hubPath().c_str());
        for (int i = 0; i < kHubDoors; ++i) {
            std::remove(leafPath(i).c_str());
        }
    }
};

struct ChainFixture {
    ChainFixture() {
        for (int i = 0; i < kChainLength; ++i) {
            writeChainLevel(i, kChainLength - 1);
        }
    }
    ~ChainFixture() {
        for (int i = 0; i < kChainLength; ++i) {
            std::remove(chainPath(i).c_str());
        }
    }
};

}  // namespace

MOTOR_TEST_CASE(a_long_chain_of_levels_is_walked_without_dangling_references) {
    ChainFixture fixture;

    auto loaded = WorldTopology::load({chainPath(0)});
    MOTOR_CHECK(loaded.isOk());

    const WorldTopology& world = loaded.value();
    MOTOR_CHECK(world.size() == static_cast<std::size_t>(kChainLength));
    MOTOR_CHECK(world.links().size() == static_cast<std::size_t>(kChainLength - 1));

    // Las rutas tienen que haber sobrevivido enteras: si alguna referencia
    // colgo, aqui saldrian nombres truncados o basura.
    for (int i = 0; i < kChainLength; ++i) {
        MOTOR_CHECK(world.contains(chainPath(i)));
    }

    // Y el recorrido completo, que es el otro bucle con el mismo patron.
    MOTOR_CHECK(world.canReach(chainPath(0), chainPath(kChainLength - 1)));
    MOTOR_CHECK(world.canReach(chainPath(0), chainPath(0)));
    // Las puertas son de ida: no hay vuelta declarada.
    MOTOR_CHECK(!world.canReach(chainPath(kChainLength - 1), chainPath(0)));
}

// EL caso que destapa el fallo: un nivel con muchas puertas. Cada puerta
// encola su destino (posible realocacion de "pending") y la siguiente
// vuelve a leer la ruta del nivel en curso.
MOTOR_TEST_CASE(a_hub_with_many_doors_keeps_every_source_path_intact) {
    HubFixture fixture;

    auto loaded = WorldTopology::load({hubPath()});
    MOTOR_CHECK(loaded.isOk());

    const WorldTopology& world = loaded.value();
    MOTOR_CHECK(world.size() == static_cast<std::size_t>(kHubDoors + 1));
    MOTOR_CHECK(world.links().size() == static_cast<std::size_t>(kHubDoors));

    // TODOS los enlaces salen del hub. Con la referencia colgante, los
    // posteriores a la primera realocacion traian una ruta corrupta.
    for (const WorldLink& link : world.links()) {
        MOTOR_CHECK(link.sourceLevelPath == hubPath());
    }

    // Y el recorrido, que lee la ruta en curso una vez por enlace.
    for (int i = 0; i < kHubDoors; ++i) {
        MOTOR_CHECK(world.canReach(hubPath(), leafPath(i)));
    }
    MOTOR_CHECK(!world.canReach(leafPath(0), leafPath(1)));
}

MOTOR_TEST_CASE(load_rejects_an_empty_root_list_and_a_missing_level) {
    std::string error;
    auto empty = WorldTopology::load({});
    MOTOR_CHECK(!empty.isOk());

    auto blank = WorldTopology::load({""});
    MOTOR_CHECK(!blank.isOk());

    auto missing = WorldTopology::load({"wt_no_existe_seguro.json"});
    MOTOR_CHECK(!missing.isOk());
    MOTOR_CHECK(missing.errorMessage().find("wt_no_existe_seguro.json") != std::string::npos);
}

// El limite de nodos protege de un manifiesto de contenido corrupto.
MOTOR_TEST_CASE(the_node_limit_stops_an_unbounded_walk) {
    ChainFixture fixture;

    auto capped = WorldTopology::load({chainPath(0)}, /*maxNodes=*/8);
    MOTOR_CHECK(!capped.isOk());
    MOTOR_CHECK(capped.errorMessage().find("limite") != std::string::npos);
}

// Un ciclo (A -> B -> A) no puede colgar el recorrido.
MOTOR_TEST_CASE(a_cycle_between_two_levels_terminates) {
    {
        std::ofstream a("wt_ciclo_a.json", std::ios::binary | std::ios::trunc);
        a << R"({"name":"A","map":"m.tmx","playerStart":{"x":0,"y":0},"objects":[
            {"objectId":"puerta","position":{"x":1,"y":0},"targetLevel":"wt_ciclo_b.json"}]})";
        std::ofstream b("wt_ciclo_b.json", std::ios::binary | std::ios::trunc);
        b << R"({"name":"B","map":"m.tmx","playerStart":{"x":0,"y":0},"objects":[
            {"objectId":"puerta","position":{"x":1,"y":0},"targetLevel":"wt_ciclo_a.json"}]})";
    }

    auto loaded = WorldTopology::load({"wt_ciclo_a.json"});
    MOTOR_CHECK(loaded.isOk());
    MOTOR_CHECK(loaded.value().size() == 2);
    MOTOR_CHECK(loaded.value().canReach("wt_ciclo_a.json", "wt_ciclo_b.json"));
    MOTOR_CHECK(loaded.value().canReach("wt_ciclo_b.json", "wt_ciclo_a.json"));

    std::remove("wt_ciclo_a.json");
    std::remove("wt_ciclo_b.json");
}

int main() { return motor_test::runAllTests(); }
