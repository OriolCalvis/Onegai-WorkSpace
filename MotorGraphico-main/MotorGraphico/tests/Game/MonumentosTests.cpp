// BLOQUES-03 — test_monumentos: que huellas son un monumento y como se
// reparten las tiras de su hoja entre las celdas de delante.
#include <cmath>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "Game/Monumentos.h"
#include "Runtime/RuntimeTileMap.h"
#include "TestHarness.h"

using Game::Monumentos;

namespace {

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
        raiz = std::filesystem::temp_directory_path() / "monumentos_test";
        std::filesystem::remove_all(raiz);
        std::filesystem::create_directories(raiz / "textures");
    }
    ~CarpetaTemporal() { std::filesystem::remove_all(raiz); }
    void escribe(const std::string& rel, const std::string& texto) const { std::ofstream(raiz / rel) << texto; }
    std::string ruta() const { return raiz.string(); }
};

// Un coliseo de 4x3 celdas: hoja de 7 tiras de 32 (224 px = (4+3)*32) por 160 de alto,
// con el anclaje en la esquina delantera (x = 224/2 = 112, y = 150).
Monumentos planDePrueba(const CarpetaTemporal& c) {
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "coliseo"}]})");
    c.escribe("textures/monumentos.json",
              R"({"tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json",
                  "monumentos": [{"id": "coliseo", "hoja": "monumento_coliseo", "tiles": ["coliseo"], "ancho": 4, "alto": 3,
                                  "tiraAncho": 32, "altoHoja": 160, "columnas": 7, "anclaje": {"x": 112, "y": 150}}]})");
    Monumentos m;
    MOTOR_CHECK(m.carga(c.ruta()).isOk());
    return m;
}

}  // namespace

MOTOR_TEST_CASE(sin_monumentos_json_no_hay_monumentos) {
    CarpetaTemporal c;
    Monumentos m;
    const auto r = m.carga(c.ruta());
    MOTOR_CHECK(r.isOk() && !r.value() && !m.cargado());
}

MOTOR_TEST_CASE(cada_tira_cae_en_la_celda_mas_delantera_de_su_columna) {
    // Huella 4x3, 7 tiras: la esquina delantera es la celda (3, 2). Las tiras
    // de la mitad izquierda caen en la fila de delante (j = 2) y las de la
    // derecha en la columna de delante (i = 3); ninguna en el interior.
    std::set<std::pair<int, int>> celdas;
    for (int k = 0; k < 7; ++k) {
        const auto [i, j] = Monumentos::celdaDeTira(4, 3, k, 7, 112, 32);
        MOTOR_CHECK(i >= 0 && i < 4 && j >= 0 && j < 3);
        MOTOR_CHECK(i == 3 || j == 2);
        celdas.insert({i, j});
    }
    MOTOR_CHECK(celdas.count({3, 2}) == 1);
    MOTOR_CHECK(celdas.count({0, 2}) == 1 && celdas.count({3, 0}) == 1);
}

MOTOR_TEST_CASE(solo_las_huellas_que_miden_lo_que_dice_el_monumento) {
    CarpetaTemporal c;
    const Monumentos m = planDePrueba(c);
    MOTOR_CHECK(m.cargado() && m.cuantos() == 1);
    MapaDePrueba mapa;
    std::vector<int> g(12 * 12, 1);
    // Coliseo bueno de 4x3 en (1..4, 1..3) con el umbral de la puerta sin tile (3, 3).
    for (int y = 1; y <= 3; ++y)
        for (int x = 1; x <= 4; ++x) g[static_cast<std::size_t>(y * 12 + x)] = 100;
    g[static_cast<std::size_t>(3 * 12 + 3)] = 1;
    // Otro «coliseo» de 2x2 en (8..9, 8..9): no mide 4x3, se queda en bloques.
    for (int y = 8; y <= 9; ++y)
        for (int x = 8; x <= 9; ++x) g[static_cast<std::size_t>(y * 12 + x)] = 100;
    mapa.rellena(12, 12, g);
    const auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 1);
    MOTOR_CHECK(plan[0].id == "coliseo" && plan[0].x0 == 1 && plan[0].y0 == 1 && plan[0].tiras.size() == 7);
    for (const auto& t : plan[0].tiras) {
        MOTOR_CHECK(t.x >= 1 && t.x <= 4 && t.y >= 1 && t.y <= 3);
        MOTOR_CHECK(t.frame >= 1 && t.frame <= 7);
    }
    // El desplazamiento deja el anclaje de la hoja en la esquina delantera:
    // la tira 0 empieza en x = esquina - 112 respecto al origen de su celda.
    const auto& t0 = plan[0].tiras[0];
    MOTOR_CHECK(t0.dx == static_cast<float>((4 - 3) * 32 + 32 - 112 - (t0.x - 1 - (t0.y - 1)) * 32));
    const auto cubiertas = Monumentos::celdasCubiertas(plan);
    MOTOR_CHECK(cubiertas.size() == 12 && cubiertas.count({1, 1}) == 1 && cubiertas.count({4, 3}) == 1 && cubiertas.count({8, 8}) == 0);
    MOTOR_CHECK(m.planifica(mapa, "otro").empty());
}

