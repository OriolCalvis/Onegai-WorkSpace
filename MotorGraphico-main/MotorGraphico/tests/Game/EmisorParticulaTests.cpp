// U4-07.E04 — tests de mezclarSemilla, uniforme01 y crearParticula (9 sorteos en orden fijo). GL-free.
#include "Game/Emisor/Particula.h"

#include "TestHarness.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <type_traits>
#include <vector>

using namespace Game::Emisor;
using Game::TipoDeParticula;

namespace {

constexpr double kPi = 3.14159265358979323846;

DefinicionEmisor defBase() {
    DefinicionEmisor d;
    d.familia = "agua";
    d.vidaMin = 0.2; d.vidaMax = 0.9;
    d.velocidadMin = 10.0; d.velocidadMax = 80.0;
    d.anguloCentro = -90.0; d.abanico = 60.0;
    d.maximo = 16; d.semilla = 1234;
    d.contraste = 0.5f;
    return d;
}

TipoDeParticula tipoBase() {
    TipoDeParticula t;
    t.colores = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {9, 9, 9}};
    t.conOtraForma = 0.5f;
    return t;
}

// Generador de guion: devuelve los valores dados por roll_in_range (0..999999) y cuenta las llamadas.
class AzarGuion : public RPG::RandomEngine {
public:
    explicit AzarGuion(std::vector<int> v) : valores(std::move(v)) {}
    int roll_d6() override { return 1; }
    int roll_in_range(int, int) override { return pos < valores.size() ? valores[pos++] : (++pos, 0); }
    std::uint64_t seed() const override { return 0; }
    std::vector<int> valores;
    std::size_t pos = 0;
};

bool igualesBits(const Particula& a, const Particula& b) { return std::memcmp(&a, &b, sizeof(Particula)) == 0; }

std::vector<Particula> tanda(std::uint64_t semilla, int n, const DefinicionEmisor& d, const TipoDeParticula& t) {
    auto azar = crearAzarEmisor(semilla);
    std::vector<Particula> v;
    for (int i = 0; i < n; ++i) v.push_back(crearParticula(d, t, *azar));
    return v;
}

double anguloGrados(const Particula& p) { return std::atan2(p.vy0, p.vx0) * 180.0 / kPi; }

MOTOR_TEST_CASE(mezclar_semilla_valores_de_referencia_splitmix64) {
    MOTOR_CHECK(mezclarSemilla(0) == 0xE220A8397B1DCDAFULL);
    MOTOR_CHECK(mezclarSemilla(1) == 0x910A2DEC89025CC1ULL);
    MOTOR_CHECK(mezclarSemilla(0xFFFFFFFFFFFFFFFFULL) != mezclarSemilla(0));
    // semillas consecutivas: muchos bits distintos (avalancha), nunca iguales
    for (std::uint64_t s = 0; s < 200; ++s) {
        const std::uint64_t x = mezclarSemilla(s) ^ mezclarSemilla(s + 1);
        int bits = 0;
        for (int i = 0; i < 64; ++i) bits += static_cast<int>((x >> i) & 1u);
        MOTOR_CHECK(bits >= 16 && bits <= 48);
    }
}

MOTOR_TEST_CASE(uniforme01_en_pasos_de_1e6_y_nunca_1) {
    AzarGuion g({0, 999999, 500000, 1});
    MOTOR_CHECK(uniforme01(g) == 0.0);
    MOTOR_CHECK(uniforme01(g) == 0.999999);
    MOTOR_CHECK(uniforme01(g) == 0.5);
    MOTOR_CHECK(uniforme01(g) == 1e-6);
    RPG::Xoroshiro128p r(77);
    for (int i = 0; i < 100000; ++i) {
        const double u = uniforme01(r);
        MOTOR_CHECK(u >= 0.0 && u < 1.0);
        const double paso = u * 1e6;
        MOTOR_CHECK(std::fabs(paso - std::round(paso)) < 1e-6);
    }
}

MOTOR_TEST_CASE(misma_semilla_misma_secuencia_bit_a_bit_y_otra_distinta) {
    const auto d = defBase();
    const auto t = tipoBase();
    const auto a = tanda(42, 1000, d, t), b = tanda(42, 1000, d, t), c = tanda(43, 1000, d, t);
    int distintas = 0;
    for (int i = 0; i < 1000; ++i) {
        MOTOR_CHECK(igualesBits(a[i], b[i]));
        if (a[i].vida != c[i].vida) ++distintas;
    }
    MOTOR_CHECK(distintas >= 900);
    // semillas 1 y 2 (guarda R3): >= 90% de las particulas difieren en vida
    const auto s1 = tanda(1, 1000, d, t), s2 = tanda(2, 1000, d, t);
    int dif = 0;
    for (int i = 0; i < 1000; ++i) dif += s1[i].vida != s2[i].vida ? 1 : 0;
    MOTOR_CHECK(dif >= 900);
}

