#include "Game/InventoryMenuState.h"
#include "Game/InventoryScreen.h"

#include "TestHarness.h"

#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"
#include "RPG/InventoryEngine.h"

#include <algorithm>
#include <string>
#include <vector>

// G-12 — Inventario, equipo y ficha utilizables.
//
// La prueba del plan (test_inventory_menu): consumir una pocion UNA vez,
// equipar un arma a dos manos sin duplicar el bonus, y cancelar el
// destinatario sin consumir. Mas lo que el criterio exige de todas las
// acciones: lo imposible explica el motivo y no gasta ni cambia nada.
//
// Antes de este paquete GameSession::useItem/equipItem/unequipItem eran
// stubs vacios: cada caso de aqui falla con ellos.

namespace {

using Game::InventoryMenuState;
using Game::ItemCategory;

// Un cuerpo con ficha de verdad (Fractura #1: el jugador lleva su
// CharacterSheet) y vida que se puede curar.
class Heroe : public ICombatant {
public:
    RPG::CharacterSheet ficha;
    int vida = 40;
    int vidaMax = 60;

    RPG::CharacterSheet* character_sheet() override { return &ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return &ficha; }
    void takeDamage(int n) override { vida = std::max(0, vida - n); }
    void heal(int n) override { vida = std::min(vidaMax, vida + n); }
    int health() const override { return vida; }
    int maxHealth() const override { return vidaMax; }
    bool isAlive() const override { return vida > 0; }
    int stat(RPG::Stat s) const override { return ficha.stat(s); }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }
};

ObjectDefinition objeto(const std::string& id, const std::string& nombre, PickupEffect efecto,
                        int poder, int precio) {
    ObjectDefinition d;
    d.id = id;
    d.name = nombre;
    d.category = ObjectCategory::Pickup;
    d.pickup.effect = efecto;
    d.pickup.power = poder;
    d.pickup.price = precio;
    return d;
}

RPG::EquipmentDefinition pieza(const std::string& id, const std::string& nombre,
                               const std::string& ranura, int des, int armadura,
                               RPG::ItemRarity rareza = RPG::ItemRarity::Common) {
    RPG::EquipmentDefinition e;
    e.id = id;
    e.name = nombre;
    e.description = nombre + " de prueba.";
    e.slot = ranura;
    e.twoHanded = ranura == "TwoHanded";
    e.rarity = rareza;
    e.statBonuses = {0, des, 0, 0};
    e.caBonus = armadura;
    e.maxHealthBonus = ranura == "TwoHanded" ? 4 : 0;
    return e;
}

LevelDefinition nivel() {
    LevelDefinition l;
    l.name = "Prueba";
    l.playerStart = GridCoord{0, 0};
    return l;
}

struct Partida {
    ObjectCatalog objetos;
    SkillCatalog habilidades;
    RPG::Catalogs::EquipmentCatalog equipo;
    RPG::TierRules reglas;
    Heroe heroe;
    SkillSet skills{10};
    std::vector<std::string> mochila;
    GameSession sesion{nullptr, &objetos, &habilidades, nivel(), &heroe, &skills, &mochila};

    Partida() {
        objetos.add(objeto("pocion", "Pocion", PickupEffect::Heal, 15, 30));
        objetos.add(objeto("eter", "Eter", PickupEffect::RestoreMana, 5, 45));
        objetos.add(objeto("agua", "Agua de manantial", PickupEffect::Heal, 2, 1));
        objetos.add(objeto("tonico", "Tonico", PickupEffect::Heal, 8, 12));
        objetos.add(objeto("llave_cripta", "Llave de la cripta", PickupEffect::None, 0, 0));
        objetos.add(objeto("piedra", "Piedra bonita", PickupEffect::None, 0, 3));
        // El objeto de mochila de cada pieza, con el prefijo del generador.
        objetos.add(objeto("item_mandoble", "Mandoble", PickupEffect::None, 0, 200));
        objetos.add(objeto("item_daga", "Daga", PickupEffect::None, 0, 20));
        objetos.add(objeto("item_broquel", "Broquel", PickupEffect::None, 0, 25));
        objetos.add(objeto("item_corona_astral", "Corona astral", PickupEffect::None, 0, 900));

        equipo.insert(pieza("mandoble", "Mandoble", "TwoHanded", 2, 3));
        equipo.insert(pieza("daga", "Daga", "MainHand", 1, 0));
        equipo.insert(pieza("broquel", "Broquel", "OffHand", 0, 2));
        equipo.insert(pieza("corona_astral", "Corona astral", "Head", 3, 1, RPG::ItemRarity::Mythic));

        heroe.ficha.displayName = "Aina";
        heroe.ficha.tier = 1;
        auto r = sesion.inventoryRules();
        r.equipment = &equipo;
        r.tiers = &reglas;
        sesion.setInventoryRules(r);
    }

