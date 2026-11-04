// E-DISPARADORES-1 E2b (D0026.7 y .8) — Lo que los disparadores de cinematica necesitan del nivel y de los objetos:
//   - la linea de vision compartida con el tablero de combate (Game/LineaDeVision.h);
//   - el ambito del nivel (Level/AmbitoNivel.h): declarado en el JSON o deducido por nombre y tamano;
//   - `tapaVista` de ObjectDefinition: false por defecto, true en las paredes, vallas, carros y armarios del catalogo real.
// GL-free; cwd = raiz del build (los catalogos reales se leen de assets/).
#include <set>
#include <string>

#include "Game/LineaDeVision.h"
#include "Game/TableroCombate.h"
#include "Level/AmbitoNivel.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"
#include "TestHarness.h"

using Game::lineaDeVision;

MOTOR_TEST_CASE(la_linea_de_vision_se_corta_con_lo_que_tapa_y_no_cuenta_los_extremos) {
    auto muroEn6 = [](int x, int) { return x == 6; };
    MOTOR_CHECK(lineaDeVision({2, 5}, {9, 5}, muroEn6) == false);   // el muro esta en medio
    MOTOR_CHECK(lineaDeVision({2, 5}, {5, 5}, muroEn6) == true);    // el muro queda detras
    MOTOR_CHECK(lineaDeVision({2, 5}, {6, 5}, muroEn6) == true);    // el extremo no cuenta (un banco)
    MOTOR_CHECK(lineaDeVision({6, 5}, {9, 5}, muroEn6) == true);    // ni el de salida
    MOTOR_CHECK(lineaDeVision({2, 5}, {9, 5}, nullptr) == true);    // nada tapa
    MOTOR_CHECK(lineaDeVision({3, 3}, {3, 3}, muroEn6) == true);    // misma casilla
    MOTOR_CHECK(Game::distanciaChebyshev({1, 1}, {4, 3}) == 3);
}

MOTOR_TEST_CASE(el_tablero_de_combate_ve_igual_que_la_funcion_compartida) {
    // El comportamiento del combate no cambia: seVen == lineaDeVision con la misma funcion de bloqueo.
    auto bloquea = [](int x, int y) { return (x == 4 && y >= 2 && y <= 6) || (x == 8 && y == 8); };
    const auto t = Game::TableroCombate::alrededor({GridCoord{5, 5}}, 12, 12, bloquea, 12);
    for (int y = 0; y < 12; y += 3)
        for (int x = 0; x < 12; x += 2)
            for (int y2 = 0; y2 < 12; y2 += 4)
                for (int x2 = 0; x2 < 12; x2 += 3)
                    MOTOR_CHECK(t.seVen({x, y}, {x2, y2}) == lineaDeVision({x, y}, {x2, y2}, bloquea));
    MOTOR_CHECK(t.seVen({1, 4}, {7, 4}) == false);
}

MOTOR_TEST_CASE(el_ambito_se_deduce_por_nombre_y_tamano_o_se_declara) {
    // Interior: menos de 200 casillas = pequeno (la taberna, 15x12 = 180); mas, grande (la iglesia, 21x15).
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/interior_taberna.json", 15, 12) == AmbitoNivel::InteriorPequeno);
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/interior_iglesia.json", 21, 15) == AmbitoNivel::InteriorGrande);
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/interior_x.json", 20, 10) == AmbitoNivel::InteriorGrande);  // 200 exactos: grande
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/interior_x.json", 19, 10) == AmbitoNivel::InteriorPequeno);
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/mina2_gorran.json", 42, 42) == AmbitoNivel::InteriorGrande);
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/dependencias_secta.json", 32, 32) == AmbitoNivel::InteriorGrande);
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/aldea_cojo_p0.json", 10, 10) == AmbitoNivel::InteriorPequeno);
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/ciudad_centro.json", 64, 64) == AmbitoNivel::Exterior);
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/ciudad_en_aldea_m.json", 8, 8) == AmbitoNivel::Exterior);  // «aldea» no es prefijo
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Deducido, "assets/levels/prado_del_rio.json", 8, 8) == AmbitoNivel::Exterior);
    // Lo declarado manda sobre el nombre y el tamano.
    MOTOR_CHECK(ambitoDelNivel(AmbitoNivel::Exterior, "assets/levels/interior_taberna.json", 15, 12) == AmbitoNivel::Exterior);
    MOTOR_CHECK(radioDelAmbito(AmbitoNivel::InteriorGrande) == 5 && radioDelAmbito(AmbitoNivel::Exterior) == 10);
    MOTOR_CHECK(radioDelAmbito(AmbitoNivel::InteriorPequeno) == kRadioAlEntrar);
}

