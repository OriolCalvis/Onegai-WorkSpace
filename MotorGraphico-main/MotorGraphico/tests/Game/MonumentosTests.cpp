// BLOQUES-03 — test_monumentos: que huellas son un monumento y como se
// reparten las tiras de su hoja entre las celdas de delante.
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "Game/Bloques.h"
#include "Game/Monumentos.h"
#include "Render/TileMap.h"
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

MOTOR_TEST_CASE(una_fila_de_mas_por_detras_sigue_siendo_el_monumento) {
    // Oriol (22/10): la manzana del Excel mide una fila mas que el sprite (el castillo 13x11 con un sprite
    // de 13x10). El monumento se pone alineado por delante; la fila de detras (J-2: en las casas) levanta bloques.
    CarpetaTemporal c;
    const Monumentos m = planDePrueba(c);
    MapaDePrueba mapa;
    std::vector<int> g(12 * 12, 1);
    for (int y = 1; y <= 4; ++y)  // 4x4: una fila de mas
        for (int x = 1; x <= 4; ++x) g[static_cast<std::size_t>(y * 12 + x)] = 100;
    mapa.rellena(12, 12, g);
    const auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 1);
    MOTOR_CHECK(plan[0].x0 == 1 && plan[0].y0 == 2 && plan[0].ancho == 4 && plan[0].alto == 3);
    for (const auto& t : plan[0].tiras) MOTOR_CHECK(t.y >= 2 && t.y <= 4);
    const auto cubiertas = Monumentos::celdasCubiertas(plan);
    // J-2: una casa o un local (menos de 30 casillas) no tiene patio: la fila de detras conserva su colision
    // y se dibuja con bloques, no es una pared invisible. Solo el sprite (4x3) queda cubierto.
    MOTOR_CHECK(cubiertas.size() == 12 && cubiertas.count({1, 1}) == 0 && cubiertas.count({1, 2}) == 1);
    // Dos de mas ya no (un monumento pequeño: una casa o un local): se queda en bloques.
    for (int x = 1; x <= 4; ++x) g[static_cast<std::size_t>(5 * 12 + x)] = 100;
    mapa.rellena(12, 12, g);
    MOTOR_CHECK(m.planifica(mapa, "terreno_mundo").empty());
}

MOTOR_TEST_CASE(un_monumento_grande_admite_tres_filas_de_patio) {
    // Oriol (22/10): pinto el coliseo de 8x9 y el dibujo mide 8x6. Un monumento de 30 casillas o mas se pone
    // delante y las tres filas de detras son su patio (sin bloques); con cuatro, ya no.
    CarpetaTemporal c;
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "arena"}]})");
    c.escribe("textures/monumentos.json",
              R"({"tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json", "monumentos": [
                  {"id": "coliseo", "hoja": "monumento_coliseo", "tiles": ["arena"], "ancho": 8, "alto": 6,
                   "tiraAncho": 32, "altoHoja": 270, "columnas": 10, "anclaje": {"x": 192, "y": 306}}]})");
    Monumentos m;
    MOTOR_CHECK(m.carga(c.ruta()).isOk());
    MapaDePrueba mapa;
    std::vector<int> g(14 * 14, 1);
    for (int y = 1; y <= 9; ++y)
        for (int x = 1; x <= 8; ++x) g[static_cast<std::size_t>(y * 14 + x)] = 100;
    mapa.rellena(14, 14, g);
    auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 1);
    MOTOR_CHECK(plan.size() == 1 && plan[0].x0 == 1 && plan[0].y0 == 4 && plan[0].alto == 6);
    MOTOR_CHECK(Monumentos::celdasCubiertas(plan).size() == 72);
    for (int x = 1; x <= 8; ++x) g[static_cast<std::size_t>(10 * 14 + x)] = 100;
    mapa.rellena(14, 14, g);
    MOTOR_CHECK(m.planifica(mapa, "terreno_mundo").empty());
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

