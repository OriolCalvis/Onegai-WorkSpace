// El combate sobre el tablero (AV13/AV14): el rival que se acerca, a quien pega, la iniciativa, el
// alcance de las cartas y los golpes apuntados.
//
// AV13-35 (14/10) — Este fichero era el de G-13 (el menu de habilidades antiguo: BattleMenuActions,
// TargetSelection, el combate sin mesa). Ese combate se borro; aqui quedan las pruebas del de ahora,
// y cada batalla lleva su mesa de cartas, como en el juego (sin ficha de clase: las basicas).

#include "TestHarness.h"

#include "Game/AlcanceCartas.h"
#include "Game/AreaDeCarta.h"
#include "Game/BarraCombate.h"
#include "Game/BattleState.h"
#include "Game/CartasEnCombate.h"
#include "Game/CatalogoCartas.h"
#include "Render/ICombatant.h"
#include "ScriptedRng.h"
#include "RPG/CharacterSheet.h"
#include "RPG/Definitions/RpgCoreDefinitions.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

class TestCombatant : public ICombatant {
public:
    TestCombatant(int maxHealth, int health, RPG::Stat castingStat,
                  int castingValue, int tier = 1,
                  std::string combatantId = "tester")
        : m_currentHealth(health),
          m_maxHealth(maxHealth),
          m_cast(castingStat),
          m_castValue(castingValue),
          m_tier(tier),
          m_id(std::move(combatantId)) {
        m_sheet.id = m_id;
        m_sheet.displayName = m_id;
        m_sheet.tier = tier;
        for (int i = 0; i < 4; ++i) {
            m_sheet.baseStats[i] = 0;
            m_sheet.racialBonuses[i] = 0;
            m_sheet.classBonuses[i] = 0;
            m_sheet.equipBonuses[i] = 0;
            m_sheet.featBonuses[i] = 0;
            m_sheet.conditionModifiers[i] = 0;
        }
        m_sheet.baseStats[static_cast<int>(m_cast)] = m_castValue;
        m_sheet.cachedHealthCap = m_maxHealth;
    }

    void takeDamage(int amount) override {
        m_currentHealth = std::max(0, m_currentHealth - amount);
    }
    void heal(int amount) override {
        m_currentHealth = std::min(m_maxHealth, m_currentHealth + amount);
    }
    int health() const override { return m_currentHealth; }
    int maxHealth() const override { return m_maxHealth; }
    bool isAlive() const override { return m_currentHealth > 0; }
    int stat(RPG::Stat s) const override { return m_sheet.stat(s); }
    RPG::DefenseBlock defenses() const override { return m_sheet.defenseValues(); }
    int tier() const override { return m_tier; }
    std::string combatant_id() const override { return m_id; }
    RPG::CharacterSheet* character_sheet() override { return &m_sheet; }
    const RPG::CharacterSheet* character_sheet() const override { return &m_sheet; }

private:
    int m_currentHealth;
    int m_maxHealth;
    RPG::Stat m_cast;
    int m_castValue;
    int m_tier;
    std::string m_id;
    RPG::CharacterSheet m_sheet;
};

// La mesa del heroe, como la monta GameSession: sin cartas de clase, las basicas.
void conMesa(BattleState& b, ICombatant& heroe) {
    static const Game::CatalogoCartas kSinCartas{{}, {}, {}, {}};
    b.setMesa(Game::CartasEnCombate::paraHeroe(kSinCartas, heroe));
}

}  // namespace

// AV13-12 — Con tablero, el turno del rival va en dos mitades: acercaEnemigos()
// (andan) y resolveEnemyTurn() (pegan, solo a quien tienen a tiro).
namespace {
Game::TableroCombate pasillo(std::initializer_list<std::pair<const char*, GridCoord>> quienes) {
    auto t = Game::TableroCombate::alrededor({GridCoord{0, 0}, GridCoord{19, 2}}, 20, 3,
                                             [](int, int) { return false; }, 20);
    for (const auto& [n, c] : quienes) t.pon(n, c);
    return t;
}
bool registroDice(const BattleState& b, const std::string& trozo) {
    for (const std::string& l : b.log())
        if (l.find(trozo) != std::string::npos) return true;
    return false;
}
}  // namespace