MOTOR_TEST_CASE(el_nivel_lee_su_ambito_y_rechaza_uno_raro) {
    auto lee = [](const std::string& ambito) {
        return LevelLoader::loadFromString(R"({"name":"n","map":"m.tmx")" + ambito + "}");
    };
    auto sin = lee("");
    MOTOR_CHECK(sin.isOk() && sin.value().ambito == AmbitoNivel::Deducido);
    auto g = lee(R"(,"ambito":"interior_grande")");
    MOTOR_CHECK(g.isOk() && g.value().ambito == AmbitoNivel::InteriorGrande);
    auto e = lee(R"(,"ambito":"exterior")");
    MOTOR_CHECK(e.isOk() && e.value().ambito == AmbitoNivel::Exterior);
    MOTOR_CHECK(!lee(R"(,"ambito":"subterraneo")").isOk());
    MOTOR_CHECK(!lee(R"(,"ambito":3)").isOk());
}

MOTOR_TEST_CASE(tapaVista_es_false_por_defecto_y_se_lee_y_se_escribe) {
    ObjectCatalog cat;
    auto r = cat.loadFromString(R"({"objects":[
        {"id":"muro","category":"prop","blocksMovement":true,"tapaVista":true},
        {"id":"farola","category":"prop","blocksMovement":true}]})");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.find("muro") != nullptr && cat.find("muro")->tapaVista);
    MOTOR_CHECK(cat.find("farola") != nullptr && !cat.find("farola")->tapaVista);
    ObjectCatalog otro;  // ida y vuelta
    MOTOR_CHECK(otro.loadFromString(cat.exportJson()).isOk());
    MOTOR_CHECK(otro.find("muro")->tapaVista && !otro.find("farola")->tapaVista);
}

MOTOR_TEST_CASE(los_catalogos_reales_marcan_paredes_vallas_carros_y_armarios_y_no_lo_pequeno) {
    ObjectCatalog cat;
    for (const char* f : {"mobles_kit", "decorados_kit", "pixellab_objetos", "boundington_npcs", "boundington_av19_objetos",
                          "botin_boundington"}) {
        auto r = cat.loadFromFile(std::string("assets/objects/") + f + ".json");
        MOTOR_CHECK(r.isOk());
    }
    int paredes = 0, marcados = 0;
    for (const std::string& id : cat.ids()) {
        const ObjectDefinition* d = cat.find(id);
        if (d->tapaVista) {
            ++marcados;
            MOTOR_CHECK(d->blocksMovement && d->category == ObjectCategory::Prop);  // solo tapa lo que bloquea
        }
        if (id.rfind("paret_", 0) == 0) {
            ++paredes;
            MOTOR_CHECK(d->tapaVista);
        }
    }
    MOTOR_CHECK(paredes == 368);
    for (const char* id : {"tanca_fusta", "empalizada", "carromato", "carro", "carro_o1", "carro_aigren", "armario", "estanteria",
                           "vitrina", "puerta_gurkazaal", "verja_dorada"})
        MOTOR_CHECK(cat.find(id) != nullptr && cat.find(id)->tapaVista);
    // Lo pequeno y las personas no tapan: farolas, taburetes, mesas, sillas, cajas, PNJ.
    for (const char* id : {"farola", "taburete", "mesa", "silla", "banco_plaza", "caja", "verina_estrado", "devoto_puerta"})
        MOTOR_CHECK(cat.find(id) == nullptr || !cat.find(id)->tapaVista);
    MOTOR_CHECK(marcados > 400);
}

int main() { return motor_test::runAllTests(); }
