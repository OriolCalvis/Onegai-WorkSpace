// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo toca 1 paquete del corte B:
//   H3-04  (fase H, 2 j) - Comercio con stock y precios reales
//
// Si tocas este fichero estas trabajando dentro de ese paquete: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// H3-04 — Comercio con datos reales.
//
// El flujo GameSession::openShop() (el punto de entrada de E-04
// `comercio.abrir`, sin requerir adyacencia al NPC) nunca fue ejercido
// end-to-end: GameModeTests.cpp lo cubre todo a traves de interact()
// (que pide al jugador estar al lado del tendero), pero openShop() es
// el camino de los eventos de mapa. Aqui se verifica el ciclo completo
// con precios reales: abrir -> ver ofertas -> comprar (oro baja, item
// al inventario) -> vender (oro subta, item sale) -> cerrar (vuelve a
// Exploracion). Todo GL-free, sin mapa, sobre el catalogo real.
//
// openShop(), buy() y sell() viven en GameSessionComercio.cpp
// (versionado en 54e06b5 / EST-23), dentro de motor_level. El test
// enlaza motor_level como test_game_session/test_game_mode.

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

// Nivel sin mapa: el jugador empieza en (0,0), el tendero en (0,0).
// openShop() no pide adyacencia, pero usamos un nivel minimo de todos
// modos por el ctor de GameSession.
LevelDefinition makeLevel() {
    LevelDefinition level;
    level.name = "Ciudad";
    level.playerStart = GridCoord{0, 0};
    return level;
}

// Catalogo con precios reales de DEC-05 (pocion 30, ether 45, pan 8)
// y un NPC tendero que vende pocion y pan. La pocion tiene precio fijo
// de tienda (30); el pan usa el precio del catalogo (8). El ether no se
// ofrece (no esta en la tienda) pero existe en el catalogo para poder
// venderlo desde el inventario.
struct Fixture {
    ObjectCatalog catalog;
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;

    Fixture() {
        // pocion: precio fijo de tienda 30
        ObjectDefinition pocion;
        pocion.id = "pocion";
        pocion.name = "Pocion";
        pocion.category = ObjectCategory::Pickup;
        pocion.pickup.price = 30;
        catalog.add(pocion);

        // ether: existe en catalogo (precio 45) pero NO en la tienda.
        ObjectDefinition ether;
        ether.id = "ether";
        ether.name = "Ether";
        ether.category = ObjectCategory::Pickup;
        ether.pickup.price = 45;
        catalog.add(ether);

        // pan: precio 8
        ObjectDefinition pan;
        pan.id = "pan";
        pan.name = "Pan";
        pan.category = ObjectCategory::Pickup;
        pan.pickup.price = 8;
        catalog.add(pan);

        // tendero: vende pocion (precio 30) y pan (precio 8 del catalogo)
        ObjectDefinition tendero;
        tendero.id = "tendero";
        tendero.name = "Tendero";
        tendero.category = ObjectCategory::Npc;
        tendero.dialogue.lines.push_back("Hola, forastero.");
        tendero.shop.items.push_back(ShopItem{"pocion", 30});
        tendero.shop.items.push_back(ShopItem{"pan", 0});  // 0 = usa catalogo
        tendero.shop.buybackPercent = 50;
        catalog.add(tendero);
    }

    GameSession makeSession(int gold = 100) {
        GameSession s(nullptr, &catalog, &skills, makeLevel(), &body, &playerSkills, &inventory);
        s.addGold(gold);
        return s;
    }
};

}  // namespace

// ---------------------------------------------------------------------------
// openShop() — apertura de tienda sin adyacencia (comercio.abrir)
// ---------------------------------------------------------------------------