    std::size_t cuantos(const std::string& id) const {
        return static_cast<std::size_t>(std::count(mochila.begin(), mochila.end(), id));
    }
    const std::string& puesto(RPG::EquipSlot s) const {
        return heroe.ficha.equippedIds[static_cast<std::size_t>(s)];
    }
};

const Game::InventoryRow* fila(const InventoryMenuState& m, const std::string& id) {
    for (const auto& f : m.rows()) {
        if (f.objectId == id) return &f;
    }
    return nullptr;
}

}  // namespace

// ---------------------------------------------------------------------
// La prueba del plan
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(una_pocion_se_consume_una_sola_vez) {
    Partida p;
    p.mochila = {"pocion", "pocion", "eter"};
    InventoryMenuState menu(p.sesion);
    MOTOR_CHECK(menu.category() == ItemCategory::Consumable);
    MOTOR_CHECK(menu.rows().size() == 2);
    MOTOR_CHECK(fila(menu, "pocion")->quantity == 2);
    MOTOR_CHECK(fila(menu, "pocion")->description == "Cura 15 PV.");

    MOTOR_CHECK(menu.select("pocion"));
    MOTOR_CHECK(menu.requestUse().isOk());
    MOTOR_CHECK(menu.hasPendingTarget());
    MOTOR_CHECK(menu.targets() == std::vector<std::string>{"Aina"});
    MOTOR_CHECK(p.cuantos("pocion") == 2);  // pedir no gasta
    MOTOR_CHECK(menu.confirmTarget(0).isOk());

    MOTOR_CHECK(p.heroe.vida == 55);
    MOTOR_CHECK(p.cuantos("pocion") == 1);  // UNA, no las dos
    MOTOR_CHECK(fila(menu, "pocion")->quantity == 1);
    MOTOR_CHECK(!menu.hasPendingTarget());
    // Confirmar otra vez sin pedir no hace nada.
    MOTOR_CHECK(!menu.confirmTarget(0).isOk());
    MOTOR_CHECK(p.cuantos("pocion") == 1);

    // La segunda cura hasta el maximo, no por encima.
    MOTOR_CHECK(menu.requestUse().isOk());
    MOTOR_CHECK(menu.confirmTarget(0).isOk());
    MOTOR_CHECK(p.heroe.vida == 60);
    MOTOR_CHECK(p.cuantos("pocion") == 0);
    // Su fila desaparece y la seleccion pasa a la siguiente, no a nada raro.
    MOTOR_CHECK(fila(menu, "pocion") == nullptr);
    MOTOR_CHECK(menu.selectedId() == "eter");
}