// ---------------------------------------------------------------------------
// H-CIU-2 (E-CIU-3): un edificio es su familia entera, no un tile.
// ---------------------------------------------------------------------------
namespace {

// Dos orientaciones de la misma familia («casa_o0», «casa_o3») con su definicion de cada una.
Monumentos familiaDePrueba(const CarpetaTemporal& c) {
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "casa_o0"}, {"gid": 103, "nombre": "casa_o3"}]})");
    c.escribe("textures/monumentos.json",
              R"({"tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json", "monumentos": [
                  {"id": "casa_2x3_o0", "hoja": "m_a", "tiles": ["casa_o0"], "ancho": 2, "alto": 3, "tiraAncho": 32, "altoHoja": 96, "columnas": 5, "anclaje": {"x": 64, "y": 90}},
                  {"id": "casa_3x2_o3", "hoja": "m_b", "tiles": ["casa_o3"], "ancho": 3, "alto": 2, "tiraAncho": 32, "altoHoja": 96, "columnas": 5, "anclaje": {"x": 96, "y": 90}},
                  {"id": "casa_2x3_o3", "hoja": "m_c", "tiles": ["casa_o3"], "ancho": 2, "alto": 3, "tiraAncho": 32, "altoHoja": 96, "columnas": 5, "anclaje": {"x": 64, "y": 90}}]})");
    Monumentos m;
    MOTOR_CHECK(m.carga(c.ruta()).isOk());
    return m;
}

}  // namespace

MOTOR_TEST_CASE(una_casa_con_dos_orientaciones_recibe_su_sprite_entero) {
    CarpetaTemporal c;
    const Monumentos m = familiaDePrueba(c);
    MapaDePrueba mapa;
    std::vector<int> g(12 * 12, 1);
    // Casa 2x3 en (2..3, 2..4): el cuerpo es o3 (4 casillas) y la fila de arriba, pintada despues, es o0.
    for (int y = 2; y <= 4; ++y)
        for (int x = 2; x <= 3; ++x) g[static_cast<std::size_t>(y * 12 + x)] = (y == 2) ? 100 : 103;
    // Otra casa suelta de una sola orientacion en (7..8, 7..9): sigue siendo una casa.
    for (int y = 7; y <= 9; ++y)
        for (int x = 7; x <= 8; ++x) g[static_cast<std::size_t>(y * 12 + x)] = 100;
    mapa.rellena(12, 12, g);
    const auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 2);
    const auto* mezclada = plan.size() == 2 ? (plan[0].cubreX0 == 2 ? &plan[0] : &plan[1]) : nullptr;
    MOTOR_CHECK(mezclada != nullptr);
    if (mezclada != nullptr) {
        // gana la orientacion con mas casillas (o3: 4 contra 2) y la casa se cubre entera
        MOTOR_CHECK(mezclada->id == "casa_2x3_o3");
        MOTOR_CHECK(mezclada->cubreX0 == 2 && mezclada->cubreY0 == 2 && mezclada->cubreAncho == 2 && mezclada->cubreAlto == 3);
        MOTOR_CHECK(mezclada->x0 == 2 && mezclada->y0 == 2 && mezclada->tiras.size() == 5);
    }
    const auto cubiertas = Monumentos::celdasCubiertas(plan);
    MOTOR_CHECK(cubiertas.size() == 12 && cubiertas.count({2, 2}) == 1 && cubiertas.count({3, 4}) == 1 && cubiertas.count({7, 7}) == 1);
}

