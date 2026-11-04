// U4-07.E06 — proyectarParticulas, mundoDePixel y el tope de quads. GL-free (solo datos de forma, sin GL).
#include "Game/Emisor/ProyeccionParticulas.h"

#include "TestHarness.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>
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
    MOTOR_CHECK(edad <= e.tiempo() + 1e-6);   // una particula no puede ser mas vieja que su emisor
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
    Emisor ea = lleno(da, 50, 0.5f);
    const auto va = proy(ea);
    MOTOR_CHECK(va.size() == ea.buffer().vivas() && !va.empty());
    for (const auto& d : va) {
        MOTOR_CHECK((d.color == std::array<std::uint8_t, 3>{0x5a, 0xb4, 0xff}));
        MOTOR_CHECK(d.suma == ea.tipo().suma);
        MOTOR_CHECK(d.alfa >= 0.0f && d.alfa <= 1.0f);
    }
    auto dc = def("chispas", 8);
    dc.colorBase = {0xff, 0xd2, 0x8c};
    Emisor ec = lleno(dc, 50, 0.5f);
    for (const auto& d : proy(ec)) MOTOR_CHECK((d.color == std::array<std::uint8_t, 3>{0xff, 0xd2, 0x8c}));

    auto dop = def("agua", 4);
    dop.opacidad = 1.45f;
    Emisor eo = lleno(dop, 50, 0.5f);
    for (const auto& d : proy(eo)) MOTOR_CHECK(casi(d.alfa, 0.725f));
    fija(eo, 0.2f);   // vivo 0.8 -> min(1, 1.16)
    for (const auto& d : proy(eo)) MOTOR_CHECK(d.alfa == 1.0f);

    auto dk = def("chispas", 4);
    dk.contraste = 1.0f; dk.colorBase = {200, 100, 50};
    Emisor ek = lleno(dk, 50, 0.5f);
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
    Emisor e1 = lleno(d, 4, 0.0f);      // tiempo 0.04, vivo 1: solo la entrada
    const auto v1 = proy(e1, s);
    MOTOR_CHECK(!v1.empty());
    for (const auto& x : v1) MOTOR_CHECK(casi(x.alfa, 0.5f));
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
    // nada nacido en los ultimos 0.1 s sobrevive: el corte vacia el buffer y despues no nace nada
    const std::uint64_t nacidasAlCortar = e.nacidas();
    for (int i = 0; i < 20; ++i) MOTOR_CHECK(avanzarEmisor(e, 0.01, 1.0, pr).isOk());
    MOTOR_CHECK(e.nacidas() == nacidasAlCortar && e.buffer().vivas() == 0 && pr.vivas == 0 && proy(e).empty());
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
        MOTOR_CHECK(v.size() == eg.buffer().datos().size());
        for (std::size_t i = 0; i < v.size() && i < eg.buffer().datos().size(); ++i) {
            const Particula& p = eg.buffer().datos()[i];
            MOTOR_CHECK(casi(v[i].alfa, std::min(1.0f, 1.0f - p.edad / p.vida)));
        }
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
    Emisor er = lleno(r, 20, 0.13f);   // edad > 0.12: ocultas
    MOTOR_CHECK(er.buffer().vivas() > 0 && proy(er).empty());
    fija(er, 0.10f);
    MOTOR_CHECK(proy(er).size() == er.buffer().vivas());
    auto c = def("agua", 6);
    Emisor ec = lleno(c, 50, 0.5f);
    fija(ec, 1.0f, [](Particula& p) { p.vida = 1.0f; });   // edad == vida: caducadas
    MOTOR_CHECK(proy(ec).empty());
    // acumulacion: out no se limpia entre dos llamadas
    Emisor ea = lleno(c, 50, 0.5f);
    std::vector<Game::Destello> out;
    const auto r1 = proyectarParticulas(ea, {}, 1000, out);
    const auto r2 = proyectarParticulas(ea, {}, 1000, out);
    MOTOR_CHECK(r1.destellos > 0 && out.size() == r1.destellos + r2.destellos);
    // escala esta IGNORADA por la proyeccion (ver el header): con escala 3 la salida es identica bit a bit
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
    avanza(e, 50); fija(e, 0.5f);
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

