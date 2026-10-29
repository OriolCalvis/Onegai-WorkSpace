// MASC-01 — test_mascotes: les mascotes sense OpenGL.
#include <cmath>
#include <cstdlib>
#include <set>
#include <utility>
#include <vector>

#include <cstdio>
#include <fstream>

#include "Game/FullaMascotes.h"
#include "Game/Mascotes.h"
#include "TestHarness.h"

using Game::EspecieMascota;
using Game::EstatMascota;
using Game::Mascota;
using Game::Mascotes;

namespace {
// Un pati de 20 x 20 amb una paret a x = 10 (amb un forat a y = 15) i una
// casa (bloc) de 3 x 3 a (4..6, 4..6).
bool pati(int x, int y) {
    if (x < 0 || y < 0 || x >= 20 || y >= 20) return false;
    if (x == 10 && y != 15) return false;
    if (x >= 4 && x <= 6 && y >= 4 && y <= 6) return false;
    return true;
}

std::vector<EspecieMascota> especies() {
    EspecieMascota gat{"gat", 1.4f, 4, true, true};
    EspecieMascota guineu{"guineu", 1.6f, 4, false, false};
    return {gat, guineu};
}

int sep(const GridCoord& a, const GridCoord& b) { return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y)); }
int manhattan(const GridCoord& a, const GridCoord& b) { return std::abs(a.x - b.x) + std::abs(a.y - b.y); }
}  // namespace

MOTOR_TEST_CASE(sense_casa_transitable_ni_especie_no_hi_ha_mascota) {
    Mascotes m;
    m.prepara({{0, GridCoord{5, 5}}, {7, GridCoord{2, 2}}, {-1, GridCoord{2, 3}}}, especies(), pati, 3u);
    MOTOR_CHECK(m.buida());
    m.prepara({{0, GridCoord{2, 2}}}, especies(), nullptr, 3u);
    MOTOR_CHECK(m.buida());
    m.prepara({{0, GridCoord{2, 2}}, {1, GridCoord{14, 14}}}, especies(), pati, 3u);
    MOTOR_CHECK(m.mascotes().size() == 2);
    const Mascota& g = m.mascotes()[0];
    MOTOR_CHECK(std::fabs(g.x - 2.5f) < 1e-4f && std::fabs(g.y - 2.5f) < 1e-4f);
    MOTOR_CHECK(g.estat == EstatMascota::Repos);
}

MOTOR_TEST_CASE(passeja_per_celles_transitables_a_prop_de_casa_i_es_determinista) {
    Mascotes a, b;
    a.prepara({{0, GridCoord{2, 2}}, {0, GridCoord{14, 3}}}, especies(), pati, 42u);
    b.prepara({{0, GridCoord{2, 2}}, {0, GridCoord{14, 3}}}, especies(), pati, 42u);
    const GridCoord lluny{19, 19};
    bool haCaminat = false;
    float recorregut = 0.0f;
    for (int i = 0; i < 1200; ++i) {
        a.avanza(0.05f, lluny);
        b.avanza(0.05f, lluny);
        for (std::size_t k = 0; k < 2; ++k) {
            const Mascota& m = a.mascotes()[k];
            const GridCoord c = Mascotes::cella(m);
            MOTOR_CHECK(pati(c.x, c.y));
            MOTOR_CHECK(sep(c, m.casa) <= 4 * 2 + 2);
            MOTOR_CHECK(std::fabs(m.x - b.mascotes()[k].x) < 1e-5f && std::fabs(m.y - b.mascotes()[k].y) < 1e-5f);
            if (m.estat == EstatMascota::Camina) haCaminat = true;
            // En eix: mai en diagonal (una de les dues coordenades al centre).
            const bool enEix = std::fabs(m.x - std::floor(m.x) - 0.5f) < 1e-3f || std::fabs(m.y - std::floor(m.y) - 0.5f) < 1e-3f;
            MOTOR_CHECK(enEix);
        }
        recorregut = a.mascotes()[0].recorregut;
    }
    MOTOR_CHECK(haCaminat);
    MOTOR_CHECK(recorregut > 3.0f);  // en 60 s ha passejat
}