MOTOR_TEST_CASE(un_arma_a_dos_manos_no_duplica_el_bonus_y_lo_desplazado_vuelve_a_la_mochila) {
    Partida p;
    p.mochila = {"item_daga", "item_broquel", "item_mandoble"};
    InventoryMenuState menu(p.sesion);
    menu.setCategory(ItemCategory::Equipment);
    MOTOR_CHECK(menu.rows().size() == 3);

    MOTOR_CHECK(menu.select("item_daga"));
    MOTOR_CHECK(menu.equipSelected().isOk());
    MOTOR_CHECK(menu.select("item_broquel"));
    MOTOR_CHECK(menu.equipSelected().isOk());
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::MainHand) == "daga");
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::OffHand) == "broquel");
    MOTOR_CHECK(p.mochila == std::vector<std::string>{"item_mandoble"});
    MOTOR_CHECK(p.heroe.ficha.stat(RPG::Stat::DES) == 1);

    // La vista previa lo cuenta antes de hacerlo, sin tocar nada.
    MOTOR_CHECK(menu.select("item_mandoble"));
    const auto previa = menu.previewSelected();
    MOTOR_CHECK(previa.isOk());
    MOTOR_CHECK(previa.value().statsBefore[1] == 1);
    MOTOR_CHECK(previa.value().statsAfter[1] == 2);   // +2, una sola vez
    MOTOR_CHECK(previa.value().armorBefore == 2);
    MOTOR_CHECK(previa.value().armorAfter == 3);
    auto desplazadas = previa.value().displaced;
    std::sort(desplazadas.begin(), desplazadas.end());
    MOTOR_CHECK(desplazadas == (std::vector<std::string>{"item_broquel", "item_daga"}));
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::MainHand) == "daga");
    MOTOR_CHECK(p.mochila.size() == 1);

    MOTOR_CHECK(menu.equipSelected().isOk());
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::MainHand) == "mandoble");
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::OffHand) == "mandoble");
    MOTOR_CHECK(p.heroe.ficha.stat(RPG::Stat::DES) == 2);       // no 4
    MOTOR_CHECK(p.heroe.ficha.equipArmorBonus + p.heroe.ficha.equipShieldBonus == 3);
    // Daga y broquel, de vuelta en la MOCHILA de la sesion (que es la que
    // ven tiendas y combate), no escondidos en la ficha.
    MOTOR_CHECK(p.cuantos("item_daga") == 1);
    MOTOR_CHECK(p.cuantos("item_broquel") == 1);
    MOTOR_CHECK(p.cuantos("item_mandoble") == 0);
    MOTOR_CHECK(p.heroe.ficha.inventoryEquipment.empty());
    // La ficha se refresca al instante: el tope de vida cuenta los +4 del
    // mandoble (y la mitad de su armadura, formula de healthCap).
    const int topeConMandoble = p.heroe.ficha.cachedHealthCap;

    // Desequipar la mano principal suelta las dos (es la misma pieza) y
    // devuelve UN mandoble.
    MOTOR_CHECK(menu.unequip(RPG::EquipSlot::MainHand).isOk());
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::MainHand).empty());
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::OffHand).empty());
    MOTOR_CHECK(p.cuantos("item_mandoble") == 1);
    MOTOR_CHECK(p.heroe.ficha.stat(RPG::Stat::DES) == 0);
    MOTOR_CHECK(p.heroe.ficha.inventoryEquipment.empty());
    // ...y al quitarselo el tope baja en el acto: sin refresco seguiria
    // contando una pieza que ya no se lleva.
    MOTOR_CHECK(p.heroe.ficha.cachedHealthCap == topeConMandoble - 4 - 1);
}

MOTOR_TEST_CASE(al_gastar_la_ultima_la_seleccion_pasa_a_la_siguiente) {
    // Filas por nombre: Agua, Pocion, Tonico. Gastar la unica Pocion no
    // deja el cursor en Agua (el primer hueco) sino en Tonico: la que
    // venia detras, que es lo que el jugador iba a mirar.
    Partida p;
    p.mochila = {"tonico", "pocion", "agua"};
    InventoryMenuState menu(p.sesion);
    MOTOR_CHECK(menu.rows().size() == 3);
    MOTOR_CHECK(menu.rows()[1].objectId == "pocion");
    MOTOR_CHECK(menu.select("pocion"));
    MOTOR_CHECK(menu.requestUse().isOk());
    MOTOR_CHECK(menu.confirmTarget(0).isOk());
    MOTOR_CHECK(menu.rows().size() == 2);
    MOTOR_CHECK(menu.selectedId() == "tonico");
    MOTOR_CHECK(*menu.selectedRow() == 1);
    // Y moverse por la lista se queda en los bordes.
    menu.moveSelection(+5);
    MOTOR_CHECK(menu.selectedId() == "tonico");
    menu.moveSelection(-5);
    MOTOR_CHECK(menu.selectedId() == "agua");
}