MOTOR_TEST_CASE(av13_el_rival_lejos_se_acerca_y_no_pega_hasta_llegar) {
    TestCombatant heroe(40, 40, RPG::Stat::DES, 0, 1, "h");
    TestCombatant lobo(40, 40, RPG::Stat::DES, 3, 1, "l");
    BattleState b({BattleParticipant{"Heroe", &heroe, nullptr}}, {BattleParticipant{"Lobo", &lobo, nullptr}}, nullptr);
    conMesa(b, heroe);
    b.ponTablero(pasillo({{"Heroe", GridCoord{0, 1}}, {"Lobo", GridCoord{9, 1}}}));
    // Ronda 1: anda sus 4 casillas (AV16-12), no llega y no pega.
    MOTOR_CHECK(b.acercaEnemigos());
    MOTOR_CHECK(registroDice(b, "Lobo se acerca, pero no llega."));
    MOTOR_CHECK(b.tablero()->dondeEsta("Lobo")->x == 5);
    b.resolveEnemyTurn();
    MOTOR_CHECK(heroe.health() == 40);
    MOTOR_CHECK(b.log().back() == "Lobo se acerca, pero no llega.");
    // Ronda 2: con los pasos repuestos llega al lado; resolveEnemyTurn ya no lo mueve.
    b.tickRound();
    MOTOR_CHECK(b.acercaEnemigos());
    MOTOR_CHECK(b.tablero()->dondeEsta("Lobo")->x == 1);
    MOTOR_CHECK(!b.acercaEnemigos());  // ya esta a tiro: no anda mas
    b.resolveEnemyTurn();
    MOTOR_CHECK(b.log().back() != "Lobo se acerca.");  // ha actuado
}

MOTOR_TEST_CASE(av13_el_rival_pega_a_quien_tiene_al_lado_no_al_heroe_lejano) {
    TestCombatant heroe(40, 40, RPG::Stat::DES, 0, 1, "h");
    TestCombatant rosa(40, 40, RPG::Stat::DES, 0, 1, "r");
    TestCombatant lobo(40, 40, RPG::Stat::DES, 3, 1, "l");
    BattleState b({BattleParticipant{"Heroe", &heroe, nullptr}, BattleParticipant{"Rosa", &rosa, nullptr}},
                  {BattleParticipant{"Lobo", &lobo, nullptr}}, nullptr);
    conMesa(b, heroe);
    b.ponTablero(pasillo({{"Heroe", GridCoord{0, 1}}, {"Rosa", GridCoord{12, 1}}, {"Lobo", GridCoord{13, 1}}}));
    MOTOR_CHECK(!b.acercaEnemigos());  // Rosa ya esta al lado
    for (int i = 0; i < 5; ++i) b.resolveEnemyTurn();
    MOTOR_CHECK(heroe.health() == 40);
    MOTOR_CHECK(registroDice(b, "Rosa"));
    MOTOR_CHECK(!registroDice(b, "Heroe"));
}

// AV13-11 — Con iniciativa, cada uno en su turno: el rival que saca mas va
// primero, y la ronda se cierra al dar la vuelta.
MOTOR_TEST_CASE(av13_la_iniciativa_da_el_turno_uno_a_uno_y_cierra_la_ronda) {
    TestCombatant heroe(40, 40, RPG::Stat::DES, 1, 1, "h");   // DES 1
    TestCombatant lobo(40, 40, RPG::Stat::DES, 2, 1, "l");    // DES 2
    TestCombatant cuervo(40, 0, RPG::Stat::DES, 0, 1, "c");   // entra caido
    BattleState b({BattleParticipant{"Heroe", &heroe, nullptr}},
                  {BattleParticipant{"Lobo", &lobo, nullptr}, BattleParticipant{"Cuervo", &cuervo, nullptr}}, nullptr);
    MOTOR_CHECK(b.enemigoEnTurno() == -1);  // sin tirarla: el turno de siempre
    ScriptedRng dados({3, 3, 6});  // heroe 3+1 = 4; lobo 3+2 = 5; cuervo 6 (pero caido)
    b.setRandomEngine(dados);
    b.tiraIniciativa();
    MOTOR_CHECK(b.log().back() == "Iniciativa (1d6 + DES): Cuervo 6, Lobo 5, Heroe 4.");
    MOTOR_CHECK(b.enemigoEnTurno() == 0);  // el cuervo esta caido: empieza el lobo
    b.pasaAlSiguiente();
    MOTOR_CHECK(b.enemigoEnTurno() == -1);  // el heroe
    MOTOR_CHECK(b.orden().quien() != nullptr && !b.orden().quien()->enemigo);
    b.pasaAlSiguiente();                    // da la vuelta (salta al cuervo): ronda nueva
    MOTOR_CHECK(b.enemigoEnTurno() == 0);
}