MOTOR_TEST_CASE(con_empate_de_orientaciones_gana_la_menor) {
    CarpetaTemporal c;
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "casa_o0"}, {"gid": 103, "nombre": "casa_o3"}]})");
    c.escribe("textures/monumentos.json",
              R"({"tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json", "monumentos": [
                  {"id": "casa_2x2_o0", "hoja": "m_a", "tiles": ["casa_o0"], "ancho": 2, "alto": 2, "tiraAncho": 32, "altoHoja": 96, "columnas": 4, "anclaje": {"x": 64, "y": 90}},
                  {"id": "casa_2x2_o3", "hoja": "m_b", "tiles": ["casa_o3"], "ancho": 2, "alto": 2, "tiraAncho": 32, "altoHoja": 96, "columnas": 4, "anclaje": {"x": 64, "y": 90}}]})");
    Monumentos m;
    MOTOR_CHECK(m.carga(c.ruta()).isOk());
    MapaDePrueba mapa;
    std::vector<int> g(8 * 8, 1);
    for (int y = 2; y <= 3; ++y)
        for (int x = 2; x <= 3; ++x) g[static_cast<std::size_t>(y * 8 + x)] = (x == 2) ? 103 : 100;  // 2 y 2
    mapa.rellena(8, 8, g);
    const auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 1 && plan[0].id == "casa_2x2_o0");
}

MOTOR_TEST_CASE(dos_casas_pegadas_que_no_forman_una_sola_siguen_siendo_dos) {
    // Dos casas 2x2 de orientaciones distintas, pegadas: la union (4x2) no es ningun edificio, y cada una
    // sigue recibiendo su sprite como antes de unir las orientaciones.
    CarpetaTemporal c;
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "casa_o0"}, {"gid": 103, "nombre": "casa_o3"}]})");
    c.escribe("textures/monumentos.json",
              R"({"tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json", "monumentos": [
                  {"id": "casa_2x2_o0", "hoja": "m_a", "tiles": ["casa_o0"], "ancho": 2, "alto": 2, "tiraAncho": 32, "altoHoja": 96, "columnas": 4, "anclaje": {"x": 64, "y": 90}},
                  {"id": "casa_2x2_o3", "hoja": "m_b", "tiles": ["casa_o3"], "ancho": 2, "alto": 2, "tiraAncho": 32, "altoHoja": 96, "columnas": 4, "anclaje": {"x": 64, "y": 90}}]})");
    Monumentos m;
    MOTOR_CHECK(m.carga(c.ruta()).isOk());
    MapaDePrueba mapa;
    std::vector<int> g(10 * 10, 1);
    for (int y = 2; y <= 3; ++y)
        for (int x = 2; x <= 5; ++x) g[static_cast<std::size_t>(y * 10 + x)] = (x <= 3) ? 100 : 103;
    mapa.rellena(10, 10, g);
    const auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 2);
    MOTOR_CHECK(Monumentos::celdasCubiertas(plan).size() == 8);
}

