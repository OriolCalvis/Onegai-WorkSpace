// UI-BG3-01 — La barra de cartas del combate (Game/BarraCombate.h), sin
// ventana, sobre la mesa real de cada clase contra un lobo (CartasPrueba.h:
// catalogos de assets/catalogs y dado fijo).
//
// Lo que se prueba es lo que pidio Oriol (05/10) en las maquetas C03: las
// cartas se ven todo el combate y se juegan con 1-0 o con clic, la estacion
// del Bardo se elige en su emergente (o se deja para luego), F pasa el
// turno, el rival actua tras su rotulo y la reaccion que salta se avisa. Y
// que lo elegido sigue siendo una fila del menu de siempre, para que el
// combate se resuelva igual que antes.
#include "CartasPrueba.h"
#include "Game/BarraCombate.h"
#include "TestHarness.h"

#include <string>

using namespace cartas_prueba;
using Game::BarraCombate;
using Game::CartaBarra;
using Game::Jugada;
using Game::VistaBarra;

namespace {

VistaBarra vista(const Mesa& m) { return Game::vistaDeBatalla(*m.batalla); }

int posicion(const VistaBarra& v, const std::string& id) {
    for (int p = 0; p < BarraCombate::total(v); ++p)
        if (BarraCombate::carta(v, p)->id == id) return p;
    return -1;
}

// Juega en la batalla la fila que devolvio la barra (lo que hace Application).
void juega(Mesa& m, const Jugada& j) {
    MOTOR_CHECK(j.tipo == Jugada::Tipo::Fila && j.fila >= 3);
    BattleAction a;
    MOTOR_CHECK(m.cartas->accionDeFila(j.fila - 3, a));
    m.batalla->resolveAllyAction(0, a);
}

}  // namespace

MOTOR_TEST_CASE(barra_las_basicas_son_las_filas_de_siempre) {
    const VistaBarra v = Game::vistaBasica(2);
    MOTOR_CHECK(v.basicas.size() == 3 && v.mano.empty());
    for (std::size_t i = 0; i < 3; ++i) {
        MOTOR_CHECK(v.basicas[i].filas.size() == 1 && v.basicas[i].filas[0] == i);
        MOTOR_CHECK(v.basicas[i].tecla == static_cast<int>(i) + 1);
        MOTOR_CHECK(v.basicas[i].estado == CartaBarra::Estado::Lista);
    }
    MOTOR_CHECK(v.basicas[1].nombre == "Poción ×2");
    // Sin pociones, la carta sigue ahi pero no se juega (y dice por que).
    const VistaBarra sin = Game::vistaBasica(0);
    MOTOR_CHECK(sin.basicas[1].estado == CartaBarra::Estado::Bloqueada && !sin.basicas[1].motivo.empty());
    BarraCombate b;
    b.mira(sin, 0.0f);
    MOTOR_CHECK(b.tecla(sin, 1).tipo == Jugada::Tipo::Nada);  // la senala
    MOTOR_CHECK(b.foco() == 1);
    MOTOR_CHECK(b.tecla(sin, 1).tipo == Jugada::Tipo::Nada);  // y no la juega
}

MOTOR_TEST_CASE(barra_del_bardo_una_carta_por_carta_y_las_de_creador_aparte) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    const VistaBarra v = vista(m);
    // La pasiva pide estacion: cuatro opciones con su texto.
    MOTOR_CHECK(v.pasiva == "Las Cuatro Estaciones");
    MOTOR_CHECK(v.opcionesPasiva.size() == 4 && v.filasPasiva.size() == 4 && v.elegida.empty());
    MOTOR_CHECK(Game::textoDeOpcion(v.pasivaTexto, "invierno").rfind("Ganas +2 a la CA", 0) == 0);
    // Siete cartas de tier 1 en la mano, con las teclas 4..0; las tres de
    // creador (tier 2) no estan en la mano sino en su hueco.
    MOTOR_CHECK(v.mano.size() == 7);
    MOTOR_CHECK(v.creador.size() == 3);
    for (std::size_t i = 0; i < v.mano.size(); ++i) {
        MOTOR_CHECK(v.mano[i].tecla == static_cast<int>(i) + 4);
        MOTOR_CHECK(!v.mano[i].filas.empty() && v.mano[i].filas[0] >= 3);
    }
    // Sin canciones gastadas, Ola de Elementos se ve pero no se juega.
    const CartaBarra* ola = BarraCombate::carta(v, posicion(v, "ola_de_elementos"));
    MOTOR_CHECK(ola != nullptr && ola->estado == CartaBarra::Estado::Bloqueada);
    MOTOR_CHECK(ola->motivo == "no hay canciones gastadas");
}