MOTOR_TEST_CASE(cancelar_el_destinatario_no_gasta_nada) {
    Partida p;
    p.mochila = {"pocion", "eter"};
    p.skills = SkillSet(10);
    MOTOR_CHECK(p.sesion.useItem("eter").isOk() == false);  // mana lleno: no se gasta
    MOTOR_CHECK(p.cuantos("eter") == 1);

    InventoryMenuState menu(p.sesion);
    MOTOR_CHECK(menu.select("pocion"));
    MOTOR_CHECK(menu.requestUse().isOk());
    // Mientras se elige destinatario, las flechas no mueven la lista.
    menu.moveSelection(+1);
    MOTOR_CHECK(menu.selectedId() == "pocion");
    menu.cancelTarget();
    MOTOR_CHECK(!menu.hasPendingTarget());
    MOTOR_CHECK(menu.message() == "Cancelado.");
    MOTOR_CHECK(p.cuantos("pocion") == 1);
    MOTOR_CHECK(p.heroe.vida == 40);
    // Cambiar de pestaña a mitad tambien cancela.
    MOTOR_CHECK(menu.requestUse().isOk());
    menu.setCategory(ItemCategory::Equipment);
    MOTOR_CHECK(!menu.hasPendingTarget());
    MOTOR_CHECK(p.cuantos("pocion") == 1);
    // Un destinatario que no existe tampoco gasta.
    menu.setCategory(ItemCategory::Consumable);
    MOTOR_CHECK(menu.select("pocion"));
    MOTOR_CHECK(menu.requestUse().isOk());
    MOTOR_CHECK(!menu.confirmTarget(3).isOk());
    MOTOR_CHECK(menu.hasPendingTarget());
    MOTOR_CHECK(p.cuantos("pocion") == 1);
}

// ---------------------------------------------------------------------
// Lo imposible explica el motivo y no cambia nada
// ---------------------------------------------------------------------

MOTOR_TEST_CASE(lo_imposible_dice_por_que_y_no_toca_nada) {
    Partida p;
    p.heroe.vida = 60;  // llena
    p.mochila = {"pocion", "llave_cripta", "item_corona_astral", "piedra"};
    InventoryMenuState menu(p.sesion);

    const auto* pocion = fila(menu, "pocion");
    MOTOR_CHECK(!pocion->use.allowed);
    MOTOR_CHECK(pocion->use.reason.find("vida al maximo") != std::string::npos);
    MOTOR_CHECK(menu.select("pocion"));
    const auto r = menu.requestUse();
    MOTOR_CHECK(!r.isOk());
    MOTOR_CHECK(!menu.hasPendingTarget());
    MOTOR_CHECK(menu.message() == pocion->use.reason);
    MOTOR_CHECK(p.cuantos("pocion") == 1);

    // Categorias: la llave es clave (sin efecto ni precio), la piedra otra cosa.
    MOTOR_CHECK(menu.countIn(ItemCategory::Consumable) == 1);
    MOTOR_CHECK(menu.countIn(ItemCategory::Quest) == 1);
    MOTOR_CHECK(menu.countIn(ItemCategory::Equipment) == 1);
    MOTOR_CHECK(menu.countIn(ItemCategory::Other) == 1);
    menu.setCategory(ItemCategory::Quest);
    const auto* llave = fila(menu, "llave_cripta");
    MOTOR_CHECK(!llave->use.allowed && !llave->equip.allowed);
    MOTOR_CHECK(llave->use.reason.find("ningun efecto") != std::string::npos);
    MOTOR_CHECK(p.sesion.useItem("llave_cripta").isOk() == false);
    MOTOR_CHECK(p.cuantos("llave_cripta") == 1);

    // Una pieza que el tier no admite: el motivo es el de InventoryEngine,
    // y la ficha y la mochila quedan igual.
    menu.setCategory(ItemCategory::Equipment);
    const auto* corona = fila(menu, "item_corona_astral");
    MOTOR_CHECK(!corona->equip.allowed);
    MOTOR_CHECK(corona->equip.reason.find("Corona astral") != std::string::npos);
    MOTOR_CHECK(corona->use.reason.find("se equipa") != std::string::npos);
    MOTOR_CHECK(menu.select("item_corona_astral"));
    MOTOR_CHECK(!menu.equipSelected().isOk());
    MOTOR_CHECK(!menu.previewSelected().isOk());
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::Head).empty());
    MOTOR_CHECK(p.cuantos("item_corona_astral") == 1);
    MOTOR_CHECK(p.heroe.ficha.inventoryEquipment.empty());
    MOTOR_CHECK(p.heroe.ficha.stat(RPG::Stat::DES) == 0);

    // Desequipar una ranura vacia.
    const auto& cabeza = menu.equipment()[0];
    MOTOR_CHECK(cabeza.label == "Cabeza" && cabeza.name == "(vacío)");
    MOTOR_CHECK(!cabeza.unequip.allowed);
    MOTOR_CHECK(cabeza.unequip.reason.find("la cabeza") != std::string::npos);
    MOTOR_CHECK(!menu.unequip(RPG::EquipSlot::Head).isOk());

    // Algo que no se lleva.
    MOTOR_CHECK(p.sesion.equipItem("item_daga").errorMessage().find("no llevas") != std::string::npos);
    MOTOR_CHECK(p.sesion.useItem("eter").errorMessage().find("no llevas") != std::string::npos);
}

