// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo toca 1 paquete del corte B:
//   C-24  (fase C, 5 j) - Xarxa de seguretat al camí que es juga
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// C-24 — Xarxa de seguretat: flujo de compra/alquiler del negocio.
//
// Complemento de BusinessEconomyTests.cpp (esas son las piezas puras
// occupancyPercent/incomeFor y el cobro via PlayerCarryOver). Aqui se
// prueba el tramo GameMode que ahi se dejo escrito como "futur test
// d'integracio": abrir el cartel con interact(), comprar, y mover la
// renta con su moral asociada -- todo a traves del estado real de la
// GameSession, sin mapa (como GameSessionTests.cpp).
//
// Los valores esperados salen del propio codigo (src/Game/GameSession.cpp
// en el corte actual):
//   - interact() barre la celda del jugador + las 4 contiguas y entra en
//     Business mode para objetos con business.price > 0.
//   - buyCurrentBusiness() descuenta el precio y anade OwnedBusiness al
//     100% de alquiler.
//   - adjustCurrentRent() mueve LA MORAL POR EL CAMBIO neto (-applied/5),
//     no por el nivel absoluto: subir y bajar en bucle no cultiva
//     reputacion. En el tope (kMinRent/kMaxRent) devuelve false.
//   - collectRent() suma incomeFor y anade el sostenido (100-rent)/25.

#include "TestHarness.h"

#include "Game/GameSession.h"
#include "Level/ObjectCatalog.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

class TestCombatant : public ICombatant {
public:
    explicit TestCombatant(int maxHealth) : m_health(maxHealth), m_maxHealth(maxHealth) {}

    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::min(m_maxHealth, m_health + amount); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }

    int stat(RPG::Stat /*s*/) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }

private:
    int m_health;
    int m_maxHealth;
};

// Nivel con UN cartel de negocio justo en la celda del jugador: un
// decorado con el que se puede interactuar sin que bloquee el paso.
LevelDefinition makeLevel() {
    LevelDefinition level;
    level.name = "Ciudad";
    level.playerStart = GridCoord{0, 0};
    ObjectSpawn sign;
    sign.objectId = "posada";
    sign.position = GridCoord{0, 0};
    level.objects.push_back(sign);
    return level;
}

struct Fixture {
    ObjectCatalog catalog;
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;

    Fixture() {
        ObjectDefinition sign;
        sign.id = "posada";
        sign.name = "Posada";
        sign.category = ObjectCategory::Prop;
        sign.business.price = 60;
        sign.business.baseIncome = 12;
        sign.business.businessName = "Posada del Cuervo";
        catalog.add(sign);
    }

    GameSession makeSession() {
        return GameSession(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills,
                           &inventory);
    }
};

}  // namespace

// --- Apertura del cartel (interact en modo Exploration) ---

MOTOR_TEST_CASE(interactuar_con_un_cartel_abre_el_modo_negocio) {
    Fixture f;
    GameSession s = f.makeSession();
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
    MOTOR_CHECK(s.interact());
    MOTOR_CHECK(s.mode() == GameMode::Business);
    MOTOR_CHECK(s.currentBusinessId() == "posada");
    MOTOR_CHECK(s.currentBusinessName() == "Posada del Cuervo");
    MOTOR_CHECK(s.currentBusinessPrice() == 60);
    MOTOR_CHECK(s.currentBusinessIncome() == 12);
}

MOTOR_TEST_CASE(sin_cartel_no_hay_modo_negocio) {
    LevelDefinition empty;
    empty.name = "Campo";
    empty.playerStart = GridCoord{0, 0};

    Fixture f;
    ObjectCatalog emptyCatalog;
    GameSession s =
        GameSession(nullptr, &emptyCatalog, &f.skills, empty, &f.body, &f.playerSkills, &f.inventory);
    MOTOR_CHECK(!s.interact());
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
}

MOTOR_TEST_CASE(interactuar_fuera_de_exploracion_no_abre_cartel) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();                // entra en Business
    MOTOR_CHECK(s.mode() == GameMode::Business);
    MOTOR_CHECK(!s.interact());  // ya no es Exploration: no re-asegura nada
}

// --- Compra ---

MOTOR_TEST_CASE(comprar_el_negocio_descuenta_el_precio_y_lo_anade_al_100) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(90);
    MOTOR_CHECK(s.interact());
    MOTOR_CHECK(s.buyCurrentBusiness());
    MOTOR_CHECK(s.gold() == 30);
    MOTOR_CHECK(s.businesses().size() == 1);
    const OwnedBusiness& bought = s.businesses()[0];
    MOTOR_CHECK(bought.objectId == "posada");
    MOTOR_CHECK(bought.name == "Posada del Cuervo");
    MOTOR_CHECK(bought.baseIncome == 12);
    MOTOR_CHECK(bought.rentPercent == 100);
    MOTOR_CHECK(s.ownsBusiness("posada"));
    MOTOR_CHECK(s.mode() == GameMode::Business);  // comprar no cierra el cartel
}