MOTOR_TEST_CASE(l_afectuosa_ve_a_saludar_i_despres_el_deixa_estar) {
    Mascotes m;
    m.prepara({{0, GridCoord{2, 2}}}, especies(), pati, 7u);
    const GridCoord jugador{2, 5};  // a 3 cel·les
    bool saluda = false;
    for (int i = 0; i < 200 && !saluda; ++i) {
        m.avanza(0.05f, jugador);
        const Mascota& g = m.mascotes()[0];
        MOTOR_CHECK(!(Mascotes::cella(g).x == jugador.x && Mascotes::cella(g).y == jugador.y));
        saluda = g.estat == EstatMascota::Saluda;
    }
    MOTOR_CHECK(saluda);
    const Mascota& g = m.mascotes()[0];
    MOTOR_CHECK(manhattan(Mascotes::cella(g), jugador) == 1);
    // Mira el jugador: el jugador es a +y (avall a l'esquerra), direccio 1.
    MOTOR_CHECK(g.direccio == Mascotes::direccioDe(jugador.x - Mascotes::cella(g).x, jugador.y - Mascotes::cella(g).y));
    for (int i = 0; i < 120; ++i) m.avanza(0.05f, jugador);  // 6 s > kSalutacio
    MOTOR_CHECK(m.mascotes()[0].estat != EstatMascota::Saluda);
    MOTOR_CHECK(m.mascotes()[0].calma > 0.0f);
}

MOTOR_TEST_CASE(l_esquiva_se_n_aparta_i_mai_trepitja_el_jugador) {
    Mascotes m;
    m.prepara({{1, GridCoord{14, 14}}}, especies(), pati, 9u);
    const GridCoord jugador{14, 16};  // a 2 cel·les
    for (int i = 0; i < 100; ++i) {
        m.avanza(0.05f, jugador);
        const GridCoord c = Mascotes::cella(m.mascotes()[0]);
        MOTOR_CHECK(!(c.x == jugador.x && c.y == jugador.y));
    }
    const Mascota& g = m.mascotes()[0];
    const float d = std::hypot(g.x - 14.5f, g.y - 16.5f);
    MOTOR_CHECK(d > 2.5f);
    MOTOR_CHECK(g.estat != EstatMascota::Saluda);
}

MOTOR_TEST_CASE(dues_mascotes_no_comparteixen_cella) {
    Mascotes m;
    // Un passadis estret: y = 15 a l'esquerra de la paret i el forat.
    auto passadis = [](int x, int y) { return y == 15 && x >= 0 && x < 20; };
    m.prepara({{0, GridCoord{3, 15}}, {0, GridCoord{6, 15}}, {0, GridCoord{9, 15}}}, especies(), passadis, 5u);
    MOTOR_CHECK(m.mascotes().size() == 3);
    const GridCoord lluny{0, 0};
    for (int i = 0; i < 1500; ++i) {
        m.avanza(0.05f, lluny);
        std::set<std::pair<int, int>> ocupades;
        for (const Mascota& g : m.mascotes()) {
            const GridCoord c = Mascotes::cella(g);
            MOTOR_CHECK(passadis(c.x, c.y));
            MOTOR_CHECK(ocupades.insert({c.x, c.y}).second);
        }
    }
}

MOTOR_TEST_CASE(dorm_a_casa_i_es_desperta_quan_arriba_el_jugador) {
    Mascotes m;
    m.prepara({{0, GridCoord{15, 3}}}, especies(), pati, 21u);
    const GridCoord lluny{1, 18};
    bool dorm = false;
    for (int i = 0; i < 20000 && !dorm; ++i) {
        m.avanza(0.05f, lluny);
        dorm = m.mascotes()[0].estat == EstatMascota::Dorm;
    }
    MOTOR_CHECK(dorm);
    const Mascota& g = m.mascotes()[0];
    MOTOR_CHECK(sep(Mascotes::cella(g), g.casa) <= 1);
    MOTOR_CHECK(g.direccio == 0 || g.direccio == 1);  // dorm de cara
    const GridCoord c = Mascotes::cella(g);
    const GridCoord alCostat = pati(c.x + 1, c.y) ? GridCoord{c.x + 1, c.y} : GridCoord{c.x, c.y + 1};
    m.avanza(0.05f, alCostat);
    MOTOR_CHECK(m.mascotes()[0].estat != EstatMascota::Dorm);
}

MOTOR_TEST_CASE(direccions_de_la_pantalla) {
    MOTOR_CHECK(Mascotes::direccioDe(1, 0) == 0);
    MOTOR_CHECK(Mascotes::direccioDe(0, 1) == 1);
    MOTOR_CHECK(Mascotes::direccioDe(-1, 0) == 2);
    MOTOR_CHECK(Mascotes::direccioDe(0, -1) == 3);
    MOTOR_CHECK(Mascotes::direccioCap(2.0f, 1.0f) == 0);
    MOTOR_CHECK(Mascotes::direccioCap(-0.5f, 3.0f) == 1);
    MOTOR_CHECK(Mascotes::direccioCap(-2.0f, 1.0f) == 2);
    MOTOR_CHECK(Mascotes::direccioCap(0.5f, -3.0f) == 3);
}

