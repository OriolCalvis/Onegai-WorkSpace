// H2-03 (Gantt Fase H) — condiciones en el HUD, con rondas restantes.
//
// Dos mitades, a proposito:
//
//   1. Casos con ficha y catalogo ESCRITOS A MANO: las reglas de
//      presentacion (nombre del catalogo, rondas, stacks, marca de dano,
//      hasta-curarse, id desconocido, orden) sobre datos que decide el
//      test, no el motor.
//   2. Casos de INTEGRACION con el motor real: se aplica una condicion de
//      verdad con ConditionEngine::apply y se avanza con tick_conditions;
//      el widget ensena las rondas que el motor descuenta, no una copia
//      escrita dos veces.
//
// GL-free: sin ventana, sin assets, sin copiar_assets.
#include "TestHarness.h"

#include "HUD/ConditionWidget.h"
#include "RPG/ConditionEngine.h"

#include <string>
#include <vector>

using Game::ConditionWidget;
using RPG::ActiveCondition;
using RPG::Catalogs::ConditionCatalog;
using RPG::CharacterSheet;
using RPG::ConditionDefinition;
using RPG::ConditionEngine;

namespace {

ConditionDefinition defCon(const char* id, const char* name, int rounds, int dmg = 0,
                           ConditionDefinition::Duration mode =
                               ConditionDefinition::Duration::ROUNDS) {
    ConditionDefinition def;
    def.id = id;
    def.name = name;
    def.defaultRounds = rounds;
    def.damagePerRound = dmg;
    def.durationMode = mode;
    return def;
}

ActiveCondition activa(const char* id, int rounds, int stacks = 1) {
    ActiveCondition condition;
    condition.conditionId = id;
    condition.roundsRemaining = rounds;
    condition.stacks = stacks;
    return condition;
}

bool contiene(const std::vector<std::string>& lineas, const std::string& trozo) {
    for (const std::string& l : lineas) {
        if (l.find(trozo) != std::string::npos) {
            return true;
        }
    }
    return false;
}

}  // namespace

MOTOR_TEST_CASE(el_nombre_sale_del_catalogo_con_sus_rondas) {
    ConditionCatalog cat;
    cat.insert(defCon("veneno", "Veneno", 3, 2));
    ConditionWidget w;
    w.show("Enemigo: Rata de cloaca", {activa("veneno", 3)}, cat);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(w.visible());
    MOTOR_CHECK(w.shownCount() == 1);
    MOTOR_CHECK(lineas.size() == 2);
    MOTOR_CHECK(lineas[0] == "Enemigo: Rata de cloaca");
    MOTOR_CHECK(contiene(lineas, "Veneno"));
    MOTOR_CHECK(contiene(lineas, "3 rondas"));
}

MOTOR_TEST_CASE(una_ronda_en_singular) {
    ConditionCatalog cat;
    cat.insert(defCon("veneno", "Veneno", 3, 2));
    ConditionWidget w;
    w.show("Portador", {activa("veneno", 1)}, cat);
    MOTOR_CHECK(contiene(w.displayLines(), "1 ronda"));
    MOTOR_CHECK(!contiene(w.displayLines(), "1 rondas"));
}

MOTOR_TEST_CASE(los_stacks_se_ven_solo_si_hay_mas_de_uno) {
    ConditionCatalog cat;
    cat.insert(defCon("sangrado", "Sangrado", 5));
    ConditionWidget w;
    w.show("Portador", {activa("sangrado", 5, 3)}, cat);
    MOTOR_CHECK(contiene(w.displayLines(), "x3"));
    w.show("Portador", {activa("sangrado", 5, 1)}, cat);
    MOTOR_CHECK(!contiene(w.displayLines(), "x1"));
}

MOTOR_TEST_CASE(el_dano_por_ronda_se_ve) {
    ConditionCatalog cat;
    cat.insert(defCon("veneno", "Veneno", 3, 2));
    ConditionWidget w;
    w.show("Portador", {activa("veneno", 3)}, cat);
    MOTOR_CHECK(contiene(w.displayLines(), "(-2/ronda)"));
}

