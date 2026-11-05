// >>> PAQUETES DEL PLAN - bloque generado, no editar a mano >>>
// Este fichero lo tocan 2 paquetes del corte B:
//   C-24  (fase C, 5 j) - Xarxa de seguretat al camí que es juga
//   D-08  (fase D, 3 j) - Els enemics del joc són els del catàleg
//
// Si tocas este fichero estas trabajando dentro de esos paquetes: su
// criterio de cierre y su prueba mandan sobre lo que es 'estar hecho'.
// Criterios y pruebas: planificacion/PLA_DEFINITIU.md
// Mapa completo fichero <-> paquete: planificacion/MAPA_DEL_CODIGO.md
// Regenerar este bloque: python3 tools/marcar_paquetes.py --marcar
// <<< fin del bloque generado <<<

// C-03 — El estado del jugador sobrevive al cambio de nivel.
//
// El bug: Application::loadLevel destruye la GameSession y construye
// otra, y de la vieja rescataba SOLO el oro. Entrar a una posada te
// dejaba la reputacion a 0, sin los negocios comprados y con el contador
// de rentas a cero -- es decir, con la mitad de la partida borrada por
// cruzar una puerta.
//
// Lo que se prueba aqui es la funcion de traspaso (captureCarryOver /
// applyCarryOver), que es exactamente lo que loadLevel llama a los dos
// lados de la frontera. Unit y sin GL: la sesion se construye sin mapa
// (TileMap nullptr), como en SaveGameTests.

#include "RPG/Narrative/FranjasDelDia.h"
#include "TestHarness.h"

#include "Game/Acompanante.h"  // ACOMPANANTES-01
#include "Game/CartasEnCombate.h"
#include "Game/CatalogoCartas.h"
#include "Game/GameSession.h"
#include "Runtime/RuntimeTileMap.h"
#include "Game/SaveGame.h"  // H2-04
#include "Level/ObjectCatalog.h"
#include "RPG/CharacterSheet.h"
#include "Render/ICombatant.h"
#include "RPG/NarrativeEngine.h"
#include "RPG/RandomEngine.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <set>
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

LevelDefinition makeLevel(const char* name) {
    LevelDefinition level;
    level.name = name;
    level.playerStart = GridCoord{0, 0};
    return level;
}

PlayerCarryOver makeState() {
    PlayerCarryOver state;
    state.gold = 90;
    state.moralityValue = -30;
    state.stepsSinceRent = 17;
    state.businesses.push_back(OwnedBusiness{"posada", "Posada del Cuervo", 12, 140});
    state.businesses.push_back(OwnedBusiness{"herreria", "Herreria de Bran", 20, 80});
    return state;
}

// Una sesion "de nivel 1" ya jugada: 90 de oro, moral -30 y dos negocios.
struct Fixture {
    ObjectCatalog catalog;
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
};

// Dado a cara fija para las tiradas Nd6 (misma idea que ScriptedRng en
// examples/, pero local: test_game_session no incluye ese directorio).
// Secuencia ciclica; cara 1 => el rival pifia con 2+ dados.
class RngFixo : public RPG::RandomEngine {
public:
    explicit RngFixo(std::initializer_list<int> seq)
        : m_seq(seq.begin(), seq.end()), m_i(0) {}

    int roll_d6() override {
        const int cara = m_seq[m_i % m_seq.size()];
        ++m_i;
        return cara;
    }
    int roll_in_range(int /*a*/, int b) override { return b; }
    uint64_t seed() const override { return 99; }

private:
    std::vector<int> m_seq;
    std::size_t m_i;
};

// Un combatiente controlable: el stat relevante devuelve un valor fijo
// (para poder tirar 1 dado Nd6 en los skillCheck y cubrir los cuatro
// grados) y el resto, 0.
class StatCombatant : public ICombatant {
public:
    StatCombatant(int maxHealth, RPG::Stat clave, int valor)
        : m_health(maxHealth), m_maxHealth(maxHealth), m_clave(clave), m_valor(valor) {}

    void takeDamage(int amount) override { m_health = std::max(0, m_health - amount); }
    void heal(int amount) override { m_health = std::min(m_maxHealth, m_health + amount); }
    int health() const override { return m_health; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_health > 0; }

    int stat(RPG::Stat s) const override { return s == m_clave ? m_valor : 0; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "stat"; }

private:
    int m_health;
    int m_maxHealth;
    RPG::Stat m_clave;
    int m_valor;
};

// Un beat de charla que lo aplica "todo" (oro, log, dos skillChecks y un
// combate con enemigo roto), un beat de charla que solo resta oro, y un
// beat de entrada. Sin assets: se carga de memoria.
static RPG::AdventureScript makeAdventureProva() {
    const auto r = RPG::AdventureScript::loadFromString(R"({
        "id": "adv_prova_c24",
        "name": "Prova C-24",
        "description": "Guio minim per arribar als camins narratius de GameSession.",
        "beats": [
            {"id": "beat_narrativo",
             "trigger": {"type": "talk", "target": "tendero_narrativo"},
             "speaker": "Voz",
             "lines": ["hola", "segunda"],
             "effects": [
                {"type": "grantGold", "value": 7},
                {"type": "log", "arg": "una nota"},
                {"type": "skillCheck", "skillCheck": {
                    "skillId": "percepcion", "rivalDice": 3,
                    "flagBotch": "tirada_nada", "flagPartial": "tirada_parcial",
                    "flagSuccess": "tirada_exito", "flagCritical": "tirada_critico"}},
                {"type": "skillCheck", "skillCheck": {
                    "skillId": "percepcion", "cd": 1.0,
                    "flagBotch": "cerradura_nada", "flagPartial": "cerradura_parcial",
                    "flagSuccess": "cerradura_exito", "flagCritical": "cerradura_critico"}},
                {"type": "startBattle", "startBattle": {
                    "monsterId": "fantasma_inexistente",
                    "flagVictory": "bnd_victoria", "flagDefeat": "bnd_derrota"}}
             ]},
            {"id": "beat_pobre",
             "trigger": {"type": "talk", "target": "tendero_pobre"},
             "speaker": "Voz", "lines": ["Vete."],
             "effects": [{"type": "grantGold", "value": -50}]},
            {"id": "beat_entrada",
             "trigger": {"type": "enter", "target": "assets/niveles/prova_c24.json"},
             "requires": {"notFlags": ["bnd_llegada"]},
             "speaker": "Voz", "lines": ["Bienvenido."],
             "effects": [{"type": "setFlag", "arg": "bnd_llegada"},
                         {"type": "grantGold", "value": 3}]},
            {"id": "beat_grados",
             "trigger": {"type": "talk", "target": "tendero_grados"},
             "speaker": "", "lines": ["Cuatro tiradas con el rival vacio."],
             "effects": [
                {"type": "skillCheck", "skillCheck": {
                    "skillId": "percepcion", "cd": 0.0,
                    "flagBotch": "grado_nada", "flagPartial": "grado_parcial",
                    "flagSuccess": "grado_exito", "flagCritical": "grado_critico"}},
                {"type": "skillCheck", "skillCheck": {
                    "skillId": "percepcion", "cd": 0.0,
                    "flagBotch": "grado2_nada", "flagPartial": "grado2_parcial",
                    "flagSuccess": "grado2_exito", "flagCritical": "grado2_critico"}},
                {"type": "skillCheck", "skillCheck": {
                    "skillId": "percepcion", "cd": 0.0,
                    "flagBotch": "grado3_nada", "flagPartial": "grado3_parcial",
                    "flagSuccess": "grado3_exito", "flagCritical": "grado3_critico"}},
                {"type": "skillCheck", "skillCheck": {
                    "skillId": "percepcion", "cd": 0.0,
                    "flagBotch": "grado4_nada", "flagPartial": "grado4_parcial",
                    "flagSuccess": "grado4_exito", "flagCritical": "grado4_critico"}}
             ]}
        ]
    })");
    MOTOR_CHECK(r.isOk());
    return r.value();
}

}  // namespace

// El caso del pla: reconstruir la sesion sobre OTRO nivel y comprobar los
// cuatro valores. Antes de C-03 solo sobrevivia el oro.
MOTOR_TEST_CASE(el_estado_del_jugador_cruza_el_cambio_de_nivel) {
    Fixture f;
    GameSession primera(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    primera.applyCarryOver(makeState());

    // La sesion saliente lee de verdad lo que se le puso.
    MOTOR_CHECK(primera.gold() == 90);
    MOTOR_CHECK(primera.morality().value() == -30);
    MOTOR_CHECK(primera.businesses().size() == 2);

    // Lo que hace loadLevel: capturar de la vieja, construir la nueva
    // sobre otro nivel y aplicarselo.
    const PlayerCarryOver carried = primera.captureCarryOver();

    GameSession segunda(nullptr, &f.catalog, &f.skills, makeLevel("Posada"), &f.body,
                        &f.playerSkills, &f.inventory);
    segunda.applyCarryOver(carried);

    MOTOR_CHECK(segunda.gold() == 90);
    MOTOR_CHECK(segunda.morality().value() == -30);
    MOTOR_CHECK(segunda.captureCarryOver().stepsSinceRent == 17);
    MOTOR_CHECK(segunda.businesses().size() == 2);
    MOTOR_CHECK(segunda.businesses()[0].objectId == "posada");
    MOTOR_CHECK(segunda.businesses()[0].rentPercent == 140);
    MOTOR_CHECK(segunda.businesses()[1].objectId == "herreria");
    MOTOR_CHECK(segunda.businesses()[1].baseIncome == 20);
}

// Sin traspaso, una sesion nueva nace vacia: es lo que veia el jugador al
// cruzar una puerta antes de C-03, y lo que hace que la prueba de arriba
// signifique algo.
MOTOR_TEST_CASE(una_sesion_recien_construida_nace_sin_estado_de_jugador) {
    Fixture f;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Posada"), &f.body,
                        &f.playerSkills, &f.inventory);

    MOTOR_CHECK(session.gold() == 0);
    MOTOR_CHECK(session.morality().value() == 0);
    MOTOR_CHECK(session.businesses().empty());
    MOTOR_CHECK(session.captureCarryOver().stepsSinceRent == 0);
}

// applyCarryOver asigna, no acumula: aplicarlo dos veces deja lo mismo.
// Importa porque loadLevel lo llama una vez por carga y una recarga del
// mismo nivel no debe duplicar el oro.
MOTOR_TEST_CASE(aplicar_el_traspaso_dos_veces_no_duplica_nada) {
    Fixture f;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);

    session.applyCarryOver(makeState());
    session.applyCarryOver(makeState());

    MOTOR_CHECK(session.gold() == 90);
    MOTOR_CHECK(session.businesses().size() == 2);
    MOTOR_CHECK(session.morality().value() == -30);
}

// El traspaso no toca lo que no es suyo: la vida y el inventario viven
// fuera de la sesion y no deben verse afectados.
MOTOR_TEST_CASE(el_traspaso_no_toca_vida_ni_inventario) {
    Fixture f;
    f.body.takeDamage(25);
    f.inventory.push_back("pocion");

    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    session.applyCarryOver(makeState());

    MOTOR_CHECK(f.body.health() == 35);
    MOTOR_CHECK(f.inventory.size() == 1);
}

// C-24 — Xarxa de seguretat al camí que es juga: economía de negocios y
// moralidad. Unit y GL-free: se ejercita GameSession (motor_level, sin
// ventana) sobre niveles sin mapa, igual que los casos de C-03 y
// SaveGameTests.

// Catalogo compartido por la economia: un cartel de negocio en venta, un
// tendero con tienda (la tienda anade dos articulos que se descartan al
// resolver la oferta: uno que no existe en el catalogo y uno sin precio —
// "cv" nunca vende, pero este es el caso de cobertura de C-24) y dos
// pickups (una pocion con precio y una llave de mision sin el).
static ObjectCatalog makeCatalogEconomia() {
    ObjectCatalog catalog;
    auto result = catalog.loadFromString(R"({
        "objects": [
            {"id": "cartel_posada", "name": "Cartel del Cuervo", "category": "prop",
             "blocksMovement": true,
             "business": {"price": 500, "baseIncome": 20, "name": "Posada del Cuervo"}},
            {"id": "pocion", "name": "Pocion", "category": "pickup",
             "pickup": {"effect": "heal", "power": 15}, "price": 40},
            {"id": "llave_oxidada", "name": "Llave oxidada", "category": "pickup",
             "pickup": {"effect": "none"}},
            {"id": "tendero", "name": "Bran", "category": "npc",
             "dialogue": {"lines": ["Bienvenido a mi puesto."]},
             "shop": {"items": ["pocion", "fantasma", "llave_oxidada"], "buybackPercent": 60}}
        ]
    })");
    MOTOR_CHECK(result.isOk());
    return catalog;
}

// Nivel sin mapa con UN objeto en una celda concreta: la sesion lo puebla
// en el World (C-17) y el jugador puede interactuar con el desde la
// casilla de salida si queda adjacente.
static LevelDefinition makeLevelConObjeto(const char* name, const char* objectId,
                                          GridCoord pos) {
    LevelDefinition level;
    level.name = name;
    level.playerStart = GridCoord{0, 0};
    ObjectSpawn spawn;
    spawn.objectId = objectId;
    spawn.position = pos;
    level.objects.push_back(spawn);
    return level;
}

