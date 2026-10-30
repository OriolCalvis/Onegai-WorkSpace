// TIENDA-02 — Game::TiendaEstado: la tienda en dos pestañas, sin ventana.
//
// Lo que se comprueba es lo que el jugador ve y paga: que comprar y vender
// estén separados (E29), que lo que la hoja dice que te pagan sea lo que
// cobras (E30), que sin oro no se compre y se diga cuánto falta, que el
// resultado salga en la propia tienda (E18) y que el foco no se salga de la
// lista cuando vendes lo último que te quedaba de algo.
#include "TestHarness.h"

#include "Game/GameSession.h"
#include "Game/TiendaEstado.h"
#include "Level/ObjectCatalog.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <string>
#include <vector>

using Game::PestanaTienda;
using Game::TiendaEstado;

namespace {

class Cuerpo : public ICombatant {
public:
    void takeDamage(int amount) override { m_vida = std::max(0, m_vida - amount); }
    void heal(int amount) override { m_vida = std::min(60, m_vida + amount); }
    int health() const override { return m_vida; }
    int maxHealth() const override { return 60; }
    bool isAlive() const override { return m_vida > 0; }
    int stat(RPG::Stat) const override { return 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "test"; }

private:
    int m_vida = 60;
};

LevelDefinition nivel() {
    LevelDefinition level;
    level.name = "Mercado";
    level.playerStart = GridCoord{0, 0};
    return level;
}

ObjectDefinition objeto(const char* id, const char* nombre, int precio, PickupEffect efecto = PickupEffect::None,
                        int poder = 0) {
    ObjectDefinition o;
    o.id = id;
    o.name = nombre;
    o.category = ObjectCategory::Pickup;
    o.pickup.price = precio;
    o.pickup.effect = efecto;
    o.pickup.power = poder;
    return o;
}

// Un tendero que vende la poción (30) y el pan (8, el del catálogo) y recompra a mitad de precio.
struct Fixture {
    ObjectCatalog catalog;
    SkillCatalog skills;
    Cuerpo cuerpo;
    SkillSet habilidades{10};
    std::vector<std::string> mochila;

    Fixture() {
        catalog.add(objeto("pocion", "Poción", 30, PickupEffect::Heal, 15));
        catalog.add(objeto("eter", "Éter", 45, PickupEffect::RestoreMana, 10));
        catalog.add(objeto("pan", "Pan", 8));
        catalog.add(objeto("llave", "Llave del sótano", 0));
        ObjectDefinition tendero;
        tendero.id = "tendero";
        tendero.name = "Tendero del mercado";
        tendero.category = ObjectCategory::Npc;
        tendero.dialogue.lines.push_back("¡Bienvenido al mercado!");
        tendero.shop.items.push_back(ShopItem{"pocion", 30});
        tendero.shop.items.push_back(ShopItem{"pan", 0});
        tendero.shop.buybackPercent = 50;
        catalog.add(tendero);
    }

    GameSession sesion(int oro) {
        GameSession s(nullptr, &catalog, &skills, nivel(), &cuerpo, &habilidades, &mochila);
        s.addGold(oro);
        MOTOR_CHECK(s.openShop("tendero").isOk());
        return s;
    }
};

}  // namespace

MOTOR_TEST_CASE(la_tienda_abre_en_comprar_con_el_foco_arriba_y_sin_resultado) {
    Fixture f;
    GameSession s = f.sesion(100);
    TiendaEstado t;
    t.sigue(true);
    MOTOR_CHECK(t.pestana() == PestanaTienda::Comprar);
    MOTOR_CHECK(t.foco(2) == 0);
    MOTOR_CHECK(t.estado().empty() && !t.estadoError());
    const auto filas = TiendaEstado::filas(s, PestanaTienda::Comprar);
    MOTOR_CHECK(filas.size() == 2);
    MOTOR_CHECK(filas[0].nombre == "Poción" && filas[0].precio == 30 && filas[0].curaPv == 15);
    MOTOR_CHECK(filas[1].nombre == "Pan" && filas[1].precio == 8 && filas[1].curaPv == 0);
}

MOTOR_TEST_CASE(comprar_cobra_entrega_y_dice_cuantas_tienes_en_la_tienda) {
    Fixture f;
    GameSession s = f.sesion(100);
    f.mochila = {"pocion"};
    TiendaEstado t;
    t.sigue(true);
    t.confirma(s);
    MOTOR_CHECK(s.gold() == 70);
    MOTOR_CHECK(std::count(f.mochila.begin(), f.mochila.end(), "pocion") == 2);
    MOTOR_CHECK(t.estado() == "Comprado: Poción · –30 de oro · ahora tienes 2");
    MOTOR_CHECK(!t.estadoError());
}

MOTOR_TEST_CASE(sin_oro_no_compra_y_dice_cuanto_falta) {
    Fixture f;
    GameSession s = f.sesion(12);
    TiendaEstado t;
    t.sigue(true);
    t.confirma(s);
    MOTOR_CHECK(s.gold() == 12);
    MOTOR_CHECK(f.mochila.empty());
    MOTOR_CHECK(t.estado() == "Te faltan 18 de oro.");
    MOTOR_CHECK(t.estadoError());
    t.mueve(+1, 2);  // el pan (8) si llega
    t.confirma(s);
    MOTOR_CHECK(s.gold() == 4);
    MOTOR_CHECK(!t.estadoError());
}

