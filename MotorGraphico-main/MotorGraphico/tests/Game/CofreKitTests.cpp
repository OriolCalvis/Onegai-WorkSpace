// UI-BG3-01 — El cofre de El Vacio (Game/CofreKit.h): se abre con la E al
// lado, deja el kit en la mochila, enciende sus flags y no vuelve; y el
// prologo nunca se queda esperando (sin cofre en el nivel, flags solas).
#include "TestHarness.h"

#include <string>
#include <vector>

#include "Game/CofreKit.h"
#include "RPG/Narrative/NarrativeState.h"
#include "World/World.h"

namespace {

using Game::CofreKit;

Engine::EntityHandle ponCofre(Engine::World& mundo, int x, int y) {
    ObjectSpawn s;
    s.objectId = CofreKit::kObjeto;
    s.position = GridCoord{x, y};
    const Engine::EntityHandle h = mundo.createObject(s);
    mundo.flushPendingChanges();
    return h;
}

RPG::Catalogs::EquipmentCatalog equipo() {
    RPG::Catalogs::EquipmentCatalog c;
    RPG::EquipmentDefinition sable;
    sable.id = "sable";
    sable.name = "Sable Ligero";
    sable.slot = "MainHand";
    c.insert(sable);
    RPG::EquipmentDefinition amuleto;
    amuleto.id = "amuleto";
    amuleto.name = "Amuleto de Foco";
    amuleto.slot = "Head";
    amuleto.statBonuses = {0, 0, 0, 1};
    c.insert(amuleto);
    return c;
}

MOTOR_TEST_CASE(cofre_al_alcance_de_la_e_y_no_en_diagonal) {
    Engine::World mundo;
    ponCofre(mundo, 7, 8);
    MOTOR_CHECK(CofreKit::alAlcance(mundo, GridCoord{7, 9}).valid());   // debajo
    MOTOR_CHECK(CofreKit::alAlcance(mundo, GridCoord{6, 8}).valid());   // al lado
    MOTOR_CHECK(!CofreKit::alAlcance(mundo, GridCoord{6, 9}).valid());  // diagonal: un paso mas
}

MOTOR_TEST_CASE(abrir_deja_el_kit_en_la_mochila_y_quita_el_cofre) {
    Engine::World mundo;
    const Engine::EntityHandle h = ponCofre(mundo, 7, 8);
    RPG::NarrativeState flags;
    std::vector<std::string> mochila = {"pocion"};
    CofreKit cofre;
    cofre.abre(mundo, h, {"sable", "amuleto"}, {"sable", "item_amuleto"}, equipo(), "Bardo", mochila, flags);
    mundo.flushPendingChanges();
    MOTOR_CHECK(mochila.size() == 3 && mochila[1] == "sable" && mochila[2] == "item_amuleto");
    MOTOR_CHECK(flags.hasFlag(CofreKit::kRecogido));
    MOTOR_CHECK(!flags.hasFlag(CofreKit::kPuesto));
    MOTOR_CHECK(!CofreKit::alAlcance(mundo, GridCoord{7, 9}).valid());
    const CofreKit::Papiro* p = cofre.papiro();
    MOTOR_CHECK(p != nullptr && p->piezas.size() == 2);
    if (p == nullptr || p->piezas.size() != 2) return;
    MOTOR_CHECK(p->piezas[0].ranura == "Arma" && p->piezas[0].bono.empty());
    MOTOR_CHECK(p->piezas[1].ranura == "Cabeza" && p->piezas[1].bono.empty());  // AV16-02: las prendas del kit no dicen que sumen
    MOTOR_CHECK(p->foco == 0);  // «Ponermelo»
    cofre.cierra();
    MOTOR_CHECK(cofre.papiro() == nullptr);
}

MOTOR_TEST_CASE(puesto_cuando_no_queda_pieza_del_kit_en_la_mochila) {
    Engine::World mundo;
    RPG::NarrativeState flags;
    flags.setFlag(CofreKit::kRecogido);
    const std::vector<std::string> kit = {"sable", "item_amuleto"};
    std::vector<std::string> mochila = {"item_amuleto", "pocion"};
    CofreKit cofre;
    cofre.revisa(mundo, mochila, kit, flags);
    MOTOR_CHECK(!flags.hasFlag(CofreKit::kPuesto));  // aun lleva el amuleto en la mochila
    mochila = {"pocion"};                            // puesto (o tirado: no se atasca)
    cofre.revisa(mundo, mochila, kit, flags);
    MOTOR_CHECK(flags.hasFlag(CofreKit::kPuesto));
}

MOTOR_TEST_CASE(sin_cofre_en_el_nivel_las_flags_se_dan_solas_y_abierto_no_vuelve) {
    Engine::World vacio;
    RPG::NarrativeState flags;
    CofreKit cofre;
    cofre.revisa(vacio, {}, {"sable"}, flags);
    MOTOR_CHECK(flags.hasFlag(CofreKit::kRecogido) && flags.hasFlag(CofreKit::kPuesto));
    // Al recargar El Vacio el nivel vuelve a poner el cofre: se quita.
    Engine::World recargado;
    ponCofre(recargado, 7, 8);
    cofre.revisa(recargado, {}, {"sable"}, flags);
    recargado.flushPendingChanges();
    MOTOR_CHECK(!CofreKit::alAlcance(recargado, GridCoord{7, 9}).valid());
}

// OBJETOS-01 — El cofre de El Vacio tiene su id: el «cofre» de los muebles de los
// interiores (la posada, la opera, la biblioteca) no es el del kit, ni se abre
// con la E ni desaparece al entrar ya con el kit recogido.
MOTOR_TEST_CASE(un_cofre_de_mueble_no_es_el_del_kit) {
    MOTOR_CHECK(std::string(CofreKit::kObjeto) != "cofre");
    Engine::World posada;
    ObjectSpawn mueble;
    mueble.objectId = "cofre";
    mueble.position = GridCoord{3, 3};
    const Engine::EntityHandle h = posada.createObject(mueble);
    posada.flushPendingChanges();
    RPG::NarrativeState flags;
    flags.setFlag(CofreKit::kRecogido);
    flags.setFlag(CofreKit::kPuesto);
    CofreKit cofre;
    cofre.revisa(posada, {}, {"sable"}, flags);
    posada.flushPendingChanges();
    MOTOR_CHECK(posada.contains(h));
    MOTOR_CHECK(!CofreKit::alAlcance(posada, GridCoord{3, 4}).valid());
}

}  // namespace