MOTOR_TEST_CASE(comprar_sin_dinero_suficiente_falla_y_no_roza_el_oro) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(30);
    s.interact();
    MOTOR_CHECK(!s.buyCurrentBusiness());
    MOTOR_CHECK(s.gold() == 30);
    MOTOR_CHECK(s.businesses().empty());
}

MOTOR_TEST_CASE(el_mismo_negocio_no_se_compra_dos_veces) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(200);
    s.interact();
    MOTOR_CHECK(s.buyCurrentBusiness());
    MOTOR_CHECK(!s.buyCurrentBusiness());
    MOTOR_CHECK(s.businesses().size() == 1);
    MOTOR_CHECK(s.gold() == 200 - 60);
}

MOTOR_TEST_CASE(cerrado_el_cartel_ya_no_se_puede_comprar) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();
    s.closeInteraction();
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
    MOTOR_CHECK(s.currentBusinessId().empty());
    MOTOR_CHECK(!s.buyCurrentBusiness());
    MOTOR_CHECK(s.businesses().empty());
}

// --- Renta: la moral se mueve con el CAMBIO, no con el nivel ---

MOTOR_TEST_CASE(subir_la_renta_cuesta_moral_por_el_cambio) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();
    s.buyCurrentBusiness();

    MOTOR_CHECK(s.adjustCurrentRent(50));
    MOTOR_CHECK(s.businesses()[0].rentPercent == 150);
    MOTOR_CHECK(s.morality().value() == -10);  // -50/5
}

MOTOR_TEST_CASE(bajar_la_renta_acerca_a_heroe) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();
    s.buyCurrentBusiness();

    MOTOR_CHECK(s.adjustCurrentRent(-50));
    MOTOR_CHECK(s.businesses()[0].rentPercent == 50);
    MOTOR_CHECK(s.morality().value() == 10);  // +50/5
}

MOTOR_TEST_CASE(subir_y_bajar_en_bucle_no_cultiva_moral) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();
    s.buyCurrentBusiness();

    s.adjustCurrentRent(30);   // -6
    s.adjustCurrentRent(-30);  // +6: neto cero
    MOTOR_CHECK(s.businesses()[0].rentPercent == 100);
    MOTOR_CHECK(s.morality().value() == 0);
}

MOTOR_TEST_CASE(la_renta_se_acota_a_los_extremos_y_el_tope_no_devuelve_cambio) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();
    s.buyCurrentBusiness();

    MOTOR_CHECK(s.adjustCurrentRent(500));
    MOTOR_CHECK(s.businesses()[0].rentPercent == GameSession::kMaxRent);
    MOTOR_CHECK(s.morality().value() == -20);  // (200-100)/5 aplicado
    MOTOR_CHECK(!s.adjustCurrentRent(10));     // ya en el tope: sin moral extra
    MOTOR_CHECK(s.morality().value() == -20);

    MOTOR_CHECK(s.adjustCurrentRent(-1000));
    MOTOR_CHECK(s.businesses()[0].rentPercent == GameSession::kMinRent);
    MOTOR_CHECK(!s.adjustCurrentRent(-10));    // ya en el minimo
}

MOTOR_TEST_CASE(ajustar_la_renta_de_un_negocio_ajeno_no_funciona) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();  // cartel abierto pero SIN comprarlo
    MOTOR_CHECK(!s.adjustCurrentRent(10));
    MOTOR_CHECK(s.businesses().empty());
}

// --- Cobro de rentas tras la compra (integración del flujo entero) ---

MOTOR_TEST_CASE(cobrar_con_alquiler_justo_cobra_el_ingreso_base) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();
    s.buyCurrentBusiness();  // oro 40

    const int cobrado = s.collectRent();
    MOTOR_CHECK(cobrado == 12);  // incomeFor(12, 100)
    MOTOR_CHECK(s.gold() == 40 + 12);
    MOTOR_CHECK(s.morality().value() == 0);  // alquiler justo: sin sostenido
}

MOTOR_TEST_CASE(cobrar_con_alquiler_abusivo_cuesta_moral_recurrente) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();
    s.buyCurrentBusiness();
    s.adjustCurrentRent(50);  // 150%: sostenido -2 por ciclo

    const int cobrado = s.collectRent();
    MOTOR_CHECK(cobrado == 14);  // incomeFor(12, 150)
    // -10 del cambio de subir + -2 del sostenido:
    MOTOR_CHECK(s.morality().value() == -12);
    MOTOR_CHECK(s.gold() == 40 + 14);
}

MOTOR_TEST_CASE(cobrar_con_alquiler_generoso_premia_la_moral) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(100);
    s.interact();
    s.buyCurrentBusiness();
    s.adjustCurrentRent(-50);  // 50%: sostenido +2 por ciclo

    s.collectRent();
    MOTOR_CHECK(s.gold() == 40 + 6);  // incomeFor(12, 50)
    MOTOR_CHECK(s.morality().value() == 10 + 2);
}

MOTOR_TEST_CASE(sin_negocios_el_cobro_devuelve_cero) {
    Fixture f;
    GameSession s = f.makeSession();
    s.addGold(50);
    MOTOR_CHECK(s.collectRent() == 0);
    MOTOR_CHECK(s.gold() == 50);
}

int main() { return motor_test::runAllTests(); }