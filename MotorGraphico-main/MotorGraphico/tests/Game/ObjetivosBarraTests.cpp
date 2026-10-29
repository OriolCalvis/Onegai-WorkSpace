// H2-02 — A quien apunta la carta senalada de la barra (Game/BarraCombate.h,
// BarraObjetivos.cpp), sobre la mesa real contra dos lobos (CartasPrueba.h,
// dado fijo en 5).
//
// El criterio de la Fase H: «En combate, elegir un hechizo de area -> se
// marcan los afectados -> cancelar no gasta PM». Y lo que lo hace posible: la
// regla de a quien alcanza un objetivo es una sola (CartasEnCombate::alcance),
// la misma al marcar que al jugar.
#include "CartasPrueba.h"
#include "Game/BarraCombate.h"
#include "TestHarness.h"

#include <string>
#include <vector>

using namespace cartas_prueba;
using Game::BarraCombate;
using Game::Jugada;
using Game::VistaBarra;
using Alcanzado = Game::CartasEnCombate::Alcanzado;

namespace {

VistaBarra vista(const Mesa& m) { return Game::vistaDeBatalla(*m.batalla); }

int posicion(const VistaBarra& v, const std::string& id) {
    for (int p = 0; p < BarraCombate::total(v); ++p)
        if (BarraCombate::carta(v, p)->id == id) return p;
    return -1;
}

// Senala la carta con su tecla (la primera pulsacion no la juega).
void senala(BarraCombate& b, const VistaBarra& v, const std::string& id) {
    const int p = posicion(v, id);
    MOTOR_CHECK(p >= 0);
    MOTOR_CHECK(b.tecla(v, BarraCombate::carta(v, p)->tecla - 1).tipo == Jugada::Tipo::Nada);
    MOTOR_CHECK(b.foco() == p);
}

std::vector<std::size_t> enemigos(const std::vector<Alcanzado>& a) {
    std::vector<std::size_t> out;
    for (const Alcanzado& x : a)
        if (!x.aliado) out.push_back(x.i);
    return out;
}

// Juega lo que devolvio la barra, como Application::confirmBattleCommand.
void juega(Mesa& m, const Jugada& j) {
    MOTOR_CHECK(j.tipo == Jugada::Tipo::Fila);
    BattleAction a;
    if (j.fila == 0) {
        a = BattleAction{BattleActionType::Attack, "", j.objetivo < 0 ? 0 : j.objetivo, ""};
    } else {
        MOTOR_CHECK(m.cartas->accionDeFila(j.fila - 3, a));
        if (j.objetivo >= 0) a.targetIndex = j.objetivo;
    }
    m.batalla->resolveAllyAction(0, a);
}

}  // namespace

MOTOR_TEST_CASE(objetivos_un_area_marca_a_todos_y_mirar_otra_no_gasta) {
    Mesa m("bardo_de_las_cuatro_estaciones", 1, 4, 2);
    MOTOR_CHECK(m.jugar("Elegir Invierno"));
    BarraCombate b;
    VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    MOTOR_CHECK(v.enemigosVivos == std::vector<bool>({true, true}) && v.aliadosVivos == std::vector<bool>({true}));
    senala(b, v, "cancion_del_otono");  // a todos los enemigos
    MOTOR_CHECK(enemigos(b.afectados(v)) == std::vector<std::size_t>({0, 1}));
    bool aliado = true;
    MOTOR_CHECK(!BarraCombate::eligeObjetivo(*BarraCombate::carta(v, b.foco()), aliado));
    // Volver atras: se mira otra carta y no se ha gastado nada.
    const std::size_t lineas = m.batalla->log().size();
    senala(b, v, "#atacar");
    MOTOR_CHECK(!m.carta("cancion_del_otono").gastada);
    MOTOR_CHECK(!m.cartas->turno().principal && m.batalla->log().size() == lineas);
    // Un lobo caido ya no se marca.
    m.lobo2.ponerVida(0);
    v = vista(m);
    senala(b, v, "cancion_del_otono");
    MOTOR_CHECK(enemigos(b.afectados(v)) == std::vector<std::size_t>({0}));
}

