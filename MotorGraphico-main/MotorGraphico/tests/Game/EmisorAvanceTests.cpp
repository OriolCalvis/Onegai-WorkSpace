// U4-07.E05 — avanzarEmisor: dt, recorte, duracion y corte, escalaTasa, k, determinismo y poda. GL-free.
#include "Game/Emisor/Emisor.h"

#include "TestHarness.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <type_traits>
#include <vector>

using namespace Game::Emisor;

static_assert(std::is_move_constructible_v<Emisor>, "el Emisor se mueve");
static_assert(std::is_move_assignable_v<Emisor>, "poda con remove_if+erase (R17)");
static_assert(!std::is_copy_constructible_v<Emisor>, "el generador es unico: no se copia");
static_assert(!std::is_copy_assignable_v<Emisor>, "el generador es unico: no se copia");
static_assert(std::is_nothrow_move_constructible_v<Emisor> && std::is_nothrow_move_assignable_v<Emisor>,
              "vector<Emisor> mueve al crecer solo si es noexcept");

namespace {

constexpr double kTick = 1.0 / 60.0;

DefinicionEmisor polvo(double tasa = 40.0, double duracion = 0.15, bool cortar = false) {
    DefinicionEmisor d;
    d.familia = "polvo";
    d.tasa = tasa; d.duracion = duracion; d.cortar = cortar;
    d.vidaMin = 0.3; d.vidaMax = 0.5;
    d.velocidadMin = 3.0; d.velocidadMax = 8.0;
    d.anguloCentro = -90.0; d.abanico = 140.0;
    d.maximo = 32; d.semilla = 7;
    return d;
}

PasoParticulas paso(Emisor& e, double dt, double escala, PresupuestoParticulas& p) {
    auto r = avanzarEmisor(e, dt, escala, p);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

std::uint64_t hashDe(const Emisor& e) {   // FNV-1a sobre los bits de las particulas (Particula no tiene relleno)
    std::uint64_t h = 1469598103934665603ULL;
    for (const auto& q : e.buffer().datos()) {
        unsigned char b[sizeof(Particula)];
        std::memcpy(b, &q, sizeof(Particula));
        for (unsigned char c : b) h = (h ^ c) * 1099511628211ULL;
    }
    return h;
}

struct Foto {
    double tiempo; std::uint64_t nacidas, hash; std::size_t vivas, presupuesto;
    bool operator==(const Foto& o) const {
        return tiempo == o.tiempo && nacidas == o.nacidas && hash == o.hash && vivas == o.vivas &&
               presupuesto == o.presupuesto;
    }
};
Foto foto(const Emisor& e, const PresupuestoParticulas& p) {
    return {e.tiempo(), e.nacidas(), hashDe(e), e.buffer().vivas(), p.vivas};
}

// Recuento de nacimientos de un emisor con `n` ticks de `dt`.
std::uint64_t nacidasTrasTicks(DefinicionEmisor d, int n, double dt, double escala) {
    Emisor e(d);
    PresupuestoParticulas p;
    for (int i = 0; i < n; ++i) paso(e, dt, escala, p);
    return e.nacidas();
}

MOTOR_TEST_CASE(buffer_aforo_fijo_y_compactacion_estable) {
    BufferParticulas b(4);
    const auto cap = b.datos().capacity();
    MOTOR_CHECK(cap >= 4 && b.aforo() == 4 && !b.llena() && b.vivas() == 0);
    for (int i = 0; i < 6; ++i) {
        Particula q; q.vida = 1.0f; q.k = static_cast<std::uint16_t>(i);
        q.edad = (i % 2 == 0) ? 0.0f : 1.0f;   // las impares estan caducadas
        b.anade(q);
    }
    MOTOR_CHECK(b.vivas() == 4 && b.llena() && b.datos().capacity() == cap);   // anade sobre aforo: ignorado
    MOTOR_CHECK(b.compacta() == 2);   // k=1 y k=3 salen
    MOTOR_CHECK(b.vivas() == 2 && b.datos()[0].k == 0 && b.datos()[1].k == 2);
    MOTOR_CHECK(b.vacia() == 2 && b.vivas() == 0 && b.datos().capacity() == cap);
}

MOTOR_TEST_CASE(dt_invalido_es_error_y_el_emisor_no_cambia) {
    Emisor e(polvo(100.0, 0.0));
    PresupuestoParticulas p;
    for (int i = 0; i < 5; ++i) paso(e, kTick, 1.0, p);
    MOTOR_CHECK(e.buffer().vivas() > 0);
    const Foto antes = foto(e, p);
    const double inf = std::numeric_limits<double>::infinity(), nan = std::numeric_limits<double>::quiet_NaN();
    for (double dt : {nan, inf, -inf, -0.01, -1e-12}) MOTOR_CHECK(!avanzarEmisor(e, dt, 1.0, p).isOk());
    for (double es : {nan, inf, -0.5}) MOTOR_CHECK(!avanzarEmisor(e, kTick, es, p).isOk());
    MOTOR_CHECK(foto(e, p) == antes);
    DefinicionEmisor d = polvo(400.0, 0.0);
    Emisor f(d);
    MOTOR_CHECK(!avanzarEmisor(f, kTick, 2.0, p).isOk());   // 400*2 > kMaxTasa: Error, sin tocar nada
    MOTOR_CHECK(f.tiempo() == 0.0 && f.nacidas() == 0 && p.vivas == antes.presupuesto);
}

MOTOR_TEST_CASE(dt_cero_no_cambia_nada) {
    Emisor e(polvo(100.0, 0.0));
    PresupuestoParticulas p;
    for (int i = 0; i < 5; ++i) paso(e, kTick, 1.0, p);
    const Foto antes = foto(e, p);
    for (int i = 0; i < 100; ++i) {
        const PasoParticulas r = paso(e, 0.0, 1.0, p);
        MOTOR_CHECK(r.nacidas == 0 && r.caducadas == 0 && r.rechazadas == 0 && !r.recortado);
    }
    MOTOR_CHECK(foto(e, p) == antes);   // D-O6: el polvo se congela con la pausa
}

MOTOR_TEST_CASE(recorte_a_kMaxPasoEmision_con_recortado) {
    DefinicionEmisor d = polvo(100.0, 0.0);
    d.vidaMin = d.vidaMax = 1.0;
    Emisor e(d);
    PresupuestoParticulas p;
    MOTOR_CHECK(!paso(e, 0.25, 1.0, p).recortado);   // justo el tope: sin recorte
    const PasoParticulas r = paso(e, 5.0, 1.0, p);
    MOTOR_CHECK(r.recortado);
    MOTOR_CHECK(e.tiempo() == 0.5);                  // 0,25 + 0,25: el sobrante se descarta
    MOTOR_CHECK(r.nacidas <= 100 * 0.25 + 1);
    for (const auto& q : e.buffer().datos()) MOTOR_CHECK(q.edad <= 0.25f);
}

// Emisor de un solo golpe: `duracion` 0,01 y tasa 500 -> 5 nacidas en el primer paso y ninguna despues.
DefinicionEmisor golpe(double vida) {
    DefinicionEmisor d = polvo(500.0, 0.01);
    d.vidaMin = d.vidaMax = vida;
    return d;
}

MOTOR_TEST_CASE(caducidad_exacta_y_recorte_en_casos_separados) {
    auto edadTras = [](double vida, int n, double dt, std::size_t* vivas, bool* recortado) {
        Emisor e(golpe(vida));
        PresupuestoParticulas p;
        paso(e, 0.01, 1.0, p);
        MOTOR_CHECK(e.buffer().vivas() == 5);
        for (int i = 0; i < n; ++i) *recortado = paso(e, dt, 1.0, p).recortado || *recortado;
        *vivas = e.buffer().vivas();
        return e.buffer().vivas() ? e.buffer().datos()[0].edad : -1.0f;
    };
    std::size_t v = 0; bool rec = false;
    MOTOR_CHECK(edadTras(1.0, 2, 0.5, &v, &rec) == 0.5f && v == 5 && rec);   // 0,5+0,5 -> cada uno recortado a 0,25
    rec = false;
    edadTras(1.0, 4, 0.25, &v, &rec);                                        // 4 x 0,25 -> edad 1,0: caduca
    MOTOR_CHECK(v == 0 && !rec);
    edadTras(0.2, 1, 0.25, &v, &rec);                                        // un paso que cruza la vida
    MOTOR_CHECK(v == 0);
    edadTras(1.0, 100, 0.0099, &v, &rec);                                    // ~0,99 en float: sobrevive
    MOTOR_CHECK(v == 5);
    edadTras(1.0, 102, 0.0099, &v, &rec);
    MOTOR_CHECK(v == 0);
}

MOTOR_TEST_CASE(duracion_y_escalaTasa_con_avanzarEmisor_real) {
    // polvo: tasa 40, duracion 0,15, ticks de 1/60 -> 6 por nube; con movimiento reducido (0,25) -> 1.
    MOTOR_CHECK(nacidasTrasTicks(polvo(), 30, kTick, 1.0) == 6);
    MOTOR_CHECK(nacidasTrasTicks(polvo(), 30, kTick, 0.25) == 1);
    MOTOR_CHECK(nacidasTrasTicks(polvo(), 30, kTick, 0.0) == 0);
    // duracion 0,15 con ticks de 0,1: 0,1 + 0,05 emitidos = 15 (no 20: el tick que cruza se acorta).
    DefinicionEmisor d = polvo(100.0, 0.15);
    d.maximo = 64;
    MOTOR_CHECK(nacidasTrasTicks(d, 10, 0.1, 1.0) == 15);
    // continuo (duracion 0): sin tope de emision
    d.duracion = 0.0;
    d.vidaMin = d.vidaMax = 0.1;
    MOTOR_CHECK(nacidasTrasTicks(d, 10, 0.1, 1.0) == 100);
}

MOTOR_TEST_CASE(corte_en_duracion_vacia_y_no_nace_nada) {
    DefinicionEmisor d = polvo(40.0, 0.15, true);
    d.vidaMin = d.vidaMax = 2.0;   // las vivas no caducan solas: solo el corte las retira
    Emisor e(d);
    PresupuestoParticulas p;
    for (int i = 1; i <= 8; ++i) paso(e, kTick, 1.0, p);
    MOTOR_CHECK(e.buffer().vivas() == 5 && p.vivas == 5 && !e.acabado());
    const PasoParticulas r = paso(e, kTick, 1.0, p);   // tick 9: tiempo pasa a 0,15
    MOTOR_CHECK(e.buffer().vivas() == 0 && p.vivas == 0);
    MOTOR_CHECK(r.nacidas == 0 && r.caducadas == 5 && e.nacidas() == 5 && e.acabado());
    paso(e, kTick, 1.0, p);
    MOTOR_CHECK(e.buffer().vivas() == 0 && e.nacidas() == 5);   // nada mas nace

    // sin cortar: las vivas se agotan visibles y solo entonces esta acabado
    d.cortar = false;
    Emisor f(d);
    PresupuestoParticulas q;
    for (int i = 1; i <= 9; ++i) paso(f, kTick, 1.0, q);
    MOTOR_CHECK(f.buffer().vivas() == 6 && !f.acabado());
    for (int i = 0; i < 9; ++i) paso(f, 0.25, 1.0, q);
    MOTOR_CHECK(f.buffer().vivas() == 0 && q.vivas == 0 && f.acabado() && f.nacidas() == 6);
}

MOTOR_TEST_CASE(k_es_nacidas_modulo_65536_y_no_se_desborda) {
    DefinicionEmisor d = polvo(500.0, 0.0);
    d.vidaMin = d.vidaMax = 0.1;
    d.maximo = 256;
    Emisor e(d);
    PresupuestoParticulas p;
    for (int i = 0; i < 700; ++i) {   // 700 x 0,25 s = 175 s a 500/s (> 131 s) -> 87 500 nacimientos
        paso(e, 0.25, 1.0, p);
        const auto& v = e.buffer().datos();
        MOTOR_CHECK(!v.empty());
        MOTOR_CHECK(v.back().k == static_cast<std::uint16_t>((e.nacidas() - 1) % 65536));
        MOTOR_CHECK(std::isfinite(v.back().x) && std::isfinite(v.back().y));
    }
    MOTOR_CHECK(e.nacidas() > 65536 + 1000);
    MOTOR_CHECK(sizeof(Particula) == 44);
}

MOTOR_TEST_CASE(determinismo_con_semilla) {
    auto corre = [](std::uint64_t semilla) {
        DefinicionEmisor d = polvo(300.0, 0.0);
        d.semilla = semilla;
        Emisor e(d);
        PresupuestoParticulas p;
        for (int i = 0; i < 600; ++i) paso(e, kTick, 1.0, p);
        return hashDe(e);
    };
    MOTOR_CHECK(corre(5) == corre(5));
    MOTOR_CHECK(corre(5) != corre(6));
    MOTOR_CHECK(corre(5) != 1469598103934665603ULL);   // el buffer no estaba vacio
}

MOTOR_TEST_CASE(capacidad_constante_y_orden_estable_tras_caducar) {
    DefinicionEmisor d = polvo(200.0, 0.0);
    d.vidaMin = 0.05; d.vidaMax = 0.6;
    d.maximo = 40;
    Emisor e(d);
    PresupuestoParticulas p;
    const auto cap = e.buffer().datos().capacity();
    MOTOR_CHECK(cap >= 40);
    bool huboCaducadas = false;
    for (int i = 0; i < 300; ++i) {
        huboCaducadas = paso(e, kTick, 1.0, p).caducadas > 0 || huboCaducadas;
        MOTOR_CHECK(e.buffer().datos().capacity() == cap && e.buffer().vivas() <= 40 && p.vivas == e.buffer().vivas());
        std::uint64_t anterior = 0; bool primera = true;
        for (const auto& q : e.buffer().datos()) {   // orden de nacimiento: k crece (sin dar la vuelta en 300 ticks)
            MOTOR_CHECK(primera || q.k > anterior);
            anterior = q.k; primera = false;
        }
    }
    MOTOR_CHECK(huboCaducadas);
}

MOTOR_TEST_CASE(poda_de_vector_conserva_el_generador_de_los_supervivientes) {
    auto crea = [](std::vector<Emisor>& v) {
        for (std::uint64_t s = 1; s <= 6; ++s) {
            DefinicionEmisor d = polvo(200.0, 0.0);
            d.semilla = s;
            v.emplace_back(d, Vector2{float(s), 0.0f}, GrupoDeDibujo{});
        }
    };
    std::vector<Emisor> real, ref;
    crea(real); crea(ref);
    PresupuestoParticulas pr, pf;
    for (int i = 0; i < 5; ++i)
        for (std::size_t j = 0; j < real.size(); ++j) { paso(real[j], kTick, 1.0, pr); paso(ref[j], kTick, 1.0, pf); }
    // poda: fuera las semillas impares (remove_if + erase mueve a los supervivientes)
    real.erase(std::remove_if(real.begin(), real.end(), [](const Emisor& e) { return e.definicion().semilla % 2 == 1; }),
               real.end());
    pr.recuenta(real);
    MOTOR_CHECK(real.size() == 3);
    for (int i = 0; i < 5; ++i) {
        for (std::size_t j = 0; j < real.size(); ++j) paso(real[j], kTick, 1.0, pr);
        for (std::size_t j = 1; j < ref.size(); j += 2) paso(ref[j], kTick, 1.0, pf);
    }
    for (std::size_t j = 0; j < real.size(); ++j) {
        MOTOR_CHECK(real[j].definicion().semilla == 2 * (j + 1));
        MOTOR_CHECK(hashDe(real[j]) == hashDe(ref[2 * j + 1]) && real[j].nacidas() == ref[2 * j + 1].nacidas());
    }
    // asignacion por movimiento entre emisores vivos
    real[0] = std::move(real[2]);
    MOTOR_CHECK(real[0].definicion().semilla == 6);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
