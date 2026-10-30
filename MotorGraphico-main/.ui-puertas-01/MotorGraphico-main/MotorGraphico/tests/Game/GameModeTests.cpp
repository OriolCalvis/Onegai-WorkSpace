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

// C-24 — Xarxa de seguretat: maquina de estados del GameMode.
//
// Exploracion -> Dialogo, Dialogo<->Tienda, y GameOver de salida cuando
// el jugador arranca muerto. Son las transiciones que el cierre de C-24
// pide cubrir (junto a BusinessModeTests.cpp para el negocio y
// MoralityTests.cpp para la moral). Todo GL-free, sin mapa, a traves de
// la GameSession real.
//
// Registros del corte actual (src/Game/GameSession.cpp):
//   - startInteraction(): NPC -> speaker + dialogo; resuelve shopOffers
//     AHORA (precio de la tienda o del catalogo; <=0 se descarta; articulo
//     desconocido se salta con log); mode = Shop si hay ofertas, si no
//     Dialogue.
//   - buy(i): solo en Shop; cobra y entrega en el mismo paso.
//   - sell(i): solo en Shop; paga max(1, base*buybackPercent/100) con el
//     buybackPercent del NPC (por nombre); retira el item.
//   - closeInteraction(): Dialogue/Shop/Business -> Exploration.
//   - ctor con ICombatant muerto -> GameOver de salida.

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

// Nivel con UNA entidad en la celda del jugador: la que diga el test.
LevelDefinition makeLevel(const std::string& objectId) {
    LevelDefinition level;
    level.name = "Ciudad";
    level.playerStart = GridCoord{0, 0};
    ObjectSpawn spawn;
    spawn.objectId = objectId;
    spawn.position = GridCoord{0, 0};
    level.objects.push_back(spawn);
    return level;
}

struct Fixture {
    ObjectCatalog catalog;
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;

    Fixture() {
        ObjectDefinition pocoion;
        pocoion.id = "pocion";
        pocoion.name = "Pocion";
        pocoion.category = ObjectCategory::Pickup;
        pocoion.pickup.price = 50;
        catalog.add(pocoion);

        ObjectDefinition hierba;
        hierba.id = "hierba";
        hierba.name = "Hierba";
        hierba.category = ObjectCategory::Pickup;
        hierba.pickup.price = 0;  // sin precio: no se ofrece
        catalog.add(hierba);

        ObjectDefinition tendero;
        tendero.id = "tendero";
        tendero.name = "Tendero";
        tendero.category = ObjectCategory::Npc;
        tendero.dialogue.lines.push_back("Hola, forastero.");
        tendero.shop.items.push_back(ShopItem{"pocion", 0});  // usa el precio del catalogo
        tendero.shop.buybackPercent = 50;
        catalog.add(tendero);

        ObjectDefinition herbolario;
        herbolario.id = "herbolario";
        herbolario.name = "Herbolario";
        herbolario.category = ObjectCategory::Npc;
        herbolario.dialogue.lines.push_back("Buenas.");
        catalog.add(herbolario);

        // NPC cuyo UNICO articulo no tiene precio: tienda que se queda en
        // dialogo puro.
        ObjectDefinition sinPrecio;
        sinPrecio.id = "camarero";
        sinPrecio.name = "Camarero";
        sinPrecio.category = ObjectCategory::Npc;
        sinPrecio.dialogue.lines.push_back("Solo hablo.");
        sinPrecio.shop.items.push_back(ShopItem{"hierba", 0});
        catalog.add(sinPrecio);

        // NPC que ofrece un articulo que no existe en el catalogo.
        ObjectDefinition fantasma;
        fantasma.id = "vendedor";
        fantasma.name = "Vendedor";
        fantasma.category = ObjectCategory::Npc;
        fantasma.shop.items.push_back(ShopItem{"noexiste", 10});
        catalog.add(fantasma);

        // NPC mudo: ni dialogo ni tienda.
        ObjectDefinition mudo;
        mudo.id = "estatua";
        mudo.name = "Estatua";
        mudo.category = ObjectCategory::Npc;
        catalog.add(mudo);
    }

