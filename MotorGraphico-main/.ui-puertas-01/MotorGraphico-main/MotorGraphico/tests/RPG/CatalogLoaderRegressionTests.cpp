#include "RPG/Catalogs/RpgCatalogs.h"

#include "TestHarness.h"

#include <algorithm>
#include <string>

// D-01 — Regresiones de los cargadores de catalogo que el comprobador
// estatico (tools/comprobar_contrato_cargadores.py) destapo.
//
// f23dba9 (13/09) quito de RaceDefinition::from_json las lecturas de la
// velocidad y de las deidades compatibles, mientras su declaracion en
// expected_fields() ganaba los alias que usa el asset («speed»,
// «compatibleDeities»). Desde entonces toda raza cargaba velocidad 30 y
// ninguna deidad, sin un solo error: el test de D-06 comprueba que la
// velocidad esta en [20, 50], y 30 siempre lo esta.
//
// Aqui se comprueba el VALOR, con el nombre antiguo y con el del asset, y
// con la raza real que no anda a 30.

using namespace RPG;
using namespace RPG::Catalogs;

namespace {

RaceCatalog cargar(const std::string& json) {
    RaceCatalog c;
    const auto r = c.loadFromString(json);
    if (!r.isOk()) MOTOR_CHECK_FAIL("carga: " + r.errorMessage());
    return c;
}

bool tiene(const std::vector<std::string>& v, const std::string& x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

}  // namespace

MOTOR_TEST_CASE(la_raza_lee_velocidad_y_deidades_con_el_nombre_del_asset) {
    const RaceCatalog c = cargar(R"({"entries": [
        {"id": "enano", "name": "Enano", "speed": 25,
         "compatibleDeities": ["deity_forge_father", "deity_hearth"]}
    ]})");
    const RaceDefinition* r = c.find("enano");
    MOTOR_CHECK(r != nullptr);
    MOTOR_CHECK(r->baseSpeed == 25);
    MOTOR_CHECK(r->compatibleDeityIds.size() == 2);
    MOTOR_CHECK(tiene(r->compatibleDeityIds, "deity_forge_father"));
}

MOTOR_TEST_CASE(la_raza_sigue_leyendo_los_nombres_antiguos_y_estos_mandan) {
    const RaceCatalog c = cargar(R"({"entries": [
        {"id": "a", "name": "A", "baseSpeed": 35, "compatibleDeityIds": ["d1"]},
        {"id": "b", "name": "B", "movementSpeed": 40},
        {"id": "c", "name": "C", "movement_speed": 20},
        {"id": "d", "name": "D", "baseSpeed": 45, "speed": 10,
         "compatibleDeityIds": ["canonica"], "compatibleDeities": ["alias"]},
        {"id": "e", "name": "E"}
    ]})");
    MOTOR_CHECK(c.find("a")->baseSpeed == 35);
    MOTOR_CHECK(c.find("a")->compatibleDeityIds == std::vector<std::string>{"d1"});
    MOTOR_CHECK(c.find("b")->baseSpeed == 40);
    MOTOR_CHECK(c.find("c")->baseSpeed == 20);
    // Con los dos nombres a la vez, el canonico gana y el alias no se mezcla.
    MOTOR_CHECK(c.find("d")->baseSpeed == 45);
    MOTOR_CHECK(c.find("d")->compatibleDeityIds == std::vector<std::string>{"canonica"});
    // Sin nada: los valores por defecto de siempre.
    MOTOR_CHECK(c.find("e")->baseSpeed == 30);
    MOTOR_CHECK(c.find("e")->compatibleDeityIds.empty());
}

MOTOR_TEST_CASE(las_razas_publicadas_no_andan_todas_a_30) {
    RaceCatalog c;
    const auto r = c.loadFromFile("assets/catalogs/races.json");
    if (!r.isOk()) MOTOR_CHECK_FAIL("races.json: " + r.errorMessage());
    // Contenido publicado el 17/09/2026: la unica raza con velocidad
    // distinta de 30 es la de Hondaforja, y cuatro razas declaran deidades.
    // DIOSES-01 (08/10): su deidad es ya Forja, la del vault de Oriol.
    const RaceDefinition* enano = c.find("race_dwarf_deepforge");
    MOTOR_CHECK(enano != nullptr);
    MOTOR_CHECK(enano->baseSpeed == 25);
    MOTOR_CHECK(tiene(enano->compatibleDeityIds, "forja"));
    std::size_t conDeidades = 0;
    c.forEach([&](const RaceDefinition& raza) {
        if (!raza.compatibleDeityIds.empty()) ++conDeidades;
    });
    MOTOR_CHECK(conDeidades >= 4);
}

int main() { return motor_test::runAllTests(); }