MOTOR_TEST_CASE(barra_la_estacion_se_elige_en_su_emergente) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    BarraCombate b;
    VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    MOTOR_CHECK(b.emergente().abierto && b.emergente().dePasiva);
    MOTOR_CHECK(b.emergente().opciones.size() == 4 && b.emergente().opciones[0] == "Primavera");
    MOTOR_CHECK(b.emergente().textos[1] == "Tienes ventaja en ataque.");
    // Con el emergente abierto la barra no juega ni pasa; la tecla 4 elige
    // la cuarta opcion.
    MOTOR_CHECK(b.pulsa(v, BarraCombate::Zona::Carta, 0).tipo == Jugada::Tipo::Nada);
    MOTOR_CHECK(b.pasa().tipo == Jugada::Tipo::Nada);
    const Jugada j = b.tecla(v, 3);
    MOTOR_CHECK(j.fila == v.filasPasiva[3]);
    juega(m, j);
    MOTOR_CHECK(m.cartas->pasiva().elegida == "invierno");
    MOTOR_CHECK(m.cartas->turnoAbierto());  // accion libre: el turno sigue
    v = vista(m);
    b.mira(v, 0.1f, m.batalla.get());
    MOTOR_CHECK(!b.emergente().abierto && v.elegida == "invierno");
}

MOTOR_TEST_CASE(barra_ahora_no_deja_la_estacion_para_luego) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    BarraCombate b;
    const VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    MOTOR_CHECK(b.emergente().abierto);
    b.pulsa(v, BarraCombate::Zona::AhoraNo, 0);
    MOTOR_CHECK(!b.emergente().abierto);
    b.mira(v, 0.1f, m.batalla.get());
    MOTOR_CHECK(!b.emergente().abierto);  // no vuelve a salir solo
    // Otro combate empieza de cero (y vuelve a preguntar).
    Mesa otra("bardo_de_las_cuatro_estaciones");
    b.mira(vista(otra), 0.2f, otra.batalla.get());
    MOTOR_CHECK(b.emergente().abierto && b.foco() == 0 && b.registro());
}

MOTOR_TEST_CASE(barra_una_tecla_senala_y_la_segunda_juega) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    BarraCombate b;
    VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    b.cancela();
    // La 4 es Cancion del Otono (la primera de la mano).
    MOTOR_CHECK(b.tecla(v, 3).tipo == Jugada::Tipo::Nada);
    MOTOR_CHECK(b.foco() == 3);
    const Jugada j = b.tecla(v, 3);
    MOTOR_CHECK(j.tipo == Jugada::Tipo::Fila && j.fila == v.mano[0].filas[0]);
    juega(m, j);
    // Gastada: se ve gris y ni la tecla ni el clic la juegan.
    v = vista(m);
    b.mira(v, 0.1f, m.batalla.get());
    MOTOR_CHECK(v.mano[0].estado == CartaBarra::Estado::Gastada);
    MOTOR_CHECK(b.tecla(v, 3).tipo == Jugada::Tipo::Nada);
    MOTOR_CHECK(b.pulsa(v, BarraCombate::Zona::Carta, 3).tipo == Jugada::Tipo::Nada);
    // Las flechas mueven sin salirse de la barra; el raton senala.
    b.mueve(v, 99);
    MOTOR_CHECK(b.foco() == BarraCombate::total(v) - 1);
    b.senala(v, BarraCombate::Zona::Carta, 1);
    MOTOR_CHECK(b.foco() == 1);
    b.senala(v, BarraCombate::Zona::Creador, 0);
    MOTOR_CHECK(b.verCreador());
    // L esconde el registro.
    MOTOR_CHECK(b.registro());
    b.pulsa(v, BarraCombate::Zona::Registro, 0);
    MOTOR_CHECK(!b.registro());
}