MOTOR_TEST_CASE(lo_que_no_hace_dano_no_lleva_marca) {
    ConditionCatalog cat;
    cat.insert(defCon("aturdido", "Aturdido", 1));
    ConditionWidget w;
    w.show("Portador", {activa("aturdido", 1)}, cat);
    MOTOR_CHECK(!contiene(w.displayLines(), "/ronda"));
}

MOTOR_TEST_CASE(hasta_curarse_no_miente_rondas) {
    ConditionCatalog cat;
    cat.insert(defCon("agarrado", "Agarrado", 3, 0,
                       ConditionDefinition::Duration::UNTIL_CURED));
    ConditionWidget w;
    // Aunque la ficha guarde rondas, esta condicion no expira por rondas:
    // pintar "3 rondas" mentiria. Manda el modo de duracion de la carta.
    w.show("Portador", {activa("agarrado", 3)}, cat);
    MOTOR_CHECK(contiene(w.displayLines(), "hasta curarse"));
    MOTOR_CHECK(!contiene(w.displayLines(), "3 rondas"));
}

MOTOR_TEST_CASE(un_id_que_no_esta_no_revienta) {
    ConditionCatalog cat;  // vacio a proposito
    ConditionWidget w;
    w.show("Portador", {activa("maldicion_rara", 2)}, cat);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 2);
    MOTOR_CHECK(contiene(lineas, "maldicion_rara"));
    MOTOR_CHECK(contiene(lineas, "2 rondas"));
}

MOTOR_TEST_CASE(se_respeta_el_orden_de_la_ficha) {
    ConditionCatalog cat;
    cat.insert(defCon("veneno", "Veneno", 3, 2));
    cat.insert(defCon("agarrado", "Agarrado", 3));
    ConditionWidget w;
    w.show("Portador", {activa("agarrado", 3), activa("veneno", 3)}, cat);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 3);
    MOTOR_CHECK(lineas[1].find("Agarrado") != std::string::npos);
    MOTOR_CHECK(lineas[2].find("Veneno") != std::string::npos);
}

MOTOR_TEST_CASE(sin_condiciones_lo_dice) {
    ConditionCatalog cat;
    ConditionWidget w;
    w.show("Portador", {}, cat);
    MOTOR_CHECK(w.visible());
    MOTOR_CHECK(w.shownCount() == 0);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(lineas.size() == 2);
    MOTOR_CHECK(lineas[1] == "(sin condiciones)");
}

MOTOR_TEST_CASE(mostrar_dos_veces_sustituye_y_ocultar_apaga) {
    ConditionCatalog cat;
    cat.insert(defCon("veneno", "Veneno", 3, 2));
    ConditionWidget w;
    MOTOR_CHECK(!w.visible());
    MOTOR_CHECK(w.displayLines().empty());
    w.show("Uno", {activa("veneno", 3)}, cat);
    w.show("Otro", {}, cat);
    MOTOR_CHECK(w.displayLines()[0] == "Otro");
    MOTOR_CHECK(contiene(w.displayLines(), "(sin condiciones)"));
    w.hide();
    MOTOR_CHECK(!w.visible());
    MOTOR_CHECK(w.shownCount() == 0);
    MOTOR_CHECK(w.displayLines().empty());
}

MOTOR_TEST_CASE(las_rondas_que_ensena_las_descuenta_el_motor) {
    // Integracion: la condicion la aplica el motor de verdad y el tick la
    // descuenta; el widget solo ensena lo que la ficha dice despues.
    ConditionCatalog cat;
    ConditionDefinition def = defCon("veneno", "Veneno", 3, 2);
    def.stacking = ConditionDefinition::Stacking::DURATION;
    cat.insert(def);
    CharacterSheet sheet;
    MOTOR_CHECK(ConditionEngine::apply(sheet, def, cat, "rata").isOk());
    ConditionWidget w;
    w.show("Enemigo: Rata de cloaca", sheet.conditions, cat);
    MOTOR_CHECK(contiene(w.displayLines(), "3 rondas"));
    (void)ConditionEngine::tick_conditions(sheet, cat);
    w.show("Enemigo: Rata de cloaca", sheet.conditions, cat);
    const std::vector<std::string> lineas = w.displayLines();
    MOTOR_CHECK(contiene(lineas, "2 rondas"));
    MOTOR_CHECK(!contiene(lineas, "3 rondas"));
}

int main() { return motor_test::runAllTests(); }
