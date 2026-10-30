// BLOQUES-01 — test_bloques: que celdas levantan bloque y con que caras.
//
// Un mapa de prueba con dos casas, una puerta y un muro; el plan real
// (assets/textures/bloques.json contra terreno_iso.json) carga y nombra
// los tiles que existen.
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "Game/Bloques.h"
#include "Runtime/RuntimeTileMap.h"
#include "TestHarness.h"

using Game::Bloques;

namespace {

// RuntimeTileMap solo se rellena desde el builder o el adaptador TMX; para
// probar el plan basta con destapar replaceCompiledData.
struct MapaDePrueba : public RuntimeTileMap {
    void rellena(int w, int h, const std::vector<int>& gids) {
        RuntimeTileLayer capa;
        for (int g : gids) capa.tiles.emplace_back(g, g >= 100);
        replaceCompiledData(w, h, 64, 32, {capa});
    }
};

struct CarpetaTemporal {
    std::filesystem::path raiz;
    CarpetaTemporal() {
        raiz = std::filesystem::temp_directory_path() / "bloques_test";
        std::filesystem::remove_all(raiz);
        std::filesystem::create_directories(raiz / "textures");
    }
    ~CarpetaTemporal() { std::filesystem::remove_all(raiz); }
    void escribe(const std::string& rel, const std::string& texto) const { std::ofstream(raiz / rel) << texto; }
    std::string ruta() const { return raiz.string(); }
};

Bloques planDePrueba(const CarpetaTemporal& c) {
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "casa"}, {"gid": 101, "nombre": "muro"}]})");
    c.escribe("textures/bloques.json",
              R"({"hoja": "bloques", "tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json", "umbral": "felpudo",
                  "tipos": {"casa": {"material": "piedra", "bandas": 2},
                            "muro": {"material": "muralla", "bandas": 1, "ventanas": false, "puertas": false}}})");
    Bloques b;
    MOTOR_CHECK(b.carga(c.ruta()).isOk());
    return b;
}

std::string frameEn(const std::vector<Game::BloqueColocado>& plan, int x, int y) {
    for (const auto& b : plan)
        if (b.x == x && b.y == y) return b.frame;
    return "";
}

}  // namespace

MOTOR_TEST_CASE(sin_bloques_json_no_hay_bloques) {
    CarpetaTemporal c;
    Bloques b;
    const auto r = b.carga(c.ruta());
    MOTOR_CHECK(r.isOk() && !r.value() && !b.cargado());
    MapaDePrueba mapa;
    mapa.rellena(2, 2, {100, 100, 100, 100});
    MOTOR_CHECK(b.planifica(mapa, "terreno_mundo", {}).empty());
}

MOTOR_TEST_CASE(caras_expuestas_puertas_y_ventanas) {
    CarpetaTemporal c;
    const Bloques b = planDePrueba(c);
    MOTOR_CHECK(b.cargado() && b.tipos() == 2 && b.hoja() == "bloques" && b.umbral() == "felpudo");
    // Casa de 2x2 en (1..2, 1..2) con la puerta en (1, 3), y un muro suelto en (4, 0).
    MapaDePrueba mapa;
    std::vector<int> g(25, 1);
    for (int y = 1; y <= 2; ++y)
        for (int x = 1; x <= 2; ++x) g[static_cast<std::size_t>(y * 5 + x)] = 100;
    g[4] = 101;
    mapa.rellena(5, 5, g);
    const auto plan = b.planifica(mapa, "terreno_mundo", {GridCoord{1, 3}});
    MOTOR_CHECK(plan.size() == 5);
    // (1,1): ninguna cara expuesta -> lisa.
    MOTOR_CHECK(frameEn(plan, 1, 1) == "piedra2_pp");
    // (1,2): la cara izquierda mira a (1,3), que tiene la puerta; la derecha mira a (2,2), casa.
    MOTOR_CHECK(frameEn(plan, 1, 2) == "piedra2_dp");
    // (2,1): la derecha mira a (3,1), fuera; sin puerta: ventana segun el reparto.
    const char der = Bloques::ventanaDerecha(2, 1) ? 'v' : 'p';
    MOTOR_CHECK(frameEn(plan, 2, 1) == std::string("piedra2_p") + der);
    // (2,2): las dos expuestas, sin puerta.
    const char i22 = Bloques::ventanaIzquierda(2, 2) ? 'v' : 'p';
    const char d22 = Bloques::ventanaDerecha(2, 2) ? 'v' : 'p';
    MOTOR_CHECK(frameEn(plan, 2, 2) == std::string("piedra2_") + i22 + d22);
    // El muro: una banda, nunca ventanas aunque este expuesto.
    MOTOR_CHECK(frameEn(plan, 4, 0) == "muralla1_pp");
    // Solo la puerta de la casa esta en la pared: su celda lleva umbral; la
    // del muro (sin bloque de puerta) sigue siendo una puerta suelta.
    const auto plan2 = b.planifica(mapa, "terreno_mundo", {GridCoord{1, 3}, GridCoord{4, 1}});
    MOTOR_CHECK(frameEn(plan2, 4, 0) == "muralla1_pp");
    const auto umbrales = Game::celdasConPuertaEnPared(plan2);
    MOTOR_CHECK(umbrales.size() == 1 && umbrales.count({1, 3}) == 1 && umbrales.count({4, 1}) == 0);
    for (const auto& bc : plan2)
        if (bc.x == 1 && bc.y == 2) MOTOR_CHECK(bc.izq == 'd' && bc.der == 'p');
    // Otro tileset: nada.
    MOTOR_CHECK(b.planifica(mapa, "otro", {}).empty());
    // El reparto de ventanas es fijo y no uniforme.
    int ventanas = 0;
    for (int i = 0; i < 30; ++i) ventanas += Bloques::ventanaIzquierda(i, 7) ? 1 : 0;
    MOTOR_CHECK(ventanas > 5 && ventanas < 25);
}