MOTOR_TEST_CASE(varias_definiciones_del_mismo_tile_por_tamano_y_variante) {
    // ARTE-03: casas de 2x2 con dos variantes (a, b) y de 3x2 con una; el
    // tile es el mismo. Cada huella toma la definicion de su tamano, y la
    // variante sale de un hash de la posicion: estable y repartida.
    CarpetaTemporal c;
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "casa"}]})");
    c.escribe("textures/monumentos.json",
              R"({"tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json", "monumentos": [
                  {"id": "casa_2x2_a", "hoja": "m_a", "tiles": ["casa"], "ancho": 2, "alto": 2, "tiraAncho": 32, "altoHoja": 96, "columnas": 4, "anclaje": {"x": 64, "y": 90}},
                  {"id": "casa_2x2_b", "hoja": "m_b", "tiles": ["casa"], "ancho": 2, "alto": 2, "tiraAncho": 32, "altoHoja": 96, "columnas": 4, "anclaje": {"x": 64, "y": 90}},
                  {"id": "casa_3x2", "hoja": "m_c", "tiles": ["casa"], "ancho": 3, "alto": 2, "tiraAncho": 32, "altoHoja": 96, "columnas": 5, "anclaje": {"x": 80, "y": 90}}]})");
    Monumentos m;
    MOTOR_CHECK(m.carga(c.ruta()).isOk() && m.cuantos() == 3);
    MapaDePrueba mapa;
    std::vector<int> g(20 * 20, 1);
    // Ocho casas de 2x2 separadas por una celda y una de 3x2; y una de 4x4 que no tiene definicion.
    std::vector<std::pair<int, int>> casas;
    for (int k = 0; k < 8; ++k) casas.push_back({1 + (k % 4) * 3, 1 + (k / 4) * 3});
    for (const auto [cx, cy] : casas)
        for (int y = cy; y < cy + 2; ++y)
            for (int x = cx; x < cx + 2; ++x) g[static_cast<std::size_t>(y * 20 + x)] = 100;
    for (int y = 10; y < 12; ++y)
        for (int x = 1; x < 4; ++x) g[static_cast<std::size_t>(y * 20 + x)] = 100;
    for (int y = 14; y < 18; ++y)
        for (int x = 14; x < 18; ++x) g[static_cast<std::size_t>(y * 20 + x)] = 100;
    mapa.rellena(20, 20, g);
    const auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 9);
    int a = 0, b = 0, c3 = 0;
    for (const auto& p : plan) {
        if (p.id == "casa_2x2_a") ++a;
        if (p.id == "casa_2x2_b") ++b;
        if (p.id == "casa_3x2") ++c3;
        MOTOR_CHECK(p.tiras.size() == static_cast<std::size_t>(p.ancho == 3 ? 5 : 4));
    }
    MOTOR_CHECK(a + b == 8 && a > 0 && b > 0 && c3 == 1);
    // Determinista: el mismo mapa da las mismas variantes.
    const auto plan2 = m.planifica(mapa, "terreno_mundo");
    for (std::size_t i = 0; i < plan.size(); ++i) MOTOR_CHECK(plan[i].id == plan2[i].id && plan[i].x0 == plan2[i].x0);
}

MOTOR_TEST_CASE(els_posaders_del_teulat_van_amb_la_casa) {
    // MASC-01: els coloms es posen al carener de les peces del kit. Els
    // posaders del manifest van des de la cantonada de la huella; al pla,
    // ja en cel·les del mapa.
    CarpetaTemporal c;
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "casa"}]})");
    c.escribe("textures/monumentos.json",
              R"({"tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json",
                  "monumentos": [{"id": "casa", "hoja": "monumento_casa", "tiles": ["casa"], "ancho": 3, "alto": 2,
                                  "tiraAncho": 32, "altoHoja": 120, "columnas": 5, "anclaje": {"x": 96, "y": 110},
                                  "posaderos": [[0.6, 1.0, 92.5], [2.4, 1.0, 92.5], [7]]}]})");
    Monumentos m;
    MOTOR_CHECK(m.carga(c.ruta()).isOk());
    MapaDePrueba mapa;
    std::vector<int> g(10 * 10, 1);
    for (int y = 4; y < 6; ++y)
        for (int x = 2; x < 5; ++x) g[static_cast<std::size_t>(y * 10 + x)] = 100;
    mapa.rellena(10, 10, g);
    const auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 1);
    MOTOR_CHECK(plan[0].posaderos.size() == 2);  // el que no te tres valors no es un posador
    MOTOR_CHECK(std::abs(plan[0].posaderos[0].x - 2.6f) < 1e-4f && std::abs(plan[0].posaderos[0].y - 5.0f) < 1e-4f);
    MOTOR_CHECK(std::abs(plan[0].posaderos[1].altura - 92.5f) < 1e-4f);
}

int main() { return motor_test::runAllTests(); }