// Camina "steps" celdas alternado derecha/izquierda: siempre se puede
// (sin mapa nada bloquea) y cada paso cuenta para el ciclo de rentas.
static void andar(GameSession& session, int steps) {
    for (int i = 0; i < steps; ++i) {
        MOTOR_CHECK(session.tryMovePlayer(i % 2 == 0 ? 1 : -1, 0));
    }
}

// La ocupacion es la restriccion que hace que subir el alquiler no sea
// gratis: a 100% se queda toda la clientela, y por cada punto extra se va
// una parte (2 de cada 5 por encima de lo justo) hasta quedarse vacio.
MOTOR_TEST_CASE(la_ocupacion_cae_al_subir_el_alquiler) {
    MOTOR_CHECK(GameSession::occupancyPercent(0) == 100);
    MOTOR_CHECK(GameSession::occupancyPercent(50) == 100);
    MOTOR_CHECK(GameSession::occupancyPercent(100) == 100);
    MOTOR_CHECK(GameSession::occupancyPercent(125) == 90);
    MOTOR_CHECK(GameSession::occupancyPercent(150) == 80);
    MOTOR_CHECK(GameSession::occupancyPercent(175) == 70);
    MOTOR_CHECK(GameSession::occupancyPercent(200) == 60);
    MOTOR_CHECK(GameSession::occupancyPercent(300) == 20);
    MOTOR_CHECK(GameSession::occupancyPercent(1000) == 0);
}

// El ingreso tiene un optimo interior: subir bastante da un poco mas,
// pero pasarse revierte (a 250% ya vuelve a lo de 100% y a 300% da menos
// que el alquiler justo). Por debajo de 100% tambien se pierde.
MOTOR_TEST_CASE(el_ingreso_tiene_un_optimo_interior) {
    MOTOR_CHECK(GameSession::incomeFor(20, 100) == 20);
    MOTOR_CHECK(GameSession::incomeFor(20, 150) == 24);
    MOTOR_CHECK(GameSession::incomeFor(20, 200) == 24);
    MOTOR_CHECK(GameSession::incomeFor(20, 250) == 20);
    MOTOR_CHECK(GameSession::incomeFor(20, 300) == 12);
    MOTOR_CHECK(GameSession::incomeFor(20, 50) == 10);
    MOTOR_CHECK(GameSession::incomeFor(100, 200) == 120);

    // La curvatura es la decision de diseno que se quiere congelar:
    MOTOR_CHECK(GameSession::incomeFor(20, 200) > GameSession::incomeFor(20, 100));
    MOTOR_CHECK(GameSession::incomeFor(20, 250) < GameSession::incomeFor(20, 200));
}

// El tiempo del juego son los pasos del jugador: cada 40 celdas andadas
// (kStepsPerRentCycle) se cobra un ciclo de rentas. Cobrar es donde la
// moral SOSTENIDA entra en juego: un alquiler generoso la sube cada vez y
// uno abusivo la baja cada vez, no solo al cambiar la renta.
MOTOR_TEST_CASE(cada_ciclo_de_renta_cobra_y_sostiene_la_moral) {
    Fixture f;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);

    PlayerCarryOver estado;
    estado.gold = 0;
    estado.moralityValue = 0;
    estado.stepsSinceRent = 0;
    // Generoso: ingresa 10 (20*50/10000*100) y sube +2 la moral cada
    // ciclo. Abusivo: ingresa 24 (20*200*60/10000) y baja -4. En
    // conjunto: 34 oro y -2 moral por ciclo.
    estado.businesses.push_back(OwnedBusiness{"posada", "Posada", 20, 50});
    estado.businesses.push_back(OwnedBusiness{"herreria", "Herreria", 20, 200});
    session.applyCarryOver(estado);

    // A los 39 pasos todavia no se ha cobrado (el ciclo no esta cerrado).
    andar(session, 39);
    MOTOR_CHECK(session.gold() == 0);
    MOTOR_CHECK(session.captureCarryOver().stepsSinceRent == 39);
    MOTOR_CHECK(session.morality().value() == 0);

    // El paso 40 cierra el ciclo: 34 oro y moral -2 por el par.
    andar(session, 1);
    MOTOR_CHECK(session.gold() == 34);
    MOTOR_CHECK(session.captureCarryOver().stepsSinceRent == 0);
    MOTOR_CHECK(session.morality().value() == -2);

    // Mantener una renta injusta tiene precio recurrente: segundo ciclo,
    // otros 34 oro y otro -2 de moral.
    andar(session, 40);
    MOTOR_CHECK(session.gold() == 68);
    MOTOR_CHECK(session.morality().value() == -4);
}

// PO-49 — Con la cuenta de dias en marcha, las rentas se cobran UNA vez por dia de historia:
// caminar en circulo no da dinero, y pasar al dia siguiente si.
MOTOR_TEST_CASE(las_rentas_se_cobran_una_vez_por_dia_de_historia) {
    Fixture f;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body, &f.playerSkills, &f.inventory);
    RPG::NarrativeEngine motor;
    RPG::NarrativeState historia;
    historia.setVariable(RPG::franjas::kVariableDia, 1);
    session.setNarrative(&motor, &historia);
    PlayerCarryOver estado;
    estado.businesses.push_back(OwnedBusiness{"posada", "Posada", 20, 100});  // 20 al 100 %: ingresa 20
    session.applyCarryOver(estado);

    andar(session, 200);  // el dia 1 se apunta, no se cobra, por mucho que se ande
    MOTOR_CHECK(session.gold() == 0);
    historia.setVariable(RPG::franjas::kVariableDia, 2);
    andar(session, 1);
    const int unaRenta = session.gold();
    MOTOR_CHECK(unaRenta > 0);
    andar(session, 200);  // el mismo dia no vuelve a cobrar
    MOTOR_CHECK(session.gold() == unaRenta);
    historia.setVariable(RPG::franjas::kVariableDia, 3);
    andar(session, 1);
    MOTOR_CHECK(session.gold() == 2 * unaRenta);
}

// El ciclo completo del negocio: abrir el cartel, comprarlo y subir/bajar
// el alquiler. La moral se mueve con el CAMBIO (-1 por cada 5 puntos), y
// el alquiler se clampa a [50, 200]: pedir mas alla de un tope no mueve
// nada (no se puede farmear reputacion).
MOTOR_TEST_CASE(comprar_un_negocio_y_subir_su_alquiler_mueve_la_moral) {
    ObjectCatalog catalog = makeCatalogEconomia();
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills,
                        makeLevelConObjeto("Ciudad", "cartel_posada", GridCoord{0, 1}),
                        &body, &playerSkills, &inventory);
    session.addGold(1000);

    // El cartel es un prop: bloquea el paso desde la casilla de salida
    // incluso en Exploration.
    MOTOR_CHECK(!session.tryMovePlayer(0, 1));
    MOTOR_CHECK(session.mode() == GameMode::Exploration);

    // El cartel al lado abre el modo Business con los datos del local.
    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Business);
    MOTOR_CHECK(session.currentBusinessId() == "cartel_posada");
    MOTOR_CHECK(session.currentBusinessName() == "Posada del Cuervo");
    MOTOR_CHECK(session.currentBusinessPrice() == 500);
    MOTOR_CHECK(session.currentBusinessIncome() == 20);

    // Fuera de Exploration no se camina ni avanza el mundo.
    MOTOR_CHECK(!session.tryMovePlayer(0, 1));
    session.update(0.5f);

    // Sin ser dueno, subir la renta es un no-op (y avisa en el log).
    MOTOR_CHECK(!session.adjustCurrentRent(50));
    MOTOR_CHECK(session.morality().value() == 0);

    // La compra cuesta lo que dice el cartel y arranca a alquiler justo.
    MOTOR_CHECK(session.buyCurrentBusiness());
    MOTOR_CHECK(session.gold() == 500);
    MOTOR_CHECK(session.ownsBusiness("cartel_posada"));
    MOTOR_CHECK(session.businesses()[0].rentPercent == 100);
    MOTOR_CHECK(!session.buyCurrentBusiness());  // ya es tuyo
    MOTOR_CHECK(session.gold() == 500);

    // Subir 60 puntos de golpe: moral -60/5 = -12.
    MOTOR_CHECK(session.adjustCurrentRent(60));
    MOTOR_CHECK(session.businesses()[0].rentPercent == 160);
    MOTOR_CHECK(session.morality().value() == -12);

    // Bajar 160 hasta el minimo clampa en 50; el cambio aplicado (-110)
    // sube la moral -(-110)/5 = +22, hasta +10.
    MOTOR_CHECK(session.adjustCurrentRent(-160));
    MOTOR_CHECK(session.businesses()[0].rentPercent == 50);
    MOTOR_CHECK(session.morality().value() == 10);

    // Ya en el tope, pedir bajar mas no mueve nada (ni la moral).
    MOTOR_CHECK(!session.adjustCurrentRent(-100));
    MOTOR_CHECK(session.morality().value() == 10);

    session.closeInteraction();
    MOTOR_CHECK(session.mode() == GameMode::Exploration);

    // Sin combate en curso, syncBattleOutcome es un no-op.
    MOTOR_CHECK(!session.syncBattleOutcome());
}

// La tienda: comprar descuenta el precio de la oferta y entrega el
// objeto; vender paga el buybackPercent (60%) del precio de catalogo, y
// un objeto sin precio (la llave de mision) no se vende. Indices fuera de
// rango y oro insuficiente no rompen nada.
MOTOR_TEST_CASE(la_tienda_compra_y_recompra_por_su_margen) {
    ObjectCatalog catalog = makeCatalogEconomia();
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory{"pocion", "llave_oxidada"};
    GameSession session(nullptr, &catalog, &skills,
                        makeLevelConObjeto("Mercado", "tendero", GridCoord{0, 1}),
                        &body, &playerSkills, &inventory);
    session.addGold(100);

    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Shop);
    MOTOR_CHECK(session.shopKeeper() == "Bran");
    MOTOR_CHECK(session.shopOffers().size() == 1);
    MOTOR_CHECK(session.shopOffers()[0].objectId == "pocion");
    MOTOR_CHECK(session.shopOffers()[0].price == 40);

    // Comprar descuenta el precio exacto y entrega el objeto.
    MOTOR_CHECK(session.buy(0));
    MOTOR_CHECK(session.gold() == 60);
    MOTOR_CHECK(inventory.size() == 3);

    // Indice fuera de rango: no-op.
    MOTOR_CHECK(!session.buy(9));
    MOTOR_CHECK(session.gold() == 60);

    // Vender: 40 * 60/100 = 24 oro (nunca por debajo de 1).
    MOTOR_CHECK(session.sell(0));
    MOTOR_CHECK(session.gold() == 84);
    MOTOR_CHECK(inventory.size() == 2);

    // La llave de mision (precio 0 del catalogo) no se vende.
    MOTOR_CHECK(!session.sell(0));
    MOTOR_CHECK(session.gold() == 84);
    MOTOR_CHECK(inventory.size() == 2);
}

// TUTORIAL-01 — Un beat al hablar con un tendero se LEE antes de comprar: la hoja del
// dialogo con sus lineas (una sola vez en el registro) y, al cerrarla, la
// tienda; cerrar la tienda vuelve a explorar. La segunda visita, sin beat, la
// tienda en el acto, como siempre.
MOTOR_TEST_CASE(un_beat_del_tendero_se_lee_antes_de_abrir_la_tienda) {
    ObjectCatalog catalog = makeCatalogEconomia();
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
    const auto guion = RPG::AdventureScript::loadFromString(R"({"id": "tut", "beats": [
        {"id": "tut_tendero", "trigger": {"type": "talk", "target": "tendero"},
         "requires": {"notFlags": ["tut_tendero"]}, "speaker": "Bran",
         "lines": ["Asi que eres el nuevo.", "Mira lo que quieras."],
         "effects": [{"type": "setFlag", "arg": "tut_tendero"}]}]})");
    MOTOR_CHECK(guion.isOk());
    if (!guion.isOk()) return;
    RPG::NarrativeEngine engine;
    engine.setAdventure(&guion.value());
    RPG::NarrativeState state;
    GameSession session(nullptr, &catalog, &skills, makeLevelConObjeto("Mercado", "tendero", GridCoord{0, 1}),
                        &body, &playerSkills, &inventory);
    session.setNarrative(&engine, &state);
    session.addGold(100);

    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Dialogue);
    MOTOR_CHECK(session.dialogueLines().size() == 2);
    MOTOR_CHECK(!session.buy(0));  // hablando aun no se compra
    MOTOR_CHECK(session.gold() == 100);
    MOTOR_CHECK(std::count(session.log().begin(), session.log().end(), "Bran: Mira lo que quieras.") == 1);

    session.closeInteraction();
    MOTOR_CHECK(session.mode() == GameMode::Shop);
    MOTOR_CHECK(session.shopOffers().size() == 1);
    MOTOR_CHECK(session.buy(0));
    MOTOR_CHECK(session.gold() == 60);
    session.closeInteraction();
    MOTOR_CHECK(session.mode() == GameMode::Exploration);

    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Shop);
}