namespace {
const char* kManifest = R"({
 "image": "x.png", "frameSize": [64, 64], "columns": 16, "rows": 2, "peu": {"x": 32, "y": 58},
 "frames": {"mascota_gat": 13},
 "especies": [
  {"id": "gat", "velocitat": 2.0, "radi": 3, "afectuosa": true,
   "anims": {"camina": {"fps": 8, "dirs": [[1, 4], [5, 4], [9, 4], [13, 4]]},
             "repos": {"fps": 4, "dirs": [[17, 2], [19, 2], [21, 2], [23, 2]]},
             "saluda": {"fps": 8, "aguanta": true, "dirs": [[25, 3], [28, 3], [0, 0], [0, 0]]},
             "dorm": {"fps": 2, "dirs": [[31, 1], [32, 1], [0, 0], [0, 0]]}},
   "magia": {"color": "cristall", "particula": "hexagon", "quantes": 3, "alcada": 8}},
  {"id": "guineu", "velocitat": 1.5, "afectuosa": false,
   "anims": {"camina": {"dirs": [[1, 4], [5, 4], [9, 4], [13, 4]]}, "repos": {"dirs": [[17, 2], [19, 2], [21, 2], [23, 2]]}},
   "magia": {"color": "vent", "anell": true}}
 ],
 "efectes": {"llum_cristall": 40, "hexagon_cristall": 41}
})";

Game::FullaMascotes fullaDeProva() {
    const char* ruta = "test_mascotes_manifest.json";
    std::ofstream(ruta) << kManifest;
    Game::FullaMascotes f;
    auto r = f.carga(ruta);
    MOTOR_CHECK(r.isOk() && r.value());
    std::remove(ruta);
    return f;
}
}  // namespace

MOTOR_TEST_CASE(sense_manifest_no_hi_ha_mascotes_i_no_es_error) {
    Game::FullaMascotes f;
    auto r = f.carga("no_existeix_mascotes.json");
    MOTOR_CHECK(r.isOk() && !r.value());
    MOTOR_CHECK(!f.carregada());
}

MOTOR_TEST_CASE(la_fulla_porta_especies_animacions_i_magia) {
    const Game::FullaMascotes f = fullaDeProva();
    MOTOR_CHECK(f.especies().size() == 2);
    MOTOR_CHECK(f.indexDe("guineu") == 1 && f.indexDe("drac") == -1);
    const auto& gat = f.especies()[0];
    MOTOR_CHECK(gat.especie.velocitat == 2.0f && gat.especie.radi == 3 && gat.especie.afectuosa && gat.especie.dorm);
    MOTOR_CHECK(!f.especies()[1].especie.afectuosa && !f.especies()[1].especie.dorm);
    MOTOR_CHECK(gat.magia.particula == "hexagon" && gat.magia.quantes == 3 && f.especies()[1].magia.anell);
    MOTOR_CHECK(f.efecte("llum_cristall") == 40 && f.efecte("res") == 0);
    MOTOR_CHECK(f.peuX() == 32 && f.peuY() == 58);
    MOTOR_CHECK(f.nomesEspecies().size() == 2);
}

MOTOR_TEST_CASE(el_frame_segueix_l_estat_la_direccio_i_el_cami) {
    const Game::FullaMascotes f = fullaDeProva();
    Mascota m;
    m.especie = 0;
    m.estat = EstatMascota::Camina;
    m.direccio = 2;
    m.recorregut = 0.0f;
    MOTOR_CHECK(f.frameDe(m) == 9);
    m.recorregut = 0.5f;  // 0,5 cel·les * 8 fps / 2 cel·les/s = 2 frames
    MOTOR_CHECK(f.frameDe(m) == 11);
    m.recorregut = 1.0f;  // 4 frames: torna a comencar
    MOTOR_CHECK(f.frameDe(m) == 9);
    m.estat = EstatMascota::Saluda;  // s'asseu i s'hi queda
    m.direccio = 1;
    m.rellotge = 10.0f;
    MOTOR_CHECK(f.frameDe(m) == 30);
    m.direccio = 3;  // de cul no te salutacio: repos
    m.rellotge = 0.0f;
    MOTOR_CHECK(f.frameDe(m) == 23);
    m.estat = EstatMascota::Dorm;
    m.direccio = 0;
    MOTOR_CHECK(f.frameDe(m) == 31);
    m.estat = EstatMascota::Repos;
    m.variant = 1;  // sense repos2, el repos de sempre
    m.rellotge = 0.3f;
    MOTOR_CHECK(f.frameDe(m) == 18);
    m.especie = 5;
    MOTOR_CHECK(f.frameDe(m) == 0);
}

int main() { return motor_test::runAllTests(); }
