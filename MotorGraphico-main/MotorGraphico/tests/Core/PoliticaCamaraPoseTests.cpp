#include "Core/Math/PoliticaCamara.h"

#include "TestHarness.h"

#include <cmath>
#include <cstring>
#include <limits>

namespace {

using PoliticaCamara::LimitesMapa;
using PoliticaCamara::PoliticaPose;
using PoliticaCamara::PoseCamara;
using PoliticaCamara::TamanoVista;
using PoliticaCamara::componerPose;
using PoliticaCamara::desplazamientoSacudida;
using PoliticaCamara::limitarAlMapa;
using PoliticaCamara::limitesDeMapa;

const float kNaN = std::numeric_limits<float>::quiet_NaN();
const TamanoVista kVista{800, 600};
const Vector2 kSin{0.0f, 0.0f};

bool cerca(float a, float b, float tol = 1e-3f) { return std::fabs(a - b) <= tol; }
bool mismosBits(const Vector2& a, const Vector2& b) { return std::memcmp(&a, &b, sizeof(Vector2)) == 0; }

LimitesMapa limites(int c, int f, float margen = 0.0f) {
    const auto r = limitesDeMapa(c, f, 64, 32, margen);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

PoseCamara pose(Vector2 pos, float zoom, Vector2 sac, const LimitesMapa* l, PoliticaPose p = {}) {
    const auto r = componerPose(pos, zoom, sac, kVista, l, p);
    MOTOR_CHECK(r.isOk());
    return r.value();
}

// Casos numericos de E05 del contrato (mapa 64x64, tiles 64x32), sin sacudida: base == vista.
MOTOR_TEST_CASE(casos_numericos_del_contrato_sin_sacudida) {
    const LimitesMapa m = limites(64, 64);
    struct Caso { float cx, cy, z, ex, ey; };
    const Caso casos[] = {
        {-2000.0f, 1000.0f, 1.7f, -1780.706f, 1000.0f}, {0.0f, 0.0f, 1.0f, 0.0f, 300.0f},
        {-2000.0f, -50.0f, 1.0f, -1573.6f, 802.8f},     {2080.0f, 2048.0f, 3.0f, 1670.4f, 1228.8f},
        {-1616.0f, 1024.0f, 1.0f, -1616.0f, 1024.0f},   {-1616.001f, 1024.0f, 1.0f, -1616.0f, 1024.0f},
    };
    for (const Caso& c : casos) {
        const PoseCamara p = pose({c.cx, c.cy}, c.z, kSin, &m);
        MOTOR_CHECK(cerca(p.base.x, c.ex) && cerca(p.base.y, c.ey));
        MOTOR_CHECK(mismosBits(p.base, p.vista));
        MOTOR_CHECK(p.zoom == c.z);
    }
    // Identidad exacta (bit a bit) en el centro exacto min+mitad sobre el eje del rombo.
    MOTOR_CHECK(mismosBits(pose({-1616.0f, 1024.0f}, 1.0f, kSin, &m).base, Vector2{-1616.0f, 1024.0f}));
    // Mapa 3x2 y cambio de mapa: destino viejo (1500,1000) => (48,40).
    const LimitesMapa m32 = limites(3, 2);
    const PoseCamara p = pose({1500.0f, 1000.0f}, 1.0f, kSin, &m32);
    MOTOR_CHECK(cerca(p.base.x, 48.0f) && cerca(p.base.y, 40.0f));
    const PoseCamara q = pose({48.0f, 40.0f}, 1.0f, kSin, &m32);
    MOTOR_CHECK(cerca(q.base.x, 48.0f) && cerca(q.base.y, 40.0f));
}

MOTOR_TEST_CASE(sin_limites_es_base_mas_sacudida_exacta) {
    for (float z : {1.0f, 1.7f, 2.0f}) {
        const Vector2 pos{100.0f, 200.0f}, sac{8.0f, -4.8f};
        const PoseCamara p = pose(pos, z, sac, nullptr);
        MOTOR_CHECK(mismosBits(p.base, pos));
        MOTOR_CHECK(p.vista.x == static_cast<float>(100.0 + 8.0f / static_cast<double>(z)));
        MOTOR_CHECK(p.vista.y == static_cast<float>(200.0 + -4.8f / static_cast<double>(z)));
    }
}

MOTOR_TEST_CASE(division_por_zoom_con_limites_holgados) {
    const LimitesMapa m = limites(64, 64);
    const Vector2 pos{0.0f, 1000.0f};  // dentro de caja y rombo con holgura
    const PoseCamara a = pose(pos, 2.0f, {10.0f, 6.0f}, &m);
    MOTOR_CHECK(cerca(a.vista.x, 5.0f) && cerca(a.vista.y, 1003.0f));
    const PoseCamara b = pose(pos, 1.7f, {17.0f, -8.5f}, &m);
    MOTOR_CHECK(cerca(b.vista.x, 10.0f) && cerca(b.vista.y, 995.0f));
    MOTOR_CHECK(mismosBits(a.base, pos) && mismosBits(b.base, pos));
}

MOTOR_TEST_CASE(sacudida_que_sacaria_del_mapa_se_recorta_en_la_vista_final) {
    const LimitesMapa m = limites(64, 64);
    // Base pegada al borde izquierdo (-1616,1024) con zoom 1: empujar hacia fuera no se ve.
    const PoseCamara fuera = pose({-1616.0f, 1024.0f}, 1.0f, {-50.0f, 0.0f}, &m);
    MOTOR_CHECK(mismosBits(fuera.base, Vector2{-1616.0f, 1024.0f}));
    MOTOR_CHECK(cerca(fuera.vista.x, -1616.0f) && cerca(fuera.vista.y, 1024.0f));
    // Hacia dentro si se mueve (unilateral).
    const PoseCamara dentro = pose({-1616.0f, 1024.0f}, 1.0f, {50.0f, 0.0f}, &m);
    MOTOR_CHECK(cerca(dentro.vista.x, -1566.0f));
    // La vista final cumple los limites: coincide con limitar(vista) y es idempotente.
    const Vector2 sacs[] = {{-300.0f, 40.0f}, {500.0f, -500.0f}, {-9.0f, 9.0f}};
    for (const Vector2& s : sacs) {
        for (float z : {1.0f, 1.7f, 2.0f}) {
            const PoseCamara p = pose({-2000.0f, -50.0f}, z, s, &m);
            const auto l = limitarAlMapa(p.vista, z, kVista, m);
            MOTOR_CHECK(l.isOk() && mismosBits(l.value(), p.vista));
        }
    }
    // Sacudida libre (alternativa B): la vista se sale de lo permitido.
    PoliticaPose libre;
    libre.sacudidaDentroDeLimites = false;
    const PoseCamara l = pose({-1616.0f, 1024.0f}, 1.0f, {-50.0f, 0.0f}, &m, libre);
    MOTOR_CHECK(cerca(l.vista.x, -1666.0f));
}

MOTOR_TEST_CASE(la_posicion_logica_no_se_modifica_y_no_hay_deriva) {
    const LimitesMapa m = limites(64, 64);
    const Vector2 pos{-1700.0f, 1000.0f};
    const Vector2 copia = pos;
    const PoseCamara primera = pose(pos, 1.7f, {3.0f, 1.0f}, &m);
    for (int i = 0; i < 10000; ++i) {
        const auto s = desplazamientoSacudida({8.0f, 100.0f}, i * 0.005, 9u);
        MOTOR_CHECK(s.isOk());
        const PoseCamara p = pose(pos, 1.7f, s.value(), &m);
        MOTOR_CHECK(mismosBits(p.base, primera.base));
    }
    MOTOR_CHECK(mismosBits(pos, copia));
}

MOTOR_TEST_CASE(idempotencia_con_sacudida_cero) {
    const LimitesMapa m = limites(64, 64);
    for (const Vector2& pos : {Vector2{-2000.0f, -50.0f}, Vector2{0.0f, 0.0f}, Vector2{500.0f, 900.0f}}) {
        const PoseCamara a = pose(pos, 1.7f, kSin, &m);
        MOTOR_CHECK(mismosBits(a.base, a.vista));
        const PoseCamara b = pose(a.base, 1.7f, kSin, &m);
        MOTOR_CHECK(mismosBits(a.base, b.base) && mismosBits(a.vista, b.vista));
    }
}

MOTOR_TEST_CASE(entradas_invalidas_dan_error) {
    const LimitesMapa m = limites(3, 2);
    const Vector2 p{0.0f, 0.0f};
    MOTOR_CHECK(!componerPose(p, 0.0f, kSin, kVista, &m).isOk());
    MOTOR_CHECK(!componerPose(p, -1.0f, kSin, kVista, nullptr).isOk());
    MOTOR_CHECK(!componerPose(p, kNaN, kSin, kVista, &m).isOk());
    MOTOR_CHECK(!componerPose({kNaN, 0.0f}, 1.0f, kSin, kVista, &m).isOk());
    MOTOR_CHECK(!componerPose(p, 1.0f, {0.0f, kNaN}, kVista, nullptr).isOk());
    MOTOR_CHECK(!componerPose(p, 1.0f, {std::numeric_limits<float>::infinity(), 0.0f}, kVista, &m).isOk());
    MOTOR_CHECK(!componerPose(p, 1.0f, kSin, TamanoVista{0, 600}, &m).isOk());
}

MOTOR_TEST_CASE(sacudida_con_tiempo_astronomico_da_error_sin_ub) {
    const double enorme = 4.0e17;  // *60 > 2^63
    MOTOR_CHECK(!desplazamientoSacudida({4.0f, 1.0e30f}, enorme, 1u).isOk());
    MOTOR_CHECK(desplazamientoSacudida({4.0f, 1.0e30f}, 1.0e17, 1u).isOk());
    MOTOR_CHECK(!desplazamientoSacudida({4.0f, 3.0e38f}, 2.0e38, 1u).isOk());
    // Fuera de duracion sigue siendo (0,0) exacto, no error.
    const auto z = desplazamientoSacudida({4.0f, 0.2f}, enorme, 1u);
    MOTOR_CHECK(z.isOk() && z.value().x == 0.0f && z.value().y == 0.0f);
}

// Continuidad (hallazgo Codex): |dvista| <= |dsacudida|/zoom + 1e-3 al barrer la sacudida de -1 a +1 px,
// y base identica, tambien en rangos colapsados (mapas estrechos 30x1 y 1x30).
void barrido(int c, int f, Vector2 pos, float zoom, bool ejeX) {
    const LimitesMapa m = limites(c, f);
    const PoseCamara p0 = pose(pos, zoom, kSin, &m);
    Vector2 prev{0.0f, 0.0f};
    for (int i = -1000; i <= 1000; ++i) {
        const float d = static_cast<float>(i) * 0.001f;
        const PoseCamara p = pose(pos, zoom, ejeX ? Vector2{d, 0.0f} : Vector2{0.0f, d}, &m);
        MOTOR_CHECK(mismosBits(p.base, p0.base));
        if (i > -1000) {
            const double paso = 0.001 / static_cast<double>(zoom) + 1e-3 + 1e-4;
            MOTOR_CHECK(std::fabs(p.vista.x - prev.x) <= paso && std::fabs(p.vista.y - prev.y) <= paso);
        }
        prev = p.vista;
    }
}

MOTOR_TEST_CASE(recorte_continuo_en_rangos_colapsados) {
    barrido(30, 1, {464.0f, 248.0f}, 1.0f, true);   // reproduccion exacta de Codex
    barrido(30, 1, {464.0f, 248.0f}, 1.0f, false);
    barrido(1, 30, {0.0f, 248.0f}, 1.0f, true);
    barrido(1, 30, {0.0f, 248.0f}, 1.0f, false);
    barrido(30, 1, {528.0f, 248.0f}, 1.0f, true);   // el otro borde del segmento
    // Rango casi colapsado (r2): zooms justo por encima del colapso en Y de 30x1.
    barrido(30, 1, {464.0f, 248.0f}, 1.2097f, true);
    barrido(30, 1, {464.0f, 248.0f}, 1.21f, true);
    barrido(30, 1, {464.0f, 248.0f}, 1.2097f, false);
    barrido(30, 1, {464.0f, 248.0f}, 1.21f, false);
    barrido(64, 64, {-1616.0f, 1024.0f}, 1.0f, true);
    barrido(64, 64, {-1616.0f, 1024.0f}, 1.7f, false);
    barrido(64, 64, {-1406.4f, 719.2f}, 1.0f, true);  // esquina caja-rombo
    // Valores exactos: ya no se vuelve al ancla.
    const LimitesMapa m = limites(30, 1);
    const PoseCamara a = pose({464.0f, 248.0f}, 1.0f, {-0.003f, 0.0f}, &m);
    MOTOR_CHECK(cerca(a.vista.x, 464.0f) && cerca(a.vista.y, 248.0f));
}

// Posicion logica FUERA de la caja con sacudida: se suma al punto ya limitado (la sacudida no se
// "come" por el recorte de la posicion logica ni se suma antes de limitar).
MOTOR_TEST_CASE(posicion_fuera_de_la_caja_con_sacudida) {
    const LimitesMapa m = limites(64, 64);
    const PoseCamara p = pose({-5000.0f, 300.0f}, 1.0f, {8.0f, 0.0f}, &m);
    MOTOR_CHECK(cerca(p.vista.x, -1608.0f) && cerca(p.vista.y, 824.0f));
}

}  // namespace

int main() { return motor_test::runAllTests(); }