// H3-05 — El ciclo completo del negocio: comprar, caminar y cobrar.
// collectRent() existe (lo verifica medir_alcance) pero nadie de fuera
// lo llama; este test cierra el circuito: al comprar un local y andar
// kStepsPerRentCycle (40) celdas, la renta se cobra automaticamente y el
// oro sube. A alquiler justo (100%) la ocupacion es 100% y no hay efecto
// de moral por SOSTENER (solo por CAMBIAR); el ingreso es baseIncome.
MOTOR_TEST_CASE(comprar_negocio_cobrar_renta_aumenta_oro) {
    ObjectCatalog catalog = makeCatalogEconomia();
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills,
                        makeLevelConObjeto("Ciudad", "cartel_posada", GridCoord{0, 1}),
                        &body, &playerSkills, &inventory);
    session.addGold(1000);

    // Abrir el cartel → modo Business.
    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Business);

    // Comprar: 1000 → 500. El negocio tiene baseIncome 20, rentPercent 100.
    MOTOR_CHECK(session.buyCurrentBusiness());
    MOTOR_CHECK(session.gold() == 500);
    MOTOR_CHECK(session.ownsBusiness("cartel_posada"));
    MOTOR_CHECK(session.businesses()[0].rentPercent == 100);

    // Dejar el modo Business y andar.
    session.closeInteraction();
    MOTOR_CHECK(session.mode() == GameMode::Exploration);

    // 39 pasos: el ciclo no esta cerrado, no se cobra nada.
    andar(session, 39);
    MOTOR_CHECK(session.gold() == 500);

    // El paso 40 cierra el ciclo: incomeFor(20, 100) = 20*100*100/10000 = 20.
    andar(session, 1);
    MOTOR_CHECK(session.gold() == 520);
    MOTOR_CHECK(session.businesses()[0].rentPercent == 100);  // sigue al 100%

    // El moratriz no cambia: a 100% el sostenido es (100-100)/25 = 0.
    MOTOR_CHECK(session.morality().value() == 0);
}

// Mientras el test anterior cubre la renta justa (sin efecto moral por
// sostener), este verifica que AJUSTAR el alquiler ANTES de cobrar cambia
// el ingreso y la moral SOSTENIDA: a 200% la ocupacion cae al 60% pero
// el ingreso por cliente sube, y la moral baja por cobrar de mas.
MOTOR_TEST_CASE(comprar_negocio_subir_alquiler_cobrar_renta_tiene_coste) {
    ObjectCatalog catalog = makeCatalogEconomia();
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
    GameSession session(nullptr, &catalog, &skills,
                        makeLevelConObjeto("Ciudad", "cartel_posada", GridCoord{0, 1}),
                        &body, &playerSkills, &inventory);
    session.addGold(1000);

    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.buyCurrentBusiness());       // 1000 → 500, rent 100
    session.closeInteraction();                       // → Exploration

    // Volver a abrir el cartel para subir el alquiler al 200%.
    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Business);
    MOTOR_CHECK(session.adjustCurrentRent(100));      // 100 → 200 (% clamp)
    MOTOR_CHECK(session.businesses()[0].rentPercent == 200);
    MOTOR_CHECK(session.morality().value() == -20);   // -100/5 = -20
    session.closeInteraction();

    // Cobrar: incomeFor(20, 200) = 20*200*60/10000 = 24 (ocupacion 60%).
    // Ademas, el sostenido abusivo: (100-200)/25 = -4 → moral -4 mas.
    andar(session, 40);
    MOTOR_CHECK(session.gold() == 524);               // 500 + 24
    MOTOR_CHECK(session.morality().value() == -24);   // -20 (cambio) + -4 (sostenido)

    // Un segundo ciclo SIN ajustar: sigue cobrando 24 y la moral baja otros 4.
    andar(session, 40);
    MOTOR_CHECK(session.gold() == 548);
    MOTOR_CHECK(session.morality().value() == -28);
}
MOTOR_TEST_CASE(la_moral_se_clampa_y_se_nombra_por_tramos) {
    Morality moral;
    MOTOR_CHECK(moral.value() == 0);
    MOTOR_CHECK(moral.fraction() == 0.5f);
    MOTOR_CHECK(std::string(moral.label()) == "NEUTRAL");

    // record acumula, clampa y anota con el signo del delta en cabeza.
    moral.record(40, "buena accion");
    moral.record(80, "otra buena accion");
    MOTOR_CHECK(moral.value() == 100);  // 120 => tope 100
    MOTOR_CHECK(moral.fraction() == 1.0f);
    moral.record(-50, "mala accion");
    MOTOR_CHECK(moral.value() == 50);

    // Un delta 0 no ensucia el historial.
    moral.record(0, "accion neutra");
    MOTOR_CHECK(moral.history().size() == 3);
    MOTOR_CHECK(moral.history()[0] == "-50  mala accion");
    MOTOR_CHECK(moral.history()[1] == "+80  otra buena accion");
    MOTOR_CHECK(moral.history()[2] == "+40  buena accion");

    // setValue restaura sin registrar motivo (cargar partida guardada).
    moral.setValue(-100);
    MOTOR_CHECK(moral.value() == -100);
    MOTOR_CHECK(moral.fraction() == 0.0f);
    MOTOR_CHECK(std::string(moral.label()) == "VILLANO");
    MOTOR_CHECK(moral.history().size() == 3);

    // Tramos (MORAL-01): <=-75 VILLANO, <=-60 DESGRACIADO, <=-20 CANALLA,
    // <20 NEUTRAL, <60 RESPETADO, <75 SALVADOR.
    moral.setValue(-60);
    MOTOR_CHECK(std::string(moral.label()) == "DESGRACIADO");
    moral.setValue(-30);
    MOTOR_CHECK(std::string(moral.label()) == "CANALLA");
    moral.setValue(19);
    MOTOR_CHECK(std::string(moral.label()) == "NEUTRAL");
    moral.setValue(40);
    MOTOR_CHECK(std::string(moral.label()) == "RESPETADO");
    moral.setValue(74);
    MOTOR_CHECK(std::string(moral.label()) == "SALVADOR");

    // El tramo de heroe arranca en 75 y deja de ser SALVADOR. Nota: el
    // label concreto de este tramo lleva una discrepancia pequena entre
    // el contrato de Morality.h ("sin acentos") y el codigo actual
    // ("HÉROE" acentuado); se fija el UMBRAL y no el glifo, y la
    // discrepancia queda registrada en la sesion C-24.
    moral.setValue(75);
    const std::string labelHeroe = moral.label();
    MOTOR_CHECK(labelHeroe != "SALVADOR");
    MOTOR_CHECK(labelHeroe != "NEUTRAL");
}

// El historial de motivos esta acotado a los 12 ultimos: material para el
// HUD, no un libro de contabilidad.
MOTOR_TEST_CASE(la_moral_acota_su_historial_a_los_ultimos_motivos) {
    Morality moral;
    for (int i = 0; i < 15; ++i) {
        moral.record(1, "motivo_" + std::to_string(i));
    }
    MOTOR_CHECK(moral.history().size() == 12);
    // Del mas reciente al mas antiguo: el 14 en cabeza, el 3 ya salio.
    MOTOR_CHECK(moral.history()[0] == "+1  motivo_14");
    MOTOR_CHECK(moral.history()[11] == "+1  motivo_3");
}

// D-08 — Els enemics del joc són els del catàleg. GameSession construeix
// cada EnemyBrain del món des de la seva MonsterDefinition del catàleg RPG
// (estadístiques, defenses, tier i habilitats), no des de quatre números
// del ObjectCatalog. La reserva Nd6 penja del stat de l'habilitat
// (1 + stat, SkillExecutor/Skill.cpp), així que dos monstres amb stats
// diferents donen reserves de daus i defenses diferents.
MOTOR_TEST_CASE(monstres_del_cataleg_donen_reserves_i_defenses_diferents) {
    // ObjectCatalog del món: un llop i un drac, tots dos de categoría enemy.
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({
        "objects": [
            {"id": "llop", "name": "Llop", "category": "enemy",
             "combat": {"maxHealth": 8, "maxMana": 0, "skills": ["udol"]}},
            {"id": "drac", "name": "Drac", "category": "enemy",
             "combat": {"maxHealth": 60, "maxMana": 6, "skills": ["flamarada"]}}
        ]
    })")
                   .isOk());

    // MonsterCatalog RPG: la versio REAL dels mateixos ids amb stats,
    // defenses (D-03: maxHealth viu dins defenses{}), tier i habilitats.
    RPG::Catalogs::MonsterCatalog monsters;
    MOTOR_CHECK(monsters.loadFromString(R"({
        "entries": [
            {"id": "llop", "name": "Llop de la Boira", "tier": 1, "role": "striker",
             "stats": {"CON": 3, "DES": 5, "INT": 1, "CAR": 0},
             "defenses": {"maxHealth": 18, "ca": 13, "defMental": 11, "resFis": 12},
             "skillIds": ["udol"]},
            {"id": "drac", "name": "Drac Ancia", "tier": 4, "role": "conjurador",
             "stats": {"CON": 6, "DES": 3, "INT": 7, "CAR": 5},
             "defenses": {"maxHealth": 120, "ca": 17, "defMental": 16, "resFis": 15},
             "skillIds": ["flamarada", "ala"]}
        ]
    })")
                   .isOk());

    // Nivell sense mapa amb els dos enemics pousats (patrulla degenerada:
    // enemics quiets, igual que fa LevelLoader sense patrulla al JSON).
    LevelDefinition level;
    level.name = "Guarida";
    level.playerStart = GridCoord{0, 0};
    level.objects.push_back(
        ObjectSpawn{"llop", GridCoord{3, 1}, GridCoord{3, 1}, GridCoord{3, 1}});
    level.objects.push_back(
        ObjectSpawn{"drac", GridCoord{5, 1}, GridCoord{5, 1}, GridCoord{5, 1}});

    Fixture f;
    GameSession session(nullptr, &objects, &f.skills, level, &f.body, &f.playerSkills,
                        &f.inventory, nullptr /* world: propio */, &monsters);

    // Els dos enemics vius al món (C-17: World n'és el propietari; enemies()
    // en dóna una instantània de lectura amb els brains compartits).
    const std::vector<WorldEnemy> enemies = session.enemies();
    MOTOR_CHECK(enemies.size() == 2);

    const WorldEnemy* llop = nullptr;
    const WorldEnemy* drac = nullptr;
    for (const WorldEnemy& e : enemies) {
        if (e.brain->combatant_id() == "llop") llop = &e;
        if (e.brain->combatant_id() == "drac") drac = &e;
    }
    MOTOR_CHECK(llop != nullptr);
    MOTOR_CHECK(drac != nullptr);

    // La vida i el tier venen del MonsterDefinition, no del ObjectCatalog
    // (8/60 allà, 18/120 aquí).
    MOTOR_CHECK(llop->brain->maxHealth() == 18);
    MOTOR_CHECK(drac->brain->maxHealth() == 120);
    MOTOR_CHECK(llop->brain->tier() == 1);
    MOTOR_CHECK(drac->brain->tier() == 4);

    // Estadístiques del catàleg: la reserva Nd6 = 1 + stat de l'habilitat,
    // així que stats diferents volen dir reserves de daus diferents.
    MOTOR_CHECK(llop->brain->stat(RPG::Stat::DES) == 5);
    MOTOR_CHECK(drac->brain->stat(RPG::Stat::INT) == 7);
    const int llopPool = 1 + llop->brain->stat(RPG::Stat::DES);
    const int dracPool = 1 + drac->brain->stat(RPG::Stat::INT);
    MOTOR_CHECK(llopPool != dracPool);
    MOTOR_CHECK(llopPool == 6);
    MOTOR_CHECK(dracPool == 8);

    // Defenses legacy des del stats: CA = 10 + DES + armadura; TS fís =
    // 10 + CON; TS vol = 10 + INT; DC = 10 + CAR. Les quatre difereixen.
    const RPG::DefenseBlock llop_def = llop->brain->defenses();
    const RPG::DefenseBlock drac_def = drac->brain->defenses();
    MOTOR_CHECK(llop_def.values[static_cast<int>(RPG::Defense::ARMOR_CLASS)] == 15);
    MOTOR_CHECK(llop_def.values[static_cast<int>(RPG::Defense::PHYSICAL_SAVE)] == 13);
    MOTOR_CHECK(drac_def.values[static_cast<int>(RPG::Defense::PHYSICAL_SAVE)] == 16);
    MOTOR_CHECK(drac_def.values[static_cast<int>(RPG::Defense::WILL_SAVE)] == 17);
    for (int i = 0; i < 4; ++i) {
        MOTOR_CHECK(llop_def.values[i] != drac_def.values[i]);
    }

    // Habilitats: les que coneix cada enemic son els skillIds de la seva
    // MonsterDefinition (el drac en sap dues; el llop en sap una).
    // knownSkillIds() retorna els ids ORDENATS ALFABÈTICAMENT.
    const std::vector<std::string> llop_skills = llop->skills->knownSkillIds();
    const std::vector<std::string> drac_skills = drac->skills->knownSkillIds();
    MOTOR_CHECK(llop_skills.size() == 1 && llop_skills[0] == "udol");
    MOTOR_CHECK(drac_skills.size() == 2);
    // Ordre alfabètic: "ala" < "flamarada"
    MOTOR_CHECK(drac_skills[0] == "ala");
    MOTOR_CHECK(drac_skills[1] == "flamarada");
}

// C-24 — Cobertura: la narrativa llega a la sesion por el MISMO camino
// que el juego (enterLevelNarrative/talkNarrative) y desde ahi se
// ejercita applyNarrative entero con datos reales: oro, log, dos tiradas
// Nd6 (una con rivalDice de la Ed. 3 y otra con cd heredada) y un combate
// cuyo enemigo no esta en el catalogo (se da por perdido, flag de
// derrota). Fixture GL-free: sin mapa, guion cargado de memoria.

