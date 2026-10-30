// H2-06 — El Apostata, el Senor de las Armas, la Furia de los Creadores y las
// cartas de Creador en combate. Las cuentas del dado fijo estan explicadas en
// tests/Game/CartasEnCombateTests.cpp; la mesa, en CartasPrueba.h.
#include "CartasPrueba.h"
#include "TestHarness.h"

MOTOR_TEST_CASE(apostata_escudo_de_sombras_crece_con_los_golpes_y_estalla) {
    Mesa m("el_apostata");
    MOTOR_CHECK(m.fila("Destructor del Escudo de Sombras (el escudo esta vacio)") >= 0);
    MOTOR_CHECK(m.jugar("Escudo de Sombras"));
    m.batalla->tickRound();
    m.atacar();
    m.batalla->tickRound();
    m.atacar();
    MOTOR_CHECK(m.cartas->situacion(true, 0)->escudo == 4);  // 2 por golpe
    MOTOR_CHECK(m.lobo.health() == 50);  // dos golpes de 1d6
    m.batalla->tickRound();
    // Magia (resistencia fisica): el lobo se salva con 1 dado y falla, los 4
    MOTOR_CHECK(m.jugar("Destructor del Escudo de Sombras"));
    MOTOR_CHECK(m.lobo.health() == 46);
    MOTOR_CHECK(m.cartas->situacion(true, 0)->escudo == 0);
}

MOTOR_TEST_CASE(apostata_el_escudo_se_gasta_antes_que_la_vida) {
    Mesa m("el_apostata");
    MOTOR_CHECK(m.jugar("Escudo de Sombras"));
    m.batalla->tickRound();
    m.atacar();  // escudo 2
    m.ronda();   // el lobo hace 5: el escudo para 2
    MOTOR_CHECK(m.heroe.health() == 27);
    MOTOR_CHECK(m.cartas->situacion(true, 0)->escudo == 0);
}

MOTOR_TEST_CASE(apostata_absorcion_de_alma_drena_tres_rondas_y_canaliza_sin_cerrar_turno) {
    Mesa m("el_apostata");
    m.heroe.ponerVida(20);
    MOTOR_CHECK(m.jugar("Absorción de Alma (canaliza)"));
    MOTOR_CHECK(m.cartas->turnoAbierto());
    m.atacar();  // lobo 55
    m.ronda();   // el lobo hace 5 (15) y al final de ronda absorbe 5: heroe 20, lobo 50
    MOTOR_CHECK(m.heroe.health() == 20);
    MOTOR_CHECK(m.lobo.health() == 50);
    m.batalla->tickRound();
    m.batalla->tickRound();
    MOTOR_CHECK(m.lobo.health() == 40);
    MOTOR_CHECK(m.situLobo().estados.empty());
    m.batalla->tickRound();
    MOTOR_CHECK(m.lobo.health() == 40);
}

MOTOR_TEST_CASE(apostata_invocacion_oscura_trae_un_aliado_que_ataca_diez_turnos) {
    Mesa m("el_apostata", 1, 1);  // lobo con DES 1: se defiende con un dado
    MOTOR_CHECK(m.fila("Invocación Oscura: Cuervo Descompuesto") >= 0);
    MOTOR_CHECK(m.jugar("Invocación Oscura: Espectro Menor"));
    MOTOR_CHECK(m.batalla->allies().size() == 2);
    // Al cerrar el turno el espectro (DES 2: 3 dados) ataca: 1,5 contra 0,5, 1d6.
    MOTOR_CHECK(m.lobo.health() == 55);
    for (int i = 0; i < 10; ++i) m.batalla->tickRound();
    MOTOR_CHECK(!m.batalla->allies()[1].combatant->isAlive());
}

MOTOR_TEST_CASE(apostata_desgarro_recupera_una_carta_gastada) {
    Mesa m("el_apostata");
    MOTOR_CHECK(m.jugar("Maldición de la Debilidad"));
    m.batalla->tickRound();
    MOTOR_CHECK(m.jugar("Desgarro: recuperar Maldición de la Debilidad (+1)"));
    MOTOR_CHECK(!m.carta("maldicion_de_la_debilidad").gastada);
    MOTOR_CHECK(m.cartas->pasiva().desgarro == 1);
    MOTOR_CHECK(m.cartas->turnoAbierto());
}