    GameSession makeSession(const std::string& objectId, int health = 60) {
        TestCombatant* player = nullptr;
        // GameSession no posee al combatiente: usamos uno local de pila si
        // hace falta vivo, o uno muerto para el caso GameOver.
        if (health <= 0) {
            m_dead.emplace_back(0);
            player = &m_dead.back();
        } else {
            m_alive.emplace_back(health);
            player = &m_alive.back();
        }
        return GameSession(nullptr, &catalog, &skills, makeLevel(objectId), player, &playerSkills,
                           &inventory);
    }

private:
    std::vector<TestCombatant> m_dead;
    std::vector<TestCombatant> m_alive;
};

}  // namespace

// --- Exploracion <-> Dialogo ---

MOTOR_TEST_CASE(interactuar_con_un_npc_sin_tienda_abre_dialogo) {
    Fixture f;
    GameSession s = f.makeSession("herbolario");
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
    MOTOR_CHECK(s.interact());
    MOTOR_CHECK(s.mode() == GameMode::Dialogue);
    MOTOR_CHECK(s.dialogueSpeaker() == "Herbolario");
    MOTOR_CHECK(s.shopKeeper() == "Herbolario");
    MOTOR_CHECK(s.dialogueLines().size() == 1);
    MOTOR_CHECK(s.dialogueLines()[0] == "Buenas.");
    MOTOR_CHECK(s.shopOffers().empty());
}

MOTOR_TEST_CASE(cerrar_el_dialogo_vuelve_a_exploracion_y_limpia) {
    Fixture f;
    GameSession s = f.makeSession("herbolario");
    s.interact();
    s.closeInteraction();
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
    MOTOR_CHECK(s.dialogueLines().empty());
    MOTOR_CHECK(s.dialogueSpeaker().empty());
    MOTOR_CHECK(s.shopOffers().empty());
}

MOTOR_TEST_CASE(un_npc_mudo_abre_un_dialogo_vacio) {
    Fixture f;
    GameSession s = f.makeSession("estatua");
    MOTOR_CHECK(s.interact());
    MOTOR_CHECK(s.mode() == GameMode::Dialogue);
    MOTOR_CHECK(s.dialogueLines().empty());
    MOTOR_CHECK(s.shopOffers().empty());
}

// --- Tienda ---

// TIENDA-02 — Hablar con un tendero: primero lo que dice (Oriol, 08/10: «un
// minimo de dialogo todos») y, al cerrar la hoja, la tienda.
void abreTienda(GameSession& s) {
    s.interact();
    s.closeInteraction();
}

MOTOR_TEST_CASE(interactuar_con_tienda_saluda_y_al_cerrar_abre_la_tienda) {
    Fixture f;
    GameSession s = f.makeSession("tendero");
    s.addGold(100);
    MOTOR_CHECK(s.interact());
    MOTOR_CHECK(s.mode() == GameMode::Dialogue);
    MOTOR_CHECK(s.dialogueLines().size() == 1 && s.dialogueLines()[0] == "Hola, forastero.");
    MOTOR_CHECK(!s.buy(0));  // desde el saludo no se compra
    s.closeInteraction();
    MOTOR_CHECK(s.mode() == GameMode::Shop);
    MOTOR_CHECK(s.shopKeeper() == "Tendero");
    MOTOR_CHECK(s.shopOffers().size() == 1);
    MOTOR_CHECK(s.shopOffers()[0].objectId == "pocion");
    MOTOR_CHECK(s.shopOffers()[0].name == "Pocion");
    MOTOR_CHECK(s.shopOffers()[0].price == 50);
}