MOTOR_TEST_CASE(un_beat_al_hablar_aplica_oro_log_tiradas_y_combate_roto) {
    Fixture f;
    RPG::Catalogs::SkillCatalog nd6;
    RPG::SkillDefinition percepcio;
    percepcio.id = "percepcion";
    percepcio.name = "Percepcion";
    percepcio.casting_stat = RPG::Stat::CON;
    nd6.insert(percepcio);

    const RPG::AdventureScript guion = makeAdventureProva();
    RPG::NarrativeEngine engine;
    engine.setAdventure(&guion);
    RPG::NarrativeState state;
    RngFixo dado({1, 1, 1, 1, 1, 1, 1});

    StatCombatant body{60, RPG::Stat::CON, 1};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &body,
                        &playerSkills, &inventory);
    session.setNd6SkillCatalog(&nd6);
    session.setRandomEngine(dado);
    session.setNarrative(&engine, &state);

    MOTOR_CHECK(session.talkNarrative("tendero_narrativo"));

    // Oro del beat (grantGold +7) y el modo dialogo con las lineas del beat.
    MOTOR_CHECK(session.gold() == 7);
    MOTOR_CHECK(session.mode() == GameMode::Dialogue);
    MOTOR_CHECK(session.dialogueSpeaker() == "Voz");
    MOTOR_CHECK(session.dialogueLines().size() == 2);

    // Tiradas: el jugador tira 1 dado (stat CON 1) y todos salen 1; el
    // rival saca 1,1,1 (2+.) y 1,1 -> pifia Ed. 3 del DEFENSOR -> la tirada
    // enfrentada se confirma en grado CRITICAL (DicePoolEngine).
    MOTOR_CHECK(state.hasFlag("tirada_critico"));
    MOTOR_CHECK(!state.hasFlag("tirada_nada"));
    MOTOR_CHECK(!state.hasFlag("tirada_exito"));
    MOTOR_CHECK(state.hasFlag("cerradura_critico"));
    MOTOR_CHECK(!state.hasFlag("cerradura_nada"));
    // H2-01 — La foto de la ultima, para la hoja del dialogo: la cerradura
    // (cd 1,0 -> 2 dados del rival, los dos 1: pifia del rival).
    const RPG::TiradaVisible& foto = state.ultimaTirada();
    MOTOR_CHECK(foto.numero == 2 && foto.que == "Percepcion" && foto.quien == "Tú");
    MOTOR_CHECK(foto.enfrentada && foto.rival.rolls.size() == 2 && foto.rival.botch);
    MOTOR_CHECK(foto.grado == RPG::Degree::CRITICAL && !foto.pifia());

    // El combate con enemigo desconocido se da por perdido (no cuelga).
    MOTOR_CHECK(state.hasFlag("bnd_derrota"));
    MOTOR_CHECK(!state.hasFlag("bnd_victoria"));

    // El envoltorio publico startNarrativeBattle hace lo mismo en directo.
    session.startNarrativeBattle(
        {.monsterId = "alien", .flagVictory = "v1", .flagDefeat = "d1"});
    MOTOR_CHECK(state.hasFlag("d1"));
}

// Los cuatro grados de la tirada enfrentada, con el rival VACIO (cd 0 ->
// 0 dados) y dos caras del jugador por check (stat CON 2). Caras 1,1 ->
// pifia propia -> BOTCH; 2,2 -> 0 exitos, empate -> PARTIAL; 5,4 -> 0.5
// exitos -> SUCCESS; 6,6 -> 1 exito + critico -> CRITICAL. Cubre los
// cuatro case del switch.
MOTOR_TEST_CASE(una_tirada_se_puede_resolver_en_los_cuatro_grados) {
    Fixture f;
    RPG::Catalogs::SkillCatalog nd6;
    RPG::SkillDefinition percepcio;
    percepcio.id = "percepcion";
    percepcio.name = "Percepcion";
    percepcio.casting_stat = RPG::Stat::CON;
    nd6.insert(percepcio);

    const RPG::AdventureScript guion = makeAdventureProva();
    RPG::NarrativeEngine engine;
    engine.setAdventure(&guion);
    RPG::NarrativeState state;
    RngFixo dado({1, 1, 2, 2, 5, 4, 6, 6});

    StatCombatant body{60, RPG::Stat::CON, 2};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &body,
                        &playerSkills, &inventory);
    session.setNd6SkillCatalog(&nd6);
    session.setRandomEngine(dado);
    session.setNarrative(&engine, &state);

    MOTOR_CHECK(session.talkNarrative("tendero_grados"));

    MOTOR_CHECK(state.hasFlag("grado_nada"));
    MOTOR_CHECK(state.hasFlag("grado2_parcial"));
    MOTOR_CHECK(state.hasFlag("grado3_exito"));
    MOTOR_CHECK(state.hasFlag("grado4_critico"));
    MOTOR_CHECK(state.flagCount() == 4);
    // H2-01 — Sin rival (cd 0) la tirada no es enfrentada y la ultima es 6,6.
    MOTOR_CHECK(state.ultimaTirada().numero == 4 && !state.ultimaTirada().enfrentada);
    MOTOR_CHECK(state.ultimaTirada().tirada.rolls == std::vector<int>({6, 6}));
}

MOTOR_TEST_CASE(entrar_a_un_nivel_sin_beat_no_dispara_y_la_entrada_si) {
    Fixture f;
    RPG::Catalogs::SkillCatalog nd6;
    RPG::SkillDefinition percepcio;
    percepcio.id = "percepcion";
    percepcio.name = "Percepcion";
    nd6.insert(percepcio);

    const RPG::AdventureScript guion = makeAdventureProva();
    RPG::NarrativeEngine engine;
    engine.setAdventure(&guion);
    RPG::NarrativeState state;
    RngFixo dado({1, 1, 1, 1, 1, 1});

    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &f.body,
                        &f.playerSkills, &f.inventory);
    session.setNd6SkillCatalog(&nd6);
    session.setRandomEngine(dado);
    session.setNarrative(&engine, &state);

    // NPC sin beat y nivel sin beat de entrada: no se dispara nada y la
    // sesion no cambia de modo ni enciende flags.
    MOTOR_CHECK(!session.talkNarrative("tendero_parado"));
    MOTOR_CHECK(!session.enterLevelNarrative("assets/niveles/otro.json"));
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
    MOTOR_CHECK(session.gold() == 0);

    // Entrar al nivel del guio: el beat de entrada dispara oro y flag.
    MOTOR_CHECK(session.enterLevelNarrative("assets/niveles/prova_c24.json"));
    MOTOR_CHECK(session.mode() == GameMode::Dialogue);
    MOTOR_CHECK(session.gold() == 3);
    MOTOR_CHECK(state.hasFlag("bnd_llegada"));

    // El notFlags lo veta: no se repite ni vuelve a pagar.
    MOTOR_CHECK(!session.enterLevelNarrative("assets/niveles/prova_c24.json"));
    MOTOR_CHECK(session.gold() == 3);

    // Reloj de timers con motor puesto pero sin timers activos: no toca nada.
    session.updateNarrativeTimers(1.0f, "assets/niveles/prova_c24.json");
    MOTOR_CHECK(state.flagCount() == 1);

    // El oro nunca baja de cero: grantGold -50 sobre 3 queda en 0.
    MOTOR_CHECK(session.talkNarrative("tendero_pobre"));
    MOTOR_CHECK(session.gold() == 0);
}

// Una puerta sobre la propia casilla del jugador (se PISA, no se mira)
// deja la transicion pendiente con su nivel destino y punto de entrada.
MOTOR_TEST_CASE(una_puerta_en_la_celda_del_jugador_abre_transicion) {
    ObjectCatalog catalog;
    MOTOR_CHECK(catalog.loadFromString(R"({
        "objects": [
            {"id": "puerta", "name": "Puerta", "category": "prop",
             "blocksMovement": true}
        ]
    })")
                   .isOk());

    LevelDefinition level = makeLevel("Superficie");
    ObjectSpawn puerta;
    puerta.objectId = "puerta";
    puerta.position = GridCoord{0, 0};
    puerta.targetLevel = "assets/niveles/sotano.json";
    puerta.targetPosition = GridCoord{2, 3};
    puerta.hasTargetPosition = true;
    level.objects.push_back(puerta);

    Fixture f;
    GameSession session(nullptr, &catalog, &f.skills, level, &f.body, &f.playerSkills,
                        &f.inventory);

    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
    MOTOR_CHECK(session.pendingTransition().pending);
    MOTOR_CHECK(session.pendingTransition().levelPath == "assets/niveles/sotano.json");
    MOTOR_CHECK(session.pendingTransition().hasEntryPosition);
    MOTOR_CHECK(session.pendingTransition().entryPosition.x == 2);
    MOTOR_CHECK(session.pendingTransition().entryPosition.y == 3);

    session.clearTransition();
    MOTOR_CHECK(!session.pendingTransition().pending);
}

// TUTORIAL-01 — Se sale por donde se entro. La sastreria tiene dos puertas a la calle
// (la de la plaza y la del sur) y una sola salida, que llevaba siempre a la del
// sur: se entraba por la plaza y se salia a la otra punta de la ciudad. Con la
// vuelta (ponVuelta, la pone Application al cruzar), la salida en la que se
// aparecio lleva a la puerta por la que se entro; otra salida, o una vuelta a
// otro nivel, siguen con su destino.
MOTOR_TEST_CASE(la_salida_de_un_interior_lleva_a_la_puerta_por_la_que_se_entro) {
    ObjectCatalog catalog;
    MOTOR_CHECK(catalog.loadFromString(R"({"objects": [{"id": "salida", "name": "Salida", "category": "prop"}]})")
                    .isOk());
    LevelDefinition level = makeLevel("Sastreria");
    level.playerStart = GridCoord{1, 7};
    ObjectSpawn salida;
    salida.objectId = "salida";
    salida.position = GridCoord{1, 7};
    salida.targetLevel = "assets/levels/ciudad.json";
    salida.targetPosition = GridCoord{56, 56};
    salida.hasTargetPosition = true;
    ObjectSpawn trasera = salida;
    trasera.position = GridCoord{4, 7};
    trasera.targetPosition = GridCoord{60, 61};
    level.objects = {salida, trasera};
    Fixture f;
    const auto destino = [&](const std::string& vueltaNivel, int pasos) {
        GameSession session(nullptr, &catalog, &f.skills, level, &f.body, &f.playerSkills, &f.inventory);
        if (!vueltaNivel.empty()) session.ponVuelta(vueltaNivel, GridCoord{41, 23});
        for (int i = 0; i < pasos; ++i) session.tryMovePlayer(1, 0);
        MOTOR_CHECK(session.interact());
        MOTOR_CHECK(session.pendingTransition().pending && session.pendingTransition().hasEntryPosition);
        return session.pendingTransition().entryPosition;
    };

    GridCoord c = destino("", 0);  // sin vuelta: el destino de la salida
    MOTOR_CHECK(c.x == 56 && c.y == 56);
    c = destino("assets/levels/ciudad.json", 0);  // la puerta por la que se entro
    MOTOR_CHECK(c.x == 41 && c.y == 23);
    c = destino("assets/levels/otro.json", 0);  // se llego de otro nivel
    MOTOR_CHECK(c.x == 56 && c.y == 56);
    c = destino("assets/levels/ciudad.json", 3);  // la otra salida, la suya
    MOTOR_CHECK(c.x == 60 && c.y == 61);
}

// Interactuar con un objeto que NO esta en el catalogo no rompe nada: el
// escaneo lo salta y sigue. Y cerrar una interaccion desde Exploration es
// un no-op (guard), tambien para syncBattleOutcome.
MOTOR_TEST_CASE(interactuar_con_un_objeto_ausente_del_catalogo_no_hace_nada) {
    ObjectCatalog catalog;
    Fixture f;
    GameSession session(nullptr, &catalog, &f.skills,
                        makeLevelConObjeto("Pradera", "misterio", GridCoord{0, 1}),
                        &f.body, &f.playerSkills, &f.inventory);

    MOTOR_CHECK(!session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
    MOTOR_CHECK(!session.pendingTransition().pending);

    session.closeInteraction();
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
}

// C-24 — Cobertura: el bucle de patrulla recorre el World y sincroniza el
// espejo de posicion del brain, sin mapa (sin mapa no bloquea nada).
MOTOR_TEST_CASE(la_patrulla_recorre_el_mundo_y_sincroniza_el_espejo_sin_mapa) {
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({
        "objects": [
            {"id": "llop", "name": "Llop de la Boira", "category": "enemy",
             "combat": {"maxHealth": 8, "maxMana": 0, "skills": ["udol"]}}
        ]
    })")
                   .isOk());

    RPG::Catalogs::MonsterCatalog monsters;
    MOTOR_CHECK(monsters.loadFromString(R"({
        "entries": [
            {"id": "llop", "name": "Llop de la Boira", "tier": 1, "role": "striker",
             "stats": {"CON": 3, "DES": 5, "INT": 1, "CAR": 0},
             "defenses": {"maxHealth": 18, "ca": 13, "defMental": 11, "resFis": 12},
             "skillIds": ["udol"]}
        ]
    })")
                   .isOk());

    LevelDefinition level;
    level.name = "Guarida";
    level.playerStart = GridCoord{0, 0};
    ObjectSpawn llop;
    llop.objectId = "llop";
    llop.position = GridCoord{2, 1};
    llop.patrolMin = GridCoord{2, 1};
    llop.patrolMax = GridCoord{4, 1};  // patrulla real, no degenerada
    level.objects.push_back(llop);

    Fixture f;
    GameSession session(nullptr, &objects, &f.skills, level, &f.body, &f.playerSkills,
                        &f.inventory, nullptr, &monsters);

    // Varios ticks: el bucle solo corre en Exploration y con enemigos vivos.
    session.update(1.0f);
    session.update(1.0f);
    session.update(1.0f);

    const std::vector<WorldEnemy> enemies = session.enemies();
    MOTOR_CHECK(enemies.size() == 1);
    MOTOR_CHECK(enemies[0].brain->isAlive());
}