MOTOR_TEST_CASE(apostata_marca_profana_da_ventaja_y_1d6_a_quien_marco) {
    Mesa sin("el_apostata");
    sin.dado.cara = 4;
    sin.atacar();  // 0 contra 0: roza (D0014), 1
    MOTOR_CHECK(sin.lobo.health() == 59);

    Mesa m("el_apostata");
    MOTOR_CHECK(m.jugar("Marca Profana"));
    m.batalla->tickRound();
    m.dado.cara = 4;
    m.atacar();  // ventaja: 3 contra 0, 3d6 (12) + 1d6 (4)
    MOTOR_CHECK(m.lobo.health() == 44);
}

MOTOR_TEST_CASE(senor_escudo_divino_bloquea_y_ciega_al_atacante) {
    Mesa m("senor_de_las_armas");
    MOTOR_CHECK(m.jugar("Escudo Divino (reaccion)"));
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    MOTOR_CHECK(!m.situLobo().estados.empty() && m.situLobo().estados[0].id == "cegado");
    m.atacar();
    m.ronda();  // cegado: desventaja en sus ataques, 0 contra 2,5
    MOTOR_CHECK(m.heroe.health() == 30);
}

MOTOR_TEST_CASE(senor_rafagas_del_arsenal_son_tres_golpes) {
    Mesa m("senor_de_las_armas");
    MOTOR_CHECK(m.jugar("Ráfagas del Arsenal"));
    MOTOR_CHECK(m.lobo.health() == 45);  // tres de 1d6
}

MOTOR_TEST_CASE(senor_lo_pendiente_sale_bloqueado_en_el_menu) {
    Mesa m("senor_de_las_armas");
    const int i = m.fila("[pendiente] Sexta habilidad (pendiente)");
    MOTOR_CHECK(i >= 0);
    BattleAction a;
    MOTOR_CHECK(!m.cartas->accionDeFila(static_cast<std::size_t>(i), a));
    // La fila lleva el motivo corto; el entero sale en el detalle.
    MOTOR_CHECK(m.cartas->detalle(static_cast<std::size_t>(i)).find("[pendiente] faltan 2 habilidades") == 0);
}

MOTOR_TEST_CASE(furia_castigo_divino_quema_tres_rondas) {
    Mesa m("furia_de_los_creadores");
    MOTOR_CHECK(m.jugar("Castigo Divino"));
    for (int i = 0; i < 4; ++i) m.batalla->tickRound();
    MOTOR_CHECK(m.lobo.health() == 45);  // 3 x 5
}

MOTOR_TEST_CASE(furia_ataque_del_rayo_paraliza_y_quema) {
    Mesa m("furia_de_los_creadores");
    MOTOR_CHECK(m.jugar("Ataque del Rayo"));
    bool paralizado = false;
    for (const Game::EstadoCarta& e : m.situLobo().estados) paralizado |= e.id == "paralizado" && e.rondas == 2;
    MOTOR_CHECK(paralizado);
    m.batalla->tickRound();
    MOTOR_CHECK(m.lobo.health() == 55);
}

MOTOR_TEST_CASE(furia_proteccion_del_creador_contrarresta_la_ronda) {
    Mesa m("furia_de_los_creadores");
    MOTOR_CHECK(m.jugar("Protección del Creador (reaccion)"));
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
}

MOTOR_TEST_CASE(furia_fe_cura_el_dano_que_causa) {
    Mesa m("furia_de_los_creadores");
    m.heroe.ponerVida(15);
    MOTOR_CHECK(m.jugar("Furia: Fe"));
    MOTOR_CHECK(m.lobo.health() == 55);
    MOTOR_CHECK(m.heroe.health() == 20);
}

MOTOR_TEST_CASE(furia_fe_con_la_vida_llena_no_apunta_una_cura_que_no_hubo) {
    Mesa m("furia_de_los_creadores");
    MOTOR_CHECK(m.jugar("Furia: Fe"));
    MOTOR_CHECK(m.heroe.health() == 30);
    bool llena = false;
    for (const std::string& l : m.batalla->log()) {
        MOTOR_CHECK(l.find("se cura") == std::string::npos);
        llena |= l.find("ya tiene la vida llena") != std::string::npos;
    }
    MOTOR_CHECK(llena);
}

