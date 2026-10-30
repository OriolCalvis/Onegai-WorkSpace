// U4-07.E06 — proyectarParticulas, mundoDePixel y el tope de quads. GL-free (solo datos de forma, sin GL).
#include "Game/Emisor/ProyeccionParticulas.h"

#include "TestHarness.h"

#include <cmath>
#include <functional>
#include <vector>

using namespace Game::Emisor;
using Game::MovimientoDeParticula;
using Game::TipoDeParticula;
using Game::tipoDeParticula;

namespace {

bool casi(float a, float b, float tol = 1e-6f) { return std::abs(a - b) <= tol; }

DefinicionEmisor def(const char* familia, int maximo, double duracion = 0.0, bool cortar = false) {
    DefinicionEmisor d;
    d.familia = familia;
    d.tasa = 500.0; d.duracion = duracion; d.cortar = cortar;
    d.vidaMin = 1.0; d.vidaMax = 1.0;
    d.velocidadMin = 3.0; d.velocidadMax = 8.0;
    d.maximo = maximo; d.semilla = 11;
    return d;
}

// Avanza hasta `pasos` ticks de 0.01 s y fija las particulas: edad `edad`, vida 1 (x,y a 0). Asi el test no depende del azar.
void avanza(Emisor& e, int pasos) {
    PresupuestoParticulas pr;
    for (int i = 0; i < pasos; ++i) MOTOR_CHECK(avanzarEmisor(e, 0.01, 1.0, pr).isOk());
}
void fija(const Emisor& e, float edad, const std::function<void(Particula&)>& f = {}) {
    // el test es el unico que escribe en el buffer (la API publica de E05 es de lectura); el objeto no es const
    for (Particula& p : const_cast<std::vector<Particula>&>(e.buffer().datos())) {
        p.edad = edad; p.vida = 1.0f; p.x = 0.0f; p.y = 0.0f;
        if (f) f(p);
    }
}
Emisor lleno(const DefinicionEmisor& d, int pasos, float edad, GrupoDeDibujo g = {}) {
    Emisor e(d, Vector2{0.0f, 0.0f}, g);
    avanza(e, pasos);
    fija(e, edad);
    return e;
}
std::vector<Game::Destello> proy(const Emisor& e, const EstiloEmisor& s = {}, std::size_t q = 100000) {
    std::vector<Game::Destello> v;
    proyectarParticulas(e, s, q, v);
    return v;
}

MOTOR_TEST_CASE(mundo_de_pixel_grupos_y_redondeo) {
    const GrupoDeDibujo A{{100.0f, 50.0f}, {64.0f, 64.0f}};
    const GrupoDeDibujo B{{300.0f, 80.0f}, {128.0f, 128.0f}};
    const Vector2 a = mundoDePixel(A, 10.0f, 20.0f, 1, 0);
    const Vector2 b = mundoDePixel(B, 10.0f, 20.0f, 1, 0);
    MOTOR_CHECK(a.x == 111.0f && a.y == 70.0f);
    MOTOR_CHECK(b.x == 322.0f && b.y == 120.0f);
    // round(0.5)=1, round(-3.5)=-4: se redondea ANTES de escalar
    const Vector2 n = mundoDePixel(B, 0.5f, -3.5f, -1, -1);
    MOTOR_CHECK(n.x == 300.0f && n.y == 70.0f);
    const Vector2 m = mundoDePixel(B, -0.5f, 2.5f, 0, 0);   // round(-0.5) = -1; round(2.5) = 3
    MOTOR_CHECK(m.x == 298.0f && m.y == 86.0f);
    MOTOR_CHECK(!Game::pixelesDeForma(Game::FormaDeParticula::Dragon).empty());
    MOTOR_CHECK(!Game::contornoDeForma(Game::FormaDeParticula::Dragon).empty());
}

MOTOR_TEST_CASE(paridad_de_color_opacidad_y_contraste) {
    auto da = def("agua", 8);
    da.colorBase = {0x5a, 0xb4, 0xff};
    Emisor ea = lleno(da, 3, 0.5f);
    const auto va = proy(ea);
    MOTOR_CHECK(va.size() == ea.buffer().vivas() && !va.empty());
    for (const auto& d : va) {
        MOTOR_CHECK((d.color == std::array<std::uint8_t, 3>{0x5a, 0xb4, 0xff}));
        MOTOR_CHECK(d.suma == ea.tipo().suma);
        MOTOR_CHECK(d.alfa >= 0.0f && d.alfa <= 1.0f);
    }
    auto dc = def("chispas", 8);
    dc.colorBase = {0xff, 0xd2, 0x8c};
    Emisor ec = lleno(dc, 3, 0.5f);
    for (const auto& d : proy(ec)) MOTOR_CHECK((d.color == std::array<std::uint8_t, 3>{0xff, 0xd2, 0x8c}));

    auto dop = def("agua", 4);
    dop.opacidad = 1.45f;
    Emisor eo = lleno(dop, 10, 0.5f);
    for (const auto& d : proy(eo)) MOTOR_CHECK(casi(d.alfa, 0.725f));
    fija(eo, 0.2f);   // vivo 0.8 -> min(1, 1.16)
    for (const auto& d : proy(eo)) MOTOR_CHECK(d.alfa == 1.0f);

    auto dk = def("chispas", 4);
    dk.contraste = 1.0f; dk.colorBase = {200, 100, 50};
    Emisor ek = lleno(dk, 2, 0.5f);
    const auto vk = proy(ek);
    MOTOR_CHECK(!vk.empty());
    for (const auto& d : vk) {
        MOTOR_CHECK(!d.suma);
        MOTOR_CHECK((d.color == std::array<std::uint8_t, 3>{200 * 42 / 100, 100 * 42 / 100, 50 * 42 / 100}));
    }
}

MOTOR_TEST_CASE(envolvente_de_entrada_y_salida_con_cortar) {
    auto d = def("agua", 4, 1.4, true);
    EstiloEmisor s;   // entrada 0.08, salida 0.4
    Emisor e1 = lleno(d, 4, 0.1f);      // tiempo 0.04
    for (const auto& x : proy(e1, s)) MOTOR_CHECK(casi(x.alfa, 0.9f * 0.5f));   // vivo 0.9 * entrada 0.5
    Emisor e2 = lleno(d, 8, 0.0f);      // tiempo 0.08
    for (const auto& x : proy(e2, s)) MOTOR_CHECK(casi(x.alfa, 1.0f));
    Emisor e3 = lleno(d, 50, 0.0f);     // tiempo 0.5
    for (const auto& x : proy(e3, s)) MOTOR_CHECK(casi(x.alfa, 1.0f));
    Emisor e4 = lleno(d, 139, 0.10f);   // tiempo 1.39: salida 0.025
    const auto v4 = proy(e4, s);
    MOTOR_CHECK(!v4.empty());
    for (const auto& x : v4) MOTOR_CHECK(casi(x.alfa, 0.9f * 0.025f));
    // la salida a 1.39 sola: vivo 1 -> 0.025
    fija(e4, 0.0f);
    for (const auto& x : proy(e4, s)) MOTOR_CHECK(casi(x.alfa, 0.025f));
    // alfaMax multiplica
    EstiloEmisor m; m.alfaMax = 0.5f;
    for (const auto& x : proy(e2, m)) MOTOR_CHECK(casi(x.alfa, 0.5f));
}

MOTOR_TEST_CASE(corte_duro_y_cortar_false_sin_salida) {
    auto d = def("agua", 4, 1.4, true);
    Emisor e(d);
    PresupuestoParticulas pr;
    for (int i = 0; i < 139; ++i) MOTOR_CHECK(avanzarEmisor(e, 0.01, 1.0, pr).isOk());
    MOTOR_CHECK(e.buffer().vivas() > 0 && pr.vivas == e.buffer().vivas());
    MOTOR_CHECK(avanzarEmisor(e, 0.01, 1.0, pr).isOk());   // tiempo 1.40: corta todo
    MOTOR_CHECK(e.buffer().vivas() == 0 && pr.vivas == 0);
    MOTOR_CHECK(proy(e).empty());
    // nada nacido en los ultimos 0.1 s sobrevive: vida 1 y corte a 1.4 -> las de t>1.3 mueren por el corte
    // cortar=false, duracion=1: sin salida
    auto f = def("agua", 4, 1.0, false);
    Emisor ef = lleno(f, 99, 0.0f);   // tiempo 0.99
    for (const auto& x : proy(ef)) MOTOR_CHECK(casi(x.alfa, 1.0f));
    f.tasa = 10.0;   // reparte los nacimientos en toda la emision
    Emisor eg(f);
    PresupuestoParticulas pg;
    for (int i = 0; i < 120; ++i) MOTOR_CHECK(avanzarEmisor(eg, 0.01, 1.0, pg).isOk());   // tiempo 1.2
    MOTOR_CHECK(eg.tiempo() > 1.19);
    if (eg.buffer().vivas() > 0) {
        const auto v = proy(eg);
        MOTOR_CHECK(v.size() == eg.buffer().vivas());
        std::size_t i = 0;
        for (const Particula& p : eg.buffer().datos()) MOTOR_CHECK(casi(v[i++].alfa, std::min(1.0f, 1.0f - p.edad / p.vida)));
    } else {
        MOTOR_CHECK(false);   // con vida 1 y emision hasta 1.0 aun quedan vivas a 1.2
    }
}

MOTOR_TEST_CASE(calaveras_cuentan_34_quads_y_tope_global) {
    auto d = def("veneno", 256);
    const TipoDeParticula tipo = tipoDeParticula("veneno");
    MOTOR_CHECK(Game::pixelesDeForma(Game::FormaDeParticula::Calavera).size() == 16);
    MOTOR_CHECK(Game::contornoDeForma(Game::FormaDeParticula::Calavera).size() == 18);
    auto uno = d; uno.maximo = 1;
    Emisor e1(uno, tipo, Vector2{}, GrupoDeDibujo{});
    avanza(e1, 1); fija(e1, 0.0f);
    MOTOR_CHECK(e1.buffer().vivas() == 1);
    MOTOR_CHECK(quadsDeDestellos(proy(e1)) == 34);

    Emisor a(d, tipo, Vector2{}, GrupoDeDibujo{});
    Emisor b(d, tipo, Vector2{}, GrupoDeDibujo{});
    avanza(a, 60); avanza(b, 60);
    fija(a, 0.0f); fija(b, 0.0f);
    MOTOR_CHECK(a.buffer().vivas() == 256 && b.buffer().vivas() == 256);
    std::vector<Game::Destello> out;
    const ResumenProyeccion ra = proyectarParticulas(a, {}, 8192, out);
    const ResumenProyeccion rb = proyectarParticulas(b, {}, 8192 - ra.quads, out);
    MOTOR_CHECK(ra.destellos == 240 && ra.quads == 240 * 34 && ra.descartados == 16);
    MOTOR_CHECK(rb.destellos == 0 && rb.descartados == 256);
    MOTOR_CHECK(ra.destellos + rb.destellos == 240 && ra.descartados + rb.descartados == 272);
    MOTOR_CHECK(out.size() == 240 && quadsDeDestellos(out) == 8160);
    // quadsMax = 0: todo descartado
    std::vector<Game::Destello> vacio;
    const ResumenProyeccion r0 = proyectarParticulas(a, {}, 0, vacio);
    MOTOR_CHECK(vacio.empty() && r0.destellos == 0 && r0.quads == 0 && r0.descartados == 256);
}

MOTOR_TEST_CASE(ocultas_caducadas_acumulacion_y_escala) {
    auto r = def("rayos", 6);
    Emisor er = lleno(r, 2, 0.13f);   // edad > 0.12: ocultas
    MOTOR_CHECK(er.buffer().vivas() > 0 && proy(er).empty());
    fija(er, 0.10f);
    MOTOR_CHECK(proy(er).size() == er.buffer().vivas());
    auto c = def("agua", 6);
    Emisor ec = lleno(c, 2, 0.5f);
    fija(ec, 1.0f, [](Particula& p) { p.vida = 1.0f; });   // edad == vida: caducadas
    MOTOR_CHECK(proy(ec).empty());
    // acumulacion: out no se limpia entre dos llamadas
    Emisor ea = lleno(c, 2, 0.5f);
    std::vector<Game::Destello> out;
    const auto r1 = proyectarParticulas(ea, {}, 1000, out);
    const auto r2 = proyectarParticulas(ea, {}, 1000, out);
    MOTOR_CHECK(r1.destellos > 0 && out.size() == r1.destellos + r2.destellos);
    // escala: solo el tamano; la proyeccion no cambia
    EstiloEmisor s2; s2.escala = 3.0f;
    const auto v1 = proy(ea), v2 = proy(ea, s2);
    MOTOR_CHECK(v1.size() == v2.size());
    for (std::size_t i = 0; i < v1.size(); ++i)
        MOTOR_CHECK(v1[i].x == v2[i].x && v1[i].y == v2[i].y && v1[i].alfa == v2[i].alfa && v1[i].color == v2[i].color);
    // alfa siempre en [0,1], incluso con opacidad 2 y alfaMax fuera de rango
    auto o = def("agua", 6); o.opacidad = 2.0f;
    Emisor eo = lleno(o, 3, 0.0f);
    EstiloEmisor g; g.alfaMax = 7.0f; g.entrada = 0.0f;
    for (const auto& x : proy(eo, g)) MOTOR_CHECK(x.alfa >= 0.0f && x.alfa <= 1.0f);
}

MOTOR_TEST_CASE(expande_dibuja_10_puntos_en_aro_aplastado) {
    auto d = def("agua", 3);
    TipoDeParticula t = tipoDeParticula("agua");
    t.movimiento = MovimientoDeParticula::Expande;
    t.forma = Game::FormaDeParticula::Punto;
    Emisor e(d, t, Vector2{5.0f, 7.0f}, GrupoDeDibujo{});
    avanza(e, 10); fija(e, 0.5f);
    const std::size_t n = e.buffer().vivas();
    MOTOR_CHECK(n == 3);
    const auto v = proy(e);
    MOTOR_CHECK(v.size() == 10 * n);
    // radio 1 + 6*0.5 = 4; k=0: (5+4, 7+0); k=2 (72 grados): y = 7 + 4*0.6*sin72 (aplastada, NO 4*sin72)
    MOTOR_CHECK(casi(v[0].x, 9.0f, 1e-5f) && casi(v[0].y, 7.0f, 1e-5f));
    MOTOR_CHECK(casi(v[2].x, 5.0f + 4.0f * std::cos(0.4 * 3.14159265358979), 1e-4f));
    MOTOR_CHECK(casi(v[2].y, 7.0f + 4.0f * 0.6f * std::sin(0.4 * 3.14159265358979), 1e-4f));
    MOTOR_CHECK(casi(v[5].x, 1.0f, 1e-4f));
    for (const auto& x : v) MOTOR_CHECK(x.forma == Game::FormaDeParticula::Punto && casi(x.alfa, 0.5f * 0.8f));
    MOTOR_CHECK(quadsDeDestellos(v) == 10 * n * Game::pixelesDeForma(Game::FormaDeParticula::Punto).size());
}

}  // namespace

int main() { return motor_test::runAllTests(); }
