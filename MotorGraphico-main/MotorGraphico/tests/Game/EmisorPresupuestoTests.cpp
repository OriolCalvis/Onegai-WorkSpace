// U4-07.E05 — presupuesto global derivado, fugas, admision y movimiento en forma cerrada. GL-free.
#include "Game/Emisor/Emisor.h"
#include "Game/Emisor/MovimientoParticula.h"

#include "TestHarness.h"

#include <cmath>
#include <cstring>
#include <vector>

using namespace Game::Emisor;
using Game::FormaDeParticula;
using Game::MovimientoDeParticula;
using Game::TipoDeParticula;

namespace {

constexpr double kTick = 1.0 / 60.0;
constexpr float kPi = 3.14159265f;

DefinicionEmisor def(const char* familia, double tasa, double duracion, double vida, int maximo, std::uint64_t semilla) {
    DefinicionEmisor d;
    d.familia = familia;
    d.tasa = tasa; d.duracion = duracion;
    d.vidaMin = d.vidaMax = vida;
    d.velocidadMin = 3.0; d.velocidadMax = 8.0;
    d.anguloCentro = -90.0; d.abanico = 140.0;
    d.maximo = maximo; d.semilla = semilla;
    return d;
}

PasoParticulas paso(Emisor& e, double dt, double escala, PresupuestoParticulas& p) {
    auto r = avanzarEmisor(e, dt, escala, p);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

bool cerca(float a, float b, float tol = 1e-4f) { return std::fabs(a - b) <= tol; }
bool igualesBits(const Particula& a, const Particula& b) { return std::memcmp(&a, &b, sizeof(Particula)) == 0; }

TipoDeParticula tipo(float vx, float vy, MovimientoDeParticula m, FormaDeParticula f = FormaDeParticula::Punto) {
    TipoDeParticula t;
    t.vx = vx; t.vy = vy; t.movimiento = m; t.forma = f;
    return t;
}

Particula nacida(float edad, float vida, float lado, std::uint16_t k) {
    Particula p;
    p.edad = edad; p.vida = vida; p.lado = lado; p.k = k;
    return p;
}

MOTOR_TEST_CASE(presupuesto_reserva_libera_y_recuenta) {
    PresupuestoParticulas p;
    p.maximo = 2;
    MOTOR_CHECK(p.reserva() && p.reserva() && !p.reserva() && p.vivas == 2);
    p.libera(5);   // nunca baja de 0
    MOTOR_CHECK(p.vivas == 0);
    std::vector<Emisor> v;
    v.emplace_back(def("polvo", 100.0, 0.0, 1.0, 8, 1));
    PresupuestoParticulas q;
    paso(v[0], 0.05, 1.0, q);
    p.vivas = 99;
    p.recuenta(v);
    MOTOR_CHECK(p.vivas == v[0].buffer().vivas() && p.vivas == 5);
}

MOTOR_TEST_CASE(reserva_antes_de_sortear_el_rechazo_no_consume_azar) {
    const DefinicionEmisor d = def("polvo", 100.0, 0.0, 1.0, 16, 77);
    Emisor e(d), ref(d);
    PresupuestoParticulas bloqueado, libre;
    bloqueado.maximo = 0;
    const PasoParticulas r = paso(e, 0.1, 1.0, bloqueado);   // 10 rechazos: no se sortea nada
    MOTOR_CHECK(r.rechazadas == 10 && r.nacidas == 0 && e.nacidas() == 0 && e.buffer().vivas() == 0);
    bloqueado.maximo = 100;
    paso(e, 0.1, 1.0, bloqueado);
    paso(ref, 0.1, 1.0, libre);
    // las particulas del segundo paso de `e` son las PRIMERAS del generador: iguales a las de `ref`
    MOTOR_CHECK(e.buffer().vivas() == 10 && ref.buffer().vivas() == 10);
    for (std::size_t i = 0; i < 10; ++i) MOTOR_CHECK(igualesBits(e.buffer().datos()[i], ref.buffer().datos()[i]));
}

MOTOR_TEST_CASE(aforo_lleno_rechaza_antes_de_sortear) {
    const DefinicionEmisor d = def("polvo", 500.0, 0.0, 2.0, 8, 3);
    Emisor e(d);
    PresupuestoParticulas p;
    const auto cap = e.buffer().datos().capacity();
    int rechazadas = 0;
    for (int i = 0; i < 60; ++i) {   // 1 s
        rechazadas += paso(e, kTick, 1.0, p).rechazadas;
        MOTOR_CHECK(e.buffer().vivas() <= 8 && p.vivas == e.buffer().vivas() && e.buffer().datos().capacity() == cap);
    }
    MOTOR_CHECK(e.buffer().vivas() == 8 && rechazadas > 0 && e.nacidas() == 8);   // solo se sortean las admitidas
}

MOTOR_TEST_CASE(las_caducadas_liberan_antes_de_nacer_las_nuevas_del_mismo_paso) {
    Emisor e(def("polvo", 100.0, 0.0, 0.04, 16, 4));
    PresupuestoParticulas p;
    p.maximo = 4;
    PasoParticulas r = paso(e, 0.04, 1.0, p);   // 4 nacidas: el presupuesto queda lleno
    MOTOR_CHECK(r.nacidas == 4 && p.vivas == 4 && r.rechazadas == 0);
    r = paso(e, 0.04, 1.0, p);                  // las 4 caducan y en el mismo paso nacen 4 mas
    MOTOR_CHECK(r.caducadas == 4 && r.nacidas == 4 && r.rechazadas == 0 && p.vivas == 4);
}

std::vector<std::size_t> reparto(int emisores) {
    std::vector<Emisor> v;
    for (int i = 0; i < emisores; ++i) v.emplace_back(def("polvo", 500.0, 0.25, 0.1, 256, 100 + i));
    PresupuestoParticulas p;
    std::vector<std::size_t> r;
    int rechazadas = 0;
    for (auto& e : v) rechazadas += paso(e, 0.25, 1.0, p).rechazadas;
    std::size_t suma = 0;
    for (const auto& e : v) { r.push_back(e.buffer().vivas()); suma += e.buffer().vivas(); }
    MOTOR_CHECK(suma <= 2048 && suma == p.vivas && rechazadas > 0);
    for (auto& e : v) paso(e, 0.25, 1.0, p);   // vida 0,1 y duracion 0,25: todo caduca y no nace nada mas
    p.recuenta(v);
    MOTOR_CHECK(p.vivas == 0);
    return r;
}

MOTOR_TEST_CASE(presupuesto_compartido_por_64_emisores_en_orden_de_creacion) {
    const auto a = reparto(64), b = reparto(64);
    MOTOR_CHECK(a == b);                                  // identico en dos runs
    for (std::size_t i = 0; i < 16; ++i) MOTOR_CHECK(a[i] == 125);   // los primeros consumen 125 cada uno...
    MOTOR_CHECK(a[16] == 48);                             // ...hasta agotar los 2048
    for (std::size_t i = 17; i < 64; ++i) MOTOR_CHECK(a[i] == 0);
}

MOTOR_TEST_CASE(fuga_de_presupuesto_x100_se_corrige_con_recuenta) {
    PresupuestoParticulas p;
    p.maximo = 20;
    for (int ciclo = 0; ciclo < 100; ++ciclo) {
        std::vector<Emisor> v;
        v.emplace_back(def("polvo", 500.0, 0.0, 5.0, 20, 9 + ciclo));
        paso(v[0], 0.25, 1.0, p);
        MOTOR_CHECK(v[0].buffer().vivas() == 20 && p.vivas == 20);   // lleno, sin caducar
        v.clear();                                                    // destruido: la cuenta incremental queda desfasada
        std::vector<Emisor> vacio;
        p.recuenta(vacio);
        MOTOR_CHECK(p.vivas == 0);
    }
    Emisor e(def("polvo", 500.0, 0.0, 5.0, 20, 5));
    MOTOR_CHECK(paso(e, 0.25, 1.0, p).nacidas == 20 && p.vivas == 20);   // se siguen admitiendo `maximo`
}

MOTOR_TEST_CASE(olvida_y_poda_recuentan) {
    std::vector<Emisor> v;
    PresupuestoParticulas p;
    for (int i = 0; i < 4; ++i) {
        v.emplace_back(def("polvo", 100.0, i < 2 ? 0.1 : 0.0, 0.05, 16, 20 + i));
        paso(v.back(), 0.1, 1.0, p);
    }
    MOTOR_CHECK(p.vivas == 40);
    paso(v[0], 0.1, 1.0, p); paso(v[1], 0.1, 1.0, p);   // tiempo 0,2 > duracion; vida 0,05: se vacian
    p.vivas = 999;                                      // cuenta incorrecta a proposito
    MOTOR_CHECK(podaAcabados(v, p) == 2 && v.size() == 2 && p.vivas == 20);
    p.vivas = 999;
    olvida(v, p);
    MOTOR_CHECK(v.empty() && p.vivas == 0);
}

// La posicion solo depende de la edad: el golpe nace en un tick y luego se envejece `total` s en n trozos.
std::vector<Particula> envejecidas(const TipoDeParticula& t, double total, int trozos) {
    Emisor e(def("polvo", 500.0, 0.01, 2.0, 16, 31), t, Vector2{}, GrupoDeDibujo{});
    PresupuestoParticulas p;
    paso(e, 0.01, 1.0, p);
    for (int i = 0; i < trozos; ++i) paso(e, total / trozos, 1.0, p);
    return e.buffer().datos();
}

MOTOR_TEST_CASE(forma_cerrada_el_troceado_no_cambia_la_posicion) {
    const MovimientoDeParticula todos[] = {MovimientoDeParticula::Sube,    MovimientoDeParticula::Cae,
                                           MovimientoDeParticula::Deriva,  MovimientoDeParticula::Converge,
                                           MovimientoDeParticula::Orbita,  MovimientoDeParticula::Expande,
                                           MovimientoDeParticula::Chispazo, MovimientoDeParticula::Salpica};
    for (const auto m : todos) {
        const TipoDeParticula t = tipo(7.0f, -12.0f, m);
        const auto a = envejecidas(t, 0.5, 2), b = envejecidas(t, 0.5, 4), c = envejecidas(t, 0.5, 8);
        MOTOR_CHECK(a.size() == 5 && b.size() == 5 && c.size() == 5);
        for (std::size_t i = 0; i < a.size(); ++i) {
            MOTOR_CHECK(a[i].edad == 0.5f);
            MOTOR_CHECK(igualesBits(a[i], b[i]) && igualesBits(a[i], c[i]));
        }
        if (m != MovimientoDeParticula::Expande) MOTOR_CHECK(a[0].x != 0.0f || a[0].y != 0.0f);
    }
    // 0,5 de golpe se recorta a 0,25: no equivale a 2 x 0,25
    const TipoDeParticula t = tipo(7.0f, -12.0f, MovimientoDeParticula::Deriva);
    MOTOR_CHECK(!igualesBits(envejecidas(t, 0.5, 1)[0], envejecidas(t, 0.5, 2)[0]));
}

MOTOR_TEST_CASE(valores_concretos_de_cada_formula) {
    Particula p;
    // Cae (agua: vx 1, vy 6), t=0,5, k=0, lado=1: x = 1*sin(1,5)*0,4 + 1*1*0,5*0,5; y = 6*0,5
    p = nacida(0.5f, 1.0f, 1.0f, 0);
    colocaParticula(tipo(1.0f, 6.0f, MovimientoDeParticula::Cae), p);
    MOTOR_CHECK(cerca(p.x, 0.648998f) && cerca(p.y, 3.0f));
    // y la velocidad inicial se suma: (vx0,vy0)*t
    p.vx0 = 4.0f; p.vy0 = -8.0f;
    colocaParticula(tipo(1.0f, 6.0f, MovimientoDeParticula::Sube), p);
    MOTOR_CHECK(cerca(p.x, 2.0f + 0.648998f) && cerca(p.y, -4.0f + 3.0f));
    // Deriva (polvo: vx 9, vy -2), lado<0 -> sentido -1, k=2: x = vx0*t - 9t + sin(5t+k)*0,6
    p = nacida(0.5f, 1.0f, -0.3f, 2);
    p.vx0 = 3.0f; p.vy0 = -6.0f;
    colocaParticula(tipo(9.0f, -2.0f, MovimientoDeParticula::Deriva), p);
    MOTOR_CHECK(cerca(p.x, 1.5f - 4.5f + static_cast<float>(std::sin(4.5)) * 0.6f) && cerca(p.y, -3.0f - 1.0f));
    p.lado = 0.3f;   // sentido +1
    colocaParticula(tipo(9.0f, -2.0f, MovimientoDeParticula::Deriva), p);
    MOTOR_CHECK(cerca(p.x, 1.5f + 4.5f + static_cast<float>(std::sin(4.5)) * 0.6f));
    // Salpica (chispas: vx 10, vy -24), vida 0,5, lado -0,5: g = 96
    p = nacida(0.25f, 0.5f, -0.5f, 0);
    colocaParticula(tipo(10.0f, -24.0f, MovimientoDeParticula::Salpica), p);
    MOTOR_CHECK(cerca(p.x, -1.25f) && cerca(p.y, -3.0f));
    p.edad = 0.5f;   // vuelve a su altura al final de SU vida (no la de la familia)
    colocaParticula(tipo(10.0f, -24.0f, MovimientoDeParticula::Salpica), p);
    MOTOR_CHECK(cerca(p.x, -2.5f) && cerca(p.y, 0.0f));
    p.vida = 2.0f; p.edad = 2.0f;   // con otra vida, a su altura en edad == vida
    colocaParticula(tipo(10.0f, -24.0f, MovimientoDeParticula::Salpica), p);
    MOTOR_CHECK(cerca(p.y, 0.0f, 1e-3f));
    // Converge: ignora vx0/vy0; radio0*(1-t/vida) hacia el origen
    p = nacida(0.25f, 1.0f, 0.0f, 0);
    p.vx0 = 50.0f; p.vy0 = 50.0f; p.radio0 = 8.0f; p.ang0 = 0.0f;
    colocaParticula(tipo(0.0f, 0.0f, MovimientoDeParticula::Converge), p);
    MOTOR_CHECK(cerca(p.x, 6.0f) && cerca(p.y, 0.0f));
    p.ang0 = kPi / 2.0f;
    colocaParticula(tipo(0.0f, 0.0f, MovimientoDeParticula::Converge), p);
    MOTOR_CHECK(cerca(p.x, 0.0f, 1e-4f) && cerca(p.y, 6.0f));
    // Orbita (espiritus: vx 6, vy -3), t=1: ang = 3; radio0=6 -> r = 5 (5+3u con u=0)
    p = nacida(1.0f, 2.0f, 0.0f, 0);
    p.vx0 = 50.0f; p.radio0 = 6.0f; p.ang0 = 0.0f;
    colocaParticula(tipo(6.0f, -3.0f, MovimientoDeParticula::Orbita), p);
    MOTOR_CHECK(cerca(p.x, 5.0f * static_cast<float>(std::cos(3.0))) && cerca(p.y, 2.5f * static_cast<float>(std::sin(3.0)) - 3.0f));
    // Expande: el aro lo dibuja E06; el estado esta en el origen e ignora vx0/vy0
    p.vy0 = 50.0f;
    colocaParticula(tipo(0.0f, 0.0f, MovimientoDeParticula::Expande), p);
    MOTOR_CHECK(p.x == 0.0f && p.y == 0.0f);
}

MOTOR_TEST_CASE(fase_k_vida_de_la_particula_vy0_y_colocacion_al_nacer) {
    // k != 0 desplaza la fase del balanceo: Cae/Sube con sin(3t+k) y Deriva con sin(5t+k)
    Particula p = nacida(0.5f, 1.0f, 1.0f, 3);
    colocaParticula(tipo(1.0f, 6.0f, MovimientoDeParticula::Cae), p);
    MOTOR_CHECK(cerca(p.x, static_cast<float>(std::sin(4.5)) * 0.4f + 0.25f) && cerca(p.y, 3.0f));
    p = nacida(0.5f, 1.0f, 0.5f, 7);
    colocaParticula(tipo(9.0f, -2.0f, MovimientoDeParticula::Deriva), p);
    MOTOR_CHECK(cerca(p.x, 4.5f + static_cast<float>(std::sin(9.5)) * 0.6f));
    // Converge usa la vida de la PARTICULA (2), no la de la familia (1 por defecto)
    p = nacida(0.5f, 2.0f, 0.0f, 0);
    p.radio0 = 8.0f; p.ang0 = 0.0f;
    colocaParticula(tipo(0.0f, 0.0f, MovimientoDeParticula::Converge), p);
    MOTOR_CHECK(cerca(p.x, 6.0f) && cerca(p.y, 0.0f));
    // Salpica suma vy0*t y vx0*t
    p = nacida(0.25f, 0.5f, -0.5f, 0);
    p.vx0 = 4.0f; p.vy0 = -10.0f;
    colocaParticula(tipo(10.0f, -24.0f, MovimientoDeParticula::Salpica), p);
    MOTOR_CHECK(cerca(p.x, -1.25f + 1.0f) && cerca(p.y, -3.0f - 2.5f));
    // el recien nacido ya esta colocado (edad 0) en el paso de su nacimiento
    Emisor o(def("polvo", 500.0, 0.01, 2.0, 16, 41), tipo(6.0f, -3.0f, MovimientoDeParticula::Orbita), Vector2{}, GrupoDeDibujo{});
    PresupuestoParticulas pr;
    paso(o, 0.01, 1.0, pr);
    for (const auto& q : o.buffer().datos()) {
        MOTOR_CHECK(q.edad == 0.0f && std::fabs(q.x) + std::fabs(q.y) > 0.5f);
        const float r = (q.radio0 - 6.0f) * 0.75f + 5.0f;
        MOTOR_CHECK(cerca(q.x, r * std::cos(q.ang0), 1e-3f));
    }
    Emisor s(def("polvo", 500.0, 0.01, 2.0, 16, 42), tipo(10.0f, 0.0f, MovimientoDeParticula::Sube), Vector2{}, GrupoDeDibujo{});
    paso(s, 0.01, 1.0, pr);
    const auto& v = s.buffer().datos();   // vx0 = 0 no se aplica a t=0; el balanceo de k=1 si
    MOTOR_CHECK(v.size() == 5 && v[0].x == 0.0f && cerca(v[1].x, 10.0f * static_cast<float>(std::sin(1.0)) * 0.4f, 1e-3f));
}

MOTOR_TEST_CASE(orbita_reescala_radio0_de_6_10_a_5_8) {
    const TipoDeParticula t = tipo(0.0f, 0.0f, MovimientoDeParticula::Orbita);   // vx 0: el angulo no gira
    Particula p = nacida(0.0f, 1.0f, 0.0f, 0);
    p.ang0 = 0.0f;
    const float radio0[] = {6.0f, 8.0f, 10.0f}, esperado[] = {5.0f, 6.5f, 8.0f};
    for (int i = 0; i < 3; ++i) {
        p.radio0 = radio0[i];
        colocaParticula(t, p);
        MOTOR_CHECK(cerca(p.x, esperado[i]) && cerca(p.y, 0.0f));
    }
    p.radio0 = 10.0f; p.ang0 = kPi / 2.0f;   // el eje vertical va aplastado a la mitad
    colocaParticula(t, p);
    MOTOR_CHECK(cerca(p.y, 4.0f, 1e-3f));
}

MOTOR_TEST_CASE(rayo_y_chispazo_se_ocultan_pasados_0_12_s) {
    const TipoDeParticula rayo = Game::tipoDeParticula("rayos");
    MOTOR_CHECK(rayo.forma == FormaDeParticula::Rayo && rayo.movimiento == MovimientoDeParticula::Chispazo);
    Particula p = nacida(0.13f, 0.25f, 0.0f, 0);
    MOTOR_CHECK(estaOculta(rayo, p));
    p.edad = 0.12f;
    MOTOR_CHECK(!estaOculta(rayo, p));   // el limite es estricto: edad > 0,12
    p.edad = 0.0f;
    MOTOR_CHECK(!estaOculta(rayo, p));
    p.edad = 1.0f;
    MOTOR_CHECK(estaOculta(tipo(0.0f, 0.0f, MovimientoDeParticula::Chispazo), p));       // solo el movimiento
    MOTOR_CHECK(estaOculta(tipo(0.0f, 0.0f, MovimientoDeParticula::Sube, FormaDeParticula::Rayo), p));   // solo la forma
    MOTOR_CHECK(!estaOculta(Game::tipoDeParticula("polvo"), p));
    // por el emisor real: sigue viva (vida 0,25) pero oculta a 0,13 s
    Emisor e(def("rayos", 500.0, 0.01, 0.25, 8, 6));
    PresupuestoParticulas pr;
    paso(e, 0.01, 1.0, pr);
    paso(e, 0.1, 1.0, pr);
    MOTOR_CHECK(e.buffer().vivas() == 5 && !estaOculta(e.tipo(), e.buffer().datos()[0]));
    paso(e, 0.03, 1.0, pr);
    MOTOR_CHECK(e.buffer().vivas() == 5 && estaOculta(e.tipo(), e.buffer().datos()[0]));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