MOTOR_TEST_CASE(barra_una_carta_con_opciones_las_ofrece_en_su_emergente) {
    Mesa m("el_apostata");
    BarraCombate b;
    const VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    MOTOR_CHECK(!b.emergente().abierto);  // el Apostata no elige nada al empezar
    const int p = posicion(v, "invocacion_oscura");
    const CartaBarra* c = BarraCombate::carta(v, p);
    MOTOR_CHECK(c != nullptr && c->filas.size() == 2 && c->opciones[0] == "Espectro Menor");
    MOTOR_CHECK(b.pulsa(v, BarraCombate::Zona::Carta, p).tipo == Jugada::Tipo::Nada);
    MOTOR_CHECK(b.emergente().abierto && !b.emergente().dePasiva);
    MOTOR_CHECK(b.emergente().opciones == c->opciones);
    b.mueve(v, 1);
    MOTOR_CHECK(b.emergente().foco == 1);
    const Jugada j = b.acepta(v);
    MOTOR_CHECK(j.tipo == Jugada::Tipo::Fila && j.fila == c->filas[1]);
    MOTOR_CHECK(!b.emergente().abierto);
    juega(m, j);
    MOTOR_CHECK(m.batalla->allies().size() == 2);  // el cuervo ya pelea
}

MOTOR_TEST_CASE(barra_pasar_cierra_el_turno_y_el_rival_espera_su_rotulo) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    BarraCombate b;
    VistaBarra v = vista(m);
    b.mira(v, 10.0f, m.batalla.get());
    b.cancela();
    MOTOR_CHECK(b.pasa().tipo == Jugada::Tipo::Pasar);
    MOTOR_CHECK(b.pulsa(v, BarraCombate::Zona::Pasar, 0).tipo == Jugada::Tipo::Pasar);
    // La mesa entiende «pasar»: lo dice el registro y el turno se cierra.
    m.batalla->resolveAllyAction(0, BattleAction{BattleActionType::UseSkill, Game::kAccionPasar, 0, ""});
    MOTOR_CHECK(!m.cartas->turnoAbierto());
    MOTOR_CHECK(m.batalla->log().back() == "Heroe pasa el turno.");
    MOTOR_CHECK(Game::rivalDeBatalla(*m.batalla) == "Lobo");
    // El rival espera su rotulo; mientras, la barra no juega.
    b.esperaRival(Game::rivalDeBatalla(*m.batalla));
    MOTOR_CHECK(b.rivalPendiente() && !b.rivalListo() && b.rival() == "Lobo");
    MOTOR_CHECK(b.acepta(v).tipo == Jugada::Tipo::Nada);
    MOTOR_CHECK(b.tecla(v, 0).tipo == Jugada::Tipo::Nada);
    MOTOR_CHECK(b.pasa().tipo == Jugada::Tipo::Nada);
    b.mira(v, 10.5f, m.batalla.get());
    MOTOR_CHECK(!b.rivalListo());
    b.mira(v, 10.0f + BarraCombate::kEsperaRival + 0.01f, m.batalla.get());
    MOTOR_CHECK(b.rivalListo());
    m.ronda();
    b.rivalResuelto();
    MOTOR_CHECK(!b.rivalPendiente());
    v = vista(m);
    MOTOR_CHECK(v.turno.ronda == 2 && !v.turno.principal);
    MOTOR_CHECK(b.acepta(v).tipo == Jugada::Tipo::Fila);  // Atacar, ya en tu turno
}

MOTOR_TEST_CASE(barra_la_reaccion_preparada_brilla_y_al_saltar_avisa) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    BarraCombate b;
    VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    b.cancela();
    const int p = posicion(v, "bendicion_de_las_estrellas");
    juega(m, b.pulsa(v, BarraCombate::Zona::Carta, p));
    v = vista(m);
    b.mira(v, 0.1f, m.batalla.get());
    MOTOR_CHECK(BarraCombate::carta(v, p)->estado == CartaBarra::Estado::Preparada);
    MOTOR_CHECK(v.turno.preparada >= 0 && b.aviso().empty());
    // Atacas, el lobo ataca y la reaccion le bloquea el golpe.
    m.atacar();
    m.ronda();
    MOTOR_CHECK(m.heroe.health() == 30);
    v = vista(m);
    b.mira(v, 1.0f, m.batalla.get());
    MOTOR_CHECK(BarraCombate::carta(v, p)->estado == CartaBarra::Estado::Gastada);
    MOTOR_CHECK(b.aviso() == "Bendición de las Estrellas" && b.avisoRestante() > 2.9f);
    b.mira(v, 4.5f, m.batalla.get());
    MOTOR_CHECK(b.avisoRestante() == 0.0f);
}