MOTOR_TEST_CASE(dragon_cuerpo_y_contorno_salen_de_mundo_de_pixel) {
    const GrupoDeDibujo B{{300.0f, 80.0f}, {128.0f, 128.0f}};
    for (const auto* pix : {&Game::pixelesDeForma(Game::FormaDeParticula::Dragon),
                            &Game::contornoDeForma(Game::FormaDeParticula::Dragon)}) {
        MOTOR_CHECK(!pix->empty());
        for (const auto& q : *pix) {
            const Vector2 w = mundoDePixel(B, 10.4f, 20.6f, q.dx, q.dy);   // round -> (10, 21)
            MOTOR_CHECK(w.x == 300.0f + (10.0f + static_cast<float>(q.dx)) * 2.0f);
            MOTOR_CHECK(w.y == 80.0f + (21.0f + static_cast<float>(q.dy)) * 2.0f);
        }
    }
}

MOTOR_TEST_CASE(corte_duro_exacto_y_tolerancia) {
    auto d = def("agua", 4, 1.0, false);
    Emisor e(d);
    PresupuestoParticulas pr;
    for (int i = 0; i < 4; ++i) MOTOR_CHECK(avanzarEmisor(e, 0.25, 1.0, pr).isOk());   // tiempo == 1.0 exacto
    MOTOR_CHECK(e.tiempo() == 1.0 && e.buffer().vivas() > 0);
    auto& dm = const_cast<DefinicionEmisor&>(e.definicion());   // el corte se decide en la proyeccion
    EstiloEmisor s; s.salida = 1e-9f;
    dm.cortar = false;
    MOTOR_CHECK(!proy(e, s).empty());           // sin cortar se proyecta
    dm.cortar = true;
    MOTOR_CHECK(proy(e, s).empty());            // tiempo == duracion: corte duro con el buffer NO vacio
    dm.duracion = e.tiempo() + 0.5e-9;
    MOTOR_CHECK(proy(e, s).empty());            // dentro de la tolerancia de 1e-9
    dm.duracion = e.tiempo() + 1e-9;
    MOTOR_CHECK(dm.duracion - 1e-9 == e.tiempo());   // justo en el borde de la tolerancia: se corta (>=)
    MOTOR_CHECK(proy(e, s).empty());
    dm.duracion = e.tiempo() + 2e-9;
    MOTOR_CHECK(!proy(e, s).empty());           // fuera de la tolerancia: aun se proyecta
}

MOTOR_TEST_CASE(color_por_variante_y_suma_de_la_familia) {
    auto v = def("veneno", 6);
    Emisor e = lleno(v, 5, 0.0f);
    MOTOR_CHECK(e.tipo().colores.size() == 3 && e.buffer().vivas() == 6);
    std::uint16_t k = 0;
    fija(e, 0.0f, [&k](Particula& p) { p.variante = static_cast<std::uint16_t>(k++ % 3); });
    const auto out = proy(e);
    MOTOR_CHECK(out.size() == 6);
    for (std::size_t i = 0; i < out.size() && i < 6; ++i) MOTOR_CHECK(out[i].color == e.tipo().colores[i % 3]);
    for (const char* fam : {"piedras", "ceniza", "veneno"}) {   // suma=false de la familia, sin oscura
        Emisor f = lleno(def(fam, 3), 5, 0.0f);
        MOTOR_CHECK(!f.tipo().suma && f.buffer().vivas() > 0);
        for (const auto& x : proy(f)) MOTOR_CHECK(!x.suma);
    }
    Emisor g = lleno(def("agua", 3), 5, 0.0f);
    for (const auto& x : proy(g)) MOTOR_CHECK(x.suma);
}

MOTOR_TEST_CASE(alfa_cero_no_cuesta_quads) {
    Emisor e = lleno(def("veneno", 4), 5, 0.0f);
    EstiloEmisor s; s.alfaMax = 0.0f;
    std::vector<Game::Destello> out;
    const ResumenProyeccion r = proyectarParticulas(e, s, 100, out);
    MOTOR_CHECK(r.destellos == 4 && r.quads == 0 && r.descartados == 0 && quadsDeDestellos(out) == 0);
    std::vector<Game::Destello> a(2);
    a[0].forma = a[1].forma = Game::FormaDeParticula::Calavera;
    a[0].alfa = 0.0f; a[1].alfa = 0.5f;
    MOTOR_CHECK(quadsDeDestellos(a) == 34);
    std::vector<Game::Destello> o2;   // con quadsMax = 0 y alfa 0 nada se descarta (no cuesta quads)
    const ResumenProyeccion r2 = proyectarParticulas(e, s, 0, o2);
    MOTOR_CHECK(r2.descartados == 0 && r2.destellos == 4);
}