MOTOR_TEST_CASE(azar_del_emisor_es_mezcla_mas_cuatro_descartes) {
    auto azar = crearAzarEmisor(5);
    RPG::Xoroshiro128p ref(mezclarSemilla(5));
    for (int i = 0; i < kSorteosDescartados; ++i) uniforme01(ref);
    for (int i = 0; i < 50; ++i) MOTOR_CHECK(uniforme01(*azar) == uniforme01(ref));
    // sin mezclarSemilla: Xoroshiro(semilla) pelada daria otra secuencia
    RPG::Xoroshiro128p cruda(5);
    for (int i = 0; i < kSorteosDescartados; ++i) uniforme01(cruda);
    auto azar3 = crearAzarEmisor(5);
    int iguales = 0;
    for (int i = 0; i < 20; ++i) iguales += uniforme01(cruda) == uniforme01(*azar3) ? 1 : 0;
    MOTOR_CHECK(iguales < 5);
    static_assert(!std::is_copy_assignable_v<RPG::Xoroshiro128p>, "R17: el generador no es asignable");
}

MOTOR_TEST_CASE(orden_de_sorteos_cada_uno_mueve_solo_su_campo) {
    const auto d = defBase();
    const auto t = tipoBase();
    const int M = 999999;
    AzarGuion base(std::vector<int>(9, 0));
    const Particula p0 = crearParticula(d, t, base);
    MOTOR_CHECK(base.pos == 9);
    MOTOR_CHECK(p0.vida == static_cast<float>(d.vidaMin) && p0.variante == 0 && p0.otraForma == 1 && p0.oscura == 1);
    MOTOR_CHECK(p0.ang0 == 0.0f && p0.radio0 == 6.0f && p0.lado == -1.0f);
    MOTOR_CHECK(std::fabs(std::hypot(p0.vx0, p0.vy0) - 10.0) < 1e-3 && std::fabs(anguloGrados(p0) - (-120.0)) < 1e-3);
    for (int i = 0; i < 9; ++i) {
        std::vector<int> v(9, 0);
        v[i] = M;
        AzarGuion g(v);
        const Particula p = crearParticula(d, t, g);
        MOTOR_CHECK(g.pos == 9);
        MOTOR_CHECK((p.vida != p0.vida) == (i == 0));
        MOTOR_CHECK((std::fabs(std::hypot(p.vx0, p.vy0) - 10.0) > 1.0) == (i == 1));
        MOTOR_CHECK((std::fabs(anguloGrados(p) - (-120.0)) > 1.0) == (i == 2));
        MOTOR_CHECK((p.variante != p0.variante) == (i == 3));
        MOTOR_CHECK((p.otraForma != p0.otraForma) == (i == 4));
        MOTOR_CHECK((p.ang0 != p0.ang0) == (i == 5));
        MOTOR_CHECK((p.radio0 != p0.radio0) == (i == 6));
        MOTOR_CHECK((p.oscura != p0.oscura) == (i == 7));
        MOTOR_CHECK((p.lado != p0.lado) == (i == 8));
    }
    // valores concretos de cada sorteo extremo
    AzarGuion g1(std::vector<int>{M, 0, 0, 0, 0, 0, 0, 0, 0});
    MOTOR_CHECK(std::fabs(crearParticula(d, t, g1).vida - 0.9f) < 1e-5f);
    AzarGuion g4(std::vector<int>{0, 0, 0, M, M, M, M, M, M});
    const Particula p4 = crearParticula(d, t, g4);
    MOTOR_CHECK(p4.variante == 3 && p4.otraForma == 0 && p4.oscura == 0);
    MOTOR_CHECK(std::fabs(p4.ang0 - 2.0f * static_cast<float>(kPi)) < 1e-4f && std::fabs(p4.radio0 - 10.0f) < 1e-4f);
    MOTOR_CHECK(std::fabs(p4.lado - 1.0f) < 1e-5f && p4.lado < 1.0f);
}

MOTOR_TEST_CASE(siempre_nueve_sorteos_aunque_no_hagan_falta) {
    auto d = defBase();
    auto t = tipoBase();
    d.contraste = 0.0f;               // oscura no hace falta
    t.colores = {{1, 2, 3}};          // variante no hace falta
    t.conOtraForma = 0.0f;            // otraForma no hace falta
    auto a = crearAzarEmisor(9), ref = crearAzarEmisor(9);
    crearParticula(d, t, *a);
    for (int i = 0; i < kSorteosPorParticula; ++i) uniforme01(*ref);
    // la particula siguiente coincide con la de un generador de referencia que se salto exactamente 9
    MOTOR_CHECK(igualesBits(crearParticula(d, t, *a), crearParticula(d, t, *ref)));
    t.colores.clear();
    auto b = crearAzarEmisor(9), r2 = crearAzarEmisor(9);
    crearParticula(d, t, *b);
    for (int i = 0; i < kSorteosPorParticula; ++i) uniforme01(*r2);
    MOTOR_CHECK(igualesBits(crearParticula(d, t, *b), crearParticula(d, t, *r2)));
    AzarGuion g(std::vector<int>(30, 5));
    crearParticula(d, t, g);
    MOTOR_CHECK(g.pos == 9);
}

