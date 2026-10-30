// AV19-02 — La altura del terreno: la rejilla, las rampas, la regla de paso, el JSON del nivel y que
// el dibujo de una entidad sube con el terreno. Sin GL.
#include "Level/LevelLoader.h"
#include "Runtime/Alturas.h"
#include "TestHarness.h"

#include <cmath>
#include <string>


MOTOR_TEST_CASE(alturas_una_meseta_y_una_rampa) {
    Alturas a;
    a.construye({AlturaRegion{2, 2, 3, 3, 2, false, 0, 'y'},          // meseta a nivel 2
                 AlturaRegion{2, 5, 3, 3, 0, true, 2, 'y'}},           // rampa 0 -> 2 a lo largo de y
                10, 10);
    MOTOR_CHECK(a.hay());
    MOTOR_CHECK(a.en(3, 3) == 2);
    MOTOR_CHECK(a.en(0, 0) == 0);
    MOTOR_CHECK(a.en(-1, 4) == 0 && a.en(99, 4) == 0);  // fuera del mapa, 0
    MOTOR_CHECK(a.en(3, 5) == 0 && a.en(3, 6) == 1 && a.en(3, 7) == 2);  // la rampa sube de a un nivel
}

MOTOR_TEST_CASE(alturas_se_pasa_con_un_nivel_y_un_acantilado_no) {
    Alturas a;
    a.construye({AlturaRegion{0, 0, 1, 1, 0, false, 0, 'y'}, AlturaRegion{1, 0, 1, 1, 1, false, 0, 'y'},
                 AlturaRegion{2, 0, 1, 1, 3, false, 0, 'y'}},
                4, 1);
    MOTOR_CHECK(a.sePuedePasar(GridCoord{0, 0}, GridCoord{1, 0}));   // 0 -> 1: escalon
    MOTOR_CHECK(!a.sePuedePasar(GridCoord{1, 0}, GridCoord{2, 0}));  // 1 -> 3: acantilado
    MOTOR_CHECK(!a.sePuedePasar(GridCoord{2, 0}, GridCoord{1, 0}));  // y tampoco al reves
    MOTOR_CHECK(a.sePuedePasar(GridCoord{2, 0}, GridCoord{2, 0}));
}

MOTOR_TEST_CASE(alturas_un_mapa_plano_no_tiene_altura) {
    Alturas a;
    a.construye({}, 5, 5);
    MOTOR_CHECK(!a.hay());
    MOTOR_CHECK(Alturas::desfasePx(0) == 0.0f);
    MOTOR_CHECK(Alturas::desfasePx(2) == -2.0f * Alturas::kPasoPx);
}

MOTOR_TEST_CASE(alturas_el_nivel_las_lee_y_rechaza_lo_raro) {
    const std::string base = R"({"name":"n","map":"m.tmx","playerStart":{"x":0,"y":0},"alturas":[
        {"x":1,"y":1,"w":2,"h":2,"nivel":3},
        {"x":4,"y":0,"w":1,"h":5,"nivel":0,"hasta":4,"eje":"y"}]})";
    auto r = LevelLoader::loadFromString(base);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().alturas.size() == 2);
    MOTOR_CHECK(r.value().alturas[0].nivel == 3 && !r.value().alturas[0].conRampa);
    MOTOR_CHECK(r.value().alturas[1].conRampa && r.value().alturas[1].hasta == 4 && r.value().alturas[1].eje == 'y');
    // un nivel fuera de rango, sin nivel o un eje raro: error, no se traga
    auto malo = [&](const char* alturas) {
        return LevelLoader::loadFromString(std::string(R"({"name":"n","map":"m.tmx","alturas":)") + alturas + "}").isOk();
    };
    MOTOR_CHECK(!malo(R"([{"x":0,"y":0,"nivel":99}])"));
    MOTOR_CHECK(!malo(R"([{"x":0,"y":0}])"));
    MOTOR_CHECK(!malo(R"([{"x":0,"y":0,"nivel":1,"hasta":2,"eje":"z"}])"));
    MOTOR_CHECK(!malo(R"({"x":0})"));
    MOTOR_CHECK(malo("[]"));
}

int main() { return motor_test::runAllTests(); }