MOTOR_TEST_CASE(furia_puno_volcanico_suma_2d6_a_los_golpes) {
    Mesa m("furia_de_los_creadores");
    MOTOR_CHECK(m.jugar("Puño Volcánico"));
    m.batalla->tickRound();
    m.atacar();  // 1d6 + 2d6: 5 + 5 + 5
    MOTOR_CHECK(m.lobo.health() == 45);
}

MOTOR_TEST_CASE(las_de_creador_esperan_al_tier_2) {
    Mesa t1("bardo_de_las_cuatro_estaciones");
    MOTOR_CHECK(t1.fila("Elevación del Ser (tier 2)") >= 0);
    MOTOR_CHECK(!t1.jugar("Elevación del Ser"));

    Mesa t2("bardo_de_las_cuatro_estaciones", 2);
    MOTOR_CHECK(t2.jugar("Elevación del Ser"));
    MOTOR_CHECK(t2.cartas->turnoAbierto());  // turno adicional
    t2.atacar();
    MOTOR_CHECK(!t2.cartas->turnoAbierto());
}

MOTOR_TEST_CASE(ultimo_aliento_devuelve_al_heroe_con_1_pv) {
    Mesa m("bardo_de_las_cuatro_estaciones", 2);
    m.heroe.ponerVida(3);
    MOTOR_CHECK(m.jugar("Último Aliento (reaccion)"));
    m.atacar();
    m.ronda();  // el lobo hace 5: cae y vuelve con 1
    MOTOR_CHECK(m.heroe.health() == 1);
    MOTOR_CHECK(m.batalla->outcome() == BattleOutcome::InProgress);
    MOTOR_CHECK(m.carta("ultimo_aliento").gastada);
}

// AV13-30 (Oriol, 14/10, D0015 §4 y §6) — El critico atraviesa el bloqueo salvo bloqueo critico, y
// la pifia del atacante da un ataque de oportunidad al que se defendia.
namespace {
int lineasCon(const BattleState& b, const std::string& trozo) {
    int n = 0;
    for (const std::string& l : b.log()) n += l.find(trozo) != std::string::npos;
    return n;
}
}  // namespace

MOTOR_TEST_CASE(av13_el_critico_atraviesa_el_bloqueo_si_el_bloqueo_no_es_critico) {
    Mesa m("furia_de_los_creadores");
    MOTOR_CHECK(m.jugar("Protección del Creador (reaccion)"));
    m.heroe.ponerStat(RPG::Stat::CON, 0);  // un solo dado para bloquear: con uno no hay critico
    m.heroe.ponerStat(RPG::Stat::DES, 0);
    m.dado.cara = 6;                        // el lobo saca todo 6: critico
    m.atacar();
    m.ronda();
    MOTOR_CHECK(lineasCon(*m.batalla, "atraviesa el bloqueo") == 1);
    MOTOR_CHECK(m.heroe.health() < 30);
}

MOTOR_TEST_CASE(av13_el_bloqueo_critico_para_hasta_el_critico) {
    Mesa m("furia_de_los_creadores");
    MOTOR_CHECK(m.jugar("Protección del Creador (reaccion)"));
    m.dado.cara = 6;  // los dos sacan todo 6: critico contra bloqueo critico
    m.atacar();
    m.ronda();
    MOTOR_CHECK(lineasCon(*m.batalla, "bloqueo critico") == 1);
    MOTOR_CHECK(m.heroe.health() == 30);
}

MOTOR_TEST_CASE(av13_la_pifia_del_atacante_da_un_ataque_de_oportunidad_y_no_encadena) {
    Mesa m("furia_de_los_creadores");
    m.dado.cara = 1;  // todo 1: pifia de todos
    m.atacar();       // el heroe pifia: el lobo aprovecha (y su golpe, aunque pifie, no da otro)
    MOTOR_CHECK(lineasCon(*m.batalla, "Heroe pifia: Lobo aprovecha el hueco") == 1);
    MOTOR_CHECK(lineasCon(*m.batalla, "aprovecha el hueco") == 1);
    m.ronda();        // el lobo pifia: el heroe aprovecha
    MOTOR_CHECK(lineasCon(*m.batalla, "Lobo pifia: Heroe aprovecha el hueco") == 1);
}

