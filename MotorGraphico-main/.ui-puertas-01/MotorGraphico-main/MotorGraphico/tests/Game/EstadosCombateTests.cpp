// H2-03 — Los estados de cada combatiente, como se ensenan
// (Game/EstadosCombate.h), sobre la mesa real (CartasPrueba.h: catalogos de
// assets/catalogs y dado fijo en 5).
//
// El criterio de la Fase H: «Un enemigo envenenado muestra el icono y el
// contador, y ambos bajan por ronda». Los estados vienen de dos sitios: las
// condiciones de la ficha (las descuenta BattleState::tickRound) y lo que
// dejan las cartas en la mesa (las descuenta CartasEnCombate::finDeRonda).
#include "CartasPrueba.h"
#include "Game/BarraCombate.h"
#include "Game/EstadosCombate.h"
#include "RPG/ConditionEngine.h"
#include "TestHarness.h"

#include <string>
#include <vector>

using namespace cartas_prueba;
using Game::BarraCombate;
using Game::EstadoVisible;

namespace {

std::vector<EstadoVisible> delLobo(const Mesa& m) { return Game::estadosDe(*m.batalla, false, 0); }

}  // namespace

MOTOR_TEST_CASE(estados_un_enemigo_envenenado_ensena_icono_y_contador_y_bajan_por_ronda) {
    // El veneno en la ficha del lobo. El del canon dura «hasta curarse»;
    // aqui, uno de tres rondas para ver la cuenta.
    RPG::ConditionDefinition veneno;
    veneno.id = "envenenado";
    veneno.name = "Envenenado";
    veneno.description = "Trabaja despacio. Cobra completo.";
    veneno.defaultRounds = 3;
    veneno.damagePerRound = 1;
    veneno.durationMode = RPG::ConditionDefinition::Duration::ROUNDS;
    RPG::Catalogs::ConditionCatalog cat;
    cat.insert(veneno);
    Mesa m("bardo_de_las_cuatro_estaciones");
    RPG::CharacterSheet fichaLobo;
    m.lobo.ficha = &fichaLobo;
    m.batalla->setConditionCatalog(&cat);
    MOTOR_CHECK(RPG::ConditionEngine::apply(fichaLobo, veneno, cat, "heroe").isOk());

    std::vector<EstadoVisible> e = delLobo(m);
    MOTOR_CHECK(e.size() == 1);
    MOTOR_CHECK(e[0].icono == "envenenado" && e[0].nombre == "Envenenado" && e[0].rondas == 3);
    MOTOR_CHECK(e[0].signo == EstadoVisible::Signo::Contra && !e[0].hastaCurarse);
    MOTOR_CHECK(e[0].descripcion == veneno.description && Game::duracionDe(e[0]) == "3 rondas");
    for (int quedan = 2; quedan >= 1; --quedan) {
        m.atacar();
        m.ronda();
        e = delLobo(m);
        MOTOR_CHECK(e.size() == 1 && e[0].icono == "envenenado" && e[0].rondas == quedan);
    }
    MOTOR_CHECK(Game::duracionDe(e[0]) == "1 ronda");
    m.atacar();
    m.ronda();
    MOTOR_CHECK(delLobo(m).empty());  // agotado: icono y contador se van
}

MOTOR_TEST_CASE(estados_lo_que_deja_una_carta_lleva_el_icono_de_lo_que_hace) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.jugar("Canción del Otoño"));  // desventaja dos rondas
    std::vector<EstadoVisible> e = delLobo(m);
    MOTOR_CHECK(e.size() == 1);
    MOTOR_CHECK(e[0].id == "cancion_del_otono" && e[0].icono == "perjuicio" && e[0].rondas == 2);
    MOTOR_CHECK(e[0].signo == EstadoVisible::Signo::Contra);
    MOTOR_CHECK(e[0].descripcion == m.carta("cancion_del_otono").carta->def->description);
    m.ronda();
    e = delLobo(m);
    MOTOR_CHECK(e.size() == 1 && e[0].rondas == 1);
    m.atacar();
    m.ronda();
    MOTOR_CHECK(delLobo(m).empty());
}

MOTOR_TEST_CASE(estados_una_condicion_de_la_mesa_lleva_su_icono_y_el_dano_sangra) {
    // Ataque del Rayo: paraliza dos rondas (una condicion del canon, sobre un
    // lobo sin ficha: va a la mesa) y deja 1d6 de dano por turno.
    Mesa m("furia_de_los_creadores");
    MOTOR_CHECK(m.jugar("Ataque del Rayo"));
    const std::vector<EstadoVisible> e = delLobo(m);
    bool paralizado = false, sangra = false;
    for (const EstadoVisible& v : e) {
        paralizado |= v.icono == "paralizado" && v.nombre == "Paralizado" && v.rondas == 2;
        sangra |= v.icono == "sangrado" && v.rondas == 2 && v.signo == EstadoVisible::Signo::Contra;
    }
    MOTOR_CHECK(paralizado && sangra);
}