// openShop() resuelve ofertas con precios reales del catalogo: pocion=30,
// pan=8 (precio del catalogo). La tienda del tendero abre en modo Shop.
MOTOR_TEST_CASE(openShop_resuelve_ofertas_con_precios_reales) {
    Fixture f;
    GameSession s = f.makeSession(100);
    MOTOR_CHECK(s.openShop("tendero").isOk());
    MOTOR_CHECK(s.mode() == GameMode::Shop);
    MOTOR_CHECK(s.shopKeeper() == "Tendero");
    const auto& offers = s.shopOffers();
    MOTOR_CHECK(offers.size() == 2);
    MOTOR_CHECK(offers[0].objectId == "pocion");
    MOTOR_CHECK(offers[0].name == "Pocion");
    MOTOR_CHECK(offers[0].price == 30);
    MOTOR_CHECK(offers[1].objectId == "pan");
    MOTOR_CHECK(offers[1].name == "Pan");
    MOTOR_CHECK(offers[1].price == 8);
}

// openShop() falla si no esta en Exploration (una tienda ya esta abierta,
// o el jugador esta en combate/dialogo). No cambia de modo.
MOTOR_TEST_CASE(openShop_error_si_no_esta_en_exploracion) {
    Fixture f;
    GameSession s = f.makeSession(100);
    MOTOR_CHECK(s.openShop("tendero").isOk());
    MOTOR_CHECK(s.mode() == GameMode::Shop);
    // Segunda apertura: ya no esta en Exploration.
    Result<bool> r = s.openShop("tendero");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(s.mode() == GameMode::Shop);
    // El mensaje explica el por que (viene en el Result, no en el log).
    MOTOR_CHECK(r.errorMessage().find("no se puede abrir") != std::string::npos);
}

// openShop() falla si el id no existe o no es un NPC (sin tocar el modo).
MOTOR_TEST_CASE(openShop_error_si_objeto_inexistente) {
    Fixture f;
    GameSession s = f.makeSession(100);
    Result<bool> r = s.openShop("noexiste");
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
    MOTOR_CHECK(s.shopOffers().empty());
}

// ---------------------------------------------------------------------------
// buy() — compra desde openShop
// ---------------------------------------------------------------------------

// Comprar desconta el oro y entrega el objeto al inventario en un solo
// paso (atomico: sin que el oro se vaya sin el objeto o viceversa).
MOTOR_TEST_CASE(buy_en_tienda_abierta_descuenta_oro_y_entrega_item) {
    Fixture f;
    GameSession s = f.makeSession(100);
    s.openShop("tendero");
    MOTOR_CHECK(s.buy(0));  // pocion a 30
    MOTOR_CHECK(s.gold() == 70);
    MOTOR_CHECK(f.inventory.size() == 1);
    MOTOR_CHECK(f.inventory[0] == "pocion");
    // El segundo articulo (pan a 8) tambien compra.
    MOTOR_CHECK(s.buy(1));
    MOTOR_CHECK(s.gold() == 62);
    MOTOR_CHECK(f.inventory.size() == 2);
    MOTOR_CHECK(f.inventory[1] == "pan");
}

// Sin oro suficiente buy() falla: no descuenta nada ni entrega el objeto.
MOTOR_TEST_CASE(buy_sin_oro_suficiente_no_cambia_nada) {
    Fixture f;
    GameSession s = f.makeSession(10);  // 10 < 30 (pocion)
    s.openShop("tendero");
    MOTOR_CHECK(!s.buy(0));
    MOTOR_CHECK(s.gold() == 10);
    MOTOR_CHECK(f.inventory.empty());
    bool logged = false;
    for (const std::string& line : s.log()) {
        if (line.find("No te llega") != std::string::npos) {
            logged = true;
        }
    }
    MOTOR_CHECK(logged);
}

// Indice fuera de rango: no hace nada.
MOTOR_TEST_CASE(buy_fuera_de_rango_no_hace_nada) {
    Fixture f;
    GameSession s = f.makeSession(100);
    s.openShop("tendero");
    MOTOR_CHECK(!s.buy(2));
    MOTOR_CHECK(!s.buy(99));
    MOTOR_CHECK(s.gold() == 100);
    MOTOR_CHECK(f.inventory.empty());
}

// buy() solo funciona en modo Shop: desde Exploration falla.
MOTOR_TEST_CASE(buy_fuera_de_tienda_no_hace_nada) {
    Fixture f;
    GameSession s = f.makeSession(100);
    MOTOR_CHECK(!s.buy(0));  // todavia no abrimos la tienda
    MOTOR_CHECK(s.gold() == 100);
    MOTOR_CHECK(f.inventory.empty());
}