MOTOR_TEST_CASE(sin_reglas_de_equipo_o_con_la_partida_ocupada_no_se_toca_nada) {
    Partida p;
    p.mochila = {"item_daga", "pocion"};
    p.sesion.setInventoryRules({});
    MOTOR_CHECK(p.sesion.whyCannotEquip("item_daga") == "no hay catalogo de equipo cargado");
    // Sin catalogo de equipo, la daga no se reconoce como pieza.
    MOTOR_CHECK(!p.sesion.equipItem("item_daga").isOk());
    MOTOR_CHECK(p.cuantos("item_daga") == 1);

    // Con una tienda abierta no se usa nada (y al cerrarla, si).
    Partida q;
    ObjectDefinition tendero;
    tendero.id = "tendero";
    tendero.name = "Tendero";
    tendero.category = ObjectCategory::Npc;
    tendero.shop.items = {ShopItem{"pocion", 0}};
    q.objetos.add(tendero);
    q.mochila = {"pocion", "item_daga"};
    MOTOR_CHECK(q.sesion.openShop("tendero").isOk());
    MOTOR_CHECK(!q.sesion.useItem("pocion").isOk());
    MOTOR_CHECK(q.sesion.whyCannotEquip("item_daga").find("tienda") != std::string::npos);
    MOTOR_CHECK(q.cuantos("pocion") == 1 && q.heroe.vida == 40);
    q.sesion.closeInteraction();
    // Con la partida en pausa (desde donde se abre el menu), si.
    MOTOR_CHECK(q.sesion.pause());
    MOTOR_CHECK(q.sesion.useItem("pocion").isOk());
    MOTOR_CHECK(q.sesion.equipItem("item_daga").isOk());
    MOTOR_CHECK(q.mochila.empty());
}

MOTOR_TEST_CASE(el_equipo_puesto_se_ensena_por_ranura_con_su_nombre) {
    Partida p;
    p.mochila = {"item_mandoble"};
    MOTOR_CHECK(p.sesion.equipItem("item_mandoble").isOk());
    InventoryMenuState menu(p.sesion);
    MOTOR_CHECK(menu.equipment().size() == RPG::kRanurasDeEquipo);  // PART-02: once ranuras
    const auto& mano = menu.equipment()[4];
    MOTOR_CHECK(mano.label == "Mano principal");
    MOTOR_CHECK(mano.name == "Mandoble");
    MOTOR_CHECK(mano.unequip.allowed);
    MOTOR_CHECK(menu.equipment()[5].name == "Mandoble");  // ocupa las dos manos
    MOTOR_CHECK(menu.countIn(ItemCategory::Equipment) == 0);
    // El log de la partida lo cuenta.
    MOTOR_CHECK(std::find(p.sesion.log().begin(), p.sesion.log().end(), "Te equipas Mandoble.") !=
                p.sesion.log().end());
}

MOTOR_TEST_CASE(el_puente_de_ids_objeto_pieza) {
    MOTOR_CHECK(GameSession::equipmentIdFor("item_mandoble") == "mandoble");
    MOTOR_CHECK(GameSession::equipmentIdFor("mandoble") == "mandoble");
    MOTOR_CHECK(GameSession::equipmentIdFor("item_") == "item_");
    Partida p;
    MOTOR_CHECK(p.sesion.objectIdForEquipment("daga") == "item_daga");
    // Pieza sin objeto con prefijo: vuelve con su propio id.
    MOTOR_CHECK(p.sesion.objectIdForEquipment("anillo_suelto") == "anillo_suelto");
    MOTOR_CHECK(p.sesion.equipmentFor("item_broquel")->name == "Broquel");
    MOTOR_CHECK(p.sesion.equipmentFor("pocion") == nullptr);
}