MOTOR_TEST_CASE(pausar_congela_el_mundo_y_reanudar_lo_suelta) {
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({
        "objects": [
            {"id": "llop", "name": "Llop de la Boira", "category": "enemy",
             "combat": {"maxHealth": 8, "maxMana": 0, "skills": ["udol"]}}
        ]
    })")
                   .isOk());

    RPG::Catalogs::MonsterCatalog monsters;
    MOTOR_CHECK(monsters.loadFromString(R"({
        "entries": [
            {"id": "llop", "name": "Llop de la Boira", "tier": 1, "role": "striker",
             "stats": {"CON": 3, "DES": 5, "INT": 1, "CAR": 0},
             "defenses": {"maxHealth": 18, "ca": 13, "defMental": 11, "resFis": 12},
             "skillIds": ["udol"]}
        ]
    })")
                   .isOk());

    LevelDefinition level;
    level.name = "Guarida";
    level.playerStart = GridCoord{0, 0};
    ObjectSpawn llop;
    llop.objectId = "llop";
    llop.position = GridCoord{2, 1};
    llop.patrolMin = GridCoord{2, 1};
    llop.patrolMax = GridCoord{4, 1};  // patrulla real, no degenerada
    level.objects.push_back(llop);

    Fixture f;
    GameSession session(nullptr, &objects, &f.skills, level, &f.body, &f.playerSkills,
                        &f.inventory, nullptr, &monsters);

    // Deja que la patrulla arranque y lee donde esta el espejo. En
    // Exploration resume() es no-op (no hay nada que reanudar).
    session.update(1.0f);
    session.update(1.0f);
    session.update(1.0f);
    MOTOR_CHECK(!session.resume());
    const GridCoord posPatrulla = session.enemies()[0].position;

    // Pausar: entra en Pause solo desde Exploration y congela el mundo.
    MOTOR_CHECK(session.pause());
    MOTOR_CHECK(session.mode() == GameMode::Pause);
    session.update(1.0f);
    session.update(1.0f);
    session.update(1.0f);
    MOTOR_CHECK(session.enemies()[0].position.x == posPatrulla.x);
    MOTOR_CHECK(session.enemies()[0].position.y == posPatrulla.y);

    // Pausado tampoco se mueve ni se habla/comercia.
    MOTOR_CHECK(!session.tryMovePlayer(0, 1));
    MOTOR_CHECK(!session.interact());

    // Reanudar: vuelve a Exploration y la patrulla sigue.
    MOTOR_CHECK(session.resume());
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
    bool patrullaSigue = false;
    for (int i = 0; i < 20 && !patrullaSigue; ++i) {
        session.update(1.0f);
        patrullaSigue = session.enemies()[0].position.x != posPatrulla.x;
    }
    MOTOR_CHECK(patrullaSigue);
}

MOTOR_TEST_CASE(la_pausa_entra_en_exploracion_y_en_combate) {
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({
        "objects": [
            {"id": "llop", "name": "Llop", "category": "enemy",
             "combat": {"maxHealth": 8, "maxMana": 0, "skills": ["udol"]}}
        ]
    })")
                   .isOk());

    RPG::Catalogs::MonsterCatalog monsters;
    MOTOR_CHECK(monsters.loadFromString(R"({
        "entries": [
            {"id": "llop", "name": "Llop de la Boira", "tier": 1, "role": "striker",
             "stats": {"CON": 3, "DES": 5, "INT": 1, "CAR": 0},
             "defenses": {"maxHealth": 18, "ca": 13, "defMental": 11, "resFis": 12},
             "skillIds": ["udol"]}
        ]
    })")
                   .isOk());

    LevelDefinition level;
    level.name = "Guarida";
    level.playerStart = GridCoord{0, 0};
    ObjectSpawn llop;
    llop.objectId = "llop";
    llop.position = GridCoord{1, 0};        // justo al lado del jugador
    llop.patrolMin = GridCoord{1, 0};
    llop.patrolMax = GridCoord{1, 0};       // quiesta: solo para el combate
    level.objects.push_back(llop);

    Fixture f;
    GameSession session(nullptr, &objects, &f.skills, level, &f.body, &f.playerSkills,
                        &f.inventory, nullptr, &monsters);

    // Pisar la celda del enemigo dispara el combate (el bool es false: no
    // hubo movimiento; el modo cambia y quien llama debe mirar mode()).
    session.tryMovePlayer(1, 0);
    MOTOR_CHECK(session.mode() == GameMode::Battle);

    // D0034: en combate la pausa tambien entra, y reanudar vuelve al combate.
    MOTOR_CHECK(session.pause());
    MOTOR_CHECK(session.mode() == GameMode::Pause);
    MOTOR_CHECK(session.resume());
    MOTOR_CHECK(session.mode() == GameMode::Battle);
}


// H2-05 — Ganar un combate da lo que el monstruo declara. El botin ya
// estaba en los datos: es el campo `loot` de monsters.json (432
// monstruos, 28 piezas), que el cargador mete en MonsterDefinition::lootIds,
// y son ids de EQUIPO. Al vencer, cada pieza entra en la mochila por el
// camino de vuelta que ya usa el juego (objectIdForEquipment): si el
// catalogo de objetos tiene "item_<x>", ese es el id; si no, el propio
// id. Asi la pieza llega como una mas y se puede equipar o vender.
MOTOR_TEST_CASE(al_ganar_el_botin_del_monstruo_entra_en_la_mochila) {
    // "item_daga_de_milicia" existe en el catalogo de objetos y
    // "pocion_de_vigor" no: entre las dos se cubren los dos caminos del
    // id de vuelta (con prefijo y sin el).
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({
        "objects": [
            {"id": "llop", "name": "Llop", "category": "enemy",
             "combat": {"maxHealth": 8, "maxMana": 0, "skills": []}},
            {"id": "item_daga_de_milicia", "name": "Daga de milicia", "category": "pickup"}
        ]
    })")
                   .isOk());

    // Ids REALES de monsters.json: el botin viene en `loot`, no en
    // `lootIds` (D-03 lo arreglo; el cargador acepta las dos grafias).
    RPG::Catalogs::MonsterCatalog monsters;
    MOTOR_CHECK(monsters.loadFromString(R"({
        "entries": [
            {"id": "llop", "name": "Llop de la Boira", "tier": 1, "role": "striker",
             "stats": {"CON": 3, "DES": 5, "INT": 1, "CAR": 0},
             "defenses": {"maxHealth": 18, "ca": 13, "defMental": 11, "resFis": 12},
             "loot": ["daga_de_milicia", "pocion_de_vigor"]}
        ]
    })")
                   .isOk());

    // Catalogo de equipo: sin el, la mochila recibe los ids pero sin
    // nombre que enseñar ni equipar (H0-04 lo inyecta en el juego real).
    RPG::Catalogs::EquipmentCatalog equipo;
    MOTOR_CHECK(equipo.loadFromString(R"({
        "entries": [
            {"id": "daga_de_milicia", "name": "Daga de milicia", "slot": "MainHand",
             "price": 25, "weight": "light"},
            {"id": "pocion_de_vigor", "name": "Pocion de vigor", "slot": "Consumable",
             "price": 30, "weight": "light"}
        ]
    })")
                   .isOk());

    LevelDefinition level;
    level.name = "Guarida";
    level.playerStart = GridCoord{0, 0};
    ObjectSpawn llop;
    llop.objectId = "llop";
    llop.position = GridCoord{1, 0};
    llop.patrolMin = GridCoord{1, 0};
    llop.patrolMax = GridCoord{1, 0};  // quieto: aqui solo para el combate
    level.objects.push_back(llop);

    Fixture f;
    GameSession session(nullptr, &objects, &f.skills, level, &f.body, &f.playerSkills,
                        &f.inventory, nullptr /* world: propio */, &monsters);
    InventoryRules reglas;
    reglas.equipment = &equipo;
    session.setInventoryRules(reglas);

    // Pisar su celda abre el combate; se le deja a 1 PV y se golpea hasta
    // que cae (el dano depende del grado de la tirada: un golpe no basta).
    MOTOR_CHECK(!session.tryMovePlayer(1, 0));
    MOTOR_CHECK(session.mode() == GameMode::Battle);
    session.battle()->enemies()[0].combatant->takeDamage(17);
    BattleAction golpe;
    golpe.type = BattleActionType::Attack;
    golpe.targetIndex = 0;
    for (int intento = 0; intento < 50 && session.battle() != nullptr &&
                          session.battle()->outcome() == BattleOutcome::InProgress;
         ++intento) {
        session.battle()->resolveAllyAction(0, golpe);
    }
    MOTOR_CHECK(session.battle() != nullptr);
    MOTOR_CHECK(session.battle()->outcome() == BattleOutcome::Victory);

    // Cerrar el combate es lo que reparte el botin (mismo paso que se
    // lleva al enemigo del World): antes de esto, la mochila esta vacia.
    MOTOR_CHECK(f.inventory.empty());
    MOTOR_CHECK(session.syncBattleOutcome());
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
    MOTOR_CHECK(session.enemies().empty());

    // Las dos piezas estan, cada una con el id que entiende el juego.
    MOTOR_CHECK(f.inventory.size() == 2);
    MOTOR_CHECK(f.inventory[0] == "item_daga_de_milicia");
    MOTOR_CHECK(f.inventory[1] == "pocion_de_vigor");

    // Y el log lo nombra con el nombre de la ficha de equipo, no con el
    // id crudo (mismo criterio que "Te equipas X.").
    bool lineaBotin = false;
    for (const std::string& line : session.log()) {
        if (line.find("Botin:") != std::string::npos) {
            lineaBotin = true;
            MOTOR_CHECK(line.find("Daga de milicia") != std::string::npos);
            MOTOR_CHECK(line.find("Pocion de vigor") != std::string::npos);
        }
    }
    MOTOR_CHECK(lineaBotin);

    // La pieza se reconoce como equipo por el camino del propio juego
    // (equipmentFor es lo que usan equipar y vender): el id de vuelta no
    // la deja=colgada fuera del catalogo.
    MOTOR_CHECK(session.equipmentFor("item_daga_de_milicia") != nullptr);
    MOTOR_CHECK(session.equipmentFor("pocion_de_vigor") != nullptr);
}

// H2-05 — Huir no es ganar: el enemigo sobrevive y no suelta nada. Sin
// este caso, "repartir al cerrar el combate" abriria la puerta a soltar
// el botin de un enemigo que sigue vivo en su celda.
MOTOR_TEST_CASE(huir_no_suelta_botin_y_el_enemigo_sigue_vivo) {
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({
        "objects": [
            {"id": "llop", "name": "Llop", "category": "enemy",
             "combat": {"maxHealth": 8, "maxMana": 0, "skills": []}}
        ]
    })")
                   .isOk());

    RPG::Catalogs::MonsterCatalog monsters;
    MOTOR_CHECK(monsters.loadFromString(R"({
        "entries": [
            {"id": "llop", "name": "Llop de la Boira", "tier": 1, "role": "striker",
             "stats": {"CON": 3, "DES": 5, "INT": 1, "CAR": 0},
             "defenses": {"maxHealth": 18, "ca": 13, "defMental": 11, "resFis": 12},
             "loot": ["daga_de_milicia"]}
        ]
    })")
                   .isOk());

    LevelDefinition level;
    level.name = "Guarida";
    level.playerStart = GridCoord{0, 0};
    ObjectSpawn llop;
    llop.objectId = "llop";
    llop.position = GridCoord{1, 0};
    llop.patrolMin = GridCoord{1, 0};
    llop.patrolMax = GridCoord{1, 0};
    level.objects.push_back(llop);

    Fixture f;
    GameSession session(nullptr, &objects, &f.skills, level, &f.body, &f.playerSkills,
                        &f.inventory, nullptr, &monsters);

    MOTOR_CHECK(!session.tryMovePlayer(1, 0));
    MOTOR_CHECK(session.mode() == GameMode::Battle);
    BattleAction correr;
    correr.type = BattleActionType::Flee;
    session.battle()->resolveAllyAction(0, correr);
    MOTOR_CHECK(session.battle() != nullptr);
    MOTOR_CHECK(session.battle()->outcome() == BattleOutcome::Fled);
    MOTOR_CHECK(session.syncBattleOutcome());

    // Huir: mochila igual y el enemigo en su celda.
    MOTOR_CHECK(f.inventory.empty());
    MOTOR_CHECK(session.enemies().size() == 1);
}



