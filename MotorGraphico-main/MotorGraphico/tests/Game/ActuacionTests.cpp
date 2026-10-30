// ACTUAR-01 — Actuar en la calle (include/Game/Actuacion.h).
//
// Oriol (08/10/2026): «podamos hacer una performance en la calle y depende de
// nuestra competencia musical y tiradas, atraer a la gente a nuestro
// espectaculo».
//
// Se prueba: lo que suma lo que se lleva en la mano, los cuatro grados con
// dados fijos (cuanta gente se para y cuanto oro echa), el descanso de cada
// sitio (los pasos), la sesion entera (interact con un sitio al lado, el
// dialogo, el oro y la foto de la tirada) y los sitios de Boundington (el
// catalogo y los niveles). GL-free; cwd = build (los assets estan copiados).

#include "RPG/Narrative/FranjasDelDia.h"
#include "TestHarness.h"

#include "Game/Actuacion.h"
#include "Game/GameSession.h"
#include "Level/LevelLoader.h"
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/DefinicionesObjetos.h"
#include "RPG/Narrative/NarrativeState.h"
#include "RPG/NarrativeEngine.h"
#include "RPG/RandomEngine.h"
#include "Render/ICombatant.h"

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

namespace {

// Dados a cara fija, en ciclo; roll_in_range da siempre el maximo.
class DadoFijo : public RPG::RandomEngine {
public:
    DadoFijo(std::initializer_list<int> caras) : m_caras(caras) {}
    int roll_d6() override { return m_caras[m_i++ % m_caras.size()]; }
    int roll_in_range(int /*a*/, int b) override { return b; }
    uint64_t seed() const override { return 7; }

private:
    std::vector<int> m_caras;
    std::size_t m_i = 0;
};

// Un heroe con ficha (para lo que lleva en la mano) y una CAR fija.
class Heroe : public ICombatant {
public:
    explicit Heroe(int car) : m_car(car) { m_ficha.displayName = "Ilse"; }
    void takeDamage(int) override {}
    void heal(int) override {}
    int health() const override { return 10; }
    int maxHealth() const override { return 10; }
    bool isAlive() const override { return true; }
    int stat(RPG::Stat s) const override { return s == RPG::Stat::CAR ? m_car : 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }
    RPG::CharacterSheet* character_sheet() override { return &m_ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return &m_ficha; }
    void lleva(const std::string& id) {
        m_ficha.equippedIds[static_cast<std::size_t>(RPG::EquipSlot::MainHand)] = id;
    }

private:
    int m_car;
    RPG::CharacterSheet m_ficha;
};

RPG::EquipmentDefinition pieza(const std::string& id, const std::string& nombre, std::vector<std::string> etiquetas,
                               RPG::ItemRarity rareza = RPG::ItemRarity::Common) {
    RPG::EquipmentDefinition e;
    e.id = id;
    e.name = nombre;
    e.slot = "MainHand";
    e.grantedTags = std::move(etiquetas);
    e.rarity = rareza;
    return e;
}

PerformanceData sitio(int rival, int afluencia, const char* lugar = "junto a la fuente") {
    PerformanceData p;
    p.rivalDice = rival;
    p.afluencia = afluencia;
    p.lugar = lugar;
    return p;
}

ObjectDefinition sitioDelCatalogo() {
    ObjectDefinition d;
    d.id = "actuacion_prueba";
    d.name = "Rincón del músico";
    d.category = ObjectCategory::Prop;
    d.interactable = true;
    d.spriteId = 16;
    d.actuacion = sitio(1, 8);
    return d;
}

}  // namespace

MOTOR_TEST_CASE(lo_que_suma_lo_que_llevas_en_la_mano) {
    const Game::ConQueActuas nada = Game::conQueActuas(nullptr);
    MOTOR_CHECK(nada.nombre.empty() && nada.dadosExtra == 0);
    const auto lira = pieza("lira", "Lira", {"Arma Ligera", "Instrumento"});
    MOTOR_CHECK(Game::conQueActuas(&lira).nombre == "tu lira");
    MOTOR_CHECK(Game::conQueActuas(&lira).dadosExtra == 1);
    const auto laud = pieza("laud", "Laúd del Juglar", {"Instrumento"}, RPG::ItemRarity::Rare);
    MOTOR_CHECK(Game::conQueActuas(&laud).nombre == "tu laúd del juglar");
    MOTOR_CHECK(Game::conQueActuas(&laud).dadosExtra == 2);
    const auto arpa = pieza("arpa", "Arpa de los Cielos", {"Instrumento"}, RPG::ItemRarity::Mythic);
    MOTOR_CHECK(Game::conQueActuas(&arpa).dadosExtra == 3);
    const auto bolas = pieza("bolas", "Bolas de malabares", {"Objeto de rol", "Malabares"});
    MOTOR_CHECK(Game::conQueActuas(&bolas).nombre == "tus bolas de malabares");
    MOTOR_CHECK(Game::conQueActuas(&bolas).dadosExtra == 1);
    // Una espada no sirve para actuar: a capela.
    const auto espada = pieza("espada", "Espada corta", {"Arma Ligera"});
    MOTOR_CHECK(Game::conQueActuas(&espada).nombre.empty() && Game::conQueActuas(&espada).dadosExtra == 0);
}

