// J-4 / J-4b — test_monumentos_antiguos: las casas de tile antiguo (viejo_chabola, viejo_casa_madera...) toman el
// sprite de su familia solo si TODA su base es del tile de la casa y la cara de su puerta queda transitable.
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Game/Monumentos.h"
#include "Render/TileMap.h"
#include "Runtime/RuntimeTileMap.h"
#include "TestHarness.h"

using Game::Monumentos;
using Game::MonumentoColocado;

namespace {

struct MapaDePrueba : public RuntimeTileMap {
    void rellena(int w, int h, const std::vector<int>& gids) {
        RuntimeTileLayer capa;
        for (int g : gids) capa.tiles.emplace_back(g, g >= 100);
        replaceCompiledData(w, h, 64, 32, {capa});
    }
};

bool cargaNivel(const std::string& mapa, TileMap& tm, Monumentos& m) {
    return tm.loadFromFile("assets/maps/" + mapa).isOk() && m.carga("assets").isOk() && m.cargado();
}

int orientacionDe(const MonumentoColocado& m) {
    const std::size_t k = m.id.find("_o");
    return k == std::string::npos ? -1 : m.id[k + 2] - '0';
}

bool deTileAntiguo(const MonumentoColocado& m) {
    for (const char* t : {"viejo_chabola", "viejo_casa_madera", "viejo_casa_piedra"}) {
        const std::string n = t;
        if (m.id.size() > n.size() && m.id.compare(m.id.size() - n.size(), n.size(), n) == 0) return true;
    }
    return false;
}

// Las celdas de delante de la cara de la puerta del sprite (o0 +y, o1 -x, o2 -y, o3 +x).
std::vector<std::pair<int, int>> delanteDe(const MonumentoColocado& m) {
    std::vector<std::pair<int, int>> v;
    const int o = orientacionDe(m);
    const int n = (o == 1 || o == 3) ? m.alto : m.ancho;
    for (int k = 0; k < n; ++k) {
        switch (o) {
            case 0: v.push_back({m.x0 + k, m.y0 + m.alto}); break;
            case 1: v.push_back({m.x0 - 1, m.y0 + k}); break;
            case 2: v.push_back({m.x0 + k, m.y0 - 1}); break;
            default: v.push_back({m.x0 + m.ancho, m.y0 + k}); break;
        }
    }
    return v;
}

bool dentro(const MonumentoColocado& m, int x, int y) {
    return x >= m.x0 && x < m.x0 + m.ancho && y >= m.y0 && y < m.y0 + m.alto;
}

// Casas 2x2 de tile «casa» (gid 100, con colision) con las cuatro orientaciones del sprite.
Monumentos casaDePrueba(const std::string& carpeta) {
    std::string defs;
    for (int o = 0; o < 4; ++o) {
        if (o > 0) defs += ",";
        defs += R"({"id": "casa_o)" + std::to_string(o) + R"(", "hoja": "monumento_casa", "tiles": ["casa"], "orientacion": )" +
                std::to_string(o) + R"(, "ancho": 2, "alto": 2, "tiraAncho": 32, "altoHoja": 120, "columnas": 4, "anclaje": {"x": 64, "y": 110}})";
    }
    std::filesystem::create_directories(carpeta + "/textures");
    std::ofstream(carpeta + "/textures/terreno_iso.json")
        << R"({"tiles": [{"gid": 1, "nombre": "pradera"}, {"gid": 100, "nombre": "casa"}, {"gid": 101, "nombre": "muro"}]})";
    std::ofstream(carpeta + "/textures/monumentos.json")
        << R"({"tileset": "terreno_mundo", "indiceTileset": "textures/terreno_iso.json", "monumentos": [)" + defs + "]}";
    Monumentos m;
    MOTOR_CHECK(m.carga(carpeta).isOk());
    return m;
}

}  // namespace

MOTOR_TEST_CASE(una_casa_con_un_hueco_de_suelo_en_la_base_no_recibe_sprite) {
    const std::string carpeta = (std::filesystem::temp_directory_path() / "monumentos_antiguos_test").string();
    std::filesystem::remove_all(carpeta);
    const Monumentos m = casaDePrueba(carpeta);
    MOTOR_CHECK(m.cargado() && m.cuantos() == 4);
    MapaDePrueba mapa;
    // La casa de 2x2 entera se coloca...
    std::vector<int> bien(8 * 8, 1);
    for (int y = 2; y <= 3; ++y)
        for (int x = 2; x <= 3; ++x) bien[static_cast<std::size_t>(y * 8 + x)] = 100;
    mapa.rellena(8, 8, bien);
    MOTOR_CHECK(m.planifica(mapa, "terreno_mundo").size() == 1);
    // ...y con un hueco de suelo dentro de la base (3 de 4 casillas: la regla del 70% la aceptaria) no.
    bien[static_cast<std::size_t>(2 * 8 + 3)] = 1;
    mapa.rellena(8, 8, bien);
    MOTOR_CHECK(m.planifica(mapa, "terreno_mundo").empty());
    std::filesystem::remove_all(carpeta);
}