// Oriol (05/10): «los combates del juego siguen siendo de uno contra uno».
// Un beat pide un grupo: salen todos (numerados si se repiten), y al ganar
// caen todos y suena la flag de victoria.
ObjectCatalog catalogoDeGrupo() {
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({
        "objects": [
            {"id": "llop", "name": "Llop", "category": "enemy",
             "combat": {"maxHealth": 8, "maxMana": 0, "skills": []}},
            {"id": "corb", "name": "Corb", "category": "enemy",
             "combat": {"maxHealth": 6, "maxMana": 0, "skills": []}}
        ]
    })")
                    .isOk());
    return objects;
}

MOTOR_TEST_CASE(un_combate_en_grupo_saca_a_todos_y_al_ganar_caen_todos) {
    ObjectCatalog objects = catalogoDeGrupo();
    Fixture f;
    RPG::NarrativeEngine engine;
    RPG::NarrativeState state;
    GameSession session(nullptr, &objects, &f.skills, makeLevel("Ciudad"), &f.body, &f.playerSkills,
                        &f.inventory);
    session.setNarrative(&engine, &state);
    session.startNarrativeBattle({.monsterId = "llop",
                                  .flagVictory = "grupo_vencido",
                                  .flagDefeat = "grupo_gana",
                                  .enemigos = {"llop", "llop", "corb"}});
    MOTOR_CHECK(session.mode() == GameMode::Battle);
    MOTOR_CHECK(session.battle() != nullptr);
    if (session.battle() == nullptr) return;
    MOTOR_CHECK(session.battle()->enemies().size() == 3);
    MOTOR_CHECK(session.enemies().size() == 3);
    MOTOR_CHECK(session.battle()->enemies()[0].name == "Llop");
    MOTOR_CHECK(session.battle()->enemies()[1].name == "Llop 2");
    MOTOR_CHECK(session.battle()->enemies()[2].name == "Corb");
    // Dos caen; el tercero, a golpes: hasta que no cae el ultimo no se gana.
    session.battle()->enemies()[0].combatant->takeDamage(100);
    session.battle()->enemies()[1].combatant->takeDamage(100);
    session.battle()->enemies()[2].combatant->takeDamage(5);
    BattleAction golpe;
    golpe.type = BattleActionType::Attack;
    golpe.targetIndex = 2;
    for (int intento = 0; intento < 50 && session.battle()->outcome() == BattleOutcome::InProgress; ++intento)
        session.battle()->resolveAllyAction(0, golpe);
    MOTOR_CHECK(session.battle()->outcome() == BattleOutcome::Victory);
    MOTOR_CHECK(session.syncBattleOutcome());
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
    MOTOR_CHECK(session.enemies().empty());
    MOTOR_CHECK(state.hasFlag("grupo_vencido") && !state.hasFlag("grupo_gana"));
}

// TUTORIAL-01 — La emboscada sale alrededor del jugador, cada uno en su casilla: todos
// encima del heroe, en el combate no se les veia (ni a quien se apuntaba).
MOTOR_TEST_CASE(una_emboscada_sale_alrededor_del_jugador_y_no_encima) {
    ObjectCatalog objects = catalogoDeGrupo();
    Fixture f;
    RPG::NarrativeEngine engine;
    RPG::NarrativeState state;
    GameSession session(nullptr, &objects, &f.skills, makeLevel("Ciudad"), &f.body, &f.playerSkills,
                        &f.inventory);
    session.setNarrative(&engine, &state);
    session.startNarrativeBattle({.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d",
                                  .enemigos = {"llop", "llop", "corb"}});
    const GridCoord p = session.playerPosition();
    std::set<std::pair<int, int>> casillas;
    for (const auto& e : session.enemies()) {
        MOTOR_CHECK(std::abs(e.position.x - p.x) <= 1 && std::abs(e.position.y - p.y) <= 1);
        MOTOR_CHECK(e.position.x != p.x || e.position.y != p.y);
        casillas.insert({e.position.x, e.position.y});
    }
    MOTOR_CHECK(session.enemies().size() == 3 && casillas.size() == 3);
}

// TUTORIAL-01 — Lo que dice un beat se lee antes de pelear: el combate queda montado
// (el enemigo ya esta en el mundo) y empieza al cerrar el dialogo. Al acabar,
// la aventura dice lo suyo sin esperar a otra charla (el «tut_victoria»).
MOTOR_TEST_CASE(un_beat_que_habla_y_pelea_se_lee_antes_y_la_victoria_se_cuenta_al_acabar) {
    ObjectCatalog objects = catalogoDeGrupo();
    Fixture f;
    const auto guion = RPG::AdventureScript::loadFromString(R"({"id": "emb", "beats": [
        {"id": "emboscada", "trigger": {"type": "talk", "target": "vigia"},
         "requires": {"notFlags": ["lanzada"]}, "speaker": "Saqueador", "lines": ["La bolsa."],
         "effects": [{"type": "setFlag", "arg": "lanzada"},
                     {"type": "startBattle", "startBattle": {"monsterId": "llop",
                      "flagVictory": "vencido", "flagDefeat": "gana"}}]},
        {"id": "victoria", "trigger": {"type": "auto"},
         "requires": {"allFlags": ["vencido"], "notFlags": ["cerrado"]},
         "speaker": "", "lines": ["Nadie ha mirado."],
         "effects": [{"type": "setFlag", "arg": "cerrado"}]}]})");
    MOTOR_CHECK(guion.isOk());
    if (!guion.isOk()) return;
    RPG::NarrativeEngine engine;
    engine.setAdventure(&guion.value());
    RPG::NarrativeState state;
    GameSession session(nullptr, &objects, &f.skills, makeLevel("Ciudad"), &f.body, &f.playerSkills,
                        &f.inventory);
    session.setNarrative(&engine, &state);

    MOTOR_CHECK(session.talkNarrative("vigia"));
    MOTOR_CHECK(session.mode() == GameMode::Dialogue);
    MOTOR_CHECK(session.battle() != nullptr && session.enemies().size() == 1);
    MOTOR_CHECK(session.dialogueLines().size() == 1 && session.dialogueLines()[0] == "La bolsa.");
    session.closeInteraction();
    MOTOR_CHECK(session.mode() == GameMode::Battle);
    if (session.battle() == nullptr) return;

    session.battle()->enemies()[0].combatant->takeDamage(7);
    BattleAction golpe;
    golpe.type = BattleActionType::Attack;
    golpe.targetIndex = 0;
    for (int intento = 0; intento < 50 && session.battle()->outcome() == BattleOutcome::InProgress; ++intento)
        session.battle()->resolveAllyAction(0, golpe);
    MOTOR_CHECK(session.battle()->outcome() == BattleOutcome::Victory);
    MOTOR_CHECK(session.syncBattleOutcome());
    MOTOR_CHECK(state.hasFlag("vencido") && state.hasFlag("cerrado"));
    MOTOR_CHECK(session.mode() == GameMode::Dialogue);
    MOTOR_CHECK(session.dialogueLines().size() == 1 && session.dialogueLines()[0] == "Nadie ha mirado.");
    session.closeInteraction();
    MOTOR_CHECK(session.mode() == GameMode::Exploration);
}

MOTOR_TEST_CASE(huir_de_un_grupo_deja_a_los_vivos_y_se_lleva_a_los_caidos) {
    ObjectCatalog objects = catalogoDeGrupo();
    Fixture f;
    RPG::NarrativeEngine engine;
    RPG::NarrativeState state;
    GameSession session(nullptr, &objects, &f.skills, makeLevel("Ciudad"), &f.body, &f.playerSkills,
                        &f.inventory);
    session.setNarrative(&engine, &state);
    session.startNarrativeBattle({.monsterId = "corb",
                                  .flagVictory = "v",
                                  .flagDefeat = "d",
                                  .enemigos = {"corb", "corb", "corb"}});
    MOTOR_CHECK(session.battle() != nullptr && session.battle()->enemies().size() == 3);
    if (session.battle() == nullptr) return;
    session.battle()->enemies()[1].combatant->takeDamage(100);  // uno cae antes de huir
    BattleAction correr;
    correr.type = BattleActionType::Flee;
    session.battle()->resolveAllyAction(0, correr);
    MOTOR_CHECK(session.battle()->outcome() == BattleOutcome::Fled);
    MOTOR_CHECK(session.syncBattleOutcome());
    MOTOR_CHECK(session.enemies().size() == 2);
    MOTOR_CHECK(state.hasFlag("d"));  // huir cuenta como derrota para la trama
}

// El formato: "enemigos" con cantidades, o "cantidad" junto a monsterId; se
// escribe compacto y de ida y vuelta da lo mismo; un id de mas, error claro.
MOTOR_TEST_CASE(el_grupo_se_lee_se_escribe_y_se_valida) {
    const char* json = R"({
      "id": "grupos",
      "beats": [
        {"id": "emboscada", "trigger": {"type": "auto"}, "lines": ["Salen tres."],
         "effects": [{"type": "startBattle", "startBattle": {
            "enemigos": [{"id": "perdido_saqueador", "cantidad": 2}, {"id": "perdido_fanatico"}],
            "flagVictory": "v1", "flagDefeat": "d1"}}]},
        {"id": "pareja", "trigger": {"type": "auto"}, "lines": ["Dos."],
         "effects": [{"type": "startBattle", "startBattle": {
            "monsterId": "cultista_carcelero", "cantidad": 2, "flagVictory": "v2", "flagDefeat": "d2"}}]},
        {"id": "solo", "trigger": {"type": "auto"}, "lines": ["Uno."],
         "effects": [{"type": "startBattle", "startBattle": {
            "monsterId": "gorran_bandido", "flagVictory": "v3", "flagDefeat": "d3"}}]}
      ]
    })";
    const auto leida = RPG::AdventureScript::loadFromString(json);
    MOTOR_CHECK(leida.isOk());
    if (!leida.isOk()) return;
    const RPG::BattleRequest& a = leida.value().beats[0].effects[0].battle;
    MOTOR_CHECK(a.monsterId == "perdido_saqueador");
    MOTOR_CHECK((a.grupo() == std::vector<std::string>{"perdido_saqueador", "perdido_saqueador", "perdido_fanatico"}));
    const RPG::BattleRequest& b = leida.value().beats[1].effects[0].battle;
    MOTOR_CHECK(b.grupo().size() == 2 && b.grupo()[1] == "cultista_carcelero");
    const RPG::BattleRequest& c = leida.value().beats[2].effects[0].battle;
    MOTOR_CHECK(c.enemigos.empty() && c.grupo().size() == 1);
    const std::string texto = RPG::serializeAdventure(leida.value());
    MOTOR_CHECK(texto.find("\"cantidad\": 2") != std::string::npos);
    const auto otra = RPG::AdventureScript::loadFromString(texto);
    MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == texto);
    MOTOR_CHECK(otra.isOk() && otra.value().beats[0].effects[0].battle.grupo() == a.grupo());
    // Nueve son demasiados para la mesa; cero no es una cantidad.
    MOTOR_CHECK(!RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b", "trigger": {"type": "auto"},
        "effects": [{"type": "startBattle", "startBattle": {"monsterId": "corb", "cantidad": 9,
        "flagVictory": "v", "flagDefeat": "d"}}]}]})").isOk());
    MOTOR_CHECK(!RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b", "trigger": {"type": "auto"},
        "effects": [{"type": "startBattle", "startBattle": {"enemigos": [{"id": "corb", "cantidad": 0}],
        "flagVictory": "v", "flagDefeat": "d"}}]}]})").isOk());
}

// H2-08 — El precio del equipo sale del CATALOGO REAL.
//
// El agujero que tapa este par de casos: `basePriceOf` solo miraba
// `ObjectDefinition::pickup.price`, y en los catalogos de objetos y niveles
// hay CERO objetos de equipo con precio. O sea, que los 413 precios de
// assets/catalogs/equipment.json no llegaban a la economia, y las 282
// piezas que llegan al jugador por vias que no pasan por el catalogo de
// objetos (el botin de los monstruos, el equipo inicial de las clases, los
// pregenerados) valian 0. Y precio 0 son dos cosas: la tienda lo descarta
// deliberadamente («a coste 0 seria oro infinito») y vender responde «eso no
// tiene precio aqui». El botin de H2-05 no se podia vender.
//
// El fixture copia esa forma: dos piezas de equipo SIN precio en el objeto,
// una pocion que no es equipo y si lo tiene, y un tendero que le pone
// precio a una de las dos piezas.
static ObjectCatalog makeCatalogJoyeria() {
    ObjectCatalog catalog;
    auto result = catalog.loadFromString(R"({
        "objects": [
            {"id": "item_anillo_de_brasa", "name": "Anillo de brasa", "category": "pickup",
             "pickup": {"effect": "none"}},
            {"id": "item_colgante_de_escarcha", "name": "Colgante de escarcha", "category": "pickup",
             "pickup": {"effect": "none"}},
            {"id": "pocion", "name": "Pocion", "category": "pickup",
             "pickup": {"effect": "heal", "power": 15}, "price": 40},
            {"id": "joyera", "name": "Nara", "category": "npc",
             "dialogue": {"lines": ["Las piedras cantan solas."]},
             "shop": {"items": ["item_anillo_de_brasa", "pocion",
                                {"objectId": "item_colgante_de_escarcha", "price": 250}],
                      "buybackPercent": 60}}
        ]
    })");
    MOTOR_CHECK(result.isOk());
    return catalog;
}