MOTOR_TEST_CASE(los_cuatro_grados_dicen_cuanta_gente_se_para_y_cuanto_echa) {
    const Game::ConQueActuas lira{"tu lira", 1};
    // Critico: 6 y 6 (dos 6, D0013) contra 2, 3 y 4. Se para todo el que
    // pasa (12) y unos pocos mas (12 / 4 = 3: el dado fijo da el maximo).
    {
        DadoFijo d({6, 6, 2, 3, 4});
        const Game::Actuacion a = Game::actua(sitio(3, 12), 1, lira, d);
        MOTOR_CHECK(a.dados == 2 && a.rivalDice == 3);
        MOTOR_CHECK(a.grado == RPG::Degree::CRITICAL);
        MOTOR_CHECK(a.publico == 15 && a.oro == 45);
        MOTOR_CHECK(a.lineas.size() == 3);
        MOTOR_CHECK(a.lineas[0] == "Junto a la fuente, con tu lira, empiezas tu número.");
        MOTOR_CHECK(a.lineas[2] == "Ganas 45 de oro.");
    }
    // Exito: 6 y 5 (1,5) contra 6 y 2 (1). La mitad (6) y uno mas: 2 de oro cada uno.
    {
        DadoFijo d({6, 5, 6, 2});
        const Game::Actuacion a = Game::actua(sitio(2, 12), 2, {}, d);
        MOTOR_CHECK(a.grado == RPG::Degree::SUCCESS);
        MOTOR_CHECK(a.publico == 7 && a.oro == 14);
        MOTOR_CHECK(a.lineas[0] == "Junto a la fuente, sin nada en las manos, cantas y bailas lo que te sabes.");
    }
    // Parcial (empate): un cuarto (3), 1 de oro cada uno.
    {
        DadoFijo d({6, 2, 6, 3});
        const Game::Actuacion a = Game::actua(sitio(2, 12), 2, {}, d);
        MOTOR_CHECK(a.grado == RPG::Degree::PARTIAL);
        MOTOR_CHECK(a.publico == 3 && a.oro == 3);
    }
    // Pierdes: nadie se para y no cae nada.
    {
        DadoFijo d({2, 3, 6, 2});
        const Game::Actuacion a = Game::actua(sitio(2, 12), 2, {}, d);
        MOTOR_CHECK(a.grado == RPG::Degree::BOTCH && !a.tirada.botch);
        MOTOR_CHECK(a.publico == 0 && a.oro == 0);
        MOTOR_CHECK(a.lineas[1] == "La gente pasa de largo: hoy nadie tiene tiempo para ti.");
        MOTOR_CHECK(a.lineas[2] == "No cae ni una moneda.");
    }
    // Pifia de verdad (ningun 5 ni 6 y dos 1): ademas, se rien.
    {
        DadoFijo d({1, 1, 6, 2});
        const Game::Actuacion a = Game::actua(sitio(2, 12), 2, {}, d);
        MOTOR_CHECK(a.grado == RPG::Degree::BOTCH && a.tirada.botch);
        MOTOR_CHECK(a.oro == 0);
        MOTOR_CHECK(a.lineas[1].find("alguien se ríe") != std::string::npos);
    }
    // Uno solo: en singular.
    {
        DadoFijo d({6, 2, 6, 3});
        const Game::Actuacion a = Game::actua(sitio(2, 2), 2, {}, d);
        MOTOR_CHECK(a.publico == 1 && a.oro == 1);
        MOTOR_CHECK(a.lineas[1] == "Se para 1 persona un momento.");
    }
}

MOTOR_TEST_CASE(andar_sin_haber_actuado_no_apunta_nada) {
    RPG::NarrativeState estado;
    // Quien no ha actuado nunca no lleva reloj: andar no apunta nada.
    Game::cuentaUnPaso(estado);
    MOTOR_CHECK(estado.variables().empty());
    MOTOR_CHECK(Game::pasosQueFaltan(estado, "actuacion_prueba") == 0);
}

// PO-49 — Con dias en marcha, se actua una vez por sitio y DIA (los pasos ya no cuentan).
MOTOR_TEST_CASE(con_dias_se_actua_una_vez_por_sitio_y_dia) {
    RPG::NarrativeState estado;
    estado.setVariable(RPG::franjas::kVariableDia, 1);
    MOTOR_CHECK(!Game::yaActuasteHoy(estado, "plaza"));
    estado.setVariable("actuacion.dia.plaza", 1);
    MOTOR_CHECK(Game::yaActuasteHoy(estado, "plaza"));
    MOTOR_CHECK(!Game::yaActuasteHoy(estado, "otro_sitio"));
    estado.setVariable(RPG::franjas::kVariableDia, 2);  // amanece: otra vez
    MOTOR_CHECK(!Game::yaActuasteHoy(estado, "plaza"));
    MOTOR_CHECK(Game::pasosQueFaltan(estado, "plaza") == 0);
}

