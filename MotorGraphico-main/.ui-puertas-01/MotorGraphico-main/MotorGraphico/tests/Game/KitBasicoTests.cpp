// UI-BG3-01 — El kit del cofre de El Vacio (Game/Personajes/KitBasico.h).
//
// Lo que Oriol pidio (05/10): un cofre con el equipo basico de la clase para
// empezar con puntos en las caracteristicas que la clase necesita, y que los
// 63 kits salgan de una regla con parametros. Y despues: «la primera
// equipacion la balanceamos a 2 dados, lo mas alto, 1 lo mas bajo;
// principal y secundaria +2». Aqui se recorre el catalogo entero: cada clase
// recibe un kit que puede ponerse SIN requisitos y que, puesto, suma
// exactamente +2 al principal, +2 al secundario y +1 a los otros dos.
//
// Tambien la regla de los topes por tier (RPG/TopesDados.h): el mejor equipo
// de Tier 1 da 4, 4, 2 y 2, nunca tres atributos arriba, y la ficha recorta
// lo que se pase.
#include "TestHarness.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <stdexcept>
#include <string>

#include "Game/Personajes/KitBasico.h"
#include "Game/Personajes/ManifiestoPersonajes.h"
#include "RPG/CharacterCreation.h"
#include "RPG/InventoryEngine.h"
#include "RPG/TopesDados.h"