MOTOR_TEST_CASE(vender_va_en_su_pestana_agrupado_y_solo_con_lo_que_tiene_precio) {
    Fixture f;
    GameSession s = f.sesion(0);
    f.mochila = {"pocion", "llave", "eter", "pocion"};
    const auto filas = TiendaEstado::filas(s, PestanaTienda::Vender);
    MOTOR_CHECK(filas.size() == 2);  // la llave no tiene precio: no se ofrece
    MOTOR_CHECK(filas[0].objectId == "pocion" && filas[0].tienes == 2 && filas[0].precio == 15);
    MOTOR_CHECK(filas[0].indice == 0);
    MOTOR_CHECK(filas[1].objectId == "eter" && filas[1].tienes == 1 && filas[1].precio == 22);
    MOTOR_CHECK(filas[1].indice == 2);
}

MOTOR_TEST_CASE(lo_que_la_hoja_dice_que_te_pagan_es_lo_que_cobras) {
    Fixture f;
    GameSession s = f.sesion(0);
    f.mochila = {"eter", "pocion", "pan"};
    for (const char* id : {"eter", "pocion", "pan"}) {
        const int antes = s.gold();
        const int dice = s.sellPriceOf(id);
        const auto it = std::find(f.mochila.begin(), f.mochila.end(), id);
        MOTOR_CHECK(s.sell(static_cast<std::size_t>(it - f.mochila.begin())));
        MOTOR_CHECK(s.gold() - antes == dice);
    }
    MOTOR_CHECK(s.sellPriceOf("llave") == 0);
    MOTOR_CHECK(s.sellPriceOf("pan") == 4);
}

MOTOR_TEST_CASE(vender_cobra_y_lo_dice_y_el_foco_no_se_sale_al_vender_lo_ultimo) {
    Fixture f;
    GameSession s = f.sesion(0);
    f.mochila = {"pocion", "eter"};
    TiendaEstado t;
    t.sigue(true);
    t.cambiaPestana(+1);
    MOTOR_CHECK(t.pestana() == PestanaTienda::Vender);
    t.mueve(+5, 2);  // al fondo: el éter
    MOTOR_CHECK(t.foco(2) == 1);
    t.confirma(s);
    MOTOR_CHECK(s.gold() == 22);
    MOTOR_CHECK(t.estado() == "Vendido: Éter · +22 de oro · ahora tienes 0");
    const auto quedan = TiendaEstado::filas(s, PestanaTienda::Vender);
    MOTOR_CHECK(quedan.size() == 1);
    MOTOR_CHECK(t.foco(quedan.size()) == 0);  // la fila del éter se ha ido: el foco sube
    t.confirma(s);
    MOTOR_CHECK(f.mochila.empty());
    MOTOR_CHECK(s.gold() == 37);
    MOTOR_CHECK(TiendaEstado::filas(s, PestanaTienda::Vender).empty());
    t.confirma(s);  // sin filas no hace nada
    MOTOR_CHECK(s.gold() == 37);
}

MOTOR_TEST_CASE(cada_pestana_recuerda_su_foco_y_cambiar_borra_el_resultado) {
    Fixture f;
    GameSession s = f.sesion(5);
    f.mochila = {"pocion", "eter", "pan"};
    TiendaEstado t;
    t.sigue(true);
    t.mueve(+1, 2);
    t.confirma(s);  // pan: 8 > 5
    MOTOR_CHECK(t.estadoError());
    t.cambiaPestana(+1);
    MOTOR_CHECK(t.estado().empty() && !t.estadoError());
    t.mueve(+2, 3);
    t.cambiaPestana(+1);  // ya en Vender: no da la vuelta
    MOTOR_CHECK(t.pestana() == PestanaTienda::Vender);
    t.cambiaPestana(-1);
    MOTOR_CHECK(t.foco(2) == 1);  // Comprar sigue en el pan
    t.cambiaPestana(+1);
    MOTOR_CHECK(t.foco(3) == 2);
}

MOTOR_TEST_CASE(cerrar_y_volver_a_abrir_empieza_de_cero) {
    Fixture f;
    GameSession s = f.sesion(100);
    TiendaEstado t;
    t.sigue(true);
    t.cambiaPestana(+1);
    t.ponFoco(7, 3);
    MOTOR_CHECK(t.foco(3) == 2);
    t.sigue(true);  // sigue abierta: nada cambia
    MOTOR_CHECK(t.pestana() == PestanaTienda::Vender);
    t.sigue(false);
    t.sigue(true);
    MOTOR_CHECK(t.pestana() == PestanaTienda::Comprar);
    MOTOR_CHECK(t.foco(3) == 0);
}

// El evento comercio.abrir es ya la escena: abre la tienda sin saludo (hablar con
// el tendero, en cambio, saluda antes: GameModeTests).
MOTOR_TEST_CASE(comercio_abrir_va_derecho_a_la_tienda) {
    Fixture f;
    GameSession s(nullptr, &f.catalog, &f.skills, nivel(), &f.cuerpo, &f.habilidades, &f.mochila);
    MOTOR_CHECK(s.openShop("tendero").isOk());
    MOTOR_CHECK(s.mode() == GameMode::Shop);
    MOTOR_CHECK(s.dialogueLines().empty());
    s.closeInteraction();
    MOTOR_CHECK(s.mode() == GameMode::Exploration);
}

int main() { return motor_test::runAllTests(); }