// ---------------------------------------------------------------------------
// H-CIU-2 (E-CIU-5): las casillas de la planta que el sprite no ocupa se pisan.
// ---------------------------------------------------------------------------
MOTOR_TEST_CASE(las_casillas_sobrantes_de_la_planta_dejan_de_bloquear) {
    CarpetaTemporal c;
    c.escribe("textures/terreno_iso.json",
              R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "ruina"}]})");
    // Sprite de 4x3 cuya base no ocupa dos esquinas de atras: (0,0) y (3,0).
    c.escribe("textures/monumentos.json",
              R"({"tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json",
                  "monumentos": [{"id": "ruina", "hoja": "monumento_ruina", "tiles": ["ruina"], "ancho": 4, "alto": 3,
                                  "tiraAncho": 32, "altoHoja": 160, "columnas": 7, "anclaje": {"x": 112, "y": 150},
                                  "libres": [[0, 0], [3, 0], [1]]}]})");
    Monumentos m;
    MOTOR_CHECK(m.carga(c.ruta()).isOk());
    MapaDePrueba mapa;
    std::vector<int> g(12 * 12, 1);
    for (int y = 1; y <= 3; ++y)  // la planta pintada es mas ancha que el sprite: 5x3 (una columna de mas por detras)
        for (int x = 1; x <= 5; ++x) g[static_cast<std::size_t>(y * 12 + x)] = 100;
    mapa.rellena(12, 12, g);
    const auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 1);
    if (plan.size() != 1) return;
    // alineado por delante: el sprite empieza en x = 2; libres = (2,1), (5,1) y la columna de mas x = 1 (3 casillas)
    MOTOR_CHECK(plan[0].x0 == 2 && plan[0].y0 == 1 && plan[0].libres.size() == 5);
    const std::set<std::pair<int, int>> libres(plan[0].libres.begin(), plan[0].libres.end());
    MOTOR_CHECK(libres.count({2, 1}) == 1 && libres.count({5, 1}) == 1 && libres.count({1, 1}) == 1 &&
                libres.count({1, 3}) == 1 && libres.count({3, 1}) == 0);
    int bloqueaban = 0;
    for (int y = 1; y <= 3; ++y)
        for (int x = 1; x <= 5; ++x) bloqueaban += mapa.getTile(0, x, y).hasCollision() ? 1 : 0;
    MOTOR_CHECK(bloqueaban == 15);
    MOTOR_CHECK(Monumentos::liberaCeldas(mapa, plan) == 5);
    int bloquean = 0;
    for (int y = 1; y <= 3; ++y)
        for (int x = 1; x <= 5; ++x) {
            const bool b = mapa.getTile(0, x, y).hasCollision();
            bloquean += b ? 1 : 0;
            MOTOR_CHECK(b == (libres.count({x, y}) == 0));
            MOTOR_CHECK(mapa.getTile(0, x, y).getTilesetID() == 100);  // el dibujo no cambia
        }
    MOTOR_CHECK(bloquean == 10);
    MOTOR_CHECK(Monumentos::liberaCeldas(mapa, plan) == 0);  // idempotente
}

MOTOR_TEST_CASE(un_monumento_sin_libres_no_libera_nada) {
    CarpetaTemporal c;
    const Monumentos m = planDePrueba(c);
    MapaDePrueba mapa;
    std::vector<int> g(12 * 12, 1);
    for (int y = 1; y <= 4; ++y)
        for (int x = 1; x <= 4; ++x) g[static_cast<std::size_t>(y * 12 + x)] = 100;
    mapa.rellena(12, 12, g);
    auto plan = m.planifica(mapa, "terreno_mundo");
    MOTOR_CHECK(plan.size() == 1 && plan[0].libres.empty());
    MOTOR_CHECK(Monumentos::liberaCeldas(mapa, plan) == 0);
}

// ---------------------------------------------------------------------------
// ciudad_centro de verdad (assets del build; mismo directorio de trabajo que el resto de pruebas).
// ---------------------------------------------------------------------------
namespace {

const Game::MonumentoColocado* buscaMonumento(const std::vector<Game::MonumentoColocado>& plan, const std::string& id) {
    for (const auto& p : plan)
        if (p.id == id) return &p;
    return nullptr;
}

bool cargaCiudad(const std::string& mapa, TileMap& tm, Monumentos& m) {
    return tm.loadFromFile("assets/maps/" + mapa).isOk() && m.carga("assets").isOk() && m.cargado();
}

}  // namespace

MOTOR_TEST_CASE(ciudad_centro_las_seis_casas_partidas_salen_con_sprite_entero) {
    TileMap tm;
    Monumentos m;
    MOTOR_CHECK(cargaCiudad("ciudad_centro.tmx", tm, m));
    const auto plan = m.planifica(tm, tm.getTilesetName());
    const auto cubiertas = Monumentos::celdasCubiertas(plan);
    // origen y medida de cada casa (H-CIU-0 §4.3, casos 1-6)
    struct Casa { int x, y, w, h; };
    const Casa casas[] = {{11, 21, 2, 3}, {14, 21, 2, 3}, {35, 21, 2, 3}, {49, 21, 2, 3}, {45, 43, 3, 3}, {40, 52, 3, 3}};
    for (const Casa& c : casas) {
        int enteras = 0;
        for (const auto& p : plan)
            if (p.id.rfind("iso_edifici_", 0) == 0 && p.cubreX0 == c.x && p.cubreY0 == c.y && p.cubreAncho == c.w && p.cubreAlto == c.h)
                ++enteras;
        MOTOR_CHECK(enteras == 1);
        for (int y = c.y; y < c.y + c.h; ++y)
            for (int x = c.x; x < c.x + c.w; ++x) MOTOR_CHECK(cubiertas.count({x, y}) == 1);  // ninguna casilla a bloques
    }
}

