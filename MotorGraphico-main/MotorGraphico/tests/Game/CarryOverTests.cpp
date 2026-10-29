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

// C-24 — Xarxa de seguretat: el traspàs entre sesiones.
//
// C-24 depende de C-03, cuyo criterio es: "Moral, negocis, passos des de
// l'últim cobrament i or es transfereixen sencers entre sessions. El
// traspàs és una sola funció". Este fichero pina esa invariante contra
// las dos funciones que la materializan (captureCarryOver /
// applyCarryOver — las mismas que usa SaveGame.cpp) y contra la parte
// GL-free de "el guardat": un negocio comprado y su renta sobreviven el
// cambio de nivel, y el cobro siguiente aplica el sostenido desde la
// renta traspasada.
//
// Registros del corte actual (src/Game/GameSession.cpp 247-265):
//   - captureCarryOver() copia gold, morality, stepsSinceRent y
//     businesses.
//   - applyCarryOver() ASIGNA (no acumula), clampa el oro a >=0 y
//     traspasa la moral con setValue (sin motivo: cruzar una puerta no
//     es accion moral).

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

    // Compra la posada y sube el alquiler a 150%: oro 40, moral -10,
    // negocio con renta 150.
    GameSession sessionConNegocioEsperando() {
        GameSession s = makeSession();
        s.addGold(100);
        s.interact();
        s.buyCurrentBusiness();
        s.adjustCurrentRent(50);
        return s;
    }
};

}  // namespace

MOTOR_TEST_CASE(capturar_traspasa_oro_y_moral_enteros) {
    Fixture f;
    GameSession s = f.sessionConNegocioEsperando();  // oro 40, moral -10

    const PlayerCarryOver state = s.captureCarryOver();
    MOTOR_CHECK(state.gold == 40);
    MOTOR_CHECK(state.moralityValue == -10);
    MOTOR_CHECK(state.businesses.size() == 1);
}

MOTOR_TEST_CASE(el_negocio_comprado_viaja_intacto_en_el_traspaso) {
    Fixture f;
    GameSession s = f.sessionConNegocioEsperando();

    GameSession destino = f.makeSession();
    destino.applyCarryOver(s.captureCarryOver());

    MOTOR_CHECK(destino.gold() == 40);
    MOTOR_CHECK(destino.morality().value() == -10);
    MOTOR_CHECK(destino.businesses().size() == 1);
    const OwnedBusiness& b = destino.businesses()[0];
    MOTOR_CHECK(b.objectId == "posada");
    MOTOR_CHECK(b.name == "Posada del Cuervo");
    MOTOR_CHECK(b.baseIncome == 12);
    MOTOR_CHECK(b.rentPercent == 150);
    MOTOR_CHECK(destino.ownsBusiness("posada"));
}

MOTOR_TEST_CASE(traspasar_setValue_no_registra_motivo_en_el_historial) {
    Fixture f;
    GameSession s = f.sessionConNegocioEsperando();  // -10 con un motivo
    MOTOR_CHECK(!s.morality().history().empty());

    GameSession destino = f.makeSession();  // historial vacio de fabrica
    destino.applyCarryOver(s.captureCarryOver());

    MOTOR_CHECK(destino.morality().value() == -10);
    // Solo viaja el VALOR; el historial no se arrastra ni se ensucia:
    // aplicar el traspaso no es una accion moral.
    MOTOR_CHECK(destino.morality().history().empty());
}

MOTOR_TEST_CASE(el_traspaso_asigna_y_no_acumula) {
    Fixture f;
    GameSession destino = f.makeSession();
    destino.addGold(200);
    destino.morality().record(50, "regalo");

    PlayerCarryOver state;
    state.gold = 7;
    state.moralityValue = 0;
    destino.applyCarryOver(state);

    MOTOR_CHECK(destino.gold() == 7);        // 200 + 7 != 7: asigna
    MOTOR_CHECK(destino.morality().value() == 0);
}

MOTOR_TEST_CASE(traspasar_oro_negativo_se_clampa_a_cero) {
    Fixture f;
    GameSession destino = f.makeSession();
    destino.addGold(50);

    PlayerCarryOver state;
    state.gold = -30;
    destino.applyCarryOver(state);

    MOTOR_CHECK(destino.gold() == 0);
}

MOTOR_TEST_CASE(el_cobro_despues_del_traspaso_parte_de_la_renta_traspasada) {
    Fixture f;
    GameSession sesion = f.sessionConNegocioEsperando();  // -10 y renta 150

    GameSession destino = f.makeSession();
    destino.applyCarryOver(sesion.captureCarryOver());
    const int cobrado = destino.collectRent();  // incomeFor(12,150)=14, sostenido (100-150)/25=-2

    MOTOR_CHECK(cobrado == 14);
    MOTOR_CHECK(destino.gold() == 40 + 14);
    MOTOR_CHECK(destino.morality().value() == -10 - 2);
}

MOTOR_TEST_CASE(traspaso_vacio_resetea_el_estado) {
    Fixture f;
    GameSession destino = f.makeSession();
    destino.applyCarryOver(f.sessionConNegocioEsperando().captureCarryOver());
    MOTOR_CHECK(!destino.businesses().empty());

    PlayerCarryOver vacio;  // todo por defecto
    destino.applyCarryOver(vacio);

    MOTOR_CHECK(destino.gold() == 0);
    MOTOR_CHECK(destino.morality().value() == 0);
    MOTOR_CHECK(destino.businesses().empty());
}

int main() { return motor_test::runAllTests(); }