// H2-08 — La pieza entra en la oferta por el precio del catalogo de equipo
// (100), la pocion sigue a su precio de objeto (40) porque no es equipo, y
// el precio del PNJ manda sobre el del catalogo (250).
MOTOR_TEST_CASE(el_precio_del_equipo_sale_del_catalogo_real) {
    ObjectCatalog catalog = makeCatalogJoyeria();
    RPG::Catalogs::EquipmentCatalog equipo;
    MOTOR_CHECK(equipo.loadFromString(R"({
        "entries": [
            {"id": "anillo_de_brasa", "name": "Anillo de brasa", "slot": "Accessory",
             "price": 100, "weight": "light"},
            {"id": "colgante_de_escarcha", "name": "Colgante de escarcha", "slot": "Accessory",
             "price": 80, "weight": "light"}
        ]
    })")
                   .isOk());

    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory{"item_anillo_de_brasa"};
    GameSession session(nullptr, &catalog, &skills,
                        makeLevelConObjeto("Joyeria", "joyera", GridCoord{0, 1}),
                        &body, &playerSkills, &inventory);
    InventoryRules reglas;
    reglas.equipment = &equipo;
    session.setInventoryRules(reglas);
    session.addGold(500);

    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Shop);

    // Antes eran 1: el anillo valia 0 y `resolveShopOffers` lo descartaba.
    MOTOR_CHECK(session.shopOffers().size() == 3);

    // Anillo: 100, el del catalogo real. El objeto NO tiene pickup.price.
    MOTOR_CHECK(session.shopOffers()[0].objectId == "item_anillo_de_brasa");
    MOTOR_CHECK(session.shopOffers()[0].price == 100);

    // Pocion: 40, su precio de objeto. Lo que no es equipo no cambia.
    MOTOR_CHECK(session.shopOffers()[1].objectId == "pocion");
    MOTOR_CHECK(session.shopOffers()[1].price == 40);

    // Colgante: el PNJ lo vende a 250 y el catalogo dice 80. Manda el PNJ.
    MOTOR_CHECK(session.shopOffers()[2].objectId == "item_colgante_de_escarcha");
    MOTOR_CHECK(session.shopOffers()[2].price == 250);
}

// H2-08 — El otro rostro del mismo bug: vender. La pieza que entro en la
// mochila por el botin de H2-05 no tiene precio en el objeto, asi que antes
// `sell` respondia «eso no tiene precio aqui» y devolvia false. Con el
// catalogo de equipo se vende a su precio de base con el margen del
// tendero: 100 * 60/100 = 60.
MOTOR_TEST_CASE(una_pieza_de_equipo_del_botin_se_vende_a_su_precio_de_catalogo) {
    ObjectCatalog catalog = makeCatalogJoyeria();
    RPG::Catalogs::EquipmentCatalog equipo;
    MOTOR_CHECK(equipo.loadFromString(R"({
        "entries": [
            {"id": "anillo_de_brasa", "name": "Anillo de brasa", "slot": "Accessory",
             "price": 100, "weight": "light"},
            {"id": "colgante_de_escarcha", "name": "Colgante de escarcha", "slot": "Accessory",
             "price": 80, "weight": "light"}
        ]
    })")
                   .isOk());

    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    // La mochila llega con la pieza puesta: el camino de vuelta de H2-05
    // (objectIdForEquipment) deja "item_anillo_de_brasa" si el objeto existe.
    std::vector<std::string> inventory{"item_anillo_de_brasa"};
    GameSession session(nullptr, &catalog, &skills,
                        makeLevelConObjeto("Joyeria", "joyera", GridCoord{0, 1}),
                        &body, &playerSkills, &inventory);
    InventoryRules reglas;
    reglas.equipment = &equipo;
    session.setInventoryRules(reglas);

    MOTOR_CHECK(session.interact());
    MOTOR_CHECK(session.mode() == GameMode::Shop);
    MOTOR_CHECK(session.gold() == 0);

    // Antes: false y «eso no tiene precio aqui».
    MOTOR_CHECK(session.sell(0));
    MOTOR_CHECK(session.gold() == 60);
    MOTOR_CHECK(inventory.empty());

    bool lineaVenta = false;
    for (const std::string& line : session.log()) {
        if (line.find("Vendes") != std::string::npos) {
            lineaVenta = true;
            MOTOR_CHECK(line.find("60") != std::string::npos);
        }
    }
    MOTOR_CHECK(lineaVenta);
}



// ACOMPANANTES-01 — Un heroe con ficha y cartas (con mesa) y una aventura que
// define a Venides y a Xila. El combate trae a los que pide; Xila, solo con
// su flag. Las curas de Xila valen para toda la aventura: se apuntan al
// cerrar el combate y el siguiente empieza con las que quedan.
namespace {
class CuerpoConFicha : public ICombatant {
public:
    explicit CuerpoConFicha(RPG::CharacterSheet* f) : ficha(f) {}
    RPG::CharacterSheet* ficha;
    int vida = 40;
    RPG::CharacterSheet* character_sheet() override { return ficha; }
    const RPG::CharacterSheet* character_sheet() const override { return ficha; }
    void takeDamage(int n) override { vida = std::max(0, vida - n); }
    void heal(int n) override { vida = std::min(40, vida + n); }
    int health() const override { return vida; }
    int maxHealth() const override { return 40; }
    bool isAlive() const override { return vida > 0; }
    int stat(RPG::Stat s) const override { return s == RPG::Stat::DES ? 4 : 2; }
    RPG::DefenseBlock defenses() const override { return {}; }
    int tier() const override { return 1; }
    std::string combatant_id() const override { return "heroe"; }
};
}  // namespace

MOTOR_TEST_CASE(un_combate_de_la_aventura_trae_a_sus_acompanantes_y_sus_curas_duran_toda_ella) {
    auto cartas = Game::CatalogoCartas::cargar("assets/catalogs");
    MOTOR_CHECK(cartas.isOk());
    if (!cartas.isOk()) return;
    const auto aventura = RPG::AdventureScript::loadFromString(R"({
      "id": "ocaso_prueba",
      "acompanantes": [
        {"id": "venides", "nombre": "Venides", "vida": 16, "stats": {"CON": 3, "DES": 2}, "ataques": 2,
         "alCaer": "se_retira"},
        {"id": "xila", "nombre": "Xila", "vida": 6, "ataques": 0, "atacable": false,
         "cura": {"dado": 4, "mas": 1, "veces": 2, "umbral": 0.5}}
      ],
      "beats": []
    })");
    MOTOR_CHECK(aventura.isOk());
    if (!aventura.isOk()) return;
    ObjectCatalog objects;  // un oso que no cae de un golpe: asi le da tiempo a Xila
    MOTOR_CHECK(objects.loadFromString(R"({"objects": [{"id": "oso", "name": "Oso", "category": "enemy",
        "combat": {"maxHealth": 200, "maxMana": 0, "skills": []}}]})").isOk());
    RPG::CharacterSheet ficha;
    ficha.tier = 1;
    ficha.knownSkillIds = {"cancion_del_otono", "cancion_del_verano"};
    CuerpoConFicha heroe(&ficha);
    SkillCatalog skills;
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
    RPG::NarrativeEngine engine;
    engine.setAdventure(&aventura.value());
    RPG::NarrativeState state;
    GameSession session(nullptr, &objects, &skills, makeLevel("Ciudad"), &heroe, &playerSkills, &inventory);
    session.setNarrative(&engine, &state);
    session.setCatalogoCartas(cartas.value().get());
    RPG::BattleRequest pide{.monsterId = "oso", .flagVictory = "v", .flagDefeat = "d"};
    pide.acompanantes = {{"venides", ""}, {"xila", "conoce_xila"}};
    const auto gana = [&] {
        for (const BattleParticipant& e : session.battle()->enemies()) e.combatant->takeDamage(1000);
        session.battle()->resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});
        MOTOR_CHECK(session.battle()->outcome() == BattleOutcome::Victory);
        MOTOR_CHECK(session.syncBattleOutcome());
    };

    // Sin la flag de Xila, solo Venides, detras del heroe.
    session.startNarrativeBattle(pide);
    MOTOR_CHECK(session.battle() != nullptr);
    if (session.battle() == nullptr) return;
    MOTOR_CHECK(session.battle()->allies().size() == 2);
    MOTOR_CHECK(session.battle()->allies()[1].name == "Venides");
    MOTOR_CHECK(session.battle()->mesa() != nullptr);
    bool aTuLado = false;
    for (const std::string& l : session.log()) aTuLado |= l == "A tu lado: Venides.";
    MOTOR_CHECK(aTuLado);
    gana();

    // Con la flag, tambien Xila; cura al heroe en apuros y gasta una.
    state.setFlag("conoce_xila");
    session.startNarrativeBattle(pide);
    MOTOR_CHECK(session.battle() != nullptr && session.battle()->allies().size() == 3);
    if (session.battle() == nullptr || session.battle()->allies().size() != 3) return;
    const auto* xila = dynamic_cast<const Game::Acompanante*>(session.battle()->allies()[2].combatant);
    MOTOR_CHECK(xila != nullptr && xila->curasQueQuedan() == 2);
    heroe.vida = 10;  // 10 de 40: en apuros; pasa el turno y actuan los suyos
    session.battle()->resolveAllyAction(0, BattleAction{BattleActionType::UseSkill, Game::kAccionPasar, 0, ""});
    MOTOR_CHECK(heroe.vida > 10);
    MOTOR_CHECK(xila != nullptr && xila->curasQueQuedan() == 1);
    gana();
    MOTOR_CHECK(state.variable(Game::Acompanante::claveCuras("ocaso_prueba", "xila")) == 1);

    // El siguiente combate de la aventura empieza con la que queda.
    session.startNarrativeBattle(pide);
    MOTOR_CHECK(session.battle() != nullptr && session.battle()->allies().size() == 3);
    if (session.battle() == nullptr || session.battle()->allies().size() != 3) return;
    xila = dynamic_cast<const Game::Acompanante*>(session.battle()->allies()[2].combatant);
    MOTOR_CHECK(xila != nullptr && xila->curasQueQuedan() == 1);
}

// AV13-29 — Con mapa, el combate monta su tablero: Venides esta en el mundo (un objeto
// "acompanante_venides" en su casilla, para su dibujo), no se guarda, y al acabar se va.
MOTOR_TEST_CASE(en_el_tablero_el_acompanante_esta_en_el_mundo_hasta_que_acaba) {
    auto cartas = Game::CatalogoCartas::cargar("assets/catalogs");
    MOTOR_CHECK(cartas.isOk());
    if (!cartas.isOk()) return;
    const auto aventura = RPG::AdventureScript::loadFromString(R"({"id": "x",
      "acompanantes": [{"id": "venides", "nombre": "Venides", "vida": 16, "ataques": 2}], "beats": []})");
    MOTOR_CHECK(aventura.isOk());
    if (!aventura.isOk()) return;
    ObjectCatalog objects;
    MOTOR_CHECK(objects.loadFromString(R"({"objects": [{"id": "oso", "name": "Oso", "category": "enemy",
        "combat": {"maxHealth": 200, "maxMana": 0, "skills": []}}]})").isOk());
    RPG::CharacterSheet ficha;
    ficha.tier = 1;
    ficha.knownSkillIds = {"cancion_del_otono"};
    CuerpoConFicha heroe(&ficha);
    SkillCatalog skills;
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
    RPG::NarrativeEngine engine;
    engine.setAdventure(&aventura.value());
    RPG::NarrativeState state;
    struct Prado : RuntimeTileMap {
        Prado() {
            RuntimeTileLayer capa;
            for (int i = 0; i < 20 * 20; ++i) capa.tiles.emplace_back(1, false);
            replaceCompiledData(20, 20, 64, 32, {capa});
        }
    } prado;
    GameSession session(&prado, &objects, &skills, makeLevel("Ciudad"), &heroe, &playerSkills, &inventory);
    session.setNarrative(&engine, &state);
    session.setCatalogoCartas(cartas.value().get());
    RPG::BattleRequest pide{.monsterId = "oso", .flagVictory = "v", .flagDefeat = "d"};
    pide.acompanantes = {{"venides", ""}};
    session.startNarrativeBattle(pide);
    MOTOR_CHECK(session.battle() != nullptr && session.battle()->tablero() != nullptr);
    if (session.battle() == nullptr || session.battle()->tablero() == nullptr) return;
    std::optional<GridCoord> dibujo;
    const auto busca = [&] {
        dibujo.reset();
        session.world()->eachEntity([&](Engine::EntityHandle, const Engine::WorldEntity& e) {
            if (e.kind == Engine::EntityKind::Object && e.objectId() == "acompanante_venides") dibujo = e.position();
        });
    };
    busca();
    const auto casilla = session.battle()->tablero()->dondeEsta("Venides");
    MOTOR_CHECK(dibujo && casilla && dibujo->x == casilla->x && dibujo->y == casilla->y);
    // Anda (su turno o no: el tablero manda) y su objeto le sigue.
    const auto libre = session.battle()->tablero()->libreMasCercana(GridCoord{casilla->x + 2, casilla->y});
    MOTOR_CHECK(libre.has_value());
    if (libre) {
        session.battle()->tablero()->mueve("Venides", *libre);
        MOTOR_CHECK(!session.sincronizaTablero().empty());
        busca();
        MOTOR_CHECK(dibujo && dibujo->x == libre->x && dibujo->y == libre->y);
    }
    bool guardado = false;
    for (const auto& o : session.captureSaveGame("x").worldObjects) guardado |= o.objectId.rfind("acompanante_", 0) == 0;
    MOTOR_CHECK(!guardado);
    for (const BattleParticipant& e : session.battle()->enemies()) e.combatant->takeDamage(1000);
    session.battle()->resolveAllyAction(0, BattleAction{BattleActionType::Attack, "", 0, ""});
    MOTOR_CHECK(session.syncBattleOutcome());
    session.world()->flushPendingChanges();
    busca();
    MOTOR_CHECK(!dibujo.has_value());
}