MOTOR_TEST_CASE(objetivos_uno_se_elige_con_w_s_y_se_juega_contra_el) {
    Mesa m("bardo_de_las_cuatro_estaciones", 1, 4, 2);
    MOTOR_CHECK(m.jugar("Elegir Invierno"));
    BarraCombate b;
    const VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    // Atacar va al primero en pie mientras no se elija.
    MOTOR_CHECK(b.foco() == 0 && enemigos(b.afectados(v)) == std::vector<std::size_t>({0}));
    MOTOR_CHECK(b.objetivoDe(v, *BarraCombate::carta(v, 0)) == -1);
    b.mueveObjetivo(v, +1);
    MOTOR_CHECK(enemigos(b.afectados(v)) == std::vector<std::size_t>({1}));
    MOTOR_CHECK(b.objetivoDe(v, *BarraCombate::carta(v, 0)) == 1);
    b.mueveObjetivo(v, +1);  // da la vuelta
    MOTOR_CHECK(enemigos(b.afectados(v)) == std::vector<std::size_t>({0}));
    b.mueveObjetivo(v, -1);
    const Jugada j = b.acepta(v);
    MOTOR_CHECK(j.tipo == Jugada::Tipo::Fila && j.fila == 0 && j.objetivo == 1);
    juega(m, j);  // 3 contra 2: 1d6 al lobo gris y nada al otro
    MOTOR_CHECK(m.lobo2.health() == 55 && m.lobo.health() == 60);
}

MOTOR_TEST_CASE(objetivos_el_raton_senala_y_un_clic_en_el_marcado_juega) {
    Mesa m("bardo_de_las_cuatro_estaciones", 1, 4, 2);
    MOTOR_CHECK(m.jugar("Elegir Invierno"));
    BarraCombate b;
    const VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    const int loboGris = BarraCombate::kObjetivoEnemigo + 1;
    b.senala(v, BarraCombate::Zona::Objetivo, loboGris);  // pasar por encima elige
    MOTOR_CHECK(enemigos(b.afectados(v)) == std::vector<std::size_t>({1}));
    // Un clic en uno que la carta no alcanza (tu retrato, con Atacar) no hace nada.
    MOTOR_CHECK(b.pulsa(v, BarraCombate::Zona::Objetivo, 0).tipo == Jugada::Tipo::Nada);
    const Jugada j = b.pulsa(v, BarraCombate::Zona::Objetivo, loboGris);
    MOTOR_CHECK(j.tipo == Jugada::Tipo::Fila && j.fila == 0 && j.objetivo == 1);
    // Una carta de area: el clic en cualquiera de los marcados la juega.
    senala(b, v, "cancion_del_otono");
    const Jugada area = b.pulsa(v, BarraCombate::Zona::Objetivo, BarraCombate::kObjetivoEnemigo);
    MOTOR_CHECK(area.tipo == Jugada::Tipo::Fila && area.objetivo == -1);
    juega(m, area);
    MOTOR_CHECK(m.carta("cancion_del_otono").gastada);
}

MOTOR_TEST_CASE(objetivos_las_tuyas_apuntan_a_tu_bando) {
    Mesa m("bardo_de_las_cuatro_estaciones", 1, 4, 2);
    MOTOR_CHECK(m.jugar("Elegir Invierno"));
    BarraCombate b;
    const VistaBarra v = vista(m);
    b.mira(v, 0.0f, m.batalla.get());
    senala(b, v, "invocacion_elemental");  // un aliado (o tu)
    std::vector<Alcanzado> a = b.afectados(v);
    MOTOR_CHECK(a.size() == 1 && a[0].aliado && a[0].i == 0);
    bool aliado = false;
    MOTOR_CHECK(BarraCombate::eligeObjetivo(*BarraCombate::carta(v, b.foco()), aliado) && aliado);
    b.senala(v, BarraCombate::Zona::Objetivo, BarraCombate::kObjetivoEnemigo);  // un enemigo: no cambia
    a = b.afectados(v);
    MOTOR_CHECK(a.size() == 1 && a[0].aliado);
    senala(b, v, "#pocion");  // la pocion es tuya; huir no apunta a nadie
    a = b.afectados(v);
    MOTOR_CHECK(a.size() == 1 && a[0].aliado && a[0].i == 0);
    senala(b, v, "#huir");
    MOTOR_CHECK(b.afectados(v).empty());
    // Mientras actua el rival no se marca nada.
    senala(b, v, "#atacar");
    b.esperaRival("los enemigos");
    MOTOR_CHECK(b.afectados(v).empty());
}