MOTOR_TEST_CASE(ciudad_centro_ocaso_las_casas_de_tres_por_tres_salen_enteras) {
    TileMap tm;
    Monumentos m;
    MOTOR_CHECK(cargaCiudad("ciudad_centro_ocaso.tmx", tm, m));
    const auto plan = m.planifica(tm, tm.getTilesetName());
    int casas3x3 = 0;
    for (const auto& p : plan)
        if (p.id.rfind("iso_edifici_3x3", 0) == 0 && ((p.cubreX0 == 72 && p.cubreY0 == 52) || (p.cubreX0 >= 76 && p.cubreY0 == 43)))
            ++casas3x3;
    MOTOR_CHECK(casas3x3 == 2);
}

MOTOR_TEST_CASE(ciudad_centro_ocaso_las_dos_casillas_sueltas_de_la_casa_levantan_bloques_y_no_hay_paredes_invisibles) {
    // J-2: la casa 3x3 (77..79, 43..45) esta en una mancha de 4x3 (76..79); (76,43) y (76,45) son edificio con
    // colision que el sprite no pisa. No hay sprite de 4x3: se dibujan con bloques del material de la casa.
    TileMap tm;
    Monumentos m;
    Game::Bloques b;
    MOTOR_CHECK(cargaCiudad("ciudad_centro_ocaso.tmx", tm, m));
    MOTOR_CHECK(b.carga("assets").isOk() && b.cargado());
    const auto plan = m.planifica(tm, tm.getTilesetName());
    const Game::MonumentoColocado* casa = nullptr;  // el id se repite en el nivel: se busca por posicion
    for (const auto& p : plan)
        if (p.id.rfind("iso_edifici_3x3", 0) == 0 && p.x0 == 77 && p.y0 == 43) casa = &p;
    MOTOR_CHECK(casa != nullptr);
    if (casa == nullptr) return;
    MOTOR_CHECK(casa->x0 == 77 && casa->y0 == 43 && casa->cubreX0 == 76 && casa->cubreAncho == 4 && casa->libres.empty());
    Monumentos::liberaCeldas(tm, plan);
    const auto cubiertas = Monumentos::celdasCubiertas(plan);
    MOTOR_CHECK(cubiertas.count({76, 43}) == 0 && cubiertas.count({76, 45}) == 0 && cubiertas.count({77, 43}) == 1);
    MOTOR_CHECK(tm.getTile(0, 76, 43).hasCollision() && tm.getTile(0, 76, 45).hasCollision());
    std::set<std::pair<int, int>> conBloque;
    for (const auto& bl : b.planifica(tm, tm.getTilesetName(), {}, &cubiertas)) conBloque.insert({bl.x, bl.y});
    MOTOR_CHECK(conBloque.count({76, 43}) == 1 && conBloque.count({76, 45}) == 1);
    // Ninguna casilla de edificio con colision se queda sin dibujo: o la pisa un sprite o lleva bloque.
    std::set<std::pair<int, int>> pisadas;
    for (const auto& p : plan)
        for (int j = 0; j < p.alto; ++j)
            for (int i = 0; i < p.ancho; ++i) pisadas.insert({p.x0 + i, p.y0 + j});
    int invisibles = 0;
    for (const auto& bl : b.planifica(tm, tm.getTilesetName(), {}, nullptr)) {
        const bool dibujada = pisadas.count({bl.x, bl.y}) == 1 || conBloque.count({bl.x, bl.y}) == 1;
        if (tm.getTile(0, bl.x, bl.y).hasCollision() && !dibujada) ++invisibles;
    }
    MOTOR_CHECK(invisibles == 0);
}

