// >>> PAQUETES DEL PLAN <<<
// Refuerzo de C-24 (xarxa de seguretat al camí que es juga): el
// ObjectCatalog («todo es un objeto», Phase 10) NO lo cita ningún paquete
// del plan (MAPA_DEL_CODIGO, tabla 3) pero es la base de datos de objetos
// del camino jugado (la ciudad, sus tiendas, sus negocios y sus melenas).
// Esta red fija el catálogo real de `assets/objects/ciudad_objetos.json`
// (121 objetos, el que carga demo_ciudad; 97 hasta TIENDA-03, que añadió la
// daga que vende Aigren, y 98 hasta INSTRUMENTOS-01, que añadió los 22 instrumentos
// y objetos de rol que venden las tiendas y las ganzúas) contra regresión.
//
// Compromisos del loader (src/Level/ObjectCatalog.cpp) que se fijan aqui:
//   - Categorias estrictas prop/enemy/pickup/npc: una desconocida es ERROR
//     estructural, nunca un Prop silencioso.
//   - Efectos de pickup estrictos none/heal/restoreMana.
//   - "price" de un pickup vive FUERA del bloque "pickup" (precio base de
//     la carta -> PickupData::price).
//   - La tienda admite forma corta ("pocion") y larga ({objectId,price});
//     el precio de carta se referencia a traves del catalogo.
//   - business{name,price,baseIncome}: precio>0 = cartel en venta.
//
// GL-free: lee JSON directamente, no toca ni Render ni GL. cwd = raiz del
// MotorGraphico (working directory del CTest, igual que el resto de
// tests de catalogo).

#include "Level/ObjectCatalog.h"

#include "TestHarness.h"

#include <cstddef>
#include <string>
#include <vector>

namespace {

void failsAmbMissatge(const std::string& missatge) {
    throw ::motor_test::AssertionFailure(missatge.c_str(), __FILE__, __LINE__);
}

using ObjectCatalogZona = ObjectCatalog;

ObjectCatalogZona carrega_ciutat() {
    ObjectCatalogZona cat;
    const auto r = cat.loadFromFile("assets/objects/ciudad_objetos.json");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == 121);
    return cat;
}

MOTOR_TEST_CASE(ciutat_objetos_carrega_els_121) {
    ObjectCatalogZona cat;
    const auto r = cat.loadFromFile("assets/objects/ciudad_objetos.json");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == 121);
    MOTOR_CHECK(cat.size() == 121);
}

MOTOR_TEST_CASE(categories_repartides_de_la_ciutat) {
    const auto cat = carrega_ciutat();
    MOTOR_CHECK(cat.ids(ObjectCategory::Prop).size() == 55);
    // con la daga (TIENDA-03) y los 23 de INSTRUMENTOS-01 (los instrumentos, los objetos de rol y las ganzúas)
    MOTOR_CHECK(cat.ids(ObjectCategory::Pickup).size() == 43);
    MOTOR_CHECK(cat.ids(ObjectCategory::Npc).size() == 23);
    MOTOR_CHECK(cat.ids(ObjectCategory::Enemy).size() == 0);
}

