// U4-07.E08 — limite y repetibilidad del sistema de particulas (gl-free, sin glad). Solo mide: sin codigo de producto.
// Los tiempos de CPU (std::chrono) se IMPRIMEN como dato informativo; ninguna asercion depende del reloj y no se
// promete cifra de FPS. La medida real de dibujo (SpriteBatch::submit) queda como observacion manual en GL.
#include "Game/Emisor/PasosDePolvo.h"
#include "Game/Emisor/ProyeccionParticulas.h"

#include "TestHarness.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace Game::Emisor;
using Game::Destello;
using Game::FormaDeParticula;

namespace {

constexpr double kTick = 1.0 / 60.0;
using Reloj = std::chrono::steady_clock;

double msDesde(Reloj::time_point t0) { return std::chrono::duration<double, std::milli>(Reloj::now() - t0).count(); }

// ---- hash FNV-1a 64 de todo el estado relevante ----
struct Hash {
    std::uint64_t h = 1469598103934665603ull;
    void bytes(const void* p, std::size_t n) {
        const auto* b = static_cast<const unsigned char*>(p);
        for (std::size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ull; }
    }
    template <class T> void val(const T& v) { bytes(&v, sizeof(T)); }
};

// Particula (44 bytes sin relleno), tiempo, nacidas, vivas por emisor y presupuesto. El residuo de emision es
// privado: queda cubierto porque el hash se toma tambien tras seguir avanzando (un residuo distinto cambia los
// nacimientos siguientes).
std::uint64_t hashDe(const std::vector<Emisor>& v, const PresupuestoParticulas& p) {
    Hash h;
    for (const Emisor& e : v) {
        h.val(e.tiempo());
        h.val(e.nacidas());
        const std::size_t n = e.buffer().vivas();
        h.val(n);
        for (const Particula& q : e.buffer().datos()) h.bytes(&q, sizeof(Particula));
    }
    h.val(p.vivas);
    h.val(p.maximo);
    return h.h;
}

DefinicionEmisor defCarga(std::uint64_t semilla, const char* familia = "veneno") {
    DefinicionEmisor d;
    d.familia = familia;
    d.tasa = kMaxTasa;                     // 500 part/s
    d.duracion = 0.0;                      // continuo
    d.vidaMin = 0.8; d.vidaMax = 1.6;
    d.velocidadMin = 3.0; d.velocidadMax = 8.0;
    d.anguloCentro = -90.0; d.abanico = 140.0;
    d.maximo = kMaxPorEmisor;              // 256
    d.semilla = semilla;
    return d;
}

// Recuento INDEPENDIENTE de quadsDeDestellos: replica las tres pasadas de pintaDestellos (PintaDestellos.cpp):
// contorno de todo lo visible, luego cuerpo de lo que tapa y cuerpo de lo que suma; todo con alfa > 0.
std::size_t quadsComoPintaDestellos(const std::vector<Destello>& ds) {
    std::size_t n = 0;
    for (const Destello& d : ds)
        if (d.alfa > 0.0f) n += Game::contornoDeForma(d.forma).size();
    for (const bool suma : {false, true})
        for (const Destello& d : ds) {
            if (d.suma != suma || d.alfa <= 0.0f) continue;
            n += Game::pixelesDeForma(d.forma).size();
        }
    return n;
}

struct Medida {
    std::uint64_t hash = 0, hashTras = 0;
    std::size_t maxVivasEmisor = 0, maxVivasGlobal = 0, maxQuads = 0, maxDescartados = 0, maxDestellos = 0;
    long rechazadas = 0, nacidas = 0;
    bool desacuerdoQuads = false, sumaIncoherente = false, excedeQuads = false;
    std::vector<double> msAvance, msProyeccion;
};

// 64 emisores de 256 a tasa maxima, `pasos` ticks de 1/60. Proyecta cada tick (como el render) con el tope.
Medida corre(std::uint64_t base, int pasos, int emisores = kMaxEmisores, bool mide = false) {
    std::vector<Emisor> v;
    v.reserve(static_cast<std::size_t>(emisores));
    for (int i = 0; i < emisores; ++i) v.emplace_back(defCarga(base + static_cast<std::uint64_t>(i)));
    PresupuestoParticulas p;   // maximo = kMaxParticulasGlobal
    Medida m;
    std::vector<Destello> out;
    out.reserve(40000);
    const EstiloEmisor estilo;
    auto tick = [&](bool proyecta) {
        const auto t0 = Reloj::now();
        p.recuenta(v);
        for (Emisor& e : v) {
            auto r = avanzarEmisor(e, kTick, 1.0, p);
            MOTOR_CHECK(r.isOk());
            m.rechazadas += r.value().rechazadas;
            m.nacidas += r.value().nacidas;
        }
        if (mide) m.msAvance.push_back(msDesde(t0));
        std::size_t suma = 0;
        for (const Emisor& e : v) {
            m.maxVivasEmisor = std::max(m.maxVivasEmisor, e.buffer().vivas());
            suma += e.buffer().vivas();
        }
        m.maxVivasGlobal = std::max(m.maxVivasGlobal, p.vivas);
        if (suma != p.vivas) m.sumaIncoherente = true;
        if (!proyecta) return;
        const auto t1 = Reloj::now();
        out.clear();
        std::size_t quads = 0, descartados = 0;
        for (const Emisor& e : v) {
            const std::size_t resto = static_cast<std::size_t>(kMaxQuadsPorFotograma) - quads;
            const ResumenProyeccion r = proyectarParticulas(e, estilo, resto, out);
            quads += r.quads;
            descartados += r.descartados;
        }
        if (mide) m.msProyeccion.push_back(msDesde(t1));
        if (quadsDeDestellos(out) != quadsComoPintaDestellos(out) || quadsDeDestellos(out) != quads)
            m.desacuerdoQuads = true;
        if (quads > static_cast<std::size_t>(kMaxQuadsPorFotograma)) m.excedeQuads = true;
        m.maxQuads = std::max(m.maxQuads, quads);
        m.maxDescartados = std::max(m.maxDescartados, descartados);
        m.maxDestellos = std::max(m.maxDestellos, out.size());
    };
    for (int i = 0; i < pasos; ++i) tick(true);
    m.hash = hashDe(v, p);
    for (int i = 0; i < 30; ++i) tick(false);   // sigue avanzando: delata un residuo/estado interno distinto
    m.hashTras = hashDe(v, p);
    return m;
}

double mediana(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}
double maximo(const std::vector<double>& v) { return v.empty() ? 0.0 : *std::max_element(v.begin(), v.end()); }

MOTOR_TEST_CASE(dos_runs_misma_entrada_mismo_hash_y_otra_semilla_distinto) {
    const Medida a = corre(1000, 600);
    std::vector<int> basura(100003, 7);   // mueve el monton entre runs: nada puede depender de direcciones
    const Medida b = corre(1000, 600);
    MOTOR_CHECK(a.hash == b.hash && a.hashTras == b.hashTras);
    MOTOR_CHECK(a.nacidas == b.nacidas && a.rechazadas == b.rechazadas);
    const Medida c = corre(2000, 600);
    MOTOR_CHECK(c.hash != a.hash && c.hashTras != a.hashTras);
    MOTOR_CHECK(basura[5] == 7);
    std::printf("[E08] hash 600 pasos: 0x%016llx (semilla 1000), tras 30 mas: 0x%016llx\n",
                static_cast<unsigned long long>(a.hash), static_cast<unsigned long long>(a.hashTras));
}

MOTOR_TEST_CASE(emisores_independientes_el_rng_no_se_comparte) {
    // Mismos datos y semilla, avanzados en paralelo con otro emisor de semilla distinta en medio: iguales bit a bit.
    Emisor a(defCarga(5)), b(defCarga(5)), c(defCarga(6));
    PresupuestoParticulas pa, pb, pc;
    for (int i = 0; i < 120; ++i) {
        MOTOR_CHECK(avanzarEmisor(a, kTick, 1.0, pa).isOk());
        MOTOR_CHECK(avanzarEmisor(c, kTick, 1.0, pc).isOk());
        if (i % 3 == 0) MOTOR_CHECK(avanzarEmisor(c, kTick, 1.0, pc).isOk());
        MOTOR_CHECK(avanzarEmisor(b, kTick, 1.0, pb).isOk());
    }
    MOTOR_CHECK(a.buffer().vivas() == b.buffer().vivas() && a.buffer().vivas() > 0);
    MOTOR_CHECK(std::memcmp(a.buffer().datos().data(), b.buffer().datos().data(),
                            a.buffer().vivas() * sizeof(Particula)) == 0);
    MOTOR_CHECK(c.buffer().vivas() != a.buffer().vivas() ||
                std::memcmp(a.buffer().datos().data(), c.buffer().datos().data(),
                            a.buffer().vivas() * sizeof(Particula)) != 0);
}

MOTOR_TEST_CASE(carga_64_emisores_respeta_aforo_presupuesto_y_quads) {
    const Medida m = corre(1000, 600, kMaxEmisores, true);
    MOTOR_CHECK(m.maxVivasEmisor <= static_cast<std::size_t>(kMaxPorEmisor));
    MOTOR_CHECK(m.maxVivasGlobal <= static_cast<std::size_t>(kMaxParticulasGlobal));
    MOTOR_CHECK(m.maxVivasGlobal > static_cast<std::size_t>(kMaxParticulasGlobal) / 2);   // la prueba SI carga
    MOTOR_CHECK(!m.sumaIncoherente);
    MOTOR_CHECK(m.rechazadas > 0);
    MOTOR_CHECK(!m.excedeQuads && m.maxQuads <= static_cast<std::size_t>(kMaxQuadsPorFotograma));
    MOTOR_CHECK(m.maxDescartados > 0);                 // el tope de quads SI actua con esta carga
    MOTOR_CHECK(!m.desacuerdoQuads);                   // quadsDeDestellos == recuento independiente == resumen
    std::printf("[E08] carga 64x256 veneno 10 s: vivas/emisor max %zu, vivas global max %zu, nacidas %ld, rechazadas %ld\n",
                m.maxVivasEmisor, m.maxVivasGlobal, m.nacidas, m.rechazadas);
    std::printf("[E08] carga: destellos/fotograma max %zu, quads/fotograma max %zu (tope %d), descartados max %zu\n",
                m.maxDestellos, m.maxQuads, kMaxQuadsPorFotograma, m.maxDescartados);
    std::printf("[E08] CPU Debug por tick (informativo): avance mediana %.3f ms max %.3f ms; proyeccion mediana %.3f ms max %.3f ms\n",
                mediana(m.msAvance), maximo(m.msAvance), mediana(m.msProyeccion), maximo(m.msProyeccion));
}

MOTOR_TEST_CASE(aforo_de_un_emisor_con_pocos_emisores_llega_a_256) {
    // Con 4 emisores el presupuesto global (2048) no es el limite: lo es el aforo de cada uno (256).
    const Medida m = corre(1000, 300, 4);
    MOTOR_CHECK(m.maxVivasEmisor == static_cast<std::size_t>(kMaxPorEmisor));
    MOTOR_CHECK(m.maxVivasGlobal <= 4u * static_cast<std::size_t>(kMaxPorEmisor) && m.rechazadas > 0);
    MOTOR_CHECK(!m.excedeQuads && !m.desacuerdoQuads && !m.sumaIncoherente);
    std::printf("[E08] 4 emisores veneno: vivas/emisor max %zu, global max %zu, rechazadas %ld, quads max %zu, descartados max %zu\n",
                m.maxVivasEmisor, m.maxVivasGlobal, m.rechazadas, m.maxQuads, m.maxDescartados);
}

MOTOR_TEST_CASE(caso_peor_calavera_quads_contra_recuento_independiente) {
    // Ninguna forma de ninguna familia cuesta mas de 34 quads: la calavera es el caso peor.
    std::size_t peor = 0;
    for (const std::string& f : Game::familiasDeParticulas()) {
        const Game::TipoDeParticula t = Game::tipoDeParticula(f);
        for (const FormaDeParticula fo : {t.forma, t.otraForma})
            peor = std::max(peor, Game::pixelesDeForma(fo).size() + Game::contornoDeForma(fo).size());
    }
    MOTOR_CHECK(peor == 34);
    MOTOR_CHECK(Game::pixelesDeForma(FormaDeParticula::Calavera).size() + Game::contornoDeForma(FormaDeParticula::Calavera).size() == 34);
    std::vector<Destello> d;
    for (int i = 0; i < 240; ++i) d.push_back({0.0f, 0.0f, 1.0f, FormaDeParticula::Calavera, i % 2 == 0, {200, 255, 100}});
    MOTOR_CHECK(quadsDeDestellos(d) == 240u * 34u && quadsDeDestellos(d) == quadsComoPintaDestellos(d));
    MOTOR_CHECK(quadsDeDestellos(d) <= static_cast<std::size_t>(kMaxQuadsPorFotograma));
    d.push_back({0.0f, 0.0f, 1.0f, FormaDeParticula::Calavera, true, {1, 2, 3}});
    MOTOR_CHECK(quadsDeDestellos(d) == 241u * 34u && quadsDeDestellos(d) > static_cast<std::size_t>(kMaxQuadsPorFotograma));
    d.push_back({0.0f, 0.0f, 0.0f, FormaDeParticula::Calavera, true, {1, 2, 3}});   // alfa 0: coste 0
    MOTOR_CHECK(quadsDeDestellos(d) == 241u * 34u && quadsComoPintaDestellos(d) == 241u * 34u);
    // Con emisores "veneno" y el tope de la proyeccion: un solo emisor lleno (256) cabe o se recorta, nunca excede.
    std::vector<Emisor> v;
    for (int i = 0; i < 2; ++i) v.emplace_back(defCarga(40 + static_cast<std::uint64_t>(i)));
    PresupuestoParticulas p;
    for (int i = 0; i < 120; ++i)
        for (Emisor& e : v) MOTOR_CHECK(avanzarEmisor(e, kTick, 1.0, p).isOk());
    std::vector<Destello> out;
    std::size_t quads = 0;
    for (const Emisor& e : v) quads += proyectarParticulas(e, EstiloEmisor{}, kMaxQuadsPorFotograma - quads, out).quads;
    MOTOR_CHECK(quads <= static_cast<std::size_t>(kMaxQuadsPorFotograma) && quads == quadsDeDestellos(out));
    MOTOR_CHECK(quadsDeDestellos(out) == quadsComoPintaDestellos(out));
}

// Camina sin parar: un paso cada 0,16 s, con entradas y salidas de combate (D-O7) y con/sin movimiento reducido.
struct Camino {
    std::uint64_t hash = 0;
    std::size_t maxEmisores = 0, maxVivas = 0, maxQuads = 0;
    std::uint64_t pasosEnCombate = 0;
    bool incoherente = false, polvoEnCombate = false;
    std::vector<double> ms;
};

Camino camina(std::uint32_t llavor, bool reducido, double segundos) {
    PasosDePolvo p;
    p.monta(llavor, 64, 32, nullptr);
    Camino c;
    int x = 0, dir = 1;
    double acumulado = 0.0;
    const int ticks = static_cast<int>(segundos / kTick);
    std::vector<Destello> out;
    for (int t = 0; t < ticks; ++t) {
        const double seg = t * kTick;
        const bool combate = (seg >= 8.0 && seg < 12.0) || (seg >= 20.0 && seg < 21.0);
        acumulado += kTick;
        if (acumulado >= 0.16) {
            acumulado -= 0.16;
            if (combate) x += 4 * dir; else x += dir;   // en combate el heroe se mueve varias casillas por tick
            if (x > 40 || x < 0) { dir = -dir; x += 2 * dir; }
        }
        const std::uint64_t antes = p.pasos();
        const auto t0 = Reloj::now();
        p.avanza(kTick, GridCoord{x, 3}, reducido, combate);
        c.ms.push_back(msDesde(t0));
        if (combate && p.pasos() != antes) c.polvoEnCombate = true;
        std::size_t suma = 0;
        for (const Emisor& e : p.emisores()) suma += e.buffer().vivas();
        if (suma != p.presupuesto().vivas) c.incoherente = true;
        c.maxEmisores = std::max(c.maxEmisores, p.emisores().size());
        c.maxVivas = std::max(c.maxVivas, p.presupuesto().vivas);
        out.clear();
        std::size_t quads = 0;
        for (const Emisor& e : p.emisores())
            quads += proyectarParticulas(e, estiloDePolvo(), kMaxQuadsPorFotograma - quads, out).quads;
        if (quadsDeDestellos(out) != quadsComoPintaDestellos(out)) c.incoherente = true;
        c.maxQuads = std::max(c.maxQuads, quads);
    }
    c.pasosEnCombate = p.pasos();
    c.hash = hashDe(p.emisores(), p.presupuesto());
    return c;
}

MOTOR_TEST_CASE(caminar_sin_parar_mantiene_los_topes_del_polvo) {
    // Complementa los 10 s/20 pasos de test_polvo_pasos con 30 s, combate (D-O7) y proyeccion de quads.
    for (const bool reducido : {false, true}) {
        const Camino a = camina(7, reducido, 30.0);
        const Camino b = camina(7, reducido, 30.0);
        MOTOR_CHECK(a.hash == b.hash);                       // repetible
        MOTOR_CHECK(a.maxEmisores <= static_cast<std::size_t>(kMaxEmisoresPolvo));
        MOTOR_CHECK(a.maxVivas <= static_cast<std::size_t>(kPresupuestoPolvo));
        MOTOR_CHECK(!a.incoherente && !a.polvoEnCombate);    // sin polvo ni pasos nuevos en combate
        MOTOR_CHECK(a.maxQuads <= static_cast<std::size_t>(kPresupuestoPolvo) * 4);   // Voluta = 4 quads
        std::printf("[E08] polvo 30 s paso/0,16 s %s: emisores max %zu (tope %d), vivas max %zu (tope %d), quads max %zu, "
                    "CPU Debug avanza mediana %.4f ms max %.4f ms\n",
                    reducido ? "reducido" : "normal", a.maxEmisores, kMaxEmisoresPolvo, a.maxVivas, kPresupuestoPolvo,
                    a.maxQuads, mediana(a.ms), maximo(a.ms));
    }
    // 17.o paso seguido sin esperar: no pasa de 16 nubes (kMaxEmisoresPolvo)
    PasosDePolvo p;
    p.monta(3, 64, 32, nullptr);
    p.avanza(kTick, GridCoord{0, 0}, false);
    for (int i = 1; i <= 40; ++i) p.avanza(kTick, GridCoord{i, 0}, false);   // un paso por tick: 40 nubes pedidas
    MOTOR_CHECK(p.emisores().size() <= static_cast<std::size_t>(kMaxEmisoresPolvo));
    MOTOR_CHECK(p.presupuesto().vivas <= static_cast<std::size_t>(kPresupuestoPolvo));
    MOTOR_CHECK(p.pasos() == 40u);
}

}  // namespace

int main() { return motor_test::runAllTests(); }