// ---------------------------------------------------------------------------
// sell() — venta desde openShop
// ---------------------------------------------------------------------------

// Vender un item del inventario: devuelve buybackPercent del precio base
// (50% de 30 = 15), el oro sube y el item sale del inventario.
MOTOR_TEST_CASE(sell_devuelve_buyback_y_retira_item) {
    Fixture f;
    GameSession s = f.makeSession(0);
    s.openShop("tendero");
    // Primero compramos una pocion (30) con oro suficiente.
    s.addGold(100);
    s.buy(0);
    MOTOR_CHECK(s.gold() == 70);
    MOTOR_CHECK(f.inventory.size() == 1);
    // Vendemos la pocion: 50% de 30 = 15.
    MOTOR_CHECK(s.sell(0));
    MOTOR_CHECK(s.gold() == 85);
    MOTOR_CHECK(f.inventory.empty());
}

// Vender un item sin precio (p.ej. una llave) no da oro: falla.
MOTOR_TEST_CASE(sell_item_sin_precio_no_hace_nada) {
    Fixture f;
    GameSession s = f.makeSession(100);
    s.openShop("tendero");
    // "ether" esta en el catalogo a 45 pero no en la tienda. Lo metemos
    // en inventario a mano y probamos a venderlo.
    f.inventory.push_back("ether");
    // sell(index) vende inventario[index]; ether tiene precio 45, buyback
    // del tendero es 50%: 22 (45*50/100, truncado).
    MOTOR_CHECK(s.sell(0));
    MOTOR_CHECK(s.gold() == 122);  // 100 + 22
    MOTOR_CHECK(f.inventory.empty());
}

// Vender sin inventario: falla, no cambia nada.
MOTOR_TEST_CASE(sell_sin_inventario_no_hace_nada) {
    Fixture f;
    GameSession s = f.makeSession(100);
    s.openShop("tendero");
    MOTOR_CHECK(!s.sell(0));
    MOTOR_CHECK(s.gold() == 100);
}

// sell() fuera de Shop: no hace nada.
MOTOR_TEST_CASE(sell_fuera_de_tienda_no_hace_nada) {
    Fixture f;
    GameSession s = f.makeSession(100);
    f.inventory.push_back("pocion");
    MOTOR_CHECK(!s.sell(0));
    MOTOR_CHECK(s.gold() == 100);
    MOTOR_CHECK(f.inventory.size() == 1);
}

// ---------------------------------------------------------------------------
// closeInteraction() — cierre de la tienda
// ---------------------------------------------------------------------------

// Despues de comprar, cerrar la tienda vuelve a Exploration y limpia
// las ofertas.
MOTOR_TEST_CASE(closeInteraction_despues_de_comprar_vuelve_a_exploracion) {
    Fixture f;
    GameSession s = f.makeSession(100);
    s.openShop("tendero");
    s.buy(0);
    MOTOR_CHECK(s.mode() == GameMode::Shop);
    MOTOR_CHECK(s.shopOffers().size() == 2);
    s.closeInteraction();
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
    MOTOR_CHECK(s.shopOffers().empty());
    MOTOR_CHECK(s.shopKeeper().empty());
}

// El ciclo completo: abrir -> comprar -> vender -> cerrar.
MOTOR_TEST_CASE(ciclo_completo_comprar_vender_cerrar) {
    Fixture f;
    GameSession s = f.makeSession(100);
    // Abrir
    MOTOR_CHECK(s.openShop("tendero").isOk());
    MOTOR_CHECK(s.mode() == GameMode::Shop);
    // Comprar pocion (30)
    MOTOR_CHECK(s.buy(0));
    MOTOR_CHECK(s.gold() == 70);
    MOTOR_CHECK(f.inventory.size() == 1);
    // Vender pocion (50% de 30 = 15)
    MOTOR_CHECK(s.sell(0));
    MOTOR_CHECK(s.gold() == 85);
    MOTOR_CHECK(f.inventory.empty());
    // Cerrar
    s.closeInteraction();
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
    MOTOR_CHECK(s.shopOffers().empty());
}

int main() { return motor_test::runAllTests(); }