// Con los catalogos publicados: todas las piezas de equipment.json tienen su
// objeto "item_<id>" en libreria_armas.json, y un personaje sin clase
// equipa una comun de verdad.
// TUTORIAL-01 — Lo que el equipo sube del tope de vida se suma a la de ahora y lo que baja se quita: el dano que
// llevas no cambia (el kit del cofre dejaba al heroe a 5/9 al empezar; quitarse la armadura, a 35/29).
// Quitarse algo no mata: como poco, 1.
MOTOR_TEST_CASE(la_vida_sigue_al_tope_al_equipar_y_desequipar) {
    struct HeroeConTope : Heroe {
        int maxHealth() const override { return ficha.healthCap(); }
        void heal(int n) override { vida = std::min(maxHealth(), vida + n); }
    };
    ObjectCatalog objetos;
    objetos.add(objeto("item_mandoble", "Mandoble", PickupEffect::None, 0, 200));
    SkillCatalog habilidades;
    RPG::Catalogs::EquipmentCatalog equipo;
    equipo.insert(pieza("mandoble", "Mandoble", "TwoHanded", 2, 3));  // sube el tope de vida
    RPG::TierRules reglas;
    HeroeConTope heroe;
    heroe.ficha.tier = 1;
    heroe.ficha.recompute_derived(reglas, nullptr);
    SkillSet skills{0};
    std::vector<std::string> mochila{"item_mandoble"};
    GameSession sesion{nullptr, &objetos, &habilidades, nivel(), &heroe, &skills, &mochila};
    auto r = sesion.inventoryRules();
    r.equipment = &equipo;
    r.tiers = &reglas;
    sesion.setInventoryRules(r);
    const int sin = heroe.maxHealth();
    heroe.vida = sin;  // entero, como al empezar
    MOTOR_CHECK(sesion.equipItem("item_mandoble").isOk());
    const int con = heroe.maxHealth();
    MOTOR_CHECK(con > sin);
    MOTOR_CHECK(heroe.vida == con);  // sigue entero
    heroe.vida = con - 3;             // herido
    MOTOR_CHECK(sesion.unequipItem(RPG::EquipSlot::MainHand).isOk());
    MOTOR_CHECK(heroe.maxHealth() == sin && heroe.vida == sin - 3);  // el mismo dano
    MOTOR_CHECK(sesion.equipItem("item_mandoble").isOk());
    MOTOR_CHECK(heroe.vida == con - 3);
    heroe.vida = 1;  // casi muerto: quitarse el mandoble no le mata
    MOTOR_CHECK(sesion.unequipItem(RPG::EquipSlot::MainHand).isOk());
    MOTOR_CHECK(heroe.vida == 1);
}

MOTOR_TEST_CASE(el_puente_cubre_el_contenido_publicado) {
    ObjectCatalog objetos;
    const auto o = objetos.loadFromFile("assets/objects/libreria_armas.json");
    if (!o.isOk()) MOTOR_CHECK_FAIL("libreria_armas.json: " + o.errorMessage());
    RPG::Catalogs::EquipmentCatalog equipo;
    const auto e = equipo.loadFromFile("assets/catalogs/equipment.json");
    if (!e.isOk()) MOTOR_CHECK_FAIL("equipment.json: " + e.errorMessage());
    std::size_t sinObjeto = 0;
    std::string comun;
    equipo.forEach([&](const RPG::EquipmentDefinition& d) {
        if (objetos.find("item_" + d.id) == nullptr) ++sinObjeto;
        if (comun.empty() && d.rarity == RPG::ItemRarity::Common && d.slot == "MainHand") comun = d.id;
    });
    MOTOR_CHECK(equipo.size() >= 413);  // SP-02: + los trajes de los especialistas
    MOTOR_CHECK(sinObjeto == 0);
    MOTOR_CHECK(!comun.empty());

    SkillCatalog habilidades;
    Heroe heroe;
    SkillSet skills{0};
    std::vector<std::string> mochila{"item_" + comun};
    GameSession sesion{nullptr, &objetos, &habilidades, nivel(), &heroe, &skills, &mochila};
    RPG::TierRules reglas;
    auto r = sesion.inventoryRules();
    r.equipment = &equipo;
    r.tiers = &reglas;
    sesion.setInventoryRules(r);
    const auto puesto = sesion.equipItem("item_" + comun);
    if (!puesto.isOk()) MOTOR_CHECK_FAIL(comun + ": " + puesto.errorMessage());
    MOTOR_CHECK(heroe.ficha.equippedIds[static_cast<std::size_t>(RPG::EquipSlot::MainHand)] == comun);
    MOTOR_CHECK(mochila.empty());
}