MOTOR_TEST_CASE(cap_entrada_amb_dades_per_defecte_silencioses) {
    const auto cat = carrega_ciutat();
    std::size_t sprites_sense_imatge = 0;
    std::size_t pickups_sense_dades = 0;
    std::size_t npcs_muts = 0;
    std::size_t npcs_sense_tenda = 0;
    std::size_t negocis_sense_preu = 0;
    std::size_t enemics_amb_vida_zero = 0;
    for (const std::string& id : cat.ids()) {
        const ObjectDefinition* o = cat.find(id);
        if (o == nullptr) continue;
        if (o->name.empty()) failsAmbMissatge("objeto sin nombre: " + id);
        if (o->spriteId < 0) ++sprites_sense_imatge;
        if (o->category == ObjectCategory::Pickup) {
            if (o->pickup.price <= 0 && o->pickup.effect == PickupEffect::None) {
                ++pickups_sense_dades;
            }
        } else if (o->category == ObjectCategory::Npc) {
            if (o->dialogue.lines.empty()) ++npcs_muts;
            if (o->shop.items.empty()) ++npcs_sense_tenda;
        } else if (o->category == ObjectCategory::Enemy) {
            if (o->combat.maxHealth <= 0) ++enemics_amb_vida_zero;
        }
        if (o->business.price > 0) {
            if (o->business.baseIncome <= 0) failsAmbMissatge("negocio sin ingreso base: " + id);
            if (o->business.businessName.empty()) failsAmbMissatge("negocio sin nombre: " + id);
        }
    }
    MOTOR_CHECK(sprites_sense_imatge == 0);
    MOTOR_CHECK(pickups_sense_dades == 0);
    MOTOR_CHECK(npcs_muts == 0);
    // Los PNJ de dialogo no necesitan una tienda; las tiendas reales
    // conservan su comprobacion de inventario y referencias debajo.
    MOTOR_CHECK(npcs_sense_tenda > 0);
    MOTOR_CHECK(enemics_amb_vida_zero == 0);
}

MOTOR_TEST_CASE(integridat_referencial_de_les_tendes) {
    const auto cat = carrega_ciutat();
    std::size_t totalBee = 0;
    for (const std::string& id : cat.ids()) {
        const ObjectDefinition* o = cat.find(id);
        if (o == nullptr || o->category != ObjectCategory::Npc) continue;
        for (const ShopItem& item : o->shop.items) {
            ++totalBee;
            if (!cat.has(item.objectId)) failsAmbMissatge("tenda apunta a objecte inexistent: " + item.objectId);
        }
    }
    MOTOR_CHECK(totalBee > 0);
}

MOTOR_TEST_CASE(cartes_concretes_del_mercat) {
    const auto cat = carrega_ciutat();

    const auto* pocio = cat.find("pocion");
    MOTOR_CHECK(pocio != nullptr);
    if (pocio) {
        MOTOR_CHECK(pocio->name == "Poción");  // TILDES-01: con su tilde
        MOTOR_CHECK(pocio->category == ObjectCategory::Pickup);
        MOTOR_CHECK(pocio->pickup.price == 30);
        MOTOR_CHECK(pocio->pickup.effect == PickupEffect::Heal);
        MOTOR_CHECK(pocio->pickup.power == 15);
    }

    const auto* eter = cat.find("eter");
    MOTOR_CHECK(eter != nullptr);
    if (eter) {
        MOTOR_CHECK(eter->pickup.price == 45);
        MOTOR_CHECK(eter->pickup.effect == PickupEffect::RestoreMana);
        MOTOR_CHECK(eter->pickup.power == 5);
    }

    const auto* pa = cat.find("pan");
    MOTOR_CHECK(pa != nullptr);
    if (pa) {
        MOTOR_CHECK(pa->pickup.price == 8);
        MOTOR_CHECK(pa->pickup.effect == PickupEffect::Heal);
        MOTOR_CHECK(pa->pickup.power == 5);
    }

    const auto* banc = cat.find("cartel_banco");
    MOTOR_CHECK(banc != nullptr);
    if (banc) {
        MOTOR_CHECK(banc->category == ObjectCategory::Prop);
        MOTOR_CHECK(banc->business.businessName == "Banco");
        MOTOR_CHECK(banc->business.price == 4200);
        MOTOR_CHECK(banc->business.baseIncome == 85);
    }

    const auto* tendero = cat.find("tendero_mercado");
    MOTOR_CHECK(tendero != nullptr);
    if (tendero) {
        MOTOR_CHECK(tendero->category == ObjectCategory::Npc);
        MOTOR_CHECK(tendero->dialogue.lines.size() == 2);
        MOTOR_CHECK(tendero->dialogue.lines[0] == "¡Bienvenido al mercado!");  // TILDES-01
        MOTOR_CHECK(tendero->shop.buybackPercent == 25);  // AV18-07: venta al 25 %
        MOTOR_CHECK(tendero->shop.items.size() == 3);
        if (tendero->shop.items.size() == 3) {
            MOTOR_CHECK(tendero->shop.items[0].objectId == "pocion");
            MOTOR_CHECK(tendero->shop.items[1].objectId == "eter");
            MOTOR_CHECK(tendero->shop.items[2].objectId == "pan");
        }
    }

    // TIENDA-03 — la daga que vende Aigren, junto a la espada corta.
    const auto* daga = cat.find("daga");
    MOTOR_CHECK(daga != nullptr);
    if (daga) {
        MOTOR_CHECK(daga->name == "Daga");
        MOTOR_CHECK(daga->category == ObjectCategory::Pickup);
        MOTOR_CHECK(daga->pickup.price == 60);
    }
}