MOTOR_TEST_CASE(tope_exacto_y_aro_atomico) {
    Emisor e = lleno(def("veneno", 10), 5, 0.0f);
    MOTOR_CHECK(e.buffer().vivas() == 10);
    std::vector<Game::Destello> o1, o2;
    const ResumenProyeccion r1 = proyectarParticulas(e, {}, 340, o1);   // suma exacta: todo entra
    MOTOR_CHECK(r1.destellos == 10 && r1.quads == 340 && r1.descartados == 0);
    const ResumenProyeccion r2 = proyectarParticulas(e, {}, 339, o2);
    MOTOR_CHECK(r2.destellos == 9 && r2.quads == 306 && r2.descartados == 1);
    // aro: 3 particulas Expande, tope a 1,5 aros -> un aro entero, los otros dos (20 destellos) descartados
    Emisor eo = lleno(def("ondas", 3), 30, 0.3f);
    MOTOR_CHECK(eo.buffer().vivas() == 3);
    const std::size_t aro = 10 * Game::pixelesDeForma(Game::FormaDeParticula::Punto).size();
    std::vector<Game::Destello> o3;
    const ResumenProyeccion r3 = proyectarParticulas(eo, {}, aro + aro / 2, o3);
    MOTOR_CHECK(r3.destellos == 10 && o3.size() == 10 && r3.quads == aro && r3.descartados == 20);
    std::vector<Game::Destello> o4;
    const ResumenProyeccion r4 = proyectarParticulas(eo, {}, 3 * aro, o4);
    MOTOR_CHECK(r4.destellos == 30 && r4.descartados == 0);
}

MOTOR_TEST_CASE(formas_gira_otraforma_y_trazo_de_salpica) {
    using F = Game::FormaDeParticula;
    const std::array<std::pair<const char*, std::pair<F, F>>, 2> giran{{
        {"monedas", {F::Moneda, F::MonedaDeCanto}}, {"engranajes", {F::Engranaje, F::EngranajeGirado}}}};
    for (const auto& par : giran) {   // alternan su otra cara cada 0.12 s
        Emisor e = lleno(def(par.first, 2), 30, 0.0f);
        auto sin = [](Particula& p) { p.otraForma = 0; };
        fija(e, 0.13f, sin);
        for (const auto& x : proy(e)) MOTOR_CHECK(x.forma == par.second.second);
        fija(e, 0.05f, sin);
        for (const auto& x : proy(e)) MOTOR_CHECK(x.forma == par.second.first);
        fija(e, 0.25f, sin);   // floor(0.25/0.12) = 2: otra vez la cara normal
        for (const auto& x : proy(e)) MOTOR_CHECK(x.forma == par.second.first);
    }
    Emisor m = lleno(def("monedas", 2), 30, 0.0f);   // otraForma=1 usa tipo.otraForma (Estrella)
    fija(m, 0.05f, [](Particula& p) { p.otraForma = 1; });
    for (const auto& x : proy(m)) MOTOR_CHECK(x.forma == F::Estrella);
    // trazo de chispas (Salpica, vy=-24, vx=10; vida 1 -> g = 48): hacia donde vuela, con vx0/vy0
    Emisor c = lleno(def("chispas", 2), 30, 0.0f);
    struct Caso { float vx0, vy0, edad; F esperada; };
    const std::array<Caso, 5> casos{{{50, 0, 0.2f, F::TrazoNE}, {50, 0, 0.9f, F::TrazoSE}, {-50, 0, 0.2f, F::TrazoNO},
                                     {-50, 0, 0.9f, F::TrazoSO}, {50, -100, 0.9f, F::TrazoNE}}};   // vy0: sin el seria SE
    for (const Caso& k : casos) {
        fija(c, k.edad, [&k](Particula& p) { p.vx0 = k.vx0; p.vy0 = k.vy0; p.lado = 0.0f; p.otraForma = 0; });
        const auto v = proy(c);
        MOTOR_CHECK(!v.empty());
        for (const auto& x : v) MOTOR_CHECK(x.forma == k.esperada);
    }
    fija(c, 0.2f, [](Particula& p) { p.vx0 = -5.0f; p.vy0 = 0.0f; p.lado = 0.0f; p.otraForma = 0; });   // vx0 manda
    for (const auto& x : proy(c)) MOTOR_CHECK(x.forma == F::TrazoNO);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