// AV13-35 — Sin catalogo de cartas (las pruebas) la mesa se monta igual, con uno vacio: ya no hay
// combate sin mesa, y los acompanantes vienen.
MOTOR_TEST_CASE(sin_catalogo_de_cartas_hay_mesa_y_los_acompanantes_vienen) {
    const auto aventura = RPG::AdventureScript::loadFromString(R"({
      "id": "x", "acompanantes": [{"id": "venides", "vida": 16}], "beats": []})");
    MOTOR_CHECK(aventura.isOk());
    if (!aventura.isOk()) return;
    ObjectCatalog objects = catalogoDeGrupo();
    Fixture f;
    RPG::NarrativeEngine engine;
    engine.setAdventure(&aventura.value());
    RPG::NarrativeState state;
    GameSession session(nullptr, &objects, &f.skills, makeLevel("Ciudad"), &f.body, &f.playerSkills, &f.inventory);
    session.setNarrative(&engine, &state);
    RPG::BattleRequest pide{.monsterId = "llop", .flagVictory = "v", .flagDefeat = "d"};
    pide.acompanantes = {{"venides", ""}};
    session.startNarrativeBattle(pide);
    MOTOR_CHECK(session.battle() != nullptr && session.battle()->allies().size() == 2 && session.battle()->mesa() != nullptr);
}

// NIVELES-01 — Los beats dan niveles; sin los dos hitos de Tier 1 no se pasa
// del 20 ni de Tier 1; con ellos, Tier 2. El guardado se lleva nivel e hitos.
MOTOR_TEST_CASE(los_beats_dan_niveles_y_el_hito_deja_pasar_a_tier_2_y_se_guarda) {
    const auto aventura = RPG::AdventureScript::loadFromString(R"({
      "id": "niveles_prueba",
      "beats": [
        {"id": "canon", "trigger": {"type": "talk", "target": "alcaldesa"}, "lines": ["Gracias."],
         "effects": [{"type": "grantLevel", "value": 7, "arg": "acabar el prologo"}]},
        {"id": "necesidad", "trigger": {"type": "auto"}, "requires": {"nivelMinimo": 8},
         "lines": ["Un mensajero te busca: el banco necesita a alguien de confianza."],
         "effects": [{"type": "setFlag", "arg": "encargo_banco"}]},
        {"id": "todo", "trigger": {"type": "talk", "target": "skilla"}, "lines": ["Todo."],
         "effects": [{"type": "grantLevel", "value": 20}]},
        {"id": "paso", "trigger": {"type": "talk", "target": "griffin"}, "lines": ["Un paso."],
         "effects": [{"type": "grantLevel", "value": 0.25}]},
        {"id": "fin_campana", "trigger": {"type": "talk", "target": "venides"}, "lines": ["Vivimos."],
         "effects": [{"type": "grantMilestone", "arg": "la primera campana"}]},
        {"id": "fin_capitulo_2", "trigger": {"type": "talk", "target": "verina"}, "lines": ["Otra vez."],
         "effects": [{"type": "grantMilestone", "arg": "el capitulo 2"}]}
      ]})");
    MOTOR_CHECK(aventura.isOk());
    if (!aventura.isOk()) return;
    RPG::TierRules tiers;
    MOTOR_CHECK(tiers.loadFromFile("assets/rules/tier_rules.json").isOk());
    RPG::CharacterSheet ficha;
    ficha.tier = 1;
    CuerpoConFicha heroe(&ficha);
    Fixture f;
    RPG::NarrativeEngine engine;
    engine.setAdventure(&aventura.value());
    RPG::NarrativeState state;
    GameSession session(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &heroe, &f.playerSkills, &f.inventory);
    session.setNarrative(&engine, &state);
    InventoryRules reglas;
    reglas.tiers = &tiers;
    session.setInventoryRules(reglas);
    const auto enLog = [&](const std::string& texto) {
        return std::count(session.log().begin(), session.log().end(), texto);
    };

    MOTOR_CHECK(!engine.tick(state).fired);  // nivel 1: la necesidad aun no surge
    MOTOR_CHECK(session.talkNarrative("alcaldesa"));
    MOTOR_CHECK(ficha.nivel == 8 && ficha.tier == 1);
    MOTOR_CHECK(state.nivelHeroe() == 8 && state.tierHeroe() == 1);
    MOTOR_CHECK(engine.tick(state).fired && state.hasFlag("encargo_banco"));  // al llegar al 8, surge
    MOTOR_CHECK(enLog("Por acabar el prologo.") == 1 && enLog("Subes a nivel 8.") == 1);
    MOTOR_CHECK(session.talkNarrative("griffin"));  // un cuarto de nivel
    MOTOR_CHECK(ficha.nivel == 8 && ficha.progresoNivel == 25);
    MOTOR_CHECK(enLog("Nivel 8, 25 % hacia el 9.") == 1);
    MOTOR_CHECK(session.talkNarrative("skilla"));
    MOTOR_CHECK(ficha.nivel == 20 && ficha.tier == 1);  // sin hito, ni el 21 ni Tier 2
    MOTOR_CHECK(ficha.progresoNivel == 0);
    MOTOR_CHECK(enLog("Nivel 20: hace falta un hito para seguir subiendo.") == 1);
    MOTOR_CHECK(session.talkNarrative("venides"));
    MOTOR_CHECK(ficha.milestones == 1 && ficha.tier == 1);  // un hito no basta: hacen falta dos
    MOTOR_CHECK(enLog("Hito: la primera campana.") == 1 && enLog("Subes a Tier 2.") == 0);
    MOTOR_CHECK(enLog("Consigues un hito (1 de 2 para Tier 2).") == 1);
    MOTOR_CHECK(session.talkNarrative("verina"));
    MOTOR_CHECK(ficha.milestones == 2 && ficha.tier == 2);
    MOTOR_CHECK(enLog("Subes a Tier 2.") == 1);

    // El guardado lleva nivel e hitos; una ficha nueva (T1) vuelve a T2.
    const SaveGameData guardado = session.captureSaveGame("nivel.json");
    MOTOR_CHECK(guardado.playerLevel == 20 && guardado.playerMilestones == 2);
    RPG::CharacterSheet otra;
    otra.tier = 1;
    CuerpoConFicha otroHeroe(&otra);
    GameSession cargada(nullptr, &f.catalog, &f.skills, makeLevel("Ciudad"), &otroHeroe, &f.playerSkills,
                        &f.inventory);
    cargada.setInventoryRules(reglas);
    RPG::NarrativeState estadoCargado;
    cargada.setNarrative(&engine, &estadoCargado);
    std::string error;
    MOTOR_CHECK(cargada.restoreSaveGame(guardado, &error));
    MOTOR_CHECK(estadoCargado.nivelHeroe() == 20 && estadoCargado.tierHeroe() == 2);
    MOTOR_CHECK(otra.nivel == 20 && otra.milestones == 2 && otra.tier == 2);
}

// H2-04 / NIVELES-01 — grantMilestone y grantLevel se leen (1 si no se
// dice), se escriben de ida y vuelta igual y rechazan cantidades que no son
// contenido.
MOTOR_TEST_CASE(el_efecto_grantmilestone_se_lee_se_escribe_y_se_valida) {
    const auto leida = RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [
        {"id": "b", "trigger": {"type": "auto"}, "effects": [{"type": "grantMilestone"},
         {"type": "grantMilestone", "value": 2, "arg": "dos de golpe"}]}]})");
    MOTOR_CHECK(leida.isOk());
    if (!leida.isOk()) return;
    const auto& efectos = leida.value().beats[0].effects;
    MOTOR_CHECK(efectos[0].type == RPG::EffectType::GrantMilestone && efectos[0].value == 1);
    MOTOR_CHECK(efectos[1].value == 2 && efectos[1].arg == "dos de golpe");
    const std::string texto = RPG::serializeAdventure(leida.value());
    const auto otra = RPG::AdventureScript::loadFromString(texto);
    MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == texto);
    MOTOR_CHECK(!RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b",
        "trigger": {"type": "auto"}, "effects": [{"type": "grantMilestone", "value": 6}]}]})").isOk());
    MOTOR_CHECK(!RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b",
        "trigger": {"type": "auto"}, "effects": [{"type": "grantMilestone", "value": -1}]}]})").isOk());
    const auto niveles = RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b",
        "trigger": {"type": "auto"}, "effects": [{"type": "grantLevel"}, {"type": "grantLevel", "value": 3},
        {"type": "grantLevel", "value": 0.125}]}]})");
    // En puntos: 100 = un nivel; 0.125 (un paso de una secundaria) = 13.
    MOTOR_CHECK(niveles.isOk() && niveles.value().beats[0].effects[0].type == RPG::EffectType::GrantLevel &&
                niveles.value().beats[0].effects[0].value == 100 && niveles.value().beats[0].effects[1].value == 300 &&
                niveles.value().beats[0].effects[2].value == 13);
    if (niveles.isOk()) {
        const std::string texto = RPG::serializeAdventure(niveles.value());
        const auto otra = RPG::AdventureScript::loadFromString(texto);
        MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == texto &&
                    otra.value().beats[0].effects[2].value == 13);
    }
    MOTOR_CHECK(!RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b",
        "trigger": {"type": "auto"}, "effects": [{"type": "grantLevel", "value": 21}]}]})").isOk());
    // requires.nivelMinimo / tierMinimo: se leen, se escriben igual y se validan.
    const auto pide = RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b",
        "trigger": {"type": "auto"}, "requires": {"nivelMinimo": 12, "tierMinimo": 2}, "effects": []}]})");
    MOTOR_CHECK(pide.isOk() && pide.value().beats[0].requirements.nivelMinimo == 12 &&
                pide.value().beats[0].requirements.tierMinimo == 2);
    if (pide.isOk()) {
        const std::string texto = RPG::serializeAdventure(pide.value());
        const auto otra = RPG::AdventureScript::loadFromString(texto);
        MOTOR_CHECK(otra.isOk() && RPG::serializeAdventure(otra.value()) == texto &&
                    otra.value().beats[0].requirements.nivelMinimo == 12);
    }
    MOTOR_CHECK(!RPG::AdventureScript::loadFromString(R"({"id": "x", "beats": [{"id": "b",
        "trigger": {"type": "auto"}, "requires": {"nivelMinimo": 101}, "effects": []}]})").isOk());
}

// Oriol (22/10): andar con el raton fuera del combate. Un muro en x = 2 con un hueco abajo: el camino lo
// rodea; a un PNJ (objeto que tapa) se llega a la casilla de al lado; en combate, nada.
MOTOR_TEST_CASE(el_camino_del_raton_rodea_los_muros_y_se_para_al_lado_del_pnj) {
    struct Patio : RuntimeTileMap {
        Patio() {
            RuntimeTileLayer capa;
            for (int y = 0; y < 5; ++y)
                for (int x = 0; x < 5; ++x) capa.tiles.emplace_back(1, x == 2 && y < 4);
            replaceCompiledData(5, 5, 64, 32, {capa});
        }
    } patio;
    ObjectCatalog catalog;
    MOTOR_CHECK(catalog.loadFromString(R"({"objects": [{"id": "tendero", "name": "Tendero", "category": "prop", "blocksMovement": true}]})")
                    .isOk());
    SkillCatalog skills;
    TestCombatant body{60};
    SkillSet playerSkills{10};
    std::vector<std::string> inventory;
    GameSession session(&patio, &catalog, &skills, makeLevelConObjeto("Patio", "tendero", GridCoord{4, 2}), &body,
                        &playerSkills, &inventory);
    const auto ruta = session.rutaAndando(GridCoord{4, 0});
    MOTOR_CHECK(!ruta.empty() && ruta.back().x == 4 && ruta.back().y == 0);
    bool porElHueco = false;
    GridCoord antes{0, 0};
    for (const GridCoord& c : ruta) {
        MOTOR_CHECK(std::abs(c.x - antes.x) + std::abs(c.y - antes.y) == 1);
        MOTOR_CHECK(!(c.x == 2 && c.y < 4));
        porElHueco |= c.x == 2 && c.y == 4;
        antes = c;
    }
    MOTOR_CHECK(porElHueco);
    const auto alTendero = session.rutaAndando(GridCoord{4, 2});
    MOTOR_CHECK(!alTendero.empty());
    const GridCoord fin = alTendero.back();
    MOTOR_CHECK(std::abs(fin.x - 4) + std::abs(fin.y - 2) == 1);
    MOTOR_CHECK(session.rutaAndando(GridCoord{0, 0}).empty());  // ya esta ahi
    MOTOR_CHECK(session.rutaAndando(GridCoord{9, 9}).empty());  // fuera del mapa
}

int main() { return motor_test::runAllTests(); }