MOTOR_TEST_CASE(rangos_en_diez_mil_particulas) {
    auto d = defBase();
    const auto t = tipoBase();
    const auto v = tanda(7, 10000, d, t);
    for (const auto& p : v) {
        MOTOR_CHECK(p.vida >= static_cast<float>(d.vidaMin) && p.vida <= static_cast<float>(d.vidaMax));
        const double vel = std::hypot(p.vx0, p.vy0);
        MOTOR_CHECK(vel >= d.velocidadMin - 1e-3 && vel <= d.velocidadMax + 1e-3);
        const double ang = anguloGrados(p);
        MOTOR_CHECK(ang >= -90.0 - 30.0 - 1e-3 && ang <= -90.0 + 30.0 + 1e-3);
        MOTOR_CHECK(p.variante < t.colores.size() && p.otraForma <= 1 && p.oscura <= 1);
        MOTOR_CHECK(p.lado >= -1.0f && p.lado <= 1.0f);
        MOTOR_CHECK(p.x == 0.0f && p.y == 0.0f && p.edad == 0.0f && p.k == 0);
        MOTOR_CHECK(p.ang0 >= 0.0f && p.ang0 < 6.2832f && p.radio0 >= 6.0f && p.radio0 <= 10.0f);
    }
}

MOTOR_TEST_CASE(casos_borde_de_abanico_velocidad_y_vida) {
    auto d = defBase();
    const auto t = tipoBase();
    d.abanico = 0.0; d.anguloCentro = -90.0;
    for (const auto& p : tanda(3, 500, d, t)) {
        MOTOR_CHECK(std::fabs(p.vx0) < 1e-3f && p.vy0 < 0.0f);     // arriba exacto
        MOTOR_CHECK(std::fabs(anguloGrados(p) - (-90.0)) < 1e-3);
    }
    d.abanico = 360.0; d.anguloCentro = 0.0; d.velocidadMin = d.velocidadMax = 50.0; d.vidaMin = d.vidaMax = 0.4;
    bool arriba = false, abajo = false, izq = false, der = false;
    for (const auto& p : tanda(4, 2000, d, t)) {
        MOTOR_CHECK(std::fabs(std::hypot(p.vx0, p.vy0) - 50.0) < 1e-3);
        MOTOR_CHECK(p.vida == 0.4f);
        arriba |= p.vy0 < -40; abajo |= p.vy0 > 40; izq |= p.vx0 < -40; der |= p.vx0 > 40;
    }
    MOTOR_CHECK(arriba && abajo && izq && der);
    d.abanico = 90.0; d.anguloCentro = 180.0;   // izquierda +-45
    for (const auto& p : tanda(5, 500, d, t)) MOTOR_CHECK(p.vx0 < 0.0f);
}

MOTOR_TEST_CASE(contraste_otraforma_y_variante) {
    auto d = defBase();
    auto t = tipoBase();
    d.contraste = 0.0f;
    for (const auto& p : tanda(11, 2000, d, t)) MOTOR_CHECK(p.oscura == 0);
    d.contraste = 1.0f;
    for (const auto& p : tanda(11, 2000, d, t)) MOTOR_CHECK(p.oscura == 1);
    d.contraste = 0.3f;
    int osc = 0;
    for (const auto& p : tanda(11, 10000, d, t)) osc += p.oscura;
    MOTOR_CHECK(osc > 2700 && osc < 3300);
    t.conOtraForma = 0.0f;
    for (const auto& p : tanda(12, 2000, d, t)) MOTOR_CHECK(p.otraForma == 0);
    t.conOtraForma = 1.0f;
    for (const auto& p : tanda(12, 2000, d, t)) MOTOR_CHECK(p.otraForma == 1);
    t.colores.clear();
    for (const auto& p : tanda(13, 500, d, t)) MOTOR_CHECK(p.variante == 0);
    t.colores = {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}};
    int cuenta[3] = {0, 0, 0};
    for (const auto& p : tanda(13, 3000, d, t)) ++cuenta[p.variante];
    MOTOR_CHECK(cuenta[0] > 800 && cuenta[1] > 800 && cuenta[2] > 800);
}

MOTOR_TEST_CASE(dos_emisores_con_semillas_distintas_no_comparten_estado) {
    const auto d = defBase();
    const auto t = tipoBase();
    auto a = crearAzarEmisor(1), b = crearAzarEmisor(2), a2 = crearAzarEmisor(1);
    const Particula pa = crearParticula(d, t, *a);
    for (int i = 0; i < 50; ++i) crearParticula(d, t, *b);   // b avanza; no debe afectar a a
    MOTOR_CHECK(igualesBits(pa, crearParticula(d, t, *a2)));
    MOTOR_CHECK(igualesBits(crearParticula(d, t, *a), crearParticula(d, t, *a2)));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