MOTOR_TEST_CASE(estados_el_heroe_ensena_lo_que_le_favorece) {
    // Moral Inconmensurable (de tier 2): +1 a la CA de los aliados todo el
    // combate.
    Mesa m("senor_de_las_armas", 2);
    MOTOR_CHECK(m.jugar("Moral Inconmensurable"));
    const std::vector<EstadoVisible> e = Game::estadosDe(*m.batalla, true, 0);
    MOTOR_CHECK(e.size() == 1);
    MOTOR_CHECK(e[0].icono == "mejora" && e[0].signo == EstadoVisible::Signo::AFavor);
    MOTOR_CHECK(e[0].rondas == -1 && Game::duracionDe(e[0]) == "todo el combate");
    MOTOR_CHECK(Game::estadosDe(*m.batalla, true, 7).empty());  // quien no esta, nada
}

MOTOR_TEST_CASE(estados_la_vista_los_lleva_y_el_raton_los_senala) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(m.jugar("Canción del Otoño"));
    const Game::VistaBarra v = Game::vistaDeBatalla(*m.batalla);
    MOTOR_CHECK(v.estadosHeroe.empty() && v.estadosEnemigos.size() == 1 && v.estadosEnemigos[0].size() == 1);
    const int delPrimero = BarraCombate::kEstadoPorQuien * 1 + 0;
    MOTOR_CHECK(BarraCombate::estado(v, delPrimero) == &v.estadosEnemigos[0][0]);
    MOTOR_CHECK(BarraCombate::estado(v, 0) == nullptr);  // el heroe no lleva ninguno
    MOTOR_CHECK(BarraCombate::estado(v, 2 * BarraCombate::kEstadoPorQuien) == nullptr);
    MOTOR_CHECK(BarraCombate::estado(v, -1) == nullptr);
    BarraCombate b;
    b.mira(v, 0.0f, m.batalla.get());
    MOTOR_CHECK(b.estadoSenalado() == -1);
    b.senala(v, BarraCombate::Zona::Estado, delPrimero);
    MOTOR_CHECK(b.estadoSenalado() == delPrimero);
    b.senala(v, BarraCombate::Zona::Carta, 1);  // otra cosa de la barra: se apaga
    MOTOR_CHECK(b.estadoSenalado() == -1);
    MOTOR_CHECK(b.pulsa(v, BarraCombate::Zona::Estado, delPrimero).tipo == Game::Jugada::Tipo::Nada);
    MOTOR_CHECK(b.estadoSenalado() == delPrimero);  // el clic lo ensena y no juega nada
    b.senalaNada();
    MOTOR_CHECK(b.estadoSenalado() == -1);
}

// AV16-14 — Los estados nuevos (PO-37) se leen del lenguaje de las cartas.
MOTOR_TEST_CASE(estados_nuevos_quemado_congelado_aturdido_silenciado_se_leen) {
    using G = Game::Gancho;
    const G q = Game::leerGancho("estado:3:quemado:1d4");
    MOTOR_CHECK(q.tipo == G::Tipo::Estado && q.rondas == 3 && q.rasgo.size() == 1);
    MOTOR_CHECK(q.rasgo[0].tipo == G::Tipo::DanoPorTurno && q.rasgo[0].texto == "quemado");
    MOTOR_CHECK(q.rasgo[0].dados.cantidad == 1 && q.rasgo[0].dados.caras == 4);
    const G c = Game::leerGancho("estado:2:congelado");
    MOTOR_CHECK(c.rasgo.size() == 1 && c.rasgo[0].tipo == G::Tipo::VelocidadMitad && c.rasgo[0].texto == "congelado");
    const G a = Game::leerGancho("estado:1:aturdido");
    MOTOR_CHECK(a.rasgo.size() == 1 && a.rasgo[0].tipo == G::Tipo::Incapacitado && a.rasgo[0].texto == "aturdido");
    const G s = Game::leerGancho("estado:2:silenciado");
    MOTOR_CHECK(s.rasgo.size() == 1 && s.rasgo[0].tipo == G::Tipo::Silenciado);
    const G v0 = Game::leerGancho("estado:1:velocidad_0");
    MOTOR_CHECK(v0.rasgo.size() == 1 && v0.rasgo[0].tipo == G::Tipo::VelocidadCero);
}