MOTOR_TEST_CASE(la_orientacion_con_la_cara_de_la_puerta_bloqueada_se_descarta) {
    const std::string carpeta = (std::filesystem::temp_directory_path() / "monumentos_antiguos_test2").string();
    std::filesystem::remove_all(carpeta);
    const Monumentos m = casaDePrueba(carpeta);
    MapaDePrueba mapa;
    std::vector<int> g(8 * 8, 1);
    auto pon = [&](int x, int y, int gid) { g[static_cast<std::size_t>(y * 8 + x)] = gid; };
    for (int y = 3; y <= 4; ++y)
        for (int x = 3; x <= 4; ++x) pon(x, y, 100);
    pon(4, 5, 101);                       // o0 (+y): una de sus dos celdas de delante, bloqueada
    pon(5, 3, 101), pon(5, 4, 101);       // o3 (+x): bloqueada
    pon(2, 3, 101);                       // o1 (-x): una celda bloqueada
    mapa.rellena(8, 8, g);
    // Solo queda libre o2 (-y): celdas (3,2) y (4,2). Aunque la puerta_* este en (3,5), delante de o0, se descarta.
    auto plan = m.planifica(mapa, "terreno_mundo", {{3, 5}});
    MOTOR_CHECK(plan.size() == 1);
    if (plan.size() == 1) MOTOR_CHECK(orientacionDe(plan[0]) == 2);
    // Con las cuatro caras bloqueadas la casa queda a bloques.
    pon(3, 2, 101);
    mapa.rellena(8, 8, g);
    MOTOR_CHECK(m.planifica(mapa, "terreno_mundo", {{3, 5}}).empty());
    // Con todo libre manda la puerta_* del nivel.
    std::vector<int> libre(8 * 8, 1);
    for (int y = 3; y <= 4; ++y)
        for (int x = 3; x <= 4; ++x) libre[static_cast<std::size_t>(y * 8 + x)] = 100;
    mapa.rellena(8, 8, libre);
    const std::pair<int, int> delante[4] = {{3, 5}, {2, 3}, {4, 2}, {5, 4}};
    for (int o = 0; o < 4; ++o) {
        const auto con = m.planifica(mapa, "terreno_mundo", {delante[o]});
        MOTOR_CHECK(con.size() == 1);
        if (con.size() == 1) MOTOR_CHECK(orientacionDe(con[0]) == o);
    }
    std::filesystem::remove_all(carpeta);
}

MOTOR_TEST_CASE(aldea_xvii_la_barraca_de_tile_antiguo_lleva_sprite_con_la_puerta_donde_toca) {
    TileMap tm;
    Monumentos m;
    MOTOR_CHECK(cargaNivel("ciudad_en_aldea_xvii.tmx", tm, m));
    const auto plan = m.planifica(tm, tm.getTilesetName());
    const MonumentoColocado* b = nullptr;
    for (const auto& p : plan)
        if (p.id.rfind("iso_barraca_", 0) == 0 && p.x0 == 16 && p.y0 == 24 && p.ancho == 3 && p.alto == 2) b = &p;
    MOTOR_CHECK(b != nullptr);
    if (b == nullptr) return;
    MOTOR_CHECK(deTileAntiguo(*b));
    for (const auto& [x, y] : delanteDe(*b)) MOTOR_CHECK(!tm.getTile(0, x, y).hasCollision());
    const auto cubiertas = Monumentos::celdasCubiertas(plan);  // sin paredes invisibles
    for (int y = 24; y <= 25; ++y)
        for (int x = 16; x <= 18; ++x) MOTOR_CHECK(cubiertas.count({x, y}) == 1);
    for (int ori = 0; ori < 4; ++ori) {  // una puerta_* delante de la cara, si esta libre, decide la orientacion
        const std::pair<int, int> delante[4] = {{17, 26}, {15, 24}, {17, 23}, {19, 25}};
        const auto con = m.planifica(tm, tm.getTilesetName(), {delante[ori]});
        for (const auto& p : con)
            if (deTileAntiguo(p) && p.x0 == 16 && p.y0 == 24 && !tm.getTile(0, delante[ori].first, delante[ori].second).hasCollision())
                MOTOR_CHECK(orientacionDe(p) == ori);
    }
}

// Las 15 casillas caminables que J-4 dejaba bajo un sprite nuevo (forat de la mancha): ninguna sprite de tile
// antiguo las pisa, y las dos puertas contra pared (J-4b) ya no existen.
MOTOR_TEST_CASE(ningun_sprite_de_tile_antiguo_cubre_las_casillas_caminables_conocidas) {
    struct Caso { const char* mapa; int x, y; };
    const Caso casos[] = {
        {"ciudad_en_aldea_xvii.tmx", 12, 11}, {"ciudad_en_aldea_xvii.tmx", 13, 11}, {"ciudad_en_ciudad_xiii.tmx", 21, 30},
        {"ciudad_en_ciudad_xiii.tmx", 25, 40}, {"ciudad_en_ciudad_xv.tmx", 13, 30}, {"ciudad_en_ciudad_xv.tmx", 7, 44},
        {"ciudad_en_guskedor.tmx", 6, 23}, {"ciudad_en_guskedor.tmx", 36, 27}, {"ciudad_en_guskedor.tmx", 38, 29},
        {"ciudad_en_qethatos.tmx", 34, 10}, {"ciudad_en_qethatos.tmx", 36, 12}, {"ciudad_en_qethatos.tmx", 25, 40},
        {"on_havargruztak.tmx", 28, 28}, {"on_havargruztak.tmx", 13, 13}, {"on_ostad.tmx", 9, 29}};
    for (const Caso& c : casos) {
        TileMap tm;
        Monumentos m;
        MOTOR_CHECK(cargaNivel(c.mapa, tm, m));
        const auto plan = m.planifica(tm, tm.getTilesetName());
        for (const auto& p : plan) {
            if (!deTileAntiguo(p)) continue;
            MOTOR_CHECK(!dentro(p, c.x, c.y));
            for (const auto& [x, y] : delanteDe(p))  // la cara de la puerta: dentro del mapa y sin colision
                MOTOR_CHECK(x >= 0 && y >= 0 && x < tm.getWidth() && y < tm.getHeight() && !tm.getTile(0, x, y).hasCollision());
        }
    }
}

int main() { return motor_test::runAllTests(); }