MOTOR_TEST_CASE(comprar_descuenta_el_precio_y_entrega_el_objeto) {
    Fixture f;
    GameSession s = f.makeSession("tendero");
    s.addGold(100);
    abreTienda(s);
    MOTOR_CHECK(s.buy(0));
    MOTOR_CHECK(s.gold() == 50);
    MOTOR_CHECK(f.inventory.size() == 1);
    MOTOR_CHECK(f.inventory[0] == "pocion");
}

MOTOR_TEST_CASE(comprar_sin_dinero_falla_y_no_entrega_nada) {
    Fixture f;
    GameSession s = f.makeSession("tendero");
    s.addGold(10);
    abreTienda(s);
    MOTOR_CHECK(!s.buy(0));
    MOTOR_CHECK(s.gold() == 10);
    MOTOR_CHECK(f.inventory.empty());
}

MOTOR_TEST_CASE(comprar_fuera_de_rango_no_hace_nada) {
    Fixture f;
    GameSession s = f.makeSession("tendero");
    s.addGold(100);
    abreTienda(s);
    MOTOR_CHECK(!s.buy(1));
    MOTOR_CHECK(!s.buy(3));
    MOTOR_CHECK(f.inventory.empty());
}

MOTOR_TEST_CASE(comprar_desde_el_dialogo_no_hace_nada) {
    Fixture f;
    GameSession s = f.makeSession("herbolario");
    s.addGold(100);
    s.interact();  // Dialogue
    MOTOR_CHECK(s.mode() == GameMode::Dialogue);
    MOTOR_CHECK(!s.buy(0));
    MOTOR_CHECK(f.inventory.empty());
}

MOTOR_TEST_CASE(vender_devuelve_el_buyback_en_oro_y_retira_el_item) {
    Fixture f;
    GameSession s = f.makeSession("tendero");
    s.addGold(100);
    abreTienda(s);
    s.buy(0);                       // oro 50, pocion en inventario
    MOTOR_CHECK(s.sell(0));         // 50 * 50/100 = 25
    MOTOR_CHECK(s.gold() == 75);
    MOTOR_CHECK(f.inventory.empty());
}

MOTOR_TEST_CASE(vender_sin_inventario_no_hace_nada) {
    Fixture f;
    GameSession s = f.makeSession("tendero");
    s.addGold(100);
    abreTienda(s);
    MOTOR_CHECK(!s.sell(0));
    MOTOR_CHECK(s.gold() == 100);
}

MOTOR_TEST_CASE(vender_desde_el_dialogo_no_hace_nada) {
    Fixture f;
    GameSession s = f.makeSession("herbolario");
    s.interact();  // Dialogue
    MOTOR_CHECK(!s.sell(0));
}

// --- Casos de contenido mal escrito ---

MOTOR_TEST_CASE(una_tienda_sin_precio_resuelto_queda_en_dialogo) {
    Fixture f;
    GameSession s = f.makeSession("camarero");
    s.interact();
    MOTOR_CHECK(s.mode() == GameMode::Dialogue);  // hierba sin precio: no es oferta
    MOTOR_CHECK(s.shopOffers().empty());
}

MOTOR_TEST_CASE(un_articulo_desconocido_se_salta_con_registro) {
    Fixture f;
    GameSession s = f.makeSession("vendedor");
    s.interact();
    MOTOR_CHECK(s.mode() == GameMode::Dialogue);  // noexiste no esta en el catalogo
    MOTOR_CHECK(s.shopOffers().empty());
    bool logged = false;
    for (const std::string& line : s.log()) {
        if (line.find("noexiste") != std::string::npos) {
            logged = true;
        }
    }
    MOTOR_CHECK(logged);
}

// --- GameOver ---

MOTOR_TEST_CASE(un_jugador_muerto_al_cargar_es_gameover_de_salida) {
    Fixture f;
    GameSession s = f.makeSession("tendero", 0);
    MOTOR_CHECK(s.mode() == GameMode::GameOver);
    MOTOR_CHECK(!s.interact());  // GameOver no explora
    MOTOR_CHECK(!s.buy(0));
}

int main() { return motor_test::runAllTests(); }