// AV13-13 — El alcance de las cartas: cuantas casillas, a quien llega cada una,
// la que no llega a nadie se bloquea y el objetivo por defecto es el primero a tiro.
MOTOR_TEST_CASE(av13_el_alcance_de_las_cartas_elige_y_bloquea) {
    using RPG::SkillRange;
    using RPG::SkillTarget;
    MOTOR_CHECK(Game::casillasDeAlcance(SkillRange::MELEE, SkillTarget::SINGLE_ENEMY) == 1);
    MOTOR_CHECK(Game::casillasDeAlcance(SkillRange::SHORT, SkillTarget::SINGLE_ENEMY) == 4);
    MOTOR_CHECK(Game::casillasDeAlcance(SkillRange::LONG, SkillTarget::SINGLE_ENEMY) == 12);
    MOTOR_CHECK(Game::casillasDeAlcance(SkillRange::UNKNOWN, SkillTarget::SELF) == 0);
    MOTOR_CHECK(Game::casillasDeAlcance(SkillRange::UNKNOWN, SkillTarget::SINGLE_ENEMY) == 4);

    TestCombatant heroe(40, 40, RPG::Stat::DES, 0, 1, "h");
    TestCombatant lobo(40, 40, RPG::Stat::DES, 0, 1, "l");
    TestCombatant cuervo(40, 40, RPG::Stat::DES, 0, 1, "c");
    BattleState b({BattleParticipant{"Heroe", &heroe, nullptr}},
                  {BattleParticipant{"Lobo", &lobo, nullptr}, BattleParticipant{"Cuervo", &cuervo, nullptr}}, nullptr);
    // El lobo a 6 casillas; el cuervo a 3.
    b.ponTablero(pasillo({{"Heroe", GridCoord{0, 1}}, {"Lobo", GridCoord{6, 1}}, {"Cuervo", GridCoord{3, 1}}}));
    Game::VistaBarra v = Game::vistaDeBatalla(b);
    const Game::CartaBarra& atacar = v.basicas[0];
    MOTOR_CHECK(atacar.alcance == 1);
    MOTOR_CHECK(atacar.estado == Game::CartaBarra::Estado::Bloqueada);
    MOTOR_CHECK(atacar.motivo == "nadie al lado: acércate");
    // Una carta corta (4): llega al cuervo, no al lobo; apunta al cuervo aunque se eligiera al lobo.
    Game::CartaBarra flecha;
    flecha.id = "flecha";
    flecha.rango = SkillRange::SHORT;
    flecha.filas = {3};
    v.mano.push_back(flecha);
    Game::ponAlcances(v, b);
    const Game::CartaBarra& f = v.mano.back();
    MOTOR_CHECK(f.alcance == 4);
    MOTOR_CHECK(f.enemigosATiro.size() == 2 && !f.enemigosATiro[0] && f.enemigosATiro[1]);
    MOTOR_CHECK(f.estado == Game::CartaBarra::Estado::Lista);
    Game::BarraCombate barra;
    barra.mira(v, -1.0f, &b);
    MOTOR_CHECK(barra.objetivoDe(v, f) == 1);
    using Zona = Game::BarraCombate::Zona;
    barra.senala(v, Zona::Carta, static_cast<int>(v.basicas.size() + v.mano.size() - 1));  // la flecha
    barra.senala(v, Zona::Objetivo, Game::BarraCombate::kObjetivoEnemigo + 0);  // el lobo: fuera de alcance, no se deja
    MOTOR_CHECK(barra.objetivoDe(v, f) == 1);
    // En el suelo: las casillas a 4 o menos, sin la del heroe.
    const auto tiro = Game::casillasATiro(*b.tablero(), "Heroe", 4);
    MOTOR_CHECK(!tiro.empty());
    for (const GridCoord& c : tiro) MOTOR_CHECK(Game::TableroCombate::distancia(GridCoord{0, 1}, c) <= 4 && !(c.x == 0 && c.y == 1));
}