MOTOR_TEST_CASE(merge_i_sobreescritura_ultim_archiu_guanya) {
    auto cat = carrega_ciutat();

    // Merge aditivo desde string: dos entradas nuevas sin tocar las 121.
    const auto r = cat.loadFromString(R"({ "objects": [
        {"id": "porta_del_nord", "name": "Porta del Nord", "category": "prop", "blocksMovement": true},
        {"id": "botiquin", "name": "Botiquin", "category": "pickup", "price": 12,
         "pickup": {"effect": "heal", "power": 20}}
    ] })");
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value() == 2);
    MOTOR_CHECK(cat.size() == 123);
    MOTOR_CHECK(cat.has("porta_del_nord"));
    MOTOR_CHECK(cat.find("botiquin")->pickup.price == 12);

    // Override: mismo id, distinto nombre -> el ultimo archivo gana.
    const auto r2 = cat.loadFromString(R"({ "objects": [
        {"id": "porta_del_nord", "name": "Porta renomenada", "category": "prop"}
    ] })");
    MOTOR_CHECK(r2.isOk());
    MOTOR_CHECK(r2.value() == 1);
    MOTOR_CHECK(cat.size() == 123);  // sin duplicado
    MOTOR_CHECK(cat.find("porta_del_nord")->name == "Porta renomenada");
}

MOTOR_TEST_CASE(errors_estructurals_de_la_carrega) {
    ObjectCatalogZona cat;

    const auto senseLlista = cat.loadFromString(R"({ "noObjects": [] })");
    MOTOR_CHECK(!senseLlista.isOk());

    const auto senseId = cat.loadFromString(R"({ "objects": [ {"name": "x", "category": "prop"} ] })");
    MOTOR_CHECK(!senseId.isOk());
    MOTOR_CHECK(senseId.errorMessage() != "");

    const auto categoriaDolenta = cat.loadFromString(
        R"({ "objects": [ {"id": "a", "name": "a", "category": "decor"} ] })");
    MOTOR_CHECK(!categoriaDolenta.isOk());
    MOTOR_CHECK(categoriaDolenta.errorMessage().find("categoria") != std::string::npos);

    const auto efectoDolent = cat.loadFromString(
        R"({ "objects": [ {"id": "b", "category": "pickup", "pickup": {"effect": "curar"}} ] })");
    MOTOR_CHECK(!efectoDolent.isOk());
    MOTOR_CHECK(efectoDolent.errorMessage().find("pickup") != std::string::npos);

    // Todo-o-nada: el catalogo valido anterior sigue intacto (carga en
    // un catalogo limpio que falla antes de tocar el mapa interno).
    MOTOR_CHECK(cat.size() == 0);
}

// ObjectCatalog::exportJson() esta DECLARADO en el header (y el comentario
// da por verificado un round-trip en demo_object_catalog) pero NO tiene
// definicion en ningun .cpp del repositorio (hallazgo 12/09, viva el C-22:
// "cap declaracio del motor es queda sense cos"). Como no existe, la red
// no puede fijar el round-trip; el hallazgo va registrado en el TABLERO
// (Pendiente) y en la sesion. No se toca la declaracion (regla 2).

}  // namespace

int main() { return motor_test::runAllTests(); }