// H2-01 — La vista lleva la ultima tirada de la mesa, y la nueva brilla un
// momento: la tuya al jugar y la del rival al actuar.
MOTOR_TEST_CASE(barra_la_tirada_nueva_brilla_y_se_apaga) {
    Mesa m("bardo_de_las_cuatro_estaciones");
    BarraCombate b;
    VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    MOTOR_CHECK(v.tirada.numero == 0 && b.brilloTirada() == 0.0f);
    m.atacar();
    v = vista(m);
    MOTOR_CHECK(v.tirada.numero == 1 && v.tirada.quien == "Heroe" && v.tirada.delJugador);
    b.mira(v, 0.1f, m.batalla.get());
    MOTOR_CHECK(b.brilloTirada() == 1.0f);
    b.mira(v, 0.7f, m.batalla.get());
    MOTOR_CHECK(b.brilloTirada() > 0.0f && b.brilloTirada() < 1.0f);
    b.mira(v, 0.1f + BarraCombate::kBrilloTirada + 0.1f, m.batalla.get());
    MOTOR_CHECK(b.brilloTirada() == 0.0f);
    m.ronda();
    v = vista(m);
    b.mira(v, 2.0f, m.batalla.get());
    MOTOR_CHECK(v.tirada.numero == 2 && !v.tirada.delJugador && b.brilloTirada() == 1.0f);
    // Otro combate empieza sin brillo aunque su mesa ya hubiera tirado.
    Mesa otra("bardo_de_las_cuatro_estaciones");
    otra.atacar();
    b.empieza(vista(otra));
    MOTOR_CHECK(b.brilloTirada() == 1.0f);  // la primera vez que se ve, es nueva
}

// Oriol (05/10): «que se vean todas las tiradas, y dale mas pausa». Un area
// tira una vez por blanco: salen una detras de otra, cada una kPausaTirada
// segundos, con su «1/2», y el rival no actua hasta que se han visto.
MOTOR_TEST_CASE(barra_las_tiradas_de_un_area_salen_una_detras_de_otra) {
    Mesa m("bardo_de_las_cuatro_estaciones", 1, 4, 2);
    BarraCombate b;
    b.mira(vista(m), 0.0f, m.batalla.get());
    MOTOR_CHECK(m.jugar("Canción del Otoño"));  // a los dos lobos
    const VistaBarra v = vista(m);
    MOTOR_CHECK(v.tiradas.size() == 2);
    if (v.tiradas.size() != 2) return;
    MOTOR_CHECK(v.tiradas[0].accion == v.tiradas[1].accion);
    MOTOR_CHECK(b.tiradasPorVer(v));  // la barra aun no las ha mirado
    b.esperaRival("los enemigos");
    b.mira(v, 0.1f, m.batalla.get());
    MOTOR_CHECK(b.tiradaEnPantalla().numero == 1 && b.tiradaDe() == 1 && b.tiradaEn() == 2);
    MOTOR_CHECK(b.brilloTirada() == 1.0f);
    b.mira(v, 0.1f + BarraCombate::kEsperaRival + 0.1f, m.batalla.get());
    MOTOR_CHECK(b.tiradaEnPantalla().numero == 1);  // aun la primera
    MOTOR_CHECK(!b.rivalListo());                    // su rotulo ya se ha visto, la tirada no
    b.mira(v, 0.1f + BarraCombate::kPausaTirada + 0.05f, m.batalla.get());
    MOTOR_CHECK(b.tiradaEnPantalla().numero == 2 && b.tiradaDe() == 2 && b.tiradaEn() == 2);
    MOTOR_CHECK(b.brilloTirada() == 1.0f);
    MOTOR_CHECK(!b.rivalListo());
    b.mira(v, 0.1f + 2.0f * BarraCombate::kPausaTirada + 0.1f, m.batalla.get());
    MOTOR_CHECK(b.tiradasVistas() && !b.tiradasPorVer(v));
    MOTOR_CHECK(b.rivalListo());
    MOTOR_CHECK(b.tiradaEnPantalla().numero == 2);  // la ultima se queda a la vista
}

int main() { return motor_test::runAllTests(); }