// AV14-01 — Cada golpe que cambia la vida queda apuntado una vez, con quien,
// a quien y cuanto: la suma de los golpes es lo que ha perdido.
MOTOR_TEST_CASE(av14_los_golpes_de_la_batalla_suman_lo_perdido) {
    TestCombatant heroe(400, 400, RPG::Stat::DES, 0, 1, "h");
    TestCombatant lobo(40, 40, RPG::Stat::DES, 4, 1, "l");
    BattleState b({BattleParticipant{"Heroe", &heroe, nullptr}}, {BattleParticipant{"Lobo", &lobo, nullptr}}, nullptr);
    conMesa(b, heroe);
    for (int i = 0; i < 20; ++i) b.resolveEnemyTurn();
    int suma = 0, numero = 0;
    for (const Game::GolpeCombate& g : b.golpes()) {
        MOTOR_CHECK(g.numero == numero + 1);
        numero = g.numero;
        if (!g.aAliado) continue;  // con la mesa, la pifia del lobo te da un ataque de oportunidad (a el)
        MOTOR_CHECK(g.a == "Heroe" && g.de == "Lobo" && g.i == 0);
        MOTOR_CHECK(g.cambio <= 0);  // y tambien se apunta lo que no entra (fallo, pifia)
        suma += g.cambio;
    }
    MOTOR_CHECK(!b.golpes().empty());
    MOTOR_CHECK(suma == heroe.health() - 400);
    MOTOR_CHECK(std::string(Game::nombreDe(Game::GolpeCombate::Grado::Critico)) == "Crítico");
}

// AV13-14 — La forma del area sale de la carta: una zona es un circulo en el objetivo (su radio, del
// texto: 5 pies por casilla); «todos los enemigos», radio 2; «linea», desde quien la lanza.
MOTOR_TEST_CASE(av13_el_area_de_una_carta) {
    RPG::SkillDefinition zona;
    zona.target = RPG::SkillTarget::ZONE_RADIUS;
    zona.description = "Lanzas una bola de fuego en un radio de 8 pies.";
    const auto a = Game::areaDe(zona);
    MOTOR_CHECK(a && a->plantilla.forma == Game::PlantillaArea::Forma::Circulo && a->plantilla.tamano == 2 && !a->enTi);
    RPG::SkillDefinition todos;
    todos.target = RPG::SkillTarget::ALL_ENEMIES;
    const auto b = Game::areaDe(todos);
    MOTOR_CHECK(b && b->plantilla.tamano == 2);
    RPG::SkillDefinition linea;
    linea.target = RPG::SkillTarget::ZONE_RADIUS;
    linea.description = "Disparas en línea recta a los enemigos en fila.";
    const auto l = Game::areaDe(linea);
    MOTOR_CHECK(l && l->plantilla.forma == Game::PlantillaArea::Forma::Linea && l->enTi);
    RPG::SkillDefinition uno;
    uno.target = RPG::SkillTarget::SINGLE_ENEMY;
    MOTOR_CHECK(!Game::areaDe(uno));
    // El circulo de radio 1 en (5,5): la cruz de 5 casillas.
    const auto zonaCasillas = Game::casillasDelArea(Game::AreaDeCarta{Game::PlantillaArea{Game::PlantillaArea::Forma::Circulo, 1}, false},
                                                    GridCoord{0, 0}, GridCoord{5, 5});
    MOTOR_CHECK(zonaCasillas.size() == 5);
    // Un muro en (6,5) tapa lo que hay detras desde el centro: (7,5) no se pilla; (3,5), al otro lado, si.
    const Game::AreaDeCarta dos{Game::PlantillaArea{Game::PlantillaArea::Forma::Circulo, 2}, false};
    const auto conMuro = Game::casillasDelArea(dos, GridCoord{0, 0}, GridCoord{5, 5}, [](int x, int y) { return x == 6 && y == 5; });
    const auto hay = [&](int x, int y) {
        return std::any_of(conMuro.begin(), conMuro.end(), [&](GridCoord c) { return c.x == x && c.y == y; });
    };
    MOTOR_CHECK(!hay(7, 5) && hay(3, 5));
}

int main() { return motor_test::runAllTests(); }