MOTOR_TEST_CASE(vista_equipa_y_desequipa_sin_duplicar_y_conserva_la_ranura) {
    Partida p;
    p.mochila = {"item_daga", "item_daga"};
    Game::InventoryScreen screen(p.sesion);
    MOTOR_CHECK(screen.inventoryLabels().size() == 1);
    MOTOR_CHECK(screen.inventoryLabels()[0] == "Daga x2");
    MOTOR_CHECK(screen.confirm() == Game::InventoryAction::EquipItem);
    MOTOR_CHECK(screen.panelText().find("Equipar") != std::string::npos);
    MOTOR_CHECK(screen.executeSelected().isOk());
    MOTOR_CHECK(p.cuantos("item_daga") == 1);
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::MainHand) == "daga");
    screen.moveTabRight();
    for (int i = 0; i < static_cast<int>(RPG::EquipSlot::MainHand); ++i) screen.moveDown();
    MOTOR_CHECK(screen.confirm() == Game::InventoryAction::UnequipItem);
    MOTOR_CHECK(screen.panelText().find("Desequipar") != std::string::npos);
    MOTOR_CHECK(screen.executeSelected().isOk());
    MOTOR_CHECK(p.cuantos("item_daga") == 2);
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::MainHand).empty());
    MOTOR_CHECK(screen.selectedInFocus() == static_cast<std::size_t>(RPG::EquipSlot::MainHand));
    MOTOR_CHECK(screen.confirm() == Game::InventoryAction::None);
    MOTOR_CHECK(!screen.executeSelected().isOk());
    MOTOR_CHECK(p.cuantos("item_daga") == 2);
    screen.moveTabLeft();
    MOTOR_CHECK(screen.inventoryLabels()[0] == "Daga x2");
}

MOTOR_TEST_CASE(vista_pocion_pide_destino_y_cancelar_no_gasta) {
    Partida p;
    p.mochila = {"pocion", "pocion"};
    Game::InventoryScreen screen(p.sesion);
    MOTOR_CHECK(screen.confirm() == Game::InventoryAction::UseItem);
    MOTOR_CHECK(screen.executeSelected().isOk());
    MOTOR_CHECK(p.cuantos("pocion") == 2);
    MOTOR_CHECK(screen.confirm() == Game::InventoryAction::ConfirmTarget);
    screen.moveDown();
    MOTOR_CHECK(screen.confirm() == Game::InventoryAction::CancelTarget);
    MOTOR_CHECK(screen.executeSelected().isOk());
    MOTOR_CHECK(p.heroe.vida == 40);
    MOTOR_CHECK(p.cuantos("pocion") == 2);
    MOTOR_CHECK(screen.executeSelected().isOk());
    MOTOR_CHECK(screen.executeSelected().isOk());
    MOTOR_CHECK(p.heroe.vida == 55);
    MOTOR_CHECK(p.cuantos("pocion") == 1);
    MOTOR_CHECK(screen.inventoryLabels()[0] == "Pocion x1");
}

MOTOR_TEST_CASE(vista_muestra_el_motivo_de_equipo_denegado_y_no_gasta) {
    Partida p;
    p.mochila = {"item_corona_astral"};
    Game::InventoryScreen screen(p.sesion);
    MOTOR_CHECK(screen.confirm() == Game::InventoryAction::EquipItem);
    const auto result = screen.executeSelected();
    MOTOR_CHECK(!result.isOk());
    MOTOR_CHECK(!screen.message().empty());
    MOTOR_CHECK(screen.panelText().find(result.errorMessage()) != std::string::npos);
    MOTOR_CHECK(p.cuantos("item_corona_astral") == 1);
    MOTOR_CHECK(p.puesto(RPG::EquipSlot::Head).empty());
}

MOTOR_TEST_CASE(vista_cambiar_pestana_cancela_destino_y_el_cursor_siempre_es_visible) {
    Partida p;
    p.mochila = {"pocion", "agua", "eter", "tonico", "item_daga", "llave_cripta"};
    Game::InventoryScreen screen(p.sesion);
    for (int i = 0; i < 4; ++i) screen.moveDown();
    const auto selected = screen.selectedEntry();
    MOTOR_CHECK(screen.panelText(2).find("> " + selected.label) != std::string::npos);
    screen.moveTabRight();
    screen.moveTabLeft(); // vuelve al primer consumible
    MOTOR_CHECK(screen.executeSelected().isOk());
    screen.moveTabRight();
    screen.moveTabLeft();
    MOTOR_CHECK(screen.confirm() == Game::InventoryAction::UseItem);
    MOTOR_CHECK(p.mochila.size() == 6);
}

int main() { return motor_test::runAllTests(); }
