// >>> PAQUETES DEL PLAN <<<
// D-03 — Monstres. PROVA DEL PLA (PLA_DEFINITIU §D-03, «Ho prova»):
//   «test_catalog_content: recorrer els 432 i afirmar que maxHealth > 0
//    i que les quatre estadístiques no són totes zero.»
//
// Aquesta prova s'implanta en target i fitxer PROPIS (no dins de
// CatalogContentTests, zona viva de D-02/D-07) per no modelar a una eina
// en mans d'altres sessions. Conviu amb tests/RPG/MonsterCatalogTests.cpp
// (WIP huerfano sense versionar, NO tocat — regla 2); aquí es compleix la
// lletra del pla, allà només s'hi escombra una cota relaxada.
//
// «Fet quan» del paquet: cap monstre del catàleg es carrega amb vida zero.
// El loader que llegeix defenses/derived va versionat a `ac17bac`
// (D-03 — rescate autorizado). Aquesta batejada el fixa contra regressió.
//
// GL-free: llegeix JSON directament (assets/catalogs/monsters.json), no
// toca ni Render ni GL. cwd = arrel del MotorGraphico (com la resta de
// tests de catàleg, via working directory del CTest).

#include "RPG/Catalogs/RpgCatalogs.h"

#include "TestHarness.h"

#include <cstddef>

namespace {

using RPG::Catalogs::CatalogStrictness;
using RPG::Catalogs::MonsterCatalog;

MonsterCatalog carrega_monstres() {
    MonsterCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/monsters.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    return cat;
}

MOTOR_TEST_CASE(els_432_monstres_es_carreguen_sense_errors) {
    MonsterCatalog cat;
    const auto r = cat.loadFromFile("assets/catalogs/monsters.json", CatalogStrictness::Structural);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(cat.size() == 432);
}

// Prova textual del pla: recorrer tot el catàleg i cap monstre amb maxHealth<=0.
MOTOR_TEST_CASE(D03_cap_monstre_duu_vida_zero) {
    const auto cat = carrega_monstres();
    std::size_t total = 0;
    std::size_t vides_zero = 0;
    cat.forEach([&](const RPG::MonsterDefinition& m) {
        ++total;
        if (m.maxHealth <= 0) ++vides_zero;
    });
    MOTOR_CHECK(total == 432);
    MOTOR_CHECK(vides_zero == 0);
}

// Prova textual del pla: les quatre estadístiques no són totes zero
// en cap monstre. A més es penalitza qualsevol individu amb la suma nul-la.
MOTOR_TEST_CASE(D03_cap_monstre_amb_estadistiques_totes_zero) {
    const auto cat = carrega_monstres();
    std::size_t total = 0;
    std::size_t amb_suma_zero = 0;
    cat.forEach([&](const RPG::MonsterDefinition& m) {
        ++total;
        const int suma = m.stats[0] + m.stats[1] + m.stats[2] + m.stats[3];
        if (suma == 0) ++amb_suma_zero;
    });
    MOTOR_CHECK(total == 432);
    MOTOR_CHECK(amb_suma_zero == 0);
}

// 2-3 cartes concretes amb els valors escrits al test (el que demana la
// fila D-03 del TABLERO): valors reals publicats i llegits pel loader.
MOTOR_TEST_CASE(D03_cartes_concretes_escrites) {
    const auto cat = carrega_monstres();

    const auto* furios = cat.find("ultimo_creador_furioso");
    MOTOR_CHECK(furios != nullptr);
    if (furios) {
        MOTOR_CHECK(furios->name == "Último Creador Furioso");
        MOTOR_CHECK(furios->rank == "jefe");
        MOTOR_CHECK(furios->tier == 5);
        MOTOR_CHECK(furios->ca == 22);
        MOTOR_CHECK(furios->maxHealth == 80);
        MOTOR_CHECK(furios->movement == 5);
        MOTOR_CHECK(furios->stats[0] == 11 && furios->stats[1] == 10 &&
                    furios->stats[2] == 9 && furios->stats[3] == 9);
        MOTOR_CHECK(furios->attacks.size() == 3);
    }

    const auto* feretre = cat.find("portador_del_feretro_real");
    MOTOR_CHECK(feretre != nullptr);
    if (feretre) {
        MOTOR_CHECK(feretre->name == "Portador del Féretro Real");
        MOTOR_CHECK(feretre->rank == "elite");
        MOTOR_CHECK(feretre->tier == 5);
        MOTOR_CHECK(feretre->ca == 19);
        MOTOR_CHECK(feretre->maxHealth == 46);
        MOTOR_CHECK(feretre->movement == 5);
        MOTOR_CHECK(feretre->stats[0] == 9 && feretre->stats[1] == 8 &&
                    feretre->stats[2] == 7 && feretre->stats[3] == 7);
        MOTOR_CHECK(feretre->attacks.size() == 2);
    }

    const auto* constel = cat.find("pedazo_de_constelacion_primigenio");
    MOTOR_CHECK(constel != nullptr);
    if (constel) {
        MOTOR_CHECK(constel->name == "Pedazo de Constelación Primigenio");
        MOTOR_CHECK(constel->rank == "elite");
        MOTOR_CHECK(constel->tier == 5);
        MOTOR_CHECK(constel->ca == 18);
        MOTOR_CHECK(constel->maxHealth == 46);
        MOTOR_CHECK(constel->movement == 5);
        MOTOR_CHECK(constel->stats[0] == 7 && constel->stats[1] == 7 &&
                    constel->stats[2] == 10 && constel->stats[3] == 8);
        MOTOR_CHECK(constel->attacks.size() == 2);
    }
}

// Sala de control: la vida ara es llegeix de defenses{}/derived{}; si algú
// tornar a mirar-la a l'arrel del JSON, ca/defMental/resFis tornaran a
// fallar (aquesta prova no les frena, però l'error es veu a ulls: dels 432
// tots entren amb ca>=5 — ho deixa de complir al primer default).
MOTOR_TEST_CASE(D03_defenses_no_es_queden_al_default) {
    const auto cat = carrega_monstres();
    std::size_t total = 0;
    std::size_t al_default = 0;
    cat.forEach([&](const RPG::MonsterDefinition& m) {
        ++total;
        if (m.ca == 10 && m.defMental == 10 && m.resFis == 10) ++al_default;
    });
    MOTOR_CHECK(total == 432);
    MOTOR_CHECK(al_default == 0);
}

}  // namespace

int main() { return motor_test::runAllTests(); }