MOTOR_TEST_CASE(plan_real_contra_el_tileset) {
    Bloques b;
    const auto r = b.carga("assets");
    MOTOR_CHECK(r.isOk() && r.value() && b.cargado());
    MOTOR_CHECK(b.tipos() >= 15);
    MOTOR_CHECK(b.hoja() == "pixellab_bloques");
    MOTOR_CHECK(b.umbral() == "umbral_puerta");
}

MOTOR_TEST_CASE(plantas_por_edificio_y_puerta_decorativa) {
    CarpetaTemporal c;
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "casa"}, {"gid": 101, "nombre": "muro"}]})");
    c.escribe("textures/bloques.json",
              R"({"hoja": "bloques", "tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json",
                  "tipos": {"casa": {"material": "piedra", "bandas": [1, 2, 3], "pesos": [5, 3, 1]},
                            "muro": {"material": "muralla", "bandas": 1, "ventanas": false, "puertas": false}}})");
    Bloques b;
    MOTOR_CHECK(b.carga(c.ruta()).isOk());
    // Veinte casas de 2x2 en fila (separadas) sobre pradera: las alturas se
    // reparten (hay de 1 y de 2 plantas, no todas iguales) y cada casa sin
    // objeto puerta lleva exactamente una puerta decorativa en una cara que
    // da a pradera; el muro, ninguna.
    MapaDePrueba mapa;
    const int W = 61, H = 6;
    std::vector<int> g(static_cast<std::size_t>(W * H), 1);
    for (int i = 0; i < 20; ++i)
        for (int y = 1; y <= 2; ++y)
            for (int x = 1 + i * 3; x <= 2 + i * 3; ++x) g[static_cast<std::size_t>(y * W + x)] = 100;
    g[static_cast<std::size_t>(4 * W + 5)] = 101;
    mapa.rellena(W, H, g);
    const auto plan = b.planifica(mapa, "terreno_mundo", {});
    MOTOR_CHECK(plan.size() == 81);
    std::set<int> plantas;
    int puertas = 0;
    for (const auto& bc : plan) {
        if (bc.frame.rfind("piedra", 0) == 0) plantas.insert(bc.frame[6] - '0');
        if (bc.frame.rfind("muralla", 0) == 0) MOTOR_CHECK(bc.izq != 'd' && bc.der != 'd');
        puertas += (bc.izq == 'd' ? 1 : 0) + (bc.der == 'd' ? 1 : 0);
    }
    MOTOR_CHECK(plantas.size() >= 2 && plantas.count(1) == 1);
    MOTOR_CHECK(puertas == 20);
    // Misma entrada, mismo plan (determinista).
    const auto plan2 = b.planifica(mapa, "terreno_mundo", {});
    MOTOR_CHECK(plan2.size() == plan.size() && plan2[7].frame == plan[7].frame);
    // Con una puerta de verdad delante de la primera casa, esa casa no anade decorativa.
    const auto plan3 = b.planifica(mapa, "terreno_mundo", {GridCoord{1, 3}});
    int puertasCasa1 = 0;
    for (const auto& bc : plan3)
        if (bc.x <= 2 && bc.y <= 2) puertasCasa1 += (bc.izq == 'd' ? 1 : 0) + (bc.der == 'd' ? 1 : 0);
    MOTOR_CHECK(puertasCasa1 == 1);
}

MOTOR_TEST_CASE(indice_roto_es_error) {
    CarpetaTemporal c;
    c.escribe("textures/terreno_iso.json", R"({"tiles": [{"gid": 1, "nombre": "pradera"}]})");
    c.escribe("textures/bloques.json",
              R"({"hoja": "bloques", "tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json",
                  "tipos": {"no_existe": {"material": "piedra"}}})");
    Bloques b;
    MOTOR_CHECK(!b.carga(c.ruta()).isOk() && !b.cargado());
}

int main() { return motor_test::runAllTests(); }