MOTOR_TEST_CASE(ciudad_centro_la_iglesia_lleva_sprite_y_sus_39_casillas_sobrantes_se_pisan) {
    TileMap tm;
    Monumentos m;
    MOTOR_CHECK(cargaCiudad("ciudad_centro.tmx", tm, m));
    const auto plan = m.planifica(tm, tm.getTilesetName());
    const auto* iglesia = buscaMonumento(plan, "iso_mon_iglesia");
    MOTOR_CHECK(iglesia != nullptr);
    if (iglesia == nullptr) return;
    MOTOR_CHECK(iglesia->x0 == 51 && iglesia->y0 == 43 && iglesia->ancho == 10 && iglesia->alto == 9);
    // las 39 casillas de H-CIU-0 §5 (fila: columnas)
    const std::pair<int, std::vector<std::pair<int, int>>> sobran[] = {
        {43, {{51, 53}, {55, 60}}}, {44, {{51, 51}, {59, 60}}}, {45, {{51, 51}, {60, 60}}}, {46, {{51, 53}, {59, 60}}},
        {47, {{51, 53}, {60, 60}}}, {48, {{51, 53}, {60, 60}}}, {49, {{51, 51}, {60, 60}}}, {50, {{51, 51}, {59, 60}}},
        {51, {{51, 53}, {55, 57}, {60, 60}}}};
    std::set<std::pair<int, int>> esperadas;
    for (const auto& [y, tramos] : sobran)
        for (const auto& [x0, x1] : tramos)
            for (int x = x0; x <= x1; ++x) esperadas.insert({x, y});
    MOTOR_CHECK(esperadas.size() == 39);
    const std::set<std::pair<int, int>> libres(iglesia->libres.begin(), iglesia->libres.end());
    MOTOR_CHECK(libres == esperadas);
    for (int y = 43; y <= 51; ++y)
        for (int x = 51; x <= 60; ++x) MOTOR_CHECK(tm.getTile(0, x, y).hasCollision());  // hoy todo bloquea
    MOTOR_CHECK(Monumentos::liberaCeldas(tm, plan) >= 39);
    int pisables = 0, bloquean = 0;
    for (int y = 43; y <= 51; ++y)
        for (int x = 51; x <= 60; ++x) {
            const bool b = tm.getTile(0, x, y).hasCollision();
            (b ? bloquean : pisables) += 1;
            MOTOR_CHECK(b == (esperadas.count({x, y}) == 0));
        }
    MOTOR_CHECK(pisables == 39 && bloquean == 51);
}

MOTOR_TEST_CASE(ciudad_centro_el_resto_de_monumentos_tambien_sueltan_sus_casillas_sobrantes) {
    TileMap tm;
    Monumentos m;
    MOTOR_CHECK(cargaCiudad("ciudad_centro.tmx", tm, m));
    const auto plan = m.planifica(tm, tm.getTilesetName());
    for (const char* id : {"iso_mon_universidad", "iso_mon_castillo", "iso_mon_opera", "iso_mon_biblioteca"}) {
        const auto* p = buscaMonumento(plan, id);
        MOTOR_CHECK(p != nullptr);
        if (p == nullptr) continue;
        MOTOR_CHECK(!p->libres.empty());
    }
    const int antes = Monumentos::liberaCeldas(tm, {});
    MOTOR_CHECK(antes == 0);
    const int n = Monumentos::liberaCeldas(tm, plan);
    MOTOR_CHECK(n > 39);
    for (const auto& p : plan)
        for (const auto& [x, y] : p.libres) MOTOR_CHECK(!tm.getTile(0, x, y).hasCollision());
    // la base del sprite sigue bloqueando: la puerta de la iglesia (56,52) no se toca, y el centro de la nave tampoco
    MOTOR_CHECK(tm.getTile(0, 55, 46).hasCollision());
}

int main() { return motor_test::runAllTests(); }