MOTOR_TEST_CASE(actuar_en_la_sesion_da_dialogo_oro_y_tirada_y_luego_descansa) {
    ObjectCatalog catalogo;
    catalogo.add(sitioDelCatalogo());
    SkillCatalog cartas;
    SkillSet habilidades{10};
    std::vector<std::string> mochila;
    Heroe heroe(2);
    LevelDefinition nivel;
    nivel.name = "Plaza";
    nivel.playerStart = GridCoord{0, 0};
    ObjectSpawn spawn;
    spawn.objectId = "actuacion_prueba";
    spawn.position = GridCoord{1, 0};
    spawn.patrolMin = spawn.patrolMax = spawn.position;
    nivel.objects.push_back(spawn);

    RPG::Catalogs::EquipmentCatalog equipo;
    equipo.insert(pieza("lira", "Lira", {"Arma Ligera", "Instrumento"}));
    InventoryRules reglas;
    reglas.equipment = &equipo;
    heroe.lleva("lira");

    RPG::NarrativeEngine motor;
    RPG::NarrativeState estado;
    DadoFijo d({6});  // todo 6: critico (3 dados contra 1)
    GameSession sesion(nullptr, &catalogo, &cartas, nivel, &heroe, &habilidades, &mochila);
    sesion.setInventoryRules(reglas);
    sesion.setNarrative(&motor, &estado);
    sesion.setRandomEngine(d);

    MOTOR_CHECK(sesion.interact());
    MOTOR_CHECK(sesion.mode() == GameMode::Dialogue);
    MOTOR_CHECK(sesion.dialogueSpeaker().empty());
    MOTOR_CHECK(sesion.dialogueLines().size() == 3);
    MOTOR_CHECK(sesion.dialogueLines()[0] == "Junto a la fuente, con tu lira, empiezas tu número.");
    // 8 que pasan y 2 mas; 3 de oro cada uno.
    MOTOR_CHECK(sesion.gold() == 30);
    // La foto de la tirada, para la hoja del dialogo.
    MOTOR_CHECK(estado.ultimaTirada().numero == 1);
    MOTOR_CHECK(estado.ultimaTirada().que == "Interpretación");
    MOTOR_CHECK(estado.ultimaTirada().quien == "Ilse");
    MOTOR_CHECK(estado.ultimaTirada().tirada.rolls.size() == 3);
    MOTOR_CHECK(Game::pasosQueFaltan(estado, "actuacion_prueba") == Game::kPasosDeDescanso);
    sesion.closeInteraction();

    // Otra vez en el acto: ya te han visto (ni tirada ni oro).
    MOTOR_CHECK(sesion.interact());
    MOTOR_CHECK(sesion.dialogueLines().size() == 1);
    MOTOR_CHECK(sesion.gold() == 30);
    MOTOR_CHECK(estado.ultimaTirada().numero == 1);
    sesion.closeInteraction();

    // Ciento veinte pasos (abajo y arriba) y se puede volver a actuar.
    for (int i = 0; i < Game::kPasosDeDescanso; ++i) MOTOR_CHECK(sesion.tryMovePlayer(0, i % 2 == 0 ? 1 : -1));
    MOTOR_CHECK(sesion.playerPosition().x == 0 && sesion.playerPosition().y == 0);
    MOTOR_CHECK(Game::pasosQueFaltan(estado, "actuacion_prueba") == 0);
    MOTOR_CHECK(sesion.interact());
    MOTOR_CHECK(sesion.gold() == 60);
    MOTOR_CHECK(estado.ultimaTirada().numero == 2);
}

MOTOR_TEST_CASE(los_sitios_de_boundington_estan_en_su_catalogo_y_en_sus_niveles) {
    ObjectCatalog catalogo;
    const auto r = catalogo.loadFromFile("assets/objects/actuaciones.json");
    MOTOR_CHECK(r.isOk());
    if (!r.isOk()) return;
    std::vector<std::string> sitios;
    for (const std::string& id : catalogo.ids()) {
        const ObjectDefinition* d = catalogo.find(id);
        MOTOR_CHECK(d->category == ObjectCategory::Prop && d->interactable);
        MOTOR_CHECK(d->actuacion.rivalDice > 0 && d->actuacion.afluencia > 0 && !d->actuacion.lugar.empty());
        sitios.push_back(id);
    }
    MOTOR_CHECK(sitios.size() >= 4);
    // Cada uno, colocado una vez en su nivel.
    std::vector<std::string> colocados;
    for (const char* ruta : {"assets/levels/ciudad_centro.json", "assets/levels/interior_taberna.json"}) {
        const auto nivel = LevelLoader::loadFromFile(ruta);
        MOTOR_CHECK(nivel.isOk());
        if (!nivel.isOk()) continue;
        for (const ObjectSpawn& o : nivel.value().objects)
            if (catalogo.has(o.objectId)) colocados.push_back(o.objectId);
    }
    std::sort(sitios.begin(), sitios.end());
    std::sort(colocados.begin(), colocados.end());
    MOTOR_CHECK(colocados == sitios);
}

int main() { return motor_test::runAllTests(); }