namespace {

namespace Kit = Game::KitBasico;

const Game::ManifiestoPersonajes& manifiesto() {
    static const Game::ManifiestoPersonajes m = [] {
        auto r = Game::ManifiestoPersonajes::cargar("assets/textures");
        if (!r.isOk()) throw std::runtime_error(r.errorMessage());
        return r.value();
    }();
    return m;
}

const RPG::CharacterCreation& reglas() {
    static const RPG::CharacterCreation c = [] {
        auto r = RPG::CharacterCreation::load("assets/catalogs");
        if (!r.isOk()) throw std::runtime_error(r.errorMessage());
        return std::move(r.value());
    }();
    return c;
}

const RPG::Catalogs::EquipmentCatalog& equipo() {
    static const RPG::Catalogs::EquipmentCatalog cat = [] {
        RPG::Catalogs::EquipmentCatalog c;
        auto r = c.loadFromFile("assets/catalogs/equipment.json");
        if (!r.isOk()) throw std::runtime_error(r.errorMessage());
        return c;
    }();
    return cat;
}

const RPG::TierRules& tiers() {
    static const RPG::TierRules t = [] {
        RPG::TierRules r;
        (void)r.loadFromFile("assets/rules/tier_rules.json");
        return r;
    }();
    return t;
}

const Kit::Reglas& reglasKit() {
    static const Kit::Reglas r = [] {
        auto c = Kit::cargarReglas("assets/rules/kit_basico.json");
        if (!c.isOk()) throw std::runtime_error(c.errorMessage());
        return c.value();
    }();
    return r;
}

int indice(const std::string& stat) {
    const char* const n[4] = {"CON", "DES", "INT", "CAR"};
    for (int i = 0; i < 4; ++i)
        if (stat == n[i]) return i;
    return -1;
}

MOTOR_TEST_CASE(las_reglas_del_json_son_las_de_oriol) {
    const Kit::Reglas& r = reglasKit();
    MOTOR_CHECK(r.tierMaximo == 1);
    MOTOR_CHECK(r.sinRequisitos);
    MOTOR_CHECK(r.bonoPrincipal == 2 && r.bonoSecundario == 2 && r.bonoResto == 1);
    MOTOR_CHECK(r.exacto);
    MOTOR_CHECK(r.piezasMaximas == 5);
}

MOTOR_TEST_CASE(reglas_mal_escritas_se_explican) {
    MOTOR_CHECK(!Kit::leerReglas("[1]").isOk());
    MOTOR_CHECK(!Kit::leerReglas(R"({"rarezaMaxima": "brillante"})").isOk());
    MOTOR_CHECK(!Kit::leerReglas(R"({"atributos": ["tercero"]})").isOk());
    MOTOR_CHECK(!Kit::leerReglas(R"({"piezasMaximas": 0})").isOk());
    MOTOR_CHECK(!Kit::leerReglas(R"({"bonos": 2})").isOk());
    MOTOR_CHECK(!Kit::leerReglas(R"({"bonos": {"resto": -1}})").isOk());
    // El formato de antes sigue leyendose: «como minimo 1» en la lista.
    const auto solo = Kit::leerReglas(R"({"atributos": ["principal"], "bonoMinimo": 1})");
    MOTOR_CHECK(solo.isOk());
    if (!solo.isOk()) return;
    MOTOR_CHECK(solo.value().bonoPrincipal == 1 && solo.value().bonoSecundario == 0);
    MOTOR_CHECK(solo.value().bonoResto == 0 && !solo.value().exacto);
}

// La prueba que pidio el encargo: las 63 clases, una a una (y las de los
// especialistas de SP-02, que se suman al catalogo).
MOTOR_TEST_CASE(todas_las_clases_reciben_dos_dos_uno_uno_exactos) {
    MOTOR_CHECK(reglas().classes().size() >= 63);
    int completas = 0;
    reglas().classes().forEach([&](const RPG::ClassDefinition& clase) {
        const Kit::Kit kit = Kit::deClase(clase, equipo(), tiers(), reglasKit(), &manifiesto());
        if (!kit.completo()) {
            std::printf("  %s: falta %s, sobra %s\n", clase.id.c_str(),
                        kit.faltan.empty() ? "-" : kit.faltan.front().c_str(),
                        kit.sobran.empty() ? "-" : kit.sobran.front().c_str());
            return;
        }
        MOTOR_CHECK(!kit.piezas.empty());
        MOTOR_CHECK(static_cast<int>(kit.piezas.size()) <= reglasKit().piezasMaximas);
        const int p = indice(clase.primaryStat), s = indice(clase.secondaryStat);
        for (int i = 0; i < 4; ++i) {
            const int quiere = i == p ? 2 : i == s ? 2 : 1;
            MOTOR_CHECK(kit.bonos[static_cast<std::size_t>(i)] == quiere);
            MOTOR_CHECK(kit.objetivo[static_cast<std::size_t>(i)] == quiere);
        }
        // Es el reparto del tope 2, y cabe de sobra en el de Tier 1.
        MOTOR_CHECK(RPG::TopesDados::respeta(kit.bonos, 2));
        MOTOR_CHECK(RPG::TopesDados::respeta(kit.bonos, tiers().at(1).topeDadosEquipo));
        // Puesto de verdad en una ficha recien creada, pieza a pieza.
        RPG::CharacterSheet ficha;
        ficha.tier = 1;
        std::set<std::string> ranuras;
        for (const std::string& id : kit.piezas) {
            const RPG::EquipmentDefinition* e = equipo().find(id);
            MOTOR_CHECK(e != nullptr);
            if (e == nullptr) return;
            for (int i = 0; i < 4; ++i) MOTOR_CHECK(e->requiredStats[i] == 0);
            MOTOR_CHECK(e->tier <= 1);
            const std::string ranura = Kit::esArma(*e) ? "arma" : e->slot;
            MOTOR_CHECK(ranuras.insert(ranura).second);  // nunca dos en la misma ranura
            ficha.inventoryEquipment.push_back({id, 1});
            const auto puesto = RPG::InventoryEngine::equip(ficha, *e, clase, tiers(), equipo());
            if (!puesto.isOk()) std::printf("  %s: %s\n", clase.id.c_str(), puesto.errorMessage().c_str());
            MOTOR_CHECK(puesto.isOk());
        }
        // Base 0 y sin raza: los dados son los del kit.
        MOTOR_CHECK(ficha.stat(static_cast<RPG::Stat>(p)) == 2);
        MOTOR_CHECK(ficha.stat(static_cast<RPG::Stat>(s)) == 2);
        ++completas;
    });
    MOTOR_CHECK(completas == static_cast<int>(reglas().classes().size()));
}

// El bardo de las maquetas (C04b): su sable y su armadura de cuero siguen en
// el kit, y ahora con lo que hace falta para el +2, +2, +1, +1.
MOTOR_TEST_CASE(el_bardo_de_las_cuatro_estaciones_lleva_su_sable_y_su_armadura) {
    const RPG::ClassDefinition* bardo = reglas().classes().find("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(bardo != nullptr);
    if (bardo == nullptr) return;
    const Kit::Kit kit = Kit::deClase(*bardo, equipo(), tiers(), reglasKit(), &manifiesto());
    MOTOR_CHECK(kit.completo());
    MOTOR_CHECK(!kit.piezas.empty() && kit.piezas[0] == "sable_ligero_de_viaje");
    MOTOR_CHECK(std::find(kit.piezas.begin(), kit.piezas.end(), "armadura_de_cuero") != kit.piezas.end());
    // CAR es su principal y DES su secundario: CON 1, DES 2, INT 1, CAR 2.
    MOTOR_CHECK(kit.bonos[0] == 1 && kit.bonos[1] == 2 && kit.bonos[2] == 1 && kit.bonos[3] == 2);
}

// --- Los topes de dados por tier (RPG/TopesDados.h) ---------------------------

MOTOR_TEST_CASE(el_mejor_reparto_de_cada_tope) {
    using RPG::TopesDados::maximo;
    // Tope 2: el kit del cofre. Tope 4: el mejor equipo de Tier 1.
    MOTOR_CHECK(maximo(2).primero == 2 && maximo(2).segundo == 2 && maximo(2).resto == 1);
    MOTOR_CHECK(maximo(4).primero == 4 && maximo(4).segundo == 4 && maximo(4).resto == 2);
    MOTOR_CHECK(maximo(6).primero == 6 && maximo(6).segundo == 5 && maximo(6).resto == 3);
    MOTOR_CHECK(maximo(8).primero == 8 && maximo(8).segundo == 7 && maximo(8).resto == 4);
    MOTOR_CHECK(maximo(0).primero == 0);
}

MOTOR_TEST_CASE(nunca_tres_atributos_arriba) {
    using RPG::TopesDados::aplica;
    using RPG::TopesDados::respeta;
    // Tier 1 (tope 4): 4, 4, 2, 2 cabe; un tercero a 3 no.
    MOTOR_CHECK(respeta({4, 4, 2, 2}, 4));
    MOTOR_CHECK(!respeta({4, 4, 3, 2}, 4));
    MOTOR_CHECK(!respeta({5, 0, 0, 0}, 4));
    // Se recorta por puesto: CON 5 -> 4; INT y CAR empatan a 3 y el empate lo
    // gana el orden CON, DES, INT, CAR: INT es segundo (3) y CAR tercero (2).
    const std::array<int, 4> r = aplica({5, 1, 3, 3}, 4);
    MOTOR_CHECK(r[0] == 4 && r[1] == 1 && r[2] == 3 && r[3] == 2);
    // Los penalizadores no se tocan, y sin tope no se recorta nada.
    MOTOR_CHECK(aplica({-1, 6, 0, 0}, 4)[0] == -1 && aplica({-1, 6, 0, 0}, 4)[1] == 4);
    MOTOR_CHECK(aplica({9, 9, 9, 9}, 0)[3] == 9);
}

MOTOR_TEST_CASE(los_topes_del_json_son_los_de_la_regla) {
    MOTOR_CHECK(tiers().at(0).topeDadosEquipo == 2);
    MOTOR_CHECK(tiers().at(1).topeDadosEquipo == 4);
    MOTOR_CHECK(tiers().at(2).topeDadosEquipo == 6);
    MOTOR_CHECK(tiers().at(3).topeDadosEquipo == 8);
    MOTOR_CHECK(tiers().at(5).topeDadosEquipo == 8);
    // Las reglas por defecto (sin JSON) dicen lo mismo.
    const RPG::TierRules porDefecto;
    for (int t = 0; t <= 5; ++t) MOTOR_CHECK(porDefecto.at(t).topeDadosEquipo == tiers().at(t).topeDadosEquipo);
}

// La ficha cuenta el equipo con el tope de su tier; la raza suma encima.
MOTOR_TEST_CASE(la_ficha_recorta_el_equipo_y_la_raza_suma_encima) {
    RPG::CharacterSheet ficha;
    ficha.tier = 1;
    ficha.equipBonuses = {5, 0, 3, 3};
    ficha.racialBonuses = {1, 0, 0, 0};
    // Sin recompute_derived no hay tope (como antes).
    MOTOR_CHECK(ficha.stat(RPG::Stat::CON) == 6);
    ficha.recompute_derived(tiers(), nullptr);
    MOTOR_CHECK(ficha.stat(RPG::Stat::CON) == 5);  // 4 del equipo + 1 de la raza
    MOTOR_CHECK(ficha.stat(RPG::Stat::INT) == 3);  // segundo: hasta 4
    MOTOR_CHECK(ficha.stat(RPG::Stat::CAR) == 2);  // tercero: hasta 2
    // Al subir a Tier 2 (tope 6) el mismo equipo cuenta entero.
    MOTOR_CHECK(ficha.tierUp(tiers(), 2));
    MOTOR_CHECK(ficha.tier == 2);
    MOTOR_CHECK(ficha.stat(RPG::Stat::CON) == 6);
    MOTOR_CHECK(ficha.stat(RPG::Stat::CAR) == 3);
}

// La cota de malla de la Furia pide CON 1 y quita DES: no entra en el kit.
MOTOR_TEST_CASE(la_furia_no_recibe_la_cota_con_requisito) {
    const RPG::ClassDefinition* furia = reglas().classes().find("furia_de_los_creadores");
    MOTOR_CHECK(furia != nullptr);
    if (furia == nullptr) return;
    const Kit::Kit kit = Kit::deClase(*furia, equipo(), tiers(), reglasKit(), &manifiesto());
    MOTOR_CHECK(kit.completo());
    for (const std::string& id : kit.piezas) MOTOR_CHECK(id != "cota_de_malla");
}

// Si la clase no trae arma (el monje lucha sin ella), el kit tampoco.
MOTOR_TEST_CASE(sin_arma_en_la_clase_no_hay_arma_en_el_kit) {
    const RPG::ClassDefinition* monje = reglas().classes().find("monje_del_despertar");
    MOTOR_CHECK(monje != nullptr);
    if (monje == nullptr) return;
    const Kit::Kit kit = Kit::deClase(*monje, equipo(), tiers(), reglasKit(), &manifiesto());
    MOTOR_CHECK(kit.completo());
    for (const std::string& id : kit.piezas) {
        const RPG::EquipmentDefinition* e = equipo().find(id);
        MOTOR_CHECK(e != nullptr && !Kit::esArma(*e));
    }
}

// Con una sola pieza permitida no se puede cubrir todo: el kit lo dice.
MOTOR_TEST_CASE(si_no_cabe_el_kit_dice_lo_que_falta) {
    Kit::Reglas una = reglasKit();
    una.piezasMaximas = 1;
    const RPG::ClassDefinition* furia = reglas().classes().find("furia_de_los_creadores");
    MOTOR_CHECK(furia != nullptr);
    if (furia == nullptr) return;
    const Kit::Kit kit = Kit::deClase(*furia, equipo(), tiers(), una, &manifiesto());
    MOTOR_CHECK(kit.piezas.size() == 1);
    MOTOR_CHECK(!kit.completo());
}

}  // namespace

int main() {
    // El listado entero, para revisarlo a ojo (documentacion del paquete).
    if (std::getenv("KIT_LISTADO") != nullptr) {
        std::vector<std::string> ids;
        reglas().classes().forEach([&](const RPG::ClassDefinition& c) { ids.push_back(c.id); });
        std::sort(ids.begin(), ids.end());
        for (const std::string& id : ids) {
            const RPG::ClassDefinition& c = *reglas().classes().find(id);
            const Kit::Kit kit = Kit::deClase(c, equipo(), tiers(), reglasKit(), &manifiesto());
            std::printf("%s\t%s/%s\t%d %d %d %d", id.c_str(), c.primaryStat.c_str(), c.secondaryStat.c_str(),
                        kit.bonos[0], kit.bonos[1], kit.bonos[2], kit.bonos[3]);
            for (const std::string& p : kit.piezas) std::printf("\t%s", equipo().find(p)->name.c_str());
            std::printf("\n");
        }
    }
    return motor_test::runAllTests